# 俯仰角保持控制器使用示例

## 概述

`PitchHoldController` 是 ParaSAFE 仿真系统中的俯仰角保持控制器，用于精确控制飞机的俯仰角。该控制器采用PID控制算法，支持实时调整目标俯仰角和PID参数。

## 主要特性

- **PID控制算法**：比例-积分-微分控制，提供精确的俯仰角控制
- **实时参数调整**：支持运行时调整PID参数和目标俯仰角
- **线程安全**：多线程环境下的安全操作
- **状态监控**：实时状态监控和日志记录
- **集成事件系统**：与ParaSAFE事件系统完全集成

## 基本使用

### 1. 创建控制器

```cpp
#include "../include/B_Flight_Control/PitchHold_controller.hpp"
#include "../include/K_Scenario/shared_state.hpp"
#include "../include/K_Scenario/event_bus.hpp"

// 创建共享状态空间和事件总线
SharedStateSpace state;
EventBus bus(state);

// 创建俯仰角保持控制器
PitchHoldController pitch_controller(state, bus);
```

### 2. 配置PID参数

```cpp
// 设置PID参数
pitch_controller.setPIDParameters(1.0, 0.1, 0.05);  // Kp=1.0, Ki=0.1, Kd=0.05

// 设置目标俯仰角（弧度）
pitch_controller.setTargetPitch(M_PI / 6.0);  // 30度
```

### 3. 启动控制器

```cpp
// 启用俯仰角控制
state.pitch_control_enabled.store(true);

// 启动控制器
pitch_controller.start();
```

### 4. 运行时调整

```cpp
// 动态调整目标俯仰角
pitch_controller.setTargetPitch(M_PI / 4.0);  // 45度

// 动态调整PID参数
pitch_controller.setPIDParameters(1.5, 0.2, 0.08);

// 重置积分项（防止积分饱和）
pitch_controller.resetIntegral();
```

### 5. 停止控制器

```cpp
// 停止控制器
pitch_controller.stop();

// 禁用俯仰角控制
state.pitch_control_enabled.store(false);
```

## 事件系统集成

### 事件定义

```cpp
// 在事件定义表中添加俯仰角控制事件
const std::unordered_map<std::string, EventDefinition> EVENT_DEFINITIONS = {
    {"START_PITCH_CONTROL", {
        "START_PITCH_CONTROL",
        "启动俯仰角控制事件",
        [](const SharedStateSpace& state) {
            return state.simulation_started.load() && 
                   state.simulation_running.load() &&
                   state.simulation_time.load() >= 5.0;  // 5秒后启动
        },
        {GenericEvents::ControllerAction::START_PITCH_CONTROL},
        "启动俯仰角保持控制器",
        false
    }},
    {"SET_CLIMB_PITCH", {
        "SET_CLIMB_PITCH",
        "设置爬升俯仰角事件",
        [](const SharedStateSpace& state) {
            return state.pitch_control_enabled.load() &&
                   state.velocity.load() >= 50.0;  // 速度达到50m/s
        },
        {GenericEvents::ControllerAction::SET_PITCH_ANGLE},
        "设置爬升俯仰角为15度",
        false
    }}
};
```

### 控制器动作配置

```txt
# 在控制器动作配置文件中添加
START_PITCH_CONTROL = 俯仰角保持, pitch_control_enabled=true
STOP_PITCH_CONTROL = 俯仰角保持, pitch_control_enabled=false
SET_PITCH_ANGLE = 俯仰角保持, target_pitch=0.261799  # 15度（弧度）
```

## 状态监控

### 获取当前状态

```cpp
// 获取当前俯仰角
double current_pitch = state.pitch_angle.load();

// 获取目标俯仰角
double target_pitch = pitch_controller.getTargetPitch();

// 获取控制输出
double control_output = pitch_controller.getCurrentValue();

// 检查控制器状态
bool is_enabled = pitch_controller.isEnabled();
std::string controller_name = pitch_controller.getName();
```

### 状态快照

```cpp
// 获取完整状态快照
StateSnapshot snapshot = state.getState();

// 访问俯仰角相关状态
double pitch_angle = snapshot.pitch_angle;
double pitch_rate = snapshot.pitch_rate;
double pitch_control_output = snapshot.pitch_control_output;
```

## 参数调优建议

### PID参数选择

1. **比例系数 (Kp)**
   - 增大Kp：响应更快，但可能产生超调
   - 减小Kp：响应较慢，但更稳定
   - 建议范围：0.5 - 2.0

2. **积分系数 (Ki)**
   - 消除稳态误差
   - 过大会导致振荡
   - 建议范围：0.05 - 0.3

3. **微分系数 (Kd)**
   - 抑制超调，提高稳定性
   - 过大会放大噪声
   - 建议范围：0.01 - 0.1

### 调优步骤

```cpp
// 1. 首先只使用P控制
pitch_controller.setPIDParameters(1.0, 0.0, 0.0);

// 2. 调整Kp直到获得合适的响应速度
pitch_controller.setPIDParameters(1.5, 0.0, 0.0);

// 3. 添加积分项消除稳态误差
pitch_controller.setPIDParameters(1.5, 0.1, 0.0);

// 4. 添加微分项抑制超调
pitch_controller.setPIDParameters(1.5, 0.1, 0.05);
```

## 注意事项

1. **角度单位**：所有角度输入输出均为弧度
2. **积分饱和**：长时间运行时应定期调用 `resetIntegral()`
3. **控制限制**：控制器输出限制在 [-1.0, 1.0] 范围内
4. **线程安全**：所有状态访问都是线程安全的
5. **实时性**：控制器运行在独立线程中，确保实时性

## 完整示例

```cpp
#include "../include/B_Flight_Control/PitchHold_controller.hpp"
#include "../include/K_Scenario/shared_state.hpp"
#include "../include/K_Scenario/event_bus.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    // 初始化
    SharedStateSpace state;
    EventBus bus(state);
    PitchHoldController pitch_controller(state, bus);
    
    // 配置控制器
    pitch_controller.setPIDParameters(1.0, 0.1, 0.05);
    pitch_controller.setTargetPitch(M_PI / 6.0);  // 30度
    
    // 启动仿真
    state.simulation_started.store(true);
    state.simulation_running.store(true);
    state.pitch_control_enabled.store(true);
    
    // 启动控制器
    pitch_controller.start();
    
    // 运行一段时间
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 调整目标俯仰角
    pitch_controller.setTargetPitch(M_PI / 4.0);  // 45度
    
    // 继续运行
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 停止
    pitch_controller.stop();
    state.simulation_running.store(false);
    
    return 0;
}
```

这个示例展示了如何创建、配置和使用俯仰角保持控制器，包括基本操作、事件系统集成、状态监控和参数调优等内容。 