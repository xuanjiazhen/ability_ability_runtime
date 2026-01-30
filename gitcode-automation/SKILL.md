---
name: gitcode-automation
description: Automate GitCode PR creation workflow for OpenHarmony repositories with DCO signature support. Use when user needs to: (1) Commit code with DCO (Developer Certificate of Origin) signature, (2) Push code to personal fork repository, (3) Create pull requests to OpenHarmony upstream, (4) Fill PR templates with auto-generated checklist based on actual code review. This skill handles the complete workflow: DCO-signed commit → git push → code analysis → PR creation via GitCode API.
---

# GitCode Automation Skill

Automates the complete GitCode pull request creation workflow for OpenHarmony development with DCO signature support.

## When to Use This Skill

Use this skill when working with GitCode repositories and OpenHarmony contributions:
- User mentions "创建PR", "create PR", "push and create PR", "提交代码"
- User wants to commit code with DCO signature (Signed-off-by)
- User wants to push code to personal fork and create PR to openharmony
- User needs to fill PR checklist items based on actual code review
- User references GitCode, OpenHarmony, ability_runtime, or similar repositories

## Configuration Files

The skill expects these configuration files in `~/.claude/`:

- **gitcode_token.json**: Contains GitCode API access token
  ```json
  {"token": "your_token_here"}
  ```

- **gitcode.json**: Contains Git user information for DCO signature
  ```json
  {
    "username": "zhang_hao_zheng",
    "email": "zhanghaozheng2@h-partners.com"
  }
  ```

- **pr_templates.json**: Contains repository-specific PR templates
  ```json
  {
    "openharmony/ability_ability_runtime": {
      "template_name": "ability_runtime PR Template",
      "template": "**IssueNo**:\\n{issue_no}\\n\\n..."
    }
  }
  ```

## DCO Signature Support

**What is DCO?**

DCO (Developer Certificate of Origin) is a requirement for OpenHarmony contributions. Every commit must include a `Signed-off-by` line in the commit message:

```
Signed-off-by: zhang_hao_zheng <zhanghaozheng2@h-partners.com>
```

**Automatic DCO Signing:**

The skill automatically adds DCO signatures from configuration files to commits:

1. **Using `commit` command**:
   ```bash
   python scripts/gitcode_automation.py commit -m "commit message"
   ```
   Automatically adds DCO signature from `gitcode.json` to the commit

2. **Using `amend-dco` command**:
   ```bash
   python scripts/gitcode_automation.py amend-dco
   ```
   Adds DCO signature to the last commit (if missing)

3. **Manual git commit with --signoff**:
   ```bash
   git commit -s -m "commit message"
   ```
   The `-s` flag automatically adds Signed-off-by line from git config

**Important:** The DCO signature must match exactly the format in `gitcode.json`:
```
Signed-off-by: zhang_hao_zheng <zhanghaozheng2@h-partners.com>
```

Do not manually sign with different names or emails - always use the configuration file to ensure consistency.

## Workflow

### Step 1: Commit with DCO Signature

Commit code changes with automatic DCO signature:
- Use `commit` command to create commits with DCO signature
- Or use `amend-dco` to add signature to existing commits
- DCO format: `Signed-off-by: Name <email>`

### Step 2: Push to Personal Fork

Push current branch to user's personal fork repository:
- Extract repository info from `git remote -v`
- For `openharmony/repo_name`, push to `zhang_hao_zheng/repo_name`
- Use current branch name for both local and remote

### Step 3: Load PR Template

Load repository-specific template from `pr_templates.json`:
- Match repository path (e.g., `openharmony/ability_ability_runtime`)
- If template not found, use default format with basic fields

### Step 3: Analyze Code Changes

Analyze git diff to auto-fill checklist items:

**File Type Detection:**
- C++ files (`.cpp`, `.cc`, `.h`, `.hpp`) → Pointer operations check
- Java/Kotlin files (`.java`, `.kt`) → Parcelable checks
- JS/TS files (`.js`, `.ts`, `.jsx`, `.tsx`) → JS thread checks
- JSON files (`.json`) → JSON validation checks

**Keyword Detection in Diff:**
- Thread-related: `thread`, `pthread`, `mutex`, `lock`, `async`
- IPC: `ipc`, `binder`, `ashmem`, `shared_memory`
- Pointer: `pointer`, `sptr`, `wptr`, `shared_ptr`, `unique_ptr`
- Memory: `malloc`, `free`, `new`, `delete`, `memcpy`
- File: `fopen`, `fclose`, `open`, `read`, `write`

**Checklist Rules:**

The skill performs intelligent code analysis to generate checklist results:

- **"√" (Passed)**: Code follows best practices, no violations detected
  - Example: No threading issues found in C++ code → "√"
  - Example: Variables are properly initialized → "√"
  - Example: No memory leaks detected → "√"

- **"不涉及" (Not Applicable)**: Code type doesn't match the check
  - Example: JavaScript file checked for C++ pointer rules → "不涉及"
  - Example: No JSON changes → JSON validation rules are "不涉及"

- **"需要检查" (Needs Review)**: Potential issues detected, requires manual verification
  - Example: Thread-related code without visible locking → "需要检查"
  - Example: Raw pointers used without null checks → "需要检查"
  - Example: Memory allocation without error handling → "需要检查"

The analysis examines:
- Added code lines (prefixed with `+`)
- File types and extensions
- Keywords and patterns (threading, IPC, pointers, memory operations, etc.)
- Code constructs (loops, conditionals, error handling, etc.)

### Step 4: Create PR via GitCode API

Use GitCode API v5 to create pull request:

**Endpoint:** `POST /api/v5/repos/{target_repo}/pulls`

**Headers:**
```
Authorization: Bearer {token}
Content-Type: application/json
```

**Request Body:**
```json
{
  "head": "zhang_hao_zheng/repo_name:branch_name",
  "base": "master",
  "title": "PR title",
  "body": "PR description with filled checklist"
}
```

**Reference:** https://docs.gitcode.com/docs/apis/post-api-v-5-repos-owner-repo-pulls

## Implementation Notes

### Repository Path Extraction

From `git remote -v` output:
```
origin  https://gitcode.com/openharmony/ability_ability_runtime.git (fetch)
```
Extract: `openharmony/ability_ability_runtime`

### Branch Detection

Use `git rev-parse --abbrev-ref HEAD` to get current branch name.

### PR Template Format

The template uses these placeholders:
- `{issue_no}`: Issue URL
- `{description}`: PR description
- `{tdd_result}`: TDD test result
- `{xts_result}`: XTS test result

Stability and security checklist sections are dynamically generated based on code analysis.

### Error Handling

Common issues and solutions:
- **403 Forbidden**: No write permission to upstream (should push to fork instead)
- **404 Not Found**: Branch doesn't exist on remote (push first)
- **Token invalid**: Check `gitcode_token.json` for correct token

## Example Usage

User says: "帮我创建PR关联issue #14320"

Workflow:
1. Get current branch: `feature/test-branch`
2. Push to: `zhang_hao_zheng/ability_ability_runtime:feature/test-branch`
3. Load template for `openharmony/ability_ability_runtime`
4. Analyze changes via `git diff HEAD~1 HEAD`
5. Generate checklist with "需要检查" or "不涉及"
6. Create PR to `openharmony/ability_ability_runtime:master`
7. Return PR URL to user

## Scripts

Use `scripts/gitcode_automation.py` for deterministic execution. This script handles:
- Git operations (push, branch detection, remote parsing)
- Code change analysis (file types, keyword detection)
- Checklist generation (18 stability + 21 security checks)
- PR creation via GitCode API

The script is preferred over inline code for reliability and consistency.

## API Reference

For API details, see `references/gitcode_api.md` which documents:
- Authentication methods
- Pull request creation endpoints
- Request/response formats
- Error codes and handling
