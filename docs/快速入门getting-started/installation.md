# 安装指南

## 系统要求

### 硬件要求
- **CPU**: Intel i5 或 AMD Ryzen 5 及以上
- **内存**: 8GB RAM (推荐 16GB)
- **存储**: 2GB 可用空间
- **网络**: 可选，用于下载依赖

### 软件要求
- **操作系统**: Windows 10/11, Linux (Ubuntu 18.04+), macOS 10.15+
- **编译器**: 
  - Windows: Visual Studio 2019+ 或 MinGW-w64
  - Linux: GCC 7.0+ 或 Clang 6.0+
  - macOS: Xcode 11+ 或 Clang 6.0+
- **C++标准**: C++17 或更高版本

## 快速安装

### Windows 安装

#### 方法一：使用 Visual Studio

1. **安装 Visual Studio**
   ```bash
   # 下载 Visual Studio Community (免费)
   # https://visualstudio.microsoft.com/downloads/
   ```

2. **安装 C++ 开发工具**
   - 打开 Visual Studio Installer
   - 选择 "C++ 桌面开发" 工作负载
   - 确保包含 MSVC 编译器

3. **克隆项目**
   ```bash
   git clone https://github.com/your-username/VFT.git
   cd VFT
   ```

4. **编译项目**
   ```bash
   # 进入场景目录
   cd Scenario_Lib/B_Abort_TakeOff
   
   # 使用 MSVC 编译
   cl /std:c++17 /EHsc main.cpp /Fe:abort_takeoff_sim.exe
   ```

#### 方法二：使用 MinGW-w64

1. **安装 MinGW-w64**
   ```bash
   # 下载 MSYS2
   # https://www.msys2.org/
   
   # 安装后打开 MSYS2 终端
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-make
   ```

2. **编译项目**
   ```bash
   cd Scenario_Lib/B_Abort_TakeOff
   g++ -std=c++17 -pthread -o abort_takeoff_sim.exe main.cpp
   ```

### Linux 安装

#### Ubuntu/Debian

1. **安装依赖**
   ```bash
   sudo apt update
   sudo apt install build-essential git cmake
   ```

2. **克隆项目**
   ```bash
   git clone https://github.com/your-username/VFT.git
   cd VFT
   ```

3. **编译项目**
   ```bash
   cd Scenario_Lib/B_Abort_TakeOff
   g++ -std=c++17 -pthread -o abort_takeoff_sim main.cpp
   ```

#### CentOS/RHEL

1. **安装依赖**
   ```bash
   sudo yum groupinstall "Development Tools"
   sudo yum install git cmake
   ```

2. **编译项目**
   ```bash
   cd Scenario_Lib/B_Abort_TakeOff
   g++ -std=c++17 -pthread -o abort_takeoff_sim main.cpp
   ```

### macOS 安装

1. **安装 Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

2. **克隆项目**
   ```bash
   git clone https://github.com/your-username/VFT.git
   cd VFT
   ```

3. **编译项目**
   ```bash
   cd Scenario_Lib/B_Abort_TakeOff
   clang++ -std=c++17 -pthread -o abort_takeoff_sim main.cpp
   ```

## 验证安装

### 运行测试场景

1. **进入场景目录**
   ```bash
   cd Scenario_Lib/B_Abort_TakeOff
   ```

2. **运行仿真**
   ```bash
   # Windows
   ./abort_takeoff_sim.exe
   
   # Linux/macOS
   ./abort_takeoff_sim
   ```

3. **检查输出文件**
   ```bash
   # 查看生成的文件
   ls -la *.csv *.txt
   
   # 检查数据文件
   head -10 data.csv
   ```

### 预期输出

运行成功后，您应该看到：
- 控制台输出仿真进度
- 生成 `data.csv` 数据文件
- 生成 `log_brief.txt` 和 `log_detail.txt` 日志文件

## 配置系统

### 修改配置文件

1. **编辑场景配置**
   ```bash
   # 编辑 abort_takeoff_config.txt
   notepad abort_takeoff_config.txt  # Windows
   nano abort_takeoff_config.txt     # Linux/macOS
   ```

2. **调整参数**
   ```ini
   # 修改中断起飞速度
   ABORT_SPEED = 30.0
   
   # 修改刹车力度
   BRAKE_FORCE = 0.8
   ```

3. **重新运行仿真**
   ```bash
   ./abort_takeoff_sim.exe
   ```

## 开发环境设置

### 使用 IDE

#### Visual Studio Code

1. **安装扩展**
   - C/C++ Extension Pack
   - CMake Tools
   - GitLens

2. **配置工作区**
   ```json
   {
     "files.associations": {
       "*.hpp": "cpp",
       "*.cpp": "cpp"
     },
     "C_Cpp.default.cppStandard": "c++17"
   }
   ```

#### CLion

1. **打开项目**
   - File → Open → 选择 VFT 目录

2. **配置 CMake**
   - 自动检测 C++17 标准
   - 配置编译选项

### 使用 CMake (可选)

1. **创建 CMakeLists.txt**
   ```cmake
   cmake_minimum_required(VERSION 3.15)
   project(VFT)
   
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   
   add_executable(abort_takeoff_sim main.cpp)
   target_compile_features(abort_takeoff_sim PRIVATE cxx_std_17)
   ```

2. **构建项目**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## 故障排除

### 常见问题

#### 编译错误

**问题**: `'thread' is not a member of 'std'`
**解决**: 确保使用 C++17 标准
```bash
g++ -std=c++17 -pthread -o program main.cpp
```

**问题**: `pthread.h: No such file or directory`
**解决**: Windows 下不需要 pthread 标志
```bash
g++ -std=c++17 -o program main.cpp
```

#### 运行时错误

**问题**: `Permission denied`
**解决**: 确保可执行文件有执行权限
```bash
chmod +x abort_takeoff_sim
```

**问题**: `找不到配置文件`
**解决**: 确保在正确的目录下运行
```bash
cd Scenario_Lib/B_Abort_TakeOff
./abort_takeoff_sim
```

### 性能优化

#### 编译优化
```bash
# 启用优化
g++ -O2 -std=c++17 -pthread -o program main.cpp

# 启用调试信息
g++ -g -std=c++17 -pthread -o program main.cpp
```

#### 运行时优化
- 调整 `SIMULATION_TIME_STEP` 参数
- 减少日志输出级别
- 关闭不必要的功能

## 下一步

安装完成后，您可以：

1. **阅读用户指南**: 了解如何使用系统
2. **运行示例场景**: 体验不同的飞行场景
3. **修改配置**: 调整仿真参数
4. **查看文档**: 深入了解系统架构

---

*如有问题，请查看 [故障排除](../troubleshooting/faq.md) 或提交 [Issue](https://github.com/your-username/VFT/issues)* 