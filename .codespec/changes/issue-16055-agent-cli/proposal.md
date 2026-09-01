# 需求文档

> 一份文档，从原始需求到基线结论。按阶段追加内容，不拆成多份独立文件。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | FEAT-20260825-001 |
| 需求号 | 20260528392515 |
| Epic | Agent |
| 需求名称 | ohos-agent CLI 工具：连接 AgentExtensionAbility 的长连会话通道 |
| 来源 | `.ignore/agent-cli-proposal.md`（仓内需求草案） |
| 提出人 | TBD（需求方） |
| Owner | yangxuguang-huawei |
| 目标发行版本 | 7.0 |
| 候选 Profile | ability_runtime（本变更属 ability_runtime 部件/仓） |
| 优先级 | P1 |
| 状态 | Approved（基线已通过 2026-08-25；Stage 2 规格 Approved；Stage 3 已实现） |

### 原始描述

**原始问题（保留需求方原话意图）：** 提供一个 AgentCLI 工具，通过 CLI 可以拉起、断连、发送数据到鸿蒙端侧 AgentExtensionAbility；工具支持长时运行，长时运行阶段不主动断连 AgentProxy；Agent 管理服务支持 CLI 连接 AgentExtensionAbility。命令行调用需通过 cliManager.execTool，cliMgr fork 子进程经 execvp 调用工具；工具需实现自己的 IAbilityConnection，其回调通过 cliManager 已有的 session 模型通知调用方。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 框架/系统侧调用方 | 缺少通过 cliMgr session 体系拉起并长连 AgentExtensionAbility 的标准 CLI 工具 | 无法以受控 session 方式对 APP 类型 Agent 做连接/双向数据/断连 |
| Agent 调试与集成 | 无标准通道把 AgentExtensionAbility 的双向数据回调回流给真正调用方 | 对端推送数据无法经 session 订阅送达 |

**期望结果：** 通过 `ohos-agent` CLI 工具（cliMgr ExecTool 拉起长驻 connect 子进程），建立方经 `SubscribeSession` 即可完成对 APP 类型 AgentExtensionAbility 的 connect / sendData / authorize / disconnect 全流程，且对端主动推送的数据经 session 回流建立方。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 仓内需求草案 | `.ignore/agent-cli-proposal.md` | 原始需求与初步设计思路（含若干需澄清项） |
| 既有范式 | `tools/ohos-aa/`（`ohos_cli_executable` + `ohos-aa.json`） | CLI 工具注册与子命令 schema 的镜像范式 |
| 既有 session 编排 | `cli_tool_framework/services/climgr/` | ExecTool/SendMessage/SubscribeSession/ClearSession 完整模型 |
| 既有 agent 连接 | `agent_runtime_framework/` | AgentManagerService、AgentManagerClient、IAgentConnector/IAgentReceiver、AgentCardType |

### 初始范围

**可能包含：**
- 新建 `tools/ohos-agent/` CLI 工具（`connect` 子命令，长驻 session 模式）
- agentmgr connect 路径对 CLI 子进程放行（IsCliToolToken）
- CCM 特性隔离开关 + selinux 配置

**明确不包含：**
- 不改 ohos-aa / aa
- 不改 cli_tool_framework 编排核心
- 不支持 LOW_CODE / ATOMIC_SERVICE 类型 Agent
- 不新增/改公共 SDK API

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| cliMgr session 模型可承载长驻 connect 子进程（background+stdin+stdout 事件） | 技术 | 源码：SessionRecord/IOMonitor/ExecOptions | 已验证 |
| OnAbilityConnectDone 交付的 remoteObject 可消费为对端数据通道 proxy | 技术 | 源码：js_agent_receiver_proxy.cpp:66 iface_cast<IAgentReceiver> | 已验证（host→agent=IAgentReceiver 双参） |
| agentmgr 现有 connect 校验对 CLI 子进程（后台进程）会因前台校验失败 | 技术 | 源码：ValidateConnectCallerForeground | 已验证（故需 IsCliToolToken 放行） |
| 目标发行版本未指定 | 业务 | 待需求方确认 | 待验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单仓 ability_runtime，多子目录，无跨仓/SIG |
| 涉及仓数量 | 1 | ability_runtime |
| 是否涉及 Public/System API | 否 | 仅只读消费既有 kits/native 接口，不新增/改 |
| 是否涉及安全/性能关键路径 | 是（安全） | connect 权限校验落点、IsCliToolToken 放行、session owner 鉴权 |
| 是否跨 SIG | 否 | 全在 ability_runtime |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确（责任人 TBD，已标注待确认）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断（标准）

---

## 二、澄清记录

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | connect/disconnect/sendmessage/authorize 的运行模型（长驻会话 / REPL / 单次） | IAbilityConnection 是进程内存对象，跨进程无法传递，决定整体架构 | 已澄清：长驻会话 + session stdin 协议 |
| Q-2 | sendmessage/authorize/disconnect 如何触达长驻 connect session | 新 fork 拿不到旧 IAbilityConnection，决定 session 路由 | 已澄清：connect=ExecTool 长驻；sendmessage/authorize=SendMessage(sessionId)→stdin；disconnect=stdin 命令走正常流程 / ClearSession 兜底 |
| Q-3 | 工具形态：复用 ohos-aa 新增 subcommand vs 独立工具 | 决定二进制/selinux/选项风格与长驻语义落点 | 已澄清：独立 `ohos-agent`（镜像 ohos-aa 范式，单 connect 子命令） |
| Q-4 | 安全/权限落点 | 原方案"查父进程 HAP"不可行（父=cliMgr/aimgr） | 已澄清：cliMgr ExecTool 入口门禁 + agentmgr IsCliToolToken 放行 + IsSessionOwner；不新增 connect 专属权限 |
| Q-5 | 连接对象 A 与双向 proxy/stub 物理落点 | 决定 B→caller 数据回流路径与 receiver stub 实现位置 | 已澄清：A 在 ohos-agent CLI 子进程（agentmgr ledger 追踪）；子进程持 IAgentConnector proxy + 实现 IAgentReceiver stub |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-25 | 需求方 + Agent | Q-1 运行模型 | 长驻会话进程 + session stdin 协议（非 REPL、非单次） | 纳入架构 |
| 2026-08-25 | 需求方 + Agent | Q-2 session 路由 | connect=ExecTool；sendmessage/authorize=SendMessage；disconnect=stdin 走正常流程，ClearSession 兜底 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | 工具命名与形态 | 独立 `ohos-agent`（非 ohos-agent-cli；不塞进 ohos-aa），单 connect 子命令 | 纳入范围 |
| 2026-08-25 | 需求方 + Agent | 连接范围 | 仅 `AgentCardType::APP`（澄清原草案"APP类型低码"自相矛盾：APP 与 LOW_CODE 为不同枚举值） | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | 设备表述 | 不体现设备（如 PC），改为"CCM 配置项隔离特性" | 纳入范围/AC |
| 2026-08-25 | 需求方 + Agent | disconnect 主路径 | stdin 下发 disconnect 走正常 DisconnectAgentExtensionAbility 流程；ClearSession 仅兜底 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | CCM 开关命名 | `const.product.ohos_agent.setting.enabled` | 纳入范围 |
| 2026-09-01 | 需求方 | CCM 开关重命名 | `const.product.ohos_agent.setting.enabled` → `const.product.ohos_agent_cli.enabled`（更简洁命名）；agentmgr 代码常量 OHOS_AGENT_SETTING_ENABLED→OHOS_AGENT_CLI_ENABLED 同步 | 修订 AC-10/BR-6 |
| 2026-08-25 | 需求方 + Agent | Q-4 安全 | cliMgr 入口门禁 + agentmgr IsCliToolToken 放行 + IsSessionOwner；不查父进程；不校验 ohos.permission.cli.CONNECT_AGENT | 纳入 AC/安全 |
| 2026-08-27 | 需求方 | Q-4 安全（修订） | connect 子命令 `requirePermissions` 声明 `ohos.permission.CONNECT_AGENT`（invoker 侧要求，补偿 agentmgr IsCliToolToken 放行跳过的 :583/:746/:882）；不新增权限值，复用既有 CONNECT_AGENT | 修订 BR-14/安全模型 |
| 2026-08-28 | 需求方 + Agent | Q-4 安全（最终修订） | 放弃 option F 放行 + requirePermissions/claw-ticket 思路；改用 agentmgr CLI 专用 ForCli 接口（ConnectAgentExtensionAbilityForCli/DisconnectAgentExtensionAbilityForCli）+ cliMgr callerIdentity(env)传播 appA 真实身份端到端：防伪(uid 一致性 GetHapTokenInfo(token).uid==CLI 子进程 uid=appA uid)+appA 真实 CONNECT_AGENT/system-app/前台+SetCallingIdentity(callerIdentity)→:438 捕获 appA→AMS 还原 appA→B 侧见 appA；cliMgr 1 行 env 改动(tool_util.cpp 加 ohos_cli_callerIdentity=ResetCallingIdentity)；身份串格式内部非公开→必须 cliMgr 捕获保存串；非 CLI 行为不变 | 修订 ADR-5/O-13/BR-14 |
| 2026-08-25 | 需求方 + Agent | 双向通道与"对端"语义 | 对端=B=被连接方=AgentExtensionAbility；A=连接对象（CLI 子进程内，agentmgr ledger 追踪）；B→A 数据经 stdout→session→SubscribeSession 回流建立方 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | Q-5 A 落点 | 乙：A 在 ohos-agent CLI 子进程（持 IAgentConnector proxy + 实现 IAgentReceiver stub） | 纳入架构 |
| 2026-08-25 | 需求方 + Agent | 汇总结论确认 | 用户对汇总结论回复"正确" | 进入基线写入 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | connect（长驻）/ sendData（caller→B）/ authorize（caller→B）/ B→caller 回流 / disconnect | 需求方 | 已确认 |
| 明确不包含哪些？ | 改 ohos-aa/aa；改 cli_tool 编排核心；LOW_CODE/ATOMIC_SERVICE 连接；公共 SDK API 变更；IPC 序列化变更；hisysevent 已发布事件变更 | 需求方 | 已确认 |
| 是否有分期策略？ | 无；单期交付 | 需求方 | 已确认 |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | 独立 `ohos-agent` 二进制（镜像 ohos-aa 范式，单 connect 子命令，长驻为默认语义） | 长驻/stdin/事件为自然默认；aa/ohos-aa 保持一次性纯净；selinux 域可配同 aa；选项风格继承 | 多一个二进制 | 推荐 |
| A-2 | 复用 ohos-aa 新增 connect 子命令 | 省一个二进制；selinux/风格天然继承 | ohos-aa main 为一次性（60s CommandTimer + _exit），长驻需 retrofit；职责混淆（能力管理 vs Agent 长连） | 放弃 |
| A-3 | 复用 `aa`（旧 ShellCommand）新增 connect | 同 A-2，且 aa 亦一次性 | 同 A-2 | 放弃 |

**取舍理由：** 长驻+stdin 协议+stdout 事件编码与 ohos-aa/aa 一次性能力管理是本质不同的关注点；独立二进制使长驻成为默认语义、无需 retrofit 一次性 main，且原草案本就预留独立工具与独立 selinux 域意图。省一个二进制的收益不足以抵消 retrofit 复杂度。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | tools/ohos-agent（新）、agent_runtime_framework/services/agentmgr（connect 放行）、cli_tool_framework（仅 json 注册消费）、sa_profile/selinux | 需求方 | 已确认 |
| 是否需要新增子系统或部件？ | 是：新增 `tools/ohos-agent` 部件（ohos_cli_executable） | 需求方 | 已确认 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 需求方 | 已确认 |
| 是否需要新增 System API？ | 否 | 需求方 | 已确认 |
| 是否会废弃已有 API？ | 否 | 需求方 | 已确认 |
| 是否需要新增权限声明？ | 复用既有 `ohos.permission.CONNECT_AGENT`：声明于 ohos-agent connect 子命令 `requirePermissions`（invoker 侧要求；agentmgr 侧因 IsCliToolToken 放行跳过该权限校验 :583/:746/:882）；cliMgr 入口仍用既有 `ohos.permission.EXEC_CLI_TOOL`。不新增权限值 | 需求方 | 已确认（2026-08-27 修订） |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容？破坏性变更？ | 仅新增工具 + agentmgr 对 CLI 进程放行分支；非 CLI 进程行为不变；无破坏性 | 需求方 | 已确认 |
| 性能 | 长驻 session 资源占用 | background session，timeout≤1800s（MAX_TIMEOUT=30min）；一个 session 一个连接；受 cliMgr 并发上限约束 | 需求方 | 已确认 |
| 安全 | 权限/鉴权 | cliMgr 入口（系统应用+EXEC_CLI_TOOL）；agentmgr IsCliToolToken 放行；SendMessage/ClearSession/SubscribeSession=IsSessionOwner；不查父进程 | 需求方 | 已确认 |
| 可靠性 | 子进程死亡/对端断连恢复 | OnAbilityDisconnectDone 上报 disconnect-done；CLI 子进程死亡由 cliMgr ReaperLoop 清理；session 兜底 ClearSession | 需求方 | 已确认 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| cli_tool_framework session 模型 | 运行 | ExecTool/SendMessage/SubscribeSession/ClearSession/IOMonitor 既有 | 已确认 |
| agent_runtime_framework kits/native 连接接口 | 编译/运行 | IAgentConnector/IAgentReceiver/AgentConnectorProxy/AgentReceiverStub 既有（只读消费） | 已确认 |
| agentmgr connect 校验逻辑 | 运行 | 需新增 IsCliToolToken 放行分支 | 已确认 |
| claw_sandbox + selinux 域 | 运行 | 工具经 claw_sandbox --cmd 拉起；selinux 域同 ohos-aa | 已确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| ~~host→agent 接口选型未确认~~ | 技术 | — | 已确认：host→agent=IAgentReceiver 双参（js_agent_receiver_proxy.cpp:66）；agent→host=IAgentConnector 单参 | 已关闭 |
| CCM 开关未启用产品上误连 | 安全 | 中 | 开关同时 gate 工具注册与 agentmgr 放行 | 已确认 |
| 目标发行版本未指定 | 进度 | 低 | manifest.target_release=TBD，待需求方确认 | 待确认 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估（无变更）
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] 标准及以上复杂度已完成方案探索（3 方案 + 取舍理由）

**结论:** 通过（用户对 Q1–Q5 汇总结论确认"正确"，所有澄清项已关闭）

---

## 三、需求基线

> **双向通道接口归属（以代码为准；iagent_receiver.h/iagent_connector.h 头注释方向反，O-7 待修）**：host→agent（CLI→B）走 `IAgentReceiver`，方法 `SendData(connectorProxy, data)` 两参数，CLI 持 `AgentReceiverProxy`、B 实现 `AgentReceiverStub`；agent→host（B→CLI）走 `IAgentConnector`，方法 `SendData(data)` 一参数，CLI 实现 `AgentConnectorStub`、B 持 `AgentConnectorProxy`。证据：`js_agent_receiver_proxy.cpp:66`、`js_agent_extension_stub_impl.h:18`、`js_agent_connector_stub_impl.h:17`。AC-2/3/4 与范围 IN 已据此修正。

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-25 |
| Owner | TBD |
| 确认人 | 需求方（待显式批准基线） |
| 复杂度 | 标准 |
| Profile | ability_runtime |
| 目标发行版本 | TBD（引用 manifest.target_release） |
| 版本状态 | proposed |

### 问题陈述

为 AgentExtensionAbility 提供一条经 cliMgr session 体系受控的长连会话 CLI 通道。当前缺少以标准 session 方式拉起并长连 APP 类型 Agent、把双向数据（sendData/authorize）经 session 回流给真正调用方的 CLI 工具。本次新增独立工具 `ohos-agent`（镜像 `tools/ohos-aa` 范式），仅 `connect` 子命令，长驻子进程实现 IAbilityConnection 与双向数据 proxy/stub，复用 cliMgr 既有 ExecTool/SendMessage/SubscribeSession/ClearSession 模型，受 CCM 配置项隔离。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 建立方经 cliMgr 拉起并长连 APP 类型 Agent | connect 成功且收到 connect-done；子进程长驻不退出 | 集成测试 + AC-1 |
| caller→B 双向数据 | sendmessage/authorize 到达 B | AC-2/3 |
| B→caller 回流 | B 推送数据经 SubscribeSession 送达建立方 | AC-4 |
| disconnect 走正常流程 | OnAbilityDisconnectDone 触发并上报 | AC-5 |
| 安全边界 | 非 CLI/非系统应用/非 owner 被拒 | AC-6/7/8 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为框架/系统侧调用方，我想要经 cliMgr 拉起长驻 connect 连接 APP 类型 Agent，以便建立可控长连会话 | P0 |
| US-2 | 作为调用方，我想要经 session 向对端发 sendData/authorize 并接收对端推送，以便双向通信 | P0 |
| US-3 | 作为调用方，我想要经 stdin disconnect 走正常断连流程，以便干净释放连接 | P0 |
| US-4 | 作为系统，我要确保仅受信 CLI 进程可连、仅 session 建立方可操作，以便守住安全边界 | P0 |
| US-5 | 作为系统，我要用 CCM 配置项隔离本特性，以便按产品启用 | P1 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 系统应用且持 `ohos.permission.EXEC_CLI_TOOL` 调 `ExecTool({toolName:"ohos-agent",subcommand:"connect",args:{bundleName,abilityName,moduleName,agentId},options:{background=true,timeout≤1800}})` THEN cliMgr fork 出 `ohos-agent connect` 子进程，连接 `AgentCardType::APP` AgentExtensionAbility 成功，建立方经 `SubscribeSession` 收到 connect-done 事件；子进程长驻不退出、不主动断连 AgentProxy | 正常 | US-1 |
| AC-2 | WHEN 建立方 `SendMessage(sessionId,{op:"sendmessage",data})` THEN cliMgr 写 CLI 子进程 stdin，子进程解析后经 `IAgentReceiver::SendData`（双参，CLI 持 `AgentReceiverProxy`）调用对端 B，B 收到 | 正常 | US-2 |
| AC-3 | WHEN 建立方 `SendMessage(sessionId,{op:"authorize",data})` THEN 子进程经 `IAgentReceiver::Authorize`（双参）调用对端 B，B 收到 | 正常 | US-2 |
| AC-4 | WHEN B（对端 AgentExtensionAbility）经双向通道回调 `sendData`/`authorize`（B 持 `AgentConnectorProxy`）THEN CLI 子进程的 `IAgentConnector` stub（`AgentConnectorStub`）接收，将数据作为事件写 stdout，经 cliMgr session→`SubscribeSession` 回调交付建立方（注：`agentInvoked` 属 host→agent 方向，语义另在 design 确认） | 正常 | US-2 |
| AC-5 | WHEN 建立方经 stdin 下发 `disconnect` THEN CLI 子进程走正常流程 `AgentManagerClient::DisconnectAgentExtensionAbility`→`OnAbilityDisconnectDone`→上报 disconnect-done 事件→干净退出；`ClearSession` 为兜底清理路径 | 正常 | US-3 |
| AC-6 | WHEN 非系统应用或无 `ohos.permission.EXEC_CLI_TOOL` 调 `ExecTool` THEN 被拒（`ERR_NOT_SYSTEM_APP`/`ERR_PERMISSION_DENIED`） | 异常 | US-4 |
| AC-7 | WHEN 非 connect 建立方对该 session 调 `SendMessage`/`ClearSession`/`SubscribeSession` THEN `IsSessionOwner` 拒绝 | 异常 | US-4 |
| AC-8 | WHEN 非 CLI 进程（`IsCliToolToken`=false）发起 agent connect THEN 走原有 connect 权限/前台校验（行为不变） | 边界 | US-4 |
| AC-9 | WHEN `ExecTool` 的 timeout>1800s THEN 被拒（`MAX_TIMEOUT` 上限） | 边界 | US-1 |
| AC-10 | WHEN `const.product.ohos_agent_cli.enabled`=false THEN 工具不可用、agentmgr 不放行 CLI 连接 | 边界 | US-5 |
| AC-11 | WHEN 同一 session 已有连接 THEN 复用同一 `IAbilityConnection`，不重复建连 | 边界 | US-1 |

### 范围边界

**包含：**
- 新建 `tools/ohos-agent/`（`ohos_cli_executable("ohos-agent")` + `ohos-agent.json` + `OhosAgentShellCommand`，仅 `connect` 子命令，长驻模式：不装 60s CommandTimer、stdin 循环、stdout 事件输出、实现 IAbilityConnection + `IAgentConnector` stub（B→CLI，`AgentConnectorStub`）+ 持 `IAgentReceiver` proxy（CLI→B，`AgentReceiverProxy`））
- agentmgr connect 路径新增 `IsCliToolToken` 放行分支（跳过前台/连接权限校验；非 CLI 进程行为不变）；invoker 侧 CONNECT_AGENT 要求改由 connect 子命令 `requirePermissions` 声明承担
- CCM 特性隔离开关 `const.product.ohos_agent_cli.enabled`（同时 gate 工具注册与 agentmgr 放行）
- selinux 域配置同 `ohos-aa`
- 链接既有：`agent_manager_client`（inner_api）+ `IAgentConnector/IAgentReceiver/AgentConnectorProxy/AgentReceiverStub`（kits/native，只读消费）

**不包含：**
- 不改 `ohos-aa` / `aa` 任何代码
- 不改 `cli_tool_framework` session 编排核心（纯数据驱动消费方，仅新增 `ohos-agent.json` 注册）
- 不支持 `AgentCardType::LOW_CODE` / `ATOMIC_SERVICE` 类型 Agent 连接
- 不新增/不修改公共 SDK API（`interfaces/kits/`）、不改 IPC 序列化格式、不改已发布 hisysevent 事件
- 不新增权限值；connect 子命令 `requirePermissions` 复用既有 `ohos.permission.CONNECT_AGENT`（invoker 侧要求，ADR-5 放行后补偿）

**分期策略（如适用）：** 无；单期交付。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| CLI 工具 | ability_runtime | `tools/ohos-agent/`（新） | — | 新增 | TBD |
| Agent 管理服务 | ability_runtime | `agent_runtime_framework/services/agentmgr/` | Agent 连接生命周期与校验 | 修改（connect 放行分支） | TBD |
| CLI 工具框架 | ability_runtime | `cli_tool_framework/` | session 编排 | 不改（仅 json 注册消费） | TBD |
| 系统能力/安全配置 | ability_runtime | `sa_profile/` + selinux 配置 | SA 注册与域配置 | 修改（新增工具域配置） | TBD |

### API 变更项清单

> 无 API 变更。仅只读消费既有 kits/native 接口（`IAgentConnector`/`IAgentReceiver`/`AgentConnectorProxy`/`AgentReceiverStub`）与 inner_api（`AgentManagerClient`）。标记 N/A。

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| N/A | — | — | 不新增/不修改任何 Public/System API |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否/N/A | 长驻 session 受既有 MAX_TIMEOUT(30min) 与 cliMgr 并发上限约束，无新指标 | — |
| 安全与权限 | 是 | connect 权限落点、IsCliToolToken 放行、IsSessionOwner、CCM 隔离 | design.md / spec.md |
| 兼容性 | 否/N/A | 仅新增工具+放行分支；非 CLI 进程行为不变 | spec.md |
| API/SDK | 否/N/A | 不新增/改任何 Public/System API | — |
| IPC/跨进程 | 是 | 双向 IPC（IAgentConnector/IAgentReceiver）、cliMgr↔子进程 stdin/stdout 管道 | design.md |
| 构建与部件 | 是 | 新增 `ohos_cli_executable` 部件、`ohos-agent.json` 注册、selinux/sa_profile | design.md |
| 国际化/无障碍 | 否/N/A | CLI 工具无 UI | — |
| 数据迁移 | 否/N/A | 无持久化数据结构变更 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API | 触发设计审批（本特性预期无） |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 manifest.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [x] `manifest.target_release` 已明确 TBD
- [x] `manifest.profile` 已确认 ability_runtime
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认

**基线结论:** ReadyForReview（澄清已通过；基线文档已写就，待需求方显式审阅批准）
