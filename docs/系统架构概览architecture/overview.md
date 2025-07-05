# 系统架构概览

## 架构设计理念

VFT系统采用**分层架构**和**模块化设计**，确保系统的可维护性、可扩展性和高性能。整个系统基于**事件驱动**和**多线程**的设计模式，实现实时飞行仿真。

## 整体架构图

```
┌─────────────────────────────────────────────────────────────┐
│                       应用层 (Application Layer)              │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │  场景管理   │  │  主程序     │  │  用户界面   │          │
│  │  Scenario   │  │  Main       │  │  UI         │          │
│  └─────────────┘  └─────────────┘  └─────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                   业务逻辑层 (Business Logic Layer)           │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │ 飞行控制    │  │ 虚拟传感器  │  │ 虚拟飞行员  │          │
│  │ Flight Ctrl │  │ Virtual     │  │ Virtual     │          │
│  │             │  │ Sensor      │  │ Pilot       │          │
│  └─────────────┘  └─────────────┘  └─────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                      模型层 (Model Layer)                    │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │ 飞机配置    │  │ 气动力模型  │  │ 动力学模型  │          │
│  │ Aircraft    │  │ Aerodynamic │  │ Dynamic     │          │
│  │ Config      │  │ Model       │  │ Model       │          │
│  └─────────────┘  └─────────────┘  └─────────────┘          │
├─────────────────────────────────────────────────────────────┤
│                  基础设施层 (Infrastructure Layer)            │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐          │
│  │ 仿真设置    │  │ 事件总线    │  │ 数据记录    │          │
│  │ Simulation  │  │ Event Bus   │  │ Data Logger │          │
│  │ Settings    │  │             │  │             │          │
│  └─────────────┘  └─────────────┘  └─────────────┘          │
└─────────────────────────────────────────────────────────────┘
```

## 分层架构详解

### 1. 应用层 (Application Layer)

**职责**: 提供用户接口和场景管理
- **场景管理**: 管理不同的飞行仿真场景
- **主程序**: 系统入口点和初始化
- **用户界面**: 提供用户交互接口

**关键组件**:
```cpp
// 场景管理器
class ScenarioManager {
    void loadScenario(const std::string& scenarioName);
    void startSimulation();
    void stopSimulation();
};

// 主程序
int main() {
    ScenarioManager manager;
    manager.loadScenario("abort_takeoff");
    manager.startSimulation();
    return 0;
}
```

### 2. 业务逻辑层 (Business Logic Layer)

**职责**: 实现核心业务逻辑和控制系统
- **飞行控制**: 各种飞行控制器
- **虚拟传感器**: 模拟飞机传感器
- **虚拟飞行员**: 模拟飞行员行为

**关键组件**:
```cpp
// 控制器基类
class BaseController {
    virtual void update() = 0;
    virtual bool hasAuthority() = 0;
};

// 油门控制器
class ThrottleController : public BaseController {
    void update() override;
    bool hasAuthority() override;
};
```

### 3. 模型层 (Model Layer)

**职责**: 提供物理模型和数学计算
- **飞机配置**: 飞机参数和特性
- **气动力模型**: 空气动力学计算
- **动力学模型**: 飞行动力学方程

**关键组件**:
```cpp
// 飞机配置
struct AircraftConfig {
    double mass;           // 质量 (kg)
    double wingArea;       // 机翼面积 (m²)
    double maxThrust;      // 最大推力 (N)
    double dragCoeff;      // 阻力系数
};

// 动力学模型
class DynamicsModel {
    void updateState(double dt);
    void calculateForces();
};
```

### 4. 基础设施层 (Infrastructure Layer)

**职责**: 提供系统基础服务
- **仿真设置**: 仿真参数和配置
- **事件总线**: 模块间通信
- **数据记录**: 仿真数据输出

**关键组件**:
```cpp
// 事件总线
class EventBus {
    void publish(const Event& event);
    void subscribe(const std::string& eventType, EventHandler handler);
};

// 数据记录器
class DataRecorder {
    void recordState(const SimulationState& state);
    void saveToFile(const std::string& filename);
};
```

## 核心设计模式

### 1. 观察者模式 (Observer Pattern)

**应用**: 事件总线系统
```cpp
// 事件发布者
class EventPublisher {
    EventBus& eventBus;
public:
    void publishEvent(const Event& event) {
        eventBus.publish(event);
    }
};

// 事件订阅者
class EventSubscriber {
    EventBus& eventBus;
public:
    void subscribe(const std::string& eventType) {
        eventBus.subscribe(eventType, [this](const Event& event) {
            this->handleEvent(event);
        });
    }
};
```

### 2. 策略模式 (Strategy Pattern)

**应用**: 控制器管理
```cpp
// 控制器策略
class ControllerStrategy {
    virtual void execute() = 0;
};

// 具体策略
class ThrottleStrategy : public ControllerStrategy {
    void execute() override {
        // 油门控制逻辑
    }
};
```

### 3. 工厂模式 (Factory Pattern)

**应用**: 控制器创建
```cpp
// 控制器工厂
class ControllerFactory {
public:
    static std::unique_ptr<BaseController> createController(
        const std::string& controllerType);
};

// 使用工厂
auto controller = ControllerFactory::createController("throttle");
```

### 4. 单例模式 (Singleton Pattern)

**应用**: 共享状态管理
```cpp
// 共享状态单例
class SharedState {
private:
    static SharedState* instance;
    SharedState() = default;
    
public:
    static SharedState* getInstance() {
        if (instance == nullptr) {
            instance = new SharedState();
        }
        return instance;
    }
};
```

## 多线程架构

### 线程设计

```
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  主线程         │  │  状态更新线程   │  │  控制器线程     │
│  Main Thread    │  │  State Thread   │  │  Controller     │
│                 │  │                 │  │  Thread         │
│ • 初始化        │  │ • 物理计算      │  │ • 控制逻辑      │
│ • 事件处理      │  │ • 状态更新      │  │ • 控制输出      │
│ • 用户交互      │  │ • 数据记录      │  │ • 模式切换      │
└─────────────────┘  └─────────────────┘  └─────────────────┘
         │                      │                      │
         └──────────────────────┼──────────────────────┘
                                │
                    ┌─────────────────┐
                    │  事件总线       │
                    │  Event Bus      │
                    │                 │
                    │ • 事件分发      │
                    │ • 线程同步      │
                    │ • 消息队列      │
                    └─────────────────┘
```

### 线程同步

```cpp
// 互斥锁保护共享资源
class ThreadSafeState {
private:
    mutable std::mutex mutex_;
    SimulationState state_;
    
public:
    void updateState(const SimulationState& newState) {
        std::lock_guard<std::mutex> lock(mutex_);
        state_ = newState;
    }
    
    SimulationState getState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }
};
```

## 数据流架构

### 数据流向图

```
传感器数据 → 状态更新 → 事件检测 → 控制器响应 → 控制输出
     ↓           ↓         ↓         ↓         ↓
  数据记录 ← 状态管理 ← 事件总线 ← 控制器 ← 物理模型
```

### 关键数据流

1. **传感器数据流**
   ```
   虚拟传感器 → 状态管理器 → 事件检测器
   ```

2. **控制数据流**
   ```
   事件总线 → 控制器管理器 → 具体控制器 → 物理模型
   ```

3. **状态数据流**
   ```
   物理模型 → 状态管理器 → 数据记录器 → 输出文件
   ```

## 扩展性设计

### 插件化架构

```cpp
// 插件接口
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void initialize() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
};

// 插件管理器
class PluginManager {
private:
    std::vector<std::unique_ptr<IPlugin>> plugins_;
    
public:
    void registerPlugin(std::unique_ptr<IPlugin> plugin);
    void updateAll();
};
```

### 配置驱动

```cpp
// 配置管理器
class ConfigManager {
private:
    std::map<std::string, std::string> config_;
    
public:
    void loadConfig(const std::string& filename);
    template<typename T>
    T getValue(const std::string& key, T defaultValue);
};
```

## 性能优化

### 内存管理

- **智能指针**: 自动内存管理
- **对象池**: 减少内存分配开销
- **缓存友好**: 优化数据结构布局

### 计算优化

- **向量化**: 利用SIMD指令
- **并行计算**: 多线程并行处理
- **算法优化**: 高效算法选择

### 实时性保证

- **优先级调度**: 关键任务优先
- **时间片管理**: 合理分配CPU时间
- **死锁预防**: 避免线程死锁

## 安全设计

### 控制安全

- **权限管理**: 控制器权限控制
- **模式保护**: 防止意外模式切换
- **异常处理**: 完善的错误处理

### 数据安全

- **数据校验**: 输入数据验证
- **备份机制**: 重要数据备份
- **错误恢复**: 系统错误恢复

---

*这个架构设计确保了VFT系统的高性能、高可靠性和高可扩展性* 