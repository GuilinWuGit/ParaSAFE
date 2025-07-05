# 版本更新控制指南

## 📋 目录

- [版本号规范](#版本号规范)
- [分支策略](#分支策略)
- [发布流程](#发布流程)
- [自动化工作流](#自动化工作流)
- [版本标签管理](#版本标签管理)
- [回滚策略](#回滚策略)
- [最佳实践](#最佳实践)

## 🏷️ 版本号规范

### 语义化版本控制 (Semantic Versioning)

我们采用 [SemVer 2.0.0](https://semver.org/lang/zh-CN/) 规范：

```
主版本号.次版本号.修订号[-预发布标识][+构建元数据]
```

**版本号组成**：
- **主版本号 (Major)**: 不兼容的API修改
- **次版本号 (Minor)**: 向下兼容的功能性新增
- **修订号 (Patch)**: 向下兼容的问题修正
- **预发布标识**: alpha, beta, rc 等
- **构建元数据**: 构建时间、提交哈希等

**示例**：
```
1.0.0          # 第一个正式版本
1.0.1          # 修复bug
1.1.0          # 新增功能
2.0.0          # 重大更新，不兼容
1.2.0-beta.1   # 预发布版本
1.2.0+20231201 # 带构建元数据
```

### 版本类型

| 类型 | 说明 | 示例 |
|------|------|------|
| 开发版 | 日常开发版本 | `1.2.0-dev.1` |
| Alpha版 | 内部测试版本 | `1.2.0-alpha.1` |
| Beta版 | 公开测试版本 | `1.2.0-beta.1` |
| RC版 | 候选发布版本 | `1.2.0-rc.1` |
| 正式版 | 稳定发布版本 | `1.2.0` |
| 热修复版 | 紧急修复版本 | `1.2.1` |

## 🌿 分支策略

### Git Flow 工作流

```
main (master)
├── develop
│   ├── feature/新功能分支
│   ├── feature/控制器优化
│   └── feature/场景扩展
├── release/版本分支
└── hotfix/紧急修复分支
```

### 分支说明

#### 主要分支

- **main**: 生产环境代码，只接受来自release或hotfix分支的合并
- **develop**: 开发主分支，包含最新的开发功能

#### 功能分支

- **feature/***: 新功能开发分支
  ```bash
  git checkout -b feature/landing-gear-controller
  git checkout -b feature/abort-takeoff-optimization
  ```

#### 发布分支

- **release/***: 版本发布准备分支
  ```bash
  git checkout -b release/v1.2.0
  ```

#### 热修复分支

- **hotfix/***: 生产环境紧急修复
  ```bash
  git checkout -b hotfix/critical-bug-fix
  ```

### 分支命名规范

```bash
# 功能分支
feature/controller-name
feature/scenario-name
feature/performance-optimization

# 发布分支
release/v1.2.0
release/v2.0.0-beta

# 热修复分支
hotfix/critical-thread-safety-issue
hotfix/memory-leak-fix

# 文档分支
docs/installation-guide-update
docs/api-reference-completion
```

## 🚀 发布流程

### 1. 功能开发阶段

```bash
# 1. 从develop分支创建功能分支
git checkout develop
git pull origin develop
git checkout -b feature/new-controller

# 2. 开发功能
# ... 编写代码 ...

# 3. 提交代码
git add .
git commit -m "feat: add new landing gear controller

- Implement gear extension/retraction logic
- Add configuration parameters
- Include unit tests
- Update documentation"

# 4. 推送到远程
git push origin feature/new-controller

# 5. 创建Pull Request
# 在GitHub上创建PR，目标分支为develop
```

### 2. 代码审查阶段

- **代码审查检查项**：
  - [ ] 代码符合项目规范
  - [ ] 功能测试通过
  - [ ] 单元测试覆盖
  - [ ] 文档更新完整
  - [ ] 性能影响评估
  - [ ] 向后兼容性检查

### 3. 发布准备阶段

```bash
# 1. 创建发布分支
git checkout develop
git pull origin develop
git checkout -b release/v1.2.0

# 2. 更新版本号
# 修改相关文件中的版本信息

# 3. 更新CHANGELOG.md
# 记录本次发布的所有变更

# 4. 最终测试
# 运行完整的测试套件

# 5. 合并到main和develop
git checkout main
git merge release/v1.2.0
git tag -a v1.2.0 -m "Release version 1.2.0"
git push origin main --tags

git checkout develop
git merge release/v1.2.0
git push origin develop

# 6. 删除发布分支
git branch -d release/v1.2.0
git push origin --delete release/v1.2.0
```

### 4. 发布后阶段

- 创建GitHub Release
- 更新文档网站
- 通知用户和贡献者
- 监控生产环境

## ⚙️ 自动化工作流

### CI/CD 流水线

#### 1. 代码质量检查

```yaml
# .github/workflows/ci.yml
name: Continuous Integration

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup C++
      uses: actions/setup-node@v3
      
    - name: Build
      run: |
        cd Scenario_Lib/B_Abort_TakeOff
        g++ -std=c++17 -pthread -o abort_takeoff_sim.exe main.cpp
        
    - name: Test
      run: |
        cd Scenario_Lib/B_Abort_TakeOff
        ./abort_takeoff_sim.exe --test
        
    - name: Code Quality
      run: |
        # 运行代码质量检查工具
        # cppcheck, clang-tidy等
```

#### 2. 自动化发布

```yaml
# .github/workflows/release.yml
name: Release

on:
  push:
    tags:
      - 'v*'

jobs:
  release:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Build Release
      run: |
        # 构建发布版本
        make release
        
    - name: Create Release
      uses: actions/create-release@v1
      env:
        GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
      with:
        tag_name: ${{ github.ref }}
        release_name: Release ${{ github.ref }}
        body: |
          ## What's Changed
          ${{ github.event.head_commit.message }}
          
          ## Download
          Download the latest release from the assets below.
        draft: false
        prerelease: false
```

#### 3. 自动化测试

```yaml
# .github/workflows/test.yml
name: Automated Testing

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main, develop ]

jobs:
  test:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        scenario: [abort-takeoff, landing, cruise]
        
    steps:
    - uses: actions/checkout@v3
    
    - name: Test Scenario
      run: |
        cd Scenario_Lib/B_Abort_TakeOff
        ./abort_takeoff_sim.exe --test --scenario=${{ matrix.scenario }}
```

## 🏷️ 版本标签管理

### 创建版本标签

```bash
# 轻量标签
git tag v1.2.0

# 带注释的标签
git tag -a v1.2.0 -m "Release version 1.2.0"

# 推送标签
git push origin v1.2.0

# 推送所有标签
git push origin --tags
```

### 标签命名规范

```bash
# 正式版本
v1.0.0
v1.2.0
v2.0.0

# 预发布版本
v1.2.0-alpha.1
v1.2.0-beta.1
v1.2.0-rc.1

# 开发版本
v1.2.0-dev.1
```

### 管理标签

```bash
# 查看所有标签
git tag -l

# 查看标签详情
git show v1.2.0

# 删除本地标签
git tag -d v1.2.0

# 删除远程标签
git push origin --delete v1.2.0
```

## 🔄 回滚策略

### 代码回滚

```bash
# 1. 回滚到特定提交
git revert <commit-hash>

# 2. 回滚到特定版本
git revert v1.2.0

# 3. 强制回滚（危险操作）
git reset --hard <commit-hash>
git push --force origin main
```

### 发布回滚

```bash
# 1. 创建回滚分支
git checkout -b hotfix/rollback-v1.2.0

# 2. 回滚到上一个稳定版本
git revert v1.2.0

# 3. 创建新的热修复版本
git tag -a v1.2.1 -m "Hotfix: rollback v1.2.0"

# 4. 发布回滚版本
git push origin hotfix/rollback-v1.2.0
git push origin v1.2.1
```

### 数据库回滚

```bash
# 如果有数据库变更，需要相应的回滚脚本
# 示例：回滚配置文件变更
git checkout v1.1.0 -- config/simulation_config.txt
```

## 📋 最佳实践

### 1. 提交信息规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 格式：

```bash
# 功能提交
feat: add new landing gear controller

# 修复提交
fix: resolve thread safety issue in event bus

# 文档提交
docs: update installation guide for Windows

# 重构提交
refactor: optimize controller performance

# 测试提交
test: add unit tests for throttle controller

# 构建提交
chore: update build configuration
```

### 2. 版本发布检查清单

#### 发布前检查
- [ ] 所有测试通过
- [ ] 代码审查完成
- [ ] 文档更新完整
- [ ] 版本号正确更新
- [ ] CHANGELOG.md 更新
- [ ] 依赖项检查
- [ ] 性能测试通过

#### 发布后检查
- [ ] GitHub Release 创建
- [ ] 文档网站更新
- [ ] 用户通知发送
- [ ] 监控系统正常
- [ ] 备份完成

### 3. 分支保护规则

在GitHub仓库设置中配置：

- **main分支保护**：
  - 需要Pull Request审查
  - 需要状态检查通过
  - 禁止强制推送
  - 需要最新代码

- **develop分支保护**：
  - 需要Pull Request审查
  - 需要状态检查通过

### 4. 自动化工具

#### 推荐工具

- **版本管理**: `semantic-release`
- **变更日志**: `conventional-changelog`
- **代码质量**: `cppcheck`, `clang-tidy`
- **测试框架**: `Google Test`, `Catch2`
- **CI/CD**: GitHub Actions

#### 配置文件示例

```json
// .releaserc.json
{
  "branches": ["main"],
  "plugins": [
    "@semantic-release/commit-analyzer",
    "@semantic-release/release-notes-generator",
    "@semantic-release/changelog",
    "@semantic-release/npm",
    "@semantic-release/github"
  ]
}
```

## 📞 支持与帮助

如果您在版本控制过程中遇到问题：

1. **查看文档**: 参考本指南和相关文档
2. **搜索问题**: 在GitHub Issues中搜索类似问题
3. **创建Issue**: 报告新的问题或建议
4. **联系维护者**: 通过GitHub Discussions联系

---

**注意**: 本指南会随着项目发展持续更新，请定期查看最新版本。 