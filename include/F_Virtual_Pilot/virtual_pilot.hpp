#pragma once
#include <memory>
#include "../K_Scenario/shared_state.hpp"
#include "../K_Scenario/event_bus.hpp"
#include <atomic>
#include <string>

/**
 * @brief 虚拟飞行员模型接口
 */
class IVirtualPilot {
public:
    virtual ~IVirtualPilot() = default;
    // 根据当前状态和目标，输出油门和刹车指令
    virtual void update(SharedStateSpace& state) = 0;
};

/**
 * @brief 简单自动驾驶飞行员（巡航/刹车自动控制）
 * 目标速度以下加油门，以上刹车
 */
class SimpleAutoPilot : public IVirtualPilot {
public:
    SimpleAutoPilot(double target_speed) : target_speed_(target_speed) {}
    void update(SharedStateSpace& state) override {
        double v = state.velocity.load();
        if (v < target_speed_ - 1.0) {
            state.throttle.store(1.0); // 全油门
            state.brake.store(0.0);
        } else if (v > target_speed_ + 1.0) {
            state.throttle.store(0.0);
            state.brake.store(1.0); // 全刹车
        } else {
            state.throttle.store(0.2); // 保持巡航
            state.brake.store(0.0);
        }
    }
private:
    double target_speed_;
};

/**
 * @brief 复杂虚拟飞行员，支持多阶段决策、事件响应、手动/自动切换
 */
class ComplexVirtualPilot : public IVirtualPilot {
public:
    enum class Mode { MANUAL, AUTO };
    enum class Phase { INIT, ACCELERATE, CRUISE, BRAKE, STOP };

    ComplexVirtualPilot(double target_speed)
        : mode_(Mode::AUTO), phase_(Phase::INIT), target_speed_(target_speed), manual_throttle_(0.0), manual_brake_(0.0) {}

    // 切换手动/自动模式
    void setMode(Mode mode) { mode_ = mode; }
    Mode getMode() const { return mode_; }
    // 手动输入
    void setManualInput(double throttle, double brake) {
        manual_throttle_ = throttle;
        manual_brake_ = brake;
    }
    // 事件响应接口
    void onEvent(const std::string& event_name) {
        if (event_name == "AbortTakeoff") {
            phase_ = Phase::BRAKE;
        } else if (event_name == "StartCruise") {
            phase_ = Phase::CRUISE;
        } else if (event_name == "Stop") {
            phase_ = Phase::STOP;
        }
    }
    // 多阶段决策主逻辑
    void update(SharedStateSpace& state) override {
        if (mode_ == Mode::MANUAL) {
            state.throttle.store(manual_throttle_);
            state.brake.store(manual_brake_);
            return;
        }
        double v = state.velocity.load();
        switch (phase_) {
        case Phase::INIT:
            phase_ = Phase::ACCELERATE;
            break;
        case Phase::ACCELERATE:
            if (v < target_speed_ - 2.0) {
                state.throttle.store(1.0);
                state.brake.store(0.0);
            } else {
                phase_ = Phase::CRUISE;
            }
            break;
        case Phase::CRUISE:
            if (v > target_speed_ + 2.0) {
                phase_ = Phase::BRAKE;
            } else {
                state.throttle.store(0.3);
                state.brake.store(0.0);
            }
            break;
        case Phase::BRAKE:
            if (v > 2.0) {
                state.throttle.store(0.0);
                state.brake.store(1.0);
            } else {
                phase_ = Phase::STOP;
            }
            break;
        case Phase::STOP:
            state.throttle.store(0.0);
            state.brake.store(0.0);
            break;
        }
    }
private:
    Mode mode_;
    Phase phase_;
    double target_speed_;
    double manual_throttle_;
    double manual_brake_;
}; 