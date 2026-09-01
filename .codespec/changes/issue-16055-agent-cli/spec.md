# 特性规格

> 固化用户可见行为和验收标准。复杂度：标准。无 InnerKit 接口定义/内部实现流程——实现细节见 design.md。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | ohos-agent CLI 工具：连接 AgentExtensionAbility 的长连会话通道 |
| 特性编号 | FEAT-20260825-001 |
| 所属 Epic | 无 |
| 优先级 | P1 |
| 目标版本 | TBD（引用 manifest.target_release） |
| SIG 归属 | Ability SIG |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

> lineage: new（全新特性）。本节跳过。

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Draft |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md。design.md 与本文档并行产出。

## 用户故事

### US-1: 拉起并长连 APP 类型 Agent

**作为** 框架/系统侧调用方，
**我想要** 经 cliMgr 拉起长驻 `ohos-agent connect` 子进程连接 APP 类型 AgentExtensionAbility，
**以便** 建立可控长连会话。

**验收标准：**
- **AC-1:** WHEN 系统应用且持 `ohos.permission.EXEC_CLI_TOOL` 的调用方调 `CliToolMGRClient::ExecTool({toolName:"ohos-agent",subcommand:"connect",args:{bundleName,abilityName,moduleName,agentId},options:{background=true,timeout≤1800}})`（agentId 须已预注册为 `AgentCardType::APP`，见 BR-9）THEN cliMgr fork 子进程；子进程经 `tool_util` 注入 callerIdentity env（`ohos_cli_callerIdentity`=appA 身份串，见 BR-7）后调 `ConnectAgentExtensionAbilityForCli(want, connection, callerIdentity)` 直连，ForCli 内联 `ResolveConnectAgentTarget`+enforce APP（无 preflight），连接 `AgentCardType::APP` AgentExtensionAbility 成功；建立方经 `SubscribeSession` 收到 type=stdout 的 CliToolEvent，**跨块按 `\n` 重组 + JSON 解析得到 `{"type":"connect-done","status":"ok"}`**；子进程长驻不退出、不主动断连。
- **AC-9:** WHEN `ExecTool` 的 timeout>1800s THEN 被拒（超过系统最大长时上限）。
- **AC-12:** WHEN 调用方以 `background=false + yieldMs=Y` 启动 connect 且 session 运行超过 Y THEN cliMgr `HandleProcessYieldTimeout` 自动 `SetBackground(true)` 并经 `DispatchExecToolReplyEvent` 回流 session info，**不 kill**，session 以后台继续运行；后续 stdout 经 `SubscribeSession` 增量交付（ADR-10 yield 复用零改动；ADR-9 callerIdentity 经 tool_util env 注入）。

### US-2: 双向数据通信

**作为** 调用方，
**我想要** 经 session 向对端发 sendData/authorize，并接收对端主动推送，
**以便** 双向通信。

**验收标准：**
- **AC-2:** WHEN 建立方调 `CliToolMGRClient::SendMessage(sessionId, inputText='{"op":"sendmessage","data":...}\n')` THEN cliMgr 将该 JSON-line 写入 CLI 子进程 stdin（受 4MB/4096 队列上限 + 30s 写超时，超限则 SendMessage 回调失败，见 EX-7；**子进程单行 ≤200KB 见 BR-10**），子进程 CommandLoop 解析后经 `IAgentReceiver::SendData(connectorProxy,data)`（两参数）把 data 发送至对端 AgentExtensionAbility，对端收到。
- **AC-3:** WHEN 建立方调 `SendMessage(sessionId, inputText='{"op":"authorize","data":...}\n')` THEN 子进程 CommandLoop 解析后经 `IAgentReceiver::Authorize` 把授权 data 透传至对端（data 语义由 Agent 侧定义，CLI 仅透传 opaque data，见 BR-8），对端收到。
- **AC-4:** WHEN 对端 AgentExtensionAbility 经双向通道（`IAgentConnector::SendData(data)` 一参数）主动回调 sendData/authorize THEN CLI 子进程（`AgentConnectorStub`）接收并将数据作为 `{"type":"OnData"/"OnAuthorize",...}` JSON-line 写 stdout，经 cliMgr 以 type=stdout 块 → `SubscribeSession` 交付建立方；建立方跨块重组 + JSON 解析得到事件。

### US-3: 断连

**作为** 调用方，
**我想要** 经 stdin disconnect 走正常断连流程，或经 `ClearSession` 强制结束 session，
**以便** 干净或强制释放连接。

**验收标准：**
- **AC-5:** WHEN 建立方经 stdin 下发 `{"op":"disconnect"}` JSON-line THEN CLI 子进程走正常断连流程（子进程调 `DisconnectAgentExtensionAbilityForCli(connection, callerIdentity)`，agentmgr `SetCallingIdentity(appA)` 后走 `DisconnectAbility`→`OnAbilityDisconnectDone`，不跳过校验），将 `{"type":"disconnect-done","reason":"user"}` JSON-line 写 stdout 经 `SubscribeSession` 交付建立方，干净退出；配额由 agentmgr `ReleaseCallerQuotaLocked` 释放（AC-14）。
- **AC-13:** WHEN 建立方调 `CliToolMGRClient::ClearSession(sessionId)` THEN cliMgr 强制终止子进程（SIGKILL），session 结束；调用方经已订阅的 `SubscribeSession` 收到 exit 事件；agent 侧由 caller-death 清理连接（不走正常 disconnect 流程，无 `disconnect-done`）。建议优先用 stdin disconnect，`ClearSession` 为强制兜底。
- **AC-15:** WHEN 对端 AgentExtensionAbility 主动断连（`OnAbilityDisconnectDone` 在 CLI Binder 线程触发）THEN CLI 写 `{"type":"disconnect-done","reason":"peer"}` JSON-line，经 wakeup fd（自管道）唤醒主循环 poll，干净退出；配额由 agentmgr `ReleaseCallerQuotaLocked` 释放（AC-14）。

### US-4: 安全边界

**作为** 系统，
**我要** 确保仅受信 CLI 进程可连、仅 session 建立方可操作，
**以便** 守住安全边界。

**验收标准：**
- **AC-6:** WHEN 非系统应用或无 `ohos.permission.EXEC_CLI_TOOL` 的调用方调 `ExecTool` THEN 被拒（`ERR_NOT_SYSTEM_APP`/`ERR_PERMISSION_DENIED`）。
- **AC-7:** WHEN 非 connect 建立方对该 session 调 `SendMessage`/`ClearSession`/`SubscribeSession` THEN `IsSessionOwner` 拒绝。
- **AC-8:** WHEN 非 CLI 进程（`IsCliToolToken`=false）发起 agent connect THEN 走原有 connect 权限/前台校验，行为不变。
- **AC-14:** WHEN 调用方（经 CLI 或 JS）已并发连 5 个不同 Agent 再发起第 6 个 connect THEN agentmgr `AdmitStandardAgentConnectionLocked` 返回 `ERR_MAX_AGENT_CONNECTIONS_REACHED`，连接被拒；CLI 与 JS 共享同一调用方配额桶（callerUid=app A uid），不绕过。disconnect 释放后可重连。

### US-5: 特性隔离

**作为** 系统，
**我要** 用 CCM 配置项隔离本特性，
**以便** 按产品启用。

**验收标准：**
- **AC-10:** WHEN `const.product.ohos_agent_cli.enabled`=false THEN ohos-agent 工具不可用、agentmgr 不接受 ForCli 连接。
- **AC-11:** WHEN 同一 session 内多次 `sendmessage`/`authorize` 指令 THEN 复用同一 `IAbilityConnection` 连接对象与双向 proxy/stub，不重连、不重复建连。

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1 | BR-1,FR-1 | TASK-1~3 | 集成 | 待补 |
| AC-2 | FR-2 | TASK-3 | 集成 | 待补 |
| AC-3 | FR-2 | TASK-3 | 集成 | 待补 |
| AC-4 | FR-3 | TASK-3 | 集成 | 待补 |
| AC-5 | FR-4,RC-1 | TASK-5 | 集成 | 待补 |
| AC-6 | BR-2 | TASK-4 | 单测 | 待补 |
| AC-7 | BR-3 | TASK-4（依赖 cliMgr 既有） | 单测 | 待补 |
| AC-8 | BR-4 | TASK-4 | 单测 | 待补 |
| AC-9 | BR-5 | TASK-1（依赖 cliMgr 既有） | 单测 | 待补 |
| AC-10 | BR-6 | TASK-4 | 单测 | 待补 |
| AC-11 | FR-5 | TASK-2 | 单测 | 待补 |
| AC-12 | BR-7 | TASK-1（依赖 cliMgr 既有 yield） | 集成 | 待补 |
| AC-13 | FR-6,RC-1 | TASK-5（依赖 cliMgr 既有 ClearSession） | 集成 | 待补 |
| AC-14 | BR-11 | TASK-2/TASK-4（依赖 agentmgr 既有配额） | 单测 | 待补 |
| AC-15 | FR-4,RC-1 | TASK-5 | 集成 | 待补 |

## 业务规则

| 编号 | 规则描述 | 约束条件 | 关联 AC |
|------|----------|----------|---------|
| BR-1 | connect 仅对 `AgentCardType::APP` 类型 Agent 生效 | 类型由 AgentCard 定义；LOW_CODE/ATOMIC_SERVICE 不支持 | AC-1 |
| BR-2 | connect 入口须系统应用 + `ohos.permission.EXEC_CLI_TOOL` | cliMgr `ValidateExecToolPermissions` 既有 | AC-6 |
| BR-3 | session 操作须建立方（IsSessionOwner） | cliMgr 既有 | AC-7 |
| BR-4 | agentmgr 经 ForCli 路径对 `IsCliToolToken`+CCM 特性门禁后处理 connect：`SetCallingIdentity(callerIdentity=appA)` 后对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台校验（不跳过、不放行）；非 CLI 走原校验 | ForCli 防伪：`GetHapTokenInfo(callerIdentity.token).uid`==CLI 子进程 uid | AC-8 |
| BR-5 | 长时运行受系统最大长时上限（1800s）约束 | `MAX_TIMEOUT` 既有 | AC-9 |
| BR-6 | 特性受 CCM 配置项 `const.product.ohos_agent_cli.enabled` 隔离 | 同时 gate 工具注册消费与 ForCli 入口 | AC-10 |
| BR-7 | 长驻/background 由 cliMgr 既有 yield-time 机制承担（background=false+yieldMs→自动转后台，不 kill） | 复用既有 yield 机制（ADR-10）；callerIdentity 经 tool_util env 注入（ADR-9 恢复采用），yield 部分零改动 | AC-12 |
| BR-8 | authorize 的 data 语义由 Agent 侧定义，CLI 仅透传 opaque data（调 `IAgentReceiver::Authorize`），不阻塞等 ack | O-5/O-6 已解决 | AC-3 |
| BR-9 | 目标 agentId 须在 `AgentCardMgr` 预注册为 `AgentCardType::APP` | `ResolveConnectAgentTarget` 要求（O-12/D-9） | AC-1 |
| BR-10 | stdin 协议为 JSON-line：每条命令一行 `{"op":"sendmessage"|"authorize"|"disconnect",...}` + `\n` 分帧；cliMgr `SendMessage` 原样写 inputText 不加 `\n`，调用方须自带；子进程按 `\n` 切行 + 跨 4096 块重组；子进程单行 ≤200KB（`MAX_LINE_BYTES`，超限 error(fatal=true) 退出） | 已定 JSON-line（非纯文本） | AC-2,AC-3,AC-5 |
| BR-11 | 调用方最多并发连 5 个不同 Agent——agentmgr 既有 `AdmitStandardAgentConnectionLocked`+`MAX_AGENT_CONNECTIONS_PER_CALLER=5`（callerUid+agentId 键、disconnect 释放）；CLI 路径自动适用、不跳过校验、与 JS 共享配额桶 | 服务侧能力已具备，无需新增逻辑 | AC-14 |
| BR-12 | **失败检测契约**：失败经 **exit 事件(exitCode≠0)**（订阅在场）或 **querySession/SubscribeSession 返回 NOT_FOUND**（任意时点）判定；`connect-done(status=fail)` 仅信息性事件，不作为失败判据（订阅竞态可能丢失）；`SubscribeSession` 需 `session.status==running`，过早订阅得 `ERR_CLI_SESSION_NOT_FOUND` | P3-5 订阅前不感知模型 | AC-1,AC-5,AC-15 |
| BR-13 | connect 支持路径=background=true；误用 background=false 须带 yieldMs>0 触发 yield 安全网（AC-12）；background=false+yieldMs=0 为未支持误用→跑至 MAX_TIMEOUT 被 kill（EX-2）。ohos-agent.json connect 声明推荐 yieldMs（如 5s）供调用方参考 | ADR-10 yield 误用兜底 | AC-1,AC-12,AC-9 |
| BR-14 | connect 子命令 `requirePermissions` 声明 `ohos.permission.CONNECT_AGENT`——作为纵深防御；ForCli 流程对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台校验（`SetCallingIdentity` 后，不跳过、不放行）；invoker 侧 `requirePermissions` 声明经 cliMgr `BatchQueryPermissionBySubCommand` 暴露供调用方/宿主校验 | ForCli 对 appA token 强制校验，requirePermissions 为纵深防御 | AC-6,AC-8 |

## 功能规则

| 编号 | 规则描述 | 触发条件 | 作用对象 | 关联 AC |
|------|----------|----------|----------|---------|
| FR-1 | connect 单阶段：CLI 调 `ConnectAgentExtensionAbilityForCli(want, connection, callerIdentity)` 直连，ForCli 内联 `ResolveConnectAgentTarget`+enforce APP（无 preflight），建立长驻 session，握手成功后写 connect-done JSON-line | ExecTool(connect,background=true) | CLI 子进程 | AC-1 |
| FR-2 | 建立→对端数据发送：sendmessage/authorize 经 stdin 下发，子进程转双向通道 | SendMessage(sessionId,指令) | CLI 子进程→对端 | AC-2,AC-3 |
| FR-3 | 对端→建立方数据回流：对端回调经 stdout JSON-line → SubscribeSession(type=stdout 块)，建立方跨块重组+JSON 解析 | 对端主动回调 | CLI 子进程→建立方 | AC-4 |
| FR-4 | disconnect 走正常断连流程并上报 disconnect-done | stdin disconnect 或 ClearSession | CLI 子进程 | AC-5 |
| FR-5 | 同 session 复用同一连接对象 | 已建连 | CLI 子进程 | AC-11 |
| FR-6 | ClearSession 强制终止 session | 建立方调 ClearSession | cliMgr→子进程 | AC-13 |
| FR-7 | connect 成功后进入阻塞式 CommandLoop REPL（poll stdin+self-pipe read fd），逐行解析 JSON-line 命令分派，至 disconnect/B 断连/EOF 退出 | OnAbilityConnectDone(0) | CLI 子进程 | AC-1,AC-2,AC-3,AC-5 |

## 异常/豁免规则

| 编号 | 异常码/枚举 | 规则描述 | 触发条件 | 超时阈值 | 处理结果 | 关联 AC |
|------|------------|----------|----------|----------|----------|---------|
| EX-1 | connect 失败 | connect 握手失败（B 不存在/权限/超时） | OnAbilityConnectDone 非 0 | 10s（AMS connect 超时 `timeoutUnitTime_`1000ms×`CONNECT_TIMEOUT_MULTIPLE`10；cliMgr ExecTool 1800s 为 session 级兜底） | **失败信号=exit 事件(exitCode≠0) 或 querySession/SubscribeSession 返回 NOT_FOUND**；connect-done(status=fail) 仅信息性（订阅竞态可能丢失，不作失败判据，见 BR-12） | AC-1 |
| EX-2 | session 超时 | timeout 到达 | bg session 达 timeout | ≤1800s | cliMgr Killpg+error("session timed out") | AC-9 |
| EX-3 | stdin 非法 JSON | 解析失败 | inputText 非合法 JSON | N/A | stdout error(fatal=false)，忽略该条继续 | AC-2 |
| EX-4 | B 主动断连 | OnAbilityDisconnectDone | 对端断连 | N/A | disconnect-done(reason=peer)+子进程退出 | AC-5,AC-15 |
| EX-5 | CLI 子进程崩溃 | 进程异常退出 | 崩溃 | N/A | cliMgr 回收+exit 事件+agent caller-death 清理 | AC-5 |
| EX-6 | yield 自动转后台 | background=false+yieldMs 到点 | ≤yieldMs | SetBackground(true)+reply，不 kill，继续 | AC-12 |
| EX-7 | stdin 写超时/队列满 | SendMessage 30s 超时(INPUT_WRITE_TIMEOUT_MS)或 4MB/4096 队列满 | 30s | NotifyInputReply(false)，SendMessage 回调失败 | AC-2,AC-3 |
| EX-8 | disconnect 被拒 | DisconnectAgentExtensionAbilityForCli ret≠0 | 鉴权失败/入参问题 | N/A | stdout error(fatal=false) "disconnect-failed"；会话不结束、循环继续，可重发 | AC-5 |
| EX-9 | poll 失败 | poll(stdin+wakeup) 返回 -1（非 EINTR） | 基础设施错误 | N/A | stdout error(fatal=true) "poll-error"；exitCode=1 退出（BR-12） | AC-5 |
| EX-10 | stdin 读失败 | read(stdin) 返回 -1（非 EINTR） | 基础设施错误 | N/A | stdout error(fatal=true) "stdin-read-error"；exitCode=1 退出（BR-12） | AC-2 |
| EX-11 | 单行超 200KB | 重组后单行 > `MAX_LINE_BYTES`(200*1024) | 敌意/破损流 | N/A | stdout error(fatal=true) "bad-json"（line exceeds 200KB）；exitCode=1 退出，不恢复 | AC-2 |

## 恢复契约

| 编号 | 触发条件 | 恢复策略 | 恢复结果 | 约束 |
|------|----------|----------|----------|------|
| RC-1 | CLI 子进程死亡 | cliMgr ReaperLoop 回收 session；agent 侧 caller-death 清理连接 | session 结束，建立方收 exit 事件 | 资源释放 |
| RC-2 | cliMgr SA 重启 | 子进程为独立进程，不受 SA 重启影响；SA 重启清空 sessionRecords→querySession/SubscribeSession 返回 NOT_FOUND；stdin EOF 子进程自然退出；登记 IH-3：cliMgr SA 死亡期间建立方 pending Promise 永挂，需建立方客户端侧设超时 | 子进程继续运行或随 stdin 断开自然结束；建立方需客户端超时兜底 | IH-3 建立方 Promise 永挂风险 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | FR-1/AC-1 | 集成 | 单阶段 ForCli connect 成功+connect-done JSON-line（跨块重组解析）+长驻不退出 |
| VM-2 | FR-2/AC-2,AC-3 | 集成 | sendmessage/authorize 到达对端 |
| VM-3 | FR-3/AC-4 | 集成 | 对端回调经 stdout JSON-line 跨块重组解析送达建立方 |
| VM-4 | FR-4/AC-5 | 集成 | disconnect 走正常流程+disconnect-done JSON-line |
| VM-4b | FR-6/AC-13 | 集成 | ClearSession→SIGKILL→exit 事件+caller-death 清理 |
| VM-5 | BR-2~4/AC-6~8 | 单测 | 入口门禁/owner/IsCliToolToken+CCM 特性门禁（ForCli 不跳过校验） |
| VM-6 | BR-5/AC-9 | 单测 | timeout 上限拒绝 |
| VM-6b | BR-7/AC-12 | 集成 | background=false+yieldMs 到点自动转后台+reply 回流 session info+session 继续 |
| VM-7 | BR-6/AC-10 | 单测 | CCM 开关隔离 |
| VM-8 | BR-11/AC-14 | 单测 | 第 6 个不同 Agent connect 被拒（ERR_MAX_AGENT_CONNECTIONS_REACHED）；CLI 与 JS 共享配额桶 |

## API 变更分析

> 新增 2 个 additive ForCli InnerAPI：`ConnectAgentExtensionAbilityForCli(want, connection, String callerIdentity)` + `DisconnectAgentExtensionAbilityForCli(connection, String callerIdentity)`（向后兼容，无序列化格式破坏）。

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| ConnectAgentExtensionAbilityForCli | InnerAPI(additive) | (Want want, IAbilityConnection connection, String callerIdentity) | int resultCode | 通用 connect 错误码 | CLI 直连 APP Agent（ForCli：防伪+SetCallingIdentity 后对 appA token 强制校验，不跳过） | AC-1 |
| DisconnectAgentExtensionAbilityForCli | InnerAPI(additive) | (IAbilityConnection connection, String callerIdentity) | int resultCode | 通用 disconnect 错误码 | CLI 直连断开（ForCli） | AC-5 |

### 变更/废弃 API

| API 名称 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|----------|----------|----------|---------|
| N/A | — | — | — | — |

## 兼容性声明

- **已有 API 行为变更:** 否（新增工具 + agentmgr 新增 ForCli 分支：IsCliToolToken+CCM 门禁，对 appA token 强制校验不跳过；非 CLI 进程行为不变；新增 2 个 additive InnerAPI 向后兼容）
- **配置文件格式变更:** 是（新增 `ohos-agent.json` 工具注册配置 + CCM 开关 `const.product.ohos_agent_cli.enabled`，均为新增，不破坏既有）
- **数据存储格式变更:** 否
- **最低支持版本:** TBD
- **API 版本号策略:** N/A（additive ForCli InnerAPI，向后兼容）

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 不改编排核心 | 不改 cli_tool_framework session 编排，仅数据驱动消费 | AC-1~5 |
| 只读消费接口 | 不改 kits/native 连接接口；新增 additive ForCli InnerAPI（不破坏序列化格式） | AC-1~5 |
| 非 CLI 行为不变 | ForCli 分支仅对 IsCliToolToken+CCM 启用生效，且对 appA token 强制校验不跳过 | AC-8 |
| 特性隔离 | CCM 开关同时 gate 工具与 ForCli 入口 | AC-10 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 长驻 session 单连接，受 MAX_TIMEOUT=1800s 与 cliMgr 并发上限约束 | 集成 | 待补 |
| 安全 | cliMgr 入口门禁（系统应用+EXEC_CLI_TOOL）+ ForCli 防伪（GetHapTokenInfo(callerIdentity.token).uid==CLI 子进程 uid）+ SetCallingIdentity(appA) 后对 appA 真实 token 强制 CONNECT_AGENT+system-app+前台（不跳过、不放行）+ `requirePermissions`=CONNECT_AGENT（invoker 侧，纵深防御）+ IsSessionOwner + CCM 隔离 | 扫描/单测 | 待补 |
| 可靠性 | 子进程死亡/对端断连均有清理与事件上报 | 压力测试 | 待补 |
| 问题定位 | connect/disconnect/error 事件经 stdout 上报 | hilog+事件 | 待补 |

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

## Spec 自审清单

- [x] 无"待定""TODO"占位（TBD 仅用于目标发行版本与 Owner，已标注）
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（做什么/不做什么清晰）
- [x] 无语义模糊表述
- [x] AC 与业务规则/异常规则/恢复契约交叉一致

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
