# 版本号标注与管理指南

## 📋 目录

- [版本号标注位置](#版本号标注位置)
- [版本信息头文件](#版本信息头文件)
- [在代码中使用版本信息](#在代码中使用版本信息)
- [版本信息工具](#版本信息工具)
- [最佳实践](#最佳实践)

## 🏷️ 版本号标注位置

在C++项目中，版本号应该标注在以下关键位置：

### 1. **版本信息头文件** (主要位置)
- **文件**: `include/L_Simulation_Settings/version.hpp`
- **作用**: 集中管理所有版本信息
- **优势**: 单一数据源，易于维护

### 2. **主程序文件**
- **文件**: `Scenario_Lib/*/main.cpp`
- **作用**: 程序启动时显示版本信息
- **实现**: 包含版本头文件并调用显示函数

### 3. **日志系统**
- **文件**: `include/L_Simulation_Settings/logger.hpp`
- **作用**: 在日志中记录版本信息
- **实现**: 添加版本信息记录功能

### 4. **配置文件**
- **文件**: `*.txt`, `*.ini`, `*.cfg`
- **作用**: 配置文件版本标识
- **格式**: `VERSION = 1.0.0`

### 5. **文档文件**
- **文件**: `README.md`, `CHANGELOG.md`
- **作用**: 文档版本同步
- **格式**: 在文档中明确标注版本号

## 📄 版本信息头文件

### 文件结构

```cpp
// include/L_Simulation_Settings/version.hpp
namespace VFT {
    struct VersionInfo {
        static constexpr int MAJOR = 1;      // 主版本号
        static constexpr int MINOR = 0;      // 次版本号
        static constexpr int PATCH = 0;      // 修订号
        static constexpr const char* PRERELEASE = "";  // 预发布标识
        static constexpr const char* BUILD_METADATA = ""; // 构建元数据
        
        // 版本字符串
        static std::string getVersionString();
        
        // 版本比较
        static bool isNewerThan(int major, int minor, int patch);
        static bool isCompatibleWith(int major, int minor);
    };
    
    struct ProjectInfo {
        static constexpr const char* NAME = "Virtual Flight Training System";
        static constexpr const char* SHORT_NAME = "VFT";
        // ... 其他项目信息
    };
    
    struct BuildInfo {
        static constexpr const char* BUILD_DATE = __DATE__;
        static constexpr const char* BUILD_TIME = __TIME__;
        // ... 构建信息
    };
    
    class VersionUtils {
        // 版本信息工具方法
    };
}
```

### 版本号规范

遵循 [语义化版本控制](https://semver.org/lang/zh-CN/) 规范：

```
主版本号.次版本号.修订号[-预发布标识][+构建元数据]
```

- **主版本号 (MAJOR)**: 不兼容的API修改
- **次版本号 (MINOR)**: 向下兼容的功能性新增
- **修订号 (PATCH)**: 向下兼容的问题修正
- **预发布标识**: alpha, beta, rc 等
- **构建元数据**: 构建时间、提交哈希等

## 💻 在代码中使用版本信息

### 1. 包含版本头文件

```cpp
#include "../../include/L_Simulation_Settings/version.hpp"
```

### 2. 显示版本信息

```cpp
// 显示版本信息
void displayVersionInfo() {
    std::cout << "==========================================" << std::endl;
    std::cout << VFT::VersionUtils::getVersionInfo() << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "场景: 中断起飞 (Abort Takeoff)" << std::endl;
    std::cout << "构建信息: " << VFT::VersionUtils::getBuildInfo() << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << std::endl;
}

int main() {
    // 显示版本信息
    displayVersionInfo();
    
    // ... 其他代码
}
```

### 3. 版本兼容性检查

```cpp
// 检查版本兼容性
if (!VFT::VersionInfo::isCompatibleWith(1, 0)) {
    std::cerr << "错误: 需要VFT 1.0或更高版本" << std::endl;
    return 1;
}
```

### 4. 在日志中记录版本信息

```cpp
// 在日志中记录版本信息
Logger::getInstance().logVersionInfo();
```

### 5. 版本比较

```cpp
// 比较版本
if (VFT::VersionInfo::isNewerThan(1, 0, 0)) {
    std::cout << "当前版本比1.0.0更新" << std::endl;
}
```

## 🛠️ 版本信息工具

### 版本信息脚本

使用 `scripts/version_info.sh` 脚本管理版本信息：

```bash
# 显示当前版本号
./scripts/version_info.sh --version

# 显示完整版本信息
./scripts/version_info.sh --info

# 显示构建信息
./scripts/version_info.sh --build

# 检查版本兼容性
./scripts/version_info.sh --check

# 更新版本号
./scripts/version_info.sh --update 1.2.0
```

### 发布脚本

使用 `scripts/release.sh` 脚本进行版本发布：

```bash
# 发布主版本
./scripts/release.sh 2.0.0 major

# 发布次版本
./scripts/release.sh 1.2.0 minor

# 发布修订版本
./scripts/release.sh 1.0.1 patch

# 发布Beta版本
./scripts/release.sh 1.2.0-beta.1 beta
```

## 📋 最佳实践

### 1. 版本号管理原则

- **单一数据源**: 所有版本信息都从 `version.hpp` 获取
- **自动化更新**: 使用脚本自动更新版本号
- **版本同步**: 确保所有文件中的版本号一致
- **语义化版本**: 严格遵循语义化版本控制规范

### 2. 版本号标注位置优先级

1. **版本信息头文件** (最高优先级)
2. **主程序文件** (程序启动时显示)
3. **日志系统** (运行时记录)
4. **配置文件** (配置版本标识)
5. **文档文件** (用户可见)

### 3. 版本信息内容

#### 必需信息
- 版本号 (MAJOR.MINOR.PATCH)
- 项目名称
- 构建时间

#### 推荐信息
- 编译器信息
- 平台信息
- Git提交信息
- 版权信息

#### 可选信息
- 构建元数据
- 预发布标识
- 依赖项版本

### 4. 版本更新流程

```bash
# 1. 更新版本号
./scripts/version_info.sh --update 1.2.0

# 2. 检查版本兼容性
./scripts/version_info.sh --check

# 3. 提交版本更新
git add .
git commit -m "chore: bump version to 1.2.0"

# 4. 创建发布标签
git tag -a v1.2.0 -m "Release version 1.2.0"

# 5. 推送更改
git push origin main --tags
```

### 5. 版本兼容性检查

在代码中添加版本兼容性检查：

```cpp
// 在程序启动时检查版本兼容性
bool checkVersionCompatibility() {
    // 检查最低版本要求
    if (!VFT::VersionInfo::isCompatibleWith(1, 0)) {
        std::cerr << "错误: 需要VFT 1.0或更高版本" << std::endl;
        return false;
    }
    
    // 检查API兼容性
    if (VFT::VersionInfo::MAJOR > 1) {
        std::cout << "警告: 检测到主版本号变更，可能存在API不兼容" << std::endl;
    }
    
    return true;
}
```

### 6. 版本信息显示格式

```cpp
// 标准版本信息显示格式
std::string getStandardVersionDisplay() {
    std::ostringstream oss;
    oss << "VFT " << VFT::VersionInfo::getVersionString() << "\n"
        << "构建时间: " << VFT::BuildInfo::BUILD_DATE << " " 
        << VFT::BuildInfo::BUILD_TIME << "\n"
        << "编译器: " << VFT::BuildInfo::COMPILER << "\n"
        << "平台: " << VFT::BuildInfo::PLATFORM << " (" 
        << VFT::BuildInfo::ARCHITECTURE << ")\n"
        << "版权: " << VFT::ProjectInfo::COPYRIGHT;
    return oss.str();
}
```

## 🔧 自动化工具

### GitHub Actions 集成

在CI/CD流程中自动更新版本信息：

```yaml
# .github/workflows/version.yml
name: Version Management

on:
  push:
    tags:
      - 'v*'

jobs:
  update-version:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Update Version
      run: |
        VERSION=${GITHUB_REF#refs/tags/}
        ./scripts/version_info.sh --update $VERSION
        
    - name: Commit Changes
      run: |
        git config --local user.email "action@github.com"
        git config --local user.name "GitHub Action"
        git add .
        git commit -m "chore: update version to $VERSION"
        git push
```

### 版本检查工具

创建版本检查工具确保版本一致性：

```bash
#!/bin/bash
# scripts/check_version_consistency.sh

# 检查所有文件中的版本号是否一致
check_version_consistency() {
    local expected_version=$(./scripts/version_info.sh --version | cut -d' ' -f3)
    
    # 检查README.md
    local readme_version=$(grep -o 'Version: [0-9.]*' README.md | cut -d' ' -f2)
    if [ "$readme_version" != "$expected_version" ]; then
        echo "❌ README.md版本不匹配: 期望 $expected_version, 实际 $readme_version"
        return 1
    fi
    
    # 检查配置文件
    local config_version=$(grep -o 'VERSION = [0-9.]*' *.txt | cut -d' ' -f3 | head -1)
    if [ "$config_version" != "$expected_version" ]; then
        echo "❌ 配置文件版本不匹配: 期望 $expected_version, 实际 $config_version"
        return 1
    fi
    
    echo "✅ 所有文件版本号一致: $expected_version"
    return 0
}
```

## 📞 支持与帮助

如果您在版本管理过程中遇到问题：

1. **查看文档**: 参考本指南和相关文档
2. **使用工具**: 使用提供的脚本工具
3. **检查一致性**: 运行版本一致性检查
4. **联系维护者**: 通过GitHub Issues联系

---

**注意**: 本指南会随着项目发展持续更新，请定期查看最新版本。 