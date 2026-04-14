# claw_aa_cli JSON 标准化输出修改总结

## 修改日期
2025-04-13

## 修改概述
将 `claw_aa_cli` 的三个核心命令（start、force-stop、help）的输出从纯文本格式改为标准化 JSON 格式。

---

## 📝 修改的命令

### 1. help 命令 - ✅ 已修改

**修改前**:
```cpp
ErrCode ClawAaShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return ERR_OK;
}
```

**修改后**:
```cpp
ErrCode ClawAaShellCommand::RunAsHelpCommand()
{
    printSuccess(HELP_MSG);
    return ERR_OK;
}
```

**输出示例**:
```json
{
  "success": true,
  "message": "usage: claw_aa_cli <command> <options>\nThese are common claw_aa_cli commands list:\n  help                        list available commands\n  start                       start ability with options\n  force-stop <bundle-name>    force stop the process with bundle name\n"
}
```

---

### 2. start 命令 - ✅ 已修改

**修改前**:
```cpp
if (result == OHOS::ERR_OK) {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_START_ABILITY_OK.c_str());
    resultReceiver_.append(STRING_START_ABILITY_OK).append("\n");
} else {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_START_ABILITY_NG.c_str(), result);
    if (result != START_ABILITY_WAITING) {
        resultReceiver_ = STRING_START_ABILITY_NG + "\n";
    }
    CheckStartAbilityResult(result);
    if (result == INNER_ERR) {
        result = INNER_ERR_START;
    }
    resultReceiver_.append(GetMessageFromCode(result));
}
```

**修改后**:
```cpp
if (result == OHOS::ERR_OK) {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_START_ABILITY_OK.c_str());
    printSuccess(STRING_START_ABILITY_OK);
} else {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_START_ABILITY_NG.c_str(), result);
    CheckStartAbilityResult(result);
    if (result == INNER_ERR) {
        result = INNER_ERR_START;
    }
    std::string errorMessage = GetMessageFromCode(result);
    printError(
        "failed to start ability.",
        std::to_string(result),
        errorMessage,
        "Check if the bundle name and ability name are correct, and ensure the application is installed."
    );
}
```

**成功输出示例**:
```json
{
  "success": true,
  "message": "start ability successfully."
}
```

**失败输出示例**:
```json
{
  "success": false,
  "message": "failed to start ability.",
  "error": {
    "code": "140",
    "message": "Inner error start"
  },
  "suggestion": "Check if the bundle name and ability name are correct, and ensure the application is installed."
}
```

---

### 3. force-stop 命令 - ✅ 已修改

**修改前**:
```cpp
if (argList_.empty()) {
    resultReceiver_.append(HELP_MSG_FORCE_STOP);
    return OHOS::ERR_INVALID_VALUE;
}

// ... 参数解析 ...

if (result == OHOS::ERR_OK) {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_FORCE_STOP_OK.c_str());
    resultReceiver_ = STRING_FORCE_STOP_OK + "\n";
} else {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
    resultReceiver_ = STRING_FORCE_STOP_NG + "\n";
    resultReceiver_.append(GetMessageFromCode(result));
}
```

**修改后**:
```cpp
if (argList_.empty()) {
    printError(
        "Missing bundle name parameter.",
        std::to_string(OHOS::ERR_INVALID_VALUE),
        HELP_MSG_FORCE_STOP,
        "Usage: claw_aa_cli force-stop <bundle-name> [-p pid] [-r kill-reason]"
    );
    return OHOS::ERR_INVALID_VALUE;
}

// ... 参数解析 ...

if (result == OHOS::ERR_OK) {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s", STRING_FORCE_STOP_OK.c_str());
    printSuccess(STRING_FORCE_STOP_OK);
} else {
    TAG_LOGI(AAFwkTag::AA_TOOL, "%{public}s result: %{public}d", STRING_FORCE_STOP_NG.c_str(), result);
    std::string errorMessage = GetMessageFromCode(result);
    printError(
        "failed to force stop process.",
        std::to_string(result),
        errorMessage,
        "Check if the bundle name is correct and the application is currently running."
    );
}
```

**成功输出示例**:
```json
{
  "success": true,
  "message": "force stop process successfully."
}
```

**失败输出示例**:
```json
{
  "success": false,
  "message": "failed to force stop process.",
  "error": {
    "code": "1",
    "message": "error details here"
  },
  "suggestion": "Check if the bundle name is correct and the application is currently running."
}
```

---

## 📊 JSON 输出格式规范

### 成功响应格式
```json
{
  "success": true,
  "message": "成功消息文本"
}
```

### 错误响应格式
```json
{
  "success": false,
  "message": "错误消息文本",
  "error": {
    "code": "错误代码（字符串形式）",
    "message": "详细错误信息"
  },
  "suggestion": "修复建议"
}
```

---

## 🔧 使用的工具函数

### printSuccess()
```cpp
void printSuccess(const std::string& message);
```
- 用于输出成功消息
- 自动设置 `"success": true`

### printError()
```cpp
void printError(const std::string& message, const std::string& code,
               const std::string& errorInfo, const std::string& suggestion);
```
- 用于输出错误消息
- 自动设置 `"success": false`
- 包含错误代码、详细信息和修复建议

---

## ✅ 修改验证

### 验证 help 命令
```bash
hdc shell claw_aa_cli help
```

**预期输出**:
```json
{"success":true,"message":"usage: claw_aa_cli <command> <options>\n..."}
```

### 验证 start 命令成功场景
```bash
hdc shell claw_aa_cli start -a MainAbility -b com.ohos.settings
```

**预期输出**:
```json
{"success":true,"message":"start ability successfully."}
```

### 验证 start 命令失败场景
```bash
hdc shell claw_aa_cli start -a TestAbility -b com.nonexistent.app
```

**预期输出**:
```json
{
  "success": false,
  "message": "failed to start ability.",
  "error": {
    "code": "140",
    "message": "Inner error start"
  },
  "suggestion": "Check if the bundle name and ability name are correct, and ensure the application is installed."
}
```

### 验证 force-stop 命令成功场景
```bash
hdc shell claw_aa_cli force-stop com.example.app
```

**预期输出**:
```json
{"success":true,"message":"force stop process successfully."}
```

### 验证 force-stop 命令失败场景
```bash
hdc shell claw_aa_cli force-stop com.nonexistent.app
```

**预期输出**:
```json
{
  "success": false,
  "message": "failed to force stop process.",
  "error": {
    "code": "1",
    "message": "error details"
  },
  "suggestion": "Check if the bundle name is correct and the application is currently running."
}
```

---

## 🎯 主要改进

### 1. 统一的输出格式
- 所有命令使用相同的 JSON 结构
- 便于机器解析和处理
- 保持向后兼容的错误代码

### 2. 增强的错误信息
- 包含错误代码（便于程序处理）
- 详细的错误描述
- 具体的修复建议

### 3. 保持日志记录
- 保留了所有 TAG_LOGI/TAG_LOGE 日志
- 不影响调试和问题排查

---

## 📖 使用示例

### 解析 JSON 输出

#### Python 示例
```python
import json
import subprocess

result = subprocess.run(['claw_aa_cli', 'start', '-a', 'MainAbility', '-b', 'com.example.app'],
                       capture_output=True, text=True)
response = json.loads(result.stdout)

if response['success']:
    print(f"Success: {response['message']}")
else:
    print(f"Error: {response['message']}")
    print(f"Code: {response['error']['code']}")
    print(f"Details: {response['error']['message']}")
    print(f"Suggestion: {response['suggestion']}")
```

#### Shell 示例 (使用 jq)
```bash
response=$(claw_aa_cli start -a MainAbility -b com.example.app)
success=$(echo "$response" | jq -r '.success')

if [ "$success" = "true" ]; then
    message=$(echo "$response" | jq -r '.message')
    echo "Success: $message"
else
    message=$(echo "$response" | jq -r '.message')
    code=$(echo "$response" | jq -r '.error.code')
    suggestion=$(echo "$response" | jq -r '.suggestion')
    echo "Error [$code]: $message"
    echo "Suggestion: $suggestion"
fi
```

---

## 🔍 与旧版本的兼容性

### 旧版本输出（纯文本）
```
start ability successfully.
```

### 新版本输出（JSON）
```json
{"success":true,"message":"start ability successfully."}
```

### 迁移建议
对于依赖旧版本输出的脚本：
1. 使用 `jq` 或类似工具解析 JSON
2. 提取 `message` 字段获取纯文本消息
3. 检查 `success` 字段判断命令是否成功

示例：
```bash
# 兼容旧脚本的方式
response=$(claw_aa_cli start -a MainAbility -b com.example.app)
message=$(echo "$response" | jq -r '.message')
echo "$message"
```

---

## 📝 修改的文件

### src/claw_aa_command.cpp
- ✅ `RunAsHelpCommand()` - 使用 printSuccess
- ✅ `RunAsStartAbility()` - 成功时使用 printSuccess，失败时使用 printError
- ✅ `RunAsForceStop()` - 成功时使用 printSuccess，失败时使用 printError

---

## ✨ 总结

所有三个核心命令已成功修改为使用标准化 JSON 输出：

✅ **help 命令** - 使用 printSuccess 输出帮助信息
✅ **start 命令** - 成功用 printSuccess，失败用 printError
✅ **force-stop 命令** - 成功用 printSuccess，失败用 printError

这些修改提供了：
- 统一的输出格式
- 机器可读的 JSON 结构
- 详细的错误信息
- 实用的修复建议

所有修改保留了原有的日志记录功能，不影响调试和问题排查能力。

---

## 🚀 后续步骤

1. **编译测试**: 确保代码能正确编译
   ```bash
   ./build.sh --product-name <product> --build-target claw_aa_cli
   ```

2. **功能测试**: 在实际设备上测试所有命令

3. **文档更新**: 更新用户文档，说明新的 JSON 输出格式

4. **脚本迁移**: 更新依赖旧版本输出的自动化脚本