# cli_tool_framework 解读

## 1. 总体定位

`cli_tool_framework` 是当前工程里负责 CLI Tool 管理和执行的子框架。它把「工具元数据注册/查询」「工具执行」「会话订阅」「stdin/stdout/stderr 事件转发」「脚本 Skill 分流」封装成一个系统服务 `climgr`，再通过 inner API 和 N-API 暴露给 native/JS/ETS 调用方。

核心链路可以理解为：

```text
JS/ETS API
  -> climanager_napi / scriptmanager_napi
  -> CliToolMGRClient
  -> ICliToolManager Binder IPC
  -> CliToolManagerService(climgr SA)
  -> CliToolDataManager / ProcessManager / IOMonitor / EventDispatcher
  -> 子进程 claw_sandbox 或 AbilityManager Skill 执行
```

这个目录和你后续做 CLI manager 最相关的部分主要在：

- `services/climgr`: CLI Manager 系统服务主体。
- `interfaces/cli_tool`: IDL、客户端代理、Parcelable 数据结构。
- `frameworks/js/napi/cli_tool_manager`: JS/ETS 侧 `app.cli.cliManager` N-API 包装。
- `frameworks/js/napi/script_manager`: `app.ability.scriptManager`，用于 ArkTS script 补全相关能力。
- `test/unittest`: CLI 侧单测覆盖较多，适合后续改动时补验证。

## 2. 目录结构

```text
cli_tool_framework/
  etc/profile/
    aimgr.cfg
    aimgr_trust.json
    BUILD.gn
  frameworks/js/napi/
    cli_tool_manager/
    script_manager/
  interfaces/cli_tool/
    ICliToolManager.idl
    ICliToolManagerScheduler.idl
    ICliToolData.idl
    include/
    src/
  services/
    climgr/
    common/
  test/unittest/
```

`etc/profile` 放系统能力相关 profile。`interfaces/cli_tool` 负责 IPC 接口和公共数据模型。`services/climgr` 是真正的服务实现。`frameworks/js/napi` 把 native client 包成 JS 模块。`services/common` 当前提供权限和配置类工具。`test/unittest` 覆盖数据模型、client、service、事件、进程、权限、ToolUtil 等关键模块。

## 3. 构建产物

主要构建目标：

- `interfaces/cli_tool:cli_tool_manager_interface`: 由 IDL 生成 `ICliToolManager` 和 `ICliToolManagerScheduler` 的 proxy/stub。
- `interfaces/cli_tool:cli_tool_client`: inner API 客户端库，包含 `CliToolMGRClient`、数据结构和回调管理。
- `services/climgr:climgr`: SA 共享库，`shlib_type = "sa"`，依赖 AbilityManager、AppManager、AccessToken、FFRT、Distributed KV、SA 框架等。
- `frameworks/js/napi/cli_tool_manager:climanager_napi`: 安装到 `module/app/cli`，模块名 `app.cli.cliManager`。
- `frameworks/js/napi/script_manager:scriptmanager_napi`: 模块名 `app.ability.scriptManager`。

## 4. IPC 接口

`ICliToolManager.idl` 是服务能力边界：

- 工具查询: `GetAllToolSummaries`、`GetToolInfoByName`、`GetAllToolInfos`
- 工具注册: `RegisterTool`
- 工具执行: `ExecTool`
- 会话管理: `SubscribeSession`、`UnsubscribeSession`、`ClearSession`、`QuerySession`
- 会话输入: `SendMessage`
- 权限查询: `BatchQueryPermissionBySubCommand`

`ICliToolManagerScheduler.idl` 是反向回调：

- `SchedulerSessionEvent`: stdout/stderr/exit/error 等会话事件。
- `SchedulerInputReplyEvent`: `SendMessage` 写 stdin 的结果。
- `SchedulerExecToolReplyEvent`: `ExecTool` 初始或最终结果。

`ICliToolData.idl` 定义批量权限查询用的 `Command` 和 `CommandPermission`。

## 5. 关键数据模型

`ToolInfo` 是工具完整元数据：

- `name`、`version`、`description`
- `executablePath`
- `requirePermissions`
- `inputSchema`、`outputSchema`
- `eventTypes`、`eventSchemas`
- `hasSubCommand`、`subcommands`

`SubCommandInfo` 描述子命令自己的权限、schema 和事件。`ToolSummary` 用于轻量列表查询。`ToolsRawData` 用 JSON/raw data 形式传输工具列表，避免大列表逐项 Parcelable 带来的开销。

执行相关模型：

- `ExecToolParam`: `toolName`、`subcommand`、`challenge`、`ExecOptions`、`WantParams args`
- `ExecOptions`: `background`、`yieldMs`、`timeout`
- `CliSessionInfo`: `sessionId`、`toolName`、`status`、可选 `ExecResult`
- `CliToolEvent`: `type`、`eventData`、`exitCode`、`timestamp`

## 6. 服务核心

`CliToolManagerService` 是 `SystemAbility + CliToolManagerStub`。

启动时：

1. 注册 `SIGCHLD` handler。
2. 创建并启动 `IOMonitor`。
3. 给 `IOMonitor` 设置 stdout/stderr、stdin 写入结果、fd 关闭、输出 drain 回调。
4. `CliToolDataManager::EnsureToolsLoaded()` 从配置目录加载工具信息。
5. `Publish(cliService)` 发布 SA。
6. 启动延迟卸载任务。

空闲卸载逻辑：

- `interfaceCalledCount_` 记录正在执行的接口调用。
- `sessionRecords_` 记录仍活跃的会话。
- `OnIdle` 和 `DelayUnloadTask` 都要求调用数为 0 且 session 数为 0。
- 当前延迟卸载间隔是 30 秒。

服务内主要协作对象：

- `CliToolDataManager`: 工具元数据存取，底层是 Distributed KV。
- `SessionRecord`: 单个执行会话状态、结果、管道 fd、输出缓存。
- `ProcessManager`: 创建管道、fork 子进程、exec `claw_sandbox`、kill 进程组。
- `IOMonitor`: epoll 监控 stdout/stderr/stdin，异步写 stdin。
- `EventDispatcher`: 保存 caller scheduler 和 session subscriber，分发 IPC 回调。
- `ToolUtil`: schema 校验、参数转换、sandbox config 生成、Skill 参数处理。

## 7. 执行调用链

普通 CLI 执行路径：

```text
JSCliManager::ExecTool
  -> CliToolMGRClient::ExecTool
  -> ICliToolManager::ExecTool(param, eventId, scheduler)
  -> CliToolManagerService::ExecTool
  -> ValidateExecToolPermissions
  -> EventDispatcher::SetScheduler
  -> TryDispatchSkillSession
  -> ValidateSessionLimit
  -> ValidateAndPrepareTool
       - CliToolDataManager::GetToolByName
       - ToolUtil::ValidateProperties
       - ToolUtil::GenerateSandboxConfig
  -> SetupAndStartSession
       - CreateSessionRecord
       - ProcessManager::CreateChildProcess
       - RegisterSessionWithMonitors
       - RegisterAppStateObserver
       - background 模式立即回调 session
```

`ProcessManager::CreateChildProcess` 会创建 stdin/stdout/stderr pipe，子进程执行：

```text
/system/bin/claw_sandbox --config <sandboxConfig> --cmd <toolInfo.executablePath> [subcommand] [--arg value...]
```

`ToolUtil::TransferToCmdParam` 把 `WantParams` 转成 CLI 参数：

- boolean true 变成 `--key`
- array 展开成多组 `--key value`
- string/int/long/float/double 变成 `--key value`
- `help=true` 只允许单独存在

Skill 特殊路径：

- `ToolUtil::IsSkillTool()` 当前只识别 `ohos-arkTSScript`。
- 如果 AbilityManager 查询到的 skill type 不是 independent，则走 `ExecuteInAppSkillWithTokenId`，不会 fork 子进程。
- Skill session 没有 stdin，所以 `SendMessage` 会失败。
- Skill 结果由 `SkillCallbackAdapter::OnExecuteDone` 回填为 `CliSessionInfo`。

## 8. 会话和事件

`SessionRecord` 管会话生命周期：

- 状态: `SPAWNING`、`RUNNING`、`CANCELLING`、`COMPLETED`、`FAILED`
- 类型: `CLI` 或 `SKILL`
- 记录 caller pid/uid、sessionId、toolName、processId、timeout、pipe fd
- 缓存 stdout/stderr，最大 64 KB
- `BuildSessionInfo` 生成对外会话信息

事件流：

```text
子进程 stdout/stderr
  -> IOMonitor::MonitorLoop
  -> record->AppendOutput
  -> EventDispatcher::DispatchIOEvent
  -> ICliToolManagerScheduler::SchedulerSessionEvent
  -> JS callback/on event handler
```

进程退出流：

```text
SIGCHLD
  -> CliToolManagerService::WaitPid
  -> record->SetTerminalResult
  -> 输出 drain 后 FinalizeBackgroundSession
  -> DispatchExitEvent
  -> ClearSessionSubscribers
  -> RemoveSessionRecord
```
异步保存流：
```text
OnExecTool
  创建 Promise result
  创建 asyncTask
  把 asyncTask 传给 DispatchCliTool
  返回 Promise 给 JS
  函数结束
        ↓
DispatchCliTool
  定义 replyCallback，捕获 asyncTask
  把 replyCallback 传给 CliToolMGRClient::ExecTool
        ↓
CliToolMGRClient::ExecTool
  把 callback 包装成 cb
  存到 CliEventReplyManager
  生成 eventId
  调用 proxy->ExecTool
  函数结束
        ↓
CliToolManagerService::ExecTool
  服务端启动工具
  函数也可能已经返回 ERR_OK
        ↓
工具异步执行完成
        ↓
通过 eventId 找到之前保存的 callback
        ↓
调用 cb(result.code, sessionInfo)
        ↓
实际触发 DispatchCliTool 里的 replyCallback
        ↓
replyCallback 通过 PostTask 回 JS 线程
        ↓
asyncTask->ResolveWithNoError(env, jsSession)
        ↓
JS Promise 完成
        ↓
JS await 拿到 session
```
`ExecOptions` 的影响：

- `background=true`: `ExecTool` 很快返回 running session，后续靠订阅事件。
- `background=false + yieldMs>0`: 超过 yield 时间仍未结束时，服务把会话转为 background 并回调一次 running session。
- `timeout=0`: 默认填充为 30 分钟。
- `timeout>30 分钟` 或负数会被拒绝。

## 9. 权限模型

当前服务侧权限检查比较集中：

- 查询工具列表/详情: system app + `ohos.permission.QUERY_CLI_TOOL`
- 执行、订阅、取消、查询 session、发送消息: system app + `ohos.permission.EXEC_CLI_TOOL`
- 批量查询子命令权限: 调用方必须是 native token/SA + `ohos.permission.QUERY_CLI_TOOL`
- `RegisterTool` 服务接口目前直接返回 `ERR_PERMISSION_DENIED`，虽然 `CliToolDataManager` 具备注册能力。

会话操作还会校验 owner：

- `ClearSession`、`SubscribeSession`、`QuerySession`、`SendMessage` 要求当前 caller pid 等于 `SessionRecord::callerPid`。
- 这对多进程代理、跨设备/分布式转发会有影响，后续如果做分布式 CLI session，需要重新设计 session owner 标识，而不是只依赖 pid。

## 10. 工具元数据存储

`CliToolDataManager` 使用 Distributed KV 的 `SingleKvStore` 存工具信息。

主要能力：

- `EnsureToolsLoaded`: 懒加载工具配置。
- `LoadToolsFromDir` / `ParseToolFromJsonFile`: 从配置目录读取 JSON。
- `RegisterTool`: 校验后写入 KV。
- `GetAllToolsRawData` / `GetAllTools` / `GetToolByName`
- `QueryToolSummaries`
- `SyncToolNames`: 清理配置目录里已经不存在的工具。
- `RestoreKvStore`: 数据损坏时尝试恢复。

虽然使用的是 Distributed KV，但配置里是本地 single version store。后续如果做真正多设备工具发现/同步，需要先确认 KV store 的 `syncable`、key 设计、权限和冲突策略。

## 11. JS/N-API 层

`JSCliManager` 暴露：

- `execTool`
- `subscribeSession`
- `clearSession`
- `querySession`
- `sendMessage`
- `getToolInfoByName`
- `queryToolSummaries`
- `queryTools`

辅助类：

- `JsCliSessionEventCallback`: 把 native session event 投递到 JS callback。
- `JsCliEventHandlerManager`: 管事件投递线程/handler。
- `JsCliManagerUtils`: JS/native 对象转换。
- `CliManagerErrorUtils`: native error 到 JS BusinessError 的转换。

`JSScriptManager` 暴露：

- `completeArkTSScriptInApp`
- `completeArkTSScript`

这个模块和 `ohos-arkTSScript` Skill 路径强相关。

## 12. 分布式相关落点

当前和“分布式”最相关的点有三处：

- 工具元数据存储用 Distributed KV，但当前语义更像本地持久化。
- 会话 owner 绑定本机 caller pid，不适合直接跨设备延伸。
- IO/event 分发依赖本机 Binder scheduler 和 fd/epoll，跨设备需要事件桥接层。

如果后续要扩展分布式 CLI manager，可以考虑分层：

1. 元数据同步层: 工具清单、版本、权限、schema、设备能力。
2. 执行调度层: 本地执行、远端执行、回落策略、超时取消。
3. 会话事件层: stdout/stderr/exit/error/input reply 的跨设备事件模型。
4. 安全层: caller 身份、challenge、sandboxConfig、token 映射。
5. owner 模型: 从 `pid` 改成稳定的 caller identity/session owner token。

## 13. 后续改动入口

CLI manager 常见需求和入口：

- 新增对外 API: 改 `ICliToolManager.idl`、`CliToolMGRClient`、`CliToolManagerService`、N-API、测试。
- 调整执行策略: 看 `CliToolManagerService::ExecTool`、`SetupAndStartSession`、`TryDispatchSkillSession`。
- 调整参数校验/schema: 看 `ToolInfo`、`SubCommandInfo`、`ToolUtil::ValidateProperties`。
- 调整 stdin/stdout 交互: 看 `IOMonitor`、`EventDispatcher`、`SessionRecord`。
- 调整权限: 看 `permission_util`、`PermissionQueryUtil`、服务接口开头的校验。
- 调整工具注册/配置加载: 看 `CliToolDataManager` 和 `etc/profile`。

## 14. 注意点

- `RegisterTool` 服务接口当前禁用，若要支持动态注册，需要补权限、调用方范围、持久化策略和测试。
- `Killpg(pid)` 使用负 pid 杀进程组，需要确认子进程是否真的被放入独立进程组；当前 `ProcessManager` 片段里没有看到 `setpgid/setsid`。
- `SessionRecord` 只缓存 64 KB 输出，长输出主要靠事件订阅实时消费。
- `SendMessage` 对 Skill session 不支持。
- `BatchQueryPermissionBySubCommand` 只允许 native/SA 调用，不是普通 JS API 能力。
- `toolsLoaded_` 是懒加载状态，配置刷新和 KV 同步要注意一致性。

## 15. 测试情况

`test/unittest` 覆盖面较好，重点测试目录包括：

- `cli_tool_mgr_service_test`
- `cli_tool_mgr_client_test`
- `cli_tool_data_manager_test`
- `event_dispatcher_test`
- `io_monitor_test`
- `process_manager_test`
- `tool_util_test`
- `tool_info_test`
- `sub_command_info_test`

后续改 CLI manager 时，建议优先补对应模块单测；涉及跨模块调用链时，至少覆盖 service + client/N-API 转换边界。
