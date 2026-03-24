# CLI API 设计：支持命令字符串参数

## 文档信息

**版本**: v1.0
**创建日期**: 2025-04-13
**状态**: 设计文档
**作者**: CLI 工具管理框架设计团队

---

## 目录

1. [背景与问题](#背景与问题)
2. [方案对比](#方案对比)
3. [命令字符串解析](#命令字符串解析)
4. [API 设计](#api-设计)
5. [完整实现](#完整实现)
6. [安全考虑](#安全考虑)
7. [使用示例](#使用示例)
8. [总结与建议](#总结与建议)

---

## 背景与问题

### 当前设计的局限性

**当前 execTool API 仅支持结构化参数**：

```typescript
await CLI.execTool('start-tool', {
  bundleName: 'com.example.abc',
  timeout: 30,
  debug: true
});
```

**存在的问题**：

1. **灵活性不足**：无法直接传递已有的命令字符串
2. **迁移成本高**：将现有命令行工具集成时需要重写参数
3. **学习曲线**：开发者需要学习新的参数映射规则
4. **场景限制**：某些复杂参数结构难以用 JSON Schema 表达

### 用户需求

**希望支持的调用方式**：

```typescript
// 直接传递命令字符串
await CLI.execTool('start-tool', {
  command: "startTool -a com.example.abc -t 30 --debug"
});
```

**优势**：
- 开发者熟悉，学习成本低
- 可以直接复用现有命令
- 灵活性高，支持复杂场景
- 迁移成本较低

---

## 方案对比

### 方案 1: 仅支持结构化参数（当前设计）

**API**：
```typescript
await CLI.execTool('start-tool', {
  bundleName: 'com.example.abc',
  timeout: 30,
  debug: true
});
```

**优点**：
- ✅ 类型安全，编译时检查
- ✅ 自动参数验证
- ✅ 避免注入攻击
- ✅ 跨平台兼容性好
- ✅ IDE 自动补全支持

**缺点**：
- ❌ 灵活性差
- ❌ 迁移成本高
- ❌ 学习曲线陡峭
- ❌ 复杂参数难以表达

**适用场景**：
- 新开发的工具
- 参数结构简单的场景
- 对安全性要求高的场景

---

### 方案 2: 仅支持命令字符串

**API**：
```typescript
await CLI.execToolRaw("start-tool -a com.example.abc -t 30 --debug");
```

**优点**：
- ✅ 灵活性极高
- ✅ 开发者熟悉
- ✅ 迁移成本极低
- ✅ 支持任意复杂参数

**缺点**：
- ❌ 无类型安全
- ❌ 无参数验证
- ❌ 注入攻击风险
- ❌ 跨平台兼容性问题
- ❌ 难以调试

**适用场景**：
- 快速原型开发
- 临时工具集成
- 参数极复杂的场景

---

### 方案 3: 混合模式（推荐）⭐

**API**：
```typescript
// 方式 1: 结构化参数（推荐）
await CLI.execTool('start-tool', {
  args: {
    bundleName: 'com.example.abc',
    timeout: 30
  }
});

// 方式 2: 命令字符串
await CLI.execTool('start-tool', {
  command: "startTool -a com.example.abc -t 30"
});
```

**优点**：
- ✅ 兼具两种模式的优势
- ✅ 向后兼容
- ✅ 灵活选择
- ✅ 渐进式迁移

**缺点**：
- ⚠️ API 稍微复杂
- ⚠️ 需要额外的解析逻辑

**适用场景**：
- 所有场景（通用方案）

---

## 命令字符串解析

### 问题：execve 需要参数数组

**execve 系统调用原型**：
```cpp
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

**需要将**：
```bash
"startTool -a com.example.abc -t 30 --debug"
```

**转换为**：
```cpp
argv[0] = "/usr/bin/startTool"
argv[1] = "-a"
argv[2] = "com.example.abc"
argv[3] = "-t"
argv[4] = "30"
argv[5] = "--debug"
argv[6] = nullptr
```

---

### 解析方案对比

#### 方案 A: 简单空格分割（不推荐）

```cpp
std::vector<std::string> ParseCommand(const std::string& cmd) {
    std::vector<std::string> args;
    std::istringstream iss(cmd);
    std::string token;
    while (iss >> token) {
        args.push_back(token);
    }
    return args;
}
```

**问题示例**：
```bash
# 输入命令
startTool -a "app with spaces" -t 30

# 错误解析结果
["startTool", "-a", "\"app", "with", "spaces\"", "-t", "30"]

# 期望结果
["startTool", "-a", "app with spaces", "-t", "30"]
```

**缺陷**：
- ❌ 无法处理带引号的参数
- ❌ 无法处理转义字符
- ❌ 无法处理参数中的空格

---

#### 方案 B: Shell 风格解析（推荐）⭐

**支持特性**：
- ✅ 单引号：`'app name'`
- ✅ 双引号：`"app name"`
- ✅ 转义字符：`\`
- ✅ 空格分隔参数

**完整实现**：

```cpp
// services/climgr/src/command_parser.h

#pragma once

#include <string>
#include <vector>

namespace OHOS {
namespace CliMgr {

/**
 * @brief 命令字符串解析器
 *
 * 支持类 Shell 的引号和转义规则：
 * - 单引号: 'app name'
 * - 双引号: "app name"
 * - 转义字符: \
 * - 空格分隔参数
 */
class CommandParser {
public:
    /**
     * @brief 解析命令字符串为参数数组
     * @param command 命令字符串，如 "startTool -a com.example.abc -t 30"
     * @return 参数数组
     *
     * @example
     * 输入: "tool --name 'My App' --path /home/user"
     * 输出: ["tool", "--name", "My App", "--path", "/home/user"]
     */
    static std::vector<std::string> ParseCommand(const std::string& command);

    /**
     * @brief 将命令字符串转换为 execve 需要的 argv 数组
     * @param executable 可执行文件路径
     * @param command 命令字符串
     * @return C 风格字符串数组（用于 execve）
     *
     * @note 调用者需要 free 返回的字符串指针
     */
    static std::vector<char*> BuildArgvFromCommand(
        const std::string& executable,
        const std::string& command);

    /**
     * @brief 验证命令字符串是否安全
     * @param command 命令字符串
     * @return 是否安全
     */
    static bool ValidateCommand(const std::string& command);

private:
    /**
     * @brief 检查字符是否为空白字符
     */
    static bool IsSpace(char c);

    /**
     * @brief 转义字符处理
     */
    static char HandleEscape(char c);
};

} // namespace CliMgr
} // namespace OHOS
```

**实现文件**：

```cpp
// services/climgr/src/command_parser.cpp

#include "command_parser.h"
#include "hilog_tag_wrapper.h"
#include <cctype>
#include <cstring>

namespace OHOS {
namespace CliMgr {

std::vector<std::string> CommandParser::ParseCommand(const std::string& command)
{
    std::vector<std::string> args;
    std::string current;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escape = false;

    for (size_t i = 0; i < command.size(); ++i) {
        char c = command[i];

        if (escape) {
            // 处理转义字符
            current += HandleEscape(c);
            escape = false;
        }
        else if (c == '\\') {
            // 遇到转义符
            escape = true;
        }
        else if (c == '\'' && !inDoubleQuote) {
            // 单引号开始/结束
            inSingleQuote = !inSingleQuote;
        }
        else if (c == '"' && !inSingleQuote) {
            // 双引号开始/结束
            inDoubleQuote = !inDoubleQuote;
        }
        else if (IsSpace(c) && !inSingleQuote && !inDoubleQuote) {
            // 空格分隔参数
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        }
        else {
            current += c;
        }
    }

    // 添加最后一个参数
    if (!current.empty()) {
        args.push_back(current);
    }

    // 错误检查：未闭合的引号
    if (inSingleQuote || inDoubleQuote) {
        TAG_LOGW(TAG, "Unclosed quote in command: %{public}s", command.c_str());
    }

    return args;
}

std::vector<char*> CommandParser::BuildArgvFromCommand(
    const std::string& executable,
    const std::string& command)
{
    // 1. 解析命令字符串
    std::vector<std::string> argsStr = ParseCommand(command);

    // 2. 构建完整的 argv（包含可执行文件路径）
    std::vector<std::string> fullArgs;
    fullArgs.push_back(executable);  // argv[0]
    fullArgs.insert(fullArgs.end(), argsStr.begin(), argsStr.end());

    // 3. 转换为 C 风格字符串数组
    std::vector<char*> argv;
    for (auto& str : fullArgs) {
        argv.push_back(strdup(str.c_str()));
    }
    argv.push_back(nullptr);  // argv 必须以 nullptr 结尾

    TAG_LOGD(TAG, "Parsed %{public}zu arguments from command",
             fullArgs.size() - 1);

    return argv;
}

bool CommandParser::ValidateCommand(const std::string& command)
{
    // 禁止的危险字符序列
    const std::vector<std::string> dangerous = {
        ";",     // 命令分隔符
        "|",     // 管道
        "&",     // 后台运行
        "$(",    // 命令替换
        "`",     // 命令替换
        ">",     // 输出重定向
        "<",     // 输入重定向
        "\n",    // 换行（命令注入）
        "\r",    // 回车
        "\t",    // 制表符
    };

    for (const auto& seq : dangerous) {
        if (command.find(seq) != std::string::npos) {
            TAG_LOGE(TAG, "Dangerous command sequence detected: %{public}s",
                     seq.c_str());
            return false;
        }
    }

    return true;
}

bool CommandParser::IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

char CommandParser::HandleEscape(char c)
{
    // 处理常见转义序列
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case ' ': return ' ';
        default: return c;
    }
}

} // namespace CliMgr
} // namespace OHOS
```

---

#### 方案 C: 使用 wordexp（POSIX 标准）

```cpp
#include <wordexp.h>

std::vector<std::string> ParseCommandWithWordexp(const std::string& command) {
    wordexp_t result;
    std::vector<std::string> args;

    // WRDE_NOCMD: 禁止命令替换（安全）
    // WRDE_UNDEF: 未定义变量视为错误
    int flags = WRDE_NOCMD | WRDE_UNDEF;

    if (wordexp(command.c_str(), &result, flags) == 0) {
        for (size_t i = 0; i < result.we_wordc; ++i) {
            args.push_back(result.we_wordv[i]);
        }
        wordfree(&result);
    }

    return args;
}
```

**⚠️ 安全警告**：
- `wordexp` 有命令注入风险
- 必须设置 `WRDE_NOCMD` 标志禁止命令替换
- 建议使用方案 B 的自定义解析器

---

## API 设计

### TypeScript 接口定义

```typescript
// interfaces/kits/native/cli.d.ts

export interface CLIManager {
  /**
   * 执行 CLI 工具（混合模式）
   * @param name 工具名称
   * @param input 输入参数（结构化或命令字符串）
   * @param options 执行选项
   * @returns 执行结果
   */
  execTool(
    name: string,
    input: ToolInput,
    options?: ExecOptions
  ): Promise<ExecResult>;

  /**
   * 执行 CLI 工具（仅结构化参数，向后兼容）
   * @deprecated 使用 execTool 替代
   */
  execToolWithArgs(
    name: string,
    args: Record<string, any>,
    options?: ExecOptions
  ): Promise<ExecResult>;

  /**
   * 执行 CLI 工具（仅命令字符串）
   * @param name 工具名称
   * @param command 命令字符串
   * @param options 执行选项
   * @returns 执行结果
   */
  execToolWithCommand(
    name: string,
    command: string,
    options?: ExecOptions
  ): Promise<ExecResult>;
}

/**
 * 工具输入参数（混合模式）
 */
export interface ToolInput {
  /**
   * 方式 1: 结构化参数（推荐）
   *
   * 优点：
   * - 类型安全
   * - 自动验证
   * - 防止注入
   */
  args?: Record<string, any>;

  /**
   * 方式 2: 命令字符串（高级用法）
   *
   * 优点：
   * - 灵活性高
   * - 迁移方便
   *
   * 注意：
   * - 无类型检查
   * - 需要自行验证安全性
   *
   * 支持的语法：
   * - 单引号: 'app name'
   * - 双引号: "app name"
   * - 转义: \
   *
   * @example
   * "tool --name 'My App' --path /home/user"
   * "tool --config \\\"key=value\\\""
   */
  command?: string;
}

/**
 * 执行选项
 */
export interface ExecOptions {
  /**
   * 执行模式
   * - sync: 同步执行，等待结果返回
   * - async: 异步执行，通过回调接收事件
   */
  mode?: 'sync' | 'async';

  /**
   * 超时时间（毫秒）
   * - 0 表示无限制
   * - 默认 30000 (30秒)
   */
  timeout?: number;

  /**
   * 工作目录
   * - 默认为当前进程的工作目录
   */
  cwd?: string;

  /**
   * 环境变量
   * - 默认继承父进程的环境变量
   */
  env?: Record<string, string>;

  /**
   * 事件回调（异步模式）
   */
  onEvent?: (event: ToolEvent) => void;
}

/**
 * 执行结果
 */
export interface ExecResult {
  /**
   * 退出码
   * - 0: 成功
   * - 非 0: 失败
   */
  exitCode: number;

  /**
   * 标准输出（如果捕获）
   */
  stdout?: string;

  /**
   * 标准错误（如果捕获）
   */
  stderr?: string;

  /**
   * 执行耗时（毫秒）
   */
  duration?: number;
}

/**
 * 工具事件（异步模式）
 */
export interface ToolEvent {
  /**
   * 事件类型
   * - stdout: 标准输出
   * - stderr: 标准错误
   * - exit: 进程退出
   * - error: 错误事件
   */
  type: 'stdout' | 'stderr' | 'exit' | 'error';

  /**
   * 事件数据
   */
  data?: string;

  /**
   * 退出码（仅 exit 事件）
   */
  exitCode?: number;

  /**
   * 错误码（仅 error 事件）
   */
  errorCode?: number;

  /**
   * 时间戳
   */
  timestamp: number;
}
```

---

## 完整实现

### NAPI 绑定层

```cpp
// frameworks/js/napi/js_cli_manager.cpp

#include "js_cli_manager.h"
#include "command_parser.h"
#include <napi/native_api.h>

namespace OHOS {
namespace CliMgr {

/**
 * @brief 解析 JS ToolInput 对象
 */
static napi_value ParseToolInput(napi_env env, napi_value input, ToolInput& outInput)
{
    // 检查 input 类型
    napi_valuetype type;
    napi_typeof(env, input, &type);

    if (type == napi_object) {
        // 检查是否有 args 属性
        napi_value argsValue;
        napi_status status = napi_get_named_property(env, input, "args", &argsValue);

        if (status == napi_ok) {
            napi_valuetype argsType;
            napi_typeof(env, argsValue, &argsType);

            if (argsType == napi_object) {
                // 方式 1: 结构化参数
                outInput.mode = InputMode::ARGS;
                outInput.args = ParseJsObject(env, argsValue);
                return nullptr;
            }
        }

        // 检查是否有 command 属性
        napi_value commandValue;
        status = napi_get_named_property(env, input, "command", &commandValue);

        if (status == napi_ok) {
            napi_valuetype commandType;
            napi_typeof(env, commandValue, &commandType);

            if (commandType == napi_string) {
                // 方式 2: 命令字符串
                outInput.mode = InputMode::COMMAND;

                size_t len = 0;
                napi_get_value_string_utf8(env, commandValue, nullptr, 0, &len);
                outInput.command.resize(len);
                napi_get_value_string_utf8(env, commandValue,
                                         &outInput.command[0], len + 1, &len);

                // 验证命令安全性
                if (!CommandParser::ValidateCommand(outInput.command)) {
                    napi_throw_error(env, nullptr, "Command contains dangerous characters");
                    return nullptr;
                }

                return nullptr;
            }
        }
    }

    napi_throw_error(env, nullptr, "Invalid ToolInput: must have 'args' or 'command'");
    return nullptr;
}

/**
 * @brief execTool 实现
 */
static napi_value ExecTool(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value argv[3];
    napi_value thisArg;
    void* data;

    napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);

    // 参数 1: name (string)
    std::string toolName = GetStringArg(env, argv[0]);

    // 参数 2: input (ToolInput)
    ToolInput input;
    if (ParseToolInput(env, argv[1], input) != nullptr) {
        return nullptr;  // 错误已抛出
    }

    // 参数 3: options (ExecOptions, 可选)
    ExecOptions options;
    if (argc >= 3) {
        ParseExecOptions(env, argv[2], options);
    }

    // 调用服务端
    auto client = CliManagerClient::GetInstance();
    napi_value promise = CreatePromise(env);

    if (input.mode == InputMode::ARGS) {
        // 结构化参数模式
        client->ExecTool(toolName, input.args, options)
            .Then([env](ExecResult result) {
                ResolvePromise(env, result);
            })
            .Catch([env](ErrorCode error) {
                RejectPromise(env, error);
            });
    }
    else if (input.mode == InputMode::COMMAND) {
        // 命令字符串模式
        client->ExecToolWithCommand(toolName, input.command, options)
            .Then([env](ExecResult result) {
                ResolvePromise(env, result);
            })
            .Catch([env](ErrorCode error) {
                RejectPromise(env, error);
            });
    }

    return promise;
}

} // namespace CliMgr
} // namespace OHOS
```

---

### Service 层实现

```cpp
// services/climgr/src/cli_manager_service.cpp

#include "cli_manager_service.h"
#include "command_parser.h"
#include "process_manager.h"

namespace OHOS {
namespace CliMgr {

ErrorCode CLIManagerService::ExecTool(
    const std::string& toolName,
    const ToolInput& input,
    const ExecOptions& options,
    sptr<ICLICallback>& callback)
{
    // 1. 获取工具信息
    ToolInfo toolInfo;
    if (!registryManager_->GetToolInfo(toolName, toolInfo)) {
        TAG_LOGE(TAG, "Tool not found: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_FOUND;
    }

    // 2. 创建执行上下文
    auto context = std::make_shared<ExecutionContext>();
    context->taskId = GenerateTaskId();
    context->toolName = toolName;
    context->mode = options.mode;
    context->timeoutMs = options.timeout;
    context->workingDir = options.cwd;
    context->caller = GetCallerInfo();
    context->callback = callback;

    // 3. 根据输入类型构建参数
    if (input.mode == InputMode::ARGS) {
        // 方式 1: 结构化参数
        TAG_LOGI(TAG, "Using structured args mode");
        context->arguments = input.args;
        context->argvStr = ParameterMapper::MapToCommandLine(toolInfo, input.args);
    }
    else if (input.mode == InputMode::COMMAND) {
        // 方式 2: 命令字符串
        TAG_LOGI(TAG, "Using command string mode: %{public}s", input.command.c_str());

        // 解析命令字符串
        std::vector<std::string> parsedArgs =
            CommandParser::ParseCommand(input.command);

        // 构建 argv[0]
        context->argvStr.push_back(toolInfo.executablePath);
        context->argvStr.insert(context->argvStr.end(),
                              parsedArgs.begin(), parsedArgs.end());
    }

    // 4. 转换为 C 风格字符串数组
    context->argv.clear();
    for (auto& str : context->argvStr) {
        context->argv.push_back(strdup(str.c_str()));
    }
    context->argv.push_back(nullptr);

    // 5. 记录日志
    std::string cmdLine;
    for (size_t i = 0; i < context->argvStr.size(); ++i) {
        if (i > 0) cmdLine += " ";
        cmdLine += context->argvStr[i];
    }
    TAG_LOGI(TAG, "Executing: %{public}s", cmdLine.c_str());

    // 6. 启动进程
    return processManager_->SpawnProcess(toolInfo, context);
}

} // namespace CliMgr
} // namespace OHOS
```

---

### Process Manager 实现

```cpp
// services/climgr/src/process_manager.cpp

pid_t ProcessManager::SpawnProcess(
    const ToolInfo& toolInfo,
    const std::shared_ptr<ExecutionContext>& context)
{
    // 创建管道
    if (!CreatePipes(*context)) {
        return -1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        TAG_LOGE(TAG, "Failed to fork: %{public}d", errno);
        CloseAllPipes(*context);
        return -1;
    }

    if (pid == 0) {
        // === 子进程 ===
        PrepareChildProcess(*context);

        // 设置环境变量
        char* env[] = { nullptr };

        // 直接使用 context->argv
        execve(toolInfo.executablePath.c_str(),
               context->argv.data(),
               env);

        // 如果 execve 返回，说明执行失败
        TAG_LOGE(TAG, "execve failed: %{public}d", errno);
        _exit(127);
    }

    // === 父进程 ===
    context->processId = pid;
    context->startTime = std::chrono::steady_clock::now();
    context->isRunning = true;

    TAG_LOGI(TAG, "Tool process spawned: pid=%{public}d, tool=%{public}s",
             pid, toolInfo.name.c_str());

    // 注册 I/O 监控
    ioMonitor_->RegisterTask(context);

    return pid;
}

void ProcessManager::PrepareChildProcess(ExecutionContext& context)
{
    // 重定向 stdout
    if (context.stdoutPipe[1] != STDOUT_FILENO) {
        dup2(context.stdoutPipe[1], STDOUT_FILENO);
    }

    // 重定向 stderr
    if (context.stderrPipe[1] != STDERR_FILENO) {
        dup2(context.stderrPipe[1], STDERR_FILENO);
    }

    // 关闭所有不需要的文件描述符
    close(context.stdoutPipe[0]);
    close(context.stderrPipe[0]);

    if (context.stdinPipe[0] >= 0) {
        dup2(context.stdinPipe[0], STDIN_FILENO);
        close(context.stdinPipe[1]);
    } else {
        int nullFd = open("/dev/null", O_RDONLY);
        if (nullFd >= 0) {
            dup2(nullFd, STDIN_FILENO);
            close(nullFd);
        }
    }

    setsid();
}
```

---

## 安全考虑

### 命令注入防护

#### 1. 危险字符检测

```cpp
bool CommandParser::ValidateCommand(const std::string& command)
{
    // 禁止的危险字符序列
    const std::vector<std::string> dangerous = {
        ";",     // 命令分隔符
        "|",     // 管道
        "&",     // 后台运行
        "$(",    // 命令替换
        "`",     // 命令替换
        ">",     // 输出重定向
        "<",     // 输入重定向
        "\n",    // 换行（命令注入）
        "\r",    // 回车
        "\t",    // 制表符
    };

    for (const auto& seq : dangerous) {
        if (command.find(seq) != std::string::npos) {
            TAG_LOGE(TAG, "Dangerous sequence detected: %{public}s",
                     seq.c_str());
            return false;
        }
    }

    return true;
}
```

#### 2. 白名单机制

```cpp
class CommandWhitelist {
public:
    static bool IsCommandAllowed(const std::string& command) {
        // 提取命令名（第一个空格前的部分）
        size_t spacePos = command.find(' ');
        std::string cmdName = (spacePos != std::string::npos)
            ? command.substr(0, spacePos)
            : command;

        // 检查白名单
        static const std::vector<std::string> whitelist = {
            "startTool",
            "stopTool",
            "deploy",
            "net-scan"
        };

        return std::find(whitelist.begin(), whitelist.end(), cmdName)
               != whitelist.end();
    }
};
```

#### 3. 参数长度限制

```cpp
bool CommandParser::ValidateCommand(const std::string& command)
{
    // 长度限制
    const size_t MAX_COMMAND_LENGTH = 4096;
    if (command.length() > MAX_COMMAND_LENGTH) {
        TAG_LOGE(TAG, "Command too long: %{public}zu > %{public}zu",
                 command.length(), MAX_COMMAND_LENGTH);
        return false;
    }

    // 参数数量限制
    auto args = ParseCommand(command);
    const size_t MAX_ARG_COUNT = 64;
    if (args.size() > MAX_ARG_COUNT) {
        TAG_LOGE(TAG, "Too many arguments: %{public}zu > %{public}zu",
                 args.size(), MAX_ARG_COUNT);
        return false;
    }

    return true;
}
```

#### 4. 日志审计

```cpp
class SecurityAudit {
public:
    static void LogCommandExecution(
        const std::string& toolName,
        const std::string& command,
        const CallerInfo& caller)
    {
        TAG_LOGI(TAG,
            "Security Audit: tool=%{public}s, cmd=\"%{public}s\", "
            "uid=%{public}d, pid=%{public}d, bundle=%{public}s",
            toolName.c_str(),
            command.c_str(),
            caller.uid,
            caller.pid,
            caller.bundleName.c_str()
        );

        // 可选：发送到审计服务
        // AuditService::Log({...});
    }
};
```

---

## 使用示例

### 示例 1: 简单参数

```typescript
// 方式 1: 结构化参数（推荐）
const result1 = await CLI.execTool('start-tool', {
  args: {
    bundleName: 'com.example.abc',
    timeout: 30
  }
});

// 方式 2: 命令字符串
const result2 = await CLI.execTool('start-tool', {
  command: "startTool -a com.example.abc -t 30"
});
```

### 示例 2: 带空格的参数

```typescript
// 使用命令字符串处理带空格的参数
await CLI.execTool('file-tool', {
  command: "file-tool --name 'My Document.txt' --path '/home/user/my docs'"
});

// 解析结果
// argv[0] = /usr/bin/file-tool
// argv[1] = --name
// argv[2] = My Document.txt
// argv[3] = --path
// argv[4] = /home/user/my docs
```

### 示例 3: 复杂参数

```typescript
// 使用转义字符处理复杂参数
await CLI.execTool('deploy-tool', {
  command: "deploy-tool --config \\\"app.name=My App\\\" --env \\\"prod=true\\\""
});

// 解析结果
// argv[0] = /usr/bin/deploy-tool
// argv[1] = --config
// argv[2] = "app.name=My App"
// argv[3] = --env
// argv[4] = "prod=true"
```

### 示例 4: 混合使用

```typescript
// 根据场景灵活选择

// 新工具：使用结构化参数（类型安全）
async function startNewTool(bundleName: string, timeout: number) {
  return await CLI.execTool('start-tool', {
    args: {
      bundleName,
      timeout
    }
  });
}

// 遗留工具：使用命令字符串（快速集成）
async function runLegacyTool(config: string) {
  return await CLI.execTool('legacy-tool', {
    command: `legacy-tool --config "${config}"`
  });
}
```

### 示例 5: 异步执行

```typescript
// 异步模式，监听输出事件
const session = await CLI.execTool('log-monitor', {
  command: "log-monitor --file /var/log/app.log --pattern 'ERROR'",
  mode: 'async',
  onEvent: (event) => {
    switch (event.type) {
      case 'stdout':
        console.log('Log:', event.data);
        break;
      case 'exit':
        console.log('Exit code:', event.exitCode);
        break;
    }
  }
});

// 等待完成
await session.wait();
```

---

## 测试用例

### 单元测试

```cpp
// test/unittest/command_parser_test.cpp

#include <gtest/gtest.h>
#include "command_parser.h"

using namespace OHOS::CliMgr;

TEST(CommandParserTest, ParseSimpleCommand) {
    auto args = CommandParser::ParseCommand("tool -a value -b 123");

    ASSERT_EQ(args.size(), 5);
    EXPECT_EQ(args[0], "tool");
    EXPECT_EQ(args[1], "-a");
    EXPECT_EQ(args[2], "value");
    EXPECT_EQ(args[3], "-b");
    EXPECT_EQ(args[4], "123");
}

TEST(CommandParserTest, ParseQuotedString) {
    auto args = CommandParser::ParseCommand("tool --name 'My App' --path /home");

    ASSERT_EQ(args.size(), 5);
    EXPECT_EQ(args[0], "tool");
    EXPECT_EQ(args[1], "--name");
    EXPECT_EQ(args[2], "My App");
    EXPECT_EQ(args[3], "--path");
    EXPECT_EQ(args[4], "/home");
}

TEST(CommandParserTest, ParseEscapedChars) {
    auto args = CommandParser::ParseCommand("tool --msg \\\"Hello\\World\\\"");

    ASSERT_EQ(args.size(), 3);
    EXPECT_EQ(args[0], "tool");
    EXPECT_EQ(args[1], "--msg");
    EXPECT_EQ(args[2], "\"HelloWorld\"");
}

TEST(CommandParserTest, ValidateSafeCommand) {
    EXPECT_TRUE(CommandParser::ValidateCommand("tool -a value -b 123"));
}

TEST(CommandParserTest, ValidateDangerousCommand) {
    // 命令注入尝试
    EXPECT_FALSE(CommandParser::ValidateCommand("tool -a value; rm -rf /"));
    EXPECT_FALSE(CommandParser::ValidateCommand("tool -a value | cat /etc/passwd"));
    EXPECT_FALSE(CommandParser::ValidateCommand("tool -a value && malicious"));
}
```

---

## 总结与建议

### 方案对比总结

| 特性 | 结构化参数 | 命令字符串 | 混合模式 |
|------|-----------|-----------|----------|
| **类型安全** | ✅ | ❌ | ✅（推荐方式） |
| **参数验证** | ✅ | ❌ | ✅（推荐方式） |
| **防注入** | ✅ | ⚠️ 需要验证 | ✅（推荐方式） |
| **灵活性** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **迁移成本** | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| **推荐度** | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

### 最佳实践建议

1. **优先使用结构化参数**
   - 新开发的工具优先使用
   - 类型安全，自动验证
   - 防止注入攻击

2. **命令字符串作为补充**
   - 用于快速原型开发
   - 遗留工具迁移
   - 复杂参数场景

3. **安全措施**
   - 实现命令验证
   - 使用白名单机制
   - 添加审计日志

4. **渐进式迁移**
   - 从结构化参数开始
   - 需要时切换到命令字符串
   - 避免混合使用造成混乱

5. **文档和示例**
   - 提供清晰的使用指南
   - 说明两种模式的适用场景
   - 提供充分的示例代码

---

## 参考资料

- [DESIGN.md](./DESIGN.md) - 完整设计文档
- [argument_mapping_design.md](./argument_mapping_design.md) - 参数映射设计
- [cli_specification.md](./cli_specification.md) - CLI 工具开发规范

---

**文档版本**: v1.0
**最后更新**: 2025-04-13
**作者**: CLI 工具管理框架设计团队
