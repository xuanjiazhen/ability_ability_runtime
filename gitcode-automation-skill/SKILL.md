---
name: gitcode-automation
description: Automate GitCode PR creation workflow for OpenHarmony repositories with DCO signature support. Use when user needs to: (1) Commit code with DCO (Developer Certificate of Origin) signature, (2) Push code to personal fork repository, (3) Create pull requests to OpenHarmony upstream, (4) Fill PR templates with auto-generated checklist based on actual code review. This skill handles the complete workflow: DCO-signed commit → git push → code analysis → PR creation via GitCode API. First-time users will be guided through setup wizard to configure tokens and credentials.
---

# GitCode Automation Skill

Automates the complete GitCode pull request creation workflow for OpenHarmony development with DCO signature support.

## Quick Start

**First-time setup:**
```bash
python scripts/setup_wizard.py
```

**Common usage:**
```bash
# Commit with DCO signature
python scripts/gitcode_automation.py commit -m "fix: bug description"

# Create PR with interactive mode (recommended for first-time users)
python scripts/gitcode_automation.py push-pr -i --issue "14320"

# Create PR with default settings
python scripts/gitcode_automation.py push-pr --issue "https://gitcode.com/openharmony/ability_ability_runtime/issues/14320"
```

## When to Use This Skill

Use this skill when working with GitCode repositories and OpenHarmony contributions:
- User mentions "创建PR", "create PR", "push and create PR", "提交代码"
- User wants to commit code with DCO signature (Signed-off-by)
- User wants to push code to personal fork and create PR to openharmony
- User needs to fill PR checklist items based on actual code review
- User references GitCode, OpenHarmony, ability_runtime, or similar repositories

## Configuration

The skill expects these configuration files in `~/.claude/`:

### Required Configuration Files

**gitcode_token.json**: Contains GitCode API access token
```json
{
  "token": "YOUR_GITCODE_TOKEN_HERE"
}
```

Get your token from: https://gitcode.com/-/settings/personal_access_tokens
Required scopes: `repo`, `write_repository`

**gitcode.json**: Contains Git user information for DCO signature
```json
{
  "username": "your_gitcode_username",
  "email": "your.email@example.com"
}
```

### Optional Configuration Files

**pr_templates.json**: Contains repository-specific PR templates
```json
{
  "openharmony/ability_ability_runtime": {
    "template_name": "ability_runtime PR Template",
    "template": "**IssueNo**:\\n{issue_no}\\n\\n**Description**:\\n{description}\\n\\n..."
  }
}
```

If not provided, a default template will be used.

### First-Time Setup Wizard

Run the setup wizard on first use to configure all required files:

```bash
python scripts/setup_wizard.py
```

The wizard will guide you through:
1. Setting up GitCode access token
2. Configuring your Git user information (for DCO signature)
3. Setting up PR templates (optional)

## DCO Signature Support

**What is DCO?**

DCO (Developer Certificate of Origin) is a requirement for OpenHarmony contributions. Every commit must include a `Signed-off-by` line in the commit message:

```
Signed-off-by: Your Name <your.email@example.com>
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

## Workflow

### Step 1: Commit with DCO Signature

Commit code changes with automatic DCO signature:
- Use `commit` command to create commits with DCO signature
- Or use `amend-dco` to add signature to existing commits
- DCO format: `Signed-off-by: Name <email>`

### Step 2: Push to Personal Fork

Push current branch to user's personal fork repository:
- Extract repository info from `git remote -v`
- For `openharmony/repo_name`, push to `your_username/repo_name`
- Use current branch name for both local and remote

### Step 3: Interactive Mode (Recommended for First-Time Users)

Using the interactive mode (`-i` flag) provides guided selection for:
- Target repository from configured templates or current git repo
- Source repository (your fork)
- Branch selection from available branches
- Confirmation before creating PR

Example:
```bash
python scripts/gitcode_automation.py push-pr -i --issue "14320"
```

### Step 4: Analyze Code Changes

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

### Step 5: Create PR via GitCode API

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
  "head": "your_username/repo_name:branch_name",
  "base": "master",
  "title": "PR title",
  "body": "PR description with filled checklist"
}
```

**Reference:** https://docs.gitcode.com/docs/apis/post-api-v-5-repos-owner-repo-pulls

## Command Reference

### setup_wizard.py

First-time configuration wizard:
```bash
python scripts/setup_wizard.py
```

### gitcode_automation.py

Main automation script with three commands:

**1. commit - Commit with DCO signature:**
```bash
python scripts/gitcode_automation.py commit -m "commit message"
```

**2. amend-dco - Add DCO to last commit:**
```bash
python scripts/gitcode_automation.py amend-dco
```

**3. push-pr - Push and create PR:**
```bash
# Interactive mode (recommended)
python scripts/gitcode_automation.py push-pr -i --issue "14320"

# Non-interactive mode
python scripts/gitcode_automation.py push-pr --issue "14320"

# With custom title and description
python scripts/gitcode_automation.py push-pr -i --issue "14320" --title "Fix bug" --description "Description here"

# Don't push, only create PR
python scripts/gitcode_automation.py push-pr -i --issue "14320" --no-push
```

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
- **Missing configuration**: Run `python scripts/setup_wizard.py`

## Example Usage

User says: "帮我创建PR关联issue #14320"

Workflow with interactive mode:
1. Run: `python scripts/gitcode_automation.py push-pr -i --issue "14320"`
2. Select target repository from list (or use current)
3. Select source repository (your fork)
4. Select branch from available branches
5. Confirm PR creation details
6. Push code to fork
7. Load template for selected repository
8. Analyze changes via `git diff HEAD~1 HEAD`
9. Generate checklist with "需要检查" or "不涉及"
10. Create PR to upstream master
11. Return PR URL to user

## Scripts

Use `scripts/gitcode_automation.py` for deterministic execution. This script handles:
- Git operations (push, branch detection, remote parsing)
- Code change analysis (file types, keyword detection)
- Checklist generation (18 stability + 21 security checks)
- PR creation via GitCode API
- Interactive mode for guided workflow

The script is preferred over inline code for reliability and consistency.

## API Reference

For API details, see `references/gitcode_api.md` which documents:
- Authentication methods
- Pull request creation endpoints
- Request/response formats
- Error codes and handling
