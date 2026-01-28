# 提交类型参考

## 主要类型

| 类型 | 描述 | 示例 |
|------|-------------|---------|
| `feat` | 新功能 | `feat(cart): 添加结账按钮` |
| `fix` | Bug 修复 | `fix(login): 修正密码验证` |
| `docs` | 仅文档变更 | `docs(readme): 更新安装步骤` |
| `style` | 格式化、缺少分号等 | `style(api): 使用 prettier 格式化` |
| `refactor` | 既不修复 bug 也不添加功能的代码变更 | `refactor(auth): 简化 token 逻辑` |
| `perf` | 性能改进 | `perf(query): 添加数据库索引` |
| `test` | 添加或更新测试 | `test(api): 添加用户端点测试` |
| `chore` | 维护任务 | `chore(deps): 更新 lodash 到 4.17.21` |

## 附加类型（可选）

| 类型 | 描述 | 示例 |
|------|-------------|---------|
| `build` | 构建系统或外部依赖 | `build(docker): 优化镜像大小` |
| `ci` | CI/CD 配置 | `ci(github): 添加 lint 工作流` |
| `revert` | 回滚之前的提交 | `revert: feat(cart): 添加结账按钮` |

## 作用域示例

作用域应简短且标识代码库区域：

- `auth` - 认证模块
- `api` - API 端点
- `ui` - 用户界面
- `db` - 数据库
- `config` - 配置
- `deps` - 依赖项
- `core` - 核心功能

## 破坏性变更

使用 `!` 在类型/作用域后表示破坏性变更：

```
feat(api)!: 更改响应格式

BREAKING CHANGE: 响应现在使用 camelCase 而非 snake_case。
迁移指南见 docs/migration-v2.md
```

## 多行提交

对于复杂变更，使用正文：

```
feat(search): 实现模糊匹配

添加模糊匹配算法以改进搜索结果。
用户现在即使有拼写错误或部分匹配也能找到项目。

- 实现了 Levenshtein 距离计算
- 添加了可配置的匹配敏感度阈值
- 更新了搜索索引以支持模糊查询

Closes #789
```
