# 中断起飞场景详解

## 场景概述

中断起飞 (Abort Takeoff) 是飞行训练中的关键场景，模拟飞机在起飞过程中因各种原因需要中断起飞的情况。这个场景对于飞行员训练和系统验证具有重要意义。

## 场景背景

### 真实情况
在真实的飞行中，中断起飞可能由以下原因触发：
- 发动机故障
- 系统警告
- 跑道障碍物
- 天气条件恶化
- 飞行员判断

### 训练目标
- 提高飞行员应急反应能力
- 验证飞行控制系统的可靠性
- 测试刹车系统的有效性
- 评估决策时机的重要性

## 场景配置

### 配置文件结构

```ini
# abort_takeoff_config.txt
# 中断起飞场景配置

# 基本参数
ABORT_SPEED = 30.0          # 中断起飞触发速度 (m/s)
BRAKE_FORCE = 0.8           # 刹车力度系数
THRUST_REDUCTION_TIME = 2.0 # 推力减小时间 (s)
SIMULATION_TIME_STEP = 0.01 # 仿真时间步长 (s)

# 飞机参数
AIRCRAFT_MASS = 50000.0     # 飞机质量 (kg)
MAX_THRUST = 120000.0       # 最大推力 (N)
DRAG_COEFFICIENT = 0.02     # 阻力系数

# 环境参数
RUNWAY_LENGTH = 3000.0      # 跑道长度 (m)
AIR_DENSITY = 1.225         # 空气密度 (kg/m³)
GRAVITY = 9.81              # 重力加速度 (m/s²)
```

### 控制器动作配置

```ini
# controller_actions_config.txt
# 控制器动作配置

# 油门控制器动作
throttle_controller = start
throttle_controller = set_thrust_reduction_time:2.0

# 刹车控制器动作
brake_controller = start
brake_controller = set_brake_force:0.8

# 跑道巡航控制器动作
cruise_controller = stop
```

## 场景流程

### 1. 初始化阶段

```cpp
// 初始状态设置
struct InitialState {
    double position = 0.0;      // 起始位置 (m)
    double velocity = 0.0;      // 初始速度 (m/s)
    double thrust = 0.0;        // 初始推力 (N)
    double brakeForce = 0.0;    // 初始刹车力 (N)
    FlightMode mode = FlightMode::MANUAL; // 初始模式
};
```

### 2. 起飞阶段

```cpp
// 起飞过程
void takeoffPhase() {
    // 1. 增加推力
    throttleController.setThrust(maxThrust);
    
    // 2. 飞机加速
    while (velocity < abortSpeed) {
        updatePhysics();
        recordData();
        
        // 检查是否需要中断起飞
        if (shouldAbortTakeoff()) {
            triggerAbortTakeoff();
            break;
        }
    }
}
```

### 3. 中断起飞触发

```cpp
// 中断起飞检测
bool shouldAbortTakeoff() {
    // 检查速度条件
    if (velocity >= abortSpeed) {
        return true;
    }
    
    // 检查其他条件（如发动机故障、系统警告等）
    if (engineFailure || systemWarning) {
        return true;
    }
    
    return false;
}

// 触发中断起飞
void triggerAbortTakeoff() {
    // 1. 发布中断起飞事件
    EventBus::getInstance().publish(Event::ABORT_TAKEOFF);
    
    // 2. 切换到自动模式
    switchToAutomaticMode();
    
    // 3. 启动刹车控制器
    controllerManager.startController("brake_controller");
    
    // 4. 减小推力
    controllerManager.startController("throttle_controller");
}
```

### 4. 制动阶段

```cpp
// 制动过程
void brakingPhase() {
    while (velocity > 0.1) {  // 直到速度接近零
        // 1. 应用刹车力
        double brakeForce = calculateBrakeForce();
        applyBrakeForce(brakeForce);
        
        // 2. 减小推力
        reduceThrust();
        
        // 3. 更新物理状态
        updatePhysics();
        
        // 4. 记录数据
        recordData();
        
        // 5. 检查是否停止
        if (velocity <= 0.1) {
            completeAbortTakeoff();
            break;
        }
    }
}
```

## 物理模型

### 动力学方程

```cpp
// 主要动力学方程
class AbortTakeoffDynamics {
public:
    void updateState(double dt) {
        // 1. 计算合力
        double totalForce = calculateTotalForce();
        
        // 2. 更新加速度
        double acceleration = totalForce / aircraftMass;
        
        // 3. 更新速度
        velocity += acceleration * dt;
        
        // 4. 更新位置
        position += velocity * dt;
        
        // 5. 确保速度不为负
        if (velocity < 0) {
            velocity = 0;
        }
    }
    
private:
    double calculateTotalForce() {
        // 推力
        double thrustForce = currentThrust;
        
        // 阻力
        double dragForce = calculateDragForce();
        
        // 刹车力
        double brakeForce = currentBrakeForce;
        
        // 摩擦力
        double frictionForce = calculateFrictionForce();
        
        return thrustForce - dragForce - brakeForce - frictionForce;
    }
    
    double calculateDragForce() {
        // 空气阻力 = 0.5 * 空气密度 * 速度² * 阻力系数 * 参考面积
        return 0.5 * airDensity * velocity * velocity * dragCoeff * referenceArea;
    }
    
    double calculateFrictionForce() {
        // 地面摩擦力 = 摩擦系数 * 重力
        return frictionCoeff * aircraftMass * gravity;
    }
};
```

### 刹车力计算

```cpp
// 刹车力计算模型
double calculateBrakeForce() {
    // 基础刹车力
    double baseBrakeForce = brakeForceCoeff * aircraftMass * gravity;
    
    // 速度因子（速度越高，刹车效果越好）
    double speedFactor = std::min(velocity / 30.0, 1.0);
    
    // 总刹车力
    double totalBrakeForce = baseBrakeForce * speedFactor;
    
    return totalBrakeForce;
}
```

## 控制器实现

### 油门控制器

```cpp
class ThrottleController : public BaseController {
private:
    double targetThrust = 0.0;
    double thrustReductionTime = 2.0;
    double currentThrust = 0.0;
    
public:
    void update() override {
        if (!hasAuthority()) return;
        
        // 计算目标推力
        if (thrustReductionTime > 0) {
            // 逐渐减小推力
            double reductionRate = maxThrust / thrustReductionTime;
            targetThrust = std::max(0.0, maxThrust - reductionRate * elapsedTime);
        } else {
            targetThrust = 0.0;
        }
        
        // 平滑推力变化
        double thrustChange = (targetThrust - currentThrust) * 0.1;
        currentThrust += thrustChange;
        
        // 应用推力
        applyThrust(currentThrust);
    }
    
    bool hasAuthority() override {
        return flightMode == FlightMode::AUTOMATIC || 
               flightMode == FlightMode::SEMI_AUTOMATIC;
    }
};
```

### 刹车控制器

```cpp
class BrakeController : public BaseController {
private:
    double brakeForceCoeff = 0.8;
    
public:
    void update() override {
        if (!hasAuthority()) return;
        
        // 计算刹车力
        double brakeForce = calculateBrakeForce();
        
        // 应用刹车力
        applyBrakeForce(brakeForce);
    }
    
    bool hasAuthority() override {
        return flightMode == FlightMode::AUTOMATIC;
    }
    
private:
    double calculateBrakeForce() {
        double baseForce = brakeForceCoeff * aircraftMass * gravity;
        double speedFactor = std::min(velocity / 30.0, 1.0);
        return baseForce * speedFactor;
    }
};
```

## 数据分析

### 关键指标

1. **停止距离**: 从触发中断到完全停止的距离
2. **停止时间**: 从触发中断到完全停止的时间
3. **最大减速度**: 制动过程中的最大减速度
4. **刹车效率**: 实际刹车力与理论刹车力的比值

### 数据输出格式

```csv
时间(s),位置(m),速度(m/s),推力(N),阻力(N),刹车力(N),控制模式
0.00,0.00,0.00,0.00,0.00,0.00,MANUAL
0.01,0.00,0.12,120000.00,0.00,0.00,MANUAL
0.02,0.00,0.24,120000.00,0.00,0.00,MANUAL
...
30.00,450.00,30.00,120000.00,22050.00,0.00,AUTOMATIC
30.01,450.00,29.85,119400.00,21945.00,39240.00,AUTOMATIC
...
```

### 性能分析

```cpp
// 性能分析函数
struct AbortTakeoffPerformance {
    double stopDistance;    // 停止距离 (m)
    double stopTime;        // 停止时间 (s)
    double maxDeceleration; // 最大减速度 (m/s²)
    double brakeEfficiency; // 刹车效率 (%)
    
    void calculate(const std::vector<SimulationState>& data) {
        // 找到触发点
        auto abortPoint = findAbortPoint(data);
        
        // 找到停止点
        auto stopPoint = findStopPoint(data, abortPoint);
        
        // 计算指标
        stopDistance = stopPoint.position - abortPoint.position;
        stopTime = stopPoint.time - abortPoint.time;
        maxDeceleration = calculateMaxDeceleration(data, abortPoint, stopPoint);
        brakeEfficiency = calculateBrakeEfficiency(data, abortPoint, stopPoint);
    }
};
```

## 场景验证

### 验证标准

1. **安全性验证**
   - 飞机能在跑道长度内停止
   - 制动过程平稳，无剧烈振动
   - 控制系统响应及时

2. **性能验证**
   - 停止距离符合设计要求
   - 停止时间在可接受范围内
   - 刹车系统工作正常

3. **功能验证**
   - 事件检测准确
   - 控制器切换正确
   - 数据记录完整

### 测试用例

```cpp
// 测试用例示例
class AbortTakeoffTest {
public:
    void testNormalAbort() {
        // 正常中断起飞测试
        setInitialConditions();
        runSimulation();
        verifyResults();
    }
    
    void testEmergencyAbort() {
        // 紧急中断起飞测试
        setEmergencyConditions();
        runSimulation();
        verifyEmergencyResults();
    }
    
    void testBoundaryConditions() {
        // 边界条件测试
        testMaxSpeedAbort();
        testMinSpeedAbort();
        testWetRunwayAbort();
    }
};
```

## 优化建议

### 参数优化

1. **刹车力度调整**
   - 根据跑道条件调整刹车系数
   - 考虑轮胎磨损影响

2. **推力减小策略**
   - 优化推力减小时间
   - 考虑发动机响应特性

3. **触发条件优化**
   - 调整触发速度阈值
   - 增加更多触发条件

### 算法改进

1. **自适应控制**
   - 根据实时状态调整控制参数
   - 学习历史数据优化策略

2. **预测控制**
   - 预测停止距离和停止时间
   - 提前调整控制策略

3. **多目标优化**
   - 平衡停止距离和舒适性
   - 考虑系统磨损最小化

---

*中断起飞场景是VFT系统的重要组成部分，为飞行员训练和系统验证提供了重要的测试平台* 