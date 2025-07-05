# 第一个场景：中断起飞（Abort Takeoff）

本教程将带您快速体验 ParaSAFE 仿真系统，通过"中断起飞"场景，完成第一个飞行仿真任务。

---

## 1. 目录结构

```
ParaSAFE/
├── Scenario_Lib/
│   └── B_Abort_TakeOff/
│       ├── main_AbortTakeoff.cpp           # 场景主程序
│       ├── abort_takeoff_config.hpp        # 场景参数头文件
│       ├── abort_takeoff_events.hpp        # 事件定义
│       ├── abort_takeoff_initial_state.hpp # 初始状态设置
│       ├── AbortTakeoff_Config/
│       │   └── abort_takeoff_config.txt    # 配置文件
│       └── output/
│           ├── data.csv                   # 仿真数据输出
│           ├── log_brief.txt              # 简要日志
│           └── log_detail.txt             # 详细日志
```

---

## 2. 编译场景程序

请确保已安装 C++17 兼容编译器（如 g++）。

```bash
cd Scenario_Lib/B_Abort_TakeOff
g++ -std=c++17 -pthread -o abort_takeoff_sim.exe main_AbortTakeoff.cpp
```

---

## 3. 运行仿真

```bash
./abort_takeoff_sim.exe
```

运行后，仿真程序会自动读取配置文件，初始化状态，并开始中断起飞流程。

---

## 4. 查看仿真输出

仿真结束后，您可以在 `output/` 目录下找到结果文件：

- `data.csv`         ：详细的仿真数据（时间、位置、速度、加速度、油门、刹车等）
- `log_brief.txt`    ：简要日志，记录关键事件和状态
- `log_detail.txt`   ：详细日志，包含每一步的状态变化和控制器动作

示例：
```bash
head -10 output/data.csv
tail -20 output/log_brief.txt
```

---

## 5. 修改仿真参数

您可以通过编辑配置文件 `AbortTakeoff_Config/abort_takeoff_config.txt` 来调整仿真参数。

常用参数示例：
```
ABORT_SPEED = 25.0         # 中断起飞触发速度 (m/s)
BRAKE_FORCE = 0.8          # 刹车力度系数 (0~1)
THRUST_REDUCTION_TIME = 1.0 # 推力减小时间 (s)
```

修改后，重新运行仿真程序即可生效。

---

## 6. 进阶探索

- 查看和修改事件定义（`abort_takeoff_events.hpp`）
- 调整初始状态（`abort_takeoff_initial_state.hpp`）
- 结合数据输出进行实验分析

---

## 7. 常见问题

- **编译报错**：请确认编译器支持 C++17，且 includePath 配置正确。
- **仿真无输出**：请检查配置文件参数是否合理，或查看 `log_detail.txt` 获取详细信息。

---

恭喜！您已成功运行了 ParaSAFE 的第一个飞行仿真场景。

如需进一步学习，请参考 [完整文档](../README.md) 或探索更多场景。 