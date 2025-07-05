# 贡献指南

感谢您对VFT项目的关注！我们欢迎各种形式的贡献，包括代码、文档、问题报告和功能建议。

## 🤝 如何贡献

### 报告问题

如果您发现了bug或有功能建议，请：

1. **搜索现有问题**: 在提交新问题前，请先搜索是否已有类似问题
2. **使用问题模板**: 使用我们提供的问题模板，包含必要信息
3. **提供详细信息**: 
   - 操作系统和版本
   - 编译器版本
   - 错误信息和日志
   - 重现步骤

### 提交代码

#### 开发流程

1. **Fork项目**: 在GitHub上fork本项目
2. **创建分支**: 为您的功能创建新分支
   ```bash
   git checkout -b feature/your-feature-name
   ```
3. **开发代码**: 编写代码并添加测试
4. **提交更改**: 使用清晰的提交信息
   ```bash
   git commit -m "feat: add new controller for landing gear"
   ```
5. **推送分支**: 推送到您的fork
   ```bash
   git push origin feature/your-feature-name
   ```
6. **创建PR**: 在GitHub上创建Pull Request

#### 代码规范

##### C++代码规范

- **命名约定**:
  - 类名: `PascalCase` (如 `ThrottleController`)
  - 函数名: `camelCase` (如 `calculateThrust`)
  - 变量名: `snake_case` (如 `max_thrust`)
  - 常量: `UPPER_SNAKE_CASE` (如 `MAX_THRUST`)

- **文件组织**:
  - 头文件: `.hpp` 扩展名
  - 源文件: `.cpp` 扩展名
  - 每个类一个文件

- **代码格式**:
  ```cpp
  class ThrottleController : public BaseController {
  private:
      double max_thrust_;
      double current_thrust_;
      
  public:
      ThrottleController(double max_thrust);
      void update() override;
      bool hasAuthority() override;
  };
  ```

##### 提交信息规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 格式：

- `feat:` 新功能
- `fix:` 修复bug
- `docs:` 文档更新
- `style:` 代码格式调整
- `refactor:` 代码重构
- `test:` 测试相关
- `chore:` 构建过程或辅助工具的变动

示例：
```bash
feat: add landing gear controller
fix: resolve thread safety issue in event bus
docs: update installation guide for Windows
```

### 文档贡献

#### 文档类型

1. **用户文档**: 安装指南、使用教程、配置说明
2. **开发者文档**: API文档、架构说明、开发指南
3. **技术文档**: 性能分析、故障排除、最佳实践

#### 文档规范

- 使用Markdown格式
- 包含代码示例
- 添加适当的图片和图表
- 保持文档结构清晰

## 🏗️ 开发环境设置

### 环境要求

- C++17 兼容的编译器
- CMake 3.15+
- Git
- 支持的操作系统: Windows, Linux, macOS

### 本地开发设置

1. **克隆项目**
   ```bash
   git clone https://github.com/your-username/VFT.git
   cd VFT
   ```

2. **创建构建目录**
   ```bash
   mkdir build
   cd build
   ```

3. **配置项目**
   ```bash
   cmake ..
   ```

4. **编译项目**
   ```bash
   make -j4
   ```

5. **运行测试**
   ```bash
   make test
   ```

## 📋 开发指南

### 添加新控制器

1. **创建控制器类**
   ```cpp
   // include/B_Flight_Control/landing_gear_controller.hpp
   class LandingGearController : public BaseController {
   public:
       LandingGearController();
       void update() override;
       bool hasAuthority() override;
   private:
       void extendGear();
       void retractGear();
   };
   ```

2. **实现控制器逻辑**
   ```cpp
   // 在相应的源文件中实现
   void LandingGearController::update() {
       if (!hasAuthority()) return;
       
       // 实现控制逻辑
   }
   ```

3. **注册控制器**
   ```cpp
   // 在控制器管理器中注册
   controllerManager.registerController("landing_gear", 
       std::make_unique<LandingGearController>());
   ```

4. **添加配置**
   ```ini
   # 在配置文件中添加参数
   LANDING_GEAR_EXTENSION_TIME = 10.0
   LANDING_GEAR_RETRACTION_TIME = 8.0
   ```

### 添加新场景

1. **创建场景目录**
   ```bash
   mkdir Scenario_Lib/C_Landing
   ```

2. **创建主程序**
   ```cpp
   // Scenario_Lib/C_Landing/main.cpp
   #include "landing_config.hpp"
   #include "landing_events.hpp"
   
   int main() {
       // 场景初始化
       // 运行仿真
       return 0;
   }
   ```

3. **定义场景配置**
   ```cpp
   // landing_config.hpp
   struct LandingConfig {
       double approach_speed = 70.0;
       double flare_height = 15.0;
       double touchdown_speed = 65.0;
   };
   ```

4. **实现事件检测**
   ```cpp
   // landing_event_detection.hpp
   class LandingEventDetection {
   public:
       void detectEvents(const SimulationState& state);
   private:
       void detectTouchdown();
       void detectFlare();
   };
   ```

### 测试指南

#### 单元测试

```cpp
// tests/test_throttle_controller.cpp
#include <gtest/gtest.h>
#include "throttle_controller.hpp"

class ThrottleControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        controller = std::make_unique<ThrottleController>(100000.0);
    }
    
    std::unique_ptr<ThrottleController> controller;
};

TEST_F(ThrottleControllerTest, InitialThrustIsZero) {
    EXPECT_EQ(controller->getCurrentThrust(), 0.0);
}

TEST_F(ThrottleControllerTest, CanSetThrust) {
    controller->setThrust(50000.0);
    EXPECT_EQ(controller->getCurrentThrust(), 50000.0);
}
```

#### 集成测试

```cpp
// tests/integration/test_abort_takeoff.cpp
TEST(AbortTakeoffIntegration, CompleteScenario) {
    // 设置场景
    AbortTakeoffScenario scenario;
    scenario.initialize();
    
    // 运行仿真
    scenario.run();
    
    // 验证结果
    EXPECT_LE(scenario.getStopDistance(), 2000.0);
    EXPECT_LE(scenario.getStopTime(), 60.0);
}
```

## 🔍 代码审查

### 审查标准

1. **功能正确性**: 代码是否实现了预期功能
2. **代码质量**: 代码是否清晰、可读、可维护
3. **性能影响**: 是否对系统性能有负面影响
4. **安全性**: 是否引入了安全风险
5. **测试覆盖**: 是否有足够的测试覆盖

### 审查流程

1. **自动检查**: CI/CD流水线自动运行测试和代码检查
2. **人工审查**: 至少需要一名维护者审查
3. **讨论反馈**: 在PR中进行讨论和反馈
4. **合并代码**: 审查通过后合并到主分支

## 📊 性能要求

### 实时性能

- 仿真频率: 1000Hz (1ms步长)
- 响应延迟: < 1ms
- CPU占用: < 10% (单核)

### 内存使用

- 内存占用: < 100MB
- 内存泄漏: 零容忍
- 内存碎片: 最小化

### 代码质量

- 代码覆盖率: > 80%
- 静态分析: 通过所有检查
- 编译警告: 零警告

## 🐛 调试指南

### 常见问题

1. **编译错误**
   ```bash
   # 检查编译器版本
   g++ --version
   
   # 检查C++标准
   g++ -std=c++17 -dM -E - < /dev/null | grep -i "cplusplus"
   ```

2. **运行时错误**
   ```bash
   # 使用调试器
   gdb ./abort_takeoff_sim
   
   # 检查内存泄漏
   valgrind --leak-check=full ./abort_takeoff_sim
   ```

3. **性能问题**
   ```bash
   # 性能分析
   perf record ./abort_takeoff_sim
   perf report
   ```

### 日志调试

```cpp
// 启用调试日志
Logger::getInstance().setLevel(LogLevel::DEBUG);

// 添加调试信息
LOG_DEBUG("Throttle controller updating, current thrust: {}", current_thrust_);
```

## 📞 获取帮助

### 社区资源

- **GitHub Issues**: 报告问题和功能请求
- **GitHub Discussions**: 讨论技术问题和最佳实践
- **Wiki**: 项目文档和教程
- **邮件列表**: 开发者交流

### 联系方式

- **项目维护者**: [维护者邮箱]
- **技术支持**: [技术支持邮箱]
- **社区论坛**: [论坛链接]

## 📜 行为准则

### 社区准则

1. **尊重他人**: 尊重所有贡献者，无论经验水平
2. **建设性反馈**: 提供建设性和有帮助的反馈
3. **包容性**: 欢迎不同背景和经验的贡献者
4. **专业行为**: 保持专业和礼貌的交流

### 不当行为

以下行为不被容忍：
- 骚扰或歧视
- 恶意攻击或威胁
- 垃圾信息或广告
- 违反开源许可证

## 📄 许可证

通过贡献代码，您同意您的贡献将在项目的许可证下发布。

---

感谢您对VFT项目的贡献！您的参与使这个项目变得更好。 