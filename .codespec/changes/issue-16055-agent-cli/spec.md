# 特性规格

> 固化用户可见行为和验收标准。复杂度：标准。无 InnerKit 接口定义/内部实现流程——实现细节见 design.md。
>
> `THEN` 中涉及内部接口归属、错误码枚举与源码名称的引用为规格不变式佐证，非内部实现断言；机器对内部实现词法的 WARN 已在本特性 `gate-checklist.md` 记录处理结论。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | ohos-agent CLI 工具：连接 AgentExtensionAbility 的长连会话通道 |
| 特性编号 | 20260528392515 |
| 所属 Epic | Agent |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-7.0-Release |
| SIG 归属 | Ability SIG |
| 状态 | Approved |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

> lineage: new（全新特性）。本节跳过。

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md。design.md 与本文档并行产出，互不依赖。

## 用户故事

### US-1: 拉起并长连 APP 类型 Agent

**作为** 框架/系统侧调用方，
**我想要** 经 cliMgr 拉起长驻 `ohos-agent connect` 子进程连接 APP 类型 AgentExtensionAbility，
**以便** 建立可控长连会话。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1 | WHEN 系统应用且持 `ohos.permission.EXEC_CLI_TOOL` 的调用方调 `CliToolMGRClient::ExecTool({toolName:"ohos-agent",subcommand:"connect",args:{bundleName,abilityName,moduleName,agentId},options:{background=true,timeout≤1800}})`（agentId 须已预注册为 `AgentCardType::APP`，见 R-19）THEN 经 System API `ExecTool` 回流，cliMgr fork 子进程；子进程经 `tool_util` 注入 callerIdentity env（`ohos_cli_callerIdentity`=appA 身份串，见 R-17）后调 `ConnectAgentExtensionAbilityForCli(want, connection, callerIdentity)` 直连（InnerAPI），ForCli 内联 `ResolveConnectAgentTarget`+enforce APP（无 preflight），连接 `AgentCardType::APP` AgentExtensionAbility 成功；建立方经 `SubscribeSession` 收到 type=stdout 的 CliToolEvent，**跨块按 `\n` 重组 + JSON 解析得到 `{"type":"connect-done","status":"ok"}`**；子进程长驻不退出、不主动断连。 <!-- ext-ok --> | InnerAPI / 终端用户 | 正常 |
| AC-9 | WHEN `ExecTool` 的 timeout>1800s THEN 经 System API 被拒（超过系统最大长时上限）。 | System API | 边界 |
| AC-12 | WHEN 调用方以 `background=false + yieldMs=Y` 启动 connect 且 session 运行超过 Y THEN 经 System API `SubscribeSession` 观察到 cliMgr `HandleProcessYieldTimeout` 自动 `SetBackground(true)` 并经 `DispatchExecToolReplyEvent` 回流 session info，**不 kill**，session 以后台继续运行；后续 stdout 经 `SubscribeSession` 增量交付（ADR-10 yield 复用零改动；ADR-9 callerIdentity 经 tool_util env 注入）。 <!-- ext-ok --> | System API / 终端用户 | 边界 |

### US-2: 双向数据通信

**作为** 调用方，
**我想要** 经 session 向对端发 sendData/authorize，并接收对端主动推送，
**以便** 双向通信。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-2 | WHEN 建立方调 `CliToolMGRClient::SendMessage(sessionId, inputText='{"op":"sendmessage","data":...}\n')`（InnerAPI）THEN cliMgr 将该 JSON-line 写入 CLI 子进程 stdin（受 4MB/4096 队列上限 + 30s 写超时，超限则 SendMessage 回调失败，见 R-28；**子进程单行 ≤200KB 见 R-10**），子进程 CommandLoop 解析后经 `IAgentReceiver::SendData(connectorProxy,data)`（两参数）把 data 发送至对端 AgentExtensionAbility，对端收到。 <!-- ext-ok --> | InnerAPI / 终端用户 | 正常 |
| AC-3 | WHEN 建立方调 `SendMessage(sessionId, inputText='{"op":"authorize","data":...}\n')`（InnerAPI）THEN 子进程 CommandLoop 解析后经 `IAgentReceiver::Authorize` 把授权 data 透传至对端（data 语义由 Agent 侧定义，CLI 仅透传 opaque data，见 R-18），对端收到。 <!-- ext-ok --> | InnerAPI / 终端用户 | 正常 |
| AC-4 | WHEN 对端 AgentExtensionAbility 经双向通道（`IAgentConnector::SendData(data)` 一参数）主动回调 sendData/authorize THEN CLI 子进程（`AgentConnectorStub`）接收并将数据作为 `{"type":"OnData"/"OnAuthorize",...}` JSON-line 写 stdout，经 cliMgr 以 type=stdout 块 → `SubscribeSession` 交付建立方；建立方跨块重组 + JSON 解析得到事件。 <!-- ext-ok --> | InnerAPI / 终端用户 | 正常 |

### US-3: 断连

**作为** 调用方，
**我想要** 经 stdin disconnect 走正常断连流程，或经 `ClearSession` 强制结束 session，
**以便** 干净或强制释放连接。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-5 | WHEN 建立方经 stdin 下发 `{"op":"disconnect"}` JSON-line THEN CLI 子进程走正常断连流程（子进程调 `DisconnectAgentExtensionAbilityForCli(connection, callerIdentity)`（InnerAPI），agentmgr `SetCallingIdentity(appA)` 后走 `DisconnectAbility`→`OnAbilityDisconnectDone`，不跳过校验），将 `{"type":"disconnect-done","reason":"user"}` JSON-line 写 stdout 经 `SubscribeSession` 交付建立方，干净退出；配额由 agentmgr `ReleaseCallerQuotaLocked` 释放（AC-14）。 <!-- ext-ok --> | InnerAPI / 终端用户 | 正常 |
| AC-13 | WHEN 建立方调 `CliToolMGRClient::ClearSession(sessionId)`（InnerAPI）THEN cliMgr 强制终止子进程（SIGKILL），session 结束；调用方经已订阅的 `SubscribeSession` 收到 exit 事件；agent 侧由 caller-death 清理连接（不走正常 disconnect 流程，无 `disconnect-done`）。建议优先用 stdin disconnect，`ClearSession` 为强制兜底。 <!-- ext-ok --> | InnerAPI / 终端用户 | 异常 |
| AC-15 | WHEN 对端 AgentExtensionAbility 主动断连（`OnAbilityDisconnectDone` 在 CLI Binder 线程触发）THEN CLI 写 `{"type":"disconnect-done","reason":"peer"}` JSON-line，经 wakeup fd（自管道）唤醒主循环 poll，干净退出；配额由 agentmgr `ReleaseCallerQuotaLocked` 释放（AC-14）。 <!-- ext-ok --> | InnerAPI / 终端用户 | 异常 |

### US-4: 安全边界

**作为** 系统，
**我要** 确保仅受信 CLI 进程可连、仅 session 建立方可操作，
**以便** 守住安全边界。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-6 | WHEN 非系统应用或无 `ohos.permission.EXEC_CLI_TOOL` 的调用方调 `ExecTool`（System API）THEN 被拒（`ERR_NOT_SYSTEM_APP`/`ERR_PERMISSION_DENIED`）。 | System API | 异常 |
| AC-7 | WHEN 非 connect 建立方对该 session 调 `SendMessage`/`ClearSession`/`SubscribeSession`（InnerAPI）THEN `IsSessionOwner` 拒绝。 | InnerAPI | 异常 |
| AC-8 | WHEN 非 CLI 进程（`IsCliToolToken`=false）发起 agent connect THEN 走原有 connect 权限/前台校验，行为不变。 <!-- ext-ok --> | InnerAPI | 正常 |
| AC-14 | WHEN 调用方（经 CLI 或 JS）已并发连 5 个不同 Agent 再发起第 6 个 connect THEN 经 System API 返回 `ERR_MAX_AGENT_CONNECTIONS_REACHED`，连接被拒；CLI 与 JS 共享同一调用方配额桶（callerUid=app A uid），不绕过。disconnect 释放后可重连。 <!-- ext-ok --> | System API | 边界 |

### US-5: 特性隔离

**作为** 系统，
**我要** 用 CCM 配置项隔离本特性，
**以便** 按产品启用。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-10 | WHEN `const.product.ohos_agent_cli.enabled`=false THEN 经 System API 观察到 ohos-agent 工具不可用、agentmgr 不接受 ForCli 连接。 | System API | 正常 |
| AC-11 | WHEN 同一 session 内多次 `sendmessage`/`authorize` 指令 THEN 经 InnerAPI 观察到复用同一 `IAbilityConnection` 连接对象与双向 proxy/stub，不重连、不重复建连。 <!-- ext-ok --> | InnerAPI | 正常 |

## 验收追溯

> 本表只维护 Spec 内部的 AC → 规则 → 可观察表面事实。AC → Task 映射由 `execution-plan.md` 的"AC 到 Task 追溯"维护；测试入口、Red 条件和通过标准由下方"验证映射"维护；实际执行证据由 `evidence/reviews/` 维护。

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-1 | R-1, R-7, R-8, R-12, R-19, R-20, R-22 | InnerAPI / 终端用户 |
| AC-2 | R-2, R-7, R-10, R-24, R-28, R-31, R-32 | InnerAPI / 终端用户 |
| AC-3 | R-2, R-7, R-10, R-18, R-28 | InnerAPI / 终端用户 |
| AC-4 | R-3 | InnerAPI / 终端用户 |
| AC-5 | R-4, R-7, R-10, R-20, R-25, R-26, R-29, R-30, R-33 | InnerAPI / 终端用户 |
| AC-6 | R-13, R-21 | System API |
| AC-7 | R-14 | InnerAPI |
| AC-8 | R-15, R-21 | InnerAPI |
| AC-9 | R-9, R-12, R-23 | System API |
| AC-10 | R-16 | System API |
| AC-11 | R-5 | InnerAPI |
| AC-12 | R-12, R-17, R-27 | System API / 终端用户 |
| AC-13 | R-6, R-33 | InnerAPI / 终端用户 |
| AC-14 | R-11 | System API |
| AC-15 | R-4, R-20, R-25, R-33 | InnerAPI / 终端用户 |

## 规则定义

> 统一规则表，取消 FR/BR/EX/RC 四分类。类型标签：**行为**（正常路径下的系统行为）、**边界**（输入/状态的临界点）、**异常**（非法输入或异常状态的处理）、**恢复**（系统异常后的恢复策略）。

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | `ExecTool(connect,background=true)` | connect 单阶段：CLI 调 `ConnectAgentExtensionAbilityForCli(want, connection, callerIdentity)` 直连，ForCli 内联 `ResolveConnectAgentTarget`+enforce APP（无 preflight），建立长驻 session，握手成功后写 connect-done JSON-line | 仅 `AgentCardType::APP` | AC-1 |
| R-2 | 行为 | `SendMessage(sessionId,指令)` | 建立→对端数据发送：sendmessage/authorize 经 stdin 下发，子进程转双向通道 | 子进程单行 ≤200KB（R-10） | AC-2,AC-3 |
| R-3 | 行为 | 对端主动回调 | 对端→建立方数据回流：对端回调经 stdout JSON-line → `SubscribeSession`(type=stdout 块)，建立方跨块重组+JSON 解析 | 跨块按 `\n` 重组 | AC-4 |
| R-4 | 行为 | stdin disconnect 或 ClearSession | disconnect 走正常断连流程并上报 disconnect-done | `DisconnectAgentExtensionAbilityForCli` 经 SetCallingIdentity 后不跳过校验 | AC-5 |
| R-5 | 行为 | 已建连 | 同 session 复用同一连接对象 | 不重连、不重复建连 | AC-11 |
| R-6 | 行为 | 建立方调 ClearSession | ClearSession 强制终止 session | SIGKILL 兜底 | AC-13 |
| R-7 | 行为 | `OnAbilityConnectDone(0)` | connect 成功后进入阻塞式 CommandLoop REPL（poll stdin+self-pipe read fd），逐行解析 JSON-line 命令分派，至 disconnect/B 断连/EOF 退出 | EOF 退出 | AC-1,AC-2,AC-3,AC-5 |
| R-8 | 边界 | 目标 Agent 类型非 `AgentCardType::APP` | connect 仅对 `AgentCardType::APP` 类型 Agent 生效 | 类型由 AgentCard 定义；LOW_CODE/ATOMIC_SERVICE 不支持 | AC-1 |
| R-9 | 边界 | `ExecTool` timeout>1800s | 长时运行受系统最大长时上限（1800s）约束，超限被拒 | `MAX_TIMEOUT=1800` 既有 | AC-9 |
| R-10 | 边界 | stdin 协议为 JSON-line：每条命令一行 `{"op":"sendmessage"|"authorize"|"disconnect",...}` + `\n` 分帧 | cliMgr `SendMessage` 原样写 inputText 不加 `\n`，调用方须自带；子进程按 `\n` 切行 + 跨 4096 块重组；子进程单行 ≤200KB（`MAX_LINE_BYTES`，超限 error(fatal=true) 退出） | `MAX_LINE_BYTES=200*1024`；JSON-line（非纯文本）；已定 JSON-line | AC-2,AC-3,AC-5 |
| R-11 | 边界 | 调用方（callerUid）已连 5 个不同 Agent 再发起第 6 个 connect | 配额与池上限为硬性不变式（详见 `## 配额与池上限规格（不变式）`）：①每调用方(callerUid)最多 5 个不同 Agent；②APP/ATOMIC_SERVICE 按 agentId 计槽；③LOW_CODE 单 ability 最多 100 实例(不分调用方)且该 ability 占调用方 1 槽；④CLI(ForCli) 与 JS/ETS 共享同一 callerUid 配额桶，经既有 `AdmitStandardAgentConnectionLocked`+`MAX_AGENT_CONNECTIONS_PER_CALLER=5`/`MAX_LOW_CODE_AGENTS_PER_HOST=100` 强制，不绕过、不另设账本 | 返回 `ERR_MAX_AGENT_CONNECTIONS_REACHED`；服务侧能力已具备，ForCli 纯委托既有路径，无需新增逻辑 | AC-14 |
| R-12 | 边界 | connect 以 `background=true` 启动；误用 `background=false` 须带 `yieldMs>0` 触发 yield 安全网（AC-12）；`background=false`+`yieldMs=0` 为未支持误用→跑至 MAX_TIMEOUT 被 kill（R-23） | connect 支持路径=background=true；ohos-agent.json connect 声明推荐 yieldMs（如 5s）供调用方参考 | ADR-10 yield 误用兜底 | AC-1,AC-12,AC-9 |
| R-13 | 行为 | 调用方非系统应用或无 `ohos.permission.EXEC_CLI_TOOL` | connect 入口须系统应用 + `ohos.permission.EXEC_CLI_TOOL` | cliMgr `ValidateExecToolPermissions` 既有 | AC-6 |
| R-14 | 行为 | 非 connect 建立方对该 session 调 `SendMessage`/`ClearSession`/`SubscribeSession` | session 操作须建立方（IsSessionOwner） | cliMgr 既有 | AC-7 |
| R-15 | 行为 | 非 CLI 进程（`IsCliToolToken`=false）发起 agent connect | agentmgr 经 ForCli 路径对 `IsCliToolToken`+CCM 特性门禁后处理 connect：`SetCallingIdentity(callerIdentity=appA)` 后对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台校验（不跳过、不放行）；非 CLI 走原校验 | ForCli 防伪：`GetHapTokenInfo(callerIdentity.token).uid`==CLI 子进程 uid | AC-8 |
| R-16 | 行为 | `const.product.ohos_agent_cli.enabled`=false | 特性受 CCM 配置项 `const.product.ohos_agent_cli.enabled` 隔离 | 同时 gate 工具注册消费与 ForCli 入口 | AC-10 |
| R-17 | 行为 | `ExecTool(connect,background=false+yieldMs=Y)` 且 session 运行超 yieldMs | 长驻/background 由 cliMgr 既有 yield-time 机制承担（background=false+yieldMs→自动转后台，不 kill） | 复用既有 yield 机制（ADR-10）；callerIdentity 经 tool_util env 注入（ADR-9 恢复采用），yield 部分零改动 | AC-12 |
| R-18 | 行为 | 建立方经 stdin 下发 `{"op":"authorize",...}` | authorize 的 data 语义由 Agent 侧定义，CLI 仅透传 opaque data（调 `IAgentReceiver::Authorize`），不阻塞等 ack | O-5/O-6 已解决 | AC-3 |
| R-19 | 边界 | 目标 agentId 未在 `AgentCardMgr` 预注册为 `AgentCardType::APP` | 目标 agentId 须在 `AgentCardMgr` 预注册为 `AgentCardType::APP` | `ResolveConnectAgentTarget` 要求（O-12/D-9） | AC-1 |
| R-20 | 行为 | 失败检测 | **失败检测契约**：失败经 **exit 事件(exitCode≠0)**（订阅在场）或 **querySession/SubscribeSession 返回 NOT_FOUND**（任意时点）判定；`connect-done(status=fail)` 仅信息性事件，不作为失败判据（订阅竞态可能丢失）；`SubscribeSession` 需 `session.status==running`，过早订阅得 `ERR_CLI_SESSION_NOT_FOUND` | P3-5 订阅前不感知模型 | AC-1,AC-5,AC-15 |
| R-21 | 行为 | connect 子命令 `requirePermissions` 声明 `ohos.permission.CONNECT_AGENT` | connect 子命令 `requirePermissions` 声明 `ohos.permission.CONNECT_AGENT`——作为纵深防御；ForCli 流程对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台校验（`SetCallingIdentity` 后，不跳过、不放行）；invoker 侧 `requirePermissions` 声明经 cliMgr `BatchQueryPermissionBySubCommand` 暴露供调用方/宿主校验 | ForCli 对 appA token 强制校验，requirePermissions 为纵深防御 | AC-6,AC-8 |
| R-22 | 异常 | connect 握手失败（B 不存在/权限/超时），`OnAbilityConnectDone` 非 0 | connect 失败 | 10s（AMS connect 超时 `timeoutUnitTime_`1000ms×`CONNECT_TIMEOUT_MULTIPLE`10；cliMgr ExecTool 1800s 为 session 级兜底）；**失败信号=exit 事件(exitCode≠0) 或 querySession/SubscribeSession 返回 NOT_FOUND**；connect-done(status=fail) 仅信息性（订阅竞态可能丢失，不作失败判据，见 R-20） | AC-1 |
| R-23 | 异常 | timeout 到达，bg session 达 timeout | session 超时 | ≤1800s；cliMgr Killpg+error("session timed out") | AC-9 |
| R-24 | 异常 | inputText 非合法 JSON | stdin 非法 JSON | N/A；stdout error(fatal=false)，忽略该条继续 | AC-2 |
| R-25 | 异常 | `OnAbilityDisconnectDone`（对端断连） | B 主动断连 | N/A；disconnect-done(reason=peer)+子进程退出 | AC-5,AC-15 |
| R-26 | 异常 | 进程异常退出（崩溃） | CLI 子进程崩溃 | N/A；cliMgr 回收+exit 事件+agent caller-death 清理 | AC-5 |
| R-27 | 异常 | `background=false`+yieldMs 到点 | yield 自动转后台 | ≤yieldMs；SetBackground(true)+reply，不 kill，继续 | AC-12 |
| R-28 | 异常 | `SendMessage` 30s 超时(`INPUT_WRITE_TIMEOUT_MS`)或 4MB/4096 队列满 | stdin 写超时/队列满 | 30s；NotifyInputReply(false)，SendMessage 回调失败 | AC-2,AC-3 |
| R-29 | 异常 | `DisconnectAgentExtensionAbilityForCli` ret≠0（鉴权失败/入参问题） | disconnect 被拒 | N/A；stdout error(fatal=false) "disconnect-failed"；会话不结束、循环继续，可重发 | AC-5 |
| R-30 | 异常 | poll(stdin+wakeup) 返回 -1（非 EINTR） | poll 失败 | N/A；stdout error(fatal=true) "poll-error"；exitCode=1 退出（R-20） | AC-5 |
| R-31 | 异常 | read(stdin) 返回 -1（非 EINTR） | stdin 读失败 | N/A；stdout error(fatal=true) "stdin-read-error"；exitCode=1 退出（R-20） | AC-2 |
| R-32 | 异常 | 重组后单行 > `MAX_LINE_BYTES`(200*1024) | 单行超 200KB | N/A；stdout error(fatal=true) "bad-json"（line exceeds 200KB）；exitCode=1 退出，不恢复 | AC-2 |
| R-33 | 恢复 | CLI 子进程死亡 | cliMgr ReaperLoop 回收 session；agent 侧 caller-death 清理连接 | 资源释放；session 结束，建立方收 exit 事件 | AC-5,AC-13,AC-15 |
| R-34 | 恢复 | cliMgr SA 重启 | 子进程为独立进程，不受 SA 重启影响；SA 重启清空 sessionRecords→querySession/SubscribeSession 返回 NOT_FOUND；stdin EOF 子进程自然退出；登记 IH-3：cliMgr SA 死亡期间建立方 pending Promise 永挂，需建立方客户端侧设超时 | 子进程继续运行或随 stdin 断开自然结束；建立方需客户端超时兜底；IH-3 建立方 Promise 永挂风险 | — |

### 配额与池上限规格（不变式）

本规格为 Agent 连接配额的硬性不变式（invariant），跨 ForCli（CLI）新增功能必须保持不变。任何新增连接入口必须复用既有 `ConnectStandardAgentExtensionAbility → RegisterStandardAgentConnection → AdmitStandardAgentConnectionLocked` 链路，不得引入独立配额账本。

1. **每调用方 5 槽上限**：同一调用方（按 `callerUid` 计）最多同时连接 5 个不同 AgentExtensionAbility。必须在 `agent_connect_manager.cpp:33` 常量 `MAX_AGENT_CONNECTIONS_PER_CALLER = 5` 处集中定义，并在 `AdmitStandardAgentConnectionLocked`（`agent_connect_manager.cpp:243`）以 `callerQuotas_[callerUid]` 单一账本强制：当存在新的 `quotaKey` 且 `callerQuota.size() >= 5` 时返回 `ERR_MAX_AGENT_CONNECTIONS_REACHED`，否则 `callerQuota[quotaKey]++`。该账本必须为进程级唯一，所有连接入口共享。

2. **APP/ATOMIC_SERVICE 按 agentId 计配额槽**：对 APP 与 ATOMIC_SERVICE 类型，配额键必须由 `BuildStandardQuotaKey`（`agent_manager_service.cpp:831`）构造，其中 `key.hostKey = BuildAgentHostKey(want, callingUid)`、`key.agentId = agentId`、`key.isLowCode = false`。不同 `agentId` 必须计为不同配额槽，同一 `agentId` 重复连接不得新增槽位。

3. **LOW_CODE 100 实例/宿主 + 占调用方 1 槽**：LOW_CODE 类型同一 AgentExtensionAbility 最多运行 100 个实例（不分调用方）。必须在 `MAX_LOW_CODE_AGENTS_PER_HOST = 100`（`agent_connect_manager.cpp:34`）与 `AdmitLowCodeAgentLocked`（`:255`）处强制：`session.agents.size() >= 100` 时拒绝；LOW_CODE ability 同时必须占用其调用方 1 个配额槽（即先过调用方 5 配额，再计宿主 100 上限）。LOW_CODE 路径不得与 APP 标准路径共享宿主计数。

4. **CLI(ForCli) 与 JS/ETS 必须共享调用方配额，不可绕过**：
   - JS/ETS 路径 `ConnectAgentExtensionAbility` → `ValidateConnectAgentPermission` 设 `callerUid = IPCSkeleton::GetCallingUid()`（`:709`）→ `ConnectStandardAgentExtensionAbility`（`:395`）→ `BuildStandardQuotaKey`（`:536/831`）→ `RegisterStandardAgentConnection`（`:563`）→ `AdmitStandardAgentConnectionLocked`（`agent_connect_manager.cpp:368/243`）。
   - ForCli 路径 `ConnectAgentExtensionAbilityForCli`（`agent_manager_service.cpp:413`）必须经 `SetCallingIdentity`（`:418-420`）还原 app A 真实 HAP 调用身份，经 anti-spoof 校验（`:421-429`，token uid == `cliToolUid`）后，于 `:440` 取 `callerUid`，于 `:468` 委托既有 `ConnectStandardAgentExtensionAbility`，复用同一 `BuildStandardQuotaKey`/`RegisterStandardAgentConnection`/`AdmitStandardAgentConnectionLocked`，不得另设 ForCli 专用账本或独立上限。
   - 配额必须按 `callerUid` 计；经 `SetCallingIdentity` 还原后的 `callerUid` 必须等于 app A 的 uid，使 ForCli 连接计入 app A 在 `callerQuotas_[appA_uid]` 的 5 槽之内，与 JS/ETS 共享。
   - 本条为 ForCli 新增功能引入后的不变式：`agent_connect_manager.{cpp,h}` 与 `agent_connect_manager_types.{cpp,h}` 的配额常量、`Admit*`、`Release*`、`callerQuotas_` 语义不得为 ForCli 改动；ForCli 不得直接引用 `callerQuotas_`/`Admit*`/`Release*`/`MAX_AGENT*`。

## 验证映射

> 每个 AC 至少关联一个可执行测试入口。`Red 条件` 描述实现前测试为什么必然失败以及失败时可观察到什么。实现后测试从 Red 转 Green 的实际结果和证据由 execution-plan / Verification Evidence 回填。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1/R-7/R-8/R-19/R-20/AC-1 | `agent_manager_service_test::ConnectForCli_AllOk`；`ohos_agent_test::OnAbilityConnectDone_Success`、`BuildEvents`、`RunCommandLoop_ReassemblesAcrossBlocks` | 集成 | ForCli 接口与单阶段 connect 未实现→`ConnectAgentExtensionAbilityForCli` 符号缺失/编译链接失败，或 connect-done JSON-line 经 `SubscribeSession` 不可得（无 connect-done 事件、子进程不长驻） | 单阶段 ForCli connect 成功 + `connect-done(status=ok)` JSON-line（跨块重组解析）+ 子进程长驻不退出 |
| VM-2 | R-2/R-7/R-10/R-28/AC-2,AC-3 | `ohos_agent_test::RunCommandLoop_SendMessage_Success`、`RunCommandLoop_SendMessage_Fail`、`RunCommandLoop_Authorize`、`ConnectorStub_SendData`、`ConnectorStub_Authorize` | 集成 | CommandLoop 分派与 `IAgentReceiver::SendData/Authorize` 接线未实现→stdin 指令不被解析、对端收不到 data（SendMessage 回调失败或对端无到达） | sendmessage/authorize 到达对端 AgentExtensionAbility |
| VM-3 | R-3/AC-4 | `ohos_agent_test::ConnectorStub_SendData`、`RunCommandLoop_ReassemblesAcrossBlocks`、`BuildEvents_OnData`/`BuildEvents_OnAuthorize` | 集成 | 对端→caller 回流通道未接线→对端回调经 stdout JSON-line 跨块重组不可达建立方（无 OnData/OnAuthorize 事件） | 对端回调经 stdout JSON-line 跨块重组解析送达建立方 |
| VM-4 | R-4/R-7/R-10/R-20/R-25/R-26/R-29/R-30/R-33/AC-5 | `agent_manager_service_test::DisconnectForCli_AllOk`；`ohos_agent_test::OnAbilityDisconnectDone_PeerReasonAndWakeup`、`OnAbilityDisconnectDone_UserReason`、`OnAbilityDisconnectDone_ErrorReason`、`TryInitiateDisconnect_CAS`、`RunCommandLoop_AfterDisconnectInitiated_SkipsDispatch` | 集成 | disconnect 流程未实现→`DisconnectAgentExtensionAbilityForCli` 缺失，stdin disconnect 不产生 `disconnect-done(reason=user)` JSON-line | disconnect 走正常流程 + `disconnect-done` JSON-line（reason=user） |
| VM-4b | R-6/R-33/AC-13 | `ohos_agent_test::RunCommandLoop_EOF_Exit`（ClearSession 兜底路径）；集成 `cliMgr ClearSession` | 集成 | ClearSession 强制终止与 caller-death 清理未接线→SIGKILL 后无 exit 事件、agent 侧连接泄漏 | ClearSession→SIGKILL→exit 事件 + caller-death 清理 |
| VM-5 | R-13/R-14/R-15/R-21/AC-6~8 | `agent_manager_service_test::ConnectForCli_CcmOff`、`ConnectForCli_NotCliTool`、`ConnectForCli_NotSystemApp`、`ConnectForCli_NoConnectAgentPerm`、`ConnectForCli_NotForeground`、`ConnectForCli_UidSpoof`、`ConnectForCli_CallerIdentityNotHap`、`ConnectForCli_NonAppType`、`DisconnectForCli_CcmOff`、`DisconnectForCli_Spoof` | 单测 | ForCli 门禁/防伪/强制未实现→非系统应用/无权限/伪造 uid/非 CLI 均未被拒（错误码不匹配 `ERR_NOT_SYSTEM_APP`/`ERR_PERMISSION_DENIED`/`ERR_WRONG_INTERFACE_CALL`），非 CLI 不走原校验 | 入口门禁/owner/IsCliToolToken+CCM 特性门禁（ForCli 不跳过校验，全分支确切错误码） |
| VM-6 | R-9/R-12/R-23/AC-9 | `agent_manager_service_test` timeout 单测分支 | 单测 | timeout 上限拒绝未实现→timeout>1800s 不被拒 | timeout 上限拒绝（>1800s 被拒） |
| VM-6b | R-12/R-17/R-27/AC-12 | 集成 `cliMgr yield-time`；`ohos_agent_test::RunCommandLoop` 退出路径 | 集成 | yield 复用未接线→background=false+yieldMs 到点不自动转后台、不 reply 回流、session 被 kill | background=false+yieldMs 到点自动转后台+reply 回流 session info+session 继续 |
| VM-7 | R-16/AC-10 | `agent_manager_service_test::ConnectForCli_CcmOff`、`DisconnectForCli_CcmOff` | 单测 | CCM 开关未 gate→CCM off 时工具仍可用/ForCli 仍进入 | CCM 开关隔离（off 时工具不可用、ForCli 拒绝） |
| VM-8 | R-11/AC-14 | `agent_manager_service_test` 配额单测（既有 `AdmitStandardAgentConnectionLocked` 路径） | 单测 | ForCli 未复用既有配额链→第 6 个不同 Agent connect 不被拒，或 CLI 与 JS 不共享 callerUid 桶（绕过） | 第 6 个不同 Agent connect 被拒（`ERR_MAX_AGENT_CONNECTIONS_REACHED`）；CLI 与 JS 共享配额桶 |

> 测试入口汇总：`agent_manager_service_test`（service 端 ForCli 14 例：`ConnectForCli_{CcmOff,NotCliTool,NullConnection,CallerIdentityNotHap,UidSpoof,NonAppType,NoConnectAgentPerm,NotSystemApp,NotForeground,AllOk}`×10 + `DisconnectForCli_{CcmOff,NullConnection,Spoof,AllOk}`×4 = 14；client 端 6 例：`ConnectAgentExtensionAbilityForCli_00{1,2,3}` + `DisconnectAgentExtensionAbilityForCli_00{1,2,3}`，合计 20）；`ohos_agent_test`（23 例：`EscapeJsonString`/`BuildEvents`/`ConnectorStub SendData+Authorize`/`OnAbilityConnectDone{Success,ProxyNull,NonZeroResult}`/`OnAbilityDisconnectDone{PeerReason+Wakeup,UserReason,ErrorReason}`/`TryInitiateDisconnect CAS`/`RunCommandLoop{SendMessage Success+Fail,Authorize,BadJson,BadData,NoReceiver,UnknownOp,ReassemblesAcrossBlocks,LineExceeds200KB-Fatal,EOF-Exit,AfterDisconnectInitiated-SkipsDispatch}`/`WaitForConnect_Timeout_ReturnsNeg1`）。nm 确认全 TestBody 符号链入；构建 PASS，运行验证待真机。

## API 变更分析

> 新增 2 个 additive ForCli InnerAPI：`ConnectAgentExtensionAbilityForCli(want, connection, String callerIdentity)` + `DisconnectAgentExtensionAbilityForCli(connection, String callerIdentity)`（向后兼容，无序列化格式破坏）。

### 新增 API

| API 名称 | 开放级别 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `ConnectAgentExtensionAbilityForCli` | InnerAPI | (Want want, IAbilityConnection connection, String callerIdentity) | int resultCode | 通用 connect 错误码 | CLI 直连 APP Agent（ForCli：防伪+`SetCallingIdentity` 后对 appA token 强制校验，不跳过） | AC-1 |
| `DisconnectAgentExtensionAbilityForCli` | InnerAPI | (IAbilityConnection connection, String callerIdentity) | int resultCode | 通用 disconnect 错误码 | CLI 直连断开（ForCli） | AC-5 |

### 变更/废弃 API

| API 名称 | 当前开放级别 | 目标开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|--------------|--------------|----------|----------|----------|---------|
| N/A | — | — | — | — | — | — |

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源 | 关联 AC |
|--------|----------|----------|---------------|----------|----------|----------|---------|
| `ConnectAgentExtensionAbilityForCli` | New API | InnerAPI | `int AgentManagerClient::ConnectAgentExtensionAbilityForCli(const Want &want, const sptr<IAbilityConnection> &connection, const std::string &callerIdentity)` | CLI 子进程经 getenv 取 `ohos_cli_callerIdentity` 后单阶段直连 APP Agent | int resultCode（0 成功 / 非 0 失败）；经 IsCliToolToken+CCM 门禁→防伪→`SetCallingIdentity(callerIdentity)`→对 appA token 强制 CONNECT_AGENT/system-app/前台，no bypass | New: proposal.md/design.md/TASK-2/TASK-4；落点 `agent_runtime_framework/interfaces/inner_api` | AC-1 |
| `DisconnectAgentExtensionAbilityForCli` | New API | InnerAPI | `int AgentManagerClient::DisconnectAgentExtensionAbilityForCli(const sptr<IAbilityConnection> &connection, const std::string &callerIdentity)` | CLI 子进程 stdin disconnect 或对端断连后经 `DisconnectAgentExtensionAbilityForCli` 走正常断连 | int resultCode（0 成功 / 非 0 失败，ret≠0 时 stdout error(fatal=false) "disconnect-failed"，会话不结束）；经 `SetCallingIdentity` 后走 `DisconnectAbility`→`OnAbilityDisconnectDone`，不跳过校验 | New: proposal.md/design.md/TASK-4/TASK-5；落点 `agent_runtime_framework/interfaces/inner_api` | AC-5 |
| `ERR_MAX_AGENT_CONNECTIONS_REACHED` | Existing Error Code | InnerAPI/System | agent_connect_manager 错误码枚举 `ERR_MAX_AGENT_CONNECTIONS_REACHED` | 同一 callerUid 已连 5 个不同 Agent 再发起第 6 个 connect，`AdmitStandardAgentConnectionLocked` 判定 `callerQuota.size() >= 5` | connect 被拒，返回该错误码；CLI 与 JS 共享 callerUid 桶 | `agent_connect_manager.cpp:243` / `AdmitStandardAgentConnectionLocked`；常量 `MAX_AGENT_CONNECTIONS_PER_CALLER=5`（`:33`） | AC-14 |
| `ERR_NOT_SYSTEM_APP` / `ERR_PERMISSION_DENIED` | Existing Error Code | System | cliMgr ExecTool 入口错误码 | 非系统应用或无 `ohos.permission.EXEC_CLI_TOOL` 调 `ExecTool` | `ExecTool` 被拒 | cliMgr `ValidateExecToolPermissions` 既有 | AC-6 |

## 接口规格

> 与"API 变更分析"（声明层）互补，本节定义 ForCli InnerAPI 的调用语义、参数约束与行为场景，以及 stdin/stdout JSON-line 协议。d.ts 落点、内部调用链与实现结构见 design.md。

### 接口定义

**ConnectAgentExtensionAbilityForCli**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int AgentManagerClient::ConnectAgentExtensionAbilityForCli(const Want &want, const sptr<IAbilityConnection> &connection, const std::string &callerIdentity)` |
| 返回值 | `int` — resultCode（0=成功；非 0=失败，见错误码范围） |
| 开放级别 | InnerAPI |
| 错误码 | 通用 connect 错误码（含 `ERR_PERMISSION_DENIED`/`ERR_NOT_SYSTEM_APP`/`ERR_WRONG_INTERFACE_CALL`/`NOT_TOP_ABILITY`/`ERR_MAX_AGENT_CONNECTIONS_REACHED`/`ERR_INVALID_VALUE`） |
| 关联 AC | AC-1 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| want | `Want` | 是 | — | 必含 `AGENTID_KEY` 非空；agentId 须预注册为 `AgentCardType::APP`（R-19）；ForCli 内联 `ResolveConnectAgentTarget`+enforce APP，无 preflight |
| connection | `sptr<IAbilityConnection>` | 是 | — | 非空；null 则返回 `ERR_INVALID_VALUE` |
| callerIdentity | `std::string` | 是 | — | 经 cliMgr `ResetCallingIdentity` 捕获 appA 身份串→env `ohos_cli_callerIdentity`→ohos-agent getenv 透传；非入 Want；防伪 `GetHapTokenInfo(callerIdentity.token).uid`==CLI 子进程 uid |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | IsCliToolToken=true + CCM enabled + 防伪通过 + appA token 持 CONNECT_AGENT+system-app+前台 + 类型=APP | InnerAPI/终端用户 | `SetCallingIdentity(callerIdentity=appA)`→复用 `ConnectStandardAgentExtensionAbility`→`OnAbilityConnectDone(0)`→握手成功；CLI 写 `connect-done(status=ok)` JSON-line；子进程长驻 | AC-1 |
| 2 | CCM off 或 非 CLI 或 防伪失败 或 appA 无权限/非前台 或 非 APP 类型 | InnerAPI | 返回确切错误码（`ERR_PERMISSION_DENIED`/`ERR_WRONG_INTERFACE_CALL`/`ERR_NOT_SYSTEM_APP`/`NOT_TOP_ABILITY`/`ERR_INVALID_VALUE`），不进入 ForCli；非 CLI 走原 `ConnectAgentExtensionAbility` 校验 | AC-6,AC-8 |

**DisconnectAgentExtensionAbilityForCli**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int AgentManagerClient::DisconnectAgentExtensionAbilityForCli(const sptr<IAbilityConnection> &connection, const std::string &callerIdentity)` |
| 返回值 | `int` — resultCode（0=成功；非 0=失败，ret≠0 时 stdout error(fatal=false) "disconnect-failed"，会话不结束、循环继续，可重发） |
| 开放级别 | InnerAPI |
| 错误码 | 通用 disconnect 错误码（含 `ERR_INVALID_VALUE`/`ERR_PERMISSION_DENIED`） |
| 关联 AC | AC-5 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| connection | `sptr<IAbilityConnection>` | 是 | — | 非空；null 则返回 `ERR_INVALID_VALUE`；须为已建连的同一 `IAbilityConnection`（R-5） |
| callerIdentity | `std::string` | 是 | — | 同 connect；经 `SetCallingIdentity` 还原 appA 后走 `DisconnectAbility`→`OnAbilityDisconnectDone`，不跳过校验 |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | stdin disconnect 或对端断连后 CLI 调 `DisconnectAgentExtensionAbilityForCli`，ret=0 | InnerAPI/终端用户 | `SetCallingIdentity(appA)`→`DisconnectAbility`→`OnAbilityDisconnectDone`；CLI 写 `disconnect-done(reason=user/peer)` JSON-line，干净退出；配额由 `ReleaseCallerQuotaLocked` 释放 | AC-5,AC-15 |
| 2 | ret≠0（鉴权失败/入参问题） | InnerAPI/终端用户 | stdout error(fatal=false) "disconnect-failed"；会话不结束、循环继续，可重发 | AC-5 |

### stdin/stdout JSON-line 协议

| 属性 | 值 |
|------|-----|
| 协议名 | ohos-agent stdin/stdout JSON-line |
| 开放级别 | 终端用户（经 cliMgr `SendMessage`/`SubscribeSession` 暴露给建立方） |
| 关联 AC | AC-1,AC-2,AC-3,AC-4,AC-5,AC-15 |

**stdin 协议（建立方→CLI 子进程）**

| 指令 | 帧格式 | 约束 |
|------|--------|------|
| sendmessage | `{"op":"sendmessage","data":...}\n` | 调用方在 inputText 末尾追加单个 `\n`；data 内换行必须 JSON 转义；cliMgr `SendMessage` 原样写 inputText 不加 `\n`；pending 队列总量 4MB/4096（R-28） |
| authorize | `{"op":"authorize","data":...}\n` | data 语义由 Agent 侧定义，CLI 仅透传 opaque data（R-18），不阻塞等 ack |
| disconnect | `{"op":"disconnect"}\n` | 走正常断连流程（R-4） |

**stdout 协议（CLI 子进程→建立方）**

| 事件 | 帧格式 | 约束 |
|------|--------|------|
| connect-done | `{"type":"connect-done","status":"ok"\|"fail"}\n` | status=fail 仅信息性，不作失败判据（R-20） |
| OnData | `{"type":"OnData",...}\n` | 对端回调经 `AgentConnectorStub` 接收→写 stdout |
| OnAuthorize | `{"type":"OnAuthorize",...}\n` | 同上 |
| disconnect-done | `{"type":"disconnect-done","reason":"user"\|"peer"\|"error"}\n` | reason=user/peer/error（R-25） |
| error | `{"type":"error","fatal":bool,"msg":...}\n` | fatal=true→exitCode=1 退出（R-30/R-31/R-32）；fatal=false→忽略继续（R-24/R-29） |

**分帧与重组约束**

- cliMgr IOMonitor 两端不分帧：stdin 原样写、stdout 按 4096 块回调（`eventData` 为裸字节块 ≤4096，不分帧）。
- 分帧责任落协议层：消费方收 type=stdout 的 CliToolEvent，**自行跨块按 `\n` 重组行→解析每行 JSON→按 `type` 字段分派**（自定界 JSON-line）。
- 子进程单行 ≤200KB（`MAX_LINE_BYTES=200*1024`，对齐 IPC 单条约定 `dataobs MAX_DATA_SIZE`，超限 error(fatal=true) 退出，R-32）。
- 主循环 poll(stdin+wakeup fd)：stdin 就绪判据兼顾 `POLLIN | POLLHUP`，部分内核对"写端已关闭、无待读数据"的管道 EOF 返回 `revents=POLLHUP` 不含 `POLLIN`；`read` 返 0→`n==0 return false` 正常退出（EOF 退出，R-7）。

## 兼容性声明

- **已有 API 行为变更:** 否（新增工具 + agentmgr 新增 ForCli 分支：IsCliToolToken+CCM 门禁，对 appA token 强制校验不跳过；非 CLI 进程行为不变；新增 2 个 additive InnerAPI 向后兼容）
- **配置文件格式变更:** 是（新增 `ohos-agent.json` 工具注册配置 + CCM 开关 `const.product.ohos_agent_cli.enabled`，均为新增，不破坏既有）
- **数据存储格式变更:** 否
- **最低支持版本:** OpenHarmony-7.0-Release
- **API 版本号策略:** N/A（additive ForCli InnerAPI，向后兼容）

## 架构约束

> 本节列出本特性 AC 验证必须满足的约束。架构规则适用性及设计方案见 design.md。

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 不改编排核心 | 不改 cli_tool_framework session 编排，仅数据驱动消费 | AC-1~5 |
| 只读消费接口 | 不改 kits/native 连接接口；新增 additive ForCli InnerAPI（不破坏序列化格式） | AC-1~5 |
| 非 CLI 行为不变 | ForCli 分支仅对 IsCliToolToken+CCM 启用生效，且对 appA token 强制校验不跳过 | AC-8 |
| 特性隔离 | CCM 开关同时 gate 工具与 ForCli 入口 | AC-10 |
| 配额不变式 | ForCli 必须复用既有 `ConnectStandardAgentExtensionAbility → RegisterStandardAgentConnection → AdmitStandardAgentConnectionLocked` 链路，不得引入独立配额账本（详见 `### 配额与池上限规格（不变式）`） | AC-14 |
| 跨块重组责任在协议层 | cliMgr IOMonitor 不分帧（stdin 原样写、stdout 按 4096 块回调），消费方须跨块按 `\n` 重组+JSON 解析 | AC-1,AC-4 |

## 非功能性需求

> N/A 判定见 proposal.md 不涉及项确认。本节仅为适用项填写具体指标。

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 长驻 session 单连接，受 `MAX_TIMEOUT=1800s` 与 cliMgr 并发上限约束 | 集成 | 待补 |
| 安全 | cliMgr 入口门禁（系统应用+EXEC_CLI_TOOL）+ ForCli 防伪（`GetHapTokenInfo(callerIdentity.token).uid`==CLI 子进程 uid）+ `SetCallingIdentity(appA)` 后对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台（不跳过、不放行）+ `requirePermissions`=CONNECT_AGENT（invoker 侧，纵深防御）+ IsSessionOwner + CCM 隔离 | 扫描/单测 | 待补 |
| 可靠性 | 子进程死亡/对端断连均有清理与事件上报（exit 事件/disconnect-done/caller-death 清理） | 压力测试 | 待补 |
| 可测试性 | ForCli 门禁/防伪/强制/配额/CCM/协议均有单测入口（`agent_manager_service_test` ForCli 14 例 + `ohos_agent_test` 23 例）；失败检测契约可经 exit 事件/NOT_FOUND 观察 | 单测/集成 | 待补 |
| 定界定位 | connect/disconnect/error 事件经 stdout JSON-line 上报 + hilog；失败可经 exit 事件(exitCode≠0) 或 querySession/SubscribeSession 返回 NOT_FOUND 判定 | hilog+事件 | 待补 |

## 多设备适配声明

> 同一功能在不同设备类型上行为一致；特性本身受 CCM 配置项隔离（按产品启用），非设备维度差异。填"无差异（CCM 隔离）"。

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 全部 | 无差异（CCM 隔离） | `const.product.ohos_agent_cli.enabled` 控制启用 | 配置检查 | 待补 |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | CLI 工具无 UI | — |
| 大字体 | 否 | CLI 工具无 UI | — |
| 深色模式 | 否 | CLI 工具无 UI | — |
| 多窗口/分屏 | 否 | CLI 工具无 UI | — |
| 多用户 | 否 | connect userId 由 caller token 语义决定（O-4 待确认） | — |
| 版本升级 | 否 | 无持久化数据格式 | — |
| 生态兼容 | 否 | 仅系统侧调用方 | — |

## 行为场景（Gherkin）

```gherkin
Feature: ohos-agent 长连会话通道
  作为 框架/系统侧调用方
  我想要 经 cliMgr 拉起 ohos-agent 连接 APP 类型 Agent 并双向通信
  以便 建立可控长连会话

  Scenario: 正常 connect 与双向通信
    Given 系统应用且持 EXEC_CLI_TOOL 权限，且 CCM 开关已启用
    When 调用方 ExecTool(ohos-agent connect, APP 类型, bg=true, timeout=1800) 并 SubscribeSession
    Then 调用方经 SubscribeSession 收到 connect-done(status=ok)，子进程长驻
    When 调用方 SendMessage(sessionId, sendmessage 指令)
    Then 对端 AgentExtensionAbility 收到 data
    When 对端主动回调 sendData
    Then 调用方经 SubscribeSession 收到 OnData 事件
    When 调用方 SendMessage(sessionId, disconnect 指令)
    Then 调用方收到 disconnect-done，session 结束

  Scenario: ClearSession 强制终止
    Given 已建连的 connect session 且调用方已 SubscribeSession
    When 调用方 ClearSession(sessionId)
    Then 子进程被 SIGKILL，调用方收到 exit 事件，agent 侧 caller-death 清理

  Scenario: 非 CLI 进程走原校验
    Given 一个非 CLI 进程（IsCliToolToken=false）
    When 它发起 agent connect
    Then 走原有 connect 权限/前台校验（行为不变）

  Scenario: CCM 未启用
    Given const.product.ohos_agent_cli.enabled=false
    When 调用方 ExecTool(ohos-agent connect)
    Then 工具不可用，agentmgr 不接受 ForCli 连接
```

## context-references

```yaml
context-queries:
  - repo: "ability_runtime/cli_tool_framework"
    query: "cliMgr session 编排模型：ExecTool/SendMessage/SubscribeSession/ClearSession/IOMonitor 分帧行为与 MAX_TIMEOUT"
  - repo: "ability_runtime/agent_runtime_framework"
    query: "agent 连接握手：OnAbilityConnectDone 的 remoteObject 接口归属、AGENTEXTENSIONHOSTPROXY_KEY 交付、APP 标准连接路径、connect 校验链"
  - repo: "ability_runtime/tools/ohos-aa"
    query: "ohos_cli_executable + json 注册范式与子命令 schema 声明"
```

**关键文档：** proposal.md、design.md
