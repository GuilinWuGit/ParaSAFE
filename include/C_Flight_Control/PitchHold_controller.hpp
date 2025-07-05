/*
 * @file PitchHold_controller.hpp
 * @brief 俯仰角保持控制器头文件
 *
 * 本文件实现了ParaSAFE仿真系统中的俯仰角保持控制器，用于控制飞机保持目标俯仰角。
 * 控制器采用PID控制算法，能够精确控制飞机的俯仰角，支持自动和手动模式。
 * 与未来的起飞离地控制器区分开来，专门用于保持指定俯仰角。
 *
 * 主要功能：
 *   - 俯仰角PID控制
 *   - 支持目标俯仰角设置
 *   - 实时状态监控和日志记录
 *   - 线程安全的控制逻辑
 */

#pragma once

// C++系统头文件
#include <iostream>         // 输入输出流，用于控制台输出和日志记录
#include <thread>           // 线程支持，用于创建和管理控制线程
#include <atomic>           // 原子操作，确保多线程环境下的数据安全
#include <sstream>          // 字符串流，用于格式化输出和状态信息
#include <iomanip>          // 输出格式控制，用于数值的精确格式化显示
#include <cmath>            // 数学函数库，提供PI常量和三角函数等

// ParaSAFE系统头文件
#include "base_controller.hpp"        // 控制器基类，提供控制器接口和基础功能
#include "../K_Scenario/shared_state.hpp"           // 共享状态空间，存储仿真系统的所有状态变量
#include "../K_Scenario/event_bus.hpp"              // 事件总线，处理事件发布和订阅机制
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步和步进控制
#include "../L_Simulation_Settings/thread_name_util.hpp"  // 线程命名工具，便于调试和监控
#include "../L_Simulation_Settings/logger.hpp"      // 日志系统，提供统一的日志记录功能
#include "controller_config.hpp"      // 控制器配置，定义控制器参数和配置结构

// 使用控制器配置中的参数
using namespace ControllerConfig;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 俯仰角保持控制器类
 * 
 * 继承自BaseController，实现俯仰角的PID控制算法。
 * 支持设置目标俯仰角，通过PID算法计算控制输出。
 * 专门用于保持指定俯仰角，与起飞离地控制器区分。
 */
class PitchHoldController : public BaseController {
public:
    /**
     * @brief 构造函数
     * @param state 共享状态空间引用
     * @param bus 事件总线引用
     */
    PitchHoldController(SharedStateSpace& state, EventBus& bus)
        : BaseController(state, bus) {
        log_detail("[俯仰角保持控制器] 初始化完成\n");
    }

    /**
     * @brief 启动控制器
     * 创建并启动控制线程
     */
    void start() override {
        if (!running) {
            running = true;
            controller_thread = std::thread(&PitchHoldController::run, this);
            log_detail("[俯仰角保持控制器] 已启动\n");
        }
    }

    /**
     * @brief 停止控制器
     * 停止控制线程并等待其结束
     */
    void stop() override {
        if (running) {
            running = false;
            if (controller_thread.joinable()) {
                controller_thread.join();
            }
            log_detail("[俯仰角保持控制器] 已停止\n");
        }
    }

    /**
     * @brief 检查控制器是否启用
     * @return 如果俯仰角控制启用则返回true
     */
    bool isEnabled() const override {
        return state.pitch_control_enabled;
    }

    /**
     * @brief 获取控制器名称
     * @return 控制器名称字符串
     */
    std::string getName() const override {
        return "俯仰角保持";
    }

    /**
     * @brief 获取当前控制值
     * @return 当前俯仰角控制输出值
     */
    double getCurrentValue() const override {
        return state.pitch_control_output.load();
    }

    /**
     * @brief 设置目标俯仰角
     * @param target_pitch 目标俯仰角（弧度）
     */
    void setTargetPitch(double target_pitch) {
        target_pitch_angle.store(target_pitch);
        log_detail("[俯仰角保持控制器] 设置目标俯仰角: " + std::to_string(target_pitch * 180.0 / M_PI) + " 度\n");
    }

    /**
     * @brief 获取目标俯仰角
     * @return 目标俯仰角（弧度）
     */
    double getTargetPitch() const {
        return target_pitch_angle.load();
    }

    /**
     * @brief 设置PID参数
     * @param kp 比例系数
     * @param ki 积分系数
     * @param kd 微分系数
     */
    void setPIDParameters(double kp, double ki, double kd) {
        pid_kp.store(kp);
        pid_ki.store(ki);
        pid_kd.store(kd);
        log_detail("[俯仰角保持控制器] 设置PID参数: Kp=" + std::to_string(kp) + 
                  ", Ki=" + std::to_string(ki) + ", Kd=" + std::to_string(kd) + "\n");
    }

    /**
     * @brief 重置PID积分项
     * 用于防止积分饱和
     */
    void resetIntegral() {
        integral_error.store(0.0);
        log_detail("[俯仰角保持控制器] 重置积分项\n");
    }

private:
    // PID控制参数
    std::atomic<double> pid_kp{1.0};        ///< 比例系数
    std::atomic<double> pid_ki{0.1};        ///< 积分系数
    std::atomic<double> pid_kd{0.05};       ///< 微分系数
    
    // 控制目标
    std::atomic<double> target_pitch_angle{0.0};  ///< 目标俯仰角（弧度）
    
    // PID控制状态
    std::atomic<double> integral_error{0.0};      ///< 积分误差
    std::atomic<double> previous_error{0.0};      ///< 上一次误差
    
    // 控制限制
    const double MAX_PITCH_ANGLE = M_PI / 4.0;    ///< 最大俯仰角（45度）
    const double MIN_PITCH_ANGLE = -M_PI / 4.0;   ///< 最小俯仰角（-45度）
    const double MAX_CONTROL_OUTPUT = 1.0;        ///< 最大控制输出
    const double MIN_CONTROL_OUTPUT = -1.0;       ///< 最小控制输出
    const double INTEGRAL_LIMIT = 10.0;           ///< 积分限幅

    /**
     * @brief 控制器主运行循环
     * 在独立线程中运行，实现实时控制
     */
    void run() {
        ThreadNaming::set_current_thread_name("PitchHoldCtrl");
        auto& clock = SimulationClock::getInstance();
        clock.registerThread(); // 注册线程
        log_detail("[俯仰角保持控制器] 开始运行\n");
        size_t current_step = 0; // 记录当前线程处理到的步数
        
        while (running && clock.isRunning()) {
            clock.waitForNextStep(current_step);
            current_step = clock.getStepCount();
            
            if (state.pitch_control_enabled) {
                updatePitchControl();
            }
            
            // 通知时钟本步骤已完成
            clock.notifyStepCompleted();
        }
        
        clock.unregisterThread(); // 注销线程
        log_detail("[俯仰角保持控制器] 运行结束\n");
    }

    /**
     * @brief 更新俯仰角控制
     * 计算PID控制输出并应用到飞机状态
     */
    void updatePitchControl() {
        // 获取当前状态
        double current_pitch = state.pitch_angle.load();
        double target_pitch = target_pitch_angle.load();
        
        // 计算控制输出
        double control_output = calculatePIDOutput(current_pitch, target_pitch);
        
        // 应用控制输出到飞机状态
        applyPitchControl(control_output);
        
        // 打印状态信息
        printPitchStatus(current_pitch, target_pitch, control_output);
    }

    /**
     * @brief 计算PID控制输出
     * @param current_pitch 当前俯仰角
     * @param target_pitch 目标俯仰角
     * @return PID控制输出值
     */
    double calculatePIDOutput(double current_pitch, double target_pitch) {
        // 计算误差
        double error = target_pitch - current_pitch;
        
        // 限制误差范围
        error = std::max(-M_PI, std::min(M_PI, error));
        
        // 比例项
        double kp = pid_kp.load();
        double proportional = kp * error;
        
        // 积分项
        double ki = pid_ki.load();
        double integral = integral_error.load() + ki * error * 0.01; // 假设时间步长为0.01s
        integral = std::max(-INTEGRAL_LIMIT, std::min(INTEGRAL_LIMIT, integral));
        integral_error.store(integral);
        
        // 微分项
        double kd = pid_kd.load();
        double derivative = kd * (error - previous_error.load()) / 0.01;
        previous_error.store(error);
        
        // 计算总输出
        double output = proportional + integral + derivative;
        
        // 限制输出范围
        output = std::max(MIN_CONTROL_OUTPUT, std::min(MAX_CONTROL_OUTPUT, output));
        
        return output;
    }

    /**
     * @brief 应用俯仰角控制
     * @param control_output 控制输出值
     */
    void applyPitchControl(double control_output) {
        // 将控制输出应用到飞机状态
        // 这里假设控制输出直接影响俯仰角变化率
        double pitch_rate = control_output * 0.1; // 缩放因子
        
        // 更新俯仰角（这里需要根据实际的物理模型来更新）
        // 暂时直接更新到共享状态中
        state.pitch_control_output.store(control_output);
        
        // 如果有俯仰角变化率状态变量，也可以更新
        // state.pitch_rate.store(pitch_rate);
    }

    /**
     * @brief 打印俯仰角控制状态
     * @param current_pitch 当前俯仰角
     * @param target_pitch 目标俯仰角
     * @param control_output 控制输出
     */
    void printPitchStatus(double current_pitch, double target_pitch, double control_output) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "[俯仰角保持控制器] 当前俯仰角: " << (current_pitch * 180.0 / M_PI) 
           << "°, 目标俯仰角: " << (target_pitch * 180.0 / M_PI)
           << "°, 控制输出: " << std::setprecision(3) << control_output << "\n";
        log_detail(ss.str());
    }
}; 