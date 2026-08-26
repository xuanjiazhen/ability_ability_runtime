# RSS 团队对齐问题清单（O-2/O-3/O-5/O-6）

> 用途：与 resource_schedule_service 团队的技术对齐材料 · 生成：2026-08-26 第十三轮
> 背景：ability_runtime 侧新增 AI 任务管理机制（并入 AppMS/SA 501），经 **ResourceControlProxy 代理层**（语义接口 ApplyControl/RestoreControl/NotifyTaskEnd/QueryControlState/ResetAll，携带 ControlRequest 完整上下文）对接 RSS；一期骨架实现（内存登记+日志+探测开关默认关），RSS 侧设计由 RSS 团队主导（proposal Q-12，2026-08-21 第四轮决策）。
> 本仓不修改 RSS 仓代码；以下问题按优先级排列，**P0 为 RCP-Impl 落地前置必答项**。

## P0：505 豁免通道语义（O-6，唯一"必须回答"项）

| # | 问题 | 我方假设/先例 | 需 RSS 确认 |
|---|------|--------------|-------------|
| 1 | `RES_TYPE_DYNAMICALLY_SET_SUSPEND_EXEMPT`(505) 的 payload 字段约定？ | 参照本仓先例（services/common/src/res_sched_util.cpp:228-234，THAW_ONE_APP payload 含 uid/pid/bundleName/reason）：拟携带 `uid + pid`（豁免粒度已决策为 pid，2026-08-24） | 字段名/必选性/是否有 clientId 或作用域字段 |
| 2 | 豁免的**施加/撤销**语义：value 取值（0/1？）？撤销是否即"恢复默认调度"？ | 拟 EXEMPT→value=1、CLEAR_EXEMPT→value=0，幂等重发安全——**注意：505 的 value 语义在 res_type.h 无任何文档（对比 503 有明确 value 0/1 注释），此为纯假设（F-35），必须确认** | 确认或给出正确约定 |
| 3 | 豁免**覆盖范围**：仅 RSS suspend 冻结，还是叠加待机（standby）冻结？（锁屏冻结 = suspend + 可能叠加 standby，proposal 基线条件 4） | 505 只覆盖 suspend 类；待机类经 device_standby ApplyAllowResource（P7 联动，design §6.1.1-7）已另行打通 | 505 与 standby 通道的边界确认；是否存在第三类冻结 |
| 4 | 豁免的时效语义：一次性申请后长期有效，还是需周期性重申（心跳）？ | 我方按"长期有效直至撤销"设计；RSS 重启后我方按内存登记幂等重放（design §6.1.1-5 P4） | 确认；RSS 重启后豁免状态是否确会清空（重放机制的前提） |
| 5 | 白名单与权限确认：505/517 上报的调用方要求 | **已实证（2026-08-26 第十四轮勘察）**：505/517 在 SA_RES 白名单（res_sched_service.cpp:145/:175）；上报要求 TOKEN_NATIVE + `ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT`（:43/:273-300）——foundation 进程 native token 已具备（AMS 既有 ResSchedUtil 上报先例佐证） | 确认无额外配置；**新增确认：507 查询与 RegisterSuspendObserver 注册（P5 冻结状态本地缓存通道）均需 `ohos.permission.GET_SUSPEND_STATE`（suspend_manager_base_service.cpp:27/:39/:103-111，F-33 扩大）——foundation native token 是否已配置该权限？未配置时的推荐路径（权限配置 vs 改走 ReportSyncEvent 直报）** |

## P0：517/507 配套语义

| # | 问题 | 我方假设/先例 | 需 RSS 确认 |
|---|------|--------------|-------------|
| 6 | `SYNC_RES_TYPE_THAW_ONE_APP`(517) payload 与同步返回值语义 | 本仓既有先例（res_sched_util.cpp:228-234）直接可仿；同步调用带超时保护（不持锁，design §6.1.1-6 P6） | 返回值结构/成功判定；已解冻进程重复 THAW 是否幂等 |
| 7 | `SYNC_RES_TYPE_GET_SUSPEND_STATE_BY_PID`(507) 返回结构 | {isFrozen}；master 上恒 false（suspend_manager_base_service.cpp:66-72，P1 勘察） | 产品环境返回结构确认；同步调用超时建议值 |
| 8 | **master 无消费插件问题（P1）**：开源 master 上 505/517/520 无执行器、507 恒 false——端到端验证会"假成功" | 我方已将"动作日志断言"定为 master 唯一验证基线；端到端验证前置=产品含 freezer 执行插件（启用开关以产品配置为准；TASK-6 门禁） | 该开关名称/作用域确认；哪些产品形态含执行器；是否有 master 可用的联调开关 |

## P1：对接方式（O-3）

| # | 问题 | 我方立场 |
|---|------|---------|
| 9 | RCP-Impl 对接形态：复用 505/517（我方代理层直接调 ressched_client），还是 RSS 提供新任务级接口？ | 均可接受——代理层隔离设计（ADR-4）保证上层零改动；若 RSS 倾向新接口，建议语义对齐 ControlAction（EXEMPT/CLEAR_EXEMPT/THAW/查询），并支持按 taskId 的任务级清理（对齐 NotifyTaskEnd） |
| 10 | 任务级策略清理：NotifyTaskEnd(taskId, pids, reason) 是否有 RSS 侧对应清理点（防残留）？ | 我方双通道清理（进程级 RestoreControl → 任务级 NotifyTaskEnd，串行，design §6.4）；需 RSS 确认任务级策略是否需要显式清理 |
| 11 | 冻结状态 push 通道：`RegisterSuspendObserver`（ISuspendStateObserverBase.idl，现网无消费者）可否供我方维护本地冻结缓存 + 登记漂移检测？ | design §6.4 P5 已计划二期采用；确认接口稳定性与订阅方权限 |
| 12 | 520 `SYNC_RES_TYPE_SHOULD_FORCE_KILL_PROCESS` 消费方语义（O-5，二期） | 我方二期拟让高优任务进程响应"不应杀"；需确认：事件触发时机（lmkd 杀前同步查询？）、我方返回"不应杀"后 lmkd 行为（跳过/延后/上报）；**已实证：520 上报需权限 5523（res_sched_service.cpp:214 权限映射表）——二期对接时需一并确认权限配置** |

## P2：锁屏豁免边界联合验收（O-2）

- EDS（device_standby）P7 联动已设计（ApplyAllowResource，IStandbyService.idl:22-25 已核实）：任务激活期间申请待机资源豁免，任务终态撤销——**联合调试验收待 device_standby 侧**，需 RSS 团队协同定义验收场景（夜间深度待机 + 定时任务唤起执行）。
- **新增确认（F-34，第十五轮实证）**：EDS AllowType 含 **FREEZE 位**（allow_type.h：NETWORK/RUNNING_LOCK/TIMER/WORK_SCHEDULER/AUTO_SYNC/PUSH/FREEZE）——待机**冻结**豁免可经 EDS 申请。我方 P7 现拟申请 NETWORK（+任务执行类）；**是否应一并申请 FREEZE 位以覆盖待机深度冻结？与 505 冻结豁免的边界如何划分（待机冻结归 EDS、suspend 冻结归 505？）**——此问题直接决定 O-2（锁屏冻结边界）的答案构成。
- 验收基线：以我方 TASK_RSSCONTROL 动作日志序列 == RSS 实际生效序列（对账基线，D-16，2026-08-24 决策）。

## 我方承诺（对齐后即可执行）

1. ControlRequest 携带完整上下文（taskId/pid/uid/tokenId/bundleName/taskType/priority/taskState/level/reason/timestamp），供 RSS 定制策略（design §6.4）；
2. 全部管控动作经 TASK_RSSCONTROL hisysevent + hilog 留痕（完整 ControlRequest 序列），对账/定位/验收信息完备；
3. RSS 重启重放、撤销失败重试 ≤3 + 告警、双通道清理等异常路径已在设计内（design §6.1.1）；
4. 不新增 RSS 插件/resType（一期），不改 RSS 仓代码。

---
*关联文档：design.md §6（资源调度子模块）/ §6.4（ResourceControlProxy）；review-analysis.md 开放项 O-2/O-3/O-5/O-6。*
