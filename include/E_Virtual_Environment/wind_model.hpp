#pragma once
#include <memory>
#include <cmath>

/**
 * @brief 风模型接口
 */
class IWindModel {
public:
    virtual ~IWindModel() = default;
    // 查询指定高度/位置的风速（m/s）
    virtual double getWindSpeed(double altitude_m, double x = 0, double y = 0) const = 0;
    // 查询指定高度/位置的风向（弧度，0为正x轴，逆时针）
    virtual double getWindDirection(double altitude_m, double x = 0, double y = 0) const = 0;
};

/**
 * @brief 恒定风模型（所有高度、位置风速风向恒定）
 */
class ConstantWindModel : public IWindModel {
public:
    ConstantWindModel(double speed = 0.0, double direction_rad = 0.0)
        : wind_speed(speed), wind_direction(direction_rad) {}
    double getWindSpeed(double altitude_m, double x = 0, double y = 0) const override {
        return wind_speed;
    }
    double getWindDirection(double altitude_m, double x = 0, double y = 0) const override {
        return wind_direction;
    }
private:
    double wind_speed;      // m/s
    double wind_direction;  // 弧度
}; 