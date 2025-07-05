/*
 * @file abort_takeoff_config.hpp
 * @brief 中止起飞场景参数配置头文件
 *
 * 本文件定义了ParaSAFE仿真系统中"中止起飞"场景的所有参数，包括油门、刹车、速度、加速度、距离、时间等物理量的默认值和可调节范围，
 * 并提供了从配置文件加载参数、获取控制器参数结构体等功能接口。
 *
 * 通过集中管理和灵活配置这些参数，用户可以方便地进行不同工况下的仿真实验、批量参数分析和场景复现。
 *
 * 典型用途：
 *   - 作为中止起飞场景的唯一参数入口，支持一键修改和批量仿真
 *   - 支持配置文件自动加载，便于参数管理和复现
 *   - 为控制器、动力学模型等模块提供标准化参数接口
 */

#ifndef ABORT_TAKEOFF_CONFIG_H
#define ABORT_TAKEOFF_CONFIG_H

#pragma once

// C++系统头文件
#include <chrono>      // 时间库，支持高精度计时和时间操作
#include <fstream>     // 文件流库，支持文件读写
#include <iostream>    // 标准输入输出流，支持控制台输入输出
#include <sstream>     // 字符串流库，支持字符串格式化和流式操作

// ParaSAFE系统头文件
#include "../../include/L_Simulation_Settings/simulation_config_base.hpp" // 仿真配置基类，支持参数管理和配置文件解析
#include "../../include/C_Flight_Control/controller_config.hpp"                 // 控制器参数配置，定义油门、刹车、巡航等控制器的参数结构体


namespace AbortTakeoffConfig {

    // ===================== 控制参数 =====================
    double MAX_THROTTLE = 1.0;  // 最大油门（单位：无量纲，1.0为100%）
    double MIN_THROTTLE = 0.0;  // 最小油门
    double MAX_BRAKE = 1.0;     // 最大刹车（单位：无量纲，1.0为100%）
    double MIN_BRAKE = 0.0;     // 最小刹车
    double THROTTLE_INCREASE_RATE = 0.2;  // 油门每秒最大增加速率（20%/s）
    double THROTTLE_DECREASE_RATE = 1.0;  // 油门每秒最大减少速率（100%/s）
    double BRAKE_RATE = 0.5;     // 刹车每秒最大增加速率（50%/s）

    // ===================== 速度参数 =====================
    double TARGET_SPEED = 100.0;     // 目标速度 (单位：m/s)
    double ABORT_SPEED = 40.0;       // 中止起飞速度阈值 (单位：m/s)
    double ZERO_VELOCITY_THRESHOLD = 0.1;   // 认为"静止"的速度阈值 (单位：m/s)
    double CRUISE_SPEED = 3.0;             // 巡航速度 (单位：m/s)
    double SPEED_TOLERANCE = 0.5;           // 速度控制容差 (单位：m/s)
    double MAX_SPEED = 120.0;               // 最大允许速度 (单位：m/s)
    double MIN_SPEED = 0.0;                 // 最小允许速度 (单位：m/s)
    double KNOTS_RATIO = 0.53996;           // 节与米每秒的换算系数

    // ===================== 加速度参数 =====================
    double MAX_ACCELERATION = 10.0;         // 最大加速度 (单位：m/s^2)
    double MAX_DECELERATION = -15.0;        // 最大减速度 (单位：m/s^2)
    double ACCELERATION = 10.0;             // 默认加速度 (单位：m/s^2)
    double DECELERATION = 10.0;             // 默认减速度 (单位：m/s^2)
    double ABORT_ACCELERATION_THRESHOLD = -5.0;  // 中止起飞的加速度阈值 (单位：m/s^2)
    double MAX_THROTTLE_RATE = 0.2;             // 油门最大变化率 (单位：1/s)
    double MAX_BRAKE_RATE = 0.5;                // 刹车最大变化率 (单位：1/s)

    // ===================== 距离参数 =====================
    double ABORT_DISTANCE_THRESHOLD = 1000.0;  // 中止起飞距离阈值 (单位：m)
    double FINAL_STOP_DISTANCE = 1000.0;       // 最终停车距离 (单位：m)

    // ===================== 时间参数 =====================
    double ABORT_DECISION_TIME = 2.0;       // 中止决策时间 (单位：s)
    double ABORT_REACTION_TIME = 1.0;       // 中止反应时间 (单位：s)
    double SIMULATION_TIME_STEP = 0.01;     // 仿真时间步长 (单位：s)

    // ===================== 控制律参数 =====================
    double SPEED_CONTROL_KP = 0.1;             // 速度控制比例系数
    double SPEED_CONTROL_KI = 0.01;            // 速度控制积分系数
    double SPEED_CONTROL_KD = 0.05;            // 速度控制微分系数

    // ===================== 配置文件读取函数 =====================
    /**
     * @brief 从配置文件读取参数，支持覆盖默认值
     * @param filename 配置文件名，默认为 "abort_takeoff_config.txt"
     * @note 支持注释和空行，参数格式为 key = value
     */
    void loadConfig(const std::string& filename = "abort_takeoff_config.txt") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "[AbortTakeoffConfig] 配置文件不存在，使用默认值" << std::endl;
            return;
        }

        std::string line;
        int line_count = 0;
        while (std::getline(file, line)) {
            line_count++;
            
            // 跳过空行和注释行
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // 查找等号位置
            size_t equal_pos = line.find('=');
            if (equal_pos == std::string::npos) {
                std::cout << "[AbortTakeoffConfig] 警告：第" << line_count << "行格式错误，缺少等号: " << line << std::endl;
                continue;
            }
            
            // 提取参数名和数值
            std::string key = line.substr(0, equal_pos);
            std::string value_str = line.substr(equal_pos + 1);
            
            // 去除前后空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value_str.erase(0, value_str.find_first_not_of(" \t"));
            value_str.erase(value_str.find_last_not_of(" \t") + 1);
            
            // 转换为数值
            double value;
            try {
                value = std::stod(value_str);
            } catch (const std::exception& e) {
                std::cout << "[AbortTakeoffConfig] 警告：第" << line_count << "行数值格式错误: " << value_str << std::endl;
                continue;
            }
            
            // 根据参数名设置对应的值
            if (key == "MAX_THROTTLE") {
                MAX_THROTTLE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MIN_THROTTLE") {
                MIN_THROTTLE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_BRAKE") {
                MAX_BRAKE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MIN_BRAKE") {
                MIN_BRAKE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "THROTTLE_INCREASE_RATE") {
                THROTTLE_INCREASE_RATE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "THROTTLE_DECREASE_RATE") {
                THROTTLE_DECREASE_RATE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "BRAKE_RATE") {
                BRAKE_RATE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "TARGET_SPEED") {
                TARGET_SPEED = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ABORT_SPEED") {
                ABORT_SPEED = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ZERO_VELOCITY_THRESHOLD") {
                ZERO_VELOCITY_THRESHOLD = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "CRUISE_SPEED") {
                CRUISE_SPEED = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "SPEED_TOLERANCE") {
                SPEED_TOLERANCE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_SPEED") {
                MAX_SPEED = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MIN_SPEED") {
                MIN_SPEED = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_ACCELERATION") {
                MAX_ACCELERATION = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_DECELERATION") {
                MAX_DECELERATION = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ACCELERATION") {
                ACCELERATION = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "DECELERATION") {
                DECELERATION = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ABORT_ACCELERATION_THRESHOLD") {
                ABORT_ACCELERATION_THRESHOLD = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_THROTTLE_RATE") {
                MAX_THROTTLE_RATE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "MAX_BRAKE_RATE") {
                MAX_BRAKE_RATE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ABORT_DISTANCE_THRESHOLD") {
                ABORT_DISTANCE_THRESHOLD = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "FINAL_STOP_DISTANCE") {
                FINAL_STOP_DISTANCE = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ABORT_DECISION_TIME") {
                ABORT_DECISION_TIME = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "ABORT_REACTION_TIME") {
                ABORT_REACTION_TIME = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "SIMULATION_TIME_STEP") {
                SIMULATION_TIME_STEP = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "SPEED_CONTROL_KP") {
                SPEED_CONTROL_KP = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "SPEED_CONTROL_KI") {
                SPEED_CONTROL_KI = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else if (key == "SPEED_CONTROL_KD") {
                SPEED_CONTROL_KD = value;
                std::cout << "[AbortTakeoffConfig] 加载 " << key << " = " << value << std::endl;
            }
            else {
                std::cout << "[AbortTakeoffConfig] 警告：未知参数 " << key << std::endl;
            }
        }
        
        std::cout << "[AbortTakeoffConfig] 配置文件加载完成" << std::endl;
    }

    /**
     * @brief 设置仿真时间步长
     * @param dt 新的时间步长 (单位：s)
     */
    inline void setSimulationTimeStep(double dt) { SIMULATION_TIME_STEP = dt; }
    /**
     * @brief 获取仿真时间步长
     * @return 当前仿真时间步长 (单位：s)
     */
    inline double getSimulationTimeStep() { return SIMULATION_TIME_STEP; }

    // ===================== 控制器参数获取函数 =====================
    /**
     * @brief 获取油门控制器参数配置
     * @return ThrottleConfig 结构体，包含油门控制器相关参数
     */
    inline ControllerConfig::ThrottleConfig getThrottleConfig() {
        ControllerConfig::ThrottleConfig config;
        config.max_rate = MAX_THROTTLE_RATE;
        config.min_value = MIN_THROTTLE;
        config.max_value = MAX_THROTTLE;
        config.tolerance = SPEED_TOLERANCE;
        config.kp = SPEED_CONTROL_KP;
        config.ki = SPEED_CONTROL_KI;
        config.kd = SPEED_CONTROL_KD;
        config.target_speed = CRUISE_SPEED;
        return config;
    }

    /**
     * @brief 获取刹车控制器参数配置
     * @return BrakeConfig 结构体，包含刹车控制器相关参数
     */
    inline ControllerConfig::BrakeConfig getBrakeConfig() {
        ControllerConfig::BrakeConfig config;
        config.max_rate = MAX_BRAKE_RATE;
        config.min_value = MIN_BRAKE;
        config.max_value = MAX_BRAKE;
        config.tolerance = SPEED_TOLERANCE;
        config.kp = SPEED_CONTROL_KP;
        config.ki = SPEED_CONTROL_KI;
        config.kd = SPEED_CONTROL_KD;
        config.target_speed = 0.0;  // 目标速度为0
        return config;
    }

    /**
     * @brief 获取巡航控制器参数配置
     * @return CruiseConfig 结构体，包含巡航控制器相关参数
     */
    inline ControllerConfig::CruiseConfig getCruiseConfig() {
        ControllerConfig::CruiseConfig config;
        config.max_rate = MAX_THROTTLE_RATE;
        config.min_value = MIN_THROTTLE;
        config.max_value = MAX_THROTTLE;
        config.tolerance = SPEED_TOLERANCE;
        config.kp = SPEED_CONTROL_KP;
        config.ki = SPEED_CONTROL_KI;
        config.kd = SPEED_CONTROL_KD;
        config.target_speed = CRUISE_SPEED;
        config.speed_tolerance = SPEED_TOLERANCE;
        return config;
    }
}

#endif // ABORT_TAKEOFF_CONFIG_H
