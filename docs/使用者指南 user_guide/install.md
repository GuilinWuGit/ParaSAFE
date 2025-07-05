# 安装与环境配置

## 1. 获取源码
- 访问 [GitHub 仓库](https://github.com/your-username/ParaSAFE) 或通过 Gitee 镜像下载。
- 推荐使用 `git clone` 获取最新版：
  ```bash
  git clone https://github.com/your-username/ParaSAFE.git
  ```

## 2. 安装依赖
- Windows：推荐使用 MSVC/MinGW + CMake + Git Bash/PowerShell
- Linux/macOS：推荐 GCC/Clang + CMake + Git
- 主要依赖：C++17、CMake 3.15+、标准库、（可选）OpenMP、Boost、spdlog 等
- 安装示例：
  ```bash
  # Ubuntu
  sudo apt update && sudo apt install build-essential cmake git
  # Windows/MSYS2
  pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake git
  ```

## 3. 编译与运行
```bash
cd ParaSAFE
mkdir build && cd build
cmake ..
cmake --build .
./Scenario_Lib/A_Taxi/Taxi_sim.exe   # 或 ./Scenario_Lib/B_Abort_TakeOff/Abrot_Takeoff_sim.exe
```

## 4. 常见环境问题
- 编译报错：检查 C++ 版本、依赖是否齐全、路径是否有中文/空格
- VSCode 配置：建议设置 includePath，参考开发者文档
- 运行崩溃：检查数据/配置文件是否齐全，日志输出位置

## 5. 其他
- 支持 Docker 镜像、云端部署（详见开发者文档）
- 如遇特殊环境问题，欢迎在社区反馈 