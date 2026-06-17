# AGENTS.md

本文件是面向后续编码代理的项目初始化说明。当前仓库是 OpenHarmony
`ability_runtime` 组件，但本工作区的重点责任边界是：

- `cli_tool_framework`
- `agent_runtime_framework`

除非任务明确要求，不要把改动扩散到根目录下其它 ability 子系统。需要理解系统依赖时可以阅读
`services/abilitymgr`、`services/appmgr`、`interfaces/inner_api` 等公共能力，但实现优先收敛在上面两个目录。

## 快速定位

CLI Tool 框架主链路：

```text
JS/ETS API
  -> frameworks/js/napi/cli_tool_manager or script_manager
  -> CliToolMGRClient
  -> ICliToolManager Binder IPC
  -> CliToolManagerService (climgr SA)
  -> CliToolDataManager / CliFunctionDataManager / ProcessManager / IOMonitor / EventDispatcher
  -> claw_sandbox child process or AbilityManager skill execution
```

Agent Runtime 框架主链路：

```text
ArkTS agentManager API
  -> ETS ANI / JS NAPI bridge
  -> AgentManagerClient / AgentConnectionManager
  -> IAgentManager Binder IPC
  -> AgentManagerService (agentmgr SA)
  -> AgentCardMgr / AgentCardDbMgr / AbilityManagerClient
  -> AgentExtensionAbility + IAgentConnector / IAgentReceiver IPC
```

## CLI 子系统

重点目录：

- `cli_tool_framework/interfaces/cli_tool`: IDL、inner API client、Parcelable/rawdata 数据模型。
- `cli_tool_framework/services/climgr`: `climgr` SystemAbility 服务实现。
- `cli_tool_framework/services/common`: CLI 权限与配置工具。
- `cli_tool_framework/frameworks/js/napi/cli_tool_manager`: `app.cli.cliManager` N-API。
- `cli_tool_framework/frameworks/js/napi/script_manager`: ArkTS script/skill 补全相关 N-API。
- `cli_tool_framework/test/unittest`: CLI 侧单测入口。
- `cli_tool_framework/etc/profile`: SA profile 和 trust 配置。

核心接口文件：

- `interfaces/cli_tool/ICliToolManager.idl`
- `interfaces/cli_tool/ICliToolManagerScheduler.idl`
- `interfaces/cli_tool/ICliToolData.idl`
- `interfaces/cli_tool/include/cli_tool_mgr_client.h`
- `services/climgr/include/cli_tool_manager_service.h`

`CliToolManagerService` 的关键行为：

- `OnStart()` 初始化 `IOMonitor`，加载 CLI tool 配置，初始化 function DB，发布 SA。
- `ExecTool()` 先做 system app 和 `ohos.permission.EXEC_CLI_TOOL` 校验，再设置 caller scheduler。
- `TryDispatchSkillSession()` 会把 `ohos-arkTSScript` 且非 independent 的 skill 分流到 AbilityManager skill 执行路径。
- 普通 CLI 执行走 `ValidateAndPrepareTool()`、`ToolUtil::GenerateSandboxConfig()`、`ProcessManager::CreateChildProcess()`。
- 会话状态、stdout/stderr/stdin、退出事件由 `SessionRecord`、`IOMonitor`、`EventDispatcher` 协作维护。
- `RegisterTool()` 当前服务端直接返回 `ERR_PERMISSION_DENIED`；不要默认它已经开放动态注册。
- Function 注册/反注册当前只允许 `FOUNDATION_UID` 调用，查询要求 system app。

CLI 数据与限制：

- Tool 元数据从 `/system/bin/cli_tool/configs` 的 JSON 加载到 Distributed KV。
- KV store 使用 single version，本地持久化语义更强，当前不是跨设备同步注册中心。
- Session owner 当前绑定 caller pid/uid；跨进程代理或分布式会话扩展不能直接复用这套 owner 判断。
- Skill session 不支持 stdin，`SendMessage()` 对 skill session 会失败。

## Agent 子系统

重点目录：

- `agent_runtime_framework/interfaces/inner_api`: `IAgentManager`、client、connection manager、`AgentCard`。
- `agent_runtime_framework/interfaces/kits/native/agent_extension/connection`: `IAgentConnector` / `IAgentReceiver` 双向 IPC。
- `agent_runtime_framework/services/agentmgr`: `agentmgr` SystemAbility、card 管理、连接复用。
- `agent_runtime_framework/services/common`: AgentCard 校验和 semver 工具。
- `agent_runtime_framework/frameworks/ets/ets`: ArkTS 对外声明和应用侧类型。
- `agent_runtime_framework/frameworks/ets/ani`: ETS native bridge。
- `agent_runtime_framework/frameworks/js/napi`: JS N-API bridge。
- `agent_runtime_framework/frameworks/native`: AgentExtension / AgentUIExtension native 基类和 loader。

核心接口文件：

- `interfaces/inner_api/IAgentManager.idl`
- `interfaces/inner_api/include/agent_manager_client.h`
- `interfaces/inner_api/include/agent_connection_manager.h`
- `interfaces/inner_api/include/agent_card.h`
- `services/agentmgr/include/agent_manager_service.h`
- `services/agentmgr/include/agent_card_mgr.h`
- `services/agentmgr/include/agent_card_db_mgr.h`

AgentCard 规则：

- Bundle 安装/更新时，`AgentCardMgr` 从 `ohos.extension.agent` metadata 导入 card。
- 只处理 `ExtensionAbilityType::AGENT`。
- 每个 bundle/user 最多 1000 个 card。
- API 注册/更新 card 需要合法 semver，且更新版本不能降级。
- API 写入的 card source 为 `API`，Bundle metadata 写入 source 为 `BUNDLE`；source 会影响后续安装包更新覆盖策略。
- Low-code card 只能接收 system app 目标 bundle。

`AgentManagerService` 的关键行为：

- `OnStart()` 创建 task handler 和 `AgentEventHandler`，发布 `agentmgr` SA，监听 BundleMgr SA。
- card 查询需要 system API caller 和 `PERMISSION_GET_AGENT_CARD`。
- card 修改需要 system API caller 和 `PERMISSION_MODIFY_AGENT_CARD`。
- AgentExtension 连接需要 system API caller、`PERMISSION_CONNECT_AGENT`、非空 connection，并要求 caller 进程在前台。
- ServiceExtension 连接走 caller token 路径，并校验目标 extension type 为 `SERVICE`。

Agent 连接模型：

- 标准 APP/Atomic Service agent 使用 `StandardAgentSession` 去重。
- 标准连接 key 是 `callerUid + agentId + bundleName + moduleName + abilityName`。
- Atomic Service agent 会附加 install-on-demand flag 和启动时间参数。
- Low-code agent 使用共享 host session：`AgentHostKey` 是 `userId + bundleName + moduleName + abilityName`。
- Low-code active 去重使用 `AgentOwnerKey { hostUid, agentId }`，不是单纯的 `agentId`。
- `NotifyLowCodeAgentComplete()` 以 calling uid + agentId 找 owner，并在最后一个 agent 完成后断开共享 host connection。
- `trackedConnections_`、`callerConnectionCounts_`、`agentHostSessions_`、`agentOwners_` 必须保持一致；改连接生命周期时优先补测试或写清楚验证路径。

Agent 双向通信：

- Host -> AgentExtension 使用 `IAgentConnector::SendData()` / `Authorize()`。
- AgentExtension -> Host 使用 `IAgentReceiver::SendData()` / `Authorize()` / `AgentInvoked()`。
- JS/ETS bridge 会把 native proxy/stub 包装成 ArkTS 侧 `AgentProxy` 和连接回调。

## 做改动时的顺序

新增或修改 CLI 对外 API 时，通常需要同步检查：

1. `interfaces/cli_tool/*.idl`
2. `interfaces/cli_tool/include` 和 `interfaces/cli_tool/src`
3. `services/climgr/include` 和 `services/climgr/src`
4. `frameworks/js/napi/cli_tool_manager` 或 `script_manager`
5. `BUILD.gn`
6. `cli_tool_framework/test/unittest`

新增或修改 Agent 对外 API 时，通常需要同步检查：

1. `interfaces/inner_api/IAgentManager.idl`
2. `interfaces/inner_api/include` 和 `interfaces/inner_api/src`
3. `services/agentmgr/include` 和 `services/agentmgr/src`
4. `frameworks/ets/ets`
5. `frameworks/ets/ani` 和/或 `frameworks/js/napi`
6. `BUILD.gn`
7. 需要时新增测试；当前 agent 目录下没有像 CLI 一样集中的 unittest 子树。

## 工程约束

- 保持现有 C++ 命名、namespace、`TAG_LOG*`、`ERR_*` 风格。
- 服务接口入口先做权限、caller 类型和空指针校验，再进入有状态逻辑。
- 大批量列表传输优先沿用 rawdata 模型，例如 `ToolsRawData`、`AgentCardsRawData`。
- 分布式 KV 当前主要承担本地持久化；不要直接假设跨设备同步语义。
- 改 caller 身份模型时要同时考虑 pid、uid、remote object、death recipient 和 session owner。
- Low-code agent 连接复用和 quota 转移比较脆弱，改动时重点检查失败回滚、死亡清理、重复激活和断开通知。
- CLI 后台会话依赖 stdout/stderr drain 后清理，处理超时、kill、yield 时不要提前丢 session 事件。
- 不要无关重构整个 ability_runtime；只在任务需要时碰公共 ability/app manager 代码。

## 构建和测试

常用构建目标在 GN 文件中：

- CLI client/interface: `cli_tool_framework/interfaces/cli_tool:cli_tool_client`
- CLI SA: `cli_tool_framework/services/climgr:climgr`
- CLI N-API: `cli_tool_framework/frameworks/js/napi/cli_tool_manager:climanager_napi`
- Agent fwk/client/interface: `agent_runtime_framework/interfaces/inner_api:agent_fwk`、`:agent_manager`、`:agent_connection_manager`
- Agent SA: `agent_runtime_framework/services/agentmgr:agentmgr`
- Agent N-API/ANI: `agent_runtime_framework/frameworks/js/napi/agent_manager:agentmanager_napi` and ETS ANI targets

CLI 单测 group：

```bash
run -t UT -tp ability_runtime
run -t UT -ts cli_tool_mgr_service_test
run -t UT -ts cli_tool_mgr_client_test
run -t UT -ts cli_tool_data_manager_test
run -t UT -ts tool_util_test
run -t UT -ts event_dispatcher_test
run -t UT -ts io_monitor_test
run -t UT -ts process_manager_test
```

完整组件构建按 OpenHarmony 环境执行：

```bash
./build.sh --product-name <product> --build-target ability_runtime
```

本地快速阅读建议：

```bash
rg --files cli_tool_framework agent_runtime_framework
rg -n "ExecTool|RegisterFunction|ConnectAgentExtensionAbility|NotifyLowCodeAgentComplete" cli_tool_framework agent_runtime_framework
```

## 额外备注

仓库根目录存在 `CLI_TOOL_FRAMEWORK_ANALYSIS.md` 和 `AGENT_RUNTIME_FRAMEWORK_ANALYSIS.md`，可作为索引，但代码始终是准绳。阅读中文文档时如出现乱码，优先直接看 `.h`、`.cpp`、`.ets`、`.idl` 和 `BUILD.gn`。
