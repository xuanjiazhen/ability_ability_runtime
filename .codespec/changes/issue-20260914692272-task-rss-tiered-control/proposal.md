---
phase_status: awaiting_approval
approval:
  status: awaiting_approval
  approver: "yangzk"
  evidence: "AC 合并与连续编号调整后的新基线，待重新批准 Define（2026-11-17 会话记录）"
  approved_at: ""
  baseline_digest: "e85f165002889962474594c5b095fee91c0bd95b11140956f425dfcbdaee5a52"
target_release:
  id: OpenHarmony-7.1-Release
  status: proposed
---

# 需求文档：任务进程 RSS 分级调度决策与代理调用（含防抖合并）

> 主文档只保存需求输入和已确认基线。本需求复杂度为“标准”，逐项澄清结论已合并入需求基线；当前状态为 Approved。

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | 20260914692272 |
| 需求名称 | 任务进程 RSS 分级调度决策与代理调用（含防抖合并） |
| 来源 | 需求前置分析（本仓 `20260914692272/`）、依赖需求 20260914692273 及 2026-11-16 需求方补充澄清；关联 Feature FEAT-AI-TASK-MGR-001（RR 30096095） |
| 提出人 | yangzk |
| 目标发行版本 | OpenHarmony-7.1-Release |
| 候选 Profile | none（ability_ability_runtime 未命中现有 arkweb/arkui/arkgraphic/arkdata profile） |
| 优先级 | P1 |
| Define 阶段状态 | AwaitingApproval |

### 原始描述

**原始问题：** 以任务为整体进行任务进程调度。依赖需求已提供任务优先级系统判定、多任务并发、进程多任务归属及按最高优先级重算能力；当前重算结果只带纳管进程 pid 和变化前后档位，缺少任务创建者、纳管对象 uid、完整受管任务与多 AttributionTool 上下文，也缺少资源管控代理和防抖合并，无法完整核对调度决策及其生命周期。本期仅 `CLI` 和新增 `VIRTUAL_SCREEN_ABILITY` 产生调度管控事件；`ABILITY` 不产生调度管控事件，`A2A/INSIGHT` 不纳入受管集合，新类型的实际纳管通道由需求方后续补充。

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 系统开发者（任务调度/资源调度） | 进程重算事件的归因上下文不完整，多任务、多创建者、多工具场景无法准确追溯 | 调度决策难以验证，结束及死亡路径难以核对 |
| 系统（任务进程调度） | 频繁状态变化可能产生重复或过期代理事件，缺少统一的立即通知与合并通知规则 | 代理事件时序和最终状态不一致 |

**期望结果：** 每次进程优先级重算均采集完整、可核对的归因上下文；仅在新建管控、升降档或管控结束时产生符合动作语义的事件；新建/升档立即通知，降档及非死亡结束按固定 1 秒窗口合并，纳管进程死亡立即结束并清理过期事件。ResourceControlProxy 本期仅提供空实现和日志，以调度决策、事件载荷、动作及时序正确性作为验收对象。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 当前需求前置分析 | `20260914692272/proposal.md`、`spec.md`、`design.md`、`execution-plan.md` | 提供初始范围；本基线按需求方补充移除真实调控效果、动态策略、资源配额和性能功耗验收 |
| 已合入依赖需求 | `20260914692273/`；本仓提交 `3541c8ee81`，合并提交 `d44973f6a9` | 任务优先级判定、多任务归属、effectiveLevel 重算及 ADR-9 初步防抖分析已合入当前基线 |
| 当前源码 | `services/appmgr/src/task_mgr/process_schedule.cpp:41-65,76-101,173-188` | 当前仅有归属重算及 `pid + oldLevel/newLevel` 空实现日志，尚无完整事件载荷、ResourceControlProxy 和合并窗口 |
| 当前源码 | `services/appmgr/include/task_mgr/task_record.h:41-53`、`attribution_record.h:31-49` | 任务记录已有创建者 pid/uid；归因记录支持 INSIGHT/CLI/A2A/ABILITY 多工具类型 |
| 领域知识库 | `abilitykit_sdd/specifications/ability/1_modules/task_flow.md`、`2_capabilities/task_flow_mechanism.md`、`3_scenarios/task_flow/` | 已有任务生命周期、归因、容量和死亡清理规格；未查到 effectiveLevel、ResourceControlProxy 或调度防抖的既有规格 |

## 需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-11-16 |
| Owner | yangzk |
| 确认人 | yangzk |
| 确认依据 | AC 合并与连续编号调整后的新基线，待重新批准 Define（2026-11-17 会话记录） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.1-Release（proposed） |
| 版本状态 | proposed |
| 基线结论 | 待批准 |

### 问题、目标与成功指标

**问题陈述：** 已合入的 process_schedule 能按最高任务档位计算进程 effectiveLevel，但代理事件缺少完整归因快照和统一动作/时序语义。需要在不改变任务生命周期及前端纳管约束的前提下，补齐可验证的调度决策输出，并以空实现隔离真实 RSS 调控效果。

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 重算上下文完整 | 每次重算采集的创建者、纳管对象、受管任务、AttributionTool、变化前后 effectiveLevel、动作和触发原因与任务事实一致，上下文矩阵用例通过率 100% | 单元测试 + 日志断言 |
| 多归属信息无损 | 多任务、多创建者、多工具受管及最后任务结束/进程死亡时均保留所需快照，无单创建者或单工具覆盖，矩阵用例通过率 100% | 多任务/多工具/结束快照单元测试 |
| 事件动作与时序正确 | 新建/升档立即 START，降档 START 与结束 END 按固定 1 秒窗口合并，纳管进程死亡立即 END；过期事件不输出，时序用例通过率 100% | 可注入时钟的单元测试 + 事件序列断言 |
| 上下文变化不错误触发 | 仅受管任务或受管工具集合变化且 effectiveLevel 与管控生命周期不变时，只更新快照和上下文日志，不产生代理事件；ABILITY、A2A、INSIGHT 变化不影响受管状态 | 无事件断言 + 后续事件最新快照断言 |
| 空实现边界清晰 | ResourceControlProxy 不调用真实 RSS 调控、生效查询或撤销重试，代理调用与日志序列符合预期；未纳管进程不产生本需求代理事件 | mock 零调用断言 + 相关单元测试/回归测试 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统调度开发者，我想要每次进程优先级重算都形成完整归因快照，以便准确核对纳管进程在多任务、多创建者、多工具场景下的调度决策 | P0 |
| US-2 | 作为系统调度开发者，我想要代理事件按明确的动作、立即通知和固定窗口合并规则输出，以便任务变化、结束和进程死亡时事件序列与最终管控状态一致 | P0 |
| US-3 | 作为现有应用及任务调用方，我想要未纳管进程和既有前端纳管约束不受本需求影响，以便保持现有行为兼容 | P1 |

**受管工具范围与合并规则：** 本期仅 `CLI` 与 `VIRTUAL_SCREEN_ABILITY` 产生调度管控事件，`ABILITY` 不产生调度管控事件，`A2A/INSIGHT` 不参与 effectiveLevel、代理事件或防抖。先在受管任务集合内按最高档计算 effectiveLevel，再以 `VIRTUAL_SCREEN_ABILITY > CLI` 作为同档时的主类型/策略优先级；其他工具不影响受管结果。新类型的实际纳管通道由需求方后续补充。

**事件上下文基线：**

| 类别 | 每次重算必须采集的内容 |
|------|------------------------|
| 纳管对象 | 纳管进程 pid/uid；死亡或结束路径使用变化前已保存的身份快照，不依赖死亡后的在线查询 |
| 受管任务 | 仅包含 CLI 与 VIRTUAL_SCREEN_ABILITY 的完整任务上下文集合，每项包含 taskId、任务创建者 pid/uid、该任务当前受管工具集合；不得用单一创建者或单一工具覆盖多归属信息；ABILITY、A2A、INSIGHT 不纳入 |
| 档位 | 先在受管任务集合内取最高 effectiveLevel，再以 VIRTUAL_SCREEN_ABILITY > CLI 作为同档主类型/策略优先级；同时记录变化前 effectiveLevel 与本次重算后的 effectiveLevel |
| 事件语义 | START/END 动作及触发原因；触发原因至少能区分新建归属、主动注销、任务结束、任务超时、创建者死亡、纳管进程死亡、前后台/负载导致的档位变化和合并窗口到期 |
| 快照时态 | START 使用触发上下文及重算后的最新受管快照；END 保留变化前完整受管上下文，即使重算后受管集合为空也不得丢失创建者和受管工具集合 |
| uid 获取与失败 | 登记时直接按 pid 查询 uid 并缓存进 AttributionRecord；相关重算及结束/死亡前重试。持续失败仅告警不输出代理事件，不拒绝既有纳管；重试成功时保留原始触发原因并恢复完整事件，结束/死亡时仍失败则仅清理，不伪造 START/END |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-5.1 | WHEN 纳管进程发生新建归属、effectiveLevel 重算、唯一受管 Task 上下文变化、最后移除或死亡 THEN 实际代理事件仅包含当前唯一受管 Task 的 taskId、创建者 pid/uid、纳管 pid/uid、受管工具、变化前后 effectiveLevel、动作和触发原因；同一进程同一时刻仅允许一个受管 Task，同一 Task 对同一进程仅允许一次纳管，CLI 与 VIRTUAL_SCREEN_ABILITY 不叠加，ABILITY、A2A、INSIGHT 不产生代理事件；不变档上下文变化不增加事件，下一次事件使用最新唯一上下文 | 正常/边界 | US-1 |
| AC-5.2 | WHEN 进程由未纳管变为纳管或 effectiveLevel 上升 THEN 立即输出包含最新状态的 START 事件；WHEN 纳管进程死亡 THEN 立即输出 END 事件，并使该 pid 尚未输出的合并事件失效 | 正常/异常 | US-2 |
| AC-5.3 | WHEN effectiveLevel 下降，或主动注销、任务正常结束、任务超时、创建者死亡导致降档或最终结束 THEN 使用固定 1 秒合并窗口且不读取系统参数；窗口内再次变化时仅保留有效触发事件和到期时的最新唯一状态，无变化时不额外输出事件 | 正常/边界 | US-2 |
| AC-5.4 | WHEN 最后一个受管 Task 或其唯一归因被移除 THEN 输出 END，最终 END 只结束当前生命周期，旧生命周期无后续 START/END；之后同一 pid 重新纳管时开启新生命周期并允许立即 START；死亡 pid 仅输出一个最终 END，其他 pid 不受影响；不新增独立配额、超限拒绝或错误码，既有前端任务与归因上限行为不变 | 正常/异常/边界 | US-2 |
| AC-5.5 | WHEN ResourceControlProxy 收到符合上述规则的 START/END 事件 THEN 本期空实现仅输出包含完整事件内容的可核对日志，不发起真实冻结、调度分组、QoS、speed、生效查询、恢复或撤销重试调用 | 正常 | US-1 |
| AC-5.6 | WHEN 纳管登记或后续重算时按 pid 查询 uid 失败 THEN 仅输出告警并抑制字段不完整的代理事件，不拒绝既有纳管；WHEN 后续重试成功 THEN 按原始触发原因恢复完整事件；WHEN 任务结束或纳管进程死亡时 uid 仍持续获取失败 THEN 仅清理并告警，不伪造 START/END 且后续无该旧生命周期事件 | 异常 | US-1 |

### 范围边界

**包含：**

- 新增 `VIRTUAL_SCREEN_ABILITY` 工具类型，仅将 `CLI` 与该新类型纳入调度管控；`ABILITY` 不产生调度管控事件，`A2A/INSIGHT` 不参与 effectiveLevel、代理事件或防抖。新类型实际纳管通道由需求方后续补充。
- 消费已合入依赖需求提供的任务归属及 effectiveLevel 重算事实，在每次重算时形成完整归因快照。
- 按进程最终管控状态输出 START/END 决策事件，携带变化前后 effectiveLevel、完整受管上下文和触发原因。
- ResourceControlProxy 空实现及关键路径日志；后续真实调度策略在该代理内固化，本需求不引入动态策略读取。
- 固定 1 秒合并窗口：新建/升档立即，降档及非死亡结束合并，纳管进程死亡立即；窗口到期按最新状态重算，过期事件失效。
- 多任务、多创建者、多 AttributionTool、最后任务结束、任务超时、创建者死亡、纳管进程死亡，以及 uid 查询失败、重试成功和持续失败的单元测试与必要集成验证。
- 内部跨进程通知通道的需求识别；具体内部接口及签名仅在后续 design.md 中设计，不在本 proposal 和 spec.md 中展开。

**不包含：**

- 性能功耗指标、功耗对比、调度基线收益及端到端性能功耗验收；本需求只验证调度功能正确性。
- 真实冻结、调度分组、QoS、speed 等 RSS 下发、生效查询、实际恢复效果及撤销失败重试。
- 具体优先级到真实管控策略组合的取值落地，以及通过系统参数或其他动态配置加载策略/防抖窗口。
- 新增或修改下游管控资源数量上限；既有前端活动任务数、单任务归因数等纳管限制保持原状，由对应前端纳管需求承担。
- `VIRTUAL_SCREEN_ABILITY` 的实际纳管通道实现（由需求方后续补充）；`ABILITY`、`A2A/INSIGHT` 的调度管控。
- 任务生命周期、任务优先级系统判定、多任务归属合并算法及前端纳管通道的重复实现。
- Public/System API、d.ts、Kit 或公开错误码变更；具体 InnerAPI 名称、参数和签名不在本 proposal/spec 中体现。
- 统一 DFX 观测面建设；本需求仅保留验证调度决策所需的关键路径日志。

| 阶段（如适用） | 范围 | 不做 | 进入条件 |
|----------------|------|------|----------|
| 完整 | 上述全部包含项 | 上述全部排除项 | proposal 批准 → spec → design → plan → 实现与功能验证 |

### 自有生态设备差异规格

| 设备/差异项 | 是否存在差异 | 差异说明 |
|-------------|--------------|----------|
| 手机（含折叠屏） | 否 | 调度决策与代理空实现为系统机制，无设备形态差异 |
| pc（平板） | 否 | 同上 |
| pc（桌面） | 否 | 同上 |
| wearable | 否 | 同上 |
| tv | 否 | 同上 |
| car | 否 | 同上 |
| 其他 | 否 | 同上 |
| 功能差异（非品类划分） | 否 | 不按设备形态读取不同策略或防抖配置；真实 RSS 效果不在本需求验收范围 |

### Agent Scope Guard

> 只约束需求探索和上下文收集阶段可访问的仓库、模块与知识源，不维护实现文件清单。发现需要越界时必须停止并重新确认基线；精确代码文件范围由 execution-plan.md / task.md 持有。

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | openharmony/ability_ability_runtime（本仓）；abilitykit_public/abilitykit_sdd（同级只读知识库） | 需要访问未列仓库，包括真实 RSS 实现仓或 SDK 仓 | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | 本仓 `services/appmgr/`、`interfaces/inner_api/`（仅任务进程调度内部通道）、`test/unittest/`、`test/integration/`；`20260914692272/` 与 `20260914692273/` 仅作只读前置分析/依赖证据；`.codespec/changes/issue-20260914692272-task-rss-tiered-control/`（本 change 交付件）；本仓 AGENTS.md/CLAUDE.md | 需要扩大模块/目录，或需修改两个原需求目录 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | 禁止访问凭据与私有配置；禁止修改无关工作区改动和构建生成物、Public/System API 与 d.ts、既有 PermissionVerification 权限规则、`20260914692272/` 与 `20260914692273/` 内任何文件、真实 RSS 策略配置及性能功耗验收资产 | 命中禁止项 | 标记 Blocked，不读取、不修改，回报 Owner |
| 外部网络/私有知识源 | 仅允许需求方已授权的 `https://gitcode.com/abilitykit_public/abilitykit_sdd.git` 知识库；其余外部网络/私有知识源 N/A | 需要新增权限或未授权来源 | 请求人工授权；未授权前不得使用 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | ability_ability_runtime | `services/appmgr/`（任务进程调度与资源管控代理边界） | 进程生命周期、任务归属及 effectiveLevel 重算 | 修改：完整快照、事件动作/时序、固定窗口合并与空实现日志 | yangzk |
| ability | ability_ability_runtime | `interfaces/inner_api/`（仅任务进程调度内部通道） | 内部组件接口与 IPC 边界 | 修改/新增：具体内部接口在 design.md 定义，不改变 Public/System API | yangzk |
| ability | ability_ability_runtime | `test/unittest/`、`test/integration/`（任务进程调度相关测试） | 功能与回归验证 | 新增/修改：上下文、动作、时序、取消、结束快照和空实现测试 | yangzk |
| ability | abilitykit_sdd | `specifications/ability/`、`summarys/`（只读） | 领域概念和既有任务流规格 | 只读参考；本需求不修改知识库文件 | yangzk |

### API 变更项清单

N/A：无 Public/System API 新增、修改或废弃，无 d.ts/Kit/公开错误码变更。内部跨进程接口仅在 design.md 中具体设计，不在本 proposal 和 spec.md 中列出名称或签名。

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | N/A | 本需求不承载性能功耗指标，仅验证调度决策、载荷、动作和时序正确性；端到端性能功耗另行整体验收 | N/A |
| 安全与权限 | N/A | 需求方明确本维度不展开；不新增鉴权点、权限项或资源数量限制，既有前端身份与纳管边界不变 | N/A |
| 兼容性 | 是 | 未纳管进程不产生本需求代理事件；不改变既有前端任务/归因上限及 Public/System API | spec.md |
| API/SDK | N/A | 无 Public/System API 变化；内部接口只在 design.md 设计 | N/A |
| IPC/跨进程 | 是 | 涉及任务进程调度的内部跨进程通知通道；具体接口在 design.md 定义，本期代理空实现不验收真实 RSS 调控效果 | design.md |
| 构建与部件 | 是 | appmgr 既有部件内调整调度/代理实现并补充测试，不新增部件 | design.md |
| 国际化/无障碍 | N/A | 无 UI、UI 文案或无障碍交互 | N/A |
| 数据迁移 | N/A | 受管快照和合并状态为内存态，不持久化，无存量数据迁移 | N/A |

### 依赖、风险与外部约束

| 依赖/风险 | 类型 | 影响 | 缓解措施/Owner | 状态 |
|-----------|------|------|----------------|------|
| 依赖需求 20260914692273（优先级判定、多任务归属、effectiveLevel 重算、ADR-9） | 技术 | 高（本需求输入事实） | 使用当前已合入实现作为基线，不重复实现归属合并；本仓提交 `3541c8ee81` / 合并提交 `d44973f6a9` / yangzk | 已确认（已合入） |
| 多创建者、多任务、多工具及 END 空集合导致上下文丢失 | 技术 | 高 | 保存完整受管快照和变化前快照；覆盖受管、最后结束及死亡矩阵 / yangzk | 已确认（实现期验证） |
| uid 查询失败、重试成功和持续失败导致残缺事件或误管控 | 技术 | 高 | 登记时直接按 pid 查询并缓存；相关重算及结束/死亡前重试，持续失败仅告警不输出并清理，不拒绝纳管；重试保留原始触发原因 / yangzk | 已确认（实现期验证） |
| 死亡后在线查询纳管对象 uid 可能失败 | 技术 | 中 | 在归因存活期保存纳管 pid/uid，死亡事件消费变化前快照；死亡用例覆盖 / yangzk | 已确认（实现期验证） |
| 合并窗口内状态反复变化产生过期事件 | 技术 | 高 | 到期按最新状态重算；新建/升档/死亡使旧合并事件失效；可注入时钟验证 / yangzk | 已确认（实现期验证） |
| 空实现被误认为真实 RSS 效果已验证 | 验证 | 中 | 验收只断言决策、载荷、日志与时序；mock 断言无真实调控调用，真实效果明确排除 / yangzk | 已确认 |
| 领域知识库未覆盖 effectiveLevel、ResourceControlProxy 和调度防抖 | 知识 | 中 | 以本 proposal 建立新增需求基线；若后续修改 abilitykit_sdd specifications，按该知识库独立提交流程处理，不在本仓代为修改 / yangzk | 已确认 |

### 依据与上下文引用

| 来源 | 查询/读取内容 | 基线采用的结论 | 可信度 | 证据路径/链接 |
|------|---------------|----------------|--------|---------------|
| 当前源码 | process_schedule 归属重算、重复归因提前返回、effectiveLevel 不变不传播及空实现日志 | 新需求需补齐完整快照、代理与合并；受管/工具单独变化不得误发事件 | 高 | `services/appmgr/src/task_mgr/process_schedule.cpp:41-65,76-101,134-188` |
| 当前源码 | TaskRecord 创建者字段、AttributionRecord 多工具字段、任务结束/超时/死亡调用链 | 创建者信息来自持久于任务存活期的记录；结束/死亡需使用变化前快照 | 高 | `services/appmgr/include/task_mgr/task_record.h:41-53`；`attribution_record.h:31-49`；`services/appmgr/src/task_mgr/task_manager_service.cpp:197-230,233-312,405-445,625-685` |
| uid 查询先例 | 只读复用 `/proc/<pid>/status` 的 `Uid:` 行解析方式；不修改 appdfr 或新增依赖 | 登记时直接按 pid 查询并缓存；失败返回 -1 并抑制不完整事件，后续重试 | 高 | `services/appdfr/src/appfreeze_util.cpp:133-157` |
| 依赖需求设计 | ADR-9 注册/死亡/升档立即、降档与主动注销合并 1 秒、到期按最新状态重算 | 继承其时序原则；按本需求澄清固定窗口，动作以进程最终状态为准 | 高 | `20260914692273/design.md:133,334-359` |
| 仓内 Agent 指南 | appmgr 高风险边界、知识库使用、测试与提交流程 | 仅修改允许模块；代码阶段须同步测试并按规定构建验证 | 高 | `AGENTS.md`、`CLAUDE.md` |
| 领域知识库 | TaskManager 模块、TaskRecord/AttributionRecord、任务结束/归因/死亡规格及既有容量限制 | 复用生命周期与归因事实；不取消既有前端 10 个活动任务、1000 条归因限制；新增调度代理与防抖属于规格空白 | 中（相关 capability/scenario review_status=pending） | `abilitykit_sdd/specifications/ability/1_modules/task_flow.md:17-55`；`2_capabilities/task_flow_mechanism.md:38-82`；`3_scenarios/task_flow/` |
| 需求方澄清 | 空实现验收、固定策略/窗口、完整受管上下文、工具变化不发事件、动作优先级、N/A 维度、文档与分支范围 | 作为本基线的直接需求输入；澄清不视为 Define 批准 | 高 | 2026-11-16 本会话逐项问答记录 |

### 变更控制与下游条件

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事、真实 RSS 效果、动态策略、资源配额或新仓/模块 | 重新评估复杂度和影响范围，重新确认基线 |
| AC 变更 | 修改事件载荷、动作、立即/合并时序、快照时态或可观察兼容性 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API | 停止当前范围，重新审批基线并触发设计审批 |
| 内部接口设计变化 | 内部跨进程接口影响事件可观察行为 | 回 design 评审；影响 AC 时同步回 spec/proposal |
| Agent 探索范围变更 | 需要访问未列仓库/模块、禁止项或新知识源 | 停止探索，更新 Agent Scope Guard 后重新确认基线 |
| 非功能/功能时序变更 | 固定 1 秒窗口、兼容性口径或可靠性约束变化 | 重新确认验证计划和受影响 AC |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

- [x] 所有 P0/P1 用户故事有可测试、可度量的 AC
- [x] 范围内外、影响范围、API 变化和不涉及项已逐项澄清
- [x] `proposal.target_release` 已确认（OpenHarmony-7.1-Release，proposed）
- [x] `manifest.profile` 已确认为 none
- [x] Agent Scope Guard 已基线化，未复制 Plan/Task 的实现文件清单
- [x] 基线结论为“通过”，且确认人和确认依据完整（yangzk，2026-11-16 Define 重新批准）
- [x] 条件附录未触发：标准复杂度，逐项澄清已关闭，无跨 SIG/多仓协调或未决争议
