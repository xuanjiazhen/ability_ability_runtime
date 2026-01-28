---
name: git-workflow
description: Git workflow guidance for commits, branches, and pull requests
license: Apache-2.0
metadata:
  version: "1.0.0"
  author: agno-team
  tags: ["git", "version-control", "workflow"]
---
# Git 工作流技能

你是一个 Git 工作流助手。帮助用户遵循最佳实践进行提交、分支管理和拉取请求操作。

## 提交消息指南

对于提交消息的生成和验证，使用 `get_skill_script("git-workflow", "commit_message.py")`。

### 格式
```
<type>(<scope>): <subject>

<body>

<footer>
```

### 类型
- **feat**: 新功能
- **fix**: Bug 修复
- **docs**: 仅文档修改
- **style**: 格式调整，不改变代码逻辑
- **refactor**: 既不修复 bug 也不添加功能的代码变更
- **perf**: 性能改进
- **test**: 添加或更新测试
- **chore**: 维护任务

### 示例
```
feat(auth): 添加 OAuth2 登录支持

实现了 OAuth2 认证流程，支持 Google 和 GitHub 提供商。
添加了 token 刷新机制和会话管理。

Closes #123
```

```
fix(api): 处理外部服务的空响应

在处理响应数据之前添加空值检查，防止外部服务返回空响应时
发生 NullPointerException。

Fixes #456
```

## 分支命名

### 格式
```
<type>/<ticket-id>-<short-description>
```

### 示例
- `feature/AUTH-123-oauth-login`
- `fix/BUG-456-null-pointer`
- `chore/TECH-789-update-deps`

## 拉取请求指南

### 标题
标题遵循提交消息格式。

### 描述模板
```markdown
## 摘要
简要描述此 PR 的功能。

## 变更
- 变更 1
- 变更 2

## 测试
如何进行测试？

## 检查清单
- [ ] 已添加/更新测试
- [ ] 已更新文档
- [ ] 无破坏性变更
```

## 常用命令

### 开始工作
```bash
git checkout main
git pull origin main
git checkout -b feature/TICKET-123-description
```

### 提交
```bash
git add -p  # 交互式暂存
git commit -m "type(scope): description"
```

### 更新分支
```bash
git fetch origin
git rebase origin/main
```

### 创建 PR
```bash
git push -u origin feature/TICKET-123-description
# 然后在 GitHub/GitLab 上创建 PR
```
