/*
 * @file brake_controller.hpp
 * @brief 刹车控制器头文件
 *
 * 本文件实现了ParaSAFE仿真系统的刹车控制器，用于控制飞机的刹车系统。
 * 提供精确的刹车控制功能，支持自动和手动模式下的刹车操作。
 *
 * 主要功能：
 *   - 刹车力计算和控制
 *   - 线程安全的控制逻辑
 *   - 实时状态监控和日志记录
 *   - 与事件系统的集成
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
#include "controller_config.hpp"      // 控制器配置，定义控制器参数和配置结构
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步和步进控制
#include "../L_Simulation_Settings/thread_name_util.hpp"  // 线程命名工具，便于调试和监控

// 使用控制器配置中的参数
using namespace ControllerConfig;

// 移除本地常量定义
// constexpr double BRAKE_RATE = 0.2;  // 每秒增加20%
// constexpr double MAX_BRAKE = 1.0;   // 最大刹车

// 刹车控制器
class BrakeController : public BaseController {
private:
    std::atomic<bool> running{false};
    std::thread controller_thread;
    double last_update_time{0.0};

public:
    BrakeController(SharedStateSpace& state, EventBus& bus)
        : BaseController(state, bus) {
        log_detail("[刹车控制器] 初始化完成\n");
    }

    void start() override {
        if (!running) {
            running = true;
            controller_thread = std::thread(&BrakeController::run, this);
        }
    }

    void stop() override {
        if (running) {
            running = false;
            if (controller_thread.joinable()) {
                controller_thread.join();
            }
            log_detail("[刹车控制器] 已停止\n");
        }
    }

    bool isEnabled() const override {
        return state.brake_control_enabled;
    }

    std::string getName() const override {
        return "刹车";
    }

    double getCurrentValue() const override {
        return state.brake.load();
    }

private:
    void run() {
        ThreadNaming::set_current_thread_name("BrakeCtrl");
        const double FIXED_DT = 0.01; // 固定时间步长
        auto& clock = SimulationClock::getInstance();
        clock.registerThread(); // 注册线程
        size_t current_step = 0; // 记录当前线程处理到的步数
        
        while (running && clock.isRunning()) {
            clock.waitForNextStep(current_step);
            current_step = clock.getStepCount();
            if (state.brake_control_enabled) {
                updateBrake(FIXED_DT);
            }
            // 通知时钟本步骤已完成
            clock.notifyStepCompleted();
        }
        clock.unregisterThread(); // 注销线程
    }

    void updateBrake(double dt) {
        double current_brake = state.brake.load();
        double new_brake = current_brake + BRAKE_RATE * dt;
        if (new_brake > MAX_BRAKE) {
            new_brake = MAX_BRAKE;
        }
        state.brake.store(new_brake);
        printBrakeStatus(new_brake);
    }

    void printBrakeStatus(double brake) {
        std::ostringstream oss;
        oss << "[刹车控制器] 当前刹车: " << std::fixed << std::setprecision(3) << (brake * 100) << "%\n";
        log_detail(oss.str());
    }
}; 