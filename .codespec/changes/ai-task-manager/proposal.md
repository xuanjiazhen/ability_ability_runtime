---
target_release: "7.1"
change_type: new-feature
issue: ""
author: "yangzk"
date: "2026-09-14"
status: committed
---

# 需求文档：AI 语音助手任务管理机制

> 一份文档，从原始需求到基线结论。按阶段追加内容，不拆成多份独立文件。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-AI-TASK-MGR-001（占位，待 issue 挂接） |
| 需求名称 | AI 语音助手任务管理机制 |
| 来源 | 需求方口头提出（AI 语音助手团队），2026-08-21 澄清 |
| 提出人 | yangzk（ability_runtime） |
| 目标发行版本 | 7.1（已确认） |
| 候选 Profile | none（ability_runtime 系统服务，不属于 arkweb/arkui/arkgraphic/arkdata 任一 UI 域） |
| 优先级 | P1 |
| 状态 | Clarifying |

### 原始描述

**原始问题：** AI 语音助手接近一半场景为锁屏下唤醒，锁屏下任务无法执行——原因：① EL3~EL5 加密等级数据在锁屏下无访问权限（安全/存储域问题）；② 锁屏下执行影响功耗（受系统冻结/挂起管控限制）。另有：长时任务切后台后被冻结无法执行；后台任务经虚拟屏执行时被当作前台应用调度，抢占用户正在使用的前台应用资源（帧率、启动等）。主要由于安全、RSS 管控（冻结、内存CPU优先级等）的影响。范围：安全管控方面的问题暂不讨论，优先分析 RSS 管控的问题；EL3~EL5 数据访问属安全/存储域，登记为外部依赖。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| AI 语音助手用户 | **接近一半场景为锁屏下唤醒，锁屏下任务无法执行**——原因：① EL3~EL5 加密等级数据在锁屏下无访问权限（安全/存储域，外部依赖）；② 锁屏下执行影响功耗（受冻结/挂起管控限制）；另有长时任务（如批量处理、持续播报）切后台后进程被 RSS 冻结，任务无法完成 | 核心语音场景不可用（锁屏场景占比近半），用户感知明显 |
| AI 语音助手用户（前台体验受扰） | 后台任务经虚拟屏执行时被当作前台应用调度，抢占用户正在使用的前台应用的资源（帧率、启动等） | 后台任务与前台体验相互干扰：前台应用卡顿/启动变慢 |
| AI 语音助手开发者 | 无任务维度的资源管控手段，只能依赖应用前台状态，无法表达"这个任务需要后台持续运行"的诉求 | 功能受限，需要绕过系统管控（不可接受） |
| 系统 | 应用长驻后台的诉求与 RSS 冻结/内存管控相冲突，缺少可信的中间机制 | 管控与体验失衡 |

**期望结果：** 提供一套任务管理机制给 AI 语音助手使用：AI 助手创建任务并指定优先级、类型等参数，任务管理模块将任务执行涉及的进程纳入任务并按任务优先级/类型进行 RSS 管控（冻结豁免、解冻、资源优先级等），任务结束自动降级恢复。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 需求方描述 | 本次需求对话（2026-08-21） | 原始需求要点见下（8 条）；反馈：模块并入 AppMS、目标版本 7.1 |
| RSS 进程管理规范 | `foundation/resourceschedule/resource_schedule_service/Resource_Schedule_Service_Process_Management_Specifications_ZH.md` | RSS 新增插件需评审进程内存基线 → 本需求倾向复用现有 resType 接口而非新增插件 |

**需求方原始需求（原文要点）：**

1. AI 语音助手通过意图工具、CLI 工具、A2A 调用工具能力，启动过程携带 taskId 等标记信息，任务管理模块将进程纳入任务。
2. 任务管理模块提供 startTask、finishTask、abortTask 三个 System API。
3. 提供基础 DFX 手段。
4. 任务管理模块根据任务优先级、类型等参数制定 RSS 参数，对 task 下进程进行管理，具体策略放在任务管理模块的资源调度子模块中。使用 RSS 现有接口或新增进程管理接口。
5. 可能存在多任务并发的情况。
6. 一个进程可能被多个任务使用，按优先级更高的处理。
7. 一个任务结束后，如果其中的进程未退出，需要降级回原来的 RSS 调度状态。
8. 任务管理模块放在 foundation/ability/ability_runtime，相关 RSS 仓在 foundation/resourceschedule/resource_schedule_service。

### 初始范围

**可能包含：**
- 任务模型与生命周期（**startTask/finishTask/abortTask 三个 System API**，无任务状态机；超时时间参数与系统自动结束；可选 payload 描述）
- 纳管通道：三链路系统接口参数对象扩展携带 taskId（task.md 4.2~4.4）——CLI `ExecOptions.taskSessionId`（ExecCmdOptions 同步）、意图 `ExecuteParam.taskSessionId`（系统保留字段，转写为 Want 兜底 `ohos.param.taskSessionId`）、A2A `A2ACallContext.taskSessionId`（带外上下文）；系统侧 owner 归属校验+存活态校验+通道隔离
- RSS 资源调度子模块：按优先级分级（冻结豁免、解冻、状态查询、结束降级），复用 RSS 现有接口，策略可配，预留扩展点
- 基础 DFX（hisysevent、hidumper/dump、日志）
- 模块落在 `services/appmgr`（AppManagerService，SA 501，foundation 进程）：任务管理模块 + 通用"App 进程调度管理"能力（抽象化，详见方案探索 A-2）

**明确不包含：**
- 安全管控问题（权限校验策略细则、跨应用访问控制等），接口鉴权仅做基础身份校验
- 新增 RSS 插件/新增 RSS resType（一期复用现有接口；二期如需细化提权，另行与 RSS 仓对齐评审）
- Public API（对三方应用开放；任务接口仅以 System API 面向系统应用）
- 任务持久化/跨设备同步/任务恢复（进程退出即清理）
- 定时任务调度本身（定时触发归调用方，本模块只管任务执行期管控）

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| RSS 现有 `RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT`（505）、`SYNC_RES_TYPE_THAW_ONE_APP`（517）等接口可直接承载任务级冻结豁免/解冻，无需新增 resType | 技术 | RSS 源码勘察 + RSS owner 确认 | 已验证（res_type.h、res_sched_service.cpp:175 白名单命中；payload 语义待 RSS owner 确认） |
| 调用方（AI 助手、系统 UI）均为系统应用，任务接口基础身份校验（SA/系统应用）足够，不需要逐权限校验 | 业务 | 需求方确认 | 待确认 |
| 锁屏场景下冻结豁免覆盖目标冻结类型（锁屏冻结 = RSS suspend 冻结 + 可能叠加 standby 待机冻结，豁免接口覆盖范围待与 RSS owner 对齐） | 技术 | 与 RSS owner 确认 + 真机场景验证 | 待确认 |
| 锁屏场景任务可执行的前提是任务所需数据在锁屏下可访问（EL3~EL5 加密等级数据锁屏不可访问的限制由安全/存储域解决，非本模块能力） | 业务/技术 | 需求方与安全/存储域确认 | 待确认 |
| 目标发行版本 7.1 | 业务 | 需求方确认 | 已确认 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单仓主体模块 + 三链路系统接口可选字段扩展（CLI/意图/A2A，跨 Kit 协同）；含并发、多链路纳管；无新增 SA；新增三个 System API |
| 涉及仓数量 | 2+（跨 Kit 协同） | ability_ability_runtime（主，含 AppMS 新模块 + AMS 收敛点解析）；resource_schedule_service（仅复用现有接口，不改动，需 owner 评审确认 payload 语义）；CLI/意图/A2A 三链路接口扩展需对应 Kit/SDK 团队协同（见影响范围与依赖表） |
| 是否涉及 Public/System API | 是（System API） | 新增 startTask/finishTask/abortTask 三个 System API（@systemapi 标注、NAPI 绑定 + d.ts，面向系统应用）；无 Public API |
| 是否涉及安全/性能关键路径 | 是 | 系统服务（foundation 进程）、AMS 启动热路径解析（Want 兜底字段）与三链路系统侧校验；安全管控本身不在本需求范围，但接口需基础鉴权 |
| 是否跨 SIG | 是 | ability（ability_runtime）+ resourceschedule（接口语义确认） |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定

---

## 二、澄清记录

> 澄清与需求变更记录如下（历史轮次详见 git 记录与 review-analysis.md）。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 任务管理模块以什么形态落地？（独立 SA / 内嵌 AMS / 内嵌 agentmgr / 并入 AppMS） | 决定 IPC 形态、进程归属、与启动链路的关系 | 已澄清：**并入 AppMS（SA 501），抽象为通用"App 进程调度管理"能力 + 任务语义层**（A2A 任务、意图、CLI 均不经过 agentmgr，但所有应用进程都经过 AppMS，AppMS 收口最佳） |
| Q-2 | RSS 管控如何集成？ | 决定一期落地方式与跨仓工作量 | 已澄清：先复用 RSS 现有接口（DYNAMICALLY_SET_SUSPEND_EXEMPT / THAW_ONE_APP / 冻结状态查询）+ 预留扩展点 |
| Q-3 | taskId 标记如何传递、进程如何纳入任务？ | 决定调用方负担与覆盖链路 | 已澄清：**三链路系统接口参数对象扩展**（task.md 4.2~4.4）——CLI `ExecOptions.taskSessionId`、意图 `ExecuteParam.taskSessionId`、A2A `A2ACallContext.taskSessionId`，可选字段、函数签名不变；意图/A2A 链路经 Want 兜底字段 `ohos.param.taskSessionId` 到 AMS；系统侧校验 owner+存活态后自动纳管 |
| Q-4 | 任务管理接口面向谁？ | 决定 API 层级与权限设计 | 已澄清：**startTask/finishTask/abortTask 以 System API 开放**（@systemapi，NAPI 绑定 + d.ts，面向系统应用）；三链路纳管经系统接口可选字段扩展承载（Q-3），系统侧内部通道为内部实现 |
| Q-5 | RSS 资源调度子模块管控粒度？ | 决定策略复杂度与一期范围 | 已澄清：按优先级分级策略（高优先级豁免冻结+解冻），CPU 提权预留扩展点 |
| Q-6 | SDD 流程产物与节奏？ | 决定工作方式 | 已澄清：先出设计文档评审，通过后再写代码 |
| Q-7 | Want 透传 taskId 是否可被仿冒？如何防？ | 决定纳管链路的信任模型 | 已澄清：同意 **B-1+B-2**（不可猜测随机 taskId + caller 身份绑定校验）；B-3 token 化可选增强 |
| Q-8 | 可否借用 QosManager 的 sessionQos/QoS 机制管控？ | 决定资源调度子模块的管控手段构成 | 已澄清：采纳借用评估结论（一期冻结豁免主线，QoS 提权二期扩展）；"sessionQos" 具体机制设计阶段再探索 |
| Q-9 | RSS 管控的维度范围？ | 决定资源调度子模块策略覆盖面 | 已澄清：**不仅限冻结**——含 CPU 调度、内存、低内存查杀、功耗、温控等，设计阶段细化分析（设计文档列出各维度接口与一期/二期取舍） |
| Q-10 | RSS 对接方式？ | 决定资源调度子模块与 RSS 的边界 | 已澄清：**RSS 具体设计由 RSS 团队主导**；本模块封装 **ResourceControlProxy 资源管控代理层**（管控动作语义接口 + 完整上下文暴露 + 一期骨架实现：内存登记+日志+告警+探测开关——记录机制不持久化），RSS 明确后替换真实实现，上层零改动 |

### 讨论记录

> 仅保留最新一条变更记录；历史记录见 git。

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-09-14 | yangzk、用户 | **需求变更：基线刷新至 v1.2** | ① 痛点修正：近半场景为锁屏下唤醒，锁屏任务无法执行的真实原因 = EL3~EL5 加密等级数据锁屏不可访问（安全/存储域，范围外+外部依赖）+ 锁屏执行功耗约束；新增痛点：后台任务经虚拟屏执行被当前台应用调度，抢占真实前台（帧率/启动）——虚拟屏为一期接口分析+扩展点预留，能力不满足列二期；② 期望结果删除状态机/状态查询监听/INPUT_REQUIRED·AUTH_REQUIRED 交互承载/ADK 参考表述（DFX 以需求要点"DFX 手段"条为锚保留）；③ 需求要点删除原 13 条中第 1/2/5/6/8 条，余 8 条重编；启动链路收窄为**意图工具/CLI 工具/A2A 三链路**；④ 接口面 = **最小三接口 startTask/finishTask/abortTask**（无查询/参数调整/监听）；**无任务状态**——超时时间系统侧自动结束 + 创建者死亡自动结束（纳管通道见⑥）；⑤ **System API 新增**：startTask/finishTask/abortTask 以 System API 开放（@systemapi 标注、@since 7.1，NAPI 绑定 + d.ts，面向系统应用；d.ts 在 interface_sdk_js 仓提审，外部协同），Public API 不开放；⑥ **纳管接口重构**：三链路 taskId 携带改为系统接口参数对象扩展（参考 task.md 4.2~4.4：CLI `ExecOptions.taskSessionId`、意图 `ExecuteParam.taskSessionId`、A2A `A2ACallContext.taskSessionId`，可选字段、签名不变、存量零适配）——移除 RegisterTaskProcess/RegisterTaskIntent/UnregisterTaskProcess 内部 IDL 接口（系统侧内部通道为内部实现）；防仿冒对齐 task.md 五道防线（系统签发/owner 归属校验/通道隔离/存活态时效/跨域 ID 降级），冒用与重放 fail-closed 拒绝执行 | proposal/spec 已刷新至 v1.2（本文档与 spec.md）；design/plan/review-analysis 等其余文件待 proposal 与 spec 评审通过后同步刷新 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | 任务模型与生命周期（startTask/finishTask/abortTask、超时与创建者死亡自动结束）、三链路（意图工具/CLI 工具/A2A）taskId 归因纳管、RSS 分级管控（豁免冻结/解冻/结束降级）、多任务并发与进程多任务归属（按高优先级）、DFX | 需求方 | 已确认 |
| 明确不包含哪些？ | 安全管控细则、新增 RSS 插件/resType（一期）、Public API（三方开放）、任务持久化/跨设备、定时调度本身 | 需求方 | 已确认 |
| 是否有分期策略？ | 一期：完整功能（含三个 System API）；二期预留：RSS 专用接口细化提权、CPU/内存优先级、更多任务接口 System API 化（按需评估） | 需求方 | 已确认 |

### 方案探索

> 标准复杂度必填。列出候选方案与取舍理由。

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | 任务管理模块并入 **agentmgr**（SA 185） | 任务语义与 Agent 框架（AI 助手/Agent 生态）贴近；`agent_runtime_framework/` 在 AGENTS.md 中标记 🟢 低风险 | ① 关键缺陷：agentmgr 只承接 Agent 扩展连接业务，**A2A 任务、意图、CLI 启动的进程均不经过 agentmgr**，而任务纳管的对象是任意应用进程，agentmgr 纳管需绕道 AMS/AppMS 取进程信息，链路长且收口不全；② agentmgr 无进程权威信息（pid/状态/死亡回调）；③ 需新增 ressched_client 依赖 | 放弃 |
| A-2 | 任务管理并入 **AppMS**（SA 501），抽象为通用"**App 进程调度管理**"能力 + 任务语义层（新推荐） | ① **进程权威信息在 AppMS**：所有应用进程（无论经 StartAbility/ConnectAbility/意图/Cli/A2A 何种链路启动）必经 AppSpawn 孵化→AttachApplicationThread→进程死亡，pid/进程状态/死亡回调/前后台切换全在手，纳管收口最完整；② RSS 管控本就是进程维度，appmgr 已依赖 `resource_schedule_service:ressched_client`（BUILD.gn:159），交互通道现成；③ **可抽象为通用能力**：AppMS 提供"进程分组调度"（按组对进程施加/撤销 RSS 管控、按优先级合并、死亡自动清理、结束恢复），不绑定 AI 任务语义，未来其他后台保障场景（系统级长时任务等）可复用；④ 收口：AI 助手与系统 UI 只对接一个服务，单一接口面；⑤ 与 AMS/agentmgr 同 foundation 进程（sa_profile/501.json），AMS 收敛点解析经 inner_api 客户端调用成本低 | AppMS 在 AGENTS.md 标记 🔴 高风险（修改影响所有应用进程），需以独立目录/独立 IDL/最小挂点控制回归面（见缓解措施） | **推荐** |
| A-2b | 分层：通用进程调度能力放 AppMS，任务语义层放 agentmgr | 职责最纯（进程管控 vs 任务业务） | 接口面分裂：两个 SA 各自对外，AI 助手/系统 UI 需跨两个服务；agentmgr 与任务语义无强绑定（Agent 连接业务 ≠ 任务业务）；进程纳管仍需从 agentmgr 绕回 AppMS | 备选，不推荐 |
| A-3 | 新增独立 SA（任务管理服务） | 职责最独立、可独立演进 | 新增 SA 需审批、进程/IPC 形态最重；进程信息仍需经 AppMS 获取，链路最长 | 放弃 |

**取舍理由（A-2 推荐）：**

1. **收口判断（用户关键反馈）**：A2A 的任务、意图、CLI 都不会经过 agentmgr，但**所有应用进程都会经过 AppMS**——任务纳管与管控的执行对象是"进程"，AppMS 是进程生命周期的权威与必经点（AppSpawn 孵化、AttachApplicationThread、前后台切换、进程死亡全部在 AppMS 发生并记录）。在 AppMS 收口，意味着无论 AI 助手通过哪条链路（意图工具/CLI 工具/A2A）启动能力，其进程纳管都能在唯一权威处完成，无需跨服务拼接信息。
2. **抽象化（用户建议）**：将"对一组进程按优先级施加/撤销 RSS 管控、进程死亡自动清理、多组归属按最高档合并、组结束恢复原调度状态"抽象为 AppMS 的**通用进程调度管理能力**（与业务无关）；任务管理（任务模型与生命周期）作为其上层语义模块，两者在 AppMS 内分目录实现，对外单一接口面。
3. **🔴 高风险目录的缓解**（AGENTS.md 将 services/appmgr 标记为"修改影响所有应用进程"）：
   - 新模块独立目录（如 `services/appmgr/src/task_mgr/` + `process_schedule/`），既有热路径（进程启停/前后台/ANR 检测）零改动；
   - 对外独立 IDL（新增 ITaskManager / IAppTaskManager），**不扩 IAppMgr 既有接口、不改既有 IPC 序列化**（遵守 AGENTS.md 约束）；
   - 与既有能力的挂点最小化：进程死亡/状态通知复用既有 observer/death-recipient 链（AppStateObserverManager、AppDeathRecipient），不新增侵入式钩子；
   - AGENTS.md 约束"services/abilitymgr 不得直接调用 AppManagerService 内部方法，应经 interfaces/inner_api/app_manager/ 的 IPC 客户端"——AMS 收敛点经既有 app_manager IPC 客户端通道（AMS 本就通过该通道调 AppMS）新增接口，符合约束；
   - 单测覆盖 `app_manager_service_test` 相关套件 + 新增任务管理单测。

### 方案探索（深化）：taskId 防仿冒

**问题**：Want 参数明文携带 taskId，任意应用发起 StartAbility/ConnectAbility 时都可在 Want 中伪造 taskId，把自身（或无关）进程注册进某任务——若任务为高优先级（冻结豁免），则恶意应用可借此**绕过后台管控长期保活**，或干扰任务进程集合。

| 编号 | 方案 | 说明 | 取舍 |
|------|------|------|------|
| B-1 | 不可猜测随机 taskId | startTask 返回 128-bit 随机 ID（UUID v4 风格），防猜测 | 基础要求，但防不了"拿到即可用"（AI 助手经 Want 传给目标能力应用后，目标应用可转发/泄露） |
| B-2 | **owner 归属校验（推荐）** | 任务签发时记录 owner = IPC 取证的 caller 真实身份（不可被参数伪造）；三链路系统侧（cliManager/意图框架/A2A SDK 边界/AMS）重取 caller 真实身份与 owner 比对 + 会话存活态校验（防重放）+ 通道隔离（模型可控通道出现任务标识一律剥离）；仅任务创建者或任务内进程可归因新进程 | 防仿冒最彻底：身份取自信任边界而非参数；链式启动（任务内进程再启动新进程）为可信链语义；无需改调用方体验 |
| B-3 | token 化（短时一次性 access token 换 taskId） | AI 助手持短时 token 发起启动，AMS 以 token 换 taskId；token 一次/短时有效 | 防重放增强；增加调用方与系统复杂度；作为 B-2 之上可选增强，设计时定 |
| B-4 | 显式注册接口（调用方逐进程注册纳管） | 最安全但调用方负担重、易遗漏 | 放弃——由三链路参数对象扩展（Q-3）以更低负担达到同等校验强度 |

**结论（推荐）**：B-1 + B-2 组合——系统签发不可猜测 taskId + owner 归属校验/存活态校验/通道隔离（task.md §5 五道防线）；B-3 作为可选增强。A2A 链路：端侧唯一采信本机签发 taskId，对端报文中的任务 ID 仅作关联映射（bindRelatedId，单向不可逆）。


### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | ability（ability_runtime：AppMS 新模块 + AMS 收敛点解析 + System API；CLI/意图/A2A 三链路接口扩展跨 Kit 协同）；resourceschedule（仅接口语义确认，不改代码） | yangzk | 已确认 |
| 是否需要新增子系统或部件？ | 否。模块并入既有 appms 部件（libappms.z.so），不新增 SA/部件 | yangzk | 已确认 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | yangzk | 已确认 |
| 是否需要新增 System API？ | 是：**startTask / finishTask / abortTask** 三个（@systemapi 标注、@since 7.1，NAPI 绑定 + d.ts，面向系统应用） | yangzk | 已确认 |
| 是否会废弃已有 API？ | 否 | yangzk | 已确认 |
| 是否需要新增权限声明？ | 是：基础身份校验（系统应用/SA 调用方）在 AppMS 侧入口校验（复用既有身份校验能力），不新增权限项（细则属安全管控，本需求范围外） | yangzk | 待设计确认 |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | 全部新增 System API（三接口）与 inner API 及新增模块；三链路扩展为既有接口可选字段（签名不变），不传字段行为与现网一致（存量零适配） | yangzk | 已确认 |
| 性能 | 响应时间/内存/并发要求？ | 任务操作（startTask/finishTask/abortTask）< 5ms（ITaskManager 跨进程 IPC 含 Binder 往返）；三链路校验附加开销无感知（不阻塞调用主流程）；支持多任务并发（≥16 并发任务设计基线）；内存：任务记录轻量（≤1KB/任务，payload 变长另计） | yangzk | 待设计验证 |
| 安全 | 权限/隐私/加密/审计要求？ | 本需求明确排除安全管控细则；接口做基础调用方校验（拒绝非系统应用）；taskId 仅用于进程纳管映射，不涉及隐私数据 | 需求方 | 已确认 |
| 可靠性 | 崩溃率/容错/恢复要求？ | AppMS 崩溃（foundation 进程异常）后任务记录丢失可接受（进程管控自动失效，任务由调用方重新创建）；纳管进程死亡需自动清理任务进程映射并降级 | yangzk | 已确认 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| `resource_schedule_service:ressched_client` | 编译 | appmgr BUILD.gn 已具备该依赖（BUILD.gn:159，无条件依赖），无需新增 | 已确认 |
| RSS suspend exempt/thaw 接口语义 | 运行 | payload 约定、豁免覆盖范围（锁屏/standby 冻结）需 RSS owner 评审确认 | 待确认 |
| EL3~EL5 加密数据锁屏访问机制 | 外部（运行） | 锁屏场景任务执行的**数据可访问前提**；本模块仅保障进程不被冻结（范围外+外部依赖，安全/存储域） | 待需求方推动 |
| d.ts 接口定义提审（interface_sdk_js 仓） | 外部（协同） | System API（startTask/finishTask/abortTask）与三链路扩展字段（ExecOptions/ExecuteParam/A2ACallContext）的 d.ts 定义在 SDK 接口仓维护提审，需与各 NAPI 绑定同步评审 | 待提审 |
| 三链路系统接口字段扩展（CLI/意图/A2A） | 外部（协同） | 既有 Kit/SDK 接口新增可选字段，需 cli_tool_framework、意图框架、A2A SDK 三方团队评审；不传字段行为与现网一致（存量零适配） | 待对齐 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| RSS 接口 payload 语义与豁免范围不符预期（尤其锁屏冻结场景） | 技术 | 高 | 设计阶段先与 RSS owner 对齐语义；一期以现有 resType 落地，异常场景降级为"不豁免"，不影响原有调度 | 待确认 |
| 冻结豁免被滥用导致资源管控失效 | 安全/技术 | 中 | 仅系统应用可创建任务；任务数量上限；高优先级任务豁免有活跃/时长限制（可配）；审计 hisysevent | 待设计 |
| 系统侧校验与纳管引入额外延迟/异常 | 性能 | 中 | 三链路校验只做身份比对与参数解析，冒用/重放按 fail-closed 拒绝执行；未携带字段时零开销；纳管失败不影响合法调用结果 | 待设计 |
| 与背景任务/standby/冻结策略联动时语义冲突（如设备熄屏深度休眠） | 技术 | 中 | 一期仅覆盖 RSS 冻结豁免场景，深度休眠场景在验证中确认边界 | 待验证 |
| 虚拟屏任务进程被当前台调度，抢占真实前台资源（帧率/启动） | 技术 | 中 | 一期经资源调度子模块完成调度抑制的接口分析（134/221 等候选通道适用性）并与 RSS/窗口（屏幕会话）域对齐；ResourceControlProxy/档位扩展位预留，能力不满足明确二期 | 待对齐 |

### AC 完整性

- [x] 每个用户故事有验收标准（见基线部分 US-1~US-4 + AC-1~AC-17）
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（3 个方案 + 取舍理由，含 AppMS 方案详细分析）

**结论:** 条件通过（需求方已确认：① 模块形态取 A-2 **AppMS 收口 + 通用进程调度能力抽象**；② 目标发行版本 **7.1**。剩余待确认：③ 任务类型/优先级枚举初稿（设计阶段细化）；④ 锁屏豁免场景边界与 RSS owner 对齐结果）

---

## 三、需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.2 |
| 基线日期 | 2026-09-14 |
| Owner | yangzk |
| 确认人 | 需求方（AI 语音助手团队）、ability_runtime 模块 Owner（待确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | 7.1（已确认） |
| 版本状态 | committed |

### 问题陈述

AI 语音助手近半场景为锁屏下唤醒，锁屏下任务无法执行：进程侧因 RSS 冻结/资源管控而执行失败或中断（本需求解决的部分）；数据侧因 EL3~EL5 加密等级数据锁屏不可访问（外部依赖，安全/存储域）。后台长时任务切后台被冻结中断；后台任务经虚拟屏执行时被当前台应用调度，抢占真实前台资源（帧率/启动）。系统缺少一个可信的"任务"级维度来表达 AI 助手"后台持续执行且不扰前台"的诉求。需要提供一套任务管理机制：AI 助手以最小 System API（startTask/finishTask/abortTask）创建并结束任务（指定优先级、类型、可选超时），任务执行涉及的进程（经意图工具/CLI 工具/A2A 调用启动）自动纳入任务，任务管理模块按任务优先级对进程施加 RSS 管控（冻结豁免、解冻、结束后降级），任务超时或创建者死亡时系统自动结束并降级；提供基础 DFX 手段。安全管控问题不在本需求范围；EL3~EL5 数据锁屏访问属安全/存储域外部依赖；虚拟屏调度抑制为一期分析+预留项。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 锁屏定时任务可执行完成 | 锁屏场景下高优先级任务进程不被冻结（**前提：任务数据锁屏可访问——EL3~EL5 限制为外部依赖，未满足时本指标标注外部阻塞，仅验证冻结豁免**），任务回归用例通过（成功率基线由需求方给出，默认 100% 于回归用例） | 真机场景用例（锁屏 + 定时任务） |
| 后台长时任务不被冻结中断 | 后台切换后任务进程不被 RSS 冻结，任务完成 | 真机场景用例（前后台切换 + 长时任务） |
| 后台任务不抢占真实前台资源（虚拟屏场景） | 一期：完成虚拟屏任务进程调度抑制的接口分析与 RSS/窗口域对齐结论，扩展点预留（ResourceControlProxy/档位扩展位）；能力不满足则明确二期交付与验证指标 | 一期：接口分析报告+对齐结论（开放项 O-8）；能力落地后：真机前后台对比用例（帧率/启动无劣化） |
| 任务级管控对原有调度无影响 | 无 taskId 的启动/进程调度行为与改动前完全一致 | 全量回归 + 性能对比 |
| 多任务并发与进程多任务归属正确 | 进程按最高优先级任务管控；任务结束降级正确（单测覆盖多任务/多进程矩阵） | 单测 + 集成用例 |
| 任务执行可观测（DFX） | 任务生命周期（start/结束/超时/创建者死亡/纳管/管控动作）经 hisysevent 可检索，dump 输出活跃任务全景（DFX 为唯一可观测面） | 事件检索用例 + dump 用例 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为 AI 语音助手，我想要以最小 System API 管理任务生命周期（startTask 创建并启动、finishTask/abortTask 结束），并指定类型、优先级与可选超时，以便表达任务执行诉求且系统可在超时/创建者死亡时自动收口 | P0 |
| US-2 | 作为 AI 语音助手，我想要通过意图工具、CLI 工具、A2A 调用工具能力时携带 taskId 并把目标进程纳入任务，以便任务级管控进程 | P0 |
| US-3 | 作为任务管理模块，我想要按任务优先级对纳管进程施加 RSS 分级管控（冻结豁免/解冻/降级），以便在锁屏/后台场景保障任务执行且结束后恢复 | P0 |
| US-4 | 作为系统开发者，我想要任务管理具备 DFX（hisysevent、dump），以便排查任务执行问题 | P2 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN AI 助手调用 startTask(类型、优先级、可选超时) THEN 返回不可猜测 taskId，任务立即激活（纳管进程开始按任务优先级受管控）并上报 TASK_START 事件 | 正常 | US-1 |
| AC-2 | WHEN 创建者调用 finishTask/abortTask THEN 任务结束，纳管进程全部降级（多任务归属时按次高优先级续管），任务记录清理，上报 TASK_END 事件（reason=FINISH/ABORT） | 正常 | US-1 |
| AC-3 | WHEN 任务存活时间超过超时时间（startTask 入参或系统默认，且不超过系统硬上限）THEN 系统自动结束任务（同 AC-2 清理路径）并上报 TASK_END（reason=TIMEOUT）告警 | 异常 | US-1 |
| AC-4 | WHEN 创建者进程死亡 THEN 其全部任务由系统自动结束（同清理路径）并上报 TASK_END（reason=CREATOR_DEAD） | 异常 | US-1 |
| AC-5 | WHEN 非任务创建者调用 finishTask/abortTask THEN 返回权限错误码；不存在的 taskId 返回 ERR_TASK_NOT_FOUND | 异常 | US-1 |
| AC-6 | WHEN 创建者活跃任务数达到上限 THEN startTask 返回 ERR_TASK_LIMIT_EXCEEDED | 边界 | US-1 |
| AC-7 | WHEN 任务处于存活期且创建者（或任务内进程）经意图工具/CLI 工具/A2A 调用工具能力、调用参数携带 taskId（ExecOptions/ExecuteParam/A2ACallContext 的 taskSessionId 字段）THEN 目标进程自动纳入任务管控（系统侧 owner+存活态校验） | 正常 | US-2 |
| AC-8 | WHEN 调用携带伪造或冒用的 taskId（非本创建者签发、任务不存在或已结束）THEN 拒绝执行该次工具调用并上报安全事件（fail-closed，防冒用与重放） | 异常 | US-2 |
| AC-9 | WHEN 同一进程被多个任务纳管 THEN 该进程按其中最高优先级任务执行 RSS 管控 | 边界 | US-2 |
| AC-10 | WHEN 纳管的进程死亡 THEN 该进程从任务记录移除，任务不自动结束（创建者可继续纳管新进程） | 异常 | US-2 |
| AC-11 | WHEN 三链路调用参数未携带 taskSessionId 字段 THEN 调用行为与现网完全一致（存量调用方零适配） | 边界 | US-2 |
| AC-12 | WHEN 高优先级任务存活期间（startTask~结束）THEN 纳管进程冻结豁免生效（经 RSS DYNAMICALLY_SET_SUSPEND_EXEMPT），锁屏/后台场景进程不被冻结（数据可访问为外部前提，未满足时仅验证冻结豁免） | 正常 | US-3 |
| AC-13 | WHEN 任务结束（finish/abort/超时/创建者死亡）THEN 纳管进程豁免撤销，恢复任务前的 RSS 调度状态（多任务归属时按次高优先级任务继续管控） | 正常 | US-3 |
| AC-14 | WHEN 低优先级（LOW）任务 THEN 不豁免，不占用管控资源（时长约束由任务超时时间统一承载） | 边界 | US-3 |
| AC-15 | WHEN 执行 aa dump/hidumper 任务相关命令 THEN 输出活跃任务列表（taskId、类型、优先级、纳管进程、管控档位、剩余超时） | 正常 | US-4 |
| AC-16 | WHEN 任务 start/结束/超时/纳管/管控动作发生 THEN 上报 hisysevent 行为事件，可检索 | 正常 | US-4 |
| AC-17 | WHEN 系统应用经 System API 调用 startTask/finishTask/abortTask THEN 接口可用且行为与规格一致；三方应用（非系统应用）调用 THEN 拒绝（@systemapi 访问控制） | 边界 | US-1 |

### 范围边界

**包含：**
- 任务模型与生命周期：**startTask/finishTask/abortTask 三个 System API**（@systemapi，优先级、类型、可选超时与 payload 描述）；无任务状态机——任务自 startTask 激活至结束（finish/abort/超时/创建者死亡），超时时间可配（sysparam 硬上限 + startTask 入参约束）
- 纳管通道：三链路系统接口参数对象扩展携带 taskId——CLI `ExecOptions.taskSessionId`（ExecCmdOptions 同步）、意图 `ExecuteParam.taskSessionId`（系统保留字段，转写为 Want 兜底 `ohos.param.taskSessionId`）、A2A `A2ACallContext.taskSessionId`（带外上下文，不进协议报文）；系统侧 owner 归属校验+存活态校验+通道隔离（模型可控通道禁止任务标识，task.md §5）
- RSS 资源调度子模块：按优先级分级（冻结豁免、解冻、状态查询、结束降级），复用 RSS 现有接口，策略可配，预留扩展点；含**虚拟屏任务进程调度抑制**维度（一期接口分析+对齐+扩展点预留，能力不满足列二期）
- DFX：hisysevent（hisysevent.yaml 新增事件）、hidumper/dump（活跃任务全景）、TAG_LOGI 日志（**DFX 为唯一可观测面**）
- 进程纳管数据：taskId↔pid 映射、多任务归属（按高优先级）、进程死亡自动清理、创建者死亡任务自动结束

**不包含：**
- 安全管控细则、Public API（三方开放）、任务持久化/跨设备、定时调度、新增 RSS 插件/resType、EL3~EL5 加密数据锁屏访问机制（安全/存储域外部依赖）、任务状态机与状态查询/监听/交互承载


### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | ability_ability_runtime | `services/appmgr/src/task_mgr/`（新增） | — | 新增：任务管理模块（任务模型与生命周期、超时与创建者死亡自动收口、inner API 服务、DFX；无状态机/监听/交互） | yangzk |
| ability | ability_ability_runtime | `services/appmgr/src/process_schedule/`（新增） | — | 新增：通用"App 进程调度管理"能力（进程组纳管、优先级档、RSS 管控施加/撤销、死亡清理、多组归属合并） | yangzk |
| ability | ability_ability_runtime | `services/appmgr/`（既有热路径） | 进程生命周期管理 | 最小挂点：复用既有进程死亡/状态通知链（AppStateObserverManager/AppDeathRecipient）接入新模块（兼创建者死亡任务自动结束），不修改既有接口与序列化 | yangzk |
| ability | ability_ability_runtime | `interfaces/inner_api/app_manager/` | 内部组件接口 | 新增：ITaskManager IDL（stub/proxy）+ C++ 客户端（NAPI 绑定层/AMS/aimgr 使用，System API 的底层承载）；不扩 IAppMgr 既有接口 | yangzk |
| ability | ability_ability_runtime | `frameworks/js/napi/`（新增任务管理 NAPI 模块） | JS/NAPI 绑定 | 新增：startTask/finishTask/abortTask 的 NAPI 绑定（经 TaskManagerClient 调 AppMS，nativecommon 组件惯例）；d.ts（@systemapi、@since 7.1）在 interface_sdk_js 仓提审（外部协同，见依赖表） | yangzk |
| ability | ability_ability_runtime | `services/abilitymgr`（含 InsightIntent 执行链路） | AMS 启动/连接调度 | 修改：启动/连接收敛点解析 Want 兜底字段 `ohos.param.taskSessionId`（意图/A2A 链路由系统接口转写注入），经 app_manager inner_api 客户端转发 AppMS 纳管（内部通道，非公共接口） | yangzk |
| ability | ability_ability_runtime | `cli_tool_framework/services/climgr/`（cliManager） | CLI 工具进程管理 | 修改：消费 `ExecOptions/ExecCmdOptions.taskSessionId` 可选字段——执行前 owner+存活态校验，CLI 工具进程及子进程挂入任务、会话退出清理（函数签名不变；需 cli_tool_framework 团队评审配合） | yangzk |
| ability | 意图框架（insightIntentDriver/AiDispatch 侧，归属以实际为准） | 意图执行 | 修改：`ExecuteParam` 新增系统保留字段 taskSessionId（与 insightIntentParam 隔离、过滤覆写），分发转写为 Want 兜底字段（需意图框架团队评审配合） | 待定 |
| ability | A2A SDK（@hw-tongtu/a2a） | A2A 调用 | 修改：connect/sendMessage 增加可选调用上下文 A2ACallContext{taskSessionId}（带外传递，不进协议报文；需 A2A SDK 团队评审配合，外部协同） | A2A SDK 团队 |
| ability | ability_ability_runtime | `hisysevent.yaml` / `services/appmgr/BUILD.gn` / `ability_runtime.gni` | DFX 事件定义/构建 | 新增：任务事件定义；BUILD.gn 新增源文件（ressched_client 依赖已具备） | yangzk |
| resourceschedule | resource_schedule_service | `ressched`（接口层） | RSS 管控 | 不改代码；RSS 具体设计由 RSS 团队主导（暂未明确），本模块经 ResourceControlProxy 资源管控代理层对接（设计 §6.4） | RSS 团队 |

### API 变更项清单

> 涉及 System API 与 inner API 变更，作为 design.md 与 spec.md 的共享锚点。

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| startTask / finishTask / abortTask（System API） | 新增 | SystemAPI | 任务生命周期三个 System API（@systemapi 标注、@since 7.1；NAPI 绑定放 `frameworks/js/napi/`，d.ts 在 interface_sdk_js 仓提审；面向系统应用，Public API 不开放） |
| ITaskManager（IDL 新增，放 `interfaces/inner_api/app_manager/task_manager/`，方法以接口定义定稿为准） | 新增 | InnerAPI | System API 的底层承载（startTask/finishTask/abortTask）；系统侧内部纳管通道（CLI/意图/A2A→AppMS）为内部实现，不在需求层清单展开 |
| 三链路 taskSessionId 字段扩展（task.md 4.2~4.4） | 扩展 | SystemAPI | CLI `ExecOptions/ExecCmdOptions.taskSessionId`、意图 `ExecuteParam.taskSessionId`、A2A `A2ACallContext.taskSessionId`——既有接口可选字段（签名不变，不传=现网行为），系统侧 owner+存活态校验后自动纳管 |
| TaskManagerClient（C++ 客户端） | 新增 | InnerAPI | NAPI 绑定层/AMS/aimgr 使用的客户端封装 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 是 | 启动热路径解析开销、三链路校验开销、并发任务数 | design.md / spec.md |
| 安全与权限 | 是（仅基础鉴权） | 安全管控细则明确不在范围；接口需拒绝非系统应用调用 | design.md |
| 兼容性 | 是 | 全新增 inner API 与模块，无既有行为变更；无 taskId 启动零影响 | spec.md |
| API/SDK | 是 | 新增 startTask/finishTask/abortTask 三个 System API（@systemapi）+ inner API（IDL/NAPI 绑定），无 Public API | design.md / spec.md |
| IPC/跨进程 | 是 | ITaskManager IPC（stub/proxy）、AMS 同进程调用（经 inner_api 客户端体系） | design.md |
| 构建与部件 | 是 | appmgr BUILD.gn 新增源文件与 IDL 生成（ressched_client 依赖已具备） | design.md |
| 国际化/无障碍 | 否（N/A） | 任务机制无 UI 文案（呈现归 AI 助手自有 UI） | — |
| 数据迁移 | 否（N/A） | 任务不持久化，进程退出即清理 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 System/inner API 方法或参数 | 触发设计审批 |
| 非功能指标变更 | 性能/兼容性阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [x] `proposal.target_release` 已确认（7.1）
- [x] `manifest.profile` 已确认（none）
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭，讨论记录包含需求方明确确认
- [x] 目标仓 Agent 指南已检查并记录关键约束（AGENTS.md：services/appmgr 🔴 高风险需最小挂点、不直接调 appmgr 内部方法需经 inner_api 客户端、BUILD.gn 必更新、DFX 定义 hisysevent.yaml）

**基线结论:** 通过 —— 需求方确认全部关键决策：模块形态（A-2 AppMS 收口 + 通用进程调度能力）、目标版本（7.1）、防仿冒（系统签发 + owner 归属校验 + 通道隔离，task.md 五道防线）、RSS 集成（复用现有接口）、RSS 对接方式（**RSS 团队主导设计，本模块封装 RSS Proxy 代理层**，Q-10）、纳管方式（**三链路系统接口参数对象扩展**：CLI ExecOptions / 意图 ExecuteParam / A2A A2ACallContext 可选字段 taskSessionId + Want 兜底 `ohos.param.taskSessionId`，签名不变、存量零适配）、接口面（**startTask/finishTask/abortTask 三个 System API + 内部 inner API，无任务状态机/查询/监听/交互**）、超时自动结束（**系统侧超时收口 + 创建者死亡自动结束**）。基线成立（v1.2），开放项随设计/验证阶段关闭：
1. 任务类型/优先级枚举初稿（design.md 细化，随设计评审确认）；
2. 锁屏豁免场景边界（**搁置**，不阻塞基线；真机验证阶段与 RSS owner 对齐后确认）；
3. RSS 管控维度细化（冻结/CPU 调度/内存/低内存查杀/功耗/温控，design.md 分析各维度接口与一期/二期取舍）；
4. sessionQos 机制（design.md 探索）；
5. **RSS 真实对接**（RSS 设计明确后由 ResourceControlProxyImpl 承接，design.md §6.4 对接策略）；
6. **枚举对齐**（任务类型/优先级枚举初稿与 AI 语音助手团队对齐，评审后确认）；
7. **EL3~EL5 锁屏数据访问**（外部依赖，安全/存储域，需求方推动——成功指标 1 的前提条件）；
8. **虚拟屏调度抑制**（一期接口分析+RSS/窗口域对齐，能力不满足列二期）。

**锁屏豁免场景边界（解释，供设计/验证阶段参考）：** "边界"指设备锁屏后应用进程面临的**冻结管控的组成与豁免接口的覆盖范围**：1) RSS suspend 冻结（按前后台/空闲判定，`RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT` 主要豁免此类）；2) standby（待机）深度冻结（设备静置进入待机管控，豁免是否覆盖待与 RSS owner 确认）；3) 豁免的时长/数量上限与调用方资质约束。该边界决定 AC-12（锁屏定时任务进程不被冻结）的可达成性与实现范围。
