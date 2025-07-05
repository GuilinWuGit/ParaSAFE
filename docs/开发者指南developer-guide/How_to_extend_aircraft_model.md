# 如何扩展 ParaSAFE 飞行器（机型）模型

本指南详细介绍如何为 ParaSAFE 仿真系统扩展新的飞行器（机型）参数模型，实现机型参数的热插拔与灵活切换。

---

## 1. 设计飞行器参数接口

首先，定义一个通用的飞行器参数接口（如 `AircraftConfigBase`），所有机型参数都应继承并实现该接口。

```cpp
// include/A_Aircraft_Configuration/aircraft_config.hpp
class AircraftConfigBase {
public:
    virtual ~AircraftConfigBase() = default;
    virtual double getMass() const = 0;
    virtual double getMaxThrust() const = 0;
    virtual double getMinThrust() const = 0;
    virtual double getMaxBrakeForce() const = 0;
    virtual double getDragCoefficient() const = 0;
    virtual double getStaticFrictionCoefficient() const = 0;
    // 可扩展更多参数...
};
```

---

## 2. 实现具体机型参数类

### 2.1 示例：固定翼AC1机型

```cpp
class AircraftConfig_FixedWin_AC1 : public AircraftConfigBase {
public:
    double getMass() const override { return 80000.0; }
    double getMaxThrust() const override { return 500000.0; }
    double getMinThrust() const override { return 0.0; }
    double getMaxBrakeForce() const override { return 400000.0; }
    double getDragCoefficient() const override { return 0.02; }
    double getStaticFrictionCoefficient() const override { return 0.02; }
};
```

### 2.2 新建自定义机型

```cpp
class AircraftConfig_FixedWin_AC2 : public AircraftConfigBase {
public:
    double getMass() const override { return 85000.0; }
    double getMaxThrust() const override { return 520000.0; }
    // ... 其余参数 ...
};
```

---

## 3. 在主流程中支持机型热插拔

在场景主程序（如 `main_Taxi.cpp`）中，通过智能指针选择和传递不同的机型参数：

```cpp
std::shared_ptr<AircraftConfigBase> aircraftConfig = std::make_shared<AircraftConfig_FixedWin_AC1>();
// 切换为AC2机型：
// std::shared_ptr<AircraftConfigBase> aircraftConfig = std::make_shared<AircraftConfig_FixedWin_AC2>();
```

---

## 4. 修改各模块支持机型接口

- 在 `dynamicsModel`、`ACForceModel`、`initializeMotionState` 等所有依赖机型参数的模块中，统一通过 `aircraftConfig` 接口获取参数。
- 不再使用全局常量或命名空间常量。

```cpp
dynamicsModel(..., std::shared_ptr<AircraftConfigBase> aircraftConfig, ...);
ForceResult forces = forceModel->calculateNetForce(state, v, aircraftConfig);
TaxiInitialState::initializeMotionState(state, aircraftConfig);
```

---

## 5. 测试与验证

- 编译并运行主程序，切换不同机型，观察仿真结果变化。
- 可通过命令行参数、配置文件等方式灵活选择机型。
- 建议为每个新机型编写单元测试或对比实验。

---

## 6. 进阶：支持配置文件/命令行选择机型

- 在配置文件中增加机型选择项，或通过命令行参数指定机型。
- 程序启动时根据配置实例化对应机型参数类。

---

## 7. 常见问题与建议

- **接口设计要简洁，参数尽量通用，便于不同机型复用。**
- **建议将所有机型参数实现集中在 `A_Aircraft_Configuration/` 目录下，便于统一管理。**
- **文档和注释要详细，方便他人理解和二次开发。**

---

## 8. 参考代码片段

```cpp
// 选择机型
std::shared_ptr<AircraftConfigBase> aircraftConfig = std::make_shared<AircraftConfig_FixedWin_AC2>();
// 主循环调用
ForceResult forces = forceModel->calculateNetForce(state, v, aircraftConfig);
```

---

如有疑问或建议，欢迎在社区或Issue区反馈！ 