/*
 * @file generic_events.hpp
 * @brief 通用事件定义头文件
 *
 * 本文件定义了ParaSAFE仿真系统中的通用事件类型和控制器动作枚举。
 * 提供统一的事件系统接口，支持事件驱动的仿真控制。
 *
 * 主要功能：
 *   - 定义控制器动作枚举类型
 *   - 提供事件系统的基础接口
 *   - 支持事件驱动的控制逻辑
 *   - 统一事件命名和分类
 */

#pragma once

// C++系统头文件
#include <string>           // 字符串类型，用于事件名称和描述

namespace GenericEvents {
    /**
     * @brief 通用控制器动作枚举
     * 
     * 定义了所有飞行场景中可能需要的控制器操作类型。
     * 这些动作可以被任何场景使用，确保控制器行为的一致性。
     */
    enum class ControllerAction {
        
        // ===== 油门控制动作 =====
        START_THROTTLE_INCREASE,    ///< 启动油门增加控制器 - 使飞机加速
        STOP_THROTTLE_INCREASE,     ///< 停止油门增加控制器 - 停止加速
        START_THROTTLE_DECREASE,    ///< 启动油门减少控制器 - 使飞机减速
        STOP_THROTTLE_DECREASE,     ///< 停止油门减少控制器 - 停止减速
        
        // ===== 刹车控制动作 =====
        START_BRAKE,                ///< 启动刹车控制器 - 施加刹车力
        STOP_BRAKE,                 ///< 停止刹车控制器 - 释放刹车
        
        // ===== 巡航控制动作 =====
        START_CRUISE,               ///< 启动巡航控制器 - 保持恒定速度
        STOP_CRUISE,                ///< 停止巡航控制器 - 退出巡航模式
        
        // ===== 俯仰角控制动作 =====
        START_PITCH_CONTROL,        ///< 启动俯仰角控制器 - 控制飞机俯仰角
        STOP_PITCH_CONTROL,         ///< 停止俯仰角控制器 - 退出俯仰角控制模式
        SET_PITCH_ANGLE,            ///< 设置目标俯仰角 - 指定期望的俯仰角
        
        // ===== 系统控制动作 =====
        STOP_ALL_CONTROLLERS,       ///< 停止所有控制器 - 紧急停止所有自动控制
        
        // ===== 飞行模式切换动作 =====
        SWITCH_TO_AUTO_MODE,        ///< 切换到自动模式 - 完全由系统控制
        SWITCH_TO_MANUAL_MODE,      ///< 切换到手动模式 - 完全由飞行员控制
        SWITCH_TO_SEMI_AUTO_MODE    ///< 切换到半自动模式 - 飞行员和系统协同控制
    };
} 