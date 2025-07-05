/*
 * @file DynamicsModel_FixedWing_Linear.hpp
 * @brief 固定翼飞机线性动力学模型头文件
 *
 * 本文件实现了ParaSAFE仿真系统中用于固定翼飞机的线性动力学模型。
 * 该模型假设飞机的动力学为线性，适用于基础训练和教学场景。
 * 支持与共享状态空间、事件系统、物理参数、力模型等模块协同工作。
 *
 * 主要功能：
 *   - 计算飞机在当前状态下的合力、加速度、速度和位置
 *   - 支持与控制器、物理参数、力模型的集成
 *   - 支持仿真步进与状态队列推送
 *   - 支持状态打印与日志记录
 *
 * 后续可扩展为非线性模型、直升机等其他机型动力学模型。
 */

#pragma once

// ================= C++系统头文件 =================
#include <sstream>      // 字符串流，用于格式化输出状态
#include <iomanip>      // 输出格式化，控制数值精度
#include <condition_variable>  // 条件变量（预留多线程同步）
#include <mutex>        // 互斥锁（预留多线程同步）
#include <thread>       // 线程支持（预留多线程扩展）
#include <chrono>       // 时间库，支持高精度计时
#include <atomic>       // 原子操作，保证多线程数据安全
#include <cmath>        // 数学库，支持基本运算
#include <iostream>     // 控制台输出

// ================= ParaSAFE系统头文件 =================
#include "../K_Scenario/shared_state.hpp"      // 共享状态空间，存储仿真系统的所有状态变量
#include "../K_Scenario/event_bus.hpp"         // 事件总线，处理事件发布和订阅机制
#include "../L_Simulation_Settings/data_recorder.hpp"  // 数据记录器，支持仿真数据记录
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供统一的仿真步进
#include "../L_Simulation_Settings/simulation_config_base.hpp"  // 仿真基础配置接口
#include "../L_Simulation_Settings/logger.hpp" // 日志系统，支持详细/简要日志输出
#include "../A_Aircraft_Configuration/aircraft_config.hpp" // 飞机物理参数配置
#include "../B_Aircraft_Forces_Model/ACForceModel.hpp"  // 飞机力模型，计算合力
#include "../C_Flight_Control/controller_config.hpp"     // 控制器参数配置
#include "../K_Scenario/state_update_queue.hpp" // 状态更新队列，支持多线程状态同步

// ================= 命名空间与配置参数 =================
using namespace SimulationConfig;   // 仿真全局配置参数
using namespace ControllerConfig;   // 控制器相关参数

// ================= 前向声明辅助函数 =================
/**
 * @brief 计算当前状态下的加速度
 * @param state 共享状态空间
 * @return 加速度（单位：m/s²）
 */
double calculateAcceleration(const SharedStateSpace& state);

/**
 * @brief 打印当前仿真状态到日志
 * @param time 当前仿真时间
 * @param position 当前位移
 * @param velocity 当前速度
 * @param acceleration 当前加速度
 * @param throttle 当前油门
 * @param brake 当前刹车
 */
void printState(double time, double position, double velocity, double acceleration, double throttle, double brake);

// 动力学模型接口
class IDynamicsModel {
public:
    virtual ~IDynamicsModel() = default;
    virtual void step(SharedStateSpace& state, StateUpdateQueue& queue, EventBus& bus, SimulationClock& clock,
                      std::shared_ptr<AircraftConfigBase> aircraftConfig, std::shared_ptr<IForceModel> forceModel) = 0;
};

// 线性动力学模型实现
class DynamicsModel_FixedWing_Linear : public IDynamicsModel {
public:
    void step(SharedStateSpace& state, StateUpdateQueue& queue, EventBus& bus, SimulationClock& clock,
              std::shared_ptr<AircraftConfigBase> aircraftConfig, std::shared_ptr<IForceModel> forceModel) override {
        // 1. 计算当前合力（推力、阻力、刹车力）
        ForceResult forces = forceModel->calculateNetForce(state, state.velocity.load(), aircraftConfig);
        // 2. 更新共享状态空间中的力值
        state.thrust.store(forces.thrust);
        state.drag_force.store(forces.drag);
        state.brake_force.store(forces.brake_force);
        // 3. 计算加速度 a = F/m
        double acceleration = forces.net_force / aircraftConfig->getMass();
        // 4. 获取当前速度和位置
        double current_velocity = state.velocity.load();
        double current_position = state.position.load();
        // 5. 更新速度 v = v0 + a*dt
        double new_velocity = current_velocity + acceleration * 0.01; // 固定步长dt=0.01s
        // 6. 更新位置 x = x0 + v0*dt
        double new_position = current_position + current_velocity * 0.01; // 固定步长dt=0.01s
        // 7. 将新状态推送到状态队列（线程安全）
        queue.push({StateUpdateType::Velocity, new_velocity});
        queue.push({StateUpdateType::Position, new_position});
        queue.push({StateUpdateType::Acceleration, acceleration});
        // 8. 记录当前状态
        // state.printState();
        // 9. 更新仿真时间
        state.simulation_time.store(clock.getCurrentTime(), std::memory_order_release);
    }
};

// ================= 非线性动力学模型实现 =================
/**
 * @brief 固定翼飞机非线性动力学模型
 * 示例：引入速度相关的非线性扰动、阻力系数随速度变化等
 */
class DynamicsModel_FixedWing_Nonlinear : public IDynamicsModel {
public:
    void step(SharedStateSpace& state, StateUpdateQueue& queue, EventBus& bus, SimulationClock& clock,
              std::shared_ptr<AircraftConfigBase> aircraftConfig, std::shared_ptr<IForceModel> forceModel) override {
        // 1. 计算当前合力（推力、阻力、刹车力），假设力学模型已体现非线性
        ForceResult forces = forceModel->calculateNetForce(state, state.velocity.load(), aircraftConfig);
        // 2. 更新共享状态空间中的力值
        state.thrust.store(forces.thrust);
        state.drag_force.store(forces.drag);
        state.brake_force.store(forces.brake_force);
        // 3. 计算非线性加速度 a = f(v, F, ...)
        double current_velocity = state.velocity.load();
        double mass = aircraftConfig->getMass();
        // 非线性扰动项：随速度变化的附加加速度
        double nonlinear_term = 0.5 * std::sin(current_velocity / 10.0);
        double acceleration = (forces.net_force / mass) + nonlinear_term;
        // 4. 获取当前速度和位置
        double current_position = state.position.load();
        // 5. 更新速度 v = v0 + a*dt + 0.5*扰动项
        double dt = 0.01;
        double new_velocity = current_velocity + acceleration * dt + 0.1 * std::cos(current_velocity / 8.0);
        // 6. 更新位置 x = x0 + v0*dt + 0.5*a*dt^2
        double new_position = current_position + current_velocity * dt + 0.5 * acceleration * dt * dt;
        // 7. 将新状态推送到状态队列（线程安全）
        queue.push({StateUpdateType::Velocity, new_velocity});
        queue.push({StateUpdateType::Position, new_position});
        queue.push({StateUpdateType::Acceleration, acceleration});
        // 8. 记录当前状态
        // state.printState();
        // 9. 更新仿真时间
        state.simulation_time.store(clock.getCurrentTime(), std::memory_order_release);
    }
};

// ================= 辅助函数实现 =================
/**
 * @brief 计算当前状态下的加速度
 * @param state 共享状态空间
 * @return 加速度（单位：m/s²）
 */
double calculateAcceleration(const SharedStateSpace& state, std::shared_ptr<AircraftConfigBase> aircraftConfig, std::shared_ptr<IForceModel> forceModel) {
    ForceResult forces = forceModel->calculateNetForce(state, state.velocity.load(), aircraftConfig);
    double total_force = forces.net_force;
    return total_force / aircraftConfig->getMass();
}

/**
 * @brief 打印当前系统的运动状态到日志，便于调试
 * @param time 当前仿真时间
 * @param position 当前位移
 * @param velocity 当前速度
 * @param acceleration 当前加速度
 * @param throttle 当前油门
 * @param brake 当前刹车
 *
 * 以统一格式输出到详细日志和简要日志，便于调试和分析。
 */
void printState(double time, double position, double velocity, double acceleration, double throttle, double brake) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2)
           << "[动力学模型状态] 时间: " << time << "s"
           << " 位置: " << position << "m"
           << " 速度: " << velocity << "m/s"
           << " 加速度: " << acceleration << "m/s²"
           << " 油门: " << std::setprecision(3) << (throttle * 100) << "%"
           << " 刹车: " << std::setprecision(3) << (brake * 100) << "%"
           << std::endl;
    log_detail(stream.str());
    log_brief(stream.str());
} 