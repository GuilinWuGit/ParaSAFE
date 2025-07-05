/*
 * @file throttle_controller.hpp
 * @brief 油门控制器头文件
 *
 * 本文件实现了ParaSAFE仿真系统的油门控制器，包括油门增加和油门减少两个控制器。
 * 提供精确的油门控制功能，支持自动和手动模式下的油门操作。
 *
 * 主要功能：
 *   - 油门增加控制器：逐步增加油门值
 *   - 油门减少控制器：逐步减少油门值
 *   - 线程安全的控制逻辑
 *   - 实时状态监控和日志记录
 */

#pragma once

// C++系统头文件
#include <iostream>         // 输入输出流，用于控制台输出和日志记录
#include <thread>           // 线程支持，用于创建和管理控制线程
#include <chrono>           // 时间库，提供高精度时间测量
#include <mutex>            // 互斥锁，提供线程同步机制
#include <condition_variable>  // 条件变量，用于线程间通信和等待
#include <atomic>           // 原子操作，确保多线程环境下的数据安全
#include <iomanip>          // 输出格式控制，用于数值的精确格式化显示
#include <sstream>          // 字符串流，用于格式化输出和状态信息

// ParaSAFE系统头文件
#include "base_controller.hpp"        // 控制器基类，提供控制器接口和基础功能
#include "../K_Scenario/shared_state.hpp"           // 共享状态空间，存储仿真系统的所有状态变量
#include "../K_Scenario/event_bus.hpp"              // 事件总线，处理事件发布和订阅机制
#include "../L_Simulation_Settings/logger.hpp"      // 日志系统，提供统一的日志记录功能
#include "../L_Simulation_Settings/simulation_config_base.hpp"  // 仿真配置基类，定义基础配置参数
#include "controller_config.hpp"      // 控制器配置，定义控制器参数和配置结构
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步和步进控制
#include "../K_Scenario/state_update_queue.hpp"     // 状态更新队列，管理状态更新操作
#include "../L_Simulation_Settings/thread_name_util.hpp"  // 线程命名工具，便于调试和监控

class ThrottleController_Increase : public BaseController {
private:
    SimulationClock& clock;
    StateUpdateQueue& queue;
    std::thread controller_thread;
    std::atomic<bool> running{false};
    const double THROTTLE_INCREASE_RATE = 0.1; // 每秒增加0.1，如果时间步长为0.01，那么每步增加0.001
    const double FIXED_DT = 0.01; // 固定时间步长

    void run() {
        ThreadNaming::set_current_thread_name("ThrottleIncreaseCtrl");
        clock.registerThread(); // 注册线程
        size_t current_step = 0; // 记录当前线程处理到的步数
        while (running && clock.isRunning()) {
            // 等待下一个时间步
            clock.waitForNextStep(current_step);
            current_step = clock.getStepCount();
            
            // 只有在油门控制启用时才更新
            if (state.throttle_control_enabled) {
                updateThrottle(FIXED_DT);
            }

            // 通知时钟本步骤已完成
            clock.notifyStepCompleted();
        }
        clock.unregisterThread(); // 注销线程
    }

    void updateThrottle(double dt) {
        double current_throttle = state.throttle.load();
        double new_throttle = current_throttle + THROTTLE_INCREASE_RATE * dt;
        
        // 确保油门值在0到1之间
        new_throttle = std::min(1.0, std::max(0.0, new_throttle));
        
        // 只有当油门值发生变化时才更新
        if (std::abs(new_throttle - current_throttle) > 1e-6) {
            queue.push({StateUpdateType::Throttle, new_throttle});
            std::ostringstream stream;
            stream << std::fixed << std::setprecision(2)
                   << "[油门控制器] 请求更新油门值: " << new_throttle << "\n";
            log_detail(stream.str());
        }
    }

public:
    ThrottleController_Increase(SharedStateSpace& state_ref, EventBus& bus_ref, SimulationClock& clock_ref, StateUpdateQueue& queue_ref)
        : BaseController(state_ref, bus_ref), clock(clock_ref), queue(queue_ref) {
        log_detail("[油门控制器] 初始化完成\n");
    }

    void start() override {
        if (!running) {
            running = true;
            controller_thread = std::thread(&ThrottleController_Increase::run, this);
            log_detail("[油门控制器] 已启动\n");
        }
    }

    void stop() override {
        if (running) {
            running = false;
            if (controller_thread.joinable()) {
                controller_thread.join();
            }
            log_detail("[油门控制器] 已停止\n");
        }
    }

    bool isEnabled() const override {
        return state.throttle_control_enabled;
    }

    std::string getName() const override {
        return "油门增加";
    }

    double getCurrentValue() const override {
        return state.throttle.load();
    }
};

class ThrottleController_Decrease : public BaseController {
private:
    std::thread controller_thread;
    std::atomic<bool> running{false};
    const double THROTTLE_DECREASE_RATE = 0.2; // 油门减小率
    const double FIXED_DT = 0.01; // 固定时间步长
    StateUpdateQueue& queue;

public:
    ThrottleController_Decrease(SharedStateSpace& state_ref, EventBus& bus_ref, StateUpdateQueue& queue_ref)
        : BaseController(state_ref, bus_ref), queue(queue_ref) {}

    void start() override {
        if (!running) {
            running = true;
            controller_thread = std::thread(&ThrottleController_Decrease::run, this);
        }
    }

    void stop() override {
        running = false;
        if (controller_thread.joinable()) {
            controller_thread.join();
        }
    }

    bool isEnabled() const override {
        return state.throttle_control_enabled;
    }

    std::string getName() const override {
        return "油门减少";
    }

    double getCurrentValue() const override {
        return state.throttle.load();
    }

private:
    void run() {
        ThreadNaming::set_current_thread_name("ThrottleDecreaseCtrl");
        const double FIXED_DT = 0.01; // 固定时间步长
        auto& clock = SimulationClock::getInstance();
        clock.registerThread(); // 注册线程
        size_t current_step = 0; // 记录当前线程处理到的步数
        
        while (running && clock.isRunning()) {
            // 先等待下一个时间步
            clock.waitForNextStep(current_step);
            current_step = clock.getStepCount();
            
            // 然后更新油门值
            if (state.throttle_control_enabled) {
                updateThrottle(FIXED_DT);
            }

            // 通知时钟本步骤已完成
            clock.notifyStepCompleted();
        }
        clock.unregisterThread(); // 注销线程
    }

    void updateThrottle(double dt) {
        double current_throttle = state.throttle.load();
        double new_throttle = current_throttle - THROTTLE_DECREASE_RATE * dt;
        if (new_throttle < 0.0) {
            new_throttle = 0.0;
        }
        queue.push({StateUpdateType::Throttle, new_throttle});
        printThrottleStatus(new_throttle);
    }

    void printThrottleStatus(double throttle) {
        std::ostringstream oss;
        oss << "[油门减少控制器] 当前油门: " << std::fixed << std::setprecision(3) << (throttle * 100) << "%\n";
        log_detail(oss.str());
    }
}; 