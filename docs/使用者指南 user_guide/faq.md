# 常见问题 FAQ

### Q1. ParaSAFE 支持哪些操作系统？
A：支持 Windows（推荐）、Linux、macOS，建议使用 64 位系统。

### Q2. 编译报错怎么办？
A：
- 检查 C++ 版本（需 C++17 及以上）
- 依赖库是否安装齐全
- 路径是否有中文、空格
- 参考开发者文档的环境配置部分

### Q3. 如何切换机型/场景/模型？
A：
- 修改 Scenario_Lib/下的配置文件（如 Taxi_config.txt）
- 或在主程序中更换 AircraftConfig/ForceModel/DynamicsModel/VirtualPilot 实例

### Q4. 输出数据在哪里？
A：
- 仿真数据、日志默认输出到各场景 output/ 目录
- 包括 data.csv、log_brief.txt、log_detail.txt

### Q5. 控制器/飞行员/力学模型能自定义吗？
A：
- 支持。可实现对应接口（如 IForceModel、IDynamicsModel、IVirtualPilot）并在主流程注册

### Q6. VSCode 报 includePath 错误？
A：
- 检查 c_cpp_properties.json 配置，确保 includePath 覆盖所有 include 目录
- 参考开发者文档的 VSCode 配置说明

### Q7. 运行崩溃/无输出？
A：
- 检查配置文件、数据文件是否齐全
- 查看 log_detail.txt 日志定位问题

### Q8. 社区/技术支持渠道？
A：
- 访问官方文档、GitHub Issues、社区论坛、微信群等

如有更多问题，欢迎在社区反馈！ 