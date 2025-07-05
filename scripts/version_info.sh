#!/bin/bash

# VFT版本信息工具脚本
# 使用方法: ./scripts/version_info.sh [选项]

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 显示帮助信息
show_help() {
    echo "VFT版本信息工具"
    echo ""
    echo "使用方法:"
    echo "  $0 [选项]"
    echo ""
    echo "选项:"
    echo "  -v, --version     显示当前版本号"
    echo "  -i, --info        显示完整版本信息"
    echo "  -b, --build       显示构建信息"
    echo "  -c, --check       检查版本兼容性"
    echo "  -u, --update      更新版本号"
    echo "  -h, --help        显示此帮助信息"
    echo ""
    echo "示例:"
    echo "  $0 --version       # 显示版本号"
    echo "  $0 --info          # 显示完整信息"
    echo "  $0 --update 1.2.0  # 更新版本号"
}

# 获取当前版本号
get_current_version() {
    # 从版本头文件中提取版本号
    if [ -f "include/L_Simulation_Settings/version.hpp" ]; then
        local major=$(grep -o 'MAJOR = [0-9]*' include/L_Simulation_Settings/version.hpp | grep -o '[0-9]*')
        local minor=$(grep -o 'MINOR = [0-9]*' include/L_Simulation_Settings/version.hpp | grep -o '[0-9]*')
        local patch=$(grep -o 'PATCH = [0-9]*' include/L_Simulation_Settings/version.hpp | grep -o '[0-9]*')
        echo "${major}.${minor}.${patch}"
    else
        print_error "版本头文件不存在"
        exit 1
    fi
}

# 显示版本号
show_version() {
    local version=$(get_current_version)
    echo "VFT版本: $version"
}

# 显示完整版本信息
show_full_info() {
    print_info "VFT项目版本信息"
    echo "=========================================="
    
    # 版本信息
    local version=$(get_current_version)
    echo "版本号: $version"
    
    # 项目信息
    echo "项目名称: Virtual Flight Training System"
    echo "项目简称: VFT"
    echo "描述: 高保真飞行仿真系统"
    
    # 构建信息
    echo "构建时间: $(date)"
    echo "构建平台: $(uname -s) $(uname -m)"
    echo "Git提交: $(git rev-parse --short HEAD 2>/dev/null || echo 'Unknown')"
    echo "Git分支: $(git branch --show-current 2>/dev/null || echo 'Unknown')"
    
    # 版权信息
    echo "版权: Copyright (c) 2024 VFT Team"
    echo "许可证: MIT License"
    
    echo "=========================================="
}

# 显示构建信息
show_build_info() {
    print_info "构建信息"
    echo "=========================================="
    
    # 系统信息
    echo "操作系统: $(uname -s) $(uname -r)"
    echo "架构: $(uname -m)"
    echo "主机名: $(hostname)"
    
    # Git信息
    if [ -d ".git" ]; then
        echo "Git仓库: $(git remote get-url origin 2>/dev/null || echo 'Local')"
        echo "当前分支: $(git branch --show-current)"
        echo "最新提交: $(git log -1 --pretty=format:'%h - %s (%cr)')"
        echo "提交数量: $(git rev-list --count HEAD)"
    else
        echo "Git仓库: 未初始化"
    fi
    
    # 文件信息
    echo "项目大小: $(du -sh . | cut -f1)"
    echo "文件数量: $(find . -type f -name "*.cpp" -o -name "*.hpp" | wc -l) 个源文件"
    
    echo "=========================================="
}

# 检查版本兼容性
check_compatibility() {
    local current_version=$(get_current_version)
    local major=$(echo $current_version | cut -d. -f1)
    local minor=$(echo $current_version | cut -d. -f2)
    
    print_info "版本兼容性检查"
    echo "=========================================="
    echo "当前版本: $current_version"
    echo "主版本号: $major"
    echo "次版本号: $minor"
    
    # 检查语义化版本兼容性
    if [ "$major" -eq 1 ]; then
        print_success "主版本号兼容 (1.x.x)"
    else
        print_warning "主版本号变更 ($major.x.x) - 可能存在不兼容的API修改"
    fi
    
    # 检查依赖项兼容性
    echo ""
    print_info "依赖项检查:"
    
    # 检查编译器版本
    if command -v g++ >/dev/null 2>&1; then
        local gcc_version=$(g++ --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+')
        echo "GCC版本: $gcc_version"
        if [ "$(echo $gcc_version | cut -d. -f1)" -ge 7 ]; then
            print_success "GCC版本满足C++17要求"
        else
            print_warning "GCC版本可能不满足C++17要求"
        fi
    fi
    
    # 检查CMake版本
    if command -v cmake >/dev/null 2>&1; then
        local cmake_version=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+')
        echo "CMake版本: $cmake_version"
        if [ "$(echo $cmake_version | cut -d. -f1)" -ge 3 ] && [ "$(echo $cmake_version | cut -d. -f2)" -ge 15 ]; then
            print_success "CMake版本满足要求 (>= 3.15)"
        else
            print_warning "CMake版本可能不满足要求"
        fi
    fi
    
    echo "=========================================="
}

# 更新版本号
update_version() {
    local new_version=$1
    
    if [ -z "$new_version" ]; then
        print_error "请提供新的版本号"
        echo "使用方法: $0 --update <版本号>"
        exit 1
    fi
    
    # 验证版本号格式
    if [[ ! $new_version =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.-]+)?(\+[a-zA-Z0-9.-]+)?$ ]]; then
        print_error "无效的版本号格式: $new_version"
        print_info "版本号格式应为: X.Y.Z[-预发布标识][+构建元数据]"
        exit 1
    fi
    
    print_info "更新版本号到 $new_version"
    
    # 解析版本号
    local major=$(echo $new_version | cut -d. -f1)
    local minor=$(echo $new_version | cut -d. -f2)
    local patch=$(echo $new_version | cut -d. -f3 | cut -d- -f1 | cut -d+ -f1)
    
    # 更新版本头文件
    if [ -f "include/L_Simulation_Settings/version.hpp" ]; then
        # 备份原文件
        cp include/L_Simulation_Settings/version.hpp include/L_Simulation_Settings/version.hpp.bak
        
        # 更新版本号
        sed -i.bak "s/MAJOR = [0-9]*/MAJOR = $major/" include/L_Simulation_Settings/version.hpp
        sed -i.bak "s/MINOR = [0-9]*/MINOR = $minor/" include/L_Simulation_Settings/version.hpp
        sed -i.bak "s/PATCH = [0-9]*/PATCH = $patch/" include/L_Simulation_Settings/version.hpp
        
        # 清理备份文件
        rm include/L_Simulation_Settings/version.hpp.bak 2>/dev/null || true
        
        print_success "版本头文件已更新"
    else
        print_error "版本头文件不存在"
        exit 1
    fi
    
    # 更新README.md
    if [ -f "README.md" ]; then
        sed -i.bak "s/Version: .*/Version: $new_version/" README.md 2>/dev/null || true
        rm README.md.bak 2>/dev/null || true
        print_success "README.md已更新"
    fi
    
    # 更新CHANGELOG.md
    if [ -f "CHANGELOG.md" ]; then
        local current_date=$(date +%Y-%m-%d)
        sed -i.bak "1i\\
## [$new_version] - $current_date\\
\\
### 新增\\
- 🚀 版本 $new_version 发布\\
\\
### 变更\\
- 更新版本号\\
\\
" CHANGELOG.md
        rm CHANGELOG.md.bak 2>/dev/null || true
        print_success "CHANGELOG.md已更新"
    fi
    
    print_success "版本号更新完成: $new_version"
}

# 主函数
main() {
    case "${1:-}" in
        -v|--version)
            show_version
            ;;
        -i|--info)
            show_full_info
            ;;
        -b|--build)
            show_build_info
            ;;
        -c|--check)
            check_compatibility
            ;;
        -u|--update)
            update_version "$2"
            ;;
        -h|--help|"")
            show_help
            ;;
        *)
            print_error "未知选项: $1"
            show_help
            exit 1
            ;;
    esac
}

# 脚本入口
main "$@" 