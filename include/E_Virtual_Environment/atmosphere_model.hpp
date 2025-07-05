#pragma once
#include <memory>
#include <cmath>

/**
 * @brief 大气模型接口
 */
class IAtmosphereModel {
public:
    virtual ~IAtmosphereModel() = default;
    // 查询指定高度下的温度（K）
    virtual double getTemperature(double altitude_m) const = 0;
    // 查询指定高度下的气压（Pa）
    virtual double getPressure(double altitude_m) const = 0;
    // 查询指定高度下的密度（kg/m^3）
    virtual double getDensity(double altitude_m) const = 0;
};

/**
 * @brief 国际标准大气模型（ISA）
 * 只实现对流层（0-11km）
 */
class ISAAtmosphereModel : public IAtmosphereModel {
public:
    double getTemperature(double altitude_m) const override {
        // 对流层线性递减
        return 288.15 - 0.0065 * altitude_m;
    }
    double getPressure(double altitude_m) const override {
        // 对流层气压公式
        double T0 = 288.15;
        double P0 = 101325.0;
        double L = 0.0065;
        double R = 287.05;
        double g = 9.80665;
        double T = getTemperature(altitude_m);
        return P0 * std::pow(1 - L * altitude_m / T0, g / (R * L));
    }
    double getDensity(double altitude_m) const override {
        double P = getPressure(altitude_m);
        double T = getTemperature(altitude_m);
        double R = 287.05;
        return P / (R * T);
    }
}; 