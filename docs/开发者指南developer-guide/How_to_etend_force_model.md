# 如何扩展 ParaSAFE 力学模型（Force Model）

本指南详细介绍如何为 ParaSAFE 仿真系统扩展新的力学模型，实现动力学算法的热插拔与灵活切换。

---

## 1. 设计动力学模型接口

首先，定义一个通用的力学模型接口（如 `IForceModel`），所有力学模型都应继承并实现该接口。

```cpp
// include/B_Aircraft_Forces_Model/ACForceModel.hpp
class IForceModel {
public:
    virtual ~IForceModel() = default;
    virtual ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) = 0;
};
```

---

## 2. 实现具体力学模型

### 2.1 线性力学模型（示例）

```cpp
class ACForceModel : public IForceModel {
public:
    ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) override {
        // ... 线性模型实现 ...
    }
};
```

### 2.2 新建自定义模型（如非线性模型）

```cpp
class ACForceModel_Nonlinear : public IForceModel {
public:
    ForceResult calculateNetForce(const SharedStateSpace& state, double current_velocity, std::shared_ptr<AircraftConfigBase> aircraftConfig) override {
        // ... 非线性模型实现 ...
    }
};
```

---

## 3. 在主流程中支持模型热插拔

在场景主程序（如 `main_Taxi.cpp`）中，通过智能指针选择和传递不同的力学模型：

```cpp
std::shared_ptr<IForceModel> forceModel = std::make_shared<ACForceModel>();
// 切换为非线性模型：
// std::shared_ptr<IForceModel> forceModel = std::make_shared<ACForceModel_Nonlinear>();
```

---

## 4. 修改动力学主循环，支持接口调用

在 `dynamicsModel` 等动力学主循环中，统一通过接口调用：

```cpp
void dynamicsModel(..., std::shared_ptr<AircraftConfigBase> aircraftConfig, std::shared_ptr<IForceModel> forceModel) {
    ForceResult forces = forceModel->calculateNetForce(state, state.velocity.load(), aircraftConfig);
    // ... 其余仿真逻辑 ...
}
```

---

## 5. 测试与验证

- 编译并运行主程序，切换不同力学模型，观察仿真结果变化。
- 可通过命令行参数、配置文件等方式灵活选择模型。
- 建议为每个新模型编写单元测试或对比实验。

---

## 6. 进阶：支持第三方模型注册/插件化

- 可设计模型工厂或注册表，支持动态加载和第三方扩展。
- 便于社区贡献更多动力学算法。

---

## 7. 常见问题与建议

- **接口设计要简洁，参数尽量通用，便于不同模型复用。**
- **建议将所有力学模型实现集中在 `B_Aircraft_Forces_Model/` 目录下，便于统一管理。**
- **文档和注释要详细，方便他人理解和二次开发。**

---

## 8. 参考代码片段

```cpp
// 选择模型
std::shared_ptr<IForceModel> forceModel = std::make_shared<ACForceModel_Nonlinear>();
// 主循环调用
ForceResult forces = forceModel->calculateNetForce(state, v, aircraftConfig);
```

---

如有疑问或建议，欢迎在社区或Issue区反馈！ 