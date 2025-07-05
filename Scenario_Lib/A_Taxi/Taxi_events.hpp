/*
 * @file taxi_events.hpp
 * @brief 滑行（Taxi）场景事件定义头文件
 *
 * 本文件用于定义ParaSAFE仿真系统中"滑行（Taxi）"场景的所有事件，包括事件优先级、事件定义结构体、事件响应动作、事件映射表等，
 * 支持事件驱动仿真和灵活扩展。
 *
 * 典型用途：
 *   - 统一管理和扩展滑行场景的所有事件
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
#include "Taxi_config.hpp"                                // 滑行场景参数配置

/**
 * @namespace TaxiEvents
 * @brief 滑行（Taxi）场景的事件定义、优先级、事件响应结构体和事件映射表。
 */
namespace TaxiEvents {
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

    // 引入全局 EventDefinition 结构体
    using ::EventDefinition;

    // ===================== 事件名称常量 =====================
    const std::string START_THROTTLE = "START_THROTTLE"; ///< 开始增加油门事件
    const std::string START_BRAKE    = "START_BRAKE";    ///< 开始刹车事件
    const std::string FINAL_STOP     = "FINAL_STOP";     ///< 最终停止事件

    /**
     * @brief 事件定义映射表（事件名称到事件定义的映射）
     * 用于事件订阅和处理。
     * 按照全局 EventDefinition 成员顺序初始化：
     * name, description, trigger_condition, actions, response_description, triggered
     */
    const std::unordered_map<std::string, EventDefinition> EVENT_DEFINITIONS = {
        // 1. 开始增加油门事件：仿真时间1秒
        {START_THROTTLE, {
            START_THROTTLE,
            "开始增加油门事件",
            [](const SharedStateSpace& state) {
                return state.simulation_started.load() && 
                       state.simulation_running.load() &&
                       state.simulation_time.load() >= 1.0;
            },
            { GenericEvents::ControllerAction::START_THROTTLE_INCREASE },
            "启动油门增加控制器",
            false
        }},
        // 2. 开始刹车事件：距离达到500米
        {START_BRAKE, {
            START_BRAKE,
            "开始刹车事件",
            [](const SharedStateSpace& state) {
                return state.position.load() >= 500.0;
            },
            { GenericEvents::ControllerAction::START_THROTTLE_DECREASE, GenericEvents::ControllerAction::START_BRAKE },
            "启动油门减小控制器和刹车控制器",
            false
        }},
        // 3. 最终停止事件：速度接近0
        {FINAL_STOP, {
            FINAL_STOP,
            "最终停止事件",
            [](const SharedStateSpace& state) {
                return state.velocity.load() <= TaxiConfig::ZERO_VELOCITY_THRESHOLD;
            },
            { GenericEvents::ControllerAction::STOP_ALL_CONTROLLERS, GenericEvents::ControllerAction::SWITCH_TO_MANUAL_MODE },
            "停止所有控制器并切换到手动模式",
            false
        }},
    };

    /**
     * @brief 事件枚举类型，便于类型安全的事件引用
     */
    enum class TaxiEventsEnum {
        START_THROTTLE, ///< 开始增加油门
        START_BRAKE,    ///< 开始刹车
        FINAL_STOP,     ///< 最终停止
    };
}