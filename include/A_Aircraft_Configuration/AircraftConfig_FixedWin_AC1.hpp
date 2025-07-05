/*
 * @file AircraftConfig_FixedWin_AC1.hpp
 * @brief 固定翼飞机AC1机型参数实现
 */

#ifndef AIRCRAFT_CONFIG_FIXEDWIN_AC1_H
#define AIRCRAFT_CONFIG_FIXEDWIN_AC1_H

#include "aircraft_config.hpp"

class AircraftConfig_FixedWin_AC1 : public AircraftConfigBase {
public:
    double getMass() const override { return 80000.0; }
    double getMaxThrust() const override { return 500000.0; }
    double getMinThrust() const override { return 0.0; }
    double getMaxBrakeForce() const override { return 400000.0; }
    double getDragCoefficient() const override { return 0.02; }
    double getStaticFrictionCoefficient() const override { return 0.02; }
};

#endif // AIRCRAFT_CONFIG_FIXEDWIN_AC1_H 