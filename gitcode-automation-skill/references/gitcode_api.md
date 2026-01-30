# GitCode API Reference

API documentation for GitCode pull request automation.

## Authentication

GitCode API v5 uses Bearer token authentication.

### Token Location

Tokens are stored in `~/.claude/gitcode_token.json`:
```json
{
  "token": "your_access_token_here"
}
```

### Authorization Header

All API requests must include:
```
Authorization: Bearer {token}
```

## Pull Request API

### Create Pull Request

**Endpoint:** `POST /api/v5/repos/{owner}/{repo}/pulls`

**URL Parameters:**
- `owner`: Repository owner (e.g., `openharmony`)
- `repo`: Repository name (e.g., `ability_ability_runtime`)

**Full URL Example:**
```
https://gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/pulls
```

**Request Headers:**
```
Authorization: Bearer YOUR_ACCESS_TOKEN
Content-Type: application/json
```

**Request Body:**
```json
{
  "head": "YOUR_USERNAME/ability_ability_runtime:feature-branch",
  "base": "master",
  "title": "Fix memory leak in event handler",
  "body": "**IssueNo**:\\nhttps://gitcode.com/openharmony/ability_ability_runtime/issues/14320\\n\\n**Description**:\\nFix memory leak...\\n\\n**稳定性自检：**\\n..."
}
```

**Field Descriptions:**
- `head`: Source branch in format `fork_owner/repo_name:branch_name`
- `base`: Target branch (usually `master` for OpenHarmony)
- `title`: PR title
- `body`: PR description in markdown format

**Response (Success):**
```json
{
  "id": 18135,
  "number": 18135,
  "html_url": "https://gitcode.com/openharmony/ability_ability_runtime/pull/18135",
  "web_url": "https://gitcode.com/openharmony/ability_ability_runtime/pull/18135",
  "state": "open",
  "title": "Fix memory leak in event handler",
  "body": "...",
  "head": {
    "label": "YOUR_USERNAME:feature-branch",
    "ref": "feature-branch",
    "repo": {
      "full_name": "YOUR_USERNAME/ability_ability_runtime"
    }
  },
  "base": {
    "label": "openharmony:master",
    "ref": "master",
    "repo": {
      "full_name": "openharmony/ability_ability_runtime"
    }
  }
}
```

**Response Fields:**
- `id`/`number`: Pull request ID
- `html_url`/`web_url`: Direct URL to the PR
- `state`: PR state (`open`, `closed`, `merged`)

### Update Pull Request

**Endpoint:** `PATCH /api/v5/repos/{owner}/{repo}/pulls/{number}`

**Example:**
```
PATCH https://gitcode.com/api/v5/repos/openharmony/ability_ability_runtime/pulls/18135
```

**Request Body:**
```json
{
  "title": "Updated title",
  "body": "Updated description"
}
```

**Important:** Use `PATCH` method, not `PUT`.

## Error Codes

### Common HTTP Status Codes

**200 OK**
Request succeeded

**201 Created**
Resource created successfully (PR created)

**400 Bad Request**
Invalid request parameters

**403 Forbidden**
```
{"error_code": 403, "error_message": "403 Forbidden - Unauthorized access"}
```
Causes:
- No write permission to repository
- Token lacks necessary permissions
- Attempting to push directly to openharmony (use fork instead)

**404 Not Found**
```
{"error_code": 404, "error_message": "Can not find the branch: branch-name in project"}
```
Causes:
- Branch doesn't exist on remote
- Fork repository doesn't exist

**422 Unprocessable Entity**
Validation error (e.g., invalid branch names)

## Rate Limiting

GitCode API has rate limits. If you hit rate limits:
- Space out requests
- Use exponential backoff
- Check response headers for rate limit info

## Token Permissions

Required token scopes:
- `repo`: Full repository access
- `write_repository`: Write repository access

Get token from: https://gitcode.com/-/settings/personal_access_tokens

## Fork Workflow

OpenHarmony uses fork-based contribution model:

1. Fork `openharmony/repo` to `YOUR_USERNAME/repo`
2. Push to `YOUR_USERNAME/repo:branch`
3. Create PR from `YOUR_USERNAME:branch` → `openharmony:master`

**Example Git Commands:**
```bash
# Add fork as remote
git remote add fork https://gitcode.com/YOUR_USERNAME/ability_ability_runtime.git

# Push to fork
git push fork feature-branch

# Create PR via API
POST /api/v5/repos/openharmony/ability_ability_runtime/pulls
{
  "head": "YOUR_USERNAME/ability_ability_runtime:feature-branch",
  "base": "master"
}
```

## Complete Example

```python
import requests

token = "your_token"
repo = "openharmony/ability_ability_runtime"
api_url = f"https://gitcode.com/api/v5/repos/{repo}/pulls"

headers = {
    "Authorization": f"Bearer {token}",
    "Content-Type": "application/json"
}

data = {
    "head": "YOUR_USERNAME/ability_ability_runtime:fix-bug-123",
    "base": "master",
    "title": "Fix bug #123",
    "body": "**IssueNo**:\\nhttps://gitcode.com/openharmony/ability_ability_runtime/issues/123\\n\\nFixes #123"
}

response = requests.post(api_url, json=data, headers=headers)

if response.status_code in [200, 201]:
    result = response.json()
    print(f"PR created: {result['html_url']}")
else:
    print(f"Error: {response.status_code} - {response.text}")
```

## References

- Official API docs: https://docs.gitcode.com/docs/apis/post-api-v-5-repos-owner-repo-pulls
- Authentication guide: https://docs.gitcode.com/docs/apis/overview
- Fork contribution guide: OpenHarmony contribution guidelines
