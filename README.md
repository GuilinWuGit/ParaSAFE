# ParaSAFE 虚拟试飞与态势感知系统

> **Parallel Situational Awareness & Forecasting Environment (ParaSAFE)**
>
> 南航航空航天大学 先进飞行器设计国防重点实验室

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/std/the-standard)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)]()
[![Documentation](https://img.shields.io/badge/Docs-Online-blue.svg)]()

> 一个高保真、面向对象、多线程并行、严格同步、软实时的虚拟试飞与飞行态势感知系统，专为航空领域大规模仿真与控制算法验证设计。完全开源，欢迎国内外学者、工程师、学生共同参与和贡献。

---

## 🚀 快速开始

### 安装与编译

```bash
# 克隆项目
git clone https://github.com/your-username/ParaSAFE.git
cd ParaSAFE

# 编译中断起飞场景
cd Scenario_Lib/B_Abort_TakeOff
g++ -std=c++17 -pthread -o abort_takeoff_sim.exe main_AbortTakeoff.cpp

# 运行仿真
./abort_takeoff_sim.exe
```

### 文档入口

- [完整文档](docs/)
- [快速开始](docs/getting-started/installation.md)
- [系统架构](docs/architecture/overview.md)
- [示例教程](docs/examples/abort-takeoff.md)

---

## 🌟 核心特性

- **高频多线程仿真**：1000Hz，微秒级精度
- **事件驱动架构**：松耦合，支持动态响应
- **多控制器系统**：油门、刹车、跑道巡航、俯仰角等
- **配置驱动**：参数热加载，无需重编译
- **高保真物理建模**：精确动力学与气动力学
- **详细数据记录**：支持实验分析与可视化

---

## 🏗️ 系统架构

```
┌───────────────────────────────┐
│        应用层 (Application)   │
├───────────────────────────────┤
│      业务逻辑层 (Business)    │
├───────────────────────────────┤
│        模型层 (Model)         │
├───────────────────────────────┤
│   基础设施层 (Infrastructure) │
└───────────────────────────────┘
```

### 主要模块

- **A_Aircraft_Configuration**：飞机参数配置
- **B_Aircraft_Forces_Model**：气动力与力学模型
- **C_Flight_Control**：飞行控制系统（油门、刹车、巡航、俯仰角等）
- **D_DynamicModel**：动力学模型（如固定翼线性模型等）
- **E_Virtual_Environment**：虚拟环境建模
- **F_Virtual_Pilot**：虚拟飞行员
- **G_Virtual_Airport**：虚拟机场
- **H_Virtual_AirTrafficManagement**：虚拟空管
- **I_Virtual_Sensor**：虚拟传感器
- **K_Scenario**：场景与事件系统
- **L_Simulation_Settings**：仿真设置与时钟
- **Scenario_Lib**：典型仿真场景（如滑行、中断起飞等）

---

## 📁 目录结构

```
ParaSAFE/
├── include/                        # 头文件库
│   ├── A_Aircraft_Configuration/   # 飞机参数配置
│   ├── B_Aircraft_Forces_Model/    # 气动力模型
│   ├── C_Flight_Control/           # 飞行控制系统
│   ├── D_DynamicModel/             # 动力学模型
│   ├── E_Virtual_Environment/      # 虚拟环境
│   ├── F_Virtual_Pilot/            # 虚拟飞行员
│   ├── G_Virtual_Airport/          # 虚拟机场
│   ├── H_Virtual_AirTrafficManagement/ # 虚拟空管
│   ├── I_Virtual_Sensor/           # 虚拟传感器
│   ├── K_Scenario/                 # 场景与事件系统
│   ├── L_Simulation_Settings/      # 仿真设置
├── Scenario_Lib/                   # 场景库
│   ├── A_Taxi/                     # 滑行场景
│   └── B_Abort_TakeOff/            # 中断起飞场景
├── docs/                           # 项目文档
│   ├── architecture/               # 架构文档
│   ├── developer-guide/            # 开发者指南
│   ├── examples/                   # 示例教程
│   ├── getting-started/            # 快速开始
│   ├── introduction/               # 项目介绍
│   └── Vision-Control/             # 视觉控制相关
├── scripts/                        # 辅助脚本
├── .github/                        # GitHub配置
├── .vscode/                        # VSCode配置
├── CONTRIBUTING.md                 # 贡献指南
└── README.md                       # 项目说明
```

---

## 🎮 典型场景

### 滑行 (A_Taxi)
- 速度保持与刹车控制
- 事件触发与状态切换
- 配置文件：`Scenario_Lib/A_Taxi/Taxi_Config/Taxi_config.txt`
- 主程序：`Scenario_Lib/A_Taxi/main_Taxi.cpp`

### 中断起飞 (B_Abort_TakeOff)
- 实时状态监控
- 多控制器协调
- 事件驱动响应
- 详细数据记录
- 配置文件：`Scenario_Lib/B_Abort_TakeOff/AbortTakeoff_Config/abort_takeoff_config.txt`
- 主程序：`Scenario_Lib/B_Abort_TakeOff/main_AbortTakeoff.cpp`

---

## 📊 性能指标

|  指标   |  数值              |
|---------|-------------------|
| 仿真频率 | 1000Hz (1ms步长)  |
| 时间精度 | 微秒级            |
| 内存占用 | < 100MB          |
| CPU占用 | < 10% (单核)      |
| 支持场景 | 10+ 种           |
| 控制器数量 | 5+ 种           |

---

## 🔧 技术栈

- **语言**: C++17
- **架构**: 多线程、事件驱动
- **设计模式**: 观察者、策略、工厂、单例
- **平台**: Windows, Linux, macOS
- **构建**: CMake, Make
- **文档**: Markdown, docsify

---

## 🚀 快速体验

### 1. 运行中断起飞场景

```bash
cd Scenario_Lib/B_Abort_TakeOff
./abort_takeoff_sim.exe
```

### 2. 查看输出文件

```bash
head -10 output/data.csv
tail -20 output/log_brief.txt
```

### 3. 修改配置参数

```bash
notepad AbortTakeoff_Config/abort_takeoff_config.txt
# 修改 ABORT_SPEED = 25.0
```

---

## 🤝 贡献指南

我们欢迎各种形式的贡献！

- 🐛 [报告问题](https://github.com/your-username/ParaSAFE/issues)
- 💡 [功能建议](https://github.com/your-username/ParaSAFE/discussions)
- 📝 [文档改进](https://github.com/your-username/ParaSAFE/pulls)
- 🔧 [代码贡献](CONTRIBUTING.md)

### 开发环境

```bash
# 环境要求
- C++17 兼容编译器
- CMake 3.15+
- Git

# 本地开发
git clone https://github.com/your-username/ParaSAFE.git
cd ParaSAFE
mkdir build && cd build
cmake ..
make -j4
```

---

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE) - 查看 [LICENSE](LICENSE) 文件了解详情。

---

## 🙏 致谢

感谢所有为这个项目做出贡献的开发者和用户！


开发者
吴桂林：南京航空航天大学航空学院
IVAN BURDUN ：南京航空航天大学航空学院
何乃峰：南京航空航天大学自动化学院

---

<div align="center">

**ParaSAFE - 让飞行仿真更简单、更精确、更实用**

</div>

---
