下面按“调用从 JS 进来后，错误怎么一路变成开发者看到的 BusinessError / session event”来梳理。先说明：这是按当前工作区代码说的，包含我上一轮对 `ERROR_INNER = 35600050` message 做的细分；如果看原始基线，未映射 native 错误统一是 `"Internal error."`。

**总览**
CLI 现在有三类“报错出口”：

1. **JS 同步 throw**：入参个数、类型、callback 结构不对，直接 `ThrowInvalidParamError/ThrowTooFewParametersError`，通常是 `401`。
   入口在 [js_cli_manager.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp:143)。

2. **Promise reject / async task reject**：参数过了 JS 层，但 client、Binder、climgr SA、数据层返回非 `ERR_OK`，最终走 `CreateCliJsErrorByNativeErr` 转成 JS 错误。
   映射在 [cli_manager_error_utils.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp:25)。

3. **session event / session result**：CLI 进程或 skill 已经启动，后续 stdout/stderr/error/exit、timeout、非 0 exitCode 不会让 API reject，而是进入 `CliSessionInfo.result` 或 `onEvent`。
   session result 构造在 [session_record.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/session_record.cpp:130)。

**错误码层**
native 内部码定义在 [cli_error_code.h](D:/vscode/ability_ability_runtime/cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h:25)：

- `35700000`：`GET_CLI_TOOL_MGR_SERVICE_FAILED`
- `35700001`：`ERR_SESSION_LIMIT_EXCEEDED`
- `35700002`：`ERR_INVALID_PARAM`
- `35700003`：`ERR_NOT_HAP`
- `35700004`：`ERR_NO_INIT`
- `35700005`：`ERR_TOOL_NOT_EXIST`
- `35700007`：`ERR_PERMISSION_DENIED`
- `35700008`：`ERR_NOT_SYSTEM_APP`
- `35700009`：`ERR_CLI_SESSION_NOT_FOUND`
- `35700010`：`ERR_CLI_SEND_MESSAGE`
- `35700011`：`ERR_NOT_SA_CALLER`
- `35700012`：`ERR_NOT_KILL`
- `35700013/17`：function 不存在/已存在
- `35700014/15/16/18`：JSON、文件、KVStore 相关

JS 对外业务码在 [cli_manager_error_utils.h](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/cli_manager_error_utils.h:25)：

- `201`：permission denied
- `202`：not system app
- `401`：invalid param
- `35600030`：tool not found
- `35600031`：session/process limit
- `35600032`：session not found
- `35600033`：send message failed
- `35600050`：inner error

明确映射的 native 错误在 [cli_manager_error_utils.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp:36)。没在这个表里的 native 错误，都会落到 `35600050`。

**JS 入参阶段**
以 `execTool` 为例：

- 参数少于 4 个：同步 throw，`Too few parameters`。
- `toolName` 不是 string 或为空：同步 throw `401`，message `"Tool toolName is required"`。
- `args` 不是 WantParams：同步 throw `401`，message `"Tool args is required"`。
- `challenge` 为空：同步 throw `401`，message `"Tool challenge is required"`。
- `options.background/yieldMs/timeout` 类型不对：同步 throw `401`，message `"Tool options is required"`。

这些都还没进 `CliToolMGRClient`，所以不会经过 `356000xx` 映射。

`execCmd` 类似：`cmd` 为空同步 `401`；`options.env` 不是 string record、`callback.onEvent` 不是 function，也同步 `401`，见 [js_cli_manager_utils.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp:216)。

**Client 阶段**
JS 入参通过后，会进 `CliToolMGRClient`。

例子：`execTool()` 在 client 里先创建 scheduler stub，再加载 climgr SA，再调用 Binder：
[cli_tool_mgr_client.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp:34)

可能失败：

- `EnsureSchedulerStubCreated()` 创建 stub 失败，返回 `ERR_NO_INIT`，JS 看到 `35600050`。
- `GetCliToolMgrProxy()` 加载 climgr SA 失败，返回 `GET_CLI_TOOL_MGR_SERVICE_FAILED`，JS 看到 `35600050`。
- Binder 方法返回非 OK，client 会清理刚注册的 callback/subscription，然后把原错误码交给 N-API 映射。

具体例子：climgr 服务起不来，`queryTools()` 会在 client 返回 `35700000`，N-API reject 为 `35600050`。

**climgr 权限阶段**
服务端入口普遍先查 system app 和权限。

`execTool/execCmd/sendMessage/querySession/clearSession/subscribeSession` 用 `EXEC_CLI_TOOL`，校验在 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:617)。

`queryTools/queryToolSummaries/getToolInfoByName` 用 `QUERY_CLI_TOOL`，见 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:433)。

例子：

- 非 system app 调 `queryTools()`：服务返回 `ERR_NOT_SYSTEM_APP`，JS reject `202`。
- system app 但没有 `ohos.permission.EXEC_CLI_TOOL` 调 `execCmd()`：返回 `ERR_PERMISSION_DENIED`，JS reject `201`。

**execTool 流程**
主入口在 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:756)。

顺序是：

1. system app + `EXEC_CLI_TOOL`。
2. `EventDispatcher::SetScheduler()` 保存 callback remote；失败返回 `ERR_NO_INIT`。
3. 如果是 `ohos-arkTSScript`，先尝试 skill 分流。
4. 校验 session 并发数，超限返回 `ERR_SESSION_LIMIT_EXCEEDED`。
5. 读取 tool 配置、校验 subcommand/args/options。
6. 生成 sandbox config，要求 caller 是 HAP。
7. fork 子进程并注册 IOMonitor。

具体例子：

- toolName 不存在：`GetToolByName` 返回 `ERR_TOOL_NOT_EXIST`，JS reject `35600030`。
- subcommand 不存在：`ToolUtil::ValidateProperties` 返回 `ERR_TOOL_NOT_EXIST`，JS reject `35600030`。
- `timeout < 0` 或 `yieldMs > timeout`：返回 `ERR_INVALID_PARAM`，JS reject `401`。
- caller 不是 HAP：`GenerateSandboxConfig` 失败，返回 `ERR_NOT_HAP`，JS reject `35600050`。
- pipe/fork 失败：`ProcessManager::CreateChildProcess` 返回 `ERR_NO_INIT`，JS reject `35600050`。

子进程启动后：

- `execvp` 失败发生在 child 内，父进程已经返回 OK；开发者不会收到 Promise reject，而是 session 最终 `status = "failed"`，`result.exitCode != 0` 或 stderr 里有内容。
- timeout 也不是 reject；会发 `"error"` event `"session timed out"`，并最终 session result 里 `timeout = true`。

**execCmd 流程**
入口在 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:795)。

和 `execTool` 类似，但没有 tool 配置读取，直接 `/bin/sh -c cmd`。额外点：

- client 先创建 provisional subscription；如果服务端后续失败，会移除。
- 服务端创建 session 后，会调用 `SubscribeSession()` 注册 stdout/stderr/exit 事件。
- `CreateShellProcess` 的 pipe/fork 失败返回 `ERR_NO_INIT`。
- shell 命令本身失败，比如 `cmd = "not_exist_cmd"`，API 不 reject；promise 最终 resolve 一个 `CliSessionInfo`，里面 `status = "failed"`，`result.exitCode` 通常是 shell 的非 0 退出码。

**session 操作**
入口在 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:1009)。

`clearSession(sessionId)`：

- session 不存在：`ERR_CLI_SESSION_NOT_FOUND` -> `35600032`。
- 不是 owner pid：`ERR_PERMISSION_DENIED` -> `201`。
- session 已完成/不可取消：现在也返回 `ERR_CLI_SESSION_NOT_FOUND` -> `35600032`。
- kill 失败：`ERR_NOT_KILL` -> `35600050`。

`querySession(sessionId)`：

- 不存在：`35600032`。
- 不是 owner：`201`。
- 成功：返回 session，失败状态在 `session.status/result` 里体现。

`subscribeSession(sessionId, callback)`：

- callback 结构错：JS 同步 `401`。
- session 不存在或不是 running：`35600032`。
- scheduler/subscriber 注册失败：`ERR_NO_INIT` -> `35600050`。

`sendMessage(sessionId, inputText)`：

- session 不存在：`35600032`。
- session 不是 running、skill session 不支持 stdin、ioMonitor 空：`ERR_CLI_SEND_MESSAGE` -> `35600033`。
- 写入后如果 IOMonitor 异步回复失败，也会通过 input reply 让 promise reject `35600033`。

**查询工具配置**
`queryTools/queryToolSummaries/getToolInfoByName` 走 `CliToolDataManager`。

数据层入口如 `GetToolByName` 在 [cli_tool_data_manager.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp:305)。

例子：

- KVStore 没起来：`ERR_NO_INIT` 或 `ERR_KVSTORE_NOT_READY`，JS 多数看到 `35600050`。
- tool key 不存在：`ERR_TOOL_NOT_EXIST`，JS 看到 `35600030`。
- KV 中 JSON 损坏：`ERR_JSON_PARSE_FAILED`，JS 看到 `35600050`。
- 配置目录打不开：`ERR_FILE_NOT_FOUND`，通常在 SA start 加载阶段记录日志；如果后续查询无数据，开发者多半看到 tool not found 或 inner。

**function / permission 查询**
Function API 目前主要是 inner API，不是 `cli_manager` JS N-API 的公开入口。服务端在 [cli_tool_manager_service.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:496)。

- `RegisterFunction/UnregisterFunction` 只允许 `FOUNDATION_UID`，否则 `ERR_PERMISSION_DENIED`。
- `GetFunctionInfo/GetAllFunctions` 要 system app。
- function 不存在：`ERR_FUNCTION_NOT_EXIST`。
- KVStore 失败：`ERR_NO_INIT/ERR_KVSTORE_ERROR`。
- JSON 损坏：`ERR_JSON_PARSE_FAILED`。

`BatchQueryPermissionBySubCommand` 比较特别：入口参数/权限失败会返回错误；但每条 command 查不到或 DB 错误时，整体仍返回 `ERR_OK`，错误放进 `CommandPermission.queryRet`，见 [permission_query_util.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/services/climgr/src/permission_query_util.cpp:22)。

**script_manager**
`completeArkTSScriptInApp` 不走 `CliManagerErrorCode`，它在 [js_script_manager.cpp](D:/vscode/ability_ability_runtime/cli_tool_framework/frameworks/js/napi/script_manager/src/js_script_manager.cpp:104)。

- 参数少：同步 throw。
- context 不是 ability/extension context：同步 throw `16000020`。
- token 取不到、requestCode 为空、result 解析失败：同步 `401`。
- `AbilityManagerClient::ExecuteSkillDone` 返回非 OK：Promise reject，使用 ability runtime 通用 `CreateJsErrorByNativeErr`。

`completeArkTSScript` 当前直接不支持，同步 throw `401`，message `"completeArkTSScript is not supported yet"`。

**最容易混淆的点**
`API 调用失败` 和 `命令执行失败` 不是一回事。

- API 调用失败：权限、参数、tool 不存在、session 不存在、服务未初始化，通常 Promise reject。
- 命令执行失败：进程已启动但 exitCode 非 0、timeout、stderr 输出，通常 Promise resolve 一个 failed session，或通过 callback 收到 error/exit event。

所以后面改代码时，先判断你要改善的是“调用阶段 BusinessError”，还是“session/result/event 的执行结果”。这两个出口不在同一层。