# 场景与模型扩展

## 1. 支持的扩展类型
- 机型（AircraftConfig）
- 力学模型（IForceModel）
- 动力学模型（IDynamicsModel）
- 飞行员模型（IVirtualPilot）
- 大气/风/环境模型（IAtmosphereModel/IWindModel）
- 跑道/机场模型（IRunwayModel）
- 空管模型（IAirTrafficControl）

## 2. 扩展方法
- 新建对应接口的子类，实现自定义逻辑
- 在主流程注册/替换为自定义实例
- 配置文件/命令行参数支持动态切换（可选）

## 3. 示例：自定义飞行员模型
```cpp
class MyPilot : public IVirtualPilot {
public:
    void update(SharedStateSpace& state) override {
        // 自定义控制逻辑
        state.throttle.store(0.5);
        state.brake.store(0.0);
    }
};
// 在主流程中注册：
std::shared_ptr<IVirtualPilot> pilot = std::make_shared<MyPilot>();
```

## 4. 注意事项
- 建议参考现有模块实现风格
- 保持接口兼容性，便于与主流程集成
- 可在社区分享你的扩展模块

## 5. 进阶
- 支持插件化、热插拔、批量仿真等高级扩展（详见开发者文档）

如有扩展需求或遇到问题，欢迎在社区交流！ 