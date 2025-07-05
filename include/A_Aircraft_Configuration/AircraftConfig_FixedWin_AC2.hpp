/*
 * @file AircraftConfig_FixedWin_AC2.hpp
 * @brief 固定翼飞机AC2机型参数实现
 */

#ifndef AIRCRAFT_CONFIG_FIXEDWIN_AC2_H
#define AIRCRAFT_CONFIG_FIXEDWIN_AC2_H

#include "aircraft_config.hpp"

class AircraftConfig_FixedWin_AC2 : public AircraftConfigBase {
public:
    double getMass() const override { return 85000.0; } // 示例：不同参数
    double getMaxThrust() const override { return 520000.0; }
    double getMinThrust() const override { return 0.0; }
    double getMaxBrakeForce() const override { return 420000.0; }
    double getDragCoefficient() const override { return 0.021; }
    double getStaticFrictionCoefficient() const override { return 0.021; }
};

#endif // AIRCRAFT_CONFIG_FIXEDWIN_AC2_H 