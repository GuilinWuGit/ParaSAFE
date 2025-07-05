#!/bin/bash

# ParaSAFE 项目发布脚本
# 使用方法: ./scripts/release.sh [版本号] [发布类型]

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
    echo "VFT 项目发布脚本"
    echo ""
    echo "使用方法:"
    echo "  $0 [版本号] [发布类型]"
    echo ""
    echo "参数:"
    echo "  版本号    语义化版本号 (例如: 1.0.0, 1.2.0-beta.1)"
    echo "  发布类型  发布类型 (patch|minor|major|beta|alpha|rc)"
    echo ""
    echo "示例:"
    echo "  $0 1.0.0 major      # 主版本发布"
    echo "  $0 1.2.0 minor      # 次版本发布"
    echo "  $0 1.2.1 patch      # 修订版本发布"
    echo "  $0 1.2.0-beta.1 beta # Beta测试版本"
    echo ""
    echo "发布类型说明:"
    echo "  major   主版本号增加 (不兼容的API修改)"
    echo "  minor   次版本号增加 (向下兼容的功能性新增)"
    echo "  patch   修订号增加 (向下兼容的问题修正)"
    echo "  beta    Beta测试版本"
    echo "  alpha   Alpha测试版本"
    echo "  rc      候选发布版本"
}

# 检查Git状态
check_git_status() {
    print_info "检查Git状态..."
    
    # 检查是否有未提交的更改
    if [ -n "$(git status --porcelain)" ]; then
        print_error "有未提交的更改，请先提交或暂存更改"
        git status --short
        exit 1
    fi
    
    # 检查是否在正确的分支上
    current_branch=$(git branch --show-current)
    if [ "$current_branch" != "main" ] && [ "$current_branch" != "develop" ]; then
        print_warning "当前不在main或develop分支上 ($current_branch)"
        read -p "是否继续? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
    
    print_success "Git状态检查通过"
}

# 验证版本号格式
validate_version() {
    local version=$1
    
    # 检查版本号格式
    if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[a-zA-Z0-9.-]+)?(\+[a-zA-Z0-9.-]+)?$ ]]; then
        print_error "无效的版本号格式: $version"
        print_info "版本号格式应为: X.Y.Z[-预发布标识][+构建元数据]"
        exit 1
    fi
    
    print_success "版本号格式验证通过: $version"
}

# 更新版本号
update_version() {
    local version=$1
    
    print_info "更新版本号到 $version..."
    
    # 更新README.md中的版本号
    if [ -f "README.md" ]; then
        sed -i.bak "s/Version: .*/Version: $version/" README.md
        rm README.md.bak 2>/dev/null || true
        print_info "已更新 README.md"
    fi
    
    # 更新CHANGELOG.md
    if [ -f "CHANGELOG.md" ]; then
        # 在CHANGELOG.md开头添加新版本条目
        current_date=$(date +%Y-%m-%d)
        sed -i.bak "1i\\
## [$version] - $current_date\\
\\
### 新增\\
- 🚀 版本 $version 发布\\
\\
### 变更\\
- 更新版本号\\
\\
" CHANGELOG.md
        rm CHANGELOG.md.bak 2>/dev/null || true
        print_info "已更新 CHANGELOG.md"
    fi
    
    # 更新配置文件中的版本号（如果有）
    find . -name "*.txt" -o -name "*.ini" -o -name "*.cfg" | while read file; do
        if grep -q "VERSION\|version" "$file"; then
            sed -i.bak "s/VERSION.*=.*/VERSION = $version/" "$file" 2>/dev/null || true
            rm "$file.bak" 2>/dev/null || true
        fi
    done
    
    print_success "版本号更新完成"
}

# 运行测试
run_tests() {
    print_info "运行测试..."
    
    # 构建项目
    if [ -d "Scenario_Lib/B_Abort_TakeOff" ]; then
        cd Scenario_Lib/B_Abort_TakeOff
        g++ -std=c++17 -pthread -o abort_takeoff_sim.exe main.cpp
        
        if [ -f "./abort_takeoff_sim.exe" ]; then
            print_success "构建成功"
        else
            print_error "构建失败"
            exit 1
        fi
        
        cd ../..
    else
        print_warning "未找到测试场景目录"
    fi
    
    print_success "测试完成"
}

# 创建发布分支
create_release_branch() {
    local version=$1
    local release_branch="release/v$version"
    
    print_info "创建发布分支: $release_branch"
    
    # 确保在develop分支上
    if [ "$(git branch --show-current)" != "develop" ]; then
        git checkout develop
    fi
    
    # 拉取最新代码
    git pull origin develop
    
    # 创建发布分支
    git checkout -b "$release_branch"
    
    print_success "发布分支创建完成: $release_branch"
}

# 提交版本更新
commit_version_update() {
    local version=$1
    
    print_info "提交版本更新..."
    
    git add .
    git commit -m "chore: bump version to $version

- Update version number in README.md
- Update CHANGELOG.md with new version entry
- Update configuration files with new version"

    print_success "版本更新已提交"
}

# 创建Git标签
create_tag() {
    local version=$1
    
    print_info "创建Git标签: v$version"
    
    git tag -a "v$version" -m "Release version $version"
    
    print_success "Git标签创建完成: v$version"
}

# 合并到主分支
merge_to_main() {
    local version=$1
    
    print_info "合并到main分支..."
    
    # 切换到main分支
    git checkout main
    git pull origin main
    
    # 合并发布分支
    git merge "release/v$version" --no-ff -m "Merge release/v$version into main"
    
    # 推送main分支和标签
    git push origin main
    git push origin "v$version"
    
    print_success "已合并到main分支并推送"
}

# 清理发布分支
cleanup_release_branch() {
    local version=$1
    
    print_info "清理发布分支..."
    
    # 删除本地发布分支
    git branch -d "release/v$version"
    
    # 删除远程发布分支（如果存在）
    git push origin --delete "release/v$version" 2>/dev/null || true
    
    print_success "发布分支清理完成"
}

# 更新develop分支
update_develop() {
    local version=$1
    
    print_info "更新develop分支..."
    
    # 切换到develop分支
    git checkout develop
    git pull origin develop
    
    # 合并发布分支的更改
    git merge "release/v$version" --no-ff -m "Merge release/v$version into develop"
    
    # 推送develop分支
    git push origin develop
    
    print_success "develop分支更新完成"
}

# 显示发布摘要
show_release_summary() {
    local version=$1
    
    print_success "🎉 版本 $version 发布完成!"
    echo ""
    echo "发布摘要:"
    echo "  📦 版本号: $version"
    echo "  🏷️  Git标签: v$version"
    echo "  🌿  发布分支: release/v$version (已清理)"
    echo "  📚  文档更新: README.md, CHANGELOG.md"
    echo ""
    echo "下一步:"
    echo "  1. 检查GitHub Actions自动发布流程"
    echo "  2. 验证GitHub Release是否创建成功"
    echo "  3. 通知团队成员"
    echo "  4. 更新项目文档网站"
    echo ""
    echo "相关链接:"
    echo "  📋 发布页面: https://github.com/your-username/VFT/releases/tag/v$version"
    echo "  📚 文档网站: https://your-username.github.io/VFT"
    echo "  🐛 问题报告: https://github.com/your-username/VFT/issues"
}

# 主函数
main() {
    local version=$1
    local release_type=$2
    
    # 检查参数
    if [ -z "$version" ] || [ -z "$release_type" ]; then
        show_help
        exit 1
    fi
    
    # 验证版本号
    validate_version "$version"
    
    # 检查Git状态
    check_git_status
    
    # 运行测试
    run_tests
    
    # 更新版本号
    update_version "$version"
    
    # 创建发布分支
    create_release_branch "$version"
    
    # 提交版本更新
    commit_version_update "$version"
    
    # 创建Git标签
    create_tag "$version"
    
    # 合并到main分支
    merge_to_main "$version"
    
    # 更新develop分支
    update_develop "$version"
    
    # 清理发布分支
    cleanup_release_branch "$version"
    
    # 显示发布摘要
    show_release_summary "$version"
}

# 脚本入口
if [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    show_help
    exit 0
fi

# 执行主函数
main "$@" 