/*
 * @file controller_manager.hpp
 * @brief 控制器管理器头文件
 *
 * 本文件实现了ParaSAFE仿真系统的控制器管理器，负责管理所有控制器的生命周期。
 * 提供统一的控制器创建、启动、停止和事件处理接口。
 *
 * 主要功能：
 *   - 管理所有控制器的创建和销毁
 *   - 处理控制器事件和状态变化
 *   - 提供控制器状态监控
 *   - 支持事件驱动的控制器操作
 */

#pragma once

// C++系统头文件
#include <iostream>         // 输入输出流，用于控制台输出和日志记录
#include <thread>           // 线程支持，用于创建和管理控制线程
#include <chrono>           // 时间库，提供高精度时间测量
#include <mutex>            // 互斥锁，提供线程同步机制
#include <condition_variable>  // 条件变量，用于线程间通信和等待
#include <vector>           // 向量容器，存储控制器列表
#include <functional>       // 函数对象，支持回调函数
#include <memory>           // 智能指针，管理控制器对象
#include <unordered_map>    // 哈希表，存储控制器映射关系
#include <sstream>          // 字符串流，用于格式化输出
#include <iomanip>          // 输出格式控制，用于数值的精确格式化显示
#include <atomic>           // 原子操作，确保多线程环境下的数据安全
#include <queue>            // 队列容器，用于事件队列管理
#include <any>              // 通用类型，支持任意类型的事件数据

// ParaSAFE系统头文件
#include "../L_Simulation_Settings/simulation_config_base.hpp"  // 仿真配置基类，定义基础配置参数
#include "shared_state.hpp"           // 共享状态空间，存储仿真系统的所有状态变量
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步和步进控制
#include "event_bus.hpp"              // 事件总线，处理事件发布和订阅机制
#include "../L_Simulation_Settings/logger.hpp"      // 日志系统，提供统一的日志记录功能
#include "../C_Flight_Control/controller_config.hpp"      // 控制器配置，定义控制器参数和配置结构
#include "../C_Flight_Control/base_controller.hpp"        // 控制器基类，提供控制器接口和基础功能
#include "state_update_queue.hpp"     // 状态更新队列，管理状态更新操作
#include "generic_events.hpp"         // 通用事件定义，定义控制器动作枚举
#include "controller_actions_config.hpp"  // 控制器动作配置，定义动作映射关系

// 控制器头文件
#include "../C_Flight_Control/brake_controller.hpp"      // 刹车控制器，实现刹车控制功能
#include "../C_Flight_Control/CruiseOnRunway_controller.hpp"     // 跑道巡航控制器，实现跑道巡航控制功能
#include "../C_Flight_Control/throttle_controller.hpp"   // 油门控制器，实现油门控制功能
#include "../C_Flight_Control/PitchHold_controller.hpp"     // 俯仰角保持控制器，实现俯仰角控制功能

// 使用SimulationConfig命名空间中的配置
using namespace SimulationConfig;

// 前向声明：部分控制器类
class ThrottleController_Increase;
class ThrottleController_Decrease;
class BrakeController;

/**
 * @brief 控制器管理器线程类
 *
 * 该类负责统一管理所有控制器的生命周期，包括创建、启动、停止、事件响应等。
 * 支持事件驱动的控制器操作，保证多线程环境下的安全和高效。
 */
class ControllerManagerThread {
private:
    SharedStateSpace& state; ///< 共享状态空间引用，存储仿真系统的所有状态变量
    EventBus& bus;           ///< 事件总线引用，负责事件的发布与订阅
    StateUpdateQueue& queue; ///< 状态更新队列引用，管理状态更新操作
    std::atomic<bool> running{false}; ///< 控制管理线程运行状态的原子变量
    std::thread manager_thread;       ///< 控制器管理线程对象
    std::queue<std::function<void()>> event_queue; ///< 事件队列，存储待处理的事件回调
    std::mutex event_mutex;           ///< 事件队列互斥锁，保证多线程安全
    std::condition_variable event_cv; ///< 事件条件变量，用于线程间事件通知
    std::unordered_map<std::string, std::shared_ptr<BaseController>> controllers; ///< 控制器名称到控制器对象的映射表
    
    std::unordered_map<std::string, bool> triggered_events; ///< 已触发事件的记录表
    mutable std::mutex events_mutex; ///< 事件记录互斥锁，保证事件状态多线程安全

    // 事件定义和处理回调
    std::unordered_map<std::string, EventDefinition> event_definitions_; ///< 事件定义表，存储所有事件的定义
    std::function<void(const std::string&)> event_state_change_callback_; ///< 事件状态变化回调函数

    /**
     * @brief 管理线程主循环
     * 负责从事件队列中取出事件并依次处理，保证事件驱动的控制器操作。
     */
    void run() {
        while (running) {
            std::function<void()> event;
            {
                std::unique_lock<std::mutex> lock(event_mutex);
                event_cv.wait(lock, [this] { return !event_queue.empty() || !running; });
                if (!running) break;
                event = event_queue.front();
                event_queue.pop();
            }
            try {
                event();
                printControllerStatus(); // 每次事件处理后打印控制器状态
            } catch (const std::exception& e) {
                std::ostringstream oss;
                oss << "[ControllerManagerThread] Exception during event execution: " << e.what() << "\n";
                log_detail(oss.str());
            } catch (...) {
                log_detail("[ControllerManagerThread] Unknown exception during event execution\n");
            }
        }
    }

public:
    /**
     * @brief 构造函数（支持事件定义和回调）
     * @param state 共享状态空间引用
     * @param bus 事件总线引用
     * @param queue 状态更新队列引用
     * @param event_definitions 事件定义表
     * @param event_state_change_callback 事件状态变化回调函数
     *
     * 支持自定义事件定义和事件状态变化回调，便于灵活扩展。
     */
    ControllerManagerThread(SharedStateSpace& state, EventBus& bus, StateUpdateQueue& queue,
        const std::unordered_map<std::string, EventDefinition>& event_definitions,
        std::function<void(const std::string&)> event_state_change_callback = nullptr)
        : state(state), bus(bus), queue(queue), running(false),
          event_definitions_(event_definitions),
          event_state_change_callback_(event_state_change_callback) {
        createControllers(); // 创建所有控制器实例
    }
    /**
     * @brief 兼容原有构造函数（无事件定义）
     * @param state 共享状态空间引用
     * @param bus 事件总线引用
     * @param queue 状态更新队列引用
     *
     * 事件定义需后续通过setEventDefinitions设置。
     */
    ControllerManagerThread(SharedStateSpace& state, EventBus& bus, StateUpdateQueue& queue)
        : state(state), bus(bus), queue(queue), running(false) {
        createControllers(); // 创建所有控制器实例
    }

    /**
     * @brief 注册事件处理器
     *
     * 遍历事件定义表，为每个事件在事件总线注册回调。
     * 当事件被触发时，执行对应的控制器动作。
     */
    void setupEventHandlers() {
        for (const auto& [event_name, event_def] : event_definitions_) {
            bus.subscribe(event_name, [this, event_name](const std::any&) {
                if (isEventTriggered(event_name)) {
                    log_detail("[ControllerManagerThread] Event " + event_name + " already triggered, skipping.\n");
                    return;
                }
                auto it = event_definitions_.find(event_name);
                if (it != event_definitions_.end()) {
                    markEventTriggered(event_name); // 标记事件已触发
                    handleEventStateChanges(event_name); // 处理事件状态变化
                    executeControllerActions(it->second.actions); // 执行控制器动作
                }
            });
        }
    }

    /**
     * @brief 设置事件定义表
     * @param event_definitions 事件定义表
     *
     * 支持运行时动态更新事件定义。
     */
    void setEventDefinitions(const std::unordered_map<std::string, EventDefinition>& event_definitions) {
        event_definitions_ = event_definitions;
    }

    /**
     * @brief 执行控制器动作
     * @param actions 控制器动作枚举列表
     *
     * 根据动作类型，自动调用对应控制器的启动、停止、参数设置等操作。
     */
    void executeControllerActions(const std::vector<GenericEvents::ControllerAction>& actions) {
        for (const auto& action : actions) {
            std::string action_name = getActionName(action);
            const ControllerActionConfig* config = ControllerActionsConfig::getActionConfig(action_name);
            if (!config) {
                log_detail("[ControllerManagerThread] Warning: Action config not found for: " + action_name + "\n");
                continue;
            }
            
            if (config->action_type == "CONTROLLER") {
                applyStateSettings(config->state_settings); // 应用状态设置
                if (action_name.rfind("START_", 0) == 0) {
                    startController(config->controller_name); // 启动控制器
                } else if (action_name.rfind("STOP_", 0) == 0) {
                    stopController(config->controller_name); // 停止控制器
                }
            } else if (config->action_type == "STOP_ALL") {
                stopAllControllers(); // 停止所有控制器
            } else if (config->action_type == "MODE") {
                auto mode_it = config->state_settings.find("flight_mode");
                if (mode_it != config->state_settings.end()) {
                    setFlightMode(mode_it->second); // 设置飞行模式
                }
            }
            log_detail("[ControllerManagerThread] Executing action: " + action_name + " -> " + config->controller_name + "\n");
        }
    }

    /**
     * @brief 获取动作名称字符串
     * @param action 控制器动作枚举
     * @return 动作名称字符串
     */
    std::string getActionName(GenericEvents::ControllerAction action) {
        switch (action) {
            case GenericEvents::ControllerAction::START_THROTTLE_INCREASE: return "START_THROTTLE_INCREASE";
            case GenericEvents::ControllerAction::STOP_THROTTLE_INCREASE: return "STOP_THROTTLE_INCREASE";
            case GenericEvents::ControllerAction::START_THROTTLE_DECREASE: return "START_THROTTLE_DECREASE";
            case GenericEvents::ControllerAction::STOP_THROTTLE_DECREASE: return "STOP_THROTTLE_DECREASE";
            case GenericEvents::ControllerAction::START_BRAKE: return "START_BRAKE";
            case GenericEvents::ControllerAction::STOP_BRAKE: return "STOP_BRAKE";
            case GenericEvents::ControllerAction::START_CRUISE: return "START_CRUISE";
            case GenericEvents::ControllerAction::STOP_CRUISE: return "STOP_CRUISE";
            case GenericEvents::ControllerAction::START_PITCH_CONTROL: return "START_PITCH_CONTROL";
            case GenericEvents::ControllerAction::STOP_PITCH_CONTROL: return "STOP_PITCH_CONTROL";
            case GenericEvents::ControllerAction::SET_PITCH_ANGLE: return "SET_PITCH_ANGLE";
            case GenericEvents::ControllerAction::STOP_ALL_CONTROLLERS: return "STOP_ALL_CONTROLLERS";
            case GenericEvents::ControllerAction::SWITCH_TO_AUTO_MODE: return "SWITCH_TO_AUTO_MODE";
            case GenericEvents::ControllerAction::SWITCH_TO_MANUAL_MODE: return "SWITCH_TO_MANUAL_MODE";
            case GenericEvents::ControllerAction::SWITCH_TO_SEMI_AUTO_MODE: return "SWITCH_TO_SEMI_AUTO_MODE";
            default: return "UNKNOWN_ACTION";
        }
    }

    /**
     * @brief 应用状态设置
     * @param state_settings 状态设置映射表
     *
     * 根据配置表自动设置共享状态空间中的相关变量。
     */
    void applyStateSettings(const std::map<std::string, std::string>& state_settings) {
        for (const auto& [var_name, value] : state_settings) {
            if (var_name == "throttle_control_enabled") state.throttle_control_enabled = (value == "true");
            else if (var_name == "brake_control_enabled") state.brake_control_enabled = (value == "true");
            else if (var_name == "cruise_control_enabled") state.cruise_control_enabled = (value == "true");
            else if (var_name == "pitch_control_enabled") state.pitch_control_enabled = (value == "true");
        }
    }

    /**
     * @brief 设置飞行模式
     * @param mode_name 飞行模式名称
     *
     * 支持自动、手动、半自动三种模式。
     */
    void setFlightMode(const std::string& mode_name) {
        if (mode_name == "AUTO") state.setFlightMode(SharedStateSpace::FlightMode::AUTO);
        else if (mode_name == "MANUAL") state.setFlightMode(SharedStateSpace::FlightMode::MANUAL);
        else if (mode_name == "SEMI_AUTO") state.setFlightMode(SharedStateSpace::FlightMode::SEMI_AUTO);
    }

    /**
     * @brief 启动指定名称的控制器
     * @param name 控制器名称
     *
     * 自动检查权限，避免无权限时误操作。
     */
    void startController(const std::string& name) {
        auto it = controllers.find(name);
        if (it != controllers.end()) {
            // 权限检查：油门/刹车/巡航等需自动系统有相应权限
            if ((name == "油门增加" || name == "油门减少" || name == "跑道巡航") && !state.control_auth.auto_system_has_throttle_control) {
                log_detail("[ControllerManagerThread] Warning: Auto system lacks throttle control for: " + name + "\n");
                return;
            }
            if (name == "刹车" && !state.control_auth.auto_system_has_brake_control) {
                log_detail("[ControllerManagerThread] Warning: Auto system lacks brake control for: " + name + "\n");
                return;
            }
            it->second->start();
            log_detail("[ControllerManagerThread] Started controller: " + name + "\n");
        } else {
            log_detail("[ControllerManagerThread] Warning: Controller not found: " + name + "\n");
        }
    }

    /**
     * @brief 停止指定名称的控制器
     * @param name 控制器名称
     */
    void stopController(const std::string& name) {
        auto it = controllers.find(name);
        if (it != controllers.end()) {
            it->second->stop();
            log_detail("[ControllerManagerThread] Stopped controller: " + name + "\n");
        }
    }

    /**
     * @brief 停止所有控制器
     *
     * 用于仿真结束或紧急情况。
     */
    void stopAllControllers() {
        for (auto& [name, controller] : controllers) controller->stop();
        log_detail("[ControllerManagerThread] All controllers stopped\n");
    }

    /**
     * @brief 启动管理线程
     *
     * 启动后可异步处理事件和控制器操作。
     */
    void start() {
        if (!running) {
            running = true;
            manager_thread = std::thread(&ControllerManagerThread::run, this);
            log_detail("[ControllerManagerThread] Started.\n");
        }
    }

    /**
     * @brief 停止管理线程
     *
     * 停止所有控制器并安全退出线程。
     */
    void stop() {
        if (running) {
            running = false;
            for (auto& [name, controller] : controllers) controller->stop();
            event_cv.notify_all();
            if (manager_thread.joinable()) manager_thread.join();
            log_detail("[ControllerManagerThread] Stopped.\n");
        }
    }

    /**
     * @brief 等待管理线程结束
     */
    void join() {
        if (manager_thread.joinable()) manager_thread.join();
    }

    /**
     * @brief 查询管理线程是否正在运行
     * @return 是否运行中
     */
    bool isRunning() const { return running.load(); }

    /**
     * @brief 添加事件到事件队列
     * @param event 事件回调函数
     *
     * 支持异步事件驱动。
     */
    void addEvent(std::function<void()> event) {
        {
            std::lock_guard<std::mutex> lock(event_mutex);
            event_queue.push(event);
        }
        event_cv.notify_one();
    }

    /**
     * @brief 获取指定名称的控制器对象
     * @param name 控制器名称
     * @return 控制器智能指针
     */
    std::shared_ptr<BaseController> getController(const std::string& name) {
        auto it = controllers.find(name);
        return (it != controllers.end()) ? it->second : nullptr;
    }

    /**
     * @brief 打印所有控制器当前状态
     *
     * 便于调试和监控。
     */
    void printControllerStatus() {
        for (const auto& [name, controller] : controllers) {
            if (controller->isEnabled()) {
                std::ostringstream oss;
                oss << "[ControllerManagerThread] " << name << " current value: " << std::fixed << std::setprecision(2) << controller->getCurrentValue() << "\n";
                log_detail(oss.str());
            }
        }
        printTriggeredEvents(); // 打印已触发事件状态
    }

    /**
     * @brief 创建所有控制器实例
     *
     * 按照名称注册到controllers映射表，便于统一管理。
     */
    void createControllers() {
        controllers["油门增加"] = std::make_shared<ThrottleController_Increase>(state, bus, SimulationClock::getInstance(), queue);
        controllers["油门减少"] = std::make_shared<ThrottleController_Decrease>(state, bus, queue);
        controllers["刹车"] = std::make_shared<BrakeController>(state, bus);
        controllers["跑道巡航"] = std::make_shared<CruiseOnRunwayController>(state, bus);
        controllers["俯仰角保持"] = std::make_shared<PitchHoldController>(state, bus);
        
        log_detail("[ControllerManagerThread] Created controllers:\n");
        for (const auto& [name, controller] : controllers) log_detail("  " + name + "\n");
    }
    
    /**
     * @brief 标记事件为已触发
     * @param event_name 事件名称
     */
    void markEventTriggered(const std::string& event_name) {
        std::lock_guard<std::mutex> lock(events_mutex);
        triggered_events[event_name] = true;
        log_detail("[ControllerManagerThread] Event triggered: " + event_name + "\n");
    }
    
    /**
     * @brief 查询事件是否已触发
     * @param event_name 事件名称
     * @return 是否已触发
     */
    bool isEventTriggered(const std::string& event_name) const {
        std::lock_guard<std::mutex> lock(events_mutex);
        auto it = triggered_events.find(event_name);
        return it != triggered_events.end() && it->second;
    }
    
    /**
     * @brief 处理事件状态变化
     * @param event_name 事件名称
     *
     * 支持自定义回调，便于外部扩展。
     */
    void handleEventStateChanges(const std::string& event_name) {
        if (event_state_change_callback_) {
            event_state_change_callback_(event_name);
        }
    }
    
    /**
     * @brief 打印所有已触发事件状态
     *
     * 便于调试和监控。
     */
    void printTriggeredEvents() const {
        std::lock_guard<std::mutex> lock(events_mutex);
        log_detail("[ControllerManagerThread] Triggered event status:\n");
        for (const auto& [event_name, triggered] : triggered_events) {
            log_detail("  " + event_name + ": " + (triggered ? "triggered" : "not triggered") + "\n");
        }
    }
}; 