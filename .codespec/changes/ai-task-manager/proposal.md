---
target_release: "7.1"
change_type: new-feature
issue: ""
author: "yangzk"
date: "2026-08-21"
status: Draft
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

**原始问题：** AI 语音助手面临着在定时任务锁屏下执行失败、长时任务切后台后被冻结无法执行。主要由于安全、RSS 管控（冻结、内存CPU优先级等）的影响。范围：安全管控方面的问题暂不讨论，优先分析 RSS 管控的问题。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| AI 语音助手用户 | 锁屏时定时任务（如闹钟联动、定时播报）执行失败；长时任务（如批量处理、持续播报）切后台后进程被 RSS 冻结，任务无法完成 | 核心语音场景不可用，用户感知明显 |
| AI 语音助手开发者 | 无任务维度的资源管控手段，只能依赖应用前台状态，无法表达"这个任务需要后台持续运行"的诉求 | 功能受限，需要绕过系统管控（不可接受） |
| 系统 | 应用长驻后台的诉求与 RSS 冻结/内存管控相冲突，缺少可信的中间机制 | 管控与体验失衡 |

**期望结果：** 提供一套任务管理机制给 AI 语音助手使用：AI 助手创建任务并指定优先级、类型等参数，任务管理模块将任务执行涉及的进程纳入任务并按任务优先级/类型进行 RSS 管控（冻结豁免、解冻、资源优先级等），任务结束自动降级恢复；任务以状态机管理，支持状态查询与监听（系统 UI 展示执行进度）、INPUT_REQUIRED/AUTH_REQUIRED 阶段的人机交互传达；提供 DFX 手段。任务概念类似 ADK 的 invocation，整体管控策略参考 ADK。（注：ADK 指 Google Agent Development Kit，见 K-9 调研）

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 需求方描述 | 本次需求对话（2026-08-21，两轮） | 原始需求 13 条，见下；第二轮反馈：模块并入 AppMS、目标版本 7.1、参考 google-adk |
| ADK（google-adk） | https://github.com/google/adk-docs 、https://adk.dev/context/index.md | Google Agent Development Kit：Invocation/Session/Event/Runner 概念，ToolContext.request_credential 认证挂起机制，A2A 参数透传；作为任务模型设计参考（K-9） |
| RSS 进程管理规范 | `foundation/resourceschedule/resource_schedule_service/Resource_Schedule_Service_Process_Management_Specifications_ZH.md` | RSS 新增插件需评审进程内存基线 → 本需求倾向复用现有 resType 接口而非新增插件 |

**需求方原始 13 条需求（原文要点）：**

1. 任务的概念类似 ADK 的 invocation，整体的管控策略可以参考 ADK 的。
2. AI 语音助手创建任务并指定任务优先级、类型等参数，并可设置任务状态（UNKNOWN、SUBMITTED、WORKING、INPUT_REQUIRED、AUTH_REQUIRED、COMPLETED、CANCELED、FAILED、REJECTED 等）。
3. AI 语音助手通过 StartAbility、ConnectAbility、意图框架、Cli、A2A 调用能力，启动过程携带 taskId 等标记信息，任务管理模块将进程纳入任务，按任务优先级、类型管控。
4. 任务管理模块提供任务创建、销毁、取消等接口，提供任务生命周期管理。
5. 任务以状态机形式处理。
6. 提供任务状态查询、监听机制，系统 UI 监听用于展示执行进度。
7. 提供基础 DFX 手段。
8. INPUT_REQUIRED、AUTH_REQUIRED 阶段（等待用户授权、明确用户意图），AI 助手给出具体描述和可选项，系统 UI 获取后提醒用户选择或输入，任务管理模块提供机制供两方传达。
9. 任务管理模块根据任务优先级、类型等参数制定 RSS 参数，对 task 下进程进行管理，具体策略放在任务管理模块的资源调度子模块中。使用 RSS 现有接口或新增进程管理接口。
10. 可能存在多任务并发的情况。
11. 一个进程可能被多个任务使用，按优先级更高的处理。
12. 一个任务结束后，如果其中的进程未退出，需要降级回原来的 RSS 调度状态。
13. 任务管理模块放在 foundation/ability/ability_runtime，相关 RSS 仓在 foundation/resourceschedule/resource_schedule_service。

### 初始范围

**可能包含：**
- 任务模型（taskId、类型、优先级、状态机、payload）与生命周期管理（创建/销毁/取消/状态流转）
- 启动链路（StartAbility/ConnectAbility/意图框架/Cli/A2A）Want 参数统一拦截，进程自动纳管
- 任务状态查询、监听（系统 UI 展示进度），INPUT/AUTH 阶段 payload 双向传达
- RSS 资源调度子模块：按优先级分级的管控（冻结豁免/解冻/状态降级为主线；CPU 调度、内存、低内存查杀、功耗、温控等维度设计阶段细化，复用 RSS 现有接口，预留扩展点）
- 基础 DFX（hisysevent、hidumper/dump、日志）
- 模块落在 `services/appmgr`（AppManagerService，SA 501，foundation 进程）：任务管理模块 + 通用"App 进程调度管理"能力（抽象化，详见方案探索 A-2）

**明确不包含：**
- 安全管控问题（权限校验策略细则、跨应用访问控制等），接口鉴权仅做基础身份校验
- 新增 RSS 插件/新增 RSS resType（一期复用现有接口；二期如需细化提权，另行与 RSS 仓对齐评审）
- Public/System SDK API（一期仅 inner API；NAPI 层预留目录结构，二期开放）
- 任务持久化/跨设备同步/任务恢复（进程退出即清理）
- 定时任务调度本身（定时触发归调用方，本模块只管任务执行期管控）

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| RSS 现有 `RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT`（505）、`SYNC_RES_TYPE_THAW_ONE_APP`（517）等接口可直接承载任务级冻结豁免/解冻，无需新增 resType | 技术 | RSS 源码勘察 + RSS owner 确认 | 已验证（res_type.h、res_sched_service.cpp:175 白名单命中；payload 语义待 RSS owner 确认） |
| 调用方（AI 助手、系统 UI）均为系统应用，任务接口基础身份校验（SA/系统应用）足够，不需要逐权限校验 | 业务 | 需求方确认 | 待确认 |
| 锁屏场景下冻结豁免覆盖目标冻结类型（锁屏冻结 = RSS suspend 冻结 + 可能叠加 standby 待机冻结，豁免接口覆盖范围待与 RSS owner 对齐） | 技术 | 与 RSS owner 确认 + 真机场景验证 | 待确认 |
| 目标发行版本 7.1 | 业务 | 需求方确认 | 已确认 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单仓主体 + 跨仓接口复用；含状态机、并发、多链路拦截，无新增 SA/无公共 API |
| 涉及仓数量 | 2 | ability_ability_runtime（主，含 AppMS 新模块 + AMS 拦截点）；resource_schedule_service（仅复用现有接口，不改动，需 owner 评审确认 payload 语义） |
| 是否涉及 Public/System API | 否 | 仅新增 inner API（IDL IPC 接口），无 Public/System SDK 变更 |
| 是否涉及安全/性能关键路径 | 是 | 系统服务（foundation 进程）、AMS 启动热路径拦截；安全管控本身不在本需求范围，但接口需基础鉴权 |
| 是否跨 SIG | 是 | ability（ability_runtime）+ resourceschedule（接口语义确认） |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定

---

## 二、澄清记录

> 2026-08-21 两轮澄清（AskUserQuestion 交互式问答），问题与结论如下。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 任务管理模块以什么形态落地？（独立 SA / 内嵌 AMS / 内嵌 agentmgr / 并入 AppMS） | 决定 IPC 形态、进程归属、与启动链路的关系 | 已澄清：**并入 AppMS（SA 501），抽象为通用"App 进程调度管理"能力 + 任务语义层**（第二轮需求方拍板：A2A 任务、意图、CLI 均不经过 agentmgr，但所有应用进程都经过 AppMS，AppMS 收口最佳） |
| Q-2 | RSS 管控如何集成？ | 决定一期落地方式与跨仓工作量 | 已澄清：先复用 RSS 现有接口（DYNAMICALLY_SET_SUSPEND_EXEMPT / THAW_ONE_APP / 冻结状态查询）+ 预留扩展点 |
| Q-3 | taskId 标记如何传递、进程如何纳入任务？ | 决定调用方负担与覆盖链路 | 已澄清：Want 参数统一拦截（启动链路收敛点解析），AI 助手调用侧无感知 |
| Q-4 | 任务管理接口面向谁？ | 决定 API 层级与权限设计 | 已澄清：先 inner API（IPC），预留 NAPI 层 |
| Q-5 | 状态机由谁驱动流转？ | 决定模块职责边界 | 已澄清：AI 助手驱动 + 模块校验流转合法性（终态不可回退等） |
| Q-6 | INPUT_REQUIRED/AUTH_REQUIRED 阶段两方如何传达？ | 决定交互通道机制 | 已澄清：状态监听回调携带 payload（描述+可选项），提交方调用 SubmitTaskInput 回传 |
| Q-7 | RSS 资源调度子模块管控粒度？ | 决定策略复杂度与一期范围 | 已澄清：按优先级分级策略（高优先级豁免冻结+解冻，低优先级限时豁免），CPU 提权预留扩展点 |
| Q-8 | SDD 流程产物与节奏？ | 决定工作方式 | 已澄清：先出设计文档评审，通过后再写代码 |
| Q-9 | Want 透传 taskId 是否可被仿冒？如何防？ | 决定纳管链路的信任模型 | 已澄清（2026-08-21 第三轮）：同意 **B-1+B-2**（不可猜测随机 taskId + caller 身份绑定校验）；B-3 token 化可选增强 |
| Q-10 | 可否借用 QosManager 的 sessionQos/QoS 机制管控？ | 决定资源调度子模块的管控手段构成 | 已澄清（2026-08-21 第三轮）：采纳借用评估结论（一期冻结豁免主线，QoS 提权二期扩展）；"sessionQos" 具体机制设计阶段再探索 |
| Q-11 | RSS 管控的维度范围？ | 决定资源调度子模块策略覆盖面 | 已澄清（2026-08-21 第三轮）：**不仅限冻结**——含 CPU 调度、内存、低内存查杀、功耗、温控等，设计阶段细化分析（设计文档列出各维度接口与一期/二期取舍） |
| Q-12 | RSS 对接方式？ | 决定资源调度子模块与 RSS 的边界 | 已澄清（2026-08-21 第四轮）：**RSS 具体设计由 RSS 团队主导**；本模块封装 **ResourceControlProxy 资源管控代理层**（管控动作语义接口 + 完整上下文暴露 + 一期骨架实现：内存登记+日志+告警+探测开关——记录机制不持久化，第十一轮定稿），RSS 明确后替换真实实现，上层零改动（第五轮确认命名与信息暴露；第十轮补 EDS P7 一期联动） |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-21 | yangzk、需求方 | 模块形态（第一轮） | 先定并入 agentmgr；补充分析并入 AppMS 的方案对比 | 写入方案探索 |
| 2026-08-21 | yangzk、需求方 | RSS 集成/纳管方式/接口面向（第一轮） | 复用现有 RSS 接口；Want 统一拦截；先 inner API | 写入基线 |
| 2026-08-21 | yangzk、需求方 | 状态机/交互/粒度/SDD 节奏（第一轮） | 助手驱动+模块校验；状态回调携带 payload；按优先级分级；先文档评审 | 写入基线 |
| 2026-08-21 | yangzk、需求方 | 模块形态（第二轮） | **改为并入 AppMS 收口**：A2A/意图/CLI 均不经过 agentmgr，但所有应用进程都经过 AppMS；抽象为通用"App 进程调度管理"能力；目标版本 7.1；参考 google-adk 设计 | 方案探索 A-2 定为推荐，重写取舍理由 |
| 2026-08-21 | yangzk、需求方 | 锁屏豁免场景边界（第二轮） | 解释边界含义：锁屏冻结的管控组成（RSS suspend / standby / 省电策略）与豁免接口覆盖范围，需与 RSS owner 对齐 | 基线条件 4 重述 |
| 2026-08-21 | yangzk、需求方 | 基线确认（第三轮） | ① 枚举初稿设计阶段细化；② 锁屏豁免边界**暂且搁置**（不阻塞基线，设计/验证阶段开放项）；③ 同意防仿冒 B-1+B-2；④ sessionQos 机制设计阶段探索；⑤ **RSS 管控维度扩展**（冻结/CPU 调度/内存/低内存查杀/功耗/温控）设计阶段细化 | 基线固化；Q-9/Q-10/Q-11 关闭 |
| 2026-08-21 | yangzk、需求方 | RSS 对接方式（第四轮） | **RSS 具体设计由 RSS 团队主导，暂未明确**；任务管理模块封装 **RSS Proxy 代理层**（语义接口 + 一期骨架：账本+告警+探测开关），RSS 明确后替换实现，上层零改动 | 设计文档 §6.4/ADR-4、spec R-19/R-20、plan TASK-4 同步更新；Q-12 关闭 |
| 2026-08-21 | yangzk、需求方 | 设计评审（第五轮） | ① 枚举初稿基本认可，**与 AI 语音助手对齐**（开放项）；② 两段式纳管安全分析：无新增仿冒面（B-2 两阶段校验），补 pid 归属校验与进程集合上限强化点；③ 同意执行计划；④ 锁屏豁免边界仍搁置；⑤ **RssProxy 更名 ResourceControlProxy，接口暴露完整上下文**（ControlRequest）以便扩展；⑥ 暂不开始写代码 | 设计 §5.3.1/§6.4 更新；proposal/spec/plan 同步 |
| 2026-08-24 | yangzk、需求方 | 模块设计深化（第六轮） | 三层模块（task_mgr / process_schedule / resource_control_proxy）细化、代码勘察 P1~P7 结论落地（开源 master 无 505/517/520 执行器、C-1 AppMS 崩溃即失效等）、对外三张 SVG 定稿 | design §6、module_design/task_flow/architecture_overview 图 |
| 2026-08-24 | yangzk、需求方 | 深度审查（第七轮） | F-1~F-7 七问题修正：激活态纳入 SUBMITTED、LOW 基础保障档、进程级纳管原则、RCP 生效校验环、交互超时、提级联动、意向清理 | review-analysis.md、design/spec 同步 |
| 2026-08-24 | yangzk、需求方 | 交互超时细化（第八轮） | F-5 两段式处置：交互超时 30min → OnTaskNotice 限期处置（宽限 5min）→ 宽限后仍未流转则主动降档撤销豁免+告警 | design §6.1、spec R-16/R-21 |
| 2026-08-24 | yangzk、需求方 | 全量复审（第九轮） | 21 项缺陷修复：状态表补 CANCELED、SubmitTaskInput 所有权、tier-2 持续豁免、单任务纳管进程上限 16、OnTaskNotice 通道、接口规格 6 节、TaskParams、多用户 userId 过滤、C-1 结论固化、风险表 D-1/D-2 | design/spec 全文（commit 365cff1acf） |
| 2026-08-24 | yangzk、需求方 | 开放点闭环（第十轮） | 13 项决策落定：**UI↔助手传递通道由本模块提供**（SubmitTaskInput 白名单=创建者+已注册监听的系统 UI，+OnTaskNotice(INPUT_ARRIVED)）；**豁免粒度=pid**（设计 D-1 关闭）；**EDS 一期经 P7 ApplyAllowResource**（设计 D-2 关闭）；续期仅 priority 变化触发、上限 N=3；各项上限经 CCM 可调；payload 权限管控 | design/spec 同步（commit 9a5864e8b5） |
| 2026-08-24 | yangzk、需求方 | 豁免记录降级（第十一轮） | "动作账本"降级为**内存登记**：不持久化、仅活跃任务 {taskId, pid, action, level, timestamp, result}，单一职责=RSS 重启重放+撤销重试，随 AppMS 进程消亡（与 C-1 一致）；计时/续期计数记 TaskRecord；审计/验收/定位/复核一律走 TASK_RSSCONTROL+hilog 日志，对账基线=动作日志序列 | design/spec/plan/HANDOVER/三图全量同步（commit 7747518002+本轮外围补齐） |
| 2026-08-26 | yangzk、用户 | 第十三轮自审与待定项确认 | ① §5.2 待定项**全部确认保留**：N=3 / INPUT_ARRIVED / 交互 30min·宽限 5min·档位1限时 30min·硬上限 60min·预警提前量 5min；② 自审修复 F-8~F-20（13 项）：**通道鉴权**（RegisterTaskIntent/RegisterTaskProcess 可被系统应用直调，native 信任入参身份/非 native 强制入参=IPC 实际身份+pid 归属校验——B-2 仿冒面闭环，安全）、**PAYLOAD_UPDATED 通知**（payload 更新实时触达系统 UI，US-3 进度展示补全）、spec 内部矛盾消除（SetTaskParams 可调字段/RegisterTaskProcess 签名）、NFR ≤1KB 与 payload 4KB 矛盾修正、拦截点①位置与 SetPid 事实精确化（ability_record.cpp:1096）、CCM→sysparam 落地机制明确、design/plan TASK 编号对齐、exemptRefCount 归属 process_schedule 等 | review-analysis.md 追加第十三轮节；design/spec/plan 同步；新增 docs/extension-analysis.md 与外部对齐材料 |
| 2026-08-26 | yangzk、用户 | 第十四轮代码实证复核 | 以 ability_runtime/resourceschedule 实际代码逐项验证文档断言，修复 F-21~F-28：**SA 501 承载机制**（AppMgrStub 描述符校验拒绝非 IAppMgr 请求——补 AppMgrService::OnRemoteRequest 描述符路由，design §5.2.2）、**扩展启动覆盖缺口**（StartServiceExtension/UIExtension/Modal 均不经 StartAbilityInner——拦截点①扩为 5 收敛点）、`low_memory_frozen_enable` 参数不存在修正、RSS 权限前置补全（REPORT_RESOURCE_SCHEDULE_EVENT/GET_SUSPEND_STATE/5523）、调用点清单恢复补全（13 处+2 文件）、hidumper 语法/测试路径惯例修正、20+ 项断言实证升级 | review-analysis.md 追加第十四轮节；design/spec/plan/rss-alignment 同步 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | 任务模型与生命周期（创建/销毁/取消/状态流转校验）、启动链路 Want 拦截纳管、状态查询/监听（系统 UI）、INPUT/AUTH 交互传达、RSS 分级管控（豁免冻结/解冻/结束降级）、多任务并发与进程多任务归属（按高优先级）、DFX | 需求方 | 已确认 |
| 明确不包含哪些？ | 安全管控细则、新增 RSS 插件/resType（一期）、Public/System API、任务持久化/跨设备、定时调度本身 | 需求方 | 已确认 |
| 是否有分期策略？ | 一期：完整功能（骨架范围）；二期预留：NAPI 开放、RSS 专用接口细化提权、CPU/内存优先级 | 需求方 | 已确认 |

### 方案探索

> 标准复杂度必填。列出候选方案与取舍理由。

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | 任务管理模块并入 **agentmgr**（SA 185） | 任务语义与 Agent 框架（AI 助手/Agent 生态）贴近；`agent_runtime_framework/` 在 AGENTS.md 中标记 🟢 低风险 | ① 关键缺陷（需求方第二轮指出）：agentmgr 只承接 Agent 扩展连接业务，**A2A 任务、意图、CLI 启动的进程均不经过 agentmgr**，而任务纳管的对象是任意应用进程，agentmgr 纳管需绕道 AMS/AppMS 取进程信息，链路长且收口不全；② agentmgr 无进程权威信息（pid/状态/死亡回调）；③ 需新增 ressched_client 依赖 | 放弃 |
| A-2 | 任务管理并入 **AppMS**（SA 501），抽象为通用"**App 进程调度管理**"能力 + 任务语义层（新推荐） | ① **进程权威信息在 AppMS**：所有应用进程（无论经 StartAbility/ConnectAbility/意图/Cli/A2A 何种链路启动）必经 AppSpawn 孵化→AttachApplicationThread→进程死亡，pid/进程状态/死亡回调/前后台切换全在手，纳管收口最完整；② RSS 管控本就是进程维度，appmgr 已依赖 `resource_schedule_service:ressched_client`（BUILD.gn:159），交互通道现成；③ **可抽象为通用能力**：AppMS 提供"进程分组调度"（按组对进程施加/撤销 RSS 管控、按优先级合并、死亡自动清理、结束恢复），不绑定 AI 任务语义，未来其他后台保障场景（系统级长时任务等）可复用；④ 收口：AI 助手与系统 UI 只对接一个服务，单一接口面；⑤ 与 AMS/agentmgr 同 foundation 进程（sa_profile/501.json），AMS 拦截点经 inner_api 客户端调用成本低 | AppMS 在 AGENTS.md 标记 🔴 高风险（修改影响所有应用进程），需以独立目录/独立 IDL/最小挂点控制回归面（见缓解措施） | **推荐** |
| A-2b | 分层：通用进程调度能力放 AppMS，任务语义层放 agentmgr | 职责最纯（进程管控 vs 任务业务） | 接口面分裂：两个 SA 各自对外，AI 助手/系统 UI 需跨两个服务；agentmgr 与任务语义无强绑定（Agent 连接业务 ≠ 任务业务）；进程纳管仍需从 agentmgr 绕回 AppMS | 备选，不推荐 |
| A-3 | 新增独立 SA（任务管理服务） | 职责最独立、可独立演进 | 新增 SA 需审批、进程/IPC 形态最重；进程信息仍需经 AppMS 获取，链路最长 | 放弃 |

**取舍理由（A-2 推荐，含用户第二轮反馈）：**

1. **收口判断（用户关键反馈）**：A2A 的任务、意图、CLI 都不会经过 agentmgr，但**所有应用进程都会经过 AppMS**——任务纳管与管控的执行对象是"进程"，AppMS 是进程生命周期的权威与必经点（AppSpawn 孵化、AttachApplicationThread、前后台切换、进程死亡全部在 AppMS 发生并记录）。在 AppMS 收口，意味着无论 AI 助手通过哪条链路（StartAbility/ConnectAbility/意图/Cli/A2A）启动能力，其进程纳管都能在唯一权威处完成，无需跨服务拼接信息。
2. **抽象化（用户建议）**：将"对一组进程按优先级施加/撤销 RSS 管控、进程死亡自动清理、多组归属按最高档合并、组结束恢复原调度状态"抽象为 AppMS 的**通用进程调度管理能力**（与业务无关）；任务管理（任务模型、状态机、监听、交互传达）作为其上层语义模块，两者在 AppMS 内分目录实现，对外单一接口面。任务概念对齐 google-adk：任务 ≈ Invocation（invocation_id=taskId）、INPUT/AUTH 交互 ≈ ToolContext.request_credential/get_auth_response 挂起机制、状态监听 ≈ Event 流、A2A 间 taskId 传递 ≈ supervisor 参数透传（K-9）。
3. **🔴 高风险目录的缓解**（AGENTS.md 将 services/appmgr 标记为"修改影响所有应用进程"）：
   - 新模块独立目录（如 `services/appmgr/src/task_mgr/` + `process_schedule/`），既有热路径（进程启停/前后台/ANR 检测）零改动；
   - 对外独立 IDL（新增 ITaskManager / IAppTaskManager），**不扩 IAppMgr 既有接口、不改既有 IPC 序列化**（遵守 AGENTS.md 约束）；
   - 与既有能力的挂点最小化：进程死亡/状态通知复用既有 observer/death-recipient 链（AppStateObserverManager、AppDeathRecipient），不新增侵入式钩子；
   - AGENTS.md 约束"services/abilitymgr 不得直接调用 AppManagerService 内部方法，应经 interfaces/inner_api/app_manager/ 的 IPC 客户端"——AMS 拦截点经既有 app_manager IPC 客户端通道（AMS 本就通过该通道调 AppMS）新增接口，符合约束；
   - 单测覆盖 `app_manager_service_test` 相关套件 + 新增任务管理单测。

### 方案探索（深化）：taskId 防仿冒

**问题**：Want 参数明文携带 taskId，任意应用发起 StartAbility/ConnectAbility 时都可在 Want 中伪造 taskId，把自身（或无关）进程注册进某任务——若任务为高优先级（冻结豁免），则恶意应用可借此**绕过后台管控长期保活**，或干扰任务进程集合。

| 编号 | 方案 | 说明 | 取舍 |
|------|------|------|------|
| B-1 | 不可猜测随机 taskId | CreateTask 返回 128-bit 随机 ID（对齐 ADK invocation_id `e-{UUID}` 风格），防猜测 | 基础要求，但防不了"拿到即可用"（AI 助手经 Want 传给目标能力应用后，目标应用可转发/泄露） |
| B-2 | **caller 身份绑定校验（推荐）** | AppMS 记录 taskId→creator（tokenId/uid）映射；AMS 拦截点校验发起方：**仅任务创建者、或任务内已纳管进程**发起的启动可携带该 taskId 纳管新进程；其余一律拒绝且不阻断启动（仅不纳管） | 防仿冒最彻底：恶意应用伪造 taskId 因非创建者/非任务内进程被拒；链式启动（任务内进程再启动新进程）语义与 ADK 一致（supervisor→sub-agent 可信链）；无需改调用方体验 |
| B-3 | token 化（短时一次性 access token 换 taskId） | AI 助手持短时 token 发起启动，AMS 以 token 换 taskId；token 一次/短时有效 | 防重放增强；增加调用方与系统复杂度；作为 B-2 之上可选增强，设计时定 |
| B-4 | 取消 Want 透传，改显式注册（RegisterTaskProcess(pid/token)） | 最安全但调用方需在启动前后显式注册，负担重、易遗漏 | 与 Q-3（Want 统一拦截）冲突，放弃 |

**结论（推荐）**：B-1 + B-2 组合——taskId 不可猜测 + 纳管仅接受"任务创建者或任务内进程"发起的携带；B-3 作为可选增强。A2A 链路同理：A2A 服务端进程若已纳管则其后续启动合法（任务内进程规则），否则需创建者（AI 助手）转发携带。

### 方案探索（深化）：QosManager 借用评估（Q-10）

**勘察事实**（K-11）：qos_manager 仓（ConcurrentTaskService）提供**线程级 QoS**（`QosController::SetThreadQosForOtherThread/ResetThreadQosForOtherThread`，QosLevel：QOS_BACKGROUND~QOS_USER_INTERACTIVE）与 RTG 线程组（AddThreadToProcRtg）；全树无 "sessionQos" 接口。RSS 侧另有**进程级**通道：`RES_TYPE_PROCESS_QOS_CHANGE=221`、`RES_TYPE_SET_BACKGROUND_PROCESS_PRIORITY=134`、`RES_TYPE_THREAD_QOS_CHANGE=23`、`RES_TYPE_CGROUP_ADJUSTER=8`。

**借用评估**：资源调度子模块将"管控动作"抽象为三类，QosManager 与冻结豁免互补：
1. **冻结豁免/解冻**（一期核心，对应需求痛点）：RSS 505/517/506/507/518，进程级；
2. **QoS 提权/恢复**（CPU 优先级维度，需求 1 提及的"内存CPU优先级"）：进程级走 RSS `RES_TYPE_PROCESS_QOS_CHANGE`（221，无需线程信息）；线程级走 QosController/RTG（需线程 tid，需目标进程协作或 /proc 遍历，复杂度高）→ 一期预留接口与策略扩展点，实现后置；
3. **恢复语义**：Reset（ResetThreadQos）/撤销豁免天然支持需求 12"降级回原状态"；多任务归属仍按最高档合并。

**结论**：QosManager 机制可借用，但建议定位为二期提权扩展；一期以进程级冻结豁免为主线。**"sessionQos" 具体所指（若来自其他仓/版本/文档）请需求方给出出处，再核对适配性。**

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 仓 AGENTS.md / CLAUDE.md | 目录职责、约束与边界 | `agent_runtime_framework/`（agentmgr，SA 185）标记 🟢；`services/abilitymgr/`、`services/appmgr/` 标记 🔴；约束：不绕过 PermissionVerification、不直接调 appmgr 内部方法（经 inner_api）、新文件必更新 BUILD.gn、DFX 事件定义在 hisysevent.yaml、特性标志在 ability_runtime.gni | 高 | 范围/设计 | 命中 |
| K-2 | 源码：agent_runtime_framework/services/agentmgr | 架构勘察 | AgentManagerService 继承 SystemAbility，SA ID=185，foundation 进程，IAgentManager.idl 生成 stub/proxy（15 接口）；AMS 经 AgentManagerClient 同进程调 agentmgr（VerifyAgentConnectRequest，ability_manager_service.cpp:6085）；agentmgr 未依赖 ressched_client | 高 | 模块形态/API/构建 | 命中 |
| K-3 | 源码：services/abilitymgr | AMS 启动链路拦截点 | StartAbility（ability_manager_service.cpp:731）→ StartAbilityInner；AbilityRecord 创建与 pid 获取（ability_connect_manager.cpp:327 `AbilityRecord::GetPid()`、AttachAbilityThread:777）；启动/连接全部收敛于 AMS | 高 | 纳管拦截点设计 | 命中 |
| K-4 | 源码：resource_schedule_service/ressched/interfaces/innerkits/ressched_client | RSS 客户端接口 | `ResSchedClient::ReportData(resType, value, mapPayload)`（res_sched_client.h:69）、`ReportSyncEvent`（:79）；res_type.h：`RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT=505`、`SYNC_RES_TYPE_GET_SUSPEND_STATE_BY_UID/PID=506/507`、`SYNC_RES_TYPE_THAW_ONE_APP=517`、`SYNC_RES_TYPE_GET_ALL_SUSPEND_STATE=518` | 高 | RSS 策略子模块 | 命中 |
| K-5 | 源码：resource_schedule_service/ressched/services | 豁免冻结处理 | `RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT` 已入 RSS 服务端白名单（res_sched_service.cpp:175），可被平台插件处理；RSS 进程管理规范：新增插件需评审内存基线 → 一期复用现有 resType 避免新增插件 | 高 | 集成方式 | 命中（payload 语义待 RSS owner 确认） |
| K-6 | 源码：services/appmgr | AppMS 结构与 RSS 依赖 | AppRunningManager/AppRunningRecord/AppStateObserverManager 等 20+ 子模块；BUILD.gn:159 已依赖 `resource_schedule_service:ressched_client` | 高 | 方案 A-2 分析 | 命中 |
| K-7 | 仓内 .codespec 样例 | issue-15382-cli-shell/proposal.md | 仓内 proposal frontmatter 与章节惯例（target_release/change_type/issue/author/date/status；影响范围；不涉及项确认） | 高 | 本文档格式 | 命中 |
| K-8 | ADK（google-adk） | 需求方语义参考 | 本地 ai 仓无 invocation 实现代码（`foundation/ai/` 下无匹配） | 中 | 任务模型 | 未命中（无对应代码） |
| K-9 | google-adk 官方文档（两轮调研） | https://github.com/google/adk-docs 、https://adk.dev/context/index.md 、DeepWiki adk-python/adk-java/adk-go | 架构参考点（第二轮深入）：① **InvocationContext 双层作用域**：InvocationContext（每次 invocation 单例，共享状态/服务/事件队列）+ Context（每节点执行上下文，1:1 节点树，createChildContext 继承 invocationId）→ 对应任务上下文与子任务/分支（branch 概念 agent_1.agent_2）；② **end_invocation 共享 kill switch**：子上下文设置可终止父执行（TS 为共享标志）→ 对应任务取消语义（任务取消→纳管进程联动）；③ **HITL 三特殊函数**：`adk_request_credential`（认证）、`adk_request_confirmation`（确认）、`adk_request_input`（输入），通过 `long_running_tool_ids` + `should_pause_invocation()` 挂起 invocation，用户响应后按 invocation_id 过滤历史事件重建状态恢复 → 与本需求 INPUT_REQUIRED/AUTH_REQUIRED 挂起-回传-恢复机制一一对应；④ **Cost limits**：max_llm_calls 防无限循环/资源滥用 → 对应任务数量/豁免时长上限（防滥用）；⑤ **SessionService 抽象**（create/get/append_event/delete_session 可插拔持久化）→ 对应任务存储抽象；⑥ **Invocation identity**：invocation_id 格式 e-{UUID}；⑦ **A2A**：远程 agent 间不共享 session，supervisor 提取状态参数透传（对应 A2A 链路 taskId 传递）；多 agent 模式（Sequential/Parallel/RemoteA2a） | 高 | 任务模型/状态机/交互设计 | 命中 |
| K-10 | 源码：services/sa_profile/501.json、services/appmgr/BUILD.gn | 进程归属与依赖 | AppMS（SA 501）运行于 foundation 进程（与 AMS 180、agentmgr 185 同进程），进程位置非取舍关键；appmgr 已依赖 `resource_schedule_service:ressched_client`（BUILD.gn:159） | 高 | 模块形态/构建 | 命中 |
| K-11 | 源码：foundation/resourceschedule/qos_manager | QosManager 能力全貌 | qos_manager 仓为 ConcurrentTaskService（SA）：**线程级 QoS**（`QosController::SetThreadQosForOtherThread/ResetThreadQosForOtherThread`，QosLevel：QOS_BACKGROUND~QOS_USER_INTERACTIVE，qos.h:21-36）与 **RTG 线程组**（AddThreadToProcRtg/RemoveThreadFromProcRtg）；**无 "sessionQos" 字样**（qos_manager/RSS/ability_runtime 全树搜索未命中）；RSS 另有**进程级通道**：`RES_TYPE_PROCESS_QOS_CHANGE=221`、`RES_TYPE_SET_BACKGROUND_PROCESS_PRIORITY=134`、`RES_TYPE_THREAD_QOS_CHANGE=23`、`RES_TYPE_CGROUP_ADJUSTER=8` | 高 | 资源调度子模块 | 命中（"sessionQos"具体所指待需求方确认） |

**上下文结论：**
- 高可信结论：**AppMS 收口**——进程权威信息在 AppMS（所有应用进程必经孵化/attach/死亡），RSS 交互通道现成（BUILD.gn:159 已依赖），与 AMS 同 foundation 进程，AMS 拦截点经 app_manager inner_api 客户端调用符合 AGENTS.md 约束；RSS 侧复用 505/517/506/507/518 现有 resType；google-adk 概念可映射：Invocation→任务（invocation_id=taskId）、request_credential→AUTH_REQUIRED 挂起、Event→状态监听、A2A 参数透传→taskId 传递。
- 待确认结论：DYNAMICALLY_SET_SUSPEND_EXEMPT 的 payload 语义与豁免覆盖范围（是否覆盖锁屏冻结的管控组成）需 RSS owner 确认；调用方身份校验粒度待需求方确认；任务类型/优先级枚举初稿待设计细化。
- 未使用来源及原因：RSS 多仓知识库无访问权限；google-adk 本地无源码（K-8），以官方文档为准（K-9）。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | ability（ability_runtime：AppMS 新模块 + AMS 拦截点 + inner API）；resourceschedule（仅接口语义确认，不改代码） | yangzk | 已确认 |
| 是否需要新增子系统或部件？ | 否。模块并入既有 appms 部件（libappms.z.so），不新增 SA/部件 | yangzk | 已确认 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | yangzk | 已确认 |
| 是否需要新增 System API？ | 否（一期） | yangzk | 已确认 |
| 是否会废弃已有 API？ | 否 | yangzk | 已确认 |
| 是否需要新增权限声明？ | 是：基础身份校验（系统应用/SA 调用方）在 AppMS 侧入口校验（复用既有身份校验能力），不新增权限项（细则属安全管控，本需求范围外） | yangzk | 待设计确认 |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | 全部新增 inner API 与新增模块，无既有行为变更；启动链路仅在 Want 携带 taskId 参数时附加处理，无 taskId 的启动完全走原路径 | yangzk | 已确认 |
| 性能 | 响应时间/内存/并发要求？ | 任务创建/状态流转 ≤ 5ms（同进程）；纳管拦截附加开销无感知（不阻塞启动主流程）；支持多任务并发（≥16 并发任务设计基线）；内存：任务记录轻量（≤1KB/任务） | yangzk | 待设计验证 |
| 安全 | 权限/隐私/加密/审计要求？ | 本需求明确排除安全管控细则；接口做基础调用方校验（拒绝非系统应用）；taskId 仅用于进程纳管映射，不涉及隐私数据 | 需求方 | 已确认 |
| 可靠性 | 崩溃率/容错/恢复要求？ | AppMS 崩溃（foundation 进程异常）后任务记录丢失可接受（进程管控自动失效，任务由调用方重新创建）；纳管进程死亡需自动清理任务进程映射并降级 | yangzk | 已确认 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| `resource_schedule_service:ressched_client` | 编译 | appmgr BUILD.gn 已具备该依赖（BUILD.gn:159，无条件依赖；第十四轮实证——早期"resource_schedule_service_enable 条件"表述已过时），无需新增 | 已确认 |
| RSS suspend exempt/thaw 接口语义 | 运行 | payload 约定、豁免覆盖范围（锁屏/standby 冻结）需 RSS owner 评审确认 | 待确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| RSS 接口 payload 语义与豁免范围不符预期（尤其锁屏冻结场景） | 技术 | 高 | 设计阶段先与 RSS owner 对齐语义；一期以现有 resType 落地，异常场景降级为"不豁免"，不影响原有调度 | 待确认 |
| 冻结豁免被滥用导致资源管控失效 | 安全/技术 | 中 | 仅系统应用可创建任务；任务数量上限；高优先级任务豁免有活跃/时长限制（可配）；审计 hisysevent | 待设计 |
| AMS 启动热路径拦截引入额外延迟/异常 | 性能 | 中 | 拦截点只做参数解析与同进程 IN_PROCESS_CALL，失败不阻断启动（降级为不纳管）；全链路无 taskId 时零开销 | 待设计 |
| 与背景任务/standby/冻结策略联动时语义冲突（如设备熄屏深度休眠） | 技术 | 中 | 一期仅覆盖 RSS 冻结豁免场景，深度休眠场景在验证中确认边界 | 待验证 |

### AC 完整性

- [x] 每个用户故事有验收标准（见基线部分 US-1~US-6 + AC-1~AC-16）
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

**结论:** 条件通过（需求方已确认：① 模块形态取 A-2 **AppMS 收口 + 通用进程调度能力抽象**（第二轮拍板）；② 目标发行版本 **7.1**。剩余待确认：③ 任务类型/优先级枚举初稿（设计阶段细化）；④ 锁屏豁免场景边界与 RSS owner 对齐结果）

---

## 三、需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-21 |
| Owner | yangzk |
| 确认人 | 需求方（AI 语音助手团队，2026-08-21 两轮确认）、ability_runtime 模块 Owner（待确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | 7.1（已确认） |
| 版本状态 | proposed |

### 问题陈述

AI 语音助手在锁屏定时任务与后台长时任务场景下，任务进程因 RSS 冻结/资源管控而执行失败或中断。系统缺少一个可信的"任务"级维度来表达 AI 助手对后台持续执行的需求。需要提供一套任务管理机制：AI 助手创建任务（指定优先级、类型、状态），任务执行涉及的进程通过启动链路自动纳入任务，任务管理模块按任务优先级对进程施加 RSS 管控（冻结豁免、解冻、结束后降级），以状态机管理任务生命周期，支持状态查询/监听（系统 UI 展示进度）与 INPUT_REQUIRED/AUTH_REQUIRED 的人机交互传达，并提供 DFX 手段。安全管控问题不在本需求范围。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 锁屏定时任务可执行完成 | 锁屏场景下高优先级任务进程不被冻结，任务回归用例通过（成功率基线由需求方给出，默认 100% 于回归用例） | 真机场景用例（锁屏 + 定时任务） |
| 后台长时任务不被冻结中断 | 后台切换后任务进程不被 RSS 冻结，任务完成 | 真机场景用例（前后台切换 + 长时任务） |
| 任务级管控对原有调度无影响 | 无 taskId 的启动/进程调度行为与改动前完全一致 | 全量回归 + 性能对比 |
| 多任务并发与进程多任务归属正确 | 进程按最高优先级任务管控；任务结束降级正确（单测覆盖多任务/多进程矩阵） | 单测 + 集成用例 |
| 任务状态可视化与交互传达 | 系统 UI 能监听任务状态并展示 INPUT/AUTH 交互 | 系统 UI 联调用例 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为 AI 语音助手，我想要创建任务（指定类型、优先级）并管理其生命周期（设置状态、销毁、取消），以便表达任务执行诉求并控制任务节奏 | P0 |
| US-2 | 作为 AI 语音助手，我想要通过 StartAbility/ConnectAbility/意图/Cli/A2A 启动能力时自动携带 taskId 并把目标进程纳入任务，以便任务级管控进程 | P0 |
| US-3 | 作为系统 UI，我想要监听任务状态变化并查询任务详情，以便展示任务执行进度 | P1 |
| US-4 | 作为 AI 语音助手和用户，我想要 INPUT_REQUIRED/AUTH_REQUIRED 阶段通过任务传达描述与可选项、回传用户选择，以便完成授权/意图确认 | P1 |
| US-5 | 作为任务管理模块，我想要按任务优先级对纳管进程施加 RSS 分级管控（冻结豁免/解冻/降级），以便在锁屏/后台场景保障任务执行且结束后恢复 | P0 |
| US-6 | 作为系统开发者，我想要任务管理具备 DFX（状态 dump、hisysevent），以便排查任务执行问题 | P2 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN AI 助手调用 CreateTask(类型、优先级、可选 taskId) THEN 返回成功且任务以 UNKNOWN 状态存在，可查询到任务记录 | 正常 | US-1 |
| AC-2 | WHEN AI 助手对存在的任务调用 SetTaskState(合法流转，如 SUBMITTED→WORKING) THEN 状态更新成功且已注册监听者收到状态回调 | 正常 | US-1 |
| AC-3 | WHEN 调用方调用 DestroyTask/CancelTask THEN 任务进入终态（COMPLETED 由助手置/CANCELED/FAILED），纳管进程全部降级，任务记录清理 | 正常 | US-1 |
| AC-4 | WHEN 调用 SetTaskState 发起非法流转（终态回退、未创建任务、REJECTED 由非创建方设置）THEN 返回明确错误码且任务状态不变 | 异常 | US-1 |
| AC-5 | WHEN 任务已存在且通过 StartAbility/ConnectAbility/意图框架/Cli/A2A 任一链路启动、Want 携带 taskId THEN 目标进程自动注册进该任务 | 正常 | US-2 |
| AC-6 | WHEN 同一进程被多个任务纳管 THEN 该进程按其中最高优先级任务执行 RSS 管控 | 边界 | US-2 |
| AC-7 | WHEN 系统 UI 注册任务监听 THEN 任务状态变化（含进入 INPUT_REQUIRED/AUTH_REQUIRED 的 payload）实时回调，卸载后不再回调 | 正常 | US-3 |
| AC-8 | WHEN 查询任务列表或任务详情 THEN 返回状态、纳管进程（pid）、payload、创建/更新时间戳 | 正常 | US-3 |
| AC-9 | WHEN 任务进入 INPUT_REQUIRED/AUTH_REQUIRED 且携带描述与可选项 THEN 系统 UI 收到并可展示，用户提交选择后通过 SubmitTaskInput(taskId, input) 回传，AI 助手可获取 | 正常 | US-4 |
| AC-10 | WHEN 任务在 INPUT/AUTH 阶段被取消/销毁 THEN 交互状态清理且不再回调，状态机进入终态 | 异常 | US-4 |
| AC-11 | WHEN 高优先级任务进入工作状态（如 WORKING）THEN 纳管进程冻结豁免生效（经 RSS DYNAMICALLY_SET_SUSPEND_EXEMPT），锁屏/后台场景进程不被冻结 | 正常 | US-5 |
| AC-12 | WHEN 任务结束/取消/销毁 THEN 纳管进程豁免撤销，恢复任务前的 RSS 调度状态（多任务归属时按次高优先级任务继续管控） | 正常 | US-5 |
| AC-13 | WHEN 低优先级任务或任务处于非工作状态 THEN 不豁免或按配置限时豁免，不长期占用管控资源 | 边界 | US-5 |
| AC-14 | WHEN 纳管的进程死亡 THEN 该进程从任务记录移除，任务状态不受影响（或按调用方设置 FAILED） | 异常 | US-2 |
| AC-15 | WHEN 执行 aa dump/hidumper 任务相关命令 THEN 输出任务列表、状态、纳管进程、策略生效信息 | 正常 | US-6 |
| AC-16 | WHEN 任务创建/状态流转/结束 THEN 上报 hisysevent 行为事件，可检索 | 正常 | US-6 |

### 范围边界

**包含：**
- 任务模型与生命周期：CreateTask/DestroyTask/CancelTask/SetTaskState/SetTaskParams（优先级、类型、payload）
- 启动链路拦截纳管：StartAbility/ConnectAbility/意图框架/Cli/A2A 收敛点解析 Want taskId（参数 key 约定如 `ohos.ai.taskId`，设计定稿）
- 状态机：UNKNOWN/SUBMITTED/WORKING/INPUT_REQUIRED/AUTH_REQUIRED/COMPLETED/CANCELED/FAILED/REJECTED + 流转校验
- 查询/监听：任务列表/详情查询、状态监听（含 payload 事件）
- 交互传达：INPUT_REQUIRED/AUTH_REQUIRED 描述+可选项下发、SubmitTaskInput 回传
- RSS 资源调度子模块：按优先级分级（冻结豁免、解冻、状态查询、结束降级），复用 RSS 现有接口，策略可配，预留扩展点
- DFX：hisysevent（hisysevent.yaml 新增事件）、hidumper/dump、TAG_LOGI 日志
- 进程纳管数据：taskId↔pid 映射、多任务归属（按高优先级）、进程死亡自动清理

**不包含：**
- 安全管控细则、Public/System API、NAPI（预留）、任务持久化/跨设备、定时调度、新增 RSS 插件/resType

**分期策略（如适用）：**

| 阶段 | 范围 | 不做 | 进入条件 |
|------|------|------|----------|
| 一期（本需求） | 上述"包含"全部 | NAPI 开放、RSS 专用接口、CPU/内存优先级提权细化 | 基线确认 |
| 二期（预留） | NAPI 绑定开放、RSS 新增任务级专用接口（提权/优先级细化） | — | 一期落地后与 RSS/应用框架对齐评审 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | ability_ability_runtime | `services/appmgr/src/task_mgr/`（新增） | — | 新增：任务管理模块（任务模型、状态机、inner API 服务、监听、交互传达） | yangzk |
| ability | ability_ability_runtime | `services/appmgr/src/process_schedule/`（新增） | — | 新增：通用"App 进程调度管理"能力（进程组纳管、优先级档、RSS 管控施加/撤销、死亡清理、多组归属合并） | yangzk |
| ability | ability_ability_runtime | `services/appmgr/`（既有热路径） | 进程生命周期管理 | 最小挂点：复用既有进程死亡/状态通知链（AppStateObserverManager/AppDeathRecipient）接入新模块，不修改既有接口与序列化 | yangzk |
| ability | ability_ability_runtime | `interfaces/inner_api/app_manager/` | 内部组件接口 | 新增：ITaskManager IDL（stub/proxy）+ C++ 客户端（AI 助手/系统 UI/AMS 使用）；不扩 IAppMgr 既有接口 | yangzk |
| ability | ability_ability_runtime | `services/abilitymgr` | AMS 启动/连接调度 | 修改：启动/连接收敛点解析 Want taskId，经 app_manager inner_api 客户端转发 AppMS（最小改动，拦截失败不阻断启动） | yangzk |
| ability | ability_ability_runtime | `hisysevent.yaml` / `services/appmgr/BUILD.gn` / `ability_runtime.gni` | DFX 事件定义/构建 | 新增：任务事件定义；BUILD.gn 新增源文件（ressched_client 依赖已具备） | yangzk |
| resourceschedule | resource_schedule_service | `ressched`（接口层） | RSS 管控 | 不改代码；RSS 具体设计由 RSS 团队主导（暂未明确），本模块经 ResourceControlProxy 资源管控代理层对接（设计 §6.4） | RSS 团队 |

### API 变更项清单

> 涉及 inner API 变更，作为 design.md 与 spec.md 的共享锚点。

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| ITaskManager（IDL 新增，放 `interfaces/inner_api/app_manager/task_manager/`，方法以接口定义定稿为准） | 新增 | InnerAPI | CreateTask / DestroyTask / CancelTask / SetTaskState / SetTaskParams / GetTaskInfo / GetTaskInfos / RegisterTaskListener / UnregisterTaskListener / SubmitTaskInput / （内部：RegisterTaskIntent/RegisterTaskProcess/UnregisterTaskProcess 供 AMS 拦截点调用；RegisterTaskProcess 兼创建者显式自注册，design §5.2.1；F-20 补全） |
| TaskManagerClient（C++ 客户端） | 新增 | InnerAPI | AI 助手/系统 UI/AMS 使用的客户端封装 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 是 | 启动热路径拦截、纳管开销、并发任务数 | design.md / spec.md |
| 安全与权限 | 是（仅基础鉴权） | 安全管控细则明确不在范围；接口需拒绝非系统应用调用 | design.md |
| 兼容性 | 是 | 全新增 inner API 与模块，无既有行为变更；无 taskId 启动零影响 | spec.md |
| API/SDK | 是 | 新增 inner API（IDL），无 Public/System | design.md / spec.md |
| IPC/跨进程 | 是 | IAgentTaskManager IPC（stub/proxy）、AMS 同进程调用 | design.md |
| 构建与部件 | 是 | appmgr BUILD.gn 新增源文件与 IDL 生成（ressched_client 依赖已具备） | design.md |
| 国际化/无障碍 | 否（N/A） | 任务机制无 UI 文案，系统 UI 展示侧归 UI 团队 | — |
| 数据迁移 | 否（N/A） | 任务不持久化，进程退出即清理 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 inner API 方法或参数 | 触发设计审批 |
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
- [x] 标准及以上复杂度的澄清问题已逐项关闭，讨论记录包含需求方明确确认（2026-08-21 两轮）
- [x] 上下文与知识源检索日志已填写；未查询关键来源的原因已记录
- [x] 目标仓 Agent 指南已检查并记录关键约束（AGENTS.md：services/appmgr 🔴 高风险需最小挂点、不直接调 appmgr 内部方法需经 inner_api 客户端、BUILD.gn 必更新、DFX 定义 hisysevent.yaml）

**基线结论:** 通过 —— 需求方 2026-08-21 四轮确认全部关键决策：模块形态（A-2 AppMS 收口 + 通用进程调度能力）、目标版本（7.1）、防仿冒（B-1+B-2）、RSS 集成（复用现有接口）、RSS 对接方式（**RSS 团队主导设计，本模块封装 RSS Proxy 代理层**，Q-12）、纳管方式（Want 统一拦截）、接口面向（inner API）、状态机驱动（助手驱动+模块校验）、交互机制（状态回调携带 payload）。基线成立，开放项随设计/验证阶段关闭：
1. 任务类型/优先级枚举初稿（design.md 细化，随设计评审确认）；
2. 锁屏豁免场景边界（**搁置**，不阻塞基线；真机验证阶段与 RSS owner 对齐后确认）；
3. RSS 管控维度细化（冻结/CPU 调度/内存/低内存查杀/功耗/温控，design.md 分析各维度接口与一期/二期取舍）；
4. sessionQos 机制（design.md 探索）；
5. **RSS 真实对接**（RSS 设计明确后由 ResourceControlProxyImpl 承接，design.md §6.4 对接策略）；
6. **枚举对齐**（任务类型/优先级/状态枚举初稿与 AI 语音助手团队对齐，评审后确认）。

**锁屏豁免场景边界（解释，供设计/验证阶段参考）：** "边界"指设备锁屏后应用进程面临的**冻结管控的组成与豁免接口的覆盖范围**：1) RSS suspend 冻结（按前后台/空闲判定，`RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT` 主要豁免此类）；2) standby（待机）深度冻结（设备静置进入待机管控，豁免是否覆盖待与 RSS owner 确认）；3) 豁免的时长/数量上限与调用方资质约束。该边界决定 AC-11（锁屏定时任务进程不被冻结）的可达成性与实现范围。
