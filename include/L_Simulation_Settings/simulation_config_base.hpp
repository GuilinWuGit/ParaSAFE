#pragma once
#include <chrono>

namespace SimulationConfig {
    // 基础时间参数
    const double TIME_STEP = 0.01;
    const double SIMULATION_TIME = 60.0;
    const std::chrono::milliseconds DT(10);
    const float SIMULATION_STEP_S = 0.01f;
    const float REAL_TIME_FACTOR = 1.0f;
} 