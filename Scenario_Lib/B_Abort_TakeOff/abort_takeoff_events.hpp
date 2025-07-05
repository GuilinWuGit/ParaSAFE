/*
 * @file abort_takeoff_events.hpp
 * @brief 中止起飞场景事件定义头文件
 * 
 * 本文件用于定义ParaSAFE仿真系统中"中止起飞"场景的所有事件，包括事件优先级、事件定义结构体、事件响应动作、事件映射表等，
 * 支持事件驱动仿真和灵活扩展。
 * 
 * 典型用途：
 *   - 统一管理和扩展中止起飞场景的所有事件
 *   - 支持事件订阅、触发、响应等机制
 */

 // C++系统头文件
#pragma once
#include <string>         // 字符串类型，事件名称和描述
#include <functional>     // 函数对象和回调，事件触发条件
#include <map>            // 有序映射容器（历史遗留，现已用unordered_map）
#include <vector>         // 向量容器，存储事件动作
#include <iomanip>        // 输出格式控制，日志美化
#include <sstream>        // 字符串流，日志格式化
#include <unordered_map>  // 哈希表容器，事件映射等

// ParaSAFE系统头文件
#include "../../include/K_Scenario/shared_state.hpp"      // 共享状态空间结构体
#include "../../include/K_Scenario/event_bus.hpp"         // 事件总线，事件发布与订阅
#include "../../include/K_Scenario/generic_events.hpp"    // 通用事件与控制器动作定义

// 本科目头文件
#include "abort_takeoff_config.hpp"                       // 中止起飞场景参数配置


/**
 * @namespace AbortTakeoffEvents
 * @brief 中止起飞场景的事件定义、优先级、事件响应结构体和事件映射表。
 */
namespace AbortTakeoffEvents {
    /**
     * @brief 事件优先级枚举
     * HIGH：最高优先级
     * MEDIUM：中等优先级
     * LOW：最低优先级
     */
    enum class Priority {
        HIGH = 0,    // 最高优先级
        MEDIUM = 1,  // 中等优先级
        LOW = 2      // 最低优先级
    };

    /**
     * @brief 事件响应动作结构体
     * actions：要执行的控制器操作列表
     * description：响应动作描述
     */
    struct EventResponse {
        std::vector<GenericEvents::ControllerAction> actions;  ///< 要执行的控制器操作列表
        std::string description;                ///< 响应动作描述
    };

    /**
     * @brief 事件定义结构体
     * name：事件名称
     * description：事件描述
     * priority：事件优先级
     * trigger_condition：事件触发条件
     * response：事件响应动作
     * triggered：事件触发标志
     */
    struct EventDefinition {
        std::string name;                    ///< 事件名称
        std::string description;             ///< 事件描述
        Priority priority;                   ///< 事件优先级
        std::function<bool(const SharedStateSpace&)> trigger_condition;  ///< 触发条件
        EventResponse response;              ///< 事件响应动作
        bool triggered{false};               ///< 事件触发标志
    };

    // ===================== 事件名称常量 =====================
    const std::string START_THROTTLE = "START_THROTTLE"; ///< 开始推油门事件
    const std::string ABORT_TAKEOFF  = "ABORT_TAKEOFF";  ///< 中止起飞事件
    const std::string START_CRUISE   = "START_CRUISE";   ///< 开始巡航事件
    const std::string START_BRAKE    = "START_BRAKE";    ///< 开始刹车事件
    const std::string FINAL_STOP     = "FINAL_STOP";     ///< 最终停止事件

    /**
     * @brief 事件定义映射表（事件名称到事件定义的映射）
     * 用于事件订阅和处理。
     */
    const std::unordered_map<std::string, ::EventDefinition> EVENT_DEFINITIONS = {
        {START_THROTTLE, {
            START_THROTTLE,
            "开始推油门事件",
            [](const SharedStateSpace& state) {
                return state.simulation_started.load() && 
                       state.simulation_running.load() &&
                       state.simulation_time.load() >= 1.0;
            },
            {GenericEvents::ControllerAction::SWITCH_TO_AUTO_MODE, GenericEvents::ControllerAction::START_THROTTLE_INCREASE},
            "切换到自动模式并启动油门增加控制器",
            false
        }},
        {ABORT_TAKEOFF, {
            ABORT_TAKEOFF,
            "中止起飞事件",
            [](const SharedStateSpace& state) {
                return state.velocity.load() >= AbortTakeoffConfig::ABORT_SPEED && 
                       !state.abort_triggered.load();
            },
            {GenericEvents::ControllerAction::STOP_THROTTLE_INCREASE, GenericEvents::ControllerAction::START_THROTTLE_DECREASE, GenericEvents::ControllerAction::START_BRAKE},
            "停止油门增加控制器，启动油门减小控制器，启动刹车控制器",
            false
        }},
        {START_CRUISE, {
            START_CRUISE,
            "开始巡航事件",
            [](const SharedStateSpace& state) {
                return state.velocity.load() <= 4.17 && // 15km/h
                       state.position.load() < 1500.0 && // 位置小于1500米
                       state.abort_triggered.load();
            },
            {GenericEvents::ControllerAction::STOP_THROTTLE_DECREASE, GenericEvents::ControllerAction::STOP_BRAKE, GenericEvents::ControllerAction::START_CRUISE},
            "停止油门减少控制器和刹车控制器，启动巡航控制器",
            false
        }},
        {START_BRAKE, {
            START_BRAKE,
            "开始刹车事件",
            [](const SharedStateSpace& state) {
                return state.position.load() >= 1000.0 ;
            },
            {GenericEvents::ControllerAction::START_BRAKE},
            "启动刹车控制器",
            false
        }},
        {FINAL_STOP, {
            FINAL_STOP,
            "最终停止事件",
            [](const SharedStateSpace& state) {
                return state.velocity.load() <= AbortTakeoffConfig::ZERO_VELOCITY_THRESHOLD &&
                       state.position.load() >= 1000.0 &&  // 确保已经过了刹车点
                       state.abort_triggered.load();       // 确保是中止起飞过程
            },
            {GenericEvents::ControllerAction::STOP_ALL_CONTROLLERS, GenericEvents::ControllerAction::SWITCH_TO_MANUAL_MODE},
            "停止所有控制器并切换到手动模式",
            false
        }},
    };

    /**
     * @brief 事件枚举类型，便于类型安全的事件引用
     */
    enum class AbortTakeoffEvents {
        START_THROTTLE, ///< 开始推油门
        ABORT_TAKEOFF,  ///< 中止起飞
        START_CRUISE,   ///< 开始巡航
        STOP_CRUISE,    ///< 停止巡航
        FINAL_STOP,     ///< 最终停止
    };
}