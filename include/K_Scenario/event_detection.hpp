/*
 * @file event_detection.hpp
 * @brief 通用事件检测线程头文件
 *
 * 本文件定义了ParaSAFE仿真系统的通用事件检测线程，
 * 支持任意场景下的事件检测、触发和事件总线交互。
 *
 * 典型用途：
 *   - 监测仿真状态，判断并触发各类场景自定义事件
 *   - 与事件总线、共享状态空间等模块协同工作，实现事件驱动仿真
 */

#pragma once

// C++系统头文件
#include <iostream>           // 标准输入输出流，调试和日志输出
#include <thread>             // 线程库，支持多线程事件检测
#include <atomic>             // 原子操作，保证多线程安全
#include <chrono>             // 时间库，支持高精度计时
#include <mutex>              // 互斥锁，线程同步
#include <condition_variable> // 条件变量，线程同步
#include <functional>         // 函数对象和回调，支持事件处理
#include <queue>              // 队列容器，事件排队
#include <any>                // 任意类型容器，事件数据传递
#include <unordered_map>      // 哈希表容器，事件映射等

// ParaSAFE系统头文件
#include "../../include/K_Scenario/shared_state.hpp"               // 共享状态空间结构体
#include "../../include/K_Scenario/event_bus.hpp"                  // 事件总线，事件发布与订阅
#include "../../include/L_Simulation_Settings/logger.hpp"          // 日志模块，支持详细/简要日志输出
#include "../../include/L_Simulation_Settings/simulation_clock.hpp"// 仿真时钟
#include "../../include/L_Simulation_Settings/thread_name_util.hpp"// 线程命名工具，便于调试

// 本科目头文件



// 事件监测线程定义
class EventMonitorThread {
private:
    SharedStateSpace& state;
    EventBus& bus;
    const std::unordered_map<std::string, EventDefinition>& event_definitions; // 通用事件定义表
    std::thread monitor_thread;
    std::atomic<bool> running{false};
    
    // 本地事件触发状态跟踪
    std::unordered_map<std::string, bool> local_triggered_events;
    mutable std::mutex local_events_mutex;

    void check_events() {
        ThreadNaming::set_current_thread_name("EventMonitor");
        auto& clock = SimulationClock::getInstance();
        clock.registerThread();
        bool last_simulation_running = state.simulation_running.load();
        bool last_simulation_started = state.simulation_started.load();
        double last_check_time = 0.0;
        size_t current_step = 0; // 记录当前线程处理到的步数
        
        while (running) {
            if (clock.isRunning()) {
                clock.waitForNextStep(current_step);
                current_step = clock.getStepCount(); // 更新为最新的时钟步数
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }
            double current_time = clock.getCurrentTime();
            bool current_simulation_running = state.simulation_running.load();
            bool current_simulation_started = state.simulation_started.load();
            if (current_simulation_running != last_simulation_running) {
                log_detail("[事件监测] 仿真运行状态变化: " + 
                    std::string(last_simulation_running ? "运行中" : "已停止") + " -> " +
                    std::string(current_simulation_running ? "运行中" : "已停止") + "\n");
                last_simulation_running = current_simulation_running;
            }
            if (current_simulation_started != last_simulation_started) {
                log_detail("[事件监测] 仿真开始状态变化: " + 
                    std::string(last_simulation_started ? "已开始" : "未开始") + " -> " +
                    std::string(current_simulation_started ? "已开始" : "未开始") + "\n");
                last_simulation_started = current_simulation_started;
            }
            for (const auto& [name, event] : event_definitions) {
                bool already_triggered = false;
                {
                    std::lock_guard<std::mutex> lock(local_events_mutex);
                    auto it = local_triggered_events.find(name);
                    already_triggered = (it != local_triggered_events.end() && it->second);
                }
                
                // 如果事件未触发过且满足触发条件，则触发事件
                if (!already_triggered && event.trigger_condition(state)) {
                    {
                        std::lock_guard<std::mutex> lock(local_events_mutex);
                        local_triggered_events[name] = true;
                    }
                    bus.publish(name);
                    log_detail("[事件监测] 触发事件: " + name + " 在时间: " + 
                        std::to_string(current_time) + " 秒\n");
                }
            }
            clock.notifyStepCompleted();
            last_check_time = current_time;
        }
        clock.unregisterThread();
    }

public:
    EventMonitorThread(SharedStateSpace& state, EventBus& bus, const std::unordered_map<std::string, EventDefinition>& event_definitions)
        : state(state), bus(bus), event_definitions(event_definitions) {}

    void start() {
        if (!running) {
            running = true;
            monitor_thread = std::thread(&EventMonitorThread::check_events, this);
            log_detail("[事件监测线程] 已启动\n");
        }
    }
    void stop() {
        if (running) {
            running = false;
            if (monitor_thread.joinable()) {
                monitor_thread.join();
            }
            log_detail("[事件监测线程] 已停止\n");
        }
    }
    void join() {
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }
    bool isRunning() const { return running.load(); }
};

