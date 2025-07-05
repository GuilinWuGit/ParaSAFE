/*
 * @file controller_config.hpp
 * @brief 控制器配置头文件
 *
 * 本文件定义了ParaSAFE仿真系统中所有控制器的配置参数和结构体。
 * 提供统一的控制器配置接口，支持不同类型控制器的参数管理。
 *
 * 主要功能：
 *   - 定义控制器配置基类和派生类
 *   - 提供各种控制器的参数结构
 *   - 定义控制器常量参数
 *   - 支持控制器状态回调函数
 */

#pragma once

// C++系统头文件
#include <functional>       // 函数对象，用于定义回调函数类型

namespace SimulationConfig {
    // 控制器常量
    constexpr double BRAKE_GAIN = 1000.0;      // 刹车增益
    
    constexpr double CRUISE_SPEED = 30.0;      // 巡航速度 (m/s)
    constexpr double CRUISE_GAIN = 0.1;        // 巡航控制增益
}

namespace ControllerConfig {
    // 控制器配置基类
    struct BaseConfig {
        double max_rate;        // 最大变化率
        double min_value;       // 最小值
        double max_value;       // 最大值
        double tolerance;       // 容差
    };

    // 油门控制器配置
    struct ThrottleConfig : public BaseConfig {
        double kp;             // 比例系数
        double ki;             // 积分系数
        double kd;             // 微分系数
        double target_speed;   // 目标速度
    };

    // 刹车控制器配置
    struct BrakeConfig : public BaseConfig {
        double kp;             // 比例系数
        double ki;             // 积分系数
        double kd;             // 微分系数
        double target_speed;   // 目标速度
    };

    // 巡航控制器配置
    struct CruiseConfig : public BaseConfig {
        double kp;             // 比例系数
        double ki;             // 积分系数
        double kd;             // 微分系数
        double target_speed;   // 目标速度
        double speed_tolerance; // 速度容差
    };

    // 俯仰角控制器配置
    struct PitchConfig : public BaseConfig {
        double kp;             // 比例系数
        double ki;             // 积分系数
        double kd;             // 微分系数
        double target_pitch;   // 目标俯仰角（弧度）
        double pitch_tolerance; // 俯仰角容差（弧度）
    };

    // 控制器状态回调函数类型
    using StateCallback = std::function<void()>;

    // 油门控制器参数
    constexpr double THROTTLE_RATE = 0.1;    // 油门变化率 (每秒)
    constexpr double MAX_THROTTLE = 1.0;     // 最大油门值
    constexpr double MIN_THROTTLE = 0.0;     // 最小油门值

    // 刹车控制器参数
    constexpr double BRAKE_RATE = 0.2;       // 刹车变化率 (每秒)
    constexpr double MAX_BRAKE = 1.0;        // 最大刹车值
    constexpr double MIN_BRAKE = 0.0;        // 最小刹车值

    // 巡航控制器参数
    constexpr double CRUISE_SPEED = 30.0;    // 巡航速度 (m/s)
    constexpr double SPEED_TOLERANCE = 0.5;  // 速度容差 (m/s)
} 