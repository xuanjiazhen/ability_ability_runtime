---
phase_status: approved
approval:
  status: approved
  approver: 需求方会话确认Y
  evidence: 澄清已通过Q1-Q5汇总结论确认正确;proposal经会话确认Y批准
  approved_at: 2026-08-25
  baseline_digest: ""  # TODO: run ohos-sdd approval-digest issue-16055-agent-cli
target_release:
  id: OpenHarmony-7.0-Release
  status: committed
---

# 需求文档

> 主文档只保存需求输入和已确认基线。澄清过程仅在 complex/critical，或出现澄清阻断、争议决策、跨 SIG/多仓协调时写入条件附录。
>
> **本文档为业务口径：只描述业务诉求与业务期望。一切实现/方案细节（内部接口签名、类名、枚举、常量、proxy/stub 归属、file:行 证据、ADR 方案对比、防伪与配额账本等）见 design.md；可观察行为与验收标准见 spec.md AC-1..15。**

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | 20260528392515 |
| 需求名称 | ohos-agent CLI 工具：连接 AgentExtensionAbility 的长连会话通道 |
| 来源 | `.ignore/agent-cli-proposal.md`（仓内需求草案） |
| 提出人 | TBD（需求方） |
| 目标发行版本 | OpenHarmony-7.0-Release |
| 候选 Profile | none |
| 优先级 | P1 |
| Define 阶段状态 | Approved |

### 原始描述

**原始问题：** 提供一个 ohos-agent CLI 工具，让框架/系统侧经标准 session 方式拉起并长连 APP 类型 AgentExtensionAbility，建立后双向通信（sendData/authorize）经 session 回流调用方，并可干净断连。

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 框架/系统侧调用方 | 缺少通过标准 session 体系拉起并长连 APP 类型 AgentExtensionAbility 的 CLI 工具 | 无法以受控 session 方式对 APP 类型 Agent 做连接/双向数据/断连 |
| Agent 调试与集成 | 无标准通道把 AgentExtensionAbility 的双向数据回调回流给真正调用方 | 对端推送数据无法经 session 订阅送达 |

**期望结果：** 通过 `ohos-agent` CLI 工具建立长驻 connect 会话，调用方即可完成对 APP 类型 AgentExtensionAbility 的 connect / sendData / authorize / disconnect 全流程，且对端主动推送的数据经 session 回流调用方。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 仓内需求草案 | `.ignore/agent-cli-proposal.md` | 原始需求与初步设计思路（含若干需澄清项） |
| 既有范式 | `tools/ohos-aa/` | 同类 CLI 工具注册与子命令 schema 的镜像范式 |
| 既有 session 编排 | cliMgr 既有 session 编排能力 | 可承载长驻 connect 子进程与双向数据回流 |
| 既有 agent 连接 | agent_runtime 既有 Agent 连接能力 | 既有 APP 类型 Agent 连接生命周期与校验链 |

## 需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-25 |
| Owner | yangxenguang-huawei |
| 确认人 | 需求方（2026-08-25 显式 'Y' 批准基线） |
| 确认依据 | 2026-08-25 会话澄清：需求方对 Q1-Q5 汇总结论确认"正确"；proposal 经会话确认 'Y' 批准 |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.0-Release（committed） |
| 版本状态 | committed |
| 基线结论 | 通过 |

### 问题、目标与成功指标

**问题陈述：** 为 AgentExtensionAbility 提供一条经标准 session 体系受控的长连会话 CLI 通道。当前缺少以标准 session 方式拉起并长连 APP 类型 Agent、把双向数据（sendData/authorize）经 session 回流给真正调用方的 CLI 工具。本次新增独立工具 `ohos-agent`（镜像 `tools/ohos-aa` 范式），仅 `connect` 子命令，长驻会话，复用 cliMgr 既有 session 编排能力与 agent_runtime 既有 Agent 连接能力，受 CCM 配置项隔离。受影响用户为框架/系统侧调用方与 Agent 调试集成方。

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 调用方经 CLI 拉起并长连 APP 类型 Agent | connect 成功且收到 connect-done；子进程长驻不退出 | 集成测试 + AC-1 |
| 调用方↔对端双向数据 | sendData/authorize 到达对端 | AC-2/3 |
| 对端→调用方回流 | 对端推送数据经 session 送达调用方 | AC-4 |
| disconnect 走正常流程 | 断连上报 disconnect-done，连接干净释放 | AC-5 |
| 安全边界 | 非 CLI/非系统应用/非 owner 被拒 | AC-6/7/8 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为框架/系统侧调用方，我想要经 CLI 拉起长驻 connect 连接 APP 类型 Agent，以便建立可控长连会话 | P0 |
| US-2 | 作为调用方，我想要经 session 向对端发 sendData/authorize 并接收对端推送，以便双向通信 | P0 |
| US-3 | 作为调用方，我想要经 session disconnect 走正常断连流程，以便干净释放连接 | P0 |
| US-4 | 作为系统，我要确保仅受信 CLI 进程可连、仅 session 建立方可操作，以便守住安全边界 | P0 |
| US-5 | 作为系统，我要用 CCM 配置项隔离本特性，以便按产品启用 | P1 |

> **验收标准详见 spec.md AC-1..15**（已为 ohos-agent CLI 可观察业务级表述：connect 长驻 / 双向数据 / disconnect 正常断连 / 安全边界 / 特性隔离 / 故障与异常恢复）。以下为每条用户故事的业务期望简述，与 spec.md AC 一一对应，不再展开可观察细节。

| US | 业务期望（对应 spec.md AC） |
|----|------------------------------|
| US-1 | connect 成功收到 `connect-done(ok)`、子进程长驻不退出、不主动断连；超时上限（1800s）与并发上限（每调用方 5）约束属边界（AC-1/AC-9/AC-14） |
| US-2 | 调用方经 session 向对端发 sendData/authorize 到达对端；对端主动回调数据经 session 回流调用方（AC-2/AC-3/AC-4） |
| US-3 | 调用方经 session disconnect 走正常断连流程、上报 `disconnect-done(reason=user)`、连接配额释放后可重连（AC-5/AC-15） |
| US-4 | 仅受信 CLI 进程可连、仅 session 建立方可操作、非 CLI 进程走原有校验行为不变（AC-6/AC-7/AC-8） |
| US-5 | CCM 配置项 `const.product.ohos_agent_cli.enabled`=false 时工具不可用、connect 被拒（AC-10/AC-11） |

> **双向通信方向（O-7：对端接口方向注释待修，详见 design.md）**：调用方→对端发送数据/授权（sendData/authorize），对端→调用方回流对端推送数据经 session 送达。接口方向归属与实现细节见 design.md。AC-2/3/4 据此方向定义。

### 范围边界

**包含：**
- 新建独立 `ohos-agent` 工具（镜像 `tools/ohos-aa` 范式），仅 `connect` 子命令，长驻会话模式（不主动断连，经 stdin 下发指令、经 stdout 输出事件）
- agentmgr connect 路径新增 CLI 专用连接分支（非 CLI 进程行为不变）
- CCM 特性隔离开关 `const.product.ohos_agent_cli.enabled`（同时 gate 工具注册与 agentmgr 放行）
- selinux 域配置同 `ohos-aa`
- 复用既有：cliMgr session 编排能力、agent_runtime Agent 连接能力（只读消费）

**不包含：**
- 不改 `ohos-aa` / `aa` 任何代码
- 不改 cliMgr session 编排核心（纯数据驱动消费方，仅新增工具注册配置）
- 不支持 LOW_CODE / ATOMIC_SERVICE 类型 Agent 连接（仅 APP 类型）
- 不新增/不修改公共 SDK API、不改 IPC 序列化格式、不改已发布 hisysevent 事件
- 不新增权限值；connect 子命令复用既有 `ohos.permission.CONNECT_AGENT`（invoker 侧要求，ADR-5 放行后补偿）

| 阶段（如适用） | 范围 | 不做 | 进入条件 |
|----------------|------|------|----------|
| 完整（单期交付，无分期） | ohos-agent 工具新建 + agentmgr CLI 专用连接分支 + CCM 隔离 + selinux/sa_profile | ohos-aa/aa 改动、cliMgr 编排核心改动、LOW_CODE/ATOMIC_SERVICE 连接、公共 SDK 变更 | 基线通过 |

### Agent Scope Guard

> 只约束需求探索和上下文收集阶段可访问的仓库、模块与知识源，不维护实现文件清单。发现需要越界时必须停止并重新确认基线；精确代码文件范围由 execution-plan.md / task.md 持有。

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | ability_runtime（单仓） | 需要访问未列仓库 | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | `tools/ohos-agent/`、`agent_runtime_framework/services/agentmgr/`、cliMgr 既有范式（只读参考）、`sa_profile/`+selinux 配置、agent 连接接口（只读消费） | 需要扩大模块/目录 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | `ohos-aa`/`aa` 代码、cliMgr 编排核心、私有目录、凭据、生成物、无关工作区 | 命中禁止项 | 标记 Blocked，不读取、不修改 |
| 外部网络/私有知识源 | N/A：本澄清基于仓内源码与既有范式，无需外部知识源 | 需要新增权限或未授权来源 | 请求人工授权；未授权前不得使用 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| CLI 工具 | ability_runtime | `tools/ohos-agent/`（新） | — | 新增 | TBD |
| Agent 管理服务 | ability_runtime | `agent_runtime_framework/services/agentmgr/` | Agent 连接生命周期与校验 | 修改（CLI 专用连接分支） | TBD |
| CLI 工具框架 | ability_runtime | cliMgr 既有 session 编排 | session 编排 | 不改（仅注册消费） | TBD |
| 系统能力/安全配置 | ability_runtime | `sa_profile/` + selinux 配置 | SA 注册与域配置 | 修改（新增工具域配置） | TBD |

### API 变更项清单

> 无公共 API 变更，标记 N/A。对外 API = ohos-agent CLI 工具本身（additive 系统 CLI API），规格见 spec.md。

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| N/A | — | — | 不新增/不修改任何 Public/System API；新增 additive 系统 CLI API（见 spec.md） |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否/N/A | 长驻 session 受既有最大超时（1800s）与每调用方并发上限（5）约束，无新指标 | — |
| 安全与权限 | 是 | connect 权限落点、session 建立方校验、CCM 隔离 | design.md / spec.md |
| 兼容性 | 否/N/A | 仅新增工具+放行分支；非 CLI 进程行为不变 | spec.md |
| API/SDK | 否/N/A | 不新增/改任何 Public/System API | — |
| IPC/跨进程 | 是 | 双向数据通道、CLI↔子进程 stdin/stdout 管道 | design.md |
| 构建与部件 | 是 | 新增工具部件、注册配置、selinux/sa_profile | design.md |
| 国际化/无障碍 | 否/N/A | CLI 工具无 UI | — |
| 数据迁移 | 否/N/A | 无持久化数据结构变更 | — |

### 依赖、风险与外部约束

| 依赖/风险 | 类型 | 影响 | 缓解措施/Owner | 状态 |
|-----------|------|------|----------------|------|
| cliMgr 既有 session 编排能力 | 运行依赖 | 长驻 session、stdin/stdout 分帧、双向数据回流均复用既有 | 复用既有 | 已确认 |
| agent_runtime 既有 Agent 连接能力 | 编译/运行 | APP 类型 Agent 连接生命周期与校验链既有（只读消费） | 复用既有 | 已确认 |
| agentmgr connect 校验逻辑 | 运行 | 需新增 CLI 专用连接分支 | 新增分支 | 已确认 |
| 工具拉起沙箱 + selinux 域 | 运行 | 工具经沙箱拉起；selinux 域同 ohos-aa | 配置同 aa | 已确认 |
| CCM 开关未启用产品上误连 | 安全 | 中 | 开关同时 gate 工具注册与 agentmgr 放行 | 已确认 |

> 历史风险（已关闭）：对端接口方向归属 → 已确认（详见 design.md）；目标发行版本未指定 → 已确认 7.0（OpenHarmony-7.0-Release）。

### 依据与上下文引用

| 来源 | 查询/读取内容 | 基线采用的结论 | 可信度 |
|------|---------------|------------------|--------|
| 仓内源码 | cliMgr session 模型可承载长驻 connect 子进程 | 长驻会话 + session stdin 协议 | 高 |
| 仓内源码 | connect 握手交付的对象可消费为对端数据通道 | 双向数据回流路径可行 | 高 |
| 仓内源码 | agentmgr 现有 connect 校验对 CLI 子进程（后台进程）会因前台校验失败 | 需 CLI 专用连接路径 | 高 |
| 仓内源码 | 双向通道方向归属 | 对端→调用方回流方向可行 | 高 |
| 既有范式 | CLI 工具注册与子命令 schema | 镜像 ohos-aa（独立二进制 + 注册配置） | 高 |

> 精确 file:行 证据与实现链路见 design.md。

### 变更控制与下游条件

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和影响范围，重新确认基线 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API | 触发设计审批（本特性预期无） |
| Agent 探索范围变更 | 需要访问未列仓库/模块、禁止项或新知识源 | 停止探索，更新 Agent Scope Guard 后重新确认基线 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认验证计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

- [x] 所有 P0/P1 用户故事有可测试、可度量的 AC
- [x] 范围内外、影响范围、API 变化和不涉及项已确认
- [x] `proposal.target_release` 已确认或明确 TBD
- [x] `manifest.profile` 已确认或明确 none
- [x] Agent Scope Guard 已基线化，未复制 Plan/Task 的实现文件清单
- [x] 基线结论为"通过"，且确认人和确认依据完整
- [x] 条件附录触发时，待澄清项和争议决策已关闭

## 附录：澄清与决策记录（条件启用）

> 本特性复杂度=标准，但出现澄清阻断（Q1-Q5 运行模型/路由/形态/安全落点/连接对象物理落点）与争议决策（ADR-5 安全方案取舍），故启用本附录。本附录仅保留业务决策结论；实现/方案探讨（ADR-5 方案对比、身份注入、防伪、配额账本等）见 design.md。

### 触发与待澄清问题

| 项目 | 内容 | 验证方式/依据 | 状态 |
|------|------|---------------|------|
| 初始假设（运行模型） | cliMgr session 模型可承载长驻 connect 子进程 | 仓内既有 session 能力 | 已验证 |
| 初始假设（握手对象） | connect 握手交付的对象可消费为对端数据通道 | 仓内既有连接能力 | 已验证 |
| 初始假设（前台校验） | agentmgr 现有 connect 校验对 CLI 子进程会因前台校验失败 | 仓内既有校验逻辑 | 已验证 |
| 初始假设（目标版本） | 目标发行版本 7.0 | 业务确认 | 已验证 |
| 复杂度触发 | 标准（单仓 ability_runtime，多子目录，无跨仓/SIG）；但安全落点出现争议决策 | 判断依据 | 已确认 |

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | connect/disconnect/sendData/authorize 的运行模型（长驻会话 / REPL / 单次） | 连接对象为进程内存对象、跨进程无法传递，决定整体架构 | 已澄清：长驻会话 + session stdin 协议 |
| Q-2 | sendData/authorize/disconnect 如何触达长驻 connect session | 新建会话拿不到旧连接对象，决定 session 路由 | 已澄清：connect 为长驻会话；sendData/authorize/disconnect 经 session 下发至该会话；disconnect 走正常流程，session 兜底清理 |
| Q-3 | 工具形态：复用 ohos-aa 新增 subcommand vs 独立工具 | 决定二进制/selinux/选项风格与长驻语义落点 | 已澄清：独立 `ohos-agent`（镜像 ohos-aa 范式，单 connect 子命令） |
| Q-4 | 安全/权限落点 | 原方案"查父进程 HAP"不可行（父为 cliMgr/aimgr） | 已澄清（经三轮修订，见 ADR-5；实现细节见 design.md） |
| Q-5 | 连接对象与双向数据回流物理落点 | 决定对端→调用方数据回流路径 | 已澄清：连接对象在 ohos-agent CLI 子进程侧；对端→调用方数据经 session 回流（实现细节见 design.md） |

### 讨论与方案决策

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-25 | 需求方 + Agent | Q-1 运行模型 | 长驻会话进程 + session stdin 协议（非 REPL、非单次） | 纳入架构 |
| 2026-08-25 | 需求方 + Agent | Q-2 session 路由 | connect 为长驻会话；sendData/authorize 经 session 下发；disconnect 走正常流程，session 兜底清理 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | 工具命名与形态 | 独立 `ohos-agent`（非 ohos-agent-cli；不塞进 ohos-aa），单 connect 子命令 | 纳入范围 |
| 2026-08-25 | 需求方 + Agent | 连接范围 | 仅 APP 类型 Agent（澄清原草案"APP类型低码"自相矛盾：APP 与 LOW_CODE 为不同类型） | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | 设备表述 | 不体现设备（如 PC），改为"CCM 配置项隔离特性" | 纳入范围/AC |
| 2026-08-25 | 需求方 + Agent | disconnect 主路径 | session 下发 disconnect 走正常断连流程；session 兜底清理仅兜底 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | CCM 开关命名 | `const.product.ohos_agent.setting.enabled` | 纳入范围 |
| 2026-09-01 | 需求方 | CCM 开关重命名 | `const.product.ohos_agent.setting.enabled` → `const.product.ohos_agent_cli.enabled`（更简洁命名）；agentmgr 内部对应常量同步 | 修订 AC-10/R-16 |
| 2026-08-25 | 需求方 + Agent | Q-4 安全（初版） | CLI 入口门禁 + session 建立方校验；不查父进程；不新增权限值 | 纳入 AC/安全（后被最终修订取代） |
| 2026-08-27 | 需求方 | Q-4 安全（修订1） | connect 子命令复用既有 `ohos.permission.CONNECT_AGENT`（invoker 侧要求）；不新增权限值 | 修订 R-21/安全模型 |
| 2026-08-28 | 需求方 + Agent | Q-4 安全（最终修订，ADR-5） | 采用 agentmgr CLI 专用连接接口方案，使调用方真实身份端到端透传到对端、真实权限/前台校验完整、非 CLI 进程行为不变；方案对比、身份注入、防伪等实现细节见 design.md（ADR-5/9/10） | 修订 ADR-5/O-13/R-21 |
| 2026-08-25 | 需求方 + Agent | 双向通道与"对端"语义 | 对端=被连接方=AgentExtensionAbility；连接对象在 CLI 子进程侧；对端→调用方数据经 session 回流 | 纳入 AC |
| 2026-08-25 | 需求方 + Agent | Q-5 连接对象落点 | 连接对象在 ohos-agent CLI 子进程侧（agentmgr 追踪） | 纳入架构 |
| 2026-08-25 | 需求方 + Agent | 汇总结论确认 | 用户对汇总结论回复"正确" | 进入基线写入 |

> ADR-5 方案对比、身份注入、防伪、配额账本、连接对象实现等实现决策细节见 design.md（ADR-5/9/10）。

### 检索与影响分析

| 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 |
|------|---------------|----------|--------|------|
| 仓内源码 | cliMgr session 模型 | 可承载长驻 connect 子进程 | 高 | 范围/架构 |
| 仓内源码 | connect 握手对象消费 | 双向数据回流路径可行 | 高 | API/设计 |
| 仓内源码 | agentmgr connect 前台校验 | CLI 子进程前台校验失败，需 CLI 专用连接路径 | 高 | 安全/设计 |
| 仓内源码 | 双向通道方向归属 | 对端→调用方回流方向可行（方向注释待修，O-7） | 高 | 设计 |
| 既有范式 | tools/ohos-aa | CLI 工具注册与子命令 schema 镜像范式 | 高 | 范围 |

> 精确 file:行 证据与实现链路见 design.md。

| 维度 | 澄清结论 | 确认人 | 状态 |
|------|----------|--------|------|
| 子系统与仓 | tools/ohos-agent（新）、agentmgr（CLI 专用连接分支）、cliMgr（仅注册消费）、sa_profile/selinux | 需求方 | 已确认 |
| API 与权限 | 无 Public/System API 变更；复用既有 CONNECT_AGENT + EXEC_CLI_TOOL；不新增权限值 | 需求方 | 已确认（2026-08-28 最终修订） |
| 兼容性与非功能 | 仅新增工具+CLI 专用连接分支；非 CLI 进程行为不变；长驻 session 受最大超时（1800s）与每调用方并发上限（5）约束 | 需求方 | 已确认 |
| 依赖与风险 | cliMgr session 模型、agent_runtime 连接能力、agentmgr 校验逻辑、沙箱+selinux；CCM 误连风险由开关同时 gate 缓解 | 需求方 | 已确认 |

### 澄清结论

- [x] 待澄清问题均已关闭或明确 N/A
- [x] 范围、API、兼容性、非功能需求、依赖和风险已形成基线结论
- [x] complex/critical 已记录方案取舍和需求方/Owner/SIG 确认证据
- [x] P0/P1 AC 覆盖正常、异常和边界路径

**结论：** 通过

> 基线内容形成后，将 `phase_status` 置为 `awaiting_approval` 并停止。只有需求方/Owner/SIG 明确批准 Define，且 `approval` 中的 status、approver、evidence、approved_at、baseline_digest 均已填写并与当前 `需求基线` 一致后，才能将 `phase_status` 置为 `approved` 并进入 Specify。回答澄清问题、表示没有补充、`ReadyForReview` 或 AI 自评均不等于批准。

> 基线候选内容固定后，运行 `ohos-sdd approval-digest <change>`，将输出的 64 位摘要填写到 `approval.baseline_digest`。该命令与 validator 共用同一摘要算法。需求基线发生任何实质变更后，必须将 `phase_status` 改回 `awaiting_approval`、重新生成摘要并取得批准。原 approval 字段保留作审计记录，当前是否生效以 `phase_status` 和 digest 校验为准。

> `需求输入` 的"Define 阶段状态"是面向人工阅读的镜像字段，必须与 frontmatter 的 `phase_status` 保持一致（状态名称可使用 `Clarifying`、`AwaitingApproval`、`Approved`、`ChangesRequested`、`Blocked` 的展示写法）。
