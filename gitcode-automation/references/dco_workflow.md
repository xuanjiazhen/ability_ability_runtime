# DCO签名工作流示例

本文档展示如何使用gitcode-automation skill创建符合DCO要求的PR。

## 完整工作流

### 1. 准备代码

创建或修改代码文件：

```bash
cd /path/to/ability_runtime
# 创建新分支
git checkout -b feature/my-feature

# 进行代码修改
vim my_code.cpp
```

### 2. 提交代码（自动添加DCO签名）

**方法1：使用skill的commit命令（推荐）**

```bash
# 添加修改的文件
git add my_code.cpp

# 使用skill提交（自动添加DCO签名）
python ~/.claude/skills/gitcode-automation/scripts/create_reviewed_pr.py commit -m "feat: add my new feature

This commit adds a new feature to improve performance.

Related: #14320"
```

**方法2：手动提交（使用git的--signoff）**

```bash
git commit -s -m "feat: add my new feature"
```

**方法3：为已有commit添加DCO签名**

```bash
# 如果已经commit但没有DCO签名
python ~/.claude/skills/gitcode-automation/scripts/create_reviewed_pr.py amend-dco

# 强制推送到远程（如果已经推送过）
git push -f zhang feature/my-feature
```

### 3. 验证DCO签名

```bash
# 查看commit message
git log -1 --pretty=%B

# 应该看到类似输出：
# feat: add my new feature
#
# Related: #14320
#
# Signed-off-by: zhang_hao_zheng <zhanghaozheng2@h-partners.com>
```

### 4. 推送到远程

```bash
# 推送到个人fork
git push zhang feature/my-feature
```

### 5. 创建PR（带代码审查）

```bash
# 使用skill创建PR
python ~/.claude/skills/gitcode-automation/scripts/create_reviewed_pr.py create-pr \
  --issue "14320" \
  --title "Add my new feature" \
  --description "This PR adds a new feature to improve performance"
```

## 常用命令

### 查看所有可用命令

```bash
python ~/.claude/skills/gitcode-automation/scripts/create_reviewed_pr.py --help
```

输出：
```
usage: create_reviewed_pr.py [-h] {commit,amend-dco,create-pr} ...

GitCode PR创建工具 - 带代码审查功能和DCO签名支持

optional arguments:
  -h, --help            show this help message and exit

subcommands:
  {commit,amend-dco,create-pr}
    commit              执行带DCO签名的commit
    amend-dco           为最后一个commit添加DCO签名
    create-pr           创建Pull Request
```

### commit命令

```bash
python create_reviewed_pr.py commit -m "commit message"
```

### amend-dco命令

```bash
python create_reviewed_pr.py amend-dco
```

### create-pr命令

```bash
python create_reviewed_pr.py create-pr --issue "14320" --title "PR Title"
```

## DCO配置

### 配置文件位置

DCO签名所需的信息存储在 `~/.claude/gitcode.json`：

```json
{
    "loginType": "gitee",
    "username": "zhang_hao_zheng",
    "email": "zhanghaozheng2@h-partners.com"
}
```

### 修改用户信息

如果需要修改用户信息，编辑配置文件：

```bash
vim ~/.claude/gitcode.json
```

或者使用git config设置：

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

## 错误处理

### 错误1：DCO签名缺失

**症状**：
```
remote: ERROR: DCO verification failed
```

**解决方案**：

使用`amend-dco`命令为最后一个commit添加签名：

```bash
python ~/.claude/skills/gitcode-automation/scripts/create_reviewed_pr.py amend-dco
git push -f zhang feature/my-feature
```

### 错误2：用户信息未配置

**症状**：
```
[WARNING] 无法获取Git用户信息
[WARNING] DCO签名可能缺失
```

**解决方案**：

检查或创建配置文件：

```bash
cat ~/.claude/gitcode.json
```

确保包含username和email字段。

### 错误3：commit已推送到远程

**症状**：
使用`amend-dco`后无法正常推送

**解决方案**：

需要强制推送（因为commit hash已改变）：

```bash
git push -f zhang feature/my-feature
```

**注意**：只有在你确定远程分支没有其他人修改的情况下才使用强制推送。

## 最佳实践

### 1. 始终使用DCO签名

- 使用`commit`命令而不是直接`git commit`
- 或者使用`git commit -s`

### 2. 提交前验证签名

```bash
# 检查最后一个commit
git log -1 --pretty=%B | grep "Signed-off-by:"

# 检查多个commits
git log --pretty=%B | grep "Signed-off-by:"
```

### 3. 批量添加DCO签名

如果多个commits缺少DCO签名，可以使用git rebase：

```bash
# 为最近3个commits添加签名
git rebase -i HEAD~3

# 在编辑器中，将每个commit的pick改为edit
# 然后对每个commit运行：
git commit --amend --no-edit --signoff
git rebase --continue
```

### 4. 配置git别名（可选）

为了方便使用，可以配置git别名：

```bash
# 添加别名
git config --global alias.dco-commit '!f() { git commit -s -m "$*"; }; f'

# 使用别名
git dco-commit "commit message"
```

## 示例输出

### 成功的commit输出

```bash
$ python create_reviewed_pr.py commit -m "test: add DCO demo"

[OK] Commit成功（包含DCO签名）
[INFO] 签名: Signed-off-by: zhang_hao_zheng <zhanghaozheng2@h-partners.com>
```

### 成功的PR创建输出

```bash
$ python create_reviewed_pr.py create-pr --issue "14320"

======================================================================
步骤1: 分析代码变更
======================================================================
变更统计:
test_dco_demo.cpp | 37 +++++++++++++++++++++++++++++++++++++
 1 file changed, 37 insertions(+)

======================================================================
步骤2: 执行代码审查
======================================================================

审查结果摘要:
  稳定性自检: 通过 6, 失败 0, 不涉及 11
  安全编码自检: 通过 8, 失败 2, 不涉及 10

======================================================================
步骤3: 创建Pull Request
======================================================================

正在创建PR...

======================================================================
[OK] PR创建成功！
======================================================================
PR ID: 18143
PR URL: https://gitcode.com/openharmony/ability_ability_runtime/merge_requests/18143
======================================================================
```

## 验证结果

### 检查PR中的commits

```bash
# 使用API检查PR的commits是否都包含DCO签名
python -c "
import requests

token = 'your_token'
pr_id = '18143'
api_url = f'https://gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/pulls/{pr_id}/commits'

headers = {'Authorization': f'Bearer {token}'}
response = requests.get(api_url, headers=headers)

commits = response.json()
for commit in commits:
    msg = commit['commit']['message']
    has_dco = 'Signed-off-by:' in msg
    print(f\"{commit['sha'][:7]}: {'PASS' if has_dco else 'FAIL'}\")
"
```

输出示例：
```
ff2a2dc: PASS
```

## 总结

使用gitcode-automation skill，可以轻松创建符合DCO要求的PR：

1. ✅ 自动添加DCO签名到commit
2. ✅ 自动进行代码审查
3. ✅ 自动填充PR自检项
4. ✅ 创建符合OpenHarmony规范的PR

关键命令：
- `commit` - 提交代码（含DCO签名）
- `amend-dco` - 为已有commit添加DCO签名
- `create-pr` - 创建PR（含代码审查）
