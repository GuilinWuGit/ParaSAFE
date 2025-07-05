/**  
 * @file taxi_initial_state.hpp
 * @brief 滑行（Taxi）场景初始状态设置头文件
 *
 * 本文件用于定义ParaSAFE仿真系统中"滑行（Taxi）"场景的初始状态设置，包括位置、速度、油门、刹车、质量、目标速度等关键物理量的初始化，
 * 并提供了初始化和重置共享状态空间的标准接口。
 *
 * 典型用途：
 *   - 在仿真主程序启动时，调用 initializeMotionState 对共享状态空间进行标准化初始化
 *   - 支持仿真多次复位和批量实验，保证每次仿真起点一致
 */

#pragma once

//C++系统头文件
#include <atomic>      // 原子操作，保证多线程安全
#include <thread>      // 线程库
#include <chrono>      // 时间库

// ParaSAFE系统头文件
#include "../../include/K_Scenario/shared_state.hpp"                  // 共享状态空间结构体
#include "../../include/A_Aircraft_Configuration/aircraft_config.hpp" // 飞机物理参数配置
#include "../../include/L_Simulation_Settings/simulation_clock.hpp"   // 仿真时钟

// 本科目头文件
#include "Taxi_config.hpp"         // 滑行场景参数配置

/**
 * @class TaxiInitialState
 * @brief 滑行（Taxi）场景的初始状态设置类，提供初始化和重置接口
 */

class TaxiInitialState {
public:
    /**
     * @brief 初始化共享状态空间的所有关键物理量和控制标志
     * @param state 需要初始化的共享状态空间对象
     * @param aircraftConfig 飞机配置对象
     * @return 初始化是否成功
     * @note 包括位置、速度、油门、刹车、质量、目标速度、控制标志、仿真步长等
     */
    static bool initializeMotionState(SharedStateSpace& state, std::shared_ptr<AircraftConfigBase> aircraftConfig) {
        try {
            // 初始化位置（单位：米）
            state.position.store(30.0);
            log_detail("[共享状态空间初始化] 位置已设置为0.0\n");

            // 初始化速度（单位：米/秒）
            state.velocity.store(0.0);
            log_detail("[共享状态空间初始化] 速度已设置为0.0\n");

            // 初始化油门（单位：无量纲，0~1）
            state.throttle.store(0.0);
            log_detail("[共享状态空间初始化] 油门已设置为0.0\n");

            // 初始化刹车（单位：无量纲，0~1）
            state.brake.store(0.0);
            log_detail("[共享状态空间初始化] 刹车已设置为0.0\n");

            // 初始化质量（单位：kg）
            log_detail("[共享状态空间初始化] 质量已设置为" + std::to_string(aircraftConfig->getMass()) + "\n");

            // 初始化目标速度（单位：米/秒）
            state.target_speed.store(TaxiConfig::TARGET_SPEED);  // 目标速度
            log_detail("[共享状态空间初始化] 目标速度已设置为" + std::to_string(TaxiConfig::TARGET_SPEED) + "\n");

            // 初始化控制标志（全部重置为false）
            state.throttle_control_enabled = false;
            state.brake_control_enabled = false;
            state.cruise_control_enabled = false;
            state.final_stop_enabled = false;
            log_detail("[共享状态空间初始化] 控制标志已重置\n");

            // 初始化仿真步长（单位：秒）
            SimulationClock::getInstance().setTimeStep(TaxiConfig::SIMULATION_TIME_STEP);
            log_detail("[共享状态空间初始化] 仿真步长已设置为" + std::to_string(TaxiConfig::SIMULATION_TIME_STEP) + "\n");

            return true;
        } catch (const std::exception& e) {
            log_detail("[错误] 初始化失败: " + std::string(e.what()) + "\n");
            return false;
        }
    }

    /**
     * @brief 重置共享状态空间的所有关键物理量和控制标志
     * @param state 需要重置的共享状态空间对象
     * @param aircraftConfig 飞机配置对象
     * @note 适用于仿真复位、批量实验等场景
     */
    static void resetMotionState(SharedStateSpace& state, std::shared_ptr<AircraftConfigBase> aircraftConfig) {
        // 重置位置
        state.position.store(0.0);
        // 重置速度
        state.velocity.store(0.0);
        // 重置油门
        state.throttle.store(0.0);
        // 重置刹车
        state.brake.store(0.0);
        // 重置质量
        log_detail("[共享状态空间重置] 质量已重置为" + std::to_string(aircraftConfig->getMass()) + "\n");
        // 重置目标速度
        state.target_speed.store(TaxiConfig::TARGET_SPEED);
        // 重置控制标志
        state.throttle_control_enabled = false;
        state.brake_control_enabled = false;
        state.cruise_control_enabled = false;
        state.final_stop_enabled = false;
        log_detail("[重置] 运动状态已重置\n");
    }
}; 