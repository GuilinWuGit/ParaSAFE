/*
 * @file aircraft_config.hpp
 * @brief 飞机构型参数接口类
 *
 * 该接口定义了仿真系统中所有飞机构型应实现的参数获取方法，
 * 支持多机型热插拔。
 */

#ifndef AIRCRAFT_CONFIG_INTERFACE_H
#define AIRCRAFT_CONFIG_INTERFACE_H

#pragma once

class AircraftConfigBase {
public:
    virtual ~AircraftConfigBase() = default;
    // 获取飞机质量（kg）
    virtual double getMass() const = 0;
    // 获取最大推力（N）
    virtual double getMaxThrust() const = 0;
    // 获取最小推力（N）
    virtual double getMinThrust() const = 0;
    // 获取最大刹车力（N）
    virtual double getMaxBrakeForce() const = 0;
    // 获取阻力系数
    virtual double getDragCoefficient() const = 0;
    // 获取静摩擦系数
    virtual double getStaticFrictionCoefficient() const = 0;
    // 可扩展更多参数...
};

#endif // AIRCRAFT_CONFIG_INTERFACE_H 