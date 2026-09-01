# 特性规格

> 固化**测试视角**的可观察行为和验收标准。本规格仅描述调用方/测试经 **cliManager 公共 SDK（execTool/subscribeSession/sendMessage/clearSession/querySession）+ ohos-agent 工具协议** 可验证的可观察行为；不关心实现细节。复杂度：标准。
>
> 本规格的 API 边界=**cliManager 公共 SDK + ohos-agent 工具协议**：调用方经 cliManager.execTool 消费 ohos-agent 工具、经 subscribeSession 收 CliToolEvent、经 sendMessage 下发指令、经 clearSession/querySession 强制结束与查询。所有内部实现（cliMgr 编排方法名、ForCli 内部签名、SetCallingIdentity、防伪、配额账本、状态机、file:行、内部类与常量符号、ADR 方案对比、内部缓冲上限）见 design.md；实现任务见 execution-plan.md。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | ohos-agent CLI 工具（经 cliManager 公共 SDK 消费）：连接 AgentExtensionAbility 的长连会话通道 |
| 特性编号 | 20260528392515 |
| 所属 Epic | Agent |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-7.0-Release |
| SIG 归属 | Ability SIG |
| 状态 | Approved |
| 复杂度 | 标准 |

> profile=none、manifest.id=issue-16055 见 `manifest.md`。

## 本次变更范围（Delta）

> lineage: new（全新特性）。本节跳过。

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md。design.md 与本文档并行产出，互不依赖。

## 用户故事

> 本节验收标准（AC）只描述调用方/测试**经 cliManager 公共 API 可观察**的行为：WHEN 调用方经 `cliManager.execTool("ohos-agent","connect",...)` 建连、经 `cliManager.sendMessage(sessionId, ...)` 下发指令、经 `cliManager.subscribeSession(sessionId, callback)` 收 CliToolEvent、经 `cliManager.clearSession(sessionId)` / `cliManager.querySession(sessionId)` 结束与查询 THEN 观察 CliToolEvent（STDOUT/EXIT）/CliSessionInfo（status:RUNNING|COMPLETED|FAILED）/execTool·sendMessage 抛码。实现细节（双向 IPC 通道、防伪、状态机、连接上限账本、ForCli 内部签名、cliMgr 编排方法名、内部缓冲上限）见 design.md；需求澄清与修订历史（ADR、option-F-vs-ForCli、Q1-Q5）见 proposal.md 附录与 design.md。
>
> **可观察表面列**区分测试形态：**终端用户**=经 cliManager 公共 API + subscribeSession callback 可直接观察、可集成测试（execTool 抛码、CliToolEvent、CliSessionInfo 均可观察）；**UT看护**=内部不变式、仅单测守护、cliManager 公共表面不可直接观察（如配额强制/防伪/非 CLI 路径/owner 校验）。

### US-1: 拉起并长连 APP 类型 Agent

**作为** 框架/系统侧调用方，
**我想要** 经 cliManager.execTool 拉起 ohos-agent 工具连接 APP 类型 AgentExtensionAbility 的长驻会话，
**以便** 建立可控长连会话。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-1 | WHEN 受信调用方经 `cliManager.execTool("ohos-agent","connect",{bundlename:B,abilityname:A,modulename:M,agentid:ID}, challenge, {background:true,timeout:≤1800})`（ID 须为已预注册的 APP 类型 AgentExtensionAbility）THEN execTool 返回 `Promise<CliSessionInfo{status:RUNNING,sessionId}>`；调用方经 `cliManager.subscribeSession(sessionId, callback)` 收到 `CliToolEvent{toolEventType:STDOUT, data='{"type":"connect-done","status":"ok"}'}`，随后会话长驻不退出、不主动断连，在超时上限到达前持续运行；STDOUT 事件的 data 跨块按 `\n` 重组 + JSON 解析可得该事件。 | 终端用户 | 正常 |
| AC-9 | WHEN 会话运行达 `execOptions.timeout` 上限（默认 1800s）THEN 经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"disconnect-done","reason":"timeout"}'}` 后收到 `CliToolEvent{EXIT}`，querySession 返回 status=FAILED/COMPLETED；WHEN 调用方以 `timeout>1800s` 调用 execTool connect THEN 会话未建立（经 subscribeSession 无 connect-done(ok)，收到 error 事件或 CliToolEvent{EXIT}）。 | 终端用户 | 边界 |
| AC-12 | WHEN 调用方以 `execOptions{background:false,yieldMs:Y}` 启动 connect 且 session 运行超过 Y THEN 经 subscribeSession 观察 session 自动转为后台运行并回流 `CliSessionInfo`（**不被 kill**），session 以后台继续；后续 STDOUT 增量事件持续交付。 | 终端用户 | 边界 |

### US-2: 双向数据通信

**作为** 调用方，
**我想要** 经 cliManager.sendMessage 向对端发 sendData/authorize，并经 subscribeSession 接收对端主动推送，
**以便** 双向通信。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-2 | WHEN 建立方经 `cliManager.sendMessage(sessionId, '{"op":"sendData","data":...}')` THEN（正常）data 经 session 投递至对端 AgentExtensionAbility，sendMessage resolve；（异常）sendMessage 的 message 长度 > 10240 THEN sendMessage reject 抛 35600033（写消息失败），对端无到达。 | 终端用户 | 正常 |
| AC-3 | WHEN 建立方经 `cliManager.sendMessage(sessionId, '{"op":"authorize","data":...}')` THEN authorize data 经 session 透传至对端 AgentExtensionAbility（data 语义由 Agent 侧定义，CLI 仅透传），sendMessage resolve。 | 终端用户 | 正常 |
| AC-4 | WHEN 对端 AgentExtensionAbility 主动回调 sendData/authorize THEN 建立方经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"OnData","data":...}'}` 或 `'{"type":"OnAuthorize","data":...}'}`（跨块按 `\n` 重组 + JSON 解析）。 | 终端用户 | 正常 |

### US-3: 断连

**作为** 调用方，
**我想要** 经 cliManager.sendMessage 下发 disconnect 走正常断连流程，
**以便** 干净释放连接。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-5 | WHEN 建立方经 `cliManager.sendMessage(sessionId, '{"op":"disconnect"}')` THEN 经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"disconnect-done","reason":"user"}'}`，session 干净退出（后续 `CliToolEvent{EXIT}`）；连接配额释放后可重连。 | 终端用户 | 正常 |

### US-4: 安全边界

**作为** 系统，
**我要** 确保仅受信调用方可经 cliManager 消费、仅 session 建立方可操作，
**以便** 守住安全边界。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-6 | WHEN 非系统应用或缺少 `ohos.permission.EXEC_CLI_TOOL` 的调用方经 `cliManager.execTool("ohos-agent","connect",...)` THEN execTool reject 抛 202（非系统应用）或 201（权限）（无 connect-done(ok)，会话未建立）。 | 终端用户 | 异常 |
| AC-7 | WHEN 非 connect 建立方对该 session 调用 sendMessage/clearSession THEN 经 owner 校验拒绝（操作不生效，session 不受影响）。 | UT看护 | 异常 |
| AC-8 | WHEN 非 CLI 进程发起 agent connect THEN 走原有 connect 权限/前台校验，行为不变（CLI 专用路径不启用）。 | UT看护 | 正常 |
| AC-14 | WHEN 同一调用方已并发连 5 个不同 Agent 再经 execTool 发起第 6 个 connect THEN 第 6 个经 subscribeSession 收到 `connect-done(status=fail)` 或 error 事件（无 connect-done(ok)）；CLI 与 JS/ETS 共享同一调用方配额，不绕过；disconnect 释放后可重连。WHEN cliMgr 全局存活 CLI 会话数达并发进程上限（默认 8，CCM 可配）THEN execTool 入口早拒抛 35600031（Maximum number of processes has been reached）。 | 终端用户 / UT看护 | 边界 |

### US-5: 特性隔离

**作为** 系统，
**我要** 用 CCM 配置项隔离本特性，
**以便** 按产品启用。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-10 | WHEN `const.product.ohos_agent_cli.enabled`=false THEN `cliManager.execTool("ohos-agent",...)` reject 抛 35600030（工具不存在，ohos-agent 工具未注册）。 | 终端用户 | 正常 |
| AC-11 | WHEN 同一 session 内多次 `sendMessage`（sendData/authorize）THEN 复用同一连接，不重连、不重复建连（经 subscribeSession 无多余 connect-done）。 | 终端用户 | 正常 |

### US-6: 故障与异常恢复

**作为** 调用方，
**我想要** 在入口拒绝、强制终止、配额耗尽、对端断连及各类异常场景下获得可观察的失败信号与资源恢复，
**以便** 可靠地感知故障并重连。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|------------|------|
| AC-6 | （见 US-4）入口拒绝：非系统应用/缺权限经 execTool connect 被拒，execTool 抛 202/201。 | 终端用户 | 异常 |
| AC-13 | WHEN 调用方经 `cliManager.clearSession(sessionId)` THEN 经 subscribeSession 收到 `CliToolEvent{EXIT}`（**无** disconnect-done 事件，不走正常断连流程）；querySession 返回 status=FAILED；连接配额由系统侧 caller-death 清理后可重连。 | 终端用户 | 异常 |
| AC-14 | （见 US-4）配额耗尽：第 6 个并发 execTool connect 经 subscribeSession 收 connect-done(fail)/error；execTool 并发进程上限抛 35600031。 | 终端用户 / UT看护 | 边界 |
| AC-15 | WHEN 对端 AgentExtensionAbility 主动断连 THEN 建立方经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"disconnect-done","reason":"peer"}'}`，session 干净退出（`CliToolEvent{EXIT}`）；连接配额释放后可重连。 | 终端用户 | 异常 |
| AC-Ex | WHEN 发生可恢复异常（sendMessage 下发非合法指令——未识别 op / 非法 JSON / data 缺失或非字符串、disconnect 被拒、sendData/authorize ret 不等于 0）THEN 经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"error","fatal":false,...}'}`，会话不结束、循环继续，可重发；WHEN 发生不可恢复异常（ohos-agent 子进程基础设施错误：poll 失败/stdin 读失败/wakeup 管道失败）THEN 经 subscribeSession 收到 `CliToolEvent{STDOUT, data='{"type":"error","fatal":true,...}'}` 后收到 `CliToolEvent{EXIT}`，querySession 返回 status=FAILED。 | 终端用户 | 异常 |

> AC-Ex 为 US-6 聚合的异常场景描述项，承接 R-24/R-29（fatal=false，含 bad-json/unknown-op/bad-data/no-receiver/send-failed/authorize-failed/disconnect-failed）与 R-30/R-31（fatal=true，含 poll-error/stdin-read-error/wakeup-pipe-failed）的可观察行为；不新增独立 AC 编号，复用 AC-2/AC-3/AC-5 既有的异常子句。

## 验收追溯

> 本表只维护 Spec 内部的 AC → 规则 → 可观察表面事实。AC → Task 映射由 `execution-plan.md` 的"AC 到 Task 追溯"维护；测试入口、Red 条件和通过标准由下方"验证映射"维护；实际执行证据由 `evidence/reviews/` 维护。

| AC | 关联规则 | 可观察表面 |
|----|----------|------------|
| AC-1 | R-1,R-7,R-8,R-12,R-19,R-20,R-22 | 终端用户 |
| AC-2 | R-2,R-7,R-10,R-24,R-28,R-31,R-32 | 终端用户 |
| AC-3 | R-2,R-7,R-10,R-18,R-28 | 终端用户 |
| AC-4 | R-3 | 终端用户 |
| AC-5 | R-4,R-7,R-10,R-20,R-25,R-26,R-29,R-30,R-33 | 终端用户 |
| AC-6 | R-13,R-21 | 终端用户 |
| AC-7 | R-14 | UT看护 |
| AC-8 | R-15,R-21 | UT看护 |
| AC-9 | R-9,R-12,R-23 | 终端用户 |
| AC-10 | R-16 | 终端用户 |
| AC-11 | R-5 | 终端用户 |
| AC-12 | R-12,R-17,R-27 | 终端用户 |
| AC-13 | R-6,R-33 | 终端用户 |
| AC-14 | R-11 | 终端用户 / UT看护 |
| AC-15 | R-4,R-20,R-25,R-33 | 终端用户 |

## 规则定义

> 类型标签：**行为**（正常路径下的系统行为）、**边界**（输入/状态的临界点，含会话超时上限与并发上限）、**异常**（非法输入或异常状态的处理）、**恢复**（系统异常后的恢复策略）。规则只描述经 cliManager 公共 API 可观察的行为与边界，不含内部符号/方法名/file:行（见 design.md）。

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 调用方经 `cliManager.execTool("ohos-agent","connect",...)`（APP 类型目标） | execTool 返回 `CliSessionInfo{status:RUNNING}`；经 subscribeSession 收到 `CliToolEvent{STDOUT, data=connect-done(ok)}` | 仅 APP 类型 Agent | AC-1 |
| R-2 | 行为 | 建立方经 `cliManager.sendMessage` 下发 sendData/authorize 指令 | 建立→对端数据发送：指令经 session 投递至对端 AgentExtensionAbility，sendMessage resolve | sendMessage message ≤10240（R-10） | AC-2,AC-3 |
| R-3 | 行为 | 对端主动回调 | 对端→建立方数据回流：对端回调经 `CliToolEvent{STDOUT, data=OnData/OnAuthorize}` 回流，建立方跨块按 `\n` 重组 + JSON 解析 | 跨块按 `\n` 重组（CliToolEvent.data 自定界 JSON-line） | AC-4 |
| R-4 | 行为 | sendMessage disconnect 或 对端断连 | disconnect 走正常断连流程并经 `CliToolEvent{STDOUT, data=disconnect-done(reason=user/peer)}` 上报 | CLI 路径 disconnect 仍走正常断连校验，不跳过 | AC-5,AC-15 |
| R-5 | 行为 | 已建连 | 同 session 复用同一连接对象，sendMessage 不重连 | 不重连、不重复建连 | AC-11 |
| R-6 | 行为 | 调用方经 `cliManager.clearSession(sessionId)` | clearSession 强制 kill 工具进程（不走正常 disconnect 流程，无 disconnect-done）；经 `CliToolEvent{EXIT}` 上报；系统侧 caller-death 清理释放连接配额 | 强制兜底路径 | AC-13 |
| R-7 | 行为 | connect 握手成功 | 进入阻塞式命令循环，按 sendMessage 指令分派，至 disconnect/对端断连/EOF/超时/fatal 退出；CliToolEvent{STDOUT} 增量交付 | EOF 退出 | AC-1,AC-2,AC-3,AC-5 |
| R-8 | 边界 | 目标 Agent 类型非 APP | connect 仅对 APP 类型 Agent 生效 | Agent 类型由注册定义；LOW_CODE/ATOMIC_SERVICE 不支持 | AC-1 |
| R-9 | 边界 | 会话运行达 `execOptions.timeout` 上限（默认 1800s），或调用方以 timeout>1800s 发起 execTool connect | 长时运行受系统最大长时上限（1800s）约束：运行期达上限→`CliToolEvent{STDOUT, data=disconnect-done(reason=timeout)}`+`CliToolEvent{EXIT}`；建立期 timeout>1800s→被拒（无 connect-done(ok)，收 error 或 CliToolEvent{EXIT}） | 会话级最大超时上限 1800s（边界，非性能） | AC-9 |
| R-10 | 边界 | `cliManager.sendMessage` 的 message 为字符串 | message 最大长度 10240，超限 sendMessage reject 抛 35600033（写消息失败） | message ≤10240（边界）；自定界 JSON 值（非纯文本，`\n` 可选） | AC-2,AC-3,AC-5 |
| R-11 | 边界 | cliMgr 全局存活 CLI 会话数达并发进程上限，或同一调用方已并发连 5 个不同 Agent 再发起第 6 个 execTool connect | **并发进程上限**（cliMgr 全局）：cliMgr 全局存活 CLI 会话数（跨所有调用方与所有 CLI 工具会话；1 会话=1 工具进程）达上限→execTool 入口早拒（spawn 前）抛 35600031（Maximum number of processes has been reached）；**连接配额上限**（agentmgr 每调用方）：第 6 个经 subscribeSession 收 connect-done(status=fail) 或 error 事件（无 connect-done(ok)）；CLI 与 JS/ETS 共享同一调用方配额，不绕过、不另设账本；disconnect 释放后可重连 | 并发进程上限=CCM `const.sys.cli.limit_maximum_concurrency_quantity`（默认 8，可配，cliMgr 既有边界，非性能）；每调用方 Agent 连接配额=5（边界，非性能） | AC-14 |
| R-12 | 边界 | connect 以"后台长驻"方式（`execOptions.background=true`）启动；若误用"前台短驻"须显式带 `yieldMs>0` 触发 yield 安全网（AC-12）；"前台短驻 + yield=0"为未支持误用→跑至 1800s 上限被终止（R-23） | connect 支持路径=后台长驻；ohos-agent.json connect 子命令声明推荐 yieldMs 供调用方参考 | 误用兜底（实现见 design.md） | AC-1,AC-12,AC-9 |
| R-13 | 行为 | 调用方非系统应用或缺少 `ohos.permission.EXEC_CLI_TOOL` | execTool 入口须系统应用 + EXEC_CLI_TOOL，不满足则 reject 抛 202（非系统应用）/201（权限） | 入口门禁 | AC-6 |
| R-14 | 行为 | 非 connect 建立方对该 session 调用 sendMessage/clearSession | session 操作须建立方（owner 校验） | owner 校验 | AC-7 |
| R-15 | 行为 | 非 CLI 进程发起 agent connect | 走原有 connect 权限/前台校验，行为不变；CLI 专用路径（特性门禁）不启用 | CLI 路径须防伪（实现见 design.md） | AC-8 |
| R-16 | 行为 | `const.product.ohos_agent_cli.enabled`=false | 特性受 CCM 配置项 `const.product.ohos_agent_cli.enabled` 隔离：off 时工具未注册、`execTool("ohos-agent",...)` reject 抛 35600030（工具不存在） | 同时 gate 工具注册消费与 CLI 入口 | AC-10 |
| R-17 | 行为 | connect 以 `execOptions{background:false,yieldMs:Y}` 启动且 session 运行超 Y | 长驻/后台由既有 yield-time 机制承担：到点自动转后台、回流 `CliSessionInfo`，不 kill，继续 | 复用既有 yield 机制（实现见 design.md） | AC-12 |
| R-18 | 行为 | 建立方经 `sendMessage` 下发 `{"op":"authorize",...}` | authorize 的 data 语义由 Agent 侧定义，CLI 仅透传 opaque data，不阻塞等 ack | O-5/O-6 已解决 | AC-3 |
| R-19 | 边界 | 目标 agentId 未预注册为 APP 类型 | 目标 agentId 须已预注册为 APP 类型 | APP 类型预注册要求（O-12/D-9） | AC-1 |
| R-20 | 行为 | 失败检测 | **失败检测契约**：失败经 **`CliToolEvent{EXIT}` + `CliSessionInfo{status=FAILED, result.exitCode≠0}`**（querySession/execTool promise）判定；`connect-done(status=fail)` 仅信息性事件，不作为失败判据（订阅竞态可能丢失）；subscribeSession/querySession/sendMessage 对不存在 session 抛 35600032 | 订阅前不感知模型 | AC-1,AC-5,AC-15 |
| R-21 | 行为 | ohos-agent connect 子命令 `requirePermissions` 声明 `ohos.permission.CONNECT_AGENT` | connect 子命令 `requirePermissions` 声明 CONNECT_AGENT 作为纵深防御；CLI 路径对真实调用方 token 强制 CONNECT_AGENT+system-app+前台校验（不跳过、不放行） | CLI 路径对真实 token 强制校验，requirePermissions 为纵深防御 | AC-6,AC-8 |
| R-22 | 异常 | connect 握手失败（对端不存在/权限/超时） | connect 失败 | connect 握手超时 10s（session 级 1800s 为兜底）；**失败信号=`CliToolEvent{EXIT}` + status=FAILED**；`connect-done(status=fail)` 仅信息性 | AC-1 |
| R-23 | 异常 | timeout 到达，会话达上限 | 会话超时 | ≤1800s；`CliToolEvent{STDOUT, data=disconnect-done(reason=timeout)}`+`CliToolEvent{EXIT}` 退出（边界，非性能） | AC-9 |
| R-24 | 异常 | sendMessage 下发非合法指令（三条子情形）：(a) 非法 JSON（不可解析/非 JSON object，含超内部上限超长行被 drop+continue）→ errCode=bad-json；(b) 合法 JSON 但 op 不属于 {sendData,authorize,disconnect}（未识别 op）→ errCode=unknown-op；(c) sendData/authorize 缺 data 字段或 data 非字符串 → errCode=bad-data | 非合法指令统一忽略该条 | N/A；`CliToolEvent{STDOUT, data=error(fatal=false)}`（errCode=bad-json/unknown-op/bad-data），忽略该条、循环继续、会话不结束、可重发 | AC-2,AC-3,AC-Ex |
| R-25 | 异常 | 对端 AgentExtensionAbility 主动断连 | 对端主动断连 | N/A；`CliToolEvent{STDOUT, data=disconnect-done(reason=peer)}`+`CliToolEvent{EXIT}` | AC-5,AC-15 |
| R-26 | 异常 | 工具进程异常退出（崩溃） | CLI 子进程崩溃 | N/A；系统侧回收 + `CliToolEvent{EXIT}` + status=FAILED + caller-death 清理 | AC-5 |
| R-27 | 异常 | 前台短驻 + yield 到点 | yield 自动转后台 | ≤yieldMs；转后台 + 回流 CliSessionInfo，不 kill，继续 | AC-12 |
| R-28 | 异常 | sendMessage 写失败（队列满/写超时） | sendMessage 写失败 | 抛 35600033（写消息失败）；指令投递失败、对端无到达 | AC-2,AC-3 |
| R-29 | 异常 | disconnect 被拒（鉴权失败/入参问题） | disconnect 被拒 | N/A；`CliToolEvent{STDOUT, data=error(fatal=false)}` disconnect-failed；会话不结束、循环继续，可重发 | AC-5 |
| R-30 | 异常 | ohos-agent 内部 poll 失败 | poll 失败 | N/A；`CliToolEvent{STDOUT, data=error(fatal=true)}` poll-error；后随 `CliToolEvent{EXIT}`，status=FAILED | AC-5 |
| R-31 | 异常 | ohos-agent stdin 读失败 | stdin 读失败 | N/A；`CliToolEvent{STDOUT, data=error(fatal=true)}` stdin-read-error；后随 `CliToolEvent{EXIT}`，status=FAILED | AC-2 |
| R-32 | 异常 | `cliManager.sendMessage` 的 message 长度 > 10240 | message 超限 | N/A；sendMessage reject 抛 35600033（写消息失败），对端无到达 | AC-2 |
| R-33 | 恢复 | 工具进程死亡 | 系统侧回收 session；agent 侧 caller-death 清理连接 | 资源释放；session 结束，建立方收 `CliToolEvent{EXIT}` | AC-5,AC-13,AC-15 |
| R-34 | 恢复 | 编排服务（cliMgr SA）重启 | 子进程为独立进程，不受 SA 重启影响；SA 重启后会话记录清空→querySession/subscribeSession/sendMessage 对已失效 session 抛 35600032；建立方需客户端超时兜底 | 子进程继续运行或随 stdin 断开自然结束；建立方需客户端超时兜底 | — |
| R-35 | 边界 | 调用方经 `cliManager.execCmd(cmd, ...)` 调起 ohos-agent connect（而非经 `cliManager.execTool`） | execCmd 非 ohos-agent connect 的受支持调用路径：ohos-agent connect 的契约消费方式仅为 `cliManager.execTool("ohos-agent","connect",...)`（见接口规格）；经 execCmd 调起不在契约范围、行为不保证、不作为通过判据（不新增可观察失败码，实现侧路径差异见 design.md） | 契约路径=execTool（execCmd 不在当前需求范围） | AC-1 |

> **no-receiver 不可达性说明**（R-24 子项脚注，不新增 AC 编号）：errCode=no-receiver 对应 receiver proxy 为 null 的防御性守卫。connect-done(ok) 保证 receiver 可用（见 design CR-1：remoteObject==null 或 iface_cast 返 null 时 ok=false → CONNECT_RESULT_PROXY_NULL=-2 → connect-done(fail)+exitCode=1 → RunCommandLoop 不进入）。RunCommandLoop 持 receiver 的 sptr const 引用（调用方 RunAsConnect 栈帧持有，循环期间存活），循环中不可变为 null。故 no-receiver 在正常连接成功流程下不可达，为防御性守卫（应对逻辑缺陷，非运行期常态）；对端销毁（agent 进程死亡）→ binder proxy 变 dead binder（仍非 null sptr）→ SendData 返 DEAD_OBJECT(ret 不等于 0) → send-failed(fatal=false)，非 no-receiver。故 no-receiver=非致命已正确（fatal=false）。

### 边界与稳定性上限（汇总）

> 以下为**边界/稳定性**约束（非性能），ohos-agent 工具（经 cliManager 消费）新增功能必须保持不变；实现侧的账本/常量/链路/内部缓冲上限见 design.md。

**1. execTool 并发进程上限（cliMgr 既有公共契约）**

`cliManager.execTool` 在 cliMgr 全局存活 CLI 会话数（跨所有调用方与所有 CLI 工具会话；1 会话=1 工具进程）达上限时入口早拒（spawn 前）抛 35600031（Maximum number of processes has been reached）。上限由 CCM `const.sys.cli.limit_maximum_concurrency_quantity` 决定（默认 8，可配）。属 cliMgr 既有公共契约边界，非性能指标。

**2. 每调用方 Agent 连接配额=5**

同一调用方最多同时建连 5 个 Agent（第 6 个 execTool connect 经 subscribeSession 收 connect-done(fail)/error，无 connect-done(ok)），disconnect 后释放槽位可重连。CLI 与 JS/ETS 共享同一调用方配额账本，不可绕过、不另设账本（实现见 design.md）。

**3. 会话级最大超时上限=1800s**

`execOptions.timeout` 默认 1800s、上限 1800s；单个长驻 session 最长运行 1800s，到期经 `CliToolEvent{STDOUT, data=disconnect-done(reason=timeout)}`+`CliToolEvent{EXIT}` 退出；建立期 timeout>1800s 的 execTool connect 在建立期即被拒。属边界约束，非性能指标。

**4. sendMessage 消息上限=10240**

`cliManager.sendMessage` 的 message 最大长度 10240，超限 reject 抛 35600033（写消息失败）。属 cliManager 公共契约边界。

## 验证映射

> 每个 AC 至少关联一个可执行测试入口。`Red 条件` 描述实现前测试为什么必然失败以及失败时可观察到什么（以 cliManager 公共 API 可观察表面表述）。实现后测试从 Red 转 Green 的实际结果和证据由 execution-plan / Verification Evidence 回填。测试入口（单测/集成用例名）属测试侧工件，实现链路见 design.md。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号，cliManager 可观察） | 通过标准（cliManager 可观察） |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1/R-7/R-8/R-19/R-20/AC-1 | `agent_manager_service_test::ConnectForCli_AllOk`；`ohos_agent_test::OnAbilityConnectDone_Success`、`BuildEvents`、`RunCommandLoop_ReassemblesAcrossBlocks` | 集成 | execTool connect 未实现→execTool 不返回 status=RUNNING、subscribeSession 无 connect-done(ok) CliToolEvent | execTool 返回 `CliSessionInfo{status:RUNNING}` + subscribeSession 收 `CliToolEvent{STDOUT, data=connect-done(ok)}`（跨块重组解析）+ 会话长驻不退出 |
| VM-2 | R-2/R-7/R-10/R-28/AC-2,AC-3,AC-11 | `ohos_agent_test::RunCommandLoop_SendMessage_Success`、`RunCommandLoop_SendMessage_Fail`、`RunCommandLoop_Authorize`、`ConnectorStub_SendData`、`ConnectorStub_Authorize` | 集成 | CommandLoop 分派未实现→sendMessage 指令不被解析、对端收不到 data | sendMessage(sendData/authorize) resolve + 对端到达 + 同 session 多指令复用连接（无多余 connect-done）；message>10240 抛 35600033 |
| VM-3 | R-3/AC-4 | `ohos_agent_test::ConnectorStub_SendData`、`RunCommandLoop_ReassemblesAcrossBlocks`、`BuildEvents_OnData`/`BuildEvents_OnAuthorize` | 集成 | 对端→caller 回流通道未接线→subscribeSession 无 OnData/OnAuthorize CliToolEvent（无 STDOUT 事件） | subscribeSession 收 `CliToolEvent{STDOUT, data=OnData/OnAuthorize}`（跨块重组解析） |
| VM-4 | R-4/R-7/R-10/R-20/R-25/R-26/R-29/R-30/R-33/AC-5,AC-15 | `agent_manager_service_test::DisconnectForCli_AllOk`；`ohos_agent_test::OnAbilityDisconnectDone_PeerReasonAndWakeup`、`OnAbilityDisconnectDone_UserReason`、`OnAbilityDisconnectDone_ErrorReason`、`TryInitiateDisconnect_CAS`、`RunCommandLoop_AfterDisconnectInitiated_SkipsDispatch` | 集成 | disconnect 流程未实现→sendMessage(disconnect) 无 disconnect-done CliToolEvent | sendMessage(disconnect) → subscribeSession 收 `CliToolEvent{STDOUT, data=disconnect-done(reason=user/peer)}`+`CliToolEvent{EXIT}` |
| VM-4b | R-6/R-33/AC-13 | `ohos_agent_test::RunCommandLoop_EOF_Exit`（强制终止兜底路径）；集成 clearSession 子进程 | 集成 | clearSession 与 caller-death 清理未接线→clearSession 后无 CliToolEvent{EXIT}、agent 侧连接泄漏 | clearSession → subscribeSession 收 `CliToolEvent{EXIT}`（无 disconnect-done）+ caller-death 清理 + querySession status=FAILED |
| VM-5 | R-13/R-14/R-15/R-21/AC-6~8 | `agent_manager_service_test::ConnectForCli_CcmOff`、`ConnectForCli_NotCliTool`、`ConnectForCli_NotSystemApp`、`ConnectForCli_NoConnectAgentPerm`、`ConnectForCli_NotForeground`、`ConnectForCli_UidSpoof`、`ConnectForCli_CallerIdentityNotHap`、`ConnectForCli_NonAppType`、`DisconnectForCli_CcmOff`、`DisconnectForCli_Spoof` | 单测 | 入口门禁/owner/防伪/CCM 特性门禁未实现→非系统应用/无权限/伪造/非 CLI 均未被拒（execTool 错误码不匹配），非 CLI 不走原校验 | 入口门禁/owner/CCM 特性门禁（execTool 抛 202/201；不跳过校验，全分支确切错误码） |
| VM-6 | R-9/R-12/R-23/AC-9 | `agent_manager_service_test` timeout 单测分支 | 单测 | timeout 上限拒绝未实现→timeout>1800s 不被拒、运行达上限无 disconnect-done(reason=timeout) | timeout 上限拒绝（>1800s 被拒；达上限 `CliToolEvent{STDOUT, data=disconnect-done(reason=timeout)}`+`CliToolEvent{EXIT}`） |
| VM-6b | R-12/R-17/R-27/AC-12 | 集成 yield-time；`ohos_agent_test::RunCommandLoop` 退出路径 | 集成 | yield 复用未接线→execOptions{background:false,yieldMs:Y} 到点不自动转后台、不回流 CliSessionInfo、session 被 kill | `execOptions{background:false,yieldMs:Y}` 到点自动转后台 + 回流 CliSessionInfo + session 继续（不被 kill） |
| VM-7 | R-16/AC-10 | `agent_manager_service_test::ConnectForCli_CcmOff`、`DisconnectForCli_CcmOff` | 单测 | CCM 开关未 gate→CCM off 时 execTool 不抛 35600030、工具仍可用 | CCM 开关隔离（off 时 execTool 抛 35600030 工具不存在） |
| VM-8 | R-11/AC-14 | `agent_manager_service_test` 配额单测（既有标准连接路径） | 单测 | CLI 未复用既有配额链→第 6 个不同 Agent execTool connect 不被拒，或 CLI 与 JS 不共享调用方桶（绕过）；execTool 并发进程上限未抛 35600031 | 第 6 个并发 execTool connect 经 subscribeSession 收 connect-done(fail)/error（无 connect-done(ok)）；execTool 并发进程上限抛 35600031；CLI 与 JS 共享配额桶 |

> 测试入口汇总：`agent_manager_service_test`（service 端 14 例 + client 端 6 例，合计 20）；`ohos_agent_test`（23 例）。nm 确认全 TestBody 符号链入；构建 PASS，运行验证待真机。

## API 变更分析

> 本特性的对外 API = **ohos-agent CLI 工具**（经既有 `cliManager.execTool` 公共方法消费，additive，无 cliManager 公共 API 方法变更——execTool/subscribeSession/sendMessage/clearSession/querySession 等自 26.0.0 已存在）。ForCli 内部签名、内部调用链与防伪/配额实现属实现细节，见 design.md。

### 新增 API

| API 名称 | 开放级别 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| ohos-agent CLI 工具（含 connect 子命令，经 cliManager.execTool 消费） | 系统 CLI 工具（additive，经 cliManager 公共 API 消费） | `cliManager.execTool("ohos-agent","connect",{bundlename,abilityname,modulename,agentid}, challenge, ExecOptions)`；建连后经 `sendMessage(sessionId, '{"op":"sendData"\|"authorize"\|"disconnect",...}')` 下发指令、经 `subscribeSession(sessionId, callback)` 收 CliToolEvent | execTool → `Promise<CliSessionInfo>`；subscribeSession/sendMessage/clearSession/querySession → `Promise<void>`/`Promise<CliSessionInfo>` | execTool 抛 201/202/35600030/35600031/35600050；sendMessage 抛 35600032/35600033；subscribeSession/clearSession/querySession 抛 35600032/35600050 | 新增独立 CLI 工具，经 cliManager 公共 API 为 APP 类型 AgentExtensionAbility 提供长连会话通道（connect 长驻、sendData/authorize 双向、disconnect 正常断连、clearSession 强制结束、querySession 状态查询） | AC-1~5,AC-10~15 |

### 变更/废弃 API

| API 名称 | 当前开放级别 | 目标开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|--------------|--------------|----------|----------|----------|---------|
| N/A | — | — | — | — | — | — |

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源 | 关联 AC |
|--------|----------|----------|---------------|----------|----------|----------|---------|
| `cliManager.execTool` | Existing Public API | 公共（cliManager，@since 26.0.0） | `execTool(toolName:string, subCommand:string, args:Record<string,Object>, challenge:string, execOptions?:ExecOptions): Promise<CliSessionInfo>`（来自 @ohos.app.cli.cliManager.d.ts，@kit AbilityKit，@syscap SystemCapability.Ability.AgentRuntime.Core，@stagemodelonly，dynamiconly） | 受信调用方经 execTool 消费 ohos-agent connect 子命令建立 APP Agent 长连会话 | 返回 `CliSessionInfo{status:RUNNING}`+经 subscribeSession 收 connect-done(ok)；失败 reject 抛 201/202/35600030/35600031/35600050 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | AC-1,AC-6,AC-10,AC-14 |
| `cliManager.sendMessage` | Existing Public API | 公共（cliManager） | `sendMessage(sessionId:string, message:string): Promise<void>`（message 最大 10240） | 建立方经 session 下发 sendData/authorize/disconnect 指令 | resolve（指令投递）；message>10240 抛 35600033（写消息失败）；session 不存在抛 35600032 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | AC-2,AC-3,AC-5 |
| `cliManager.subscribeSession` | Existing Public API | 公共（cliManager） | `subscribeSession(sessionId:string, callback:ToolEventCallback): Promise<void>` | 订阅 session 事件 | 收到 `CliToolEvent{toolEventType, data}`（STDOUT/STDERR/EXIT/ERROR）；session 不存在抛 35600032 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | AC-1,AC-4,AC-5,AC-13,AC-15 |
| `cliManager.clearSession` | Existing Public API | 公共（cliManager） | `clearSession(sessionId:string): Promise<void>`（force kill tool process） | 调用方强制结束会话 | 经 subscribeSession 收 `CliToolEvent{EXIT}`（无 disconnect-done）；session 不存在抛 35600032 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | AC-13 |
| `cliManager.querySession` | Existing Public API | 公共（cliManager） | `querySession(sessionId:string): Promise<CliSessionInfo>` | 查询会话状态 | 返回 `CliSessionInfo{status, result?}`；session 不存在抛 35600032 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | AC-1,AC-9,AC-13 |
| ohos-agent connect 子命令（经 execTool 消费） | New CLI 工具 | 系统 CLI 工具（additive，经 cliManager 消费） | execTool args=`{bundlename,abilityname,modulename,agentid}`；stdin 指令 op=sendData\|authorize\|disconnect（经 sendMessage）；stdout 事件 connect-done/OnData/OnAuthorize/disconnect-done/error（经 `CliToolEvent{STDOUT}.data` 交付） | 受信调用方建立 APP Agent 长连会话 | 握手成功→connect-done(ok)+长驻命令循环；失败→`CliToolEvent{EXIT}`+status=FAILED 或 connect-done(fail)信息性 | `tools/ohos-agent/ohos-agent.json` | AC-1~5,AC-10~15 |
| `ERR_MAX_AGENT_CONNECTIONS_REACHED` | Existing Error Code | 系统 CLI 工具（outputSchema errCode，经 cliManager 消费） | 错误码枚举 `ERR_MAX_AGENT_CONNECTIONS_REACHED` | 同一调用方已并发连 5 个不同 Agent 再发起第 6 个 execTool connect | connect 被拒，经 subscribeSession 收 connect-done(fail)/error（携带该 errCode）；CLI 与 JS 共享调用方桶 | 既有错误码；ohos-agent.json outputSchema errCode | AC-14 |

## 接口规格

> 本节定义 **cliManager 公共 API（execTool/subscribeSession/sendMessage/clearSession/querySession 签名 + ExecOptions + CliSessionInfo/SessionStatus/ExecResult + CliToolEvent/ToolEventType + 错误码）+ ohos-agent 工具协议（connect 子命令入参 + stdin 指令 op=sendData\|authorize\|disconnect + stdout 事件 schema 经 `CliToolEvent{STDOUT}.data` 交付）**。ForCli 内部签名、d.ts 落点、内部调用链与实现结构见 design.md。

### cliManager 公共 API（@ohos.app.cli.cliManager，@kit AbilityKit，@syscap SystemCapability.Ability.AgentRuntime.Core，@stagemodelonly，dynamiconly，@since 26.0.0）

| 方法 | 签名 | 权限 | 错误码 |
|------|------|------|--------|
| execTool | `execTool(toolName:string, subCommand:string, args:Record<string,Object>, challenge:string, execOptions?:ExecOptions): Promise<CliSessionInfo>` | `ohos.permission.EXEC_CLI_TOOL` | 201（权限）/202（非系统应用）/35600030（工具不存在）/35600031（并发进程上限）/35600050（系统错误） |
| subscribeSession | `subscribeSession(sessionId:string, callback:ToolEventCallback): Promise<void>` | `ohos.permission.EXEC_CLI_TOOL` | 201/202/35600032（session 不存在）/35600050 |
| sendMessage | `sendMessage(sessionId:string, message:string): Promise<void>`（message 最大 10240） | `ohos.permission.EXEC_CLI_TOOL` | 201/202/35600032（session 不存在）/35600033（写消息失败）/35600050 |
| clearSession | `clearSession(sessionId:string): Promise<void>`（force kill tool process） | `ohos.permission.EXEC_CLI_TOOL` | 201/202/35600032/35600050 |
| querySession | `querySession(sessionId:string): Promise<CliSessionInfo>` | `ohos.permission.EXEC_CLI_TOOL` | 201/202/35600032/35600050 |
| queryTools/queryToolSummaries/getToolInfoByName | （查询工具元数据） | `ohos.permission.QUERY_CLI_TOOL` | 201/202/35600030/35600050 |

### 类型

| 类型 | 字段 |
|------|------|
| `ExecOptions` | `background?:boolean`（默认 false）；`yieldMs?:long`（前台等待超时，毫秒）；`timeout?:long`（最大执行时长，秒，默认 1800） |
| `CliSessionInfo` | `sessionId:string`；`toolName:string`；`status:SessionStatus`；`result?:ExecResult`（status=COMPLETED/FAILED 时有值） |
| `SessionStatus` | `RUNNING`/`COMPLETED`/`FAILED` |
| `ExecResult` | `exitCode?:int`（0=成功）；`outputText?:string`；`errorText?:string`；`signalNumber?:int`；`timeOut:boolean`；`executionTime:long` |
| `CliToolEvent` | `toolEventType:ToolEventType`；`data:string` |
| `ToolEventType` | `STDOUT`/`STDERR`/`EXIT`/`ERROR` |
| `ToolEventCallback` | `(event: CliToolEvent) => void` |

### ohos-agent connect 子命令（经 execTool 消费）

| 属性 | 值 |
|------|-----|
| 消费方式 | `cliManager.execTool("ohos-agent","connect",{bundlename,abilityname,modulename,agentid}, challenge, ExecOptions)` |
| 描述 | Connect an AgentExtensionAbility (APP type) and enter a long-lived command loop. |
| 契约路径 | 仅 `cliManager.execTool`（`cliManager.execCmd` 非受支持调用路径，见 R-35） |
| requirePermissions | `ohos.permission.CONNECT_AGENT`（子命令声明，纵深防御） |
| eventTypes（经 `CliToolEvent{STDOUT}.data` 交付） | `connect-done`、`OnData`、`OnAuthorize`、`disconnect-done`、`error` |
| 关联 AC | AC-1,AC-2,AC-3,AC-4,AC-5,AC-15 |

**入参约束（execTool args）**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| bundlename | string | 是 | — | 目标 AgentExtensionAbility 宿主 bundleName |
| abilityname | string | 是 | — | 目标 AgentExtensionAbility abilityName |
| modulename | string | 是 | — | 目标 AgentExtensionAbility moduleName |
| agentid | string | 是 | — | agentId，须预注册为 APP 类型（R-19/O-12/D-9） |
| challenge | string | 是 | — | 从 access token manager 取的唯一标识（execTool 第 4 参） |
| background | boolean | 否 | false | 后台长驻建议 true（R-12） |
| yieldMs | long | 否 | 0 | 前台短驻时须 >0 触发 yield 安全网（AC-12） |
| timeout | long | 否 | 1800 | 最大执行时长（秒），上限 1800（R-9） |

### stdin 指令协议（建立方→工具进程，经 `cliManager.sendMessage` 下发，自定界 JSON 值）

> 调用方经 `sendMessage(sessionId, <json-value>)` 下发指令；message 最大 10240，超限抛 35600033。ohos-agent 事件经 `CliToolEvent{STDOUT}.data` 交付。

| 指令 | 帧格式 | 约束 |
|------|--------|------|
| sendData | `{"op":"sendData","data":...}` | data 内换行必须 JSON 转义；sendMessage message ≤10240（R-10/R-32） |
| authorize | `{"op":"authorize","data":...}` | data 语义由 Agent 侧定义，CLI 仅透传 opaque data（R-18），不阻塞等 ack |
| disconnect | `{"op":"disconnect"}` | 走正常断连流程（R-4） |

### stdout 事件协议（工具进程→建立方，经 `CliToolEvent{STDOUT}.data` 交付，JSON-line）

| 事件 | data 帧格式 | 约束 |
|------|--------|------|
| connect-done | `{"type":"connect-done","status":"ok"\|"fail","agentId"?,"errCode"?,"errMsg"?}` | status=fail 仅信息性，不作失败判据（R-20） |
| OnData | `{"type":"OnData","data":...}` | 对端回调经 CliToolEvent{STDOUT}.data 回流 |
| OnAuthorize | `{"type":"OnAuthorize","data":...}` | 同上 |
| disconnect-done | `{"type":"disconnect-done","reason":"user"\|"peer"\|"timeout"\|"error"}` | reason=user/peer/timeout/error（R-25/R-23） |
| error | `{"type":"error","errCode":"<closed-enum>","errMsg"?,"fatal":bool}` | errCode 闭合枚举=["bad-json","unknown-op","bad-data","no-receiver","send-failed","authorize-failed","disconnect-failed","wakeup-pipe-failed","poll-error","stdin-read-error"]；errCode 必填（闭合集合）；fatal=true→后随 `CliToolEvent{EXIT}`、status=FAILED（R-30/R-31，含 poll-error/stdin-read-error/wakeup-pipe-failed）；fatal=false→忽略该条、循环继续、会话不结束、可重发（R-24/R-29，含 bad-json/unknown-op/bad-data/no-receiver/send-failed/authorize-failed/disconnect-failed） |

### 退出与事件映射

- 工具进程退出 → 调用方经 subscribeSession 收 `CliToolEvent{EXIT}`；querySession 返回 status=COMPLETED/FAILED + `result.exitCode`。
- ohos-agent 内部错误 → `CliToolEvent{STDOUT, data=error(fatal)}`；fatal=true 后随 EXIT。
- cliManager 层错误 → `CliToolEvent{ERROR}`（区别于 ohos-agent 的 STDOUT error 事件）。

### 分帧与重组约束

- **stdin（建立方→工具进程）**：自定界 JSON 值（self-delimiting）——每条指令一个 JSON 值，`\n` 为可选空白非必要；cliMgr `sendMessage` 原样写 inputText（不追加 `\n`），ohos-agent 经 SAX 结构化解析在 JSON 值闭合时派发（支持多值拼接与 pretty-print）；故调用方无需追加 `\n`，data 内换行用 JSON 转义。
- **stdout（工具进程→建立方）**：ohos-agent 每个 event 追加 `\n`；cliMgr `CliToolEvent{STDOUT}.data` 按裸字节块（≤4096）回调，不分帧、不按行切；调用方须自行跨块重组——可按 `\n` 重组行（因 ohos-agent 每 event 追加 `\n`）或结构化自定界解析，再按 `type` 字段分派。
- `cliManager.sendMessage` 的 message 最大 10240，超限抛 35600033（R-10/R-32）。
- 主循环 poll(stdin + wakeup)：stdin EOF（read 返 0）→正常退出（R-7）。

## 兼容性声明

- **已有 API 行为变更:** 否（ohos-agent CLI 工具经既有 cliManager.execTool 公共方法消费，additive；cliManager 公共 API 方法签名无变更——execTool/subscribeSession/sendMessage/clearSession/querySession 等自 26.0.0 已存在；agentmgr CLI 专用路径对真实调用方 token 强制校验不跳过；非 CLI 进程行为不变；均为 additive，向后兼容）
- **配置文件格式变更:** 是（新增 `ohos-agent.json` 工具注册配置 + CCM 开关 `const.product.ohos_agent_cli.enabled`，均为新增，不破坏既有）
- **数据存储格式变更:** 否
- **最低支持版本:** OpenHarmony-7.0-Release
- **API 版本号策略:** cliManager 公共 API 自 26.0.0 已存在（additive 消费，无签名变更）

## 架构约束

> 本节列出本特性 AC 验证必须满足的约束（用途：架构约束→受影响 AC 的可测性追溯与边界守卫）。架构规则适用性、设计方案与内部调用链见 design.md。

| 关键约束 | 约束说明 |
|----------|----------|
| 不改编排核心 | 不改 cli_tool_framework session 编排，仅数据驱动消费 |
| 只读消费接口 | 不改 kits/native 连接接口；ohos-agent 工具经既有 cliManager 公共 API 消费（additive，不破坏序列化格式） |
| 非 CLI 行为不变 | CLI 专用路径仅对特性门禁+CCM 启用生效，且对真实调用方 token 强制校验不跳过 |
| 特性隔离 | CCM 开关同时 gate 工具注册与 CLI 入口 |
| 连接上限硬性约束 | CLI 必须复用既有每调用方 Agent 连接配额（5 槽），不得引入独立配额账本（见「边界与稳定性上限」与 design.md） |
| 跨块重组责任在协议层 | `CliToolEvent{STDOUT}.data` 按块回调不分帧，调用方须跨块按 `\n` 重组或自定界解析 + JSON 解析 |

## 非功能性需求

> N/A 判定见 proposal.md 不涉及项确认。本节仅为适用项填写具体指标。1800s 会话超时上限、execTool 并发进程上限（35600031，默认 8）、每调用方并发上限（5）、sendMessage 消息上限（10240）均属**边界/稳定性**约束（见「规则定义」R-9/R-10/R-11/R-23 与「边界与稳定性上限」），不归性能类。

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | N/A（长驻会话，无独立吞吐/时延指标；数据投递时效由对端 Agent 决定） | — | — |
| 安全 | cliManager 入口门禁（系统应用 + EXEC_CLI_TOOL）+ CLI 路径对真实调用方 token 强制 CONNECT_AGENT/system-app/前台（不跳过、不放行）+ connect 子命令 `requirePermissions`=CONNECT_AGENT（纵深防御）+ session owner 校验 + CCM 隔离；防伪/身份传播属实现见 design.md | 扫描/单测 | 待补 |
| 可靠性 | 工具进程死亡/对端断连均有清理与事件上报（CliToolEvent{EXIT}/disconnect-done/caller-death 清理） | 压力测试 | 待补 |
| 可测试性 | 入口门禁/owner/CCM/协议/配额均有单测入口；失败检测契约可经 CliToolEvent{EXIT} + CliSessionInfo{status=FAILED} 或 session 不存在（35600032）观察 | 单测/集成 | 待补 |
| 定界定位 | connect/disconnect/error 事件经 `CliToolEvent{STDOUT}.data` 上报 + hilog；失败可经 CliToolEvent{EXIT} + status=FAILED 或 35600032 判定 | hilog+事件 | 待补 |

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
Feature: ohos-agent 长连会话通道（经 cliManager 公共 SDK 消费）
  作为 框架/系统侧调用方
  我想要 经 cliManager.execTool 消费 ohos-agent connect 连接 APP 类型 Agent 并双向通信
  以便 建立可控长连会话

  Scenario: 正常 connect 与双向通信
    Given 系统应用且持 ohos.permission.EXEC_CLI_TOOL，且 CCM 开关已启用
    When 调用方经 cliManager.execTool("ohos-agent","connect",{bundlename:"B",abilityname:"A",modulename:"M",agentid:"ID"}, challenge, {background:true,timeout:1800})（ID 为已注册 APP 类型）
    Then execTool 返回 CliSessionInfo{status:RUNNING, sessionId}
    When 调用方经 cliManager.subscribeSession(sessionId, callback)
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"connect-done","status":"ok"}'}，会话长驻
    When 调用方经 cliManager.sendMessage(sessionId, '{"op":"sendData","data":...}')
    Then 对端 AgentExtensionAbility 收到 data
    When 对端主动回调 sendData
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"OnData","data":...}'}
    When 调用方经 cliManager.sendMessage(sessionId, '{"op":"disconnect"}')
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"disconnect-done","reason":"user"}'}，后随 CliToolEvent{EXIT}，session 结束

  Scenario: 强制结束 session（clearSession）
    Given 已建连的 ohos-agent session（sessionId 已知）
    When 调用方经 cliManager.clearSession(sessionId)
    Then callback 收到 CliToolEvent{EXIT}（无 disconnect-done 事件），querySession 返回 status=FAILED，连接配额由 caller-death 清理后可重连

  Scenario: 对端主动断连
    Given 已建连的 ohos-agent session
    When 对端 AgentExtensionAbility 主动断连
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"disconnect-done","reason":"peer"}'}，后随 CliToolEvent{EXIT}，session 干净退出

  Scenario: 非法 JSON 与 fatal error 退出
    Given 已建连的 ohos-agent session
    When 调用方经 cliManager.sendMessage(sessionId, '<非法 JSON>')
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"error","fatal":false,...}'，会话不结束、循环继续
    When ohos-agent 子进程发生不可恢复内部错误
    Then callback 收到 CliToolEvent{STDOUT, data='{"type":"error","fatal":true,...}'，后随 CliToolEvent{EXIT}，querySession 返回 status=FAILED

  Scenario: sendMessage 消息超限
    Given 已建连的 ohos-agent session
    When 调用方经 cliManager.sendMessage(sessionId, '<长度>10240 的 message>')
    Then sendMessage reject 抛 35600033（写消息失败），对端无到达

  Scenario: 第 6 个并发 connect 被拒（连接配额）
    Given 同一调用方已并发建连 5 个不同 Agent（5 个 execTool connect session 在运行）
    When 调用方经 cliManager.execTool("ohos-agent","connect",...) 发起第 6 个
    Then 第 6 个经 subscribeSession 收到 connect-done(status=fail) 或 error 事件（无 connect-done(ok)）；CLI 与 JS/ETS 共享调用方配额不绕过

  Scenario: execTool 并发进程上限
    Given cliMgr 全局存活 CLI 会话数已达上限（CCM const.sys.cli.limit_maximum_concurrency_quantity，默认 8）
    When 调用方经 cliManager.execTool("ohos-agent","connect",...)
    Then execTool reject 抛 35600031（Maximum number of processes has been reached）

  Scenario: 入口拒绝（非系统应用/缺权限）
    Given 非系统应用或缺少 ohos.permission.EXEC_CLI_TOOL 的调用方
    When 调用方经 cliManager.execTool("ohos-agent","connect",...)
    Then execTool reject 抛 202（非系统应用）或 201（权限），无 connect-done(ok)

  Scenario: 非 CLI 进程走原校验
    Given 一个非 CLI 进程（CLI 特性门禁不适用）
    When 它发起 agent connect
    Then 走原有 connect 权限/前台校验（行为不变）

  Scenario: CCM 未启用
    Given const.product.ohos_agent_cli.enabled=false
    When 调用方经 cliManager.execTool("ohos-agent","connect",...)
    Then execTool reject 抛 35600030（工具不存在）
```

## context-references

```yaml
context-queries:
  - repo: "interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts"
    query: "cliManager 公共 API：execTool/subscribeSession/sendMessage/clearSession/querySession 签名 + ExecOptions/CliSessionInfo/SessionStatus/ExecResult/CliToolEvent/ToolEventType + 错误码 201/202/35600030/35600031/35600032/35600033/35600050"
  - repo: "ability_runtime/tools/ohos-agent"
    query: "ohos-agent CLI 工具注册配置（ohos-agent.json）：connect 子命令 schema、eventTypes/eventSchemas、requirePermissions"
  - repo: "ability_runtime/cli_tool_framework"
    query: "cliMgr session 编排模型：长驻 session、stdin/stdout JSON-line 分帧、yield-time、会话超时上限（实现细节，供 design 参考）"
  - repo: "ability_runtime/agent_runtime_framework"
    query: "agent 连接握手：APP 标准连接路径、connect 校验链、每调用方连接配额（实现细节，供 design 参考）"
  - repo: "ability_runtime/tools/ohos-aa"
    query: "ohos_cli_executable + json 注册范式与子命令 schema 声明"
```

**关键文档：** proposal.md、design.md
