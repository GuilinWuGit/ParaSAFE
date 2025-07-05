#pragma once

#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>
#include <sstream>
#include <iomanip>
#include "../K_Scenario/shared_state.hpp"
#include "../K_Scenario/event_bus.hpp"
#include "../L_Simulation_Settings/simulation_clock.hpp"

// 仿真控制线程类
class SimulationControlThread {
private:
    SharedStateSpace& state;
    EventBus& bus;
    std::thread control_thread;
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};

    void run() {
        log_detail("[仿真控制] 仿真控制线程已启动\n");
        log_detail("[仿真控制] 按空格键暂停/恢复仿真，按ESC键结束仿真\n");
        
        bool last_space_pressed = false;
        bool last_esc_pressed = false;
        
        while (running) {
            // 检查空格键（暂停/恢复）
            bool space_pressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
            if (space_pressed && !last_space_pressed) {
                if (paused.load()) {
                    // 恢复仿真
                    paused.store(false);
                    state.simulation_running.store(true);
                    SimulationClock::getInstance().resume();
                    log_detail("[仿真控制] 仿真已恢复\n");
                } else {
                    // 暂停仿真
                    paused.store(true);
                    state.simulation_running.store(false);
                    SimulationClock::getInstance().pause();
                    log_detail("[仿真控制] 仿真已暂停\n");
                }
            }
            last_space_pressed = space_pressed;
            
            // 检查ESC键（结束仿真）
            bool esc_pressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
            if (esc_pressed && !last_esc_pressed) {
                log_detail("[仿真控制] 用户按ESC键，准备结束仿真\n");
                state.simulation_running.store(false);
                SimulationClock::getInstance().stop();
                break;
            }
            last_esc_pressed = esc_pressed;
            
            // 检查位置或时间超限条件
            if (state.position.load() > 1500.0 || state.simulation_time.load() > 180.0) {
                double current_position = state.position.load();
                double current_time = state.simulation_time.load();
                
                std::ostringstream oss;
                oss << "[仿真控制] 检测到仿真停止条件：\n";
                oss << "  当前位置: " << std::fixed << std::setprecision(2) << current_position << "m\n";
                oss << "  当前时间: " << std::fixed << std::setprecision(2) << current_time << "s\n";
                
                if (current_position > 1000.0) {
                    oss << "  停止原因: 位置超过1000米限制\n";
                }
                if (current_time > 60.0) {
                    oss << "  停止原因: 时间超过60秒限制\n";
                }
                oss << "[仿真控制] 自动结束仿真\n";
                
                log_detail(oss.str());
                state.simulation_running.store(false);
                SimulationClock::getInstance().stop();
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        log_detail("[仿真控制] 仿真控制线程已结束\n");
    }

public:
    SimulationControlThread(SharedStateSpace& state, EventBus& bus) 
        : state(state), bus(bus) {}

    void start() {
        if (!running) {
            running = true;
            control_thread = std::thread(&SimulationControlThread::run, this);
        }
    }

    void stop() {
        if (running) {
            running = false;
            if (control_thread.joinable()) {
                control_thread.join();
            }
        }
    }

    void join() {
        if (control_thread.joinable()) {
            control_thread.join();
        }
    }

    bool isRunning() const { return running.load(); }
    bool isPaused() const { return paused.load(); }
};

namespace SimulationManager {

// 仿真管理线程函数
inline void simulationManagerThread(SharedStateSpace& state, EventBus& bus) {
    // 确保初始状态正确
    state.simulation_started.store(false);
    state.simulation_running.store(true);
    
    // 等待所有组件初始化完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 输出初始化完成提示
    log_detail("\n==========================================\n");
    log_detail("        仿真系统初始化完成\n");
    log_detail("==========================================\n");
    
    // 等待仿真开始
    while (!state.simulation_started.load() && state.simulation_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // 设置系统就绪和用户确认状态
    state.system_ready.store(true);
    state.user_confirmed.store(true);
    
    // 直接设置仿真开始状态，不发布事件
    state.simulation_started.store(true);
    log_detail("[状态] 仿真已开始\n");

    // 等待用户按ESC键来停止模拟
    log_detail("模拟运行中... 按ESC键停止\n");
    while (state.simulation_running) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            // 直接设置仿真停止状态，不发布事件
            state.simulation_running.store(false);
            log_detail("[仿真控制] 检测到用户按ESC键\n");
            log_detail("[仿真控制] 用户主动停止仿真\n");
            log_detail("[状态] 用户按ESC键，仿真停止\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 停止模拟
    state.simulation_running = false;
    log_detail("[状态] 设置 simulation_running = false\n");
}

// 启动仿真管理线程
inline std::thread startSimulationManager(SharedStateSpace& state, EventBus& bus) {
    return std::thread(simulationManagerThread, std::ref(state), std::ref(bus));
}

} // namespace SimulationManager 