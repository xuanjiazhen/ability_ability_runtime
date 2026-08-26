# 方案深度审查报告（2026-08-22 第七轮；2026-08-26 第十三轮自审追加；2026-08-26 第十四轮代码实证复核追加）

> 对 proposal / spec / design / execution-plan 的逐项审查。审查方法：对照需求 13 条、已确认决策（六轮）、AGENTS.md 约束、既有源码事实，逐项核查设计完整性。

## 审查结果总览

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-1 | 状态语义 | 状态激活边界不一致：AC-5.3 将 SUBMITTED 列为"非工作状态"，而冻结豁免在 WORKING 即生效——**SUBMITTED（任务就绪/触发）但尚未 WORKING 的锁屏定时任务，其进程可能已被冻结，无法启动执行任务** | 高 | 已修正：激活态（档位≥1）扩展为 {SUBMITTED, WORKING, INPUT_REQUIRED, AUTH_REQUIRED}；UNKNOWN 保持非激活 | 已修正（spec R-21/AC-5.3、design 6.1） |
| F-2 | 进程归属 | 原设计仅高优先级管控，**未明确：已纳管进低优先级任务的进程是否应防 RSS 干扰**（低优先级任务切后台同样会被冻结，导致任务失败） | 高 | 已修正：LOW 升级为"基础保障档"——切后台/锁屏事件时经 RCP 施加解冻（517）+ 低等级豁免，不长期占用；与需求 11"按更高优先级处理"合并档位语义不冲突（仍按 max 合并，不同档位强度不同） | 已修正（design 6.1、spec R-21） |
| F-3 | 归属收敛 | **同进程内新建 ability/连接未携带 taskId 时任务归属歧义**：拦截点仅按启动请求处理，该 ability 是否算任务内、应不应该被管控 | 中 | 已修正设计原则：**以进程为纳管单位**——进程一旦纳管，其内全部组件活动共享任务管控档位（无需每组件单独纳管）；taskId 仅用于"进程加入任务"的触发点，组件级归属不细分（需求 11 以进程粒度管控） | 已修正（design 5.6） |
| F-4 | 可信度 | **豁免/解冻后无生效校验**：RSS 下发后未验证管控是否真正生效；锁屏定时任务若豁免被 RSS 内部策略拒绝，任务仍失败且系统不知 | 高 | 已修正：RCP 增加**生效校验环**——施加后查询冻结状态（SYNC_RES_TYPE_GET_SUSPEND_STATE_BY_PID 507）；未生效 → 上报 TASK_ABUSE_WARNING + 按策略处理（对 AI 助手为 TASK 事件告警，供其侧决策重试/降级） | 已修正（design 6.4、spec R-19/R-20） |
| F-5 | 交互时序 | **INPUT/AUTH 状态与进程冻结的时序冲突**：交互挂起期间（等用户输入）进程管控应维持，原设计仅"工作态豁免"未覆盖交互态时长管理（用户可能长时间不响应） | 中 | 已修正：交互态属激活态（F-1 已含）；增加**交互超时配置**（可配，默认 30min）——超时由 task_mgr 主动流转（助手可重设状态）或保持豁免并告警，避免无限挂起占用豁免 | 已修正（design 6.1、spec R-16） |
| F-6 | 变化响应 | **任务激活态优先级提升未及时生效**：SetTaskParams 提升优先级（LOW→HIGH）且任务处于激活态时，管控档位应立即提升 | 低 | 已修正：SetTaskParams 触发 OnTaskActiveChanged → process_schedule 档位重算（与状态变更同一联动路径，已存在） | 已修正（design 5.6） |
| F-7 | 异常链路 | **AMS 拦截点 ON_START 通知后 attach 失败的残留**：①已发 RegisterTaskIntent，但②attach 失败（进程启动失败）→ 意向残留 30s | 低 | 已修正：意向超时 30s 自动清理 + attach 失败时主动 UnregisterTaskIntent（清理挂点，复用既有 attach 失败回调） | 已修正（design 5.4.2） |

## 此前已覆盖（前六轮闭环，本次确认）

| 编号 | 已覆盖项 | 覆盖位置 |
|------|----------|----------|
| C-1 | 已存在进程两段式拦截缺陷 | 条件两段式（design 5.4.2/ADR-3） |
| C-2 | 代理启动身份规则（AsCaller/WithSpecifyTokenId） | 业务发起方身份优先（design 5.4.1 场景 3/4、§5.3.1 面⑥） |
| C-3 | taskId 防仿冒（B-1+B-2 + 各仿冒面分析） | design 5.3/5.3.1 |
| C-4 | 启动链路 12 场景全覆盖 | design 5.4.1 |
| C-5 | 任务结束 RSS 清理 | 双通道 NotifyTaskEnd（design 6.4、spec R-20） |
| C-6 | RSS 对接方式（代理层） | ResourceControlProxy（design 6.4） |
| C-7 | 多任务并发与进程多任务归属 | 档位合并 max（design 5.6/ADR-5） |
| C-8 | 进程死亡清理 | 死亡通知链（design 5.6、spec R-12） |

## 开放项（设计无法单方闭环，需对齐/评审）

| 编号 | 开放项 | 依赖方 | 关联 |
|------|--------|--------|------|
| O-1 | 任务类型/优先级/状态枚举最终对齐 | AI 语音助手团队 | design 4.1；对齐材料 docs/ai-assistant-alignment.md |
| O-2 | 锁屏豁免场景边界（豁免覆盖哪些冻结类型、时长/数量上限） | RSS 团队 | design §6 开放项；docs/rss-alignment.md P2 |
| O-3 | RSS 真实对接（ResourceControlProxyImpl 落地） | RSS 团队 | design 6.4；docs/rss-alignment.md P1 |
| O-4 | 子进程自动纳入（F-3 扩展场景 12 是否纳入一期） | 需求方确认 | design 5.4.1 |
| O-5 | 520 低内存查杀豁免消费方语义 | RSS 团队 | design 6.2 二期；docs/rss-alignment.md #12 |
| O-6 | RSS 505 payload 语义与豁免覆盖范围（RCP-Impl 对接前必须确认） | RSS 团队 | design 6.2/6.4；docs/rss-alignment.md P0（含 507 GET_SUSPEND_STATE 权限确认，F-24） |

## 结论

经第七轮审查，新发现 7 个问题点（F-1~F-7）已全部在设计文档中修正（见上述"处理"列对应章节），无新增"设计无法闭环"的阻塞问题；方案覆盖需求 13 条、六轮决策与 AGENTS.md 约束。剩余开放项 O-1~O-5 均需外部对齐（AI 助手团队/RSS 团队），不阻塞模块级开发（ResourceControlProxyStub 骨架先行）。

---

## 第十三轮自审（2026-08-26）

> 审查方法：全量通读 design/spec/proposal/execution-plan + 关键源码事实复核（拦截点收敛、SetPid 路径、IStandbyService 接口、sysparam 惯例、res_sched_util payload 先例）。发现接第七轮编号 F-8~F-20，均已直接修复至 design/spec/plan/proposal。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-8 | 安全 | **RegisterTaskIntent/RegisterTaskProcess 通道鉴权缺失**：callerTokenId 为 IPC 入参（非通道派生），ITaskManager 为可直调 IPC 接口——任意系统应用可自报 callerTokenId=creatorTokenId 绕过 B-2（§5.3.1⑧"恶意应用无法直接触达内部通道"断言需显式校验支撑）；且 §5.2.1 助手自注册依赖直调，与"仅内部通道"矛盾；直调路径亦无 pid 归属校验（可注册他人进程骗取豁免） | 高 | **通道鉴权四规则**（design §5.3）：① IPC 调用方为 foundation native → 信任入参 callerTokenId；② 非 native → 入参 callerTokenId 必须等于 IPC 实际调用方 tokenId；③ 直调路径 pid 归属校验（uid 一致或 pid 已在任务内）；④ 不满足返回 ERR_TASK_PERMISSION_DENIED（不透传应用侧）。§5.2.1/§5.3.1⑧/§5.7、spec RegisterTaskIntent/RegisterTaskProcess 接口规格同步 | 已修正 |
| F-9 | 功能缺口 | **payload 更新无通知通道**：R-14 仅状态变更回调，SetTaskParams 的 payload-only 变更无任何回调——US-3"展示执行进度"实时性落空（UI 只能轮询 GetTaskInfo） | 高 | 新增 noticeType **PAYLOAD_UPDATED**：SetTaskParams payload 变化经 OnTaskNotice 派发已注册监听者（design §5.5/§5.6、spec AC-3.1/SetTaskParams 行为场景同步） | 已修正 |
| F-10 | 规格一致性 | spec 内部矛盾两处：① API 变更分析表 SetTaskParams"优先级/类型可调"与接口规格"type 创建后不可调"矛盾；② RegisterTaskProcess 接口规格签名 2 参（缺 callerTokenId），与 design IDL 3 参不一致 | 中 | ① 改为"可调字段仅 priority/payload"；② 签名补 callerTokenId（含参数约束与通道鉴权行为场景） | 已修正 |
| F-11 | 设计精确性 | 拦截点①位置表述矛盾（§5.4.1"参数解析处" vs §5.4.2"获得 AbilityRecord 后"）；勘察补强：`AbilityRecord` pid **仅在 attach 时设置**（SetScheduler 内 SetPid(GetCallingPid())，ability_record.cpp:1096，全库唯一路径）——"进程已存在直接纳管"仅对既有 record 成立，热启动新实例走意向路径 | 中 | §5.4.1 场景 1 精确为"Want 解析于入口、分支判定于获得 record 后"；§5.4.2 补分支①适用边界与 file:line 证据（两路径全覆盖无遗漏） | 已修正 |
| F-12 | NFR 矛盾 | 任务记录 ≤1KB（design 4.3/spec NFR）与 payload/input 各 ≤4KB 矛盾（合计可达 ~9KB） | 中 | 修正为"基础字段 ≤1KB，payload/input 变长字段各 ≤4KB 另计，单任务记录上限 ~9KB"（design 4.3/spec NFR 同步） | 已修正 |
| F-13 | 文档错位 | design"后续 Task 拆分"表与 execution-plan TASK 编号错位（design T2=进程调度/T4=监听交互 vs plan T2=监听交互/T4=管控RCP）；骨架表 TASK-SKELETON-1 vs plan TASK-0 | 中 | design 两表对齐 execution-plan 编号（TASK-0~6）并注明以 plan 为准 | 已修正 |
| F-14 | 落地机制 | "CCM"术语未映射本仓参数体系（本仓无 CCM 实现；惯例为 sysparam：.para 定义+GetParameter 读取，先例 appfwk.para / app_running_manager.cpp:1145）；execution-plan 文件清单缺参数定义文件 | 中 | design §6.1 明确参数落地机制（sysparam，命名 const.sys.appms.taskmanager.*）；execution-plan 全量清单与 TASK-4 Files 补 `services/appmgr/etc/taskmanager.para`；spec R-21/R-22 措辞同步 | 已修正 |
| F-15 | 结构归属 | TaskRecord.exemptRefCount 归属错误：R-11 幂等键=taskId+pid+豁免类型（per-pid），单一 TaskRecord 字段无法表达多进程计数 | 中 | 从 TaskRecord 移除；幂等引用计数按 pid 维护于 process_schedule 档位状态（design 4.3/5.6、plan TASK-4、module_design.svg 同步） | 已修正 |
| F-16 | AC 措辞 | AC-1.4"任务记录不可再查询"对 CancelTask 不成立（CANCELED 终态记录保留至 DestroyTask 仍可查询） | 低 | AC-1.4 措辞区分：CancelTask 后记录保留至 DestroyTask；DestroyTask 后不可查询 | 已修正 |
| F-17 | 字段完整性 | R-19 豁免登记字段列表漏 taskId（D-16 定义含 taskId） | 低 | R-19 补 taskId（对齐 D-16 字段集） | 已修正 |
| F-18 | 行为完备性 | 监听重复注册/注销未注册行为未定义；SubmitTaskInput 多次提交覆盖语义未定义 | 低 | spec 补幂等语义：重复注册幂等（单份派发）、注销未注册幂等 ERR_OK；input 多次提交以最后一次为准（覆盖） | 已修正 |
| F-19 | 清单准确性 | TASK-0"2 个 struct"——design §5.2 实为 3 个 struct（TaskCreateParams/TaskParams/TaskInfo）+ ITaskListener | 低 | 修正为"13 方法 + 3 struct + ITaskListener（task_listener.h 手写）" | 已修正 |
| F-20 | 锚点完整性 | proposal API 变更项清单（design/spec 共享锚点）漏 RegisterTaskIntent（第十二轮条件两段式新增） | 低 | 清单补 RegisterTaskIntent（含自注册说明） | 已修正 |

### 术语残留清理（随本轮）

design.md 与 task_flow.svg 中"动作记账/施加记账/记账"活描述 5 处统一为"内存登记"（D-16 术语，历史决策记录行保留原文）。

### 待定项确认（用户 2026-08-26）

N=3 续期次数上限、INPUT_ARRIVED 枚举命名、交互 30min/宽限 5min/档位1限时 30min/硬上限 60min/预警提前量 5min 默认值——**全部确认保留**（design §6.1/spec R-21 同步标注"已确认"）。

### 结论

第十三轮自审发现 13 项（F-8~F-20：高 2 / 中 6 / 低 5），已全部修复至交付件；其中 F-8 通道鉴权为安全闭环必改项（落地时 TASK-3 需覆盖通道鉴权用例：非 native 自报身份拒绝、pid 归属不符拒绝）。开放项 O-1~O-6 维持不变（外部对齐），不阻塞模块级开发。

---

## 第十四轮：代码实证复核（2026-08-26）

> 审查方法：以 ability_runtime 与 resourceschedule **实际代码逐项验证文档断言**（拦截链路收敛性/SA 承载机制/RSS 权限与插件/工程惯例），非文档级自审。发现接续编号 F-21~F-28。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-21 | 架构落地缺口 | **ITaskManager 挂 SA 501 的承载机制未设计**：AppMgrStub::OnRemoteRequest 校验接口描述符（app_mgr_stub.cpp:57-62），非 IAppMgr 描述符请求直接被拒——ITaskManager 代理请求无法经既有桩到达 TaskManagerService，TASK-0 无法直接开工 | 高 | 新增 design §5.2.2：AppMgrService 覆写 OnRemoteRequest 按描述符路由（ITaskManager → TaskManagerService 桩内部分发；IAppMgr 零改动）；客户端 GetSystemAbility(501)+iface_cast（app_service_manager.cpp:29-33 先例）；否决备选=同进程新 SA ID（与 D-1 冲突）；TASK-0 目标/判据/Files 同步 | 已修正 |
| F-22 | 覆盖缺口 | **扩展启动不收敛于 StartAbilityInner**（场景 6 断言错误）：StartServiceExtensionAbility/ImplicitStartExtensionAbility（:4338/:4357）直连 StartExtensionAbilityInner（:4656）；StartUIExtensionAbility（:4989）走 SessionInfo 独立路径；RequestModalUIExtension（:4475→:4508）独立路径——仅拦 StartAbilityInner 将漏掉全部扩展启动 | 高 | 拦截点①扩为 **5 收敛点**（StartAbilityInner:1360 / StartExtensionAbilityInner:4656 / StartUIExtensionAbility:4989 / RequestModalUIExtensionInner:4508 / ConnectAbilityCommon:6063），共用同一解析/上报辅助函数；场景 5/6/7/8 收敛点精确化（场景 7 意图链路实证收敛点=StartAbilityWithInsightIntent:15241）；"新变体自动覆盖"原则表述修正；TASK-3 同步 | 已修正 |
| F-23 | 事实错误 | **`const.taskmanager.low_memory_frozen_enable` 参数在开源 RSS master 不存在**（全仓 grep 无匹配；freezer 仅现于 init cfg）——TASK-6 门禁前置条件描述基于不存在的参数 | 中 | 全部 8 处替换为"产品含 freezer 执行插件（启用开关以产品配置为准）"；实际开关名列入 RSS 对齐问题 #8 | 已修正 |
| F-24 | 遗漏前置 | **RSS 侧权限前置未记录**：505/517 上报需 TOKEN_NATIVE + `ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT`（res_sched_service.cpp:43/:273-300，foundation 已具备——AMS 既有上报先例佐证）；507 查询经 suspend_manager_base_client 另需 `ohos.permission.GET_SUSPEND_STATE`（suspend_manager_base_service.cpp:27/:39，**foundation 是否已配置待确认**）；520 需权限 5523（:214） | 中 | design §6.4 新增"RSS 侧权限前置"条目（三项全列）；rss-alignment.md #5 更新为实证结论 + 新增 GET_SUSPEND_STATE 确认项 | 已修正 |
| F-25 | 清单不全 | StartAbilityInner 调用点清单不全：完整 13 处（…/:15241/:16359/:18349/:19367）+ implicit_start_processor.cpp:820 + preload_manager_service.cpp:111；第十三轮 F-11 修复时因 grep 截断**误删**了原文档正确的 :15241/:16359 | 低 | 场景 1 调用点清单恢复并补全（:15241=StartAbilityWithInsightIntent 意图收敛点、:16359=RestartApp、:18349=StartUIAbilityWithCallback、:19367=StartSandboxCloneAbility） | 已修正 |
| F-26 | 命令语法 | "hidumper -a 501" 语法错误——实际为 `hidumper -s 501 -a "<选项>"`（SA dump 入口=AppMgrService::Dump(fd,args)，app_mgr_service.cpp:928） | 低 | design §7.2 / spec R-23 修正；补 HasDumpPermission 复用说明 | 已修正 |
| F-27 | 路径惯例 | 测试目录路径不符仓惯例：test/unittest/ 为扁平命名（app_mgr_client_test 等先例），无 appmgr/ 子目录 | 低 | plan 全部测试路径改为扁平命名（task_mgr_test/、task_intercept_test/，与 `run -t UT -ts` 套件名一致） | 已修正 |
| F-28 | 实证升级 | 20+ 项此前"待确认/推断"断言本轮实证（增强可信度）：master 插件仅 device_standby/frame_aware/socperf/cgroup_sched 且均不订阅 505/517/520（P1 结论坐实）；suspend_manager_base_client 随 ressched_client 交付（无独立 BUILD.gn）；ISuspendStateObserverBase 回调携带 pidList+uid；mmi observer 先例（suspend_state_manager.cpp:60-78，与 P5 本地缓存同构）；StandbyServiceClient::ApplyAllowResource/Unapply（standby_service_client.h:76/84）；PermissionVerification::IsSystemAppCall（:106/:108）；GetUserIdByUid(GetCallingUid())（app_mgr_service_inner.cpp:679）；AMSEventHandler:EventHandlerWrap（ADR-8 先例）；aa→AbilityManagerClient::StartAbility（ability_command.cpp:486）；IAgentManager.idl 15 方法；res_type.h 全部枚举值（8/23/66/92/93/134/155/170/215/221/505~520）；OnAbilityRequestDone（:8788）；child_process_record.cpp；RSS 进程管理规范文档存在；:895/:1145/:1156 行号准确；AMS→AppMS 经 samgr 代理 IPC（同进程，native token 信任锚成立）；appmgr BUILD.gn:159 ressched_client 为无条件依赖（proposal"resource_schedule_service_enable 条件"表述过时） | 信息 | 关键实证已落入 design §5.2.2/§5.4.1/§6.4/§7.2 与本表（其余以本行清单为准）；proposal 依赖表述修正 | 已记录 |

### 结论

第十四轮以代码实证复核发现 8 项（F-21~F-28：高 2 / 中 2 / 低 3 / 实证记录 1），已全部修复。两项高严重度均为**落地阻断级**：F-21（SA 承载机制——不补则 TASK-0 的 IPC 通路不通）、F-22（扩展启动覆盖缺口——不补则 ServiceExtension/UIExtension/Modal 扩展启动全部漏拦，A2A 与意图扩展场景受损）。开放项 O-1~O-6 维持；新增 RSS 对齐确认项：GET_SUSPEND_STATE 权限（foundation native token）、freezer 启用开关实际名称。

---

## 维护说明（原交接文档关键过程信息收口，2026-08-26）

> 本特性历经十四轮评审（第一~十二轮见 proposal.md 讨论记录；第七/十三/十四轮审查见本文档），文档阶段收口，代码未启动（TASK-0 启动前置 = design/spec 评审通过，见 execution-plan.md）。

**交付件导航**：proposal.md（需求基线与决策链）→ spec.md（AC/规则/接口规格）→ design.md（架构设计，重点 §5.2.2 SA 承载 / §5.4.1 五收敛点 / §6.4 RCP）→ execution-plan.md（TASK-0~6）→ 本文档（三轮审查记录 + 开放项 O-1~O-6）→ docs/（扩展场景分析 + 两份外部对齐材料）→ 三张 SVG。

**文档变更后必做**：
1. `grep -n '账本\|记账' *.md *.svg docs/*.md` 确认无新增残留（历史决策记录行除外——"内存登记"为 D-16 定稿术语，旧"账本"已废弃）；
2. 在本目录执行 `ohos-sdd validate . --level c`（应 PASS；`--level all` 需归档证据，流程末端才可达，勿误报）；
3. `git commit -s`（Signed-off-by 签名；push 由 owner 手动执行）。

**术语陷阱**：N 指 SetTaskParams 续期次数上限（仅 priority 变化触发）；"动作日志断言"≠旧"账本断言"；master 环境 505/517/520 无消费者、507 恒返回未冻结——端到端验证会假成功（P1/F-23，TASK-6 门禁）；"CCM"为需求侧术语，本仓落地 = sysparam（.para + GetParameter，F-14）。

**勿轻易推翻的决策**（变更须与需求方重开澄清并更新 proposal.md 变更控制）：模块形态（AppMS 收口，D-1）、防仿冒（B-1+B-2+通道鉴权 D-17）、RCP 代理层（ADR-4）、inner API（D-5）、SA 501 描述符路由（D-19）、五收敛点拦截（D-20）。
