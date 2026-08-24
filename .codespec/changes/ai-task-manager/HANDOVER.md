# 交接文档：AI 语音助手任务管理机制（AI Task Manager）

> 供后续 AI/工程师接手。目标：不依赖上下文对话即可继续分析与开发。
> 生成时间：2026-08-22 · 当前进度：SDD 文档阶段完成（设计评审第七轮），**尚未开始写代码**。

---

## 1. 项目背景与目标

**问题**：AI 语音助手面临 ① 定时任务锁屏下执行失败；② 长时任务切后台后被冻结无法执行。根因是系统 RSS（资源调度）管控（冻结、内存/CPU 优先级）。安全管控问题暂不讨论，本期聚焦 RSS 冻结管控。

**目标**：提供任务管理机制给 AI 语音助手：
- AI 助手创建任务（指定优先级、类型），可设置任务状态（9 态状态机）；
- AI 助手通过 StartAbility / ConnectAbility / 意图框架 / Cli / A2A 启动能力时携带 taskId，目标进程自动纳入任务，按任务优先级进行 RSS 管控（冻结豁免/解冻）；
- 任务结束自动降级恢复原调度状态；多任务并发、一进程多任务按最高优先级管控；
- 提供状态查询/监听（系统 UI 展示进度）、INPUT_REQUIRED/AUTH_REQUIRED 交互传达、DFX；
- 任务概念参考 **google-adk** 的 Invocation；管控策略参考其设计。

**参考概念（google-adk）**：Invocation ≈ 任务（invocation_id = taskId）；`ToolContext.request_credential`/`adk_request_input` HITL 挂起机制 ≈ INPUT/AUTH_REQUIRED 挂起-回传；A2A 参数透传 ≈ A2A 链路 taskId 传递；Cost limits ≈ 任务上限防滥用。

---

## 2. 工作方式：OHOS SDD 流程

本项目按 ohos-sdd 插件的 SDD 流程推进，交付件在 `.codespec/changes/ai-task-manager/`：

```
.codespec/changes/ai-task-manager/
├── manifest.md              # 元数据（profile: none, complexity: standard, status: draft）
├── proposal.md              # 需求文档 + 基线（已通过，七轮澄清/评审）
├── spec.md                  # 特性规格：US-1~6 + AC-1.1~AC-6.2 + 规则 R-1~R-24 + 接口行为规格
├── design.md                # 架构设计：任务模型/状态机/模块设计/RSS 细化/防仿冒/拦截点 + ADR-1~9
├── execution-plan.md        # 执行计划：TASK-0~6 + 文件全量清单 + AC 追溯 + 代码量估算
├── review-analysis.md       # 第七轮深度审查报告（F-1~F-7 修正记录）
├── architecture_overview.svg # 对外总览图（背景/方案/修改点）
├── module_design.svg        # 三模块具体设计图（类图式）
└── task_flow.svg            # 整体流程图（泳道时序）
```

**校验命令**（在 item 目录执行，插件路径版本号可能变化）：
```bash
cd .codespec/changes/ai-task-manager
/home/huawei/.claude/plugins/cache/ohos-sdd-local/ohos-sdd/0.3.3/bin/ohos-sdd validate . --level all
# A=结构存在 B=锚点标题 C=依赖边一致 → 应 PASS
# D=归档就绪（registry.md 索引、review 证据）→ 当前 2 broken 属正常（流程末端项）
# E=profile 命中 → none
```

**Git**：分支 `feature/ai-task-manager`（基于 aaf020f296 master），7 次提交均含 `Signed-off-by: yangzk <yangzhongkai@huawei.com>`，最新 `7fe9e832bf`。

---

## 3. 关键决策汇总（七轮评审固化，勿轻易推翻）

| # | 决策 | 结论 |
|---|------|------|
| D-1 | 模块形态 | **并入 AppMS**（AppManagerService，SA 501，foundation 进程）：`services/appmgr/src/task_mgr/`（任务语义层）+ `process_schedule/`（通用进程调度能力层）。否决 agentmgr 方案的理由：A2A/意图/CLI 启动的进程不经过 agentmgr，但**所有应用进程都经过 AppMS**（进程权威信息在手），且 appmgr 已依赖 `ressched_client`（BUILD.gn:159） |
| D-2 | RSS 集成 | **ResourceControlProxy 代理层**（代理模式）：RSS 具体设计由 RSS 团队主导（暂未明确），一期 RCP-Stub 骨架（动作账本+告警+探测开关默认关），对接后 RCP-Impl 替换（条件编译），上层零改动 |
| D-3 | taskId 传递 | **Want 参数统一拦截**（key 约定 `ohos.ai.taskId`），AI 助手调用侧无感知 |
| D-4 | 防仿冒 | **B-1 + B-2**：taskId 随机 128bit（UUID v4 风格）+ caller 身份绑定校验（callerTokenId ∈ {creator} ∪ {任务内进程 tokenId}）；仿冒拒绝不阻断启动、不报错（防探测） |
| D-5 | 接口面向 | 一期仅 **inner API**（ITaskManager IDL，放 `interfaces/inner_api/app_manager/task_manager/`），预留 NAPI 层 |
| D-6 | 状态机驱动 | AI 助手驱动 + 模块校验（流转合法性、终态不可回退、REJECTED 仅创建者可设） |
| D-7 | 交互传达 | 状态监听回调携带 payload（描述+可选项）→ 系统 UI 展示 → SubmitTaskInput 回传 |
| D-8 | RSS 管控粒度 | 按优先级分级（4 档）；管控维度含冻结/CPU 调度/内存/低内存查杀/功耗/温控（冻结一期主线，其余二期扩展） |
| D-9 | 目标版本 | 7.1 |
| D-10 | 节奏 | 先文档评审再写代码；当前暂不写代码，待设计评审正式通过 |

---

## 4. 核心技术设计（关键细节）

### 4.1 任务模型（design.md §4，枚举初稿待与 AI 助手团队对齐）

- **TaskState（9 态）**：UNKNOWN / SUBMITTED / WORKING / INPUT_REQUIRED / AUTH_REQUIRED / COMPLETED / CANCELED / FAILED / REJECTED
- **激活态集合** = {SUBMITTED, WORKING, INPUT_REQUIRED, AUTH_REQUIRED}（F-1 修正：SUBMITTED 纳入，否则锁屏定时任务就绪态被冻结后进不到 WORKING）；UNKNOWN 与终态非激活
- **TaskPriority（4 档 → 管控档位）**：LOW=0 不豁免 / MEDIUM=1 限时豁免(30min) / HIGH=2 激活态全时豁免 / CRITICAL=3 豁免+提权(二期)
- **TaskType（初稿）**：GENERAL / VOICE_PLAYBACK / TIMER_TASK / DATA_PROCESSING / SYNC_TRANSFER / INTERACTION / DOWNLOAD / CUSTOM
- **状态机流转**：见 design.md §4.2 流转表；终态不可回退；交互态必带 payload；REJECTED 仅创建者
- **TaskRecord**（≤1KB，内存态）：taskId / type / priority / state / payload / input / creatorTokenId / creatorUid / userId / processPids / createTime / updateTime / exemptRefCount

### 4.2 拦截点：条件两段式（design.md §5.4，第六轮修正）

```
taskId = want.GetStringParam("ohos.ai.taskId")   // 无 → 短路零开销
callerTokenId = 业务发起方（specifyTokenId / asCallerSourceToken 优先，否则 callerToken 解析）
targetPid = abilityRecord ? abilityRecord->GetPid() : 0
if (targetPid > 0)  → RegisterTaskProcess(taskId, pid, callerTokenId)   // 进程已存在：直接纳管
else                → RegisterTaskIntent(taskId, callerTokenId)          // 进程未创建：意向登记(30s 超时)
                      attach（pid 就绪）时补 RegisterTaskProcess
                      attach 失败 → UnregisterTaskIntent 主动清理（F-7）
```

**启动链路 12 场景覆盖**（design.md §5.4.1，含 file:line 证据）：
- 所有 Start 变体收敛 `StartAbilityInner`（ability_manager_service.cpp:1360）——含 FreeInstall(:876) / WithSpecifyTokenId(:1013) / AsCaller(:1195) 代理启动（校验身份取业务发起方）
- Connect 收敛 `AbilityConnectManager`；意图/Cli/A2A 天然覆盖（A2A 经 agentmgr→AMS ConnectAbilityWithExtensionType）
- 后台拉起（WorkScheduler）/卡片启动：无 taskId，**不纳管**（原则）
- 子进程派生（ChildProcess）：扩展场景（开放项 O-4）
- 已核实：`AbilityRecord::GetPid()` 可得（ability_connect_manager.cpp:327）

### 4.3 进程纳管原则（F-3）

纳管以**进程**为单位：进程加入任务后其内全部组件活动共享任务管控档位；taskId 仅用于"进程加入任务"触发点；多任务归属按 max 档位合并（`effectiveLevel(pid) = max(激活任务档位)`）。

### 4.4 ResourceControlProxy（design.md §6.4）

```
class ResourceControlProxy {
    ApplyControl(ControlRequest)      // 施加管控（ControlAction: EXEMPT/CLEAR_EXEMPT/THAW/SET_QOS…）
    RestoreControl(ControlRequest)    // 撤销（单进程）
    NotifyTaskEnd(TaskEndInfo)        // 任务级清理通知（RSS 侧清理任务级策略/账本）
    QueryControlState(pid, controlled)// 生效校验（F-4：施加后查询冻结状态，未生效告警）
    ResetAll(pid)                     // 恢复该进程全部管控
}
ControlRequest = {taskId, pid, uid, tokenId, bundleName, taskType, priority, taskState, level, reason, timestamp}  // 完整上下文供 RSS 定制策略与未来管控方复用
```

**任务结束双通道清理**：① 进程级 RestoreControl（逐进程，按剩余任务最高档重算）→ ② NotifyTaskEnd（任务级策略清理，防 RSS 残留）；串行执行，失败重试 ≤3 + 告警 + 保留账本。

### 4.5 RSS 维度细化（design.md §6.2）

| 维度 | 一期 | 二期 | 接口 |
|------|------|------|------|
| 冻结/解冻 | ✅ 主线 | — | 505 DYNAMICALLY_SET_SUSPEND_EXEMPT（白名单 res_sched_service.cpp:175）/ 517 THAW_ONE_APP / 506·507·518 查询 |
| CPU 调度(进程级) | 预留 | ✅ | 221 PROCESS_QOS_CHANGE（白名单 res_sched_service.cpp:151）/ 134 |
| CPU 调度(线程级) | — | ✅ | QosManager QosController/RTG（需线程 tid；qos_manager 仓=内核 sched_qos 封装，无 sessionQos 接口） |
| 内存/低内存查杀 | 评估预留 | ✅ | 520 SYNC_RES_TYPE_SHOULD_FORCE_KILL_PROCESS（lmkd 杀前同步查询，消费方语义待确认） |
| 功耗/温控 | 观察维度，不主动豁免 | 待评估 | 92/155/170/215、66/93/519 |

**sessionQos 结论**：全代码树无 "sessionQos" 接口；"会话级 QoS"语义由任务管控档位承载。

### 4.6 ITaskManager 接口（13 方法，design.md §5.2）

CreateTask / DestroyTask / CancelTask / SetTaskState / SetTaskParams / GetTaskInfo / GetTaskInfos / RegisterTaskListener / UnregisterTaskListener / SubmitTaskInput / RegisterTaskIntent / RegisterTaskProcess / UnregisterTaskProcess（后 3 个为 AMS 内部调用）

错误码：ERR_TASK_NOT_FOUND / ERR_TASK_INVALID_STATE / ERR_TASK_INVALID_STATE_TRANSITION / ERR_TASK_PERMISSION_DENIED / ERR_TASK_LIMIT_EXCEEDED（+ 复用 ERR_INVALID_PARAM）

### 4.7 DFX

- hisysevent 6 事件（AAFWK 域，hisysevent.yaml 新增）：TASK_CREATE / TASK_STATE_CHANGE / TASK_DESTROY / TASK_INPUT / TASK_RSSCONTROL / TASK_ABUSE_WARNING
- dump：hidumper -a 501（任务全景：状态/进程/档位/payload 摘要/账本）
- 日志：hilog_tag_wrapper

### 4.8 防滥用上限

创建者活跃任务 ≤16；任务进程集合 ≤64；豁免时长上限 60min；交互态超时 30min（F-5）；上限超限拒绝 + TASK_ABUSE_WARNING。

---

## 5. 代码勘察结论（源码事实，含 file:line）

| 事实 | 证据 |
|------|------|
| AppMS(501)/AMS(180)/agentmgr(185) 同在 foundation 进程 | `services/sa_profile/501.json`（libappms.z.so） |
| appmgr 已依赖 ressched_client | `services/appmgr/BUILD.gn:159` |
| agentmgr：AgentManagerService 继承 SystemAbility，IAgentManager.idl 15 接口 | `agent_runtime_framework/services/agentmgr/` |
| AMS→agentmgr 同进程调用先例 | `services/abilitymgr/src/ability_manager_service.cpp:6085`（VerifyAgentConnectRequest） |
| StartAbility 各变体收敛 StartAbilityInner | `ability_manager_service.cpp:1360`（定义），调用点 :807/:876/:1013/:1130/:1195/:2372/:14183/:14777 |
| AbilityRecord::GetPid() 可用 | `services/abilitymgr/src/ability_connect_manager.cpp:327` |
| appmgr 跟踪前后台切换（无冻结事件监控） | `services/appmgr/src/app_running_record.cpp:256/612/629`（SCHEDULE_FOREGROUNDING/BACKGROUNDING） |
| RSS ResSchedClient::ReportData / ReportSyncEvent | `resource_schedule_service/ressched/interfaces/innerkits/ressched_client/include/res_sched_client.h:69/79` |
| 505/221 在 RSS 服务端白名单 | `resource_schedule_service/ressched/services/resschedservice/src/res_sched_service.cpp:175/151` |
| QosManager = 内核 sched_qos 用户态封装 | `qos_manager/services/src/qos_interface.cpp:48`（/proc/thread-self/sched_qos_ctrl） |
| RSS 新增插件需评审内存基线（故倾向复用现有 resType） | `resource_schedule_service/Resource_Schedule_Service_Process_Management_Specifications_ZH.md` |

---

## 6. 仓库约束（AGENTS.md 关键项）

- `services/appmgr/` 与 `services/abilitymgr/` 为 🔴 高风险目录（修改影响所有应用进程/全局调度）→ 新模块独立目录、最小挂点；
- 不要绕过 `PermissionVerification`；AMS 不得直接调 AppMS 内部方法（经 `interfaces/inner_api/app_manager/` 客户端）；
- 不修改 IPC Stub/Proxy 既有序列化格式；新文件必须更新 BUILD.gn；
- `hisysevent.yaml` 已发布事件定义不可改（仅新增）；
- 特性开关在 `ability_runtime.gni`；
- 构建：`./build.sh --product-name <product> --build-target libappms`（从源码根目录执行）；
- 单测：`run -t UT -ts task_mgr_test`（拟建）。

## 7. 开放项（需外部对齐，不阻塞模块级开发）

| 编号 | 开放项 | 依赖方 |
|------|--------|--------|
| O-1 | 任务类型/优先级/状态枚举最终对齐 | AI 语音助手团队 |
| O-2 | 锁屏豁免场景边界（豁免覆盖哪些冻结类型、时长/数量上限；锁屏冻结 = suspend 冻结 + 可能叠加 standby 待机冻结） | RSS 团队（已搁置） |
| O-3 | RSS 真实对接（ResourceControlProxyImpl 落地） | RSS 团队 |
| O-4 | 子进程自动纳入任务是否纳入一期 | 需求方确认 |
| O-5 | 520 低内存查杀豁免消费方语义 | RSS 团队 |

## 8. 执行计划（execution-plan.md，待评审通过后启动）

| Task | 内容 | 规模预估 | 依赖 |
|------|------|----------|------|
| TASK-0 | 骨架：IDL/客户端/模块目录/BUILD | ~1,000 行 | 无 |
| TASK-1 | 任务模型与状态机（生命周期） | 生产 ~1,300 + 单测 ~800 | TASK-0 |
| TASK-2 | 查询/监听/交互传达 | 生产 ~900 + 单测 ~700 | TASK-1 |
| TASK-3 | 拦截纳管+防仿冒（条件两段式 + B-2） | 生产 ~950 + 集成 ~900 | TASK-1 |
| TASK-4 | 管控与 ResourceControlProxy（档位合并/豁免/双通道清理/上限） | 生产 ~1,700 + 单测 ~1,000 | TASK-1, TASK-3 |
| TASK-5 | DFX（6 事件/dump/错误码） | 生产 ~630 + 验证 ~300 | TASK-1~4 |
| TASK-6 | 验证收口（全量 AC + 真机锁屏/后台场景） | 用例补强 ~700 | TASK-1~5 + RSS 对接 |

合计 ≈ 1.1 万行（生产 ~6,600 + 单测 ~4,300），单人约 4~6 周。RSS 真实对接（RCP-Impl）另估 ~300~500 行。

## 9. 后续 AI 接手指引

1. **先读**：`.codespec/changes/ai-task-manager/proposal.md`（基线与决策记录）→ `design.md`（设计全貌）→ `spec.md`（AC/规则）→ `execution-plan.md`（任务分解）→ `review-analysis.md`（第七轮审查修正）
2. **再看图**：三张 SVG（总览/模块设计/流程）
3. **如需继续分析**：重点审查 design.md §4.1（枚举，待 AI 助手对齐）、§5.4（拦截点）、§6.4（RCP 对接策略）
4. **如需写代码**：确认设计评审通过后从 TASK-0 开始（IDL + 骨架），每 Task 的 Files/完成判据/验证命令见 execution-plan.md
5. **提交规范**：`-s` 签名 + `Co-Authored-By: Claude <noreply@anthropic.com>`；每阶段完成跑 ohos-sdd validate
6. **勿推翻的决策**：模块形态（AppMS）、防仿冒（B-1+B-2）、RCP 代理层、inner API——若要变更须与需求方重开澄清并更新 proposal.md（变更控制规则）

---

**文档/代码勘察可信度说明**：本交接文档中所有源码引用均经实地勘察（file:line 可复核）；google-adk 概念经官方文档调研（K-9）；RSS/qos_manager 结论经全树搜索（K-4/K-5/K-11）。唯一未证实项：RSS 505 payload 语义与豁免覆盖范围（RSS 团队对齐后确认）。
