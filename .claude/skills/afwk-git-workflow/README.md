# AFwk Git Workflow Skill

自动处理 ability_runtime 项目的 Git 工作流，包括代码提交、推送、PR 创建和 Issue 绑定。

## 功能特性

- ✅ 自动暂存和提交代码
- ✅ 符合项目规范的 commit message 格式
- ✅ 自动创建命名规范的分支
- ✅ 推送到 GitCode 远程仓库
- ✅ 检测并创建/更新 PR
- ✅ 根据项目模板生成 PR 描述
- ✅ 自动创建并关联 Issue
- ✅ 智能检测已存在的 PR/Issue

## 使用方法

### 第一次使用

1. **配置个人信息**：
   复制 `config.template.json` 为 `config.json` 并填写：
   ```json
   {
     "gitcode_token": "你的 GitCode Token",
     "username": "你的用户名",
     "email": "你的邮箱",
     "fork_repo": "你的 fork 仓库地址",
     "upstream_repo": "上游仓库地址"
   }
   ```

2. **获取 GitCode Token**：
   - 访问：https://gitcode.com/-/user_settings/personal_access_tokens
   - 创建新 token，需要 `repo`, `user`, `project` 权限

### 日常使用

当您完成代码修改后，只需说：
- "提交代码"
- "我想 push 代码"
- "帮我创建 PR"
- "合并代码"

skill 会自动执行完整流程。

## 工作流程

```
1. 检查更改 → git status
2. 暂存文件 → git add
3. 创建提交 → git commit
4. 创建分支 → git checkout -b
5. 推送远程 → git push
6. 检查 PR → GitCode API
7. 创建 PR → GitCode API
8. 创建 Issue → GitCode API
9. 关联 PR ← GitCode API
```

## 分支命名规范

格式：`master-MMDD-简短描述`

示例：
- `master-0322-fix-null-pointer`
- `master-0322-add-lifecycle-callback`
- `master-0322-refactor-mission-list`

## Commit Message 格式

```
<简短描述>

Co-Authored-By: Agent
Signed-off-by: duansizhao <duansizhao@huawei.com>
```

类型前缀：
- `feat:` 新功能
- `fix:` Bug 修复
- `refactor:` 重构
- `docs:` 文档更新
- `test:` 测试相关
- `chore:` 构建/工具相关

## 辅助脚本

位于 `scripts/` 目录：

- `check_pr.sh` - 检查 PR 是否已存在
- `create_pr.sh` - 创建新 PR
- `create_issue.sh` - 创建 Issue 并关联 PR
- `detect_target_branch.sh` - 检测目标分支

## API 文档

- [创建 PR](https://docs.gitcode.com/docs/apis/post-api-v-5-repos-owner-repo-pulls)
- [更新 PR](https://docs.gitcode.com/docs/apis/patch-api-v-5-repos-owner-repo-pulls-number)
- [创建 Issue](https://docs.gitcode.com/docs/apis/post-api-v-5-repos-owner-issues)

## 注意事项

1. **安全**：`config.json` 包含敏感信息，不会被提交到 Git
2. **模板**：PR 描述使用项目根目录的 `PULL_REQUEST_TEMPLATE.zh-CN.md`
3. **排除项**：TDD、XTS、L0 测试暂不作为检查项
4. **目标分支**：自动检测，通常为 `master`

## 故障排除

### Token 无效
```
错误：Authentication Failed (401)
解决：检查 config.json 中的 gitcode_token 是否正确
```

### 分支已存在
```
错误：branch already exists
解决：skill 会提示使用现有分支或创建新分支
```

### PR 已存在
```
提示：PR #12345 已存在
解决：可选择更新现有 PR 或跳过创建
```

## 示例场景

### 场景 1：修复 Bug
```
用户：修复了 ability_manager_service 的空指针问题，想提交

Skill 执行：
1. 检测到 .cpp 文件修改
2. 生成 commit: "fix: 修复 ability 启动时的空指针问题"
3. 创建分支: master-0322-fix-null-pointer
4. 推送并创建 PR
5. 自动关联 Issue
```

### 场景 2：添加新功能
```
用户：添加了新的 UI 扩展组件，需要合并

Skill 执行：
1. 分析新增的 extension 相关代码
2. 生成 commit: "feat: 添加 UIExtension 组件支持"
3. 创建分支: master-0322-add-ui-extension
4. 生成详细的 PR 描述
5. 完成完整流程
```

## 技术细节

- **语言**：Bash scripts + GitCode API
- **依赖**：git, curl, jq
- **平台**：Linux, macOS, Windows (Git Bash)
- **配置存储**：`.claude/skills/afwk-git-workflow/config.json`
