# CLI 工具帮助信息输出修复总结

## 问题描述

在 ohos-example、ohos-simple、ohos-timer 三个示例工具中，`ShowHelp()` 函数使用了事件格式输出帮助信息：

```cpp
// ❌ 错误的做法
void ShowHelp()
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"Usage: ...\""
              << "}}" << std::endl;
}
```

**问题**：帮助信息不应该包装在事件格式中，应该直接输出纯文本。

## 修复方案

### 修改前

```cpp
void ShowHelp()
{
    EmitStdout("Usage: ohos-simple [options]");
    EmitStdout("Options:");
    // ...
}
```

或者

```cpp
void ShowHelp()
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"Usage: ...\""
              << "}}" << std::endl;
}
```

### 修改后 ✅

```cpp
void ShowHelp()
{
    std::cout << "Usage: ohos-simple [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --message=<msg>  Set message to display" << std::endl;
    // ...
}
```

## 修复的文件

1. ✅ `/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/ohos-example/src/main.cpp`
2. ✅ `/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/ohos-simple/src/main.cpp`
3. ✅ `/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/ohos-timer/src/main.cpp`

## 修复效果对比

### 修改前

```bash
$ ohos-example help
{"event": "stdout", "data": {"message": "Usage: ohos-example <subcommand> [args]\nSubcommands:\n  run [args...]     Run the tool with arguments\n  version          Show version information\n  help             Show this help message"}}
```

### 修改后 ✅

```bash
$ ohos-example help
Usage: ohos-example <subcommand> [args]
Subcommands:
  run [args...]     Run the tool with arguments
  version          Show version information
  help             Show this help message
```

## 设计原则

### 事件系统 vs 纯文本输出

**使用事件系统的场景**（结构化数据）:
- ✅ 执行结果（result 事件）
- ✅ 进度更新（progress 事件）
- ✅ 错误信息（error 事件）
- ✅ 详细输出（verbose 模式下的 stdout 事件）

**使用纯文本输出的场景**:
- ✅ 帮助信息（--help）
- ✅ 版本信息（部分工具的 --version）
- ✅ 简单的状态提示
- ✅ 用户友好的错误提示

### 为什么帮助信息应该用纯文本？

1. **易读性**: 用户直接看到易读的文本，而不是 JSON 格式
2. **惯例**: 符合传统 CLI 工具的设计惯例（如 `ls --help`、`git --help`）
3. **兼容性**: 可以直接用 `man` 或其他文档工具处理
4. **性能**: 避免不必要的 JSON 序列化和事件包装开销

## 其他需要注意的输出

### ✅ 应该使用事件系统的输出

```cpp
// 执行结果 - 使用 result 事件
EmitResult(result);

// 进度更新 - 使用 progress 事件
EmitProgress(percentage, status);

// 错误信息 - 使用 error 事件
EmitError(error_message);

// 详细输出 - 使用 stdout 事件
EmitStdout(verbose_message);
```

### ❌ 不应该使用事件系统的输出

```cpp
// 帮助信息 - 直接输出
std::cout << "Help text" << std::endl;

// 简单状态提示 - 直接输出
std::cout << "Processing..." << std::endl;
```

## 验证方法

### 测试命令

```bash
# 测试帮助信息输出
ohos-example help
ohos-simple --help
ohos-timer --help

# 测试错误情况下的帮助信息输出
ohos-example unknown
```

### 预期输出

所有帮助信息应该是纯文本格式，不包含 `{"event": ...}` 包装。

## 总结

这次修复确保了三个示例工具的帮助信息输出符合 CLI 工具的设计惯例：

- ✅ 帮助信息直接输出纯文本
- ✅ 不使用事件格式包装
- ✅ 符合传统 CLI 工具的预期行为
- ✅ 提供更好的用户体验

**重要原则**: 事件系统用于结构化数据输出，纯文本用于用户友好的帮助信息。
