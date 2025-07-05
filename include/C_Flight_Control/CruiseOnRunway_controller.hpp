/*
 * @file CruiseOnRunway_controller.hpp
 * @brief 跑道巡航控制器头文件
 *
 * 本文件实现了ParaSAFE仿真系统的跑道匀速滑行控制器，用于在跑道上保持飞机的恒定速度。
 * 提供精确的速度控制功能，支持自动匀速滑行模式，与空中巡航控制器区分开来。
 *
 * 主要功能：
 *   - 速度PID控制算法
 *   - 油门和刹车的协调控制
 *   - 线程安全的控制逻辑
 *   - 实时状态监控和日志记录
 */

#pragma once

// C++系统头文件
#include <iostream>         // 输入输出流，用于控制台输出和日志记录
#include <thread>           // 线程支持，用于创建和管理控制线程
#include <atomic>           // 原子操作，确保多线程环境下的数据安全
#include <sstream>          // 字符串流，用于格式化输出和状态信息
#include <iomanip>          // 输出格式控制，用于数值的精确格式化显示

// ParaSAFE系统头文件
#include "base_controller.hpp"        // 控制器基类，提供控制器接口和基础功能
#include "controller_config.hpp"      // 控制器配置，定义控制器参数和配置结构
#include "../K_Scenario/shared_state.hpp"           // 共享状态空间，存储仿真系统的所有状态变量
#include "../K_Scenario/event_bus.hpp"              // 事件总线，处理事件发布和订阅机制
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步和步进控制
#include "../L_Simulation_Settings/thread_name_util.hpp"  // 线程命名工具，便于调试和监控
#include "../L_Simulation_Settings/logger.hpp"      // 日志系统，提供统一的日志记录功能

// 使用控制器配置中的参数
using namespace ControllerConfig;
using SimulationConfig::CRUISE_GAIN;  // 只使用 SimulationConfig 中的 CRUISE_GAIN

// 跑道巡航控制器
class CruiseOnRunwayController : public BaseController {
public:
    CruiseOnRunwayController(SharedStateSpace& state, EventBus& bus)
        : BaseController(state, bus) {
        log_detail("[跑道巡航控制器] 初始化完成\n");
    }

    void start() override {
        if (!running) {
            running = true;
            controller_thread = std::thread(&CruiseOnRunwayController::run, this);
            log_detail("[跑道巡航控制器] 已启动\n");
        }
    }

    void stop() override {
        if (running) {
            running = false;
            if (controller_thread.joinable()) {
                controller_thread.join();
            }
            log_detail("[跑道巡航控制器] 已停止\n");
        }
    }

    bool isEnabled() const override {
        return state.cruise_control_enabled;
    }

    std::string getName() const override {
        return "跑道巡航";
    }

    double getCurrentValue() const override {
        return state.throttle.load();
    }

private:
    void run() {
        ThreadNaming::set_current_thread_name("CruiseOnRunwayCtrl");
        auto& clock = SimulationClock::getInstance();
        clock.registerThread(); // 注册线程
        log_detail("[跑道巡航控制器] 开始运行\n");
        size_t current_step = 0; // 记录当前线程处理到的步数
        
        while (running && clock.isRunning()) {
            clock.waitForNextStep(current_step);
            current_step = clock.getStepCount();
            if (state.cruise_control_enabled) {
                updateThrottle();
            }
            // 通知时钟本步骤已完成
            clock.notifyStepCompleted();
        }
        
        clock.unregisterThread(); // 注销线程
        log_detail("[跑道巡航控制器] 运行结束\n");
    }

    void updateThrottle() {
        // 根据当前速度和目标速度计算油门和刹车
        double current_velocity = state.velocity.load();
        double target_velocity = 100.0; // 默认巡航速度由场景外部设置

        // 计算油门和刹车
        auto [throttle, brake] = calculateThrottleAndBrake(current_velocity, target_velocity);
        
        // 更新油门和刹车
        state.throttle.store(throttle);
        state.brake.store(brake);

        // 打印状态
        printCruiseStatus(current_velocity, target_velocity, throttle, brake);
    }

    std::pair<double, double> calculateThrottleAndBrake(double current_velocity, double target_velocity) {
        // 简单的比例控制
        double velocity_error = target_velocity - current_velocity;
        
        double throttle = 0.0;
        double brake = 0.0;
        
        if (velocity_error > 0) {
            // 需要加速：增加油门，减少刹车
            throttle = CRUISE_GAIN * velocity_error;
            throttle = std::max(0.0, std::min(throttle, MAX_THROTTLE));
            brake = 0.0;  // 不需要刹车
        } else {
            // 需要减速：减少油门，增加刹车
            throttle = 0.0;  // 不需要油门
            brake = -CRUISE_GAIN * velocity_error;  // 注意这里velocity_error是负值
            brake = std::max(0.0, std::min(brake, MAX_BRAKE));
        }

        return {throttle, brake};
    }

    void printCruiseStatus(double current_velocity, double target_velocity, double throttle, double brake) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "[跑道巡航控制器] 当前速度: " << current_velocity 
           << " m/s, 目标速度: " << target_velocity
           << " m/s, 油门: " << std::setprecision(3) << throttle * 100 
           << "%, 刹车: " << std::setprecision(3) << brake * 100 << "%\n";
        log_detail(ss.str());
    }
}; 