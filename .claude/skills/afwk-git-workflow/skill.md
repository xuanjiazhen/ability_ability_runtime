---
name: afwk-git-workflow
description: ability_runtime 项目的完整 Git 工作流，包括代码提交、推送、PR 创建和 Issue 关联。当用户需要提交代码、推送到远程、创建 PR 或管理 GitCode 仓库中的 Issue 时使用。
---

# AFwk Git 工作流

OpenHarmony ability_runtime 项目的自动化 Git 工作流。此 Skill 处理从代码提交到 GitCode 上创建 PR 和 Issue 的完整流程。

## 使用场景

当用户出现以下情况时使用此 Skill：
- 需要提交并推送代码更改
- 需要创建符合项目规范的 Pull Request
- 需要 Issue 与 PR 关联
- 提到 commit、push、PR 或 merge 等 GitCode 相关工作
- 说"提交代码"、"创建 PR"、"push 代码"、"merge request"等

## 前置条件

此 Skill 需要以下配置（如未设置将提示）：

1. **GitCode Token**: 用于 API 认证的个人访问令牌
2. **用户信息**:
   - 用户名: `dsz2025` (GitCode 用户名)
   - 邮箱: `duansizhao@huawei.com` (提交者邮箱)
3. **仓库 URL**:
   - Fork 仓库: `https://gitcode.com/dsz2025/ability_ability_runtime.git`
   - 上游仓库: `https://gitcode.com/openharmony/ability_ability_runtime.git`

## 工作流步骤

### 步骤 1：保存并暂存更改

检查当前 git 状态并暂存所有更改：

```bash
git status
git add .
```

**询问用户**："是否需要暂存所有更改？如需选择特定文件，请告诉我文件名。"

### 步骤 2：创建提交

按照项目规范生成 commit message：

**格式**：
```
<简要变更说明>

Co-Authored-By: Agent
Signed-off-by: duansizhao <duansizhao@huawei.com>
```

**流程**：
1. 使用 `git diff --cached` 分析已暂存的更改
2. 生成简要说明（通常 50 字符以内）
3. 执行提交：
   ```bash
   git commit -m "<message>"
   ```

**示例**：
- `feat: add new ability lifecycle callback`
- `fix: resolve memory leak in extension manager`
- `refactor: optimize mission list initialization`

### 步骤 3：推送到远程

**分支命名规范**：`master-MMDD-<简要描述>`

**流程**：
1. 获取当前日期，格式为 MMDD
2. 从 commit message 生成简要分支描述（小写，空格用连字符）
3. 检查分支是否已存在
4. 如果分支不存在，创建并推送：
   ```bash
   git checkout -b master-MMDD-<brief-desc>
   git push -u origin master-MMDD-<brief-desc>
   ```
5. 如果分支已存在，推送到现有分支：
   ```bash
   git checkout master-MMDD-<brief-desc>
   git merge master-0321-update_copyright  # 或当前分支
   git push
   ```

**示例分支**：
- `master-0322-add-lifecycle-callback`
- `master-0322-fix-memory-leak`
- `master-0322-refactor-mission-list`

### 步骤 4：检查现有的 PR 和 Issue

**检查 PR API**：`GET https://api.gitcode.com/v5/repos/openharmony/ability_ability_runtime/pulls?state=open&head=<用户名>:<分支名>`

**检查 Issue API**：`GET https://api.gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/issues?state=open&creator=<用户名>&search=<commit标题>`

**响应检查**：
- 如果 PR 存在：询问用户是否要更新现有 PR
- 如果 PR 不存在：继续创建新 PR
- 如果 Issue 存在：使用现有的 issue 编号
- 如果 Issue 不存在：先创建新 issue

**重要**：在创建 PR 之前先创建 Issue，以便 PR 描述可以包含 issue 编号。

### 步骤 5：创建 Issue（如果不存在）

**创建 Issue API**：
```
POST https://api.gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/issues
```

**请求体**：
```json
{
  "title": "<从 commit message 自动生成>",
  "body": "<简化模板>",
  "assignee": "dsz2025"
}
```

**流程**：
1. 从 commit message 生成 issue 标题
2. 检查是否已存在类似的 issue
3. 如果没有现有 issue，创建新的
4. 保存 issue 编号用于 PR 描述

**重要 - 中文字符编码**：
创建包含中文内容的 Issue 时，使用正确的 UTF-8 编码：
```bash
# 1. 创建 UTF-8 编码的 JSON 文件
cat > /tmp/issue.json << 'EOF'
{
  "title": "docs: 添加文档",
  "body": "## 问题描述\n详细描述",
  "assignee": "dsz2025"
}
EOF

# 2. 使用 --data-binary 配合 charset=utf-8
curl -s -X POST \
  "https://api.gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/issues?access_token=${GITCODE_TOKEN}" \
  -H "Content-Type: application/json; charset=utf-8" \
  --data-binary @/tmp/issue.json
```

### 步骤 6：生成 PR 描述

**模板位置**：项目根目录的 `.gitee/PULL_REQUEST_TEMPLATE.zh-CN.md`

**流程**：
1. 从 `.gitee/PULL_REQUEST_TEMPLATE.zh-CN.md` 读取 PR 模板文件
2. 根据 commit 信息填写模板
3. 保留所有部分，包括稳定性检查、安全检查、TDD、XTS、L0
4. 用 commit 详情填写 Description 字段
5. 对中文字符使用正确的 UTF-8 编码

**重要 - 实际模板结构**：
真实的项目模板（`PULL_REQUEST_TEMPLATE.zh-CN.md`）具有以下确切结构：
```markdown
**IssueNo**: #<issue-number>

**Description**:
<变更简要说明>

**稳定性自检：**
| 自检项 | 自检结果 |
| ... (保留全部 16 项稳定性检查) |

**安全编码自检：**
| 自检项 | 自检结果 |
| ... (保留全部 22 项安全检查) |

**TDD Result**:
<填写或标记为 不涉及>

**XTS Result**:
<fill in or mark as 不涉及>

### 是否已执行L0用例
- [ ] 已验证
- [x] 不涉及。如不涉及，请写明理由：测试场景不涉及L0用例
```

**关键编码要求**：
1. 对于中文 PR/Issue 标题：先使用 ASCII/英文，然后通过 Web UI 编辑
2. 或使用 `--data-urlencode` 正确编码中文字符
3. 切勿在 curl 中使用 `-d` 传递内联中文 JSON
4. 始终先将 JSON 写入文件，然后使用 `--data-binary @file`

正确的编码示例：
```bash
# 创建 JSON 文件
cat > /tmp/pr_body.json << 'EOF'
{
  "title": "feat: add test file",
  "head": "dsz2025:branch-name",
  "base": "master",
  "body": "**IssueNo**: #12345\n\n**Description**:\nAdd test file..."
}
EOF

# 使用正确的编码创建 PR
curl -X POST \
  "https://api.gitcode.com/api/v5/repos/owner/repo/pulls?access_token=xxx" \
  -H "Content-Type: application/json; charset=utf-8" \
  --data-binary @/tmp/pr_body.json
```

### 步骤 7：创建或更新 PR

**创建 PR API**：
```
POST https://api.gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/pulls
```

**关键 - 使用文件方式配合正确编码**：

步骤 1：创建 PR body JSON 文件（UTF-8 编码）：
```bash
cat > /tmp/create_pr.json << 'EOF'
{
  "title": "feat: add test file",
  "head": "dsz2025:master-0322-test-feature",
  "base": "master",
  "body": "**IssueNo**: #12345\n\n**Description**:\n\nAdd test file for feature validation.\n\n**稳定性自检：**\n| 自检项 | 自检结果 |\n| ------------------------------------------------------------ | -------- |\n| 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发 | 不涉及 |\n| 成员变量进行赋值或创建需要排查并发 | 不涉及 |\n| 谨慎在lambda表达式中使用引用捕获 | 不涉及 |\n| 谨慎在未经拷贝的情况下使用外部传入的string、C字符串 | 不涉及 |\n| map\\vector\\list\\set等stl模板类使用时需要排查并发 | 不涉及 |\n| 谨慎考虑加锁范围 | 不涉及 |\n| 在IPC通信中谨慎使用同步通信方式 | 不涉及 |\n| 禁止传递this指针至其他模块或线程（特别是eventhandler任务） | 不涉及 |\n| 禁止将外部传入的裸指针在内部直接构造智能指针 | 不涉及 |\n| 禁止多个独立创建的智能指针管理同一地址 | 不涉及 |\n| 禁止在析构函数中抛异步任务 | 不涉及 |\n| 禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁 | 不涉及 |\n| 禁止在对外接口中未经判空直接使用外部传入的指针 | 不涉及 |\n| 禁止接口返回局部变量引用 | 不涉及 |\n| 禁止在信号函数中加锁 | 不涉及 |\n| 禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作 | 不涉及 |\n| 禁止将同一个cpp编译在不同的so中 | 不涉及 |\n\n**安全编码自检：**\n| 自检项 | 自检结果 |\n| -------------------------------------------------------------- | -------- |\n| 裸指针避免通过隐式转换构造为sptr | 不涉及 |\n| json对象在取值之前必须先判断类型，避免类型不匹配 | 不涉及 |\n| 序列化时必须对传入的数组大小进行校验，避免出现超大数组 | 不涉及 |\n| 避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型 | 不涉及 |\n| 外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验 | 不涉及 |\n| 指针变量、表示资源描述符的变量、bool变量必须赋初值 | 不涉及 |\n| readParcelable获取的对象使用前需要判空 | 不涉及 |\n| 分配和释放内存的函数需要成对出现 | 不涉及 |\n| 申请内存后异常退出前需要及时进行内存释放 | 不涉及 |\n| 内存申请前必须对内存大小进行合法性校验 | 不涉及 |\n| 内存分配后必须判断是否成功 | 不涉及 |\n| 禁止使用realloc、alloca函数 | 不涉及 |\n| 禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰 | 不涉及 |\n| 禁止打印内存地址 | 不涉及 |\n| 整数之间运算时必须严格检查，确保不会出现溢出、反转、除0 | 不涉及 |\n| 禁止对有符号整数进行位操作符运算 | 不涉及 |\n| 禁止对指针进行逻辑或位运算 | 不涉及 |\n| 循环次数如果收外部数据控制，需要检验其合法性 | 不涉及 |\n| 禁止使用内存操作类危险函数，需要使用安全函数 | 不涉及 |\n| 谨慎使用不可重入函数 | 不涉及 |\n| 必须检查安全函数的返回值，并进行正确处理 | 不涉及 |\n| 禁止仅通过TokenType类型判断绕过权限校验 | 不涉及 |\n\n**TDD Result**:\n不涉及\n\n**XTS Result**:\n不涉及\n\n### 是否已执行L0用例\n- [x] 不涉及。如不涉及，请写明理由：测试场景不涉及L0用例",
  "maintainer_can_modify": true
}
EOF
```

步骤 2：使用文件创建 PR：
```bash
curl -s -X POST \
  "https://api.gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/pulls?access_token=${GITCODE_TOKEN}" \
  -H "Content-Type: application/json; charset=utf-8" \
  --data-binary @/tmp/create_pr.json
```

**关键要求**：
1. 标题：使用英文或简单中文避免编码问题
2. Body 必须在最开始包含 `**IssueNo**: #<issue-number>`
3. 保留所有稳定性和安全检查项（不要删除）
4. 将不相关项目标记为"不涉及"
5. 使用 `--data-binary @file`，切勿使用 `-d` 传递内联 JSON
6. 添加 `charset=utf-8` 到 Content-Type 头

**更新 PR API**（如果 PR 存在）：
```
PATCH https://api.gitcode.com/v5/repos/openharmony/ability_ability_runtime/pulls/<pr-number>
```

**目标分支检测**：
- 如果当前分支名以 `master-` 开头，目标是 `master`
- 否则，使用 `git remote show origin` 检测默认分支

### 步骤 8：验证关联

创建 PR 和 Issue 后，验证关联关系：

1. 检查 PR 描述包含 `**IssueNo**: #<issue-number>`
2. 检查 Issue 描述包含 PR 引用
3. 可选：在 Issue 中添加提及 PR 的评论

**注意**：当满足以下条件时，GitCode 自动关联 PR 和 Issue：
- PR 描述包含 `#<issue-number>`
- Issue body 包含 PR 引用

## 错误处理

**常见错误及解决方案**：

1. **认证失败** (401):
   - 验证 GitCode token 是否有效
   - 提示用户更新 token

2. **分支已存在**:
   - 询问用户是否使用现有分支
   - 提供创建不同名称分支的选项

3. **PR 已存在**:
   - 提供更新现有 PR 描述的选项
   - 或跳过 PR 创建

4. **合并冲突**:
   - 告知用户存在冲突
   - 提供手动解决说明

5. **API 速率限制**:
   - 等待后重试
   - 告知用户延迟信息

6. **中文字符编码**:
   - **最佳方案**: 标题使用英文，创建后通过 Web UI 编辑中文内容
   - **替代方案**: 使用文件方式配合正确的 UTF-8 编码
   - 添加 `charset=utf-8` 到 Content-Type 头
   - 先将 JSON 写入文件，使用 UTF-8 编码
   - 使用 `--data-binary @file`，切勿使用 `-d` 传递内联 JSON
   - 错误: `curl -d '{"title":"中文标题"}'` ❌
   - 正确:
     ```bash
     # 步骤 1：将 JSON 写入文件
     cat > /tmp/request.json << 'EOF'
     {
       "title": "English title to avoid encoding issues",
       "body": "**IssueNo**: #12345\n\n**Description**:\n中文描述内容..."
     }
     EOF

     # 步骤 2：使用文件配合 --data-binary
     curl -X POST \
       "https://api.gitcode.com/api/v5/repos/owner/repo/issues?access_token=xxx" \
       -H "Content-Type: application/json; charset=utf-8" \
       --data-binary @/tmp/request.json
     ```

## 配置存储

用户配置存储在 `.claude/skills/afwk-git-workflow/config.json`：

```json
{
  "gitcode_token": "kzBHGjyB9VyiY1CV8bGtnYJg",
  "username": "duansizhao",
  "email": "duansizhao@huawei.com",
  "fork_repo": "https://gitcode.com/dsz2025/ability_ability_runtime.git",
  "upstream_repo": "https://gitcode.com/openharmony/ability_ability_runtime.git"
}
```

## 交流风格

执行此工作流时：
1. **明确每个步骤**: 执行前告知用户正在进行的操作
2. **请求确认**: 在 git push、创建 PR 和 Issue 前请求确认
3. **显示进度**: 显示 API 响应和分支名称
4. **优雅处理错误**: 提供清晰的错误信息和恢复选项
5. **使用中文**: 由于这是中文项目，使用中文交流

## 示例工作流

```
用户: 我想提交代码

助手: 好的，我来帮你提交代码。

[步骤 1] 检查当前更改...
发现 3 个文件已修改：ability_manager_service.cpp, ability_context.cpp

[步骤 2] 生成 commit message...
根据更改内容，建议的 commit message：
"fix: resolve null pointer issue in ability startup"
（包含 Co-Authored-By: Agent 和 Signed-off-by 字段）

是否使用此 commit message？(y/n)

[步骤 3] 创建分支并推送...
分支名：master-0322-fix-null-pointer
正在推送到 fork/master-0322-fix-null-pointer...

[步骤 4] 检查 Issue...
未发现相关 Issue，创建新 Issue
Issue #67890 已创建

[步骤 5] 生成 PR 描述...
使用 PULL_REQUEST_TEMPLATE.zh-CN.md 模板
包含 IssueNo: #67890
包含完整的稳定性自检表（16项）
包含完整的安全编码自检表（22项）
TDD/XTS/L0 标记为"不涉及"

[步骤 6] 创建 PR 文件（UTF-8编码）...
JSON 已写入 /tmp/create_pr.json

[步骤 7] 创建 PR...
使用 --data-binary @/tmp/create_pr.json 发送请求
PR #12345 已创建：https://gitcode.com/openharmony/ability_ability_runtime/pull/12345

[步骤 8] 验证关联...
PR 描述包含 **IssueNo**: #67890
Issue 和 PR 已自动关联
```

## 注意事项

- 此 Skill 专为 ability_runtime 项目工作流设计
- 分支命名遵循项目规范：`master-MMDD-description`
- 始终使用 Co-Authored-By: Agent 和 Signed-off-by 署名
- **关键**: PR 描述必须严格遵循 `.gitee/PULL_REQUEST_TEMPLATE.zh-CN.md`
- **关键**: 保留所有 16 项稳定性检查和所有 22 项安全检查
- **关键**: 将不相关项目标记为"不涉及"，切勿删除
- **关键**: 使用英文标题或文件方式 JSON 避免编码问题
- **关键**: 始终使用 `--data-binary @file` 处理包含中文的 curl 请求
- 当 PR 描述包含 `**IssueNo**: #<number>` 时，Issue 自动关联到 PR
- 不适用时，TDD、XTS 和 L0 部分应标记为"不涉及"

## 快速参考：PR 描述模板

```markdown
**IssueNo**: #<issue-number>

**Description**:
<brief change description>

**稳定性自检：**
| 自检项 | 自检结果 |
| ------------------------------------------------------------ | -------- |
| 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发 | 不涉及 |
... (keep all 16 items) ...

**安全编码自检：**
| 自检项 | 自检结果 |
| -------------------------------------------------------------- | -------- |
| 裸指针避免通过隐式转换构造为sptr | 不涉及 |
... (keep all 22 items) ...

**TDD Result**:
不涉及

**XTS Result**:
不涉及

### 是否已执行L0用例
- [x] 不涉及。如不涉及，请写明理由：测试场景不涉及L0用例
```
