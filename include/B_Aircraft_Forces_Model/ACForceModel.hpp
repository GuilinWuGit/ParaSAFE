#pragma once

// C++头文件
#include <atomic>
#include <cmath>
#include <memory>

// ParaSAFE头文件
#include "../K_Scenario/shared_state.hpp"
#include "../A_Aircraft_Configuration/aircraft_config.hpp"

// 使用配置文件中的参数
// using namespace SimulationConfig; // 如有需要，可按需开启

// 计算合外力
struct ForceResult {
    double net_force;      // 合外力
    double thrust;         // 推力
    double drag;           // 阻力
    double brake_force;    // 刹车力
    double static_friction;// 静摩擦力
};

// 力学模型接口
class IForceModel {
public:
    virtual ~IForceModel() = default;
    virtual ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) = 0;
};

// 线性力学模型实现
class ACForceModel : public IForceModel {
public:
    ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) override {
        ForceResult result;
        
        // 计算推力
        result.thrust = state.throttle.load() * aircraftConfig->getMaxThrust();
        
        // 计算阻力
        const double AIR_DENSITY = 1.225;  // 空气密度 (kg/m^3)
        const double FRONTAL_AREA = 50.0;  // 迎风面积 (m^2)
        double drag_coeff = aircraftConfig->getDragCoefficient();
        result.drag = 0.5 * AIR_DENSITY * FRONTAL_AREA * drag_coeff * current_velocity * current_velocity;
        
        // 计算刹车力和静摩擦力处理
        if (std::abs(current_velocity) < 0.01) {
            // 静止状态：无刹车力，需要考虑静摩擦力
            result.brake_force = 0.0;
            double static_friction_coeff = aircraftConfig->getStaticFrictionCoefficient();
            double normal_force = aircraftConfig->getMass() * 9.81;
            result.static_friction = static_friction_coeff * normal_force;
        } else {
            // 运动状态：有刹车力，无静摩擦力
            // 速度因子：低速时接近1.0，高速时也接近1.0，避免过度变化
            double speed_factor = std::min(1.0, std::max(0.3, std::abs(current_velocity) / 50.0)); // 速度因子范围0.3-1.0
            result.brake_force = state.brake.load() * aircraftConfig->getMaxBrakeForce() * speed_factor;
            result.static_friction = 0.0; // 运动时无静摩擦力
        }
        
        // 计算合外力
        result.net_force = result.thrust - result.drag - result.brake_force;
        
        // 静止状态下的静摩擦力处理
        if (std::abs(current_velocity) < 0.01) {
            if (std::abs(result.net_force) < result.static_friction) {
                result.net_force = 0; // 力小于静摩擦力，保持静止
            } else {
                result.net_force -= result.static_friction * (result.net_force > 0 ? 1 : -1);
            }
        }
        
        return result;
    }
};

// 非线性力学模型实现（示例）
class ACForceModel_Nonlinear : public IForceModel {
public:
    ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) override {
        ForceResult result;
        // 非线性推力：加入速度相关扰动
        result.thrust = state.throttle.load() * aircraftConfig->getMaxThrust() * (1.0 - 0.1 * std::sin(current_velocity / 10.0));
        // 非线性阻力：阻力系数随速度变化
        const double AIR_DENSITY = 1.225;
        const double FRONTAL_AREA = 50.0;
        double drag_coeff = aircraftConfig->getDragCoefficient() * (1.0 + 0.05 * std::abs(current_velocity) / 100.0);
        result.drag = 0.5 * AIR_DENSITY * FRONTAL_AREA * drag_coeff * current_velocity * current_velocity;
        // 非线性刹车力：刹车效率随速度降低
        if (std::abs(current_velocity) < 0.01) {
            result.brake_force = 0.0;
            double static_friction_coeff = aircraftConfig->getStaticFrictionCoefficient();
            double normal_force = aircraftConfig->getMass() * 9.81;
            result.static_friction = static_friction_coeff * normal_force;
        } else {
            double speed_factor = std::min(1.0, std::max(0.2, std::abs(current_velocity) / 60.0));
            // 非线性：刹车力随速度降低而减弱
            result.brake_force = state.brake.load() * aircraftConfig->getMaxBrakeForce() * speed_factor * (1.0 - 0.1 * std::cos(current_velocity / 15.0));
            result.static_friction = 0.0;
        }
        result.net_force = result.thrust - result.drag - result.brake_force;
        if (std::abs(current_velocity) < 0.01) {
            if (std::abs(result.net_force) < result.static_friction) {
                result.net_force = 0;
            } else {
                result.net_force -= result.static_friction * (result.net_force > 0 ? 1 : -1);
            }
        }
        return result;
    }
}; 