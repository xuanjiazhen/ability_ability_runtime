# CLI Manager 错误信息优化说明

## 背景

`app.cli.cliManager` 的 JS API 原先会把 native 层错误码转换成固定的业务错误信息。部分错误信息过于笼统，开发者很难直接判断问题发生在哪个参数、哪个工具、哪个会话或哪个内部阶段。

典型问题如下：

- `Permission denied.` 未说明缺少哪个权限。
- `The tool does not exist.` 未说明具体是哪个 `toolName` 或 `subcommand`。
- `Internal error.` 未说明内部失败原因。

本次修改的目标是：在不改变 JS API 对外错误码的前提下，完善错误 `message`，让开发者可以根据错误信息更快定位问题。

## 原有错误链路

原有链路如下：

1. CLI service 或 client 返回 native `ErrCode`。
2. `js_cli_manager.cpp` 收到错误码。
3. 调用 `CreateCliJsErrorByNativeErr(env, nativeErr)`。
4. `cli_manager_error_utils.cpp` 将 native 错误码映射为 JS API 对外业务错误码。
5. `CreateCliManagerError` 调用 `napi_create_error` 创建 JS `Error`。
6. 异步任务通过 `Reject` 将错误返回给开发者。

例如：

```text
ERR_TOOL_NOT_EXIST -> ERROR_TOOL_NOT_FOUND = 35600030 -> The tool does not exist.
```

原有问题是：错误码和基础文案有了，但缺少调用现场上下文。

## 修改内容

### 1. 支持追加错误上下文

修改 `CreateCliJsErrorByNativeErr` 函数签名，增加可选参数 `errMsgContext`：

```cpp
napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr, const std::string& errMsgContext = "");
```

`errMsgContext` 不改变错误码，只追加到最终 JS Error 的 `message` 后面。

示例：

```text
The tool does not exist. toolName: 'network_tool', subcommand: 'ping'.
```

### 2. 在 `js_cli_manager.cpp` 中补充调用上下文

`js_cli_manager.cpp` 是 JS API 到 native CLI manager 的 NAPI 适配层。它能拿到 JS 调用时的参数，因此适合补充上下文。

本次新增的上下文包括：

- `execTool`：补充 `toolName`、`subcommand`。
- `getToolInfoByName`：补充 `toolName`。
- `subscribeSession`、`clearSession`、`querySession`、`sendMessage`：补充 `sessionId`。
- `queryToolSummaries`、`queryTools`：补充 operation 名称。
- 权限失败时：补充接口需要的框架权限。

示例：

```text
Permission denied. Required permission: ohos.permission.EXEC_CLI_TOOL. toolName: 'network_tool'.
The session not found. sessionId: 'network_tool_1718000000000_123'.
Internal error. CLI tool database is not ready. Try again later. operation: 'queryTools'.
```

### 3. 完善 `ERROR_INNER = 35600050` 的错误信息

对外错误码仍保持 `ERROR_INNER = 35600050`，但当 native 错误码能表达更具体原因时，message 会细化。

当前覆盖的内部错误包括：

| native 错误 | 优化后的含义 |
| --- | --- |
| `GET_CLI_TOOL_MGR_SERVICE_FAILED` | CLI tool manager 服务连接失败 |
| `ERR_NO_INIT` | CLI 运行时组件未初始化或不可用 |
| `ERR_NOT_HAP` | 沙箱配置生成失败，调用方不是 HAP 应用 |
| `ERR_NOT_KILL` | CLI 工具进程终止失败 |
| `ERR_JSON_PARSE_FAILED` | CLI tool 元数据 JSON 解析失败 |
| `ERR_FILE_NOT_FOUND` | CLI tool 配置文件或目录不存在 |
| `ERR_KVSTORE_NOT_READY` | CLI tool 数据库未就绪 |
| `ERR_KVSTORE_ERROR` | CLI tool 数据库操作失败 |
| `ERR_INVALID_VALUE` | 服务返回的 CLI tool 数据解码失败，可能是数据异常或内存分配失败 |

示例：

```text
Internal error. Failed to parse CLI tool metadata JSON. Check the tool configuration JSON format. toolName: 'network_tool'.
```

## 涉及文件

- `include/cli_manager_error_utils.h`
  - `CreateCliJsErrorByNativeErr` 增加可选上下文参数。
- `src/cli_manager_error_utils.cpp`
  - 增加 native 错误码到具体 message 的映射。
  - 在创建 JS Error 前追加上下文。
- `src/js_cli_manager.cpp`
  - 在 reject 错误时传入 tool/session/operation/permission 上下文。
