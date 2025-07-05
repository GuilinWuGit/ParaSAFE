#pragma once

#include <thread>
#include <atomic>
#include "../../include/K_Scenario/shared_state.hpp"
#include "../../include/L_Simulation_Settings/simulation_clock.hpp"
#include "../../include/K_Scenario/state_update_queue.hpp"
#include "../../include/L_Simulation_Settings/thread_name_util.hpp"
#include "logger.hpp"

class StateManagerThread {
private:
    SharedStateSpace& state_;
    StateUpdateQueue& queue_;
    SimulationClock& clock_;
    std::thread thread_;
    std::atomic<bool> running_{false};

    void run() {
        ThreadNaming::set_current_thread_name("StateManager");
        log_detail("[状态空间线程] 线程已启动\n");
        clock_.registerThread();
        size_t current_step = 0; // 记录当前线程处理到的步数

        while (running_.load(std::memory_order_acquire)) {
            
            // 1. 与仿真时钟同步，等待下一个时间步
            clock_.waitForNextStep(current_step);
            current_step = clock_.getStepCount();
            if (!clock_.isRunning()) {
                break;
            }

            // 2. 处理当前队列中的所有消息
            StateUpdateMessage msg;
            while (queue_.try_pop(msg)) {
                process_raw_update(msg);
            }

            // 3. 在这里执行周期性的二次处理
            perform_secondary_processing();

            // 4. 每步长输出一次状态
            state_.printState();

            // 5. 通知时钟本线程已完成当前步骤
            clock_.notifyStepCompleted();
        }

        clock_.unregisterThread();
        log_detail("[状态空间线程] 线程已结束\n");
    }

    // 处理从队列接收到的原始数据
    void process_raw_update(const StateUpdateMessage& msg) {
        switch (msg.type) {
            case StateUpdateType::Position:
                state_.position.store(msg.value);
                break;
            case StateUpdateType::Velocity:
                state_.velocity.store(msg.value);
                break;
            case StateUpdateType::Acceleration:
                state_.acceleration.store(msg.value);
                break;
            case StateUpdateType::Throttle:
                state_.throttle.store(msg.value);
                break;
            case StateUpdateType::Brake:
                state_.brake.store(msg.value);
                break;
        }
    }

    // 执行二次数据处理（例如单位转换、滤波等）
    void perform_secondary_processing() {
        // 这是一个示例，你可以添加任何你需要的数据处理逻辑
        // 例如：将速度从 m/s 转换为 knots
        // double velocity_mps = state_.velocity.load();
        // double velocity_knots = velocity_mps * 1.94384; 
        // state_.velocity_knots.store(velocity_knots); // 假设 SharedStateSpace 中有 velocity_knots
    }

public:
    StateManagerThread(SharedStateSpace& state, StateUpdateQueue& queue, SimulationClock& clock)
        : state_(state), queue_(queue), clock_(clock) {}

    ~StateManagerThread() {
        stop();
    }

    void start() {
        if (!running_.load()) {
            running_ = true;
            thread_ = std::thread(&StateManagerThread::run, this);
        }
    }

    void stop() {
        if (running_.load()) {
            running_ = false;
            if (thread_.joinable()) {
                thread_.join();
            }
        }
    }
}; 