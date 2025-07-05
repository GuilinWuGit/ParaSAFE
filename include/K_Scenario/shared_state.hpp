/*
 * @file shared_state.hpp
 * @brief 共享状态空间头文件
 *
 * 本文件定义了ParaSAFE仿真系统的共享状态空间，用于存储和管理所有仿真状态变量。
 * 提供线程安全的状态访问机制，支持多线程环境下的并发操作。
 *
 * 主要功能：
 *   - 定义仿真系统的所有状态变量
 *   - 提供线程安全的状态访问接口
 *   - 支持状态快照和版本控制
 *   - 实现飞行模式和控制权管理
 */

// shared_state.hpp
#pragma once

// C++系统头文件
#include <atomic>           // 原子操作，确保多线程环境下的数据安全
#include <string>           // 字符串类型，用于状态描述和模式名称
#include <mutex>            // 互斥锁，提供线程同步机制
#include <condition_variable>  // 条件变量，用于线程间通信和等待
#include <chrono>           // 时间库，提供高精度时间测量
#include <memory>           // 智能指针，管理动态内存
#include <functional>       // 函数对象，支持回调函数
#include <iomanip>          // 输出格式控制，用于状态信息的格式化显示

// ParaSAFE系统头文件
#include "../L_Simulation_Settings/simulation_clock.hpp"  // 仿真时钟，提供时间同步功能
#include "../L_Simulation_Settings/simulation_config_base.hpp"  // 仿真配置基类，定义基础配置参数
#include "../A_Aircraft_Configuration/aircraft_config.hpp"  // 飞机配置，定义飞机相关参数

// 状态快照结构体
struct StateSnapshot {
    double position;
    double velocity;
    double acceleration;
    double throttle;
    double brake;
    double thrust;
    double drag_force;
    double brake_force;
    double simulation_time;
    double pitch_angle;            ///< 俯仰角（弧度）
    double pitch_rate;             ///< 俯仰角变化率（弧度/秒）
    double pitch_control_output;    ///< 俯仰角控制输出
};

class SharedStateSpace {
public:
    // 构造函数
    SharedStateSpace() = default;

    // 需要外部访问的成员变量
    std::atomic<double> position{0.0};
    std::atomic<double> velocity{0.0};
    std::atomic<double> acceleration{0.0};
    std::atomic<double> throttle{0.0};
    std::atomic<double> brake{0.0};
    std::atomic<double> simulation_time{0.0};
    std::atomic<double> thrust{0.0};
    std::atomic<double> brake_force{0.0};
    std::atomic<double> drag_force{0.0};
    std::atomic<bool> simulation_running{false};
    std::atomic<bool> simulation_started{false};
    std::atomic<bool> final_stop_enabled{false};
    std::atomic<bool> throttle_control_enabled{false};
    std::atomic<bool> brake_control_enabled{false};
    std::atomic<bool> cruise_control_enabled{false};
    std::atomic<bool> abort_triggered{false};
    std::atomic<bool> system_ready{false};
    std::atomic<bool> user_confirmed{false};
    std::atomic<double> target_speed{0.0};
    std::atomic<double> abort_speed{0.0};
    std::atomic<double> abort_speed_threshold{0.0};
    std::atomic<int> zero_velocity_count{0};
    std::atomic<SimulationClock*> simulation_clock{nullptr};
    
    // 俯仰角控制相关状态变量
    std::atomic<double> pitch_angle{0.0};           ///< 当前俯仰角（弧度）
    std::atomic<double> pitch_rate{0.0};            ///< 俯仰角变化率（弧度/秒）
    std::atomic<double> pitch_control_output{0.0};  ///< 俯仰角控制输出
    std::atomic<bool> pitch_control_enabled{false}; ///< 俯仰角控制启用标志

    // 同步原语
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::mutex confirmation_mutex;
    std::condition_variable confirmation_cv;

    // 状态快照机制
    StateSnapshot current_state;
    std::mutex state_mutex;
    std::atomic<uint64_t> state_version{0};

    // 飞行模式管理
    enum class FlightMode {
        MANUAL,     // 手动模式 - 飞行员完全控制
        AUTO,       // 自动模式 - 系统自动控制
        SEMI_AUTO   // 半自动模式 - 部分自动，部分手动
    };
    
    std::atomic<FlightMode> flight_mode{FlightMode::MANUAL};
    
    // 控制权管理
    struct ControlAuthority {
        std::atomic<bool> pilot_has_throttle_control{true};    // 飞行员是否有油门控制权
        std::atomic<bool> pilot_has_brake_control{true};       // 飞行员是否有刹车控制权
        std::atomic<bool> auto_system_has_throttle_control{false}; // 自动系统是否有油门控制权
        std::atomic<bool> auto_system_has_brake_control{false};    // 自动系统是否有刹车控制权
    };
    
    ControlAuthority control_auth;
    
    // 状态快照方法
    void updateState(const StateSnapshot& new_state) {
        std::lock_guard<std::mutex> lock(state_mutex);
        current_state = new_state;
        state_version.fetch_add(1, std::memory_order_release);
    }
    StateSnapshot getState() const {
        std::unique_lock<std::mutex> lock(const_cast<std::mutex&>(state_mutex));
        return StateSnapshot{
            position.load(std::memory_order_acquire),
            velocity.load(std::memory_order_acquire),
            acceleration.load(std::memory_order_acquire),
            throttle.load(std::memory_order_acquire),
            brake.load(std::memory_order_acquire),
            thrust.load(std::memory_order_acquire),
            drag_force.load(std::memory_order_acquire),
            brake_force.load(std::memory_order_acquire),
            simulation_time.load(std::memory_order_acquire),
            pitch_angle.load(std::memory_order_acquire),
            pitch_rate.load(std::memory_order_acquire),
            pitch_control_output.load(std::memory_order_acquire)
        };
    }
    uint64_t getStateVersion() const {
        return state_version.load(std::memory_order_acquire);
    }
    bool waitForStateUpdate(uint64_t current_version, std::chrono::milliseconds timeout) {
        auto start = std::chrono::steady_clock::now();
        while (state_version.load(std::memory_order_acquire) == current_version) {
            if (std::chrono::steady_clock::now() - start > timeout) {
                return false;
            }
            std::this_thread::yield();
        }
        return true;
    }

    // 控制标志访问器
    bool isSimulationRunning() const { return simulation_running.load(); }
    void setSimulationRunning(bool value) { simulation_running.store(value); }
    bool isThrottleControlEnabled() const { return throttle_control_enabled.load(); }
    void setThrottleControlEnabled(bool value) { throttle_control_enabled.store(value); }
    bool isBrakeControlEnabled() const { return brake_control_enabled.load(); }
    void setBrakeControlEnabled(bool value) { brake_control_enabled.store(value); }
    bool isCruiseControlEnabled() const { return cruise_control_enabled.load(); }
    void setCruiseControlEnabled(bool value) { cruise_control_enabled.store(value); }
    bool isAbortTriggered() const { return abort_triggered.load(); }
    void setAbortTriggered(bool value) { abort_triggered.store(value); }
    bool isFinalStopEnabled() const { return final_stop_enabled.load(); }
    void setFinalStopEnabled(bool value) { final_stop_enabled.store(value); }

    // 用户确认等待
    void wait_for_user_confirmation() {
        std::unique_lock<std::mutex> lock(confirmation_mutex);
        confirmation_cv.wait(lock, [this] { return user_confirmed.load(); });
    }
    void confirm() {
        std::lock_guard<std::mutex> lock(confirmation_mutex);
        user_confirmed = true;
        confirmation_cv.notify_all();
    }

    // 等待仿真开始
    void wait_for_start() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { 
            return simulation_started.load(std::memory_order_acquire) || 
                  !simulation_running.load(std::memory_order_acquire); 
        });
        log_detail("[SharedState] 仿真开始等待完成\n");
    }

    // 通知仿真开始
    void notify_start() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            simulation_started.store(true, std::memory_order_release);
        }
        cv.notify_all();
        log_detail("[SharedState] 仿真开始通知已发送\n");
    }

    // 等待最终停止
    void wait_for_final_stop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { 
            return final_stop_enabled.load(std::memory_order_acquire) || 
                  !simulation_running.load(std::memory_order_acquire); 
        });
        log_detail("[SharedState] 最终停止等待完成\n");
    }

    // 通知最终停止
    void notify_final_stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            final_stop_enabled.store(true, std::memory_order_release);
        }
        cv.notify_all();
        log_detail("[SharedState] 最终停止通知已发送\n");
    }

    // 原子更新状态变量
    void update_state(double new_position, double new_velocity, double new_acceleration) {
        position.store(new_position, std::memory_order_release);
        velocity.store(new_velocity, std::memory_order_release);
        acceleration.store(new_acceleration, std::memory_order_release);
    }

    void update_pitch_state(double new_pitch_angle, double new_pitch_rate, double new_pitch_control_output) {
        pitch_angle.store(new_pitch_angle, std::memory_order_release);
        pitch_rate.store(new_pitch_rate, std::memory_order_release);
        pitch_control_output.store(new_pitch_control_output, std::memory_order_release);
    }

    // 原子更新控制变量
    void update_controls(double new_throttle, double new_brake) {
        throttle.store(new_throttle, std::memory_order_release);
        brake.store(new_brake, std::memory_order_release);
    }

    // 获取当前状态（原子读取）
    void get_state(double& pos, double& vel, double& acc) const {
        pos = position.load(std::memory_order_acquire);
        vel = velocity.load(std::memory_order_acquire);
        acc = acceleration.load(std::memory_order_acquire);
    }

    // 获取当前控制值（原子读取）
    void get_controls(double& thr, double& brk) const {
        thr = throttle.load(std::memory_order_acquire);
        brk = brake.load(std::memory_order_acquire);
    }

    // 速度相关
    std::atomic<double> dt{0.01};  // 10ms

    // 初始化回调函数类型
    using InitializationCallback = std::function<bool(SharedStateSpace&)>;

    // 创建共享状态空间的工厂函数
    static std::unique_ptr<SharedStateSpace> create(InitializationCallback init_callback = nullptr) {
        try {
            auto state = std::make_unique<SharedStateSpace>();
            
            // 基础初始化
            state->system_ready.store(true, std::memory_order_release);
            log_detail("[SharedState] 共享状态空间基础初始化成功\n");

            // 如果提供了初始化回调，则调用它
            if (init_callback) {
                if (!init_callback(*state)) {
                    log_detail("[错误] 共享状态空间自定义初始化失败\n");
                    return nullptr;
                }
                log_detail("[SharedState] 共享状态空间自定义初始化成功\n");
            }

            return state;
        } catch (const std::exception& e) {
            log_detail("[错误] 共享状态空间创建失败: " + std::string(e.what()) + "\n");
            return nullptr;
        }
    }

    // 模式切换方法
    void setFlightMode(FlightMode mode) {
        FlightMode old_mode = flight_mode.load();
        flight_mode.store(mode);
        
        // 根据模式设置控制权
        switch (mode) {
            case FlightMode::MANUAL:
                control_auth.pilot_has_throttle_control = true;
                control_auth.pilot_has_brake_control = true;
                control_auth.auto_system_has_throttle_control = false;
                control_auth.auto_system_has_brake_control = false;
                break;
            case FlightMode::AUTO:
                control_auth.pilot_has_throttle_control = false;
                control_auth.pilot_has_brake_control = false;
                control_auth.auto_system_has_throttle_control = true;
                control_auth.auto_system_has_brake_control = true;
                break;
            case FlightMode::SEMI_AUTO:
                // 半自动模式下，飞行员和自动系统可以共享控制权
                control_auth.pilot_has_throttle_control = true;
                control_auth.pilot_has_brake_control = true;
                control_auth.auto_system_has_throttle_control = true;
                control_auth.auto_system_has_brake_control = true;
                break;
        }
        
        log_detail("[FlightMode] 飞行模式切换: " + 
                  std::to_string(static_cast<int>(old_mode)) + " -> " + 
                  std::to_string(static_cast<int>(mode)) + "\n");
    }
    
    // 检查控制冲突
    bool hasControlConflict() const {
        return (control_auth.pilot_has_throttle_control && control_auth.auto_system_has_throttle_control) ||
               (control_auth.pilot_has_brake_control && control_auth.auto_system_has_brake_control);
    }

    void printState() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "时间: " << simulation_time.load() << "s, ";
        oss << "位置: " << position.load() << "m, ";
        oss << "速度: " << velocity.load() << "m/s, ";
        oss << "加速度: " << acceleration.load() << "m/s², ";
        oss << "油门: " << std::setprecision(3) << throttle.load() * 100 << "%, ";
        oss << "刹车: " << std::setprecision(3) << brake.load() * 100 << "%, ";
        oss << "推力: " << thrust.load() << "N, ";
        oss << "阻力: " << drag_force.load() << "N, ";
        oss << "刹车力: " << brake_force.load() << "N";
        
        // 添加飞行模式信息
        auto mode = flight_mode.load();
        std::string mode_str;
        switch (mode) {
            case FlightMode::MANUAL: mode_str = "手动"; break;
            case FlightMode::AUTO: mode_str = "自动"; break;
            case FlightMode::SEMI_AUTO: mode_str = "半自动"; break;
        }
        oss << ", 飞行模式: " << mode_str;
        
        // 添加控制权信息
        if (control_auth.pilot_has_throttle_control) oss << " [飞行员控制油门]";
        if (control_auth.auto_system_has_throttle_control) oss << " [自动系统控制油门]";
        if (control_auth.pilot_has_brake_control) oss << " [飞行员控制刹车]";
        if (control_auth.auto_system_has_brake_control) oss << " [自动系统控制刹车]";
        
        log_detail(oss.str() + "\n");
    }
};