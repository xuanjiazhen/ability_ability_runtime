# 方案深度审查报告（2026-08-22 第七轮；2026-08-26 第十三轮自审追加；2026-08-26 第十四轮代码实证复核追加；2026-08-26 第十五轮 IPC 机制深挖追加；2026-08-26 第十六轮拦截器框架与参数语义追加；2026-08-26 第十七轮状态机闭环与线程模型追加；2026-08-26 第十八轮需求变更 D-21 追加；2026-08-26 第十九轮清理完整性追加；2026-08-26 第二十轮需求变更 D-22（CLI=cli_tool_framework）追加）

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

## 第十五轮：IPC 机制深挖（2026-08-26）

> 审查方法：针对第十三/十四轮**修复方案自身的底层假设**做机制级验证（同进程 SA 调用的身份语义、SA 501 分发结构、P5/P7 依赖细节）。发现接续编号 F-29~F-35。本轮特点：**F-29 直接推翻 F-8 修复的一个隐含前提**。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-29 | 安全+功能（**前轮修复缺陷**） | **F-8 通道鉴权规则①前提不成立**：设计假设"AMS 同进程转发 → AppMS 侧 GetCallingTokenID 为 native token"，但同进程 SA 代理调用为进程内直接分发，被调方看到的身份**透传原始 IPC 调用方**——本仓全部 AMS→AppMS/agentmgr 调用均须包裹 `IN_PROCESS_CALL`（ResetCallingIdentity）即为此机制的存在证明（in_process_call_wrapper.h；先例 app_scheduler.cpp:82、ability_manager_service.cpp:6092）。**后果推演（不修复）**：① attach 路径（param=①登记发起方 ≠ actual=目标进程 token）与代理启动路径（param=specifyTokenId ≠ actual=发起 app）被规则②误拒——两条核心纳管链路断裂；② 规则①若用裸 IsSACall 判定，任何进程的 native SA 直调+伪造入参即绕过（如其他系统进程 SA） | 高 | design §5.3 通道鉴权重写：新增规则 0（AMS 侧全部 TaskManagerClient 调用经 IN_PROCESS_CALL 包装）；规则①判定原语改为 `CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)`（IsSACall+native processName，agentmgr 鉴权 AMS 同进程调用同一先例 agent_manager_service.cpp:993、permission_verification.cpp:115-131）；信任边界=foundation 进程内互信；§5.2.2/§5.4.2/性能行同步；TASK-3 目标/判据/Files 同步 | 已修正 |
| F-30 | 依赖缺口 | **P7 EDS 落地依赖缺失**：appmgr BUILD.gn 现无 device_standby 依赖（deps 清单实证），plan BUILD 变更未列；目标名 `device_standby:standby_innerkits`。权限无阻碍（standby_service_impl.cpp:626-636：native 免权限，仅 HAP 需 DEVICE_STANDBY_EXEMPTION） | 中 | plan 全量清单与 TASK-4 Files 补依赖行；design §6.1.1-7 补落地要点（ResourceRequest 字段/权限/依赖） | 已修正 |
| F-31 | 计划缺口 | **AbilityRecord 无"业务发起方 tokenId"字段**（ability_record.h 实证：仅 want_/CallerRecord）——§5.4.2"存入 AbilityRecord"需新增字段或 AMS 侧表，TASK-3 文件清单原缺 ability_record.h/cpp | 中 | design §5.4.2 补存储缺口说明；TASK-3 Files 补 ability_record.h/cpp（规模 +50 行） | 已修正 |
| F-32 | 设计缺口 | **场景 6 UIExtension 路径身份解析来源未指定**：StartUIExtensionAbility(SessionInfo, userId) 无 callerToken 参数，IPC 调用方为 sceneboard——直接取 GetCallingTokenID 得 sceneboard 身份被 B-2 必拒；须取 SessionInfo.callerToken_/callingTokenId_（ws_common.h:464/:488 实证）。RequestModalUIExtension 由应用侧框架直调（js_service_extension_context.cpp:187）→ GetCallingTokenID=应用自身 ✓ 无此问题 | 中 | design §5.4.1 场景 6 补身份解析来源；TASK-3 目标同步 | 已修正 |
| F-33 | 权限前置扩大 | **RegisterSuspendObserver 同样要求 GET_SUSPEND_STATE 权限**（suspend_manager_base_service.cpp:103-111）——P5 observer 注册与 507 查询同受 F-24 确认项约束 | 低 | design §6.4 权限前置条目②合并标注 | 已修正 |
| F-34 | EDS 能力发现 | **AllowType 含 FREEZE 位**（allow_type.h:25-34：NETWORK/RUNNING_LOCK/TIMER/WORK_SCHEDULER/AUTO_SYNC/PUSH/FREEZE）——待机**冻结**豁免可经 EDS 申请，与 O-2（锁屏冻结边界）直接相关；P7 原仅提"网络/任务执行" | 低 | design §6.1.1-7 补 FREEZE 位评估；rss-alignment 新增对齐问题 | 已修正 |
| F-35 | 假设标注 | **505 value 施加/撤销语义在 res_type.h 无文档**（对比 503 有明确 value 0/1 注释）——设计"EXEMPT→1/CLEAR→0"纯为假设；另 §6.2 引 res_sched_util.cpp:228-234 为"505 payload 先例"实为 517 payload（字段模式先例） | 信息 | design §6.2 显式标注"假设待 RSS 确认"+先例措辞精确化（rss-alignment #2 已列） | 已修正 |

### 本轮机制级实证清单（增强可信度）

| 断言 | 证据 |
|------|------|
| 同进程 SA 代理调用透传原始调用方身份；IN_PROCESS_CALL 为身份重置标准手段 | in_process_call_wrapper.h（ResetCallingIdentity 包装宏）；AMS→AppMS 全调用包裹（app_scheduler.cpp:82/:96 等 8+ 处）；AMS→agentmgr 同（ability_manager_service.cpp:6092） |
| 同进程 SA 鉴权既有原语 = CheckSpecificSystemAbilityAccessPermission(processName) | agentmgr 鉴权 AMS：agent_manager_service.cpp:993-1001；实现 permission_verification.cpp:115-131（IsSACall + GetNativeTokenInfo.processName） |
| SystemAbility 不继承 IRemoteStub——SA 501 唯一 IRemoteObject 基座是 AppMgrStub，单入口分发（F-21 路由结构可行） | safwk system_ability.h:62（`class SystemAbility {` 纯基类）；AppMgrService : SystemAbility + AppMgrStub |
| Parcel::RewindRead 存在（F-21 路由实现选项可行） | commonlibrary/c_utils/base/include/parcel.h:617 |
| StartAbilityWrapParam 含 callerToken/specifyTokenId/specifiedFullTokenId（拦截点身份解析输入完备） | start_ability_utils.h:96-124 |
| hisysevent.yaml 域名 = AAFWK（design 7.1 断言成立） | hisysevent.yaml:14 |

### 未与用户对齐的自主决策清单（待用户确认）

> 以下为历轮 AI 自主做出、用户尚未逐项显式确认的决策点（用户要求暴露）：

| # | 决策 | 轮次 | 性质 | 若不认可的回退成本 |
|---|------|------|------|-------------------|
| A-1 | PAYLOAD_UPDATED noticeType 新增（API 面变更） | 第十三轮 F-9 | inner API 增量（未编码，零兼容成本） | 低：删枚举+回调场景描述 |
| A-2 | SA 501 描述符路由方案（含否决"同进程新 SA ID"备选） | 第十四轮 F-21 | 架构决策 D-19 | 中：改独立 SA 需 SA ID 审批+sa_profile |
| A-3 | 拦截点扩为 5 收敛点（TASK-3 范围约翻倍） | 第十四轮 F-22 | 覆盖范围决策 D-20 | 中：若一期只做 StartAbilityInner+Connect 两点，扩展启动场景（A2A/意图扩展）暂不纳管 |
| A-4 | 通道鉴权规则 0/①（IN_PROCESS_CALL + CheckSpecificSystemAbilityAccessPermission） | 本轮 F-29 | 安全机制细化 | 低：实现细节层，方案框架不变 |
| A-5 | proposal frontmatter status Draft→committed | 上轮 | 元数据 | 极低 |
| A-6 | registry.md / evidence/reviews/spec-compliance.md / review.md 创建与"文档阶段 PASS"自评 | 上轮 | Level D 证据 | 低：自评结论有十四轮审查记录支撑，可复核 |
| A-7 | D-17~D-20 决策条目化 | 第十三/十四轮 | 决策记录 | 极低 |

### 结论

第十五轮发现 7 项（F-29~F-35：高 1 / 中 3 / 低 2 / 信息 1），已全部修复。**F-29 是对本特性审查方法的警示**：第十三轮设计通道鉴权时未验证"同进程调用身份语义"这一底层机制，第十四轮 F-28 虽实证"AMS→AppMS 经 samgr 代理"却未追问身份透传问题——两轮都在"看起来合理"的假设上构建了安全规则。经本轮修正，通道鉴权机制与本仓既有惯例（IN_PROCESS_CALL + CheckSpecificSystemAbilityAccessPermission）完全对齐，attach/代理启动两链路的功能正确性与"非 foundation native 伪造"的安全性同时闭环。

---

## 第十六轮：拦截器框架与参数语义（2026-08-26）

> 审查方法：攻击尚未验证过的机制层——AMS 拦截器框架（设计通篇未提）、sysparam 前缀语义、attach 兜底链路措辞、IDL 细节、错误码段约束。发现接续编号 F-36~F-41。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-36 | **架构分析缺失** | **AMS 拦截器框架未分析**：`AbilityInterceptorExecuter`（CLAUDE.md 列"Adding a New Interceptor"为标准扩展程序）在 5 收敛点中的 **4 处已运行**——StartAbilityInner(:1490)/StartExtensionAbilityInner(:4712)/StartUIExtensionAbility(:5100)/ConnectAbilityCommon(:6135, fromConnect=true)，仅 RequestModalUIExtension 无。设计提出 ad-hoc 辅助函数插入 5 处而**从未分析框架选项**——违背 AMS 架构惯例的风险：自建挂点绕过规范扩展机制，后续维护者需理解两套拦截逻辑。**摩擦点（诚实列出）**：① 全部拦截器调用点 callerToken=nullptr（:1489/:4712/:5100/:6135 实证）——需扩展 AbilityInterceptorParam（纯增字段）或拦截器内另行取身份；② 拦截器在 record 解析前执行——"既有 record 直接纳管"分支需拦截器自查 record 管理器；③ 拦截器返回非 ERR_OK 阻断启动——任务拦截器必须恒返 ERR_OK（观察者模式，与 R-8/R-13 一致）；④ RequestModalUIExtension 仍需 ad-hoc 挂点 | 高 | design 新增 §5.4.1a 方案对比（A 框架 vs B ad-hoc），**推荐混合方案**（TaskInterceptor 走框架覆盖 4 点 + Modal 单点 ad-hoc + AbilityInterceptorParam 增 callerTokenId 字段），**列入未对齐清单 A-8 待评审确认**；TASK-3 目标/Files 同步（新增 task_interceptor.h/cpp + ability_interceptor_interface.h 修改） | 已修正（方案待确认） |
| F-37 | **前轮修复缺陷** | **const.* 参数前缀语义错误**（第十三轮 F-14 自身缺陷）：本仓惯例实证——`const.*` 启动后**不可变**（appfwk.para:43-49），**运行时可调用 `persist.*`**（先例 persist.sys.abilityms.support_ecologicalrulemgrservice 等，appfwk.para:21-25，运行时 param set + Get*Parameter 实时读取）。"CCM 可调"若含运行时调参（产品调优不重启），const.sys.appms.taskmanager.* **不达目标** | 中 | design §6.1/plan 全量清单与 TASK-4 Files 改 `persist.sys.appms.taskmanager.*` | 已修正 |
| F-38 | IDL 细节 | OnTaskStateChanged 缺 oneway 标注（OnTaskNotice 有）——R-14"回调不阻塞状态机"要求两者均 oneway，否则慢监听者阻塞 taskHandler 派发线程 | 低 | design §5.2 IDL 两回调均补 [oneway] | 已修正 |
| F-39 | 措辞错误 | "attach 晚于超时由 attach 处重发①"（两处）——attach 时 pid 已就绪，应**直接补发②**（RegisterTaskProcess，身份取 AbilityRecord 存储值 F-31）；重发①（意向登记）在 pid 已知时无意义，系早期两段式设计残留 | 低 | design §5.4.2/风险表两处修正 | 已修正 |
| F-40 | 约束未声明 | ① 错误码段：16000001~16000205 为公共 JS API 错误码契约段（57 个 EXTERNAL 错误码占用），任务域错误码必须避开——设计"编号设计定稿"未声明该约束；② hisysevent 事件定义缺 `__BASE`（type/level/tag）属性约定（yaml 格式实证） | 低 | design 错误码节补编号约束（建议 16000300+ 或独立段）；§7.1 补 __BASE 格式约束 | 已修正 |
| F-41 | 推测项 | **StartUIExtensionAbility 的 Want 参数存活性未验证**：Want 经 sceneboard 组装进 SessionInfo 再到 AMS——自定义参数（ohos.ai.taskId）应随 Want 序列化保留（**此处为推测**：Want 为 parcelable 透传、未见参数清洗逻辑），但依赖 sceneboard 不剥离未知参数；若剥离则 UIExtension 链路纳管失效 | 信息 | TASK-3 集成用例补 UIExtension Want 参数 taskId 存活验证（plan Files 已同步）；若实测被剥离，需 sceneboard 侧确认保留策略（新增对齐项） | 已标注 |

### 本轮机制级实证清单

| 断言 | 证据 |
|------|------|
| 拦截器框架覆盖 4/5 收敛点（含连接路径 fromConnect） | interceptorExecuter_->DoProcess：:1490（StartAbilityInner）/ :4712（StartExtensionAbilityInner）/ :5100（StartUIExtensionAbility）/ :6135（ConnectAbilityCommon）；RequestModalUIExtension（:4508）无 |
| 拦截器 param 全部 callerToken=nullptr | :1489/:4712/:5100/:6135 构造处实证（AbilityInterceptorParam 第 5 参 nullptr） |
| afterCheckExecuter_ 后置段携带 abilityInfo | ability_interceptor_interface.h:50 注释 + :1705/:1922/:2234 调用 |
| const.* 不可变 / persist.* 运行时可调 | appfwk.para:21-25（persist.sys.abilityms.* 先例）vs :43-49（const.sys.abilityms.*）；运行时读取先例 ability_connect_manager.cpp:2323（persist.sceneboard.restart） |
| OnAbilityRequestDone 为 AppMS→AMS 状态回调（attach 兜底可行） | app_state_callback_proxy.cpp:37 → ability_manager_service.cpp:8788（按 DATA/SERVICE/EXTENSION/default 分发） |
| 公共错误码段占用 16000001~16000205（57 个） | ability_runtime_error_util.h 实证 |
| hisysevent 事件需 __BASE + 类型化字段 | hisysevent.yaml:16-23 格式实证 |
| Want 保留参数机制存在（PARAM_RESV_CALLER_TOKEN 由 AMS 内部写入） | update_caller_info_util.cpp:88/:145/:231；ability_manager_service.cpp:8993 |

### 未与用户对齐的自主决策清单（更新）

> 历轮 AI 自主做出、用户尚未逐项显式确认的决策点（用户要求暴露；A-1~A-7 见第十五轮，本轮新增 A-8）：

| # | 决策 | 轮次 | 性质 | 若不认可的回退成本 |
|---|------|------|------|-------------------|
| A-8 | **拦截实现混合方案**（TaskInterceptor 走拦截器框架覆盖 4 收敛点 + RequestModalUIExtension ad-hoc + AbilityInterceptorParam 增 callerTokenId 字段） | 本轮 F-36 | 架构决策（TASK-3 实现路径） | 低：回退方案 B（5 点 ad-hoc 辅助函数，原第十四轮 F-22 方案），文件清单相应回退 |

### 结论

第十六轮发现 6 项（F-36~F-41：高 1 / 中 1 / 低 3 / 信息 1），已全部修复（F-36 方案待评审确认）。**F-36 再次印证第十五轮的教训**：第十四轮 F-22 修正拦截点覆盖时逐点勘察了收敛性，却未注意到这些点上**已经运行着 AMS 的规范拦截器框架**——"看到函数"不等于"看到架构"。F-37 则是第十三轮 F-14 修复在未验证参数前缀语义的情况下选错了前缀（const vs persist），所幸未编码零成本修正。累计十六轮共 41 项发现（F-1~F-41），文档阶段审查深度已覆盖：需求映射/状态机/安全（B-1+B-2+通道鉴权）/IPC 机制（身份语义/SA 承载）/拦截架构（收敛点+框架）/参数体系/权限前置/DFX 格式。

---

## 第十七轮：状态机闭环与线程模型（2026-08-26）

> 审查方法：端到端场景推演（控制面失联时序）+ 线程模型一致性 + 收口文档时效性。发现接续编号 F-43~F-46。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-43 | **内部矛盾 + 设计缺口** | **§5.2.1 #4 虚假断言**："30min 交互超时/60min 豁免上限使任务自动流转到超时终态"——与 ADR-6（状态机仅创建者驱动，"模块自动推导"已被明确否决）、F-5（超时处置=降档+告警，且明言"后续助手 SetTaskState 回 WORKING"即任务仍在交互态）、R-21（到期撤销豁免，不改状态）**三方矛盾**。**后果推演**：助手冻结/死亡时任务永久滞留激活态——① 占用 R-2 活跃配额（非终态计数），16 个滞留任务即配额死锁；② 创建者死亡时无人可 Cancel/Destroy（R-12 仅覆盖纳管进程死亡，创建者死亡清理缺失）；③ 资源面无死锁（豁免已撤销）但配额面死锁。原"不会死锁"断言只对资源面成立 | 高 | §5.2.1 #4 重写为"超时兜底覆盖资源面，不覆盖状态面"（如实表述）；风险表新增条目；R-2 补"活跃=非终态"定义；**处置两方案列入 A-9 待用户对齐**：(a) ADR-6 增补窄口径系统流转（超时→FAILED/创建者死亡→CANCELED，复用 AppDeathRecipient 链——推荐）；(b) 接受滞留（创建者重启可恢复，永久死亡则配额滞留至 foundation 重启） | 已修正（处置方案待对齐 A-9） |
| F-44 | 线程模型张力 | **taskHandler 串行 vs RCP 同步调用阻塞放大未分析**：P6 要求"不持锁发起同步调用"，但 ADR-8 的串行 taskHandler **本身就是全局锁**——若 507/517 同步调用在 taskHandler 上执行，一次超时（如 5s）阻塞全部任务操作；且 IPC stub 需 post-and-wait（CreateTask 要同步返回 taskId），等待中的 binder 线程随之堆积。两要求（串行免锁 vs 同步调用不持锁）在"同步调用跑在哪"上未给出自洽答案 | 中 | ADR-8 补 IPC 入口线程模型（stub post-and-wait）+ RCP 同步调用移至 RCP 内部工作线程、结果回投 taskHandler；P6 补强（含 ≤500ms 超时预算建议，随 TASK-4 定稿） | 已修正 |
| F-45 | 收口文档过时 | review.md"十四轮审查收口"（实际十七轮）；manifest.md commits 缺第十五/十六轮提交 | 低 | review.md 更新（十七轮 + A-1~A-9 待对齐提示）；manifest commits 补全 | 已修正 |
| F-46 | 方案细化 | §5.4.1a"既有 record 直接纳管"分支未提 afterCheckExecuter_ 选项（abilityInfo 已解析处 :1705/:1922/:2234，比拦截器自查 record 管理器更贴合既有结构） | 信息 | §5.4.1a 补 afterCheck 挂点选项 | 已修正 |

### 本轮机制级实证清单

| 断言 | 证据 |
|------|------|
| **attach 每 ability 实例触发**（热启动新实例走意向路径的前提成立，F-11 结论再确认） | ui_ability_thread.cpp:207 / extension_ability_thread.cpp:337 / fa_ability_thread.cpp:320（每 ability 线程 Attach 时调 AttachAbilityThread） |
| AppMS 进程死亡通知链存在（R-12 依据 + F-43 方案 (a) 创建者死亡监听可复用） | app_death_recipient.h / app_running_record.h:579-597（SetAppDeathRecipient） |
| §5.2.1 #4 矛盾三方证据 | design ADR-6（仅创建者驱动）/ F-5 第 2 步（降档不改状态，"助手 SetTaskState 回 WORKING"）/ R-21（撤销豁免） |

### 未与用户对齐的自主决策清单（更新）

> A-1~A-8 见第十五/十六轮节，本轮新增 A-9：

| # | 决策 | 轮次 | 性质 | 若不认可的回退成本 |
|---|------|------|------|-------------------|
| A-9 | **滞留任务处置**：推荐方案 (a)——ADR-6 增补窄口径系统流转（超时→FAILED(reason=timeout)/创建者进程死亡→CANCELED，复用 AppDeathRecipient 链）；备选 (b) 接受滞留 | 本轮 F-43 | 状态机语义变更（触碰 D-6"勿轻易推翻"决策边界） | 低：选 (b) 仅需回退 §5.2.1 表述与风险表，状态机零改动；但配额死锁风险自担 |

### 结论

第十七轮发现 4 项（F-43~F-46：高 1 / 中 1 / 低 1 / 信息 1），已全部修复（F-43 处置方案待 A-9 对齐）。**F-43 的教训与前两轮同构但更严重**：§5.2.1 #4 是第五轮（2026-08-21）为论证"助手不自动纳管"而写的支撑理由，其"超时自动流转终态"的断言在后续八轮细化（ADR-6/F-5/R-21）中被逐步证伪，却无人回头核对原始论证——**决策理由与决策结论的同步失效**是文档迭代的系统性风险。累计十七轮 45 项发现（F-1~F-46，含 F-28/F-35 实证记录项）。

### 结论

第十七轮发现 4 项（F-43~F-46：高 1 / 中 1 / 低 1 / 信息 1），已全部修复（F-43 处置方案待 A-9 对齐）。**F-43 的教训与前两轮同构但更严重**：§5.2.1 #4 是第五轮（2026-08-21）为论证"助手不自动纳管"而写的支撑理由，其"超时自动流转终态"的断言在后续八轮细化（ADR-6/F-5/R-21）中被逐步证伪，却无人回头核对原始论证——**决策理由与决策结论的同步失效**是文档迭代的系统性风险。累计十七轮 45 项发现（F-1~F-46，含 F-28/F-35 实证记录项）。

---

## 第十八轮：需求变更 D-21——AI bar 内化交互（2026-08-26）

> **性质：用户需求变更（非审查发现）**。用户指令：① 原设计"系统 UI 展示执行进度"改由 AI 助手自有 AI bar 实现；② 无需外部通知和交互；③ 保留状态监听能力，提供给安全等模块使用。按 proposal 变更控制规则执行（AC 变更→重新审批基线，本次指令即需求方审批）。

### 变更内容

| 项 | 变更前 | 变更后 |
|----|--------|--------|
| 进度展示 | 系统 UI 经 OnTaskNotice(PAYLOAD_UPDATED) 实时收到（F-9/D-18） | **AI bar 自呈现**（助手即创建者、数据在己方）；PAYLOAD_UPDATED 移除 |
| INPUT/AUTH 交互 | 系统 UI 展示→SubmitTaskInput 回传→INPUT_ARRIVED 通知创建者（D-11 传递通道） | **AI bar 进程内闭环**（呈现→用户响应→助手直接获得→SetTaskState 流转，响应可选经流转 payload 携带）；SubmitTaskInput/INPUT_ARRIVED/input 字段/白名单移除 |
| 状态监听 | 消费方=系统 UI（展示） | **保留**，消费方重定位=安全管控/审计/系统状态监控等系统模块（状态事实感知）；OnTaskStateChanged（含交互态 payload）+ OnTaskNotice（余 3 类超时通知）不变 |
| 接口面 | ITaskManager 13 方法 | **12 方法**（SubmitTaskInput 移除） |
| 状态机 | 9 态不变 | 9 态不变（INPUT/AUTH_REQUIRED 保留，payload 留存为事实记录） |

### 连带影响（已全部同步）

- **design.md**：ADR-7 重写/§5.2 IDL（12 方法+通知枚举余 3）/§5.5 重写/§5.6 B-4/§5.7 权限表/TaskRecord·TaskInfo 去 input/数据流交互表重写/TASK_INPUT 事件 action 收窄（require/clear）/调用链层级与架构图消费方表述
- **spec.md**：US-3/US-4 重写、AC-3.1/AC-4.1~4.3 改写、AC-4.4 删除、R-6/R-16/R-17/R-18 改写、API 表与接口规格节（SubmitTaskInput 节保留为变更记录）、NFR 内存（~9KB→~5KB）、无障碍/多设备表述
- **proposal.md**：活基线（问题陈述/成功指标/US/AC/范围/API 清单）+ 讨论记录追加变更行；原始需求 13 条与历史讨论记录**保留原文**（历史事实）
- **execution-plan.md**：TASK-2 重写（规模 ~900→~700 生产/~700→~550 单测）、合计 1.1 万→1.05 万行、TASK-0 12 方法、AC 追溯
- **docs/**：flow-diagrams 图 1 阶段三/图 5 重写（AI bar 参与方）；ai-assistant-alignment Q-2/Q-3/Q-10 更新；extension-analysis 可复用资产表（挂起-回传通道→状态监听通道）
- **SVG×3**：module_design（方法列表）/task_flow（交互分支）/architecture_overview（消费方）同步

### 决策清单影响

- **D-11（2026-08-24 UI↔助手传递通道）→ 被 D-21 取代**（外部传递通道无消费方）；
- **A-1（PAYLOAD_UPDATED 待对齐）→ 关闭**（随 D-21 移除，无需对齐）；
- A-2~A-9 不受影响（A-9 滞留任务处置仍待对齐）。

### 结论

需求变更 D-21 全局落地完成：接口面收敛（13→12 方法）、职责收敛（任务模块=状态事实记录+管控，交互归助手自有 UI）、监听能力保留并重定位消费方。validate --level all PASS。

---

## 第十九轮：清理完整性（2026-08-26，用户指出阶段四遗漏后系统核查）

> **触发：用户指令**——"阶段 4 还需要通知 RSS 清理豁免冻结、调度优先级的修改等，确认其他地方是否有遗漏"。对任务结束/降级/死亡全路径的**管控状态清理清单**逐项核查。发现接续编号 F-47。

### 发现与处理

| 编号 | 类别 | 问题点 | 严重度 | 处理 | 状态 |
|------|------|--------|--------|------|------|
| F-47 | 清理完整性（8 项） | **① EDS 撤销粒度错误（核心）**：EDS 按 uid+bundleName 授权（ResourceRequest 实证 resource_request.h），图 7 原将 UnapplyAllowResource 置于通道①逐 pid 流程且无剩余任务判断——**同 uid 多任务并发时任一任务结束即 Unapply 会误杀其余任务的待机豁免**；② CLEAR_QOS（恢复调度优先级，二期）未在图 7 体现（用户点名的"调度优先级修改"清理项）；③ 降档路径（F-5 第 2 步超时处置/F-6 优先级降档）的 EDS 重评估未明确；④ 进程死亡路径（R-12）的 EDS 处置未明确；⑤ NotifyTaskEnd 与 EDS 关系未声明（EDS 无任务级策略，防对接误解）；⑥ **终态取消计时器未规定**——终态后 INTERACTIVE_TIMEOUT 等计时器仍挂起，存在终态任务被误触发超时处置的风险；⑦ 内存登记随终态清理未入流程图（D-16 有语义无动作）；⑧ 图 7 R18"payload/input 清除"过时（input 随 D-21 移除） | 高（①为功能缺陷：多任务并发场景待机豁免被误杀→夜间定时任务失败——恰为本特性要解决的核心场景） | design §6.4 双通道重写：前置清理（R-18+**取消全部计时器**，超时处理器校验非终态双保险）+ ①CLEAR_EXEMPT/**CLEAR_QOS**/EDS **uid 级引用计数**（归零才 Unapply）+ ②NotifyTaskEnd（显式声明 EDS 无任务级策略）+ ③**内存登记清理**；新增"EDS uid 级引用计数规则"（施加=首个激活任务 Apply 幂等/撤销=最后一个结束或降档或全进程死亡时 Unapply，计数在 process_schedule）；P7/F-5/F-6/R-9/R-12 同步；图 7 重写（含 EDS 判定分支）/图 6/图 8/图 1 阶段四联动 | 已修正 |

### 清理清单全景（修正后，回答用户问题）

| 管控状态 | 施加时机 | 清理时机 | 清理方式 | 依据 |
|----------|----------|----------|----------|------|
| 冻结豁免（505） | 激活态+档位≥1 | 终态/降档/超时/进程死亡 | CLEAR_EXEMPT（按剩余任务 max 重算；无任务完全恢复，RSS 自然回冻无需 re-freeze） | R-9/R-19 |
| 调度优先级（QoS 221，二期） | 档位 3 提权 | 同上 | CLEAR_QOS（ControlAction 预留） | R-9/ADR-9 |
| EDS 待机豁免 | uid 首个激活态任务 | uid 最后一个激活任务结束/降档/全进程死亡 | UnapplyAllowResource（**uid 级引用计数**，归零才撤） | P7/F-47 |
| RSS 任务级策略（豁免组/优先级组） | RSS 对接后按 taskId 建立 | 任务终态 | NotifyTaskEnd(taskId,pids,reason) | R-20 |
| 内存登记条目 | 每次管控动作 | 任务终态（随终态移除） | 登记清理（通道③） | D-16/F-47 |
| 计时器/续期计数 | 激活态启动 | **终态强制取消**（处理器另校验非终态） | 前置清理 | F-47 |

### 结论

第十九轮发现 1 项复合问题（F-47，含 8 子项，核心为 EDS 撤销粒度——多任务并发下待机豁免误杀会直接击穿"夜间定时任务"核心场景）。已全部修复：design §6.4 清理清单全景化 + EDS uid 引用计数规则、spec R-9/R-12、图 1/6/7/8 联动。**教训**：P7（第十五轮）补入 EDS 联动时只定义了施加/撤销的成对调用，未推导"撤销的粒度必须与授权粒度一致"（uid≠pid）——新增联动通道时必须同步推导其**计数模型**。

---

## 第二十轮：需求变更 D-22——CLI 确指 cli_tool_framework（2026-08-26）

> **触发：用户确认**——需求中的"Cli"指 cli_tool_framework 拉起的 CLI（非 tools/aa 调试工具）。原方案场景 8 基于错误理解（aa 命令→StartAbilityInner 天然覆盖），实际进程模型完全不同。

### cli_tool_framework 进程模型（实地勘察实证）

| 维度 | 事实 | 证据 |
|------|------|------|
| 服务进程 | aimgr **独立进程**（uid=aimgr，SA 186 libclimgr，ondemand，SELinux u:r:aimgr:s0） | cli_tool_framework/etc/profile/aimgr.cfg；sa_profile/186.json |
| CLI 工具进程 | aimgr `ProcessManager::CreateChildProcess` = **fork() + execvp("/system/bin/claw_sandbox")**（沙箱内执行工具）；ExecCmd 同构（/bin/sh -c） | process_manager.cpp:139-191/:203-240 |
| **关键：不经 AMS/AppSpawn** | CLI 进程无 AbilityRecord、无 attach、不经 5 收敛点任何一处——**Want 拦截方案对其完全失效** | process_manager.cpp 全文 |
| CLI 进程身份 | 调用方 HAP tokenId + CliBinaryInvoker 标志（AddCliBinaryInvokerTokenFlag 经 /dev/access_token_id ioctl 注入）——**≠原始 tokenId 数值** | process_manager.cpp:273-289 |
| 死亡监控 | aimgr 自带 SIGCHLD reaper（waitpid 循环）+ 会话超时 Killpg | cli_tool_manager_service.cpp:228/:364/:121 |
| Skill 会话路径 | ExecuteInAppSkillWithTokenId → AMS StartExtensionAbilityInner(:4656)/StartAbilityByCall——**经收敛点，天然覆盖**；skillArgs 经 ohos.skill.executeParam.args.* 写入 Want | cli_tool_manager_service.cpp:1685；skill_execute_param.cpp:210-213 |
| 权限门 | ExecTool 需 ohos.permission.EXEC_CLI_TOOL | cli_tool_manager_service.cpp:52/:856 |

### 发现与处理（F-48）

| 编号 | 问题点 | 严重度 | 处理 |
|------|--------|--------|------|
| F-48 | **场景 8 理解错误（覆盖断裂）**：原设计"Cli=tools/aa→StartAbilityInner 天然覆盖"——实际 CLI 工具进程 fork+claw_sandbox 不经 AMS，5 收敛点全覆盖不到；且 B-2 集合匹配对 Invoker 标志 tokenId 不命中；R-12 死亡链（AppMS AppDeathRecipient）不覆盖 aimgr 子进程 | 高（若不修正：CLI 场景纳管完全失效——需求 3 明确列 Cli 为五大启动链路之一） | design 场景 8 重写（双路径）+ 新增 §5.4.3 CLI 挂点设计（taskId 经 ExecToolParam.args 透传/aimgr fork 后 RegisterTaskProcess/通道鉴权规则 1b（processName=="aimgr"，EXEC_CLI_TOOL 权限门+现场取值三重约束）/B-2 集合匹配双值/SIGCHLD+Killpg+ClearSession 死亡清理/Skill 路径 args 透传天然覆盖/沙箱 RSS 适用性待真机验证）；§5.3 规则 1b/§5.3.1 ⑨ 仿冒面分析/调用链层级表；spec R-7/R-12/RegisterTaskProcess 场景 6；plan TASK-3（climgr 挂点文件+用例+规模 +100）；图 2 CLI 分支 | 已修正 |

### 结论

需求变更 D-22 落地完成：CLI 纳管经"aimgr 挂点（工具路径）+ 收敛点天然覆盖（Skill 路径）"双路径闭环；通道鉴权扩展规则 1b（aimgr native 信任，三重约束论证）；B-2/R-12 语义适配完成。**教训**：需求术语（"Cli"）在基线阶段未与需求方确认具体所指，方案基于字面理解（aa 工具）建立了错误覆盖断言并延续十九轮——**关键术语应在澄清阶段要求需求方给出实体级定义**（本项已列入 ai-assistant-alignment 后续对齐惯例）。

---

## 维护说明（原交接文档关键过程信息收口，2026-08-26）

> 本特性历经十四轮评审（第一~十二轮见 proposal.md 讨论记录；第七/十三/十四轮审查见本文档），文档阶段收口，代码未启动（TASK-0 启动前置 = design/spec 评审通过，见 execution-plan.md）。

**交付件导航**：proposal.md（需求基线与决策链）→ spec.md（AC/规则/接口规格）→ design.md（架构设计，重点 §5.2.2 SA 承载 / §5.4.1 五收敛点 / §6.4 RCP）→ execution-plan.md（TASK-0~6）→ 本文档（三轮审查记录 + 开放项 O-1~O-6）→ docs/（**flow-diagrams.md 分步流程图集（8 图，含决策分支/失败路径/规则追溯）** + 扩展场景分析 + 两份外部对齐材料）→ 三张 SVG。

**文档变更后必做**：
1. `grep -n '账本\|记账' *.md *.svg docs/*.md` 确认无新增残留（历史决策记录行除外——"内存登记"为 D-16 定稿术语，旧"账本"已废弃）；
2. 在本目录执行 `ohos-sdd validate . --level all`（2026-08-26 起 D 级证据已补齐——registry.md 索引 + evidence/reviews/spec-compliance.md——应全量 PASS；若 review 边 FAIL，先更新 spec-compliance.md 再收口）；
3. `git commit -s`（Signed-off-by 签名；push 由 owner 手动执行）。

**术语陷阱**：N 指 SetTaskParams 续期次数上限（仅 priority 变化触发）；"动作日志断言"≠旧"账本断言"；master 环境 505/517/520 无消费者、507 恒返回未冻结——端到端验证会假成功（P1/F-23，TASK-6 门禁）；"CCM"为需求侧术语，本仓落地 = sysparam（.para + GetParameter，F-14）。

**勿轻易推翻的决策**（变更须与需求方重开澄清并更新 proposal.md 变更控制）：模块形态（AppMS 收口，D-1）、防仿冒（B-1+B-2+通道鉴权 D-17）、RCP 代理层（ADR-4）、inner API（D-5）、SA 501 描述符路由（D-19）、五收敛点拦截（D-20）、**AI bar 内化交互（D-21，2026-08-26 需求变更——取代 D-11 传递通道）**。
