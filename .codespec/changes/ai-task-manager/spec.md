# 特性规格：AI 语音助手任务管理机制

> 固化用户可见行为和验收标准。任务状态机、异常流程、交互传达通过"规则定义"与"接口规格"承载。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | AI 语音助手任务管理机制（AI Task Manager） |
| 特性编号 | FEAT-AI-TASK-MGR-001 |
| 所属 Epic | 无（独立特性） |
| 优先级 | P1 |
| 目标版本 | 7.1 |
| SIG 归属 | ability（ability_runtime，AppFwk） |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

> lineage: new（全新特性），无存量行为变更，跳过。兼容性要求见"兼容性声明"。

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `.codespec/changes/ai-task-manager/proposal.md` | Approved（基线通过，2026-08-21 三轮确认） |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。

## 用户故事

### US-1: 任务创建与生命周期管理

**作为** AI 语音助手（系统应用）,
**我想要** 创建任务（指定类型、优先级、描述），设置任务状态，并在结束时销毁或取消任务,
**以便** 表达任务执行诉求并按任务级维度控制执行节奏.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN 合法调用方调用 CreateTask(type=语音播报, priority=HIGH) THEN 返回成功与不可猜测的 taskId，任务以 UNKNOWN 状态存在且可通过 GetTaskInfo 查询 | 正常 |
| AC-1.2 | WHEN 调用 SetTaskState(taskId, SUBMITTED→WORKING)（合法流转）THEN 状态更新成功且已注册监听者收到状态回调 | 正常 |
| AC-1.3 | WHEN 调用 SetTaskState 发起非法流转（终态回退、任意跳转、未创建任务）THEN 返回明确错误码且任务状态不变 | 异常 |
| AC-1.4 | WHEN 调用 CancelTask/DestroyTask 且任务存在 THEN 任务进入 CANCELED/被清理，纳管进程全部降级，任务记录不可再查询 | 正常 |
| AC-1.5 | WHEN 同一创建者连续创建任务至任务数上限 THEN 返回错误码（ERR_TASK_LIMIT_EXCEEDED），不创建新任务 | 边界 |
| AC-1.6 | WHEN 非任务创建者调用 CancelTask/DestroyTask/SetTaskState THEN 返回权限错误码且任务不受影响 | 异常 |

### US-2: 启动链路纳管与进程归属

**作为** AI 语音助手,
**我想要** 通过 StartAbility/ConnectAbility/意图框架/Cli/A2A 启动能力时携带 taskId，使目标进程自动纳入任务管控,
**以便** 对任务执行涉及的进程统一按任务优先级管控.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-2.1 | WHEN 任务存在且任务创建者（或其进程）发起启动、Want 携带 taskId THEN 目标进程注册进该任务（GetTaskInfo 可见纳管 pid） | 正常 |
| AC-2.2 | WHEN 非任务创建者、非任务内进程发起启动且 Want 携带伪造 taskId THEN 拒绝纳管（返回成功但该进程不纳入任务，启动本身不受影响） | 异常 |
| AC-2.3 | WHEN 同一进程被多个任务纳管 THEN 该进程按最高优先级任务执行管控 | 边界 |
| AC-2.4 | WHEN 纳管进程死亡 THEN 该进程从任务记录移除，其余进程管控不受影响 | 异常 |
| AC-2.5 | WHEN 启动的 Want 未携带 taskId THEN 启动行为与改动前完全一致（零影响） | 边界 |

### US-3: 任务状态查询与监听

**作为** 系统 UI（系统应用）,
**我想要** 监听任务状态变化并查询任务列表/详情,
**以便** 向用户展示任务执行进度与交互信息.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-3.1 | WHEN 系统 UI 注册任务监听（RegisterTaskListener）THEN 任务状态变化（含进入 INPUT_REQUIRED/AUTH_REQUIRED 的 payload）实时回调 | 正常 |
| AC-3.2 | WHEN 系统 UI 注销监听 THEN 不再收到回调 | 正常 |
| AC-3.3 | WHEN 查询 GetTaskInfos/GetTaskInfo THEN 返回状态、纳管进程、payload、创建/更新时间戳 | 正常 |
| AC-3.4 | WHEN 查询不存在的 taskId THEN 返回 ERR_TASK_NOT_FOUND | 异常 |

### US-4: INPUT/AUTH 交互传达

**作为** AI 语音助手与用户,
**我想要** 在 INPUT_REQUIRED/AUTH_REQUIRED 阶段通过任务传达描述与可选项，并回传用户选择,
**以便** 完成用户授权或意图确认.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-4.1 | WHEN 任务进入 INPUT_REQUIRED/AUTH_REQUIRED 且携带描述与可选项 payload THEN 监听者（系统 UI）收到包含该 payload 的回调并可展示 | 正常 |
| AC-4.2 | WHEN 用户选择后调用 SubmitTaskInput(taskId, input) THEN 返回成功，AI 助手（创建者）通过查询/监听获得 input，任务可流转出交互状态 | 正常 |
| AC-4.3 | WHEN 任务在交互状态被取消/销毁 THEN 交互状态清理，不再回调，任务进入终态 | 异常 |
| AC-4.4 | WHEN 非任务创建者调用 SubmitTaskInput THEN 返回权限错误码 | 异常 |

### US-5: RSS 分级管控与降级恢复

**作为** 任务管理模块（系统）,
**我想要** 按任务优先级对纳管进程施加 RSS 管控（冻结豁免/解冻），并在任务结束/进程退出后恢复,
**以便** 保障锁屏/后台场景任务执行且不影响原有调度.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-5.1 | WHEN 高优先级任务进入**激活态**（SUBMITTED/WORKING/INPUT_REQUIRED/AUTH_REQUIRED，策略判定激活）THEN 任务管理模块经 RSS 管控层（ResourceControlProxy）向纳管进程下发豁免请求并记录账本（模块级可验证）；施加后生效校验（查询冻结状态），未生效上报 TASK 告警；RSS 对接完成后端到端豁免生效，后台/锁屏场景进程不被冻结（TASK-6 门禁） | 正常 |
| AC-5.2 | WHEN 任务结束/取消/销毁 THEN 经 RSS 管控层撤销豁免并记录账本，恢复任务前的调度状态（多任务归属时按次高优先级继续管控） | 正常 |
| AC-5.3 | WHEN 低优先级任务或任务处于**非激活态**（UNKNOWN/终态）THEN 不豁免；MEDIUM 任务按配置限时豁免 | 边界 |
| AC-5.4 | WHEN 同一进程被多任务纳管且高优先级任务结束 THEN 降级至次高优先级任务的管控档位 | 边界 |
| AC-5.5 | WHEN 豁免数量/时长超过配置上限 THEN 拒绝新增豁免并按策略降级（防滥用） | 边界 |

### US-6: DFX

**作为** 系统开发者,
**我想要** 通过 dump 与 hisysevent 查看任务状态与事件,
**以便** 排查任务执行问题.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-6.1 | WHEN 执行任务相关 dump（hidumper/aa）THEN 输出任务列表、状态、纳管进程、管控生效信息 | 正常 |
| AC-6.2 | WHEN 任务创建/状态流转/结束 THEN 上报对应 hisysevent 行为事件，可检索 | 正常 |

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1, R-2 | TASK-create-task | 单测+集成 | `[test/unittest/task_mgr/…]` |
| AC-1.2 | R-3 | TASK-state-machine | 单测 | `[…]` |
| AC-1.3 | R-4 | TASK-state-machine | 单测 | `[…]` |
| AC-1.4 | R-5, R-9 | TASK-lifecycle | 单测+集成 | `[…]` |
| AC-1.5 | R-2 | TASK-create-task | 单测 | `[…]` |
| AC-1.6 | R-6 | TASK-lifecycle | 单测 | `[…]` |
| AC-2.1 | R-7 | TASK-intercept | 集成 | `[…]` |
| AC-2.2 | R-8 | TASK-intercept | 集成 | `[…]` |
| AC-2.3 | R-10 | TASK-process-mapping | 单测 | `[…]` |
| AC-2.4 | R-12 | TASK-process-mapping | 单测+集成 | `[…]` |
| AC-2.5 | R-13 | TASK-intercept | 回归 | `[…]` |
| AC-3.1 | R-14 | TASK-listener | 单测+集成 | `[…]` |
| AC-3.2 | R-14 | TASK-listener | 单测 | `[…]` |
| AC-3.3 | R-15 | TASK-listener | 单测 | `[…]` |
| AC-3.4 | R-15 | TASK-listener | 单测 | `[…]` |
| AC-4.1 | R-16 | TASK-interaction | 集成 | `[…]` |
| AC-4.2 | R-17 | TASK-interaction | 集成 | `[…]` |
| AC-4.3 | R-18 | TASK-interaction | 单测 | `[…]` |
| AC-4.4 | R-6 | TASK-interaction | 单测 | `[…]` |
| AC-5.1 | R-19 | TASK-ressched | 真机+集成 | `[…]` |
| AC-5.2 | R-9, R-20 | TASK-ressched | 集成+真机 | `[…]` |
| AC-5.3 | R-21 | TASK-ressched | 单测 | `[…]` |
| AC-5.4 | R-10, R-20 | TASK-ressched | 单测 | `[…]` |
| AC-5.5 | R-22 | TASK-ressched | 单测 | `[…]` |
| AC-6.1 | R-23 | TASK-dfx | 人工+单测 | `[…]` |
| AC-6.2 | R-24 | TASK-dfx | 集成 | `[…]` |

> Task 编号为占位，具体任务分解见 execution-plan.md；证据路径为占位，执行阶段填写。

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 系统应用调用 CreateTask，参数（type/priority/payload）合法 | 返回 ERR_OK 与不可猜测 taskId（随机，≥128bit 熵）；任务以 UNKNOWN 状态创建；同创建者可查询 | type/priority 为枚举值，非法值返回 ERR_INVALID_PARAM | AC-1.1 |
| R-2 | 边界 | 创建任务时系统中该创建者活跃任务数达到上限（可配，默认 16） | 返回 ERR_TASK_LIMIT_EXCEEDED，不创建任务 | 上限为配置项，默认 16；上限 0 表示禁用创建 | AC-1.1, AC-1.5 |
| R-3 | 行为 | 调用 SetTaskState 发起合法流转（按状态机表，如 SUBMITTED→WORKING） | 状态更新成功返回 ERR_OK；监听者收到回调（携带 payload） | 状态机合法流转表见 design.md | AC-1.2 |
| R-4 | 异常 | 调用 SetTaskState 发起非法流转（终态回退、未注册流转、REJECTED 由非创建者设置） | 返回 ERR_TASK_INVALID_STATE_TRANSITION，任务状态不变 | 终态（COMPLETED/CANCELED/FAILED/REJECTED）不可再流转 | AC-1.3 |
| R-5 | 行为 | 调用 CancelTask/DestroyTask，任务存在且调用方为创建者 | CancelTask：任务进入 CANCELED（非终态时）并触发降级；DestroyTask：任务记录清理并触发降级；返回 ERR_OK | DestroyTask 对任意状态可执行（幂等：不存在返回 ERR_TASK_NOT_FOUND） | AC-1.4 |
| R-6 | 异常 | 非任务创建者调用 SetTaskState/CancelTask/DestroyTask/SubmitTaskInput | 返回 ERR_TASK_PERMISSION_DENIED，任务不受影响 | 创建者为 CreateTask 时的调用方 tokenId/uid 绑定 | AC-1.6, AC-4.4 |
| R-7 | 行为 | 启动链路（StartAbility/ConnectAbility/意图/Cli/A2A，含代理启动变体）请求中 Want 携带 taskId，且业务发起方为任务创建者或任务内已纳管进程 | 目标进程注册进任务（**条件两段式**：进程已存在→直接注册；未创建→意向登记+attach 补注册），GetTaskInfo 可见；纳管成功不影响启动结果 | 校验身份取业务发起方（asCallerSourceToken/specifyTokenId 优先）；任务不存在时忽略 taskId，不报错不阻断 | AC-2.1 |
| R-8 | 异常 | Want 携带 taskId，但发起方既非任务创建者也非任务内进程（伪造/仿冒） | 拒绝纳管：启动正常执行但进程不纳入任务；不上报错误给调用方 | 校验基于发起方 tokenId/uid 与任务创建者绑定；taskId 不可猜测（R-1） | AC-2.2 |
| R-9 | 行为 | 任务进入终态（COMPLETED/CANCELED/FAILED/REJECTED）或 DestroyTask | 对该任务下全部纳管进程执行降级：撤销豁免/恢复 QoS；进程归属按剩余任务最高优先级重算（无任务则完全恢复） | 降级在状态机事件处理中同步执行；失败重试 ≤3 次后记录告警 | AC-1.4, AC-5.2 |
| R-10 | 边界 | 进程被多个任务纳管，各任务管控档位不同 | 进程实际管控档位 = 纳管它的任务中最高档位；高任务结束后降至次高档位 | 档位映射：任务优先级→管控档位表见 design.md | AC-2.3, AC-5.4 |
| R-11 | 边界 | 进程被多个任务纳管且最高档任务冻结豁免与次高任务冲突 | 豁免以最高档任务为准；其余任务豁免申请合并（同进程同类型豁免幂等） | 幂等键：taskId+pid+豁免类型 | AC-2.3 |
| R-12 | 异常 | 任务内纳管进程死亡（进程退出/被杀） | 该进程从任务记录移除，释放其豁免；任务其余进程与任务状态不受影响 | 进程死亡经 appmgr 既有死亡通知链驱动，延迟 ≤ 1s | AC-2.4 |
| R-13 | 边界 | 启动请求 Want 未携带 taskId，或携带非法格式 taskId | 启动行为与改动前完全一致；不产生任何纳管/管控副作用 | 拦截仅做参数解析与校验，失败不阻断启动 | AC-2.5 |
| R-14 | 行为 | 调用 RegisterTaskListener（系统应用），任务状态发生变更 | 监听者回调收到变更事件（含 taskId、新状态、payload、时间戳）；UnregisterTaskListener 后停止回调 | 回调事件顺序与状态变更顺序一致；监听回调不阻塞状态机 | AC-3.1, AC-3.2 |
| R-15 | 行为 | 调用 GetTaskInfo(taskId)/GetTaskInfos() | 返回任务详情（状态、类型、优先级、纳管进程、payload、时间戳）/全部任务列表；不存在的 taskId 返回 ERR_TASK_NOT_FOUND | 查询对系统应用开放；返回数据为快照 | AC-3.3, AC-3.4 |
| R-16 | 行为 | 任务进入 INPUT_REQUIRED/AUTH_REQUIRED，payload 含描述（text）与可选项（options 数组） | 状态回调携带完整 payload；监听者（系统 UI）可展示并可调用 SubmitTaskInput 回传 | payload 大小上限可配（默认 4KB） | AC-4.1 |
| R-17 | 行为 | 用户选择后调用 SubmitTaskInput(taskId, input)，调用方为创建者，任务处于交互状态 | 返回 ERR_OK；input 持久到任务记录，创建者通过监听/查询获取；任务可流转出交互状态 | 非交互状态调用返回 ERR_TASK_INVALID_STATE；input 大小上限可配（默认 4KB） | AC-4.2 |
| R-18 | 异常 | 任务处于交互状态时被取消/销毁 | 交互状态清理（payload/input 清除），不再回调，任务进入终态 | 清理在状态机终态处理中同步完成 | AC-4.3 |
| R-19 | 行为 | 高优先级任务处于**激活态**（SUBMITTED/WORKING/INPUT_REQUIRED/AUTH_REQUIRED），策略判定需豁免 | 经 RSS 管控层（ResourceControlProxy）对纳管进程下发冻结豁免请求并留存豁免账本（含 pid/动作/档位/时间戳/结果）；**施加后生效校验（查询冻结状态，未生效上报 TASK 告警）**；RSS 对接完成后豁免实际生效 | 优先级→豁免档位映射与豁免判定见 design.md 资源调度子模块；ResourceControlProxy 一期为骨架实现（账本+告警，探测开关默认关），对接策略见 design.md §6.4；激活态集合含 SUBMITTED（F-1：就绪态保障可调度） | AC-5.1 |
| R-20 | 行为 | 任务降级/结束（终态/DestroyTask） | **双通道清理**：① 进程级——经 ResourceControlProxy 对纳管进程撤销豁免/解冻并记录账本，进程按剩余任务最高档重算；② 任务级——NotifyTaskEnd(taskId, pids, reason) 通知 RSS 清理按任务建立的策略/账本 | 两通道在 taskHandler 串行执行，②在①之后；失败重试 ≤3 次后告警并保留账本供复核 | AC-5.2, AC-5.4 |
| R-21 | 边界 | 低优先级任务（LOW）或任务处于非激活态（UNKNOWN/终态） | 不豁免（与原系统调度行为一致）；MEDIUM 任务限时豁免（时长可配，默认 30min） | 交互态属激活态且配置交互超时（默认 30min，超时触发策略动作防无限挂起占用）；豁免时长上限可配（默认 60min） | AC-5.3 |
| R-22 | 边界 | 系统中活跃豁免数量或单任务豁免时长超过配置上限 | 拒绝新增豁免，任务降级为不豁免档，记录告警 hisysevent | 豁免数量上限可配（默认 64 进程）；时长上限见 R-21 | AC-5.5 |
| R-23 | 行为 | 执行任务相关 dump（hidumper -a 501 / aa 命令） | 输出任务列表：taskId、状态、类型、优先级、纳管进程（pid）、管控档位、payload 摘要、时间戳 | dump 仅 shell/hidumper 可调用（复用既有校验） | AC-6.1 |
| R-24 | 行为 | 任务创建、状态流转、终态、豁免施加/撤销 | 上报对应 hisysevent 行为事件（事件名/字段见 hisysevent.yaml 新增定义） | 事件定义不可破坏既有事件（AGENTS.md 约束） | AC-6.2 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1~R-6 / AC-1.x | 单测（任务管理模块单测）+ 集成 | 任务生命周期、状态机流转合法性、权限校验 |
| VM-2 | R-7~R-13 / AC-2.x | 集成（AMS 拦截点桩）+ 单测 | 纳管正确性、仿冒拒绝、多任务归属、进程死亡清理、无 taskId 零影响 |
| VM-3 | R-14~R-15 / AC-3.x | 单测 + 集成 | 监听回调顺序/负载、查询快照 |
| VM-4 | R-16~R-18 / AC-4.x | 集成 | payload 传达、SubmitTaskInput 回传、交互清理 |
| VM-5 | R-19~R-22 / AC-5.x | 真机（锁屏/后台切换）+ 集成 | 豁免生效/撤销、多任务最高档、防滥用上限、降级恢复 |
| VM-6 | R-23~R-24 / AC-6.x | 人工 + 集成 | dump 输出、hisysevent 检索 |

## API 变更分析

> 全部为新增 InnerApi（无 Public/System API 变更），方法签名与参数结构以 design.md 定稿为准。

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `ITaskManager::CreateTask` | InnerApi | TaskCreateParams{type, priority, payload} | taskId (string) | ERR_OK / ERR_INVALID_PARAM / ERR_TASK_LIMIT_EXCEEDED / ERR_TASK_PERMISSION_DENIED | 创建任务，返回不可猜测 taskId | AC-1.1 |
| `ITaskManager::DestroyTask` | InnerApi | taskId | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED | 销毁任务（任意状态可执行），触发降级 | AC-1.4 |
| `ITaskManager::CancelTask` | InnerApi | taskId | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED / ERR_TASK_INVALID_STATE（终态时） | 取消任务，任务进入 CANCELED 并触发降级 | AC-1.4 |
| `ITaskManager::SetTaskState` | InnerApi | taskId, state, payload | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_INVALID_STATE_TRANSITION / ERR_TASK_PERMISSION_DENIED / ERR_INVALID_PARAM | 设置任务状态（校验流转合法性） | AC-1.2, AC-1.3 |
| `ITaskManager::SetTaskParams` | InnerApi | taskId, params | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED / ERR_INVALID_PARAM | 更新任务参数（优先级/类型可调，受限规则见 design） | AC-1.2 |
| `ITaskManager::GetTaskInfo` | InnerApi | taskId | TaskInfo | ERR_OK / ERR_TASK_NOT_FOUND | 查询任务详情 | AC-3.3 |
| `ITaskManager::GetTaskInfos` | InnerApi | 无 | List\<TaskInfo\> | ERR_OK | 查询全部任务 | AC-3.3 |
| `ITaskManager::RegisterTaskListener` | InnerApi | listener (IRemoteObject) | ERR_OK | ERR_OK / ERR_INVALID_PARAM | 注册任务状态监听（系统 UI） | AC-3.1 |
| `ITaskManager::UnregisterTaskListener` | InnerApi | listener | ERR_OK | ERR_OK / ERR_INVALID_PARAM | 注销任务监听 | AC-3.2 |
| `ITaskManager::SubmitTaskInput` | InnerApi | taskId, input | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_INVALID_STATE / ERR_TASK_PERMISSION_DENIED / ERR_INVALID_PARAM | 提交交互输入（用户选择/授权结果） | AC-4.2 |
| `ITaskManager::RegisterTaskProcess`（内部） | InnerApi | taskId, pid/token | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED | AMS 拦截点调用：将目标进程纳入任务 | AC-2.1 |
| `ITaskManager::UnregisterTaskProcess`（内部） | InnerApi | taskId, pid | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND | 进程死亡/任务移除时解除纳管 | AC-2.4 |

### 变更/废弃 API

无（全部新增，无既有 API 变更/废弃）。

## 接口规格

> 每个接口的调用语义、参数约束与行为场景。实现细节（d.ts/桩位置、调用链）见 design.md。

### CreateTask

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t CreateTask(const TaskCreateParams& params, std::string& taskId)` |
| 返回值 | `int32_t` — 0=ERR_OK，负值为错误码 |
| 开放范围 | InnerApi |
| 错误码 | ERR_INVALID_PARAM / ERR_TASK_LIMIT_EXCEEDED / ERR_TASK_PERMISSION_DENIED |
| 关联 AC | AC-1.1 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| params.type | TaskType 枚举 | 是 | — | 枚举值，非法值触发 ERR_INVALID_PARAM |
| params.priority | TaskPriority 枚举 | 是 | — | 枚举值，非法值触发 ERR_INVALID_PARAM |
| params.payload | string/JSON | 否 | 空 | ≤4KB；超限 ERR_INVALID_PARAM |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 系统应用调用，参数合法，未达任务数上限 | 返回 ERR_OK；taskId 为不可猜测随机串（≥128bit 熵）；任务 UNKNOWN 状态创建 | AC-1.1 |
| 2 | 参数非法（枚举越界/payload 超限） | 返回 ERR_INVALID_PARAM，不创建任务 | AC-1.1 |
| 3 | 创建者活跃任务数达上限 | 返回 ERR_TASK_LIMIT_EXCEEDED | AC-1.5 |
| 4 | 非系统应用调用 | 返回 ERR_TASK_PERMISSION_DENIED | AC-1.1 |

### SetTaskState

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t SetTaskState(const std::string& taskId, TaskState state, const std::string& payload)` |
| 返回值 | `int32_t` — 0=ERR_OK |
| 开放范围 | InnerApi |
| 错误码 | ERR_TASK_NOT_FOUND / ERR_TASK_INVALID_STATE_TRANSITION / ERR_TASK_PERMISSION_DENIED / ERR_INVALID_PARAM |
| 关联 AC | AC-1.2, AC-1.3 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| taskId | string | 是 | — | 已存在任务，否则 ERR_TASK_NOT_FOUND |
| state | TaskState 枚举 | 是 | — | 目标状态；从当前状态的合法流转表（design.md）校验 |
| payload | string/JSON | 否 | 空 | 交互状态（INPUT/AUTH）必填（描述+可选项），≤4KB |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 合法流转（如 SUBMITTED→WORKING） | 返回 ERR_OK；监听者收到回调 | AC-1.2 |
| 2 | 非法流转（终态回退/任意跳转） | 返回 ERR_TASK_INVALID_STATE_TRANSITION，状态不变 | AC-1.3 |
| 3 | 设置 REJECTED 且调用方非创建者 | 返回 ERR_TASK_PERMISSION_DENIED（REJECTED 仅创建者可设） | AC-1.3 |
| 4 | 任务进入交互状态但 payload 缺失 | 返回 ERR_INVALID_PARAM（交互状态必须携带描述与可选项） | AC-4.1 |

### CancelTask / DestroyTask

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t CancelTask(const std::string& taskId)` / `int32_t DestroyTask(const std::string& taskId)` |
| 返回值 | `int32_t` — 0=ERR_OK |
| 开放范围 | InnerApi |
| 错误码 | ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED / ERR_TASK_INVALID_STATE（CancelTask 于终态） |
| 关联 AC | AC-1.4 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 任务存在且非终态，创建者调用 CancelTask | 任务进入 CANCELED；纳管进程降级；监听者收到回调 | AC-1.4 |
| 2 | 任务已终态，调用 CancelTask | 返回 ERR_TASK_INVALID_STATE（幂等策略见 design） | AC-1.4 |
| 3 | 任意状态调用 DestroyTask | 任务清理；降级；返回 ERR_OK | AC-1.4 |
| 4 | 任务不存在 | 返回 ERR_TASK_NOT_FOUND | AC-1.4 |

### RegisterTaskProcess（内部，AMS 拦截点调用）

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t RegisterTaskProcess(const std::string& taskId, int32_t pid)` |
| 返回值 | `int32_t` — 0=ERR_OK |
| 开放范围 | InnerApi（AMS→AppMS 内部通道） |
| 错误码 | ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED |
| 关联 AC | AC-2.1, AC-2.2 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| taskId | string | 是 | — | 任务存在；调用方身份（AMS 转发的发起方 tokenId）需通过 B-2 校验 |
| pid | int32 | 是 | — | 目标进程 pid（>0） |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 发起方为任务创建者或任务内进程，taskId 存在 | 目标 pid 注册进任务；返回 ERR_OK | AC-2.1 |
| 2 | 发起方身份校验不通过（仿冒） | 拒绝纳管，返回 ERR_TASK_PERMISSION_DENIED；启动不受影响 | AC-2.2 |
| 3 | 进程已在任务中（重复纳管） | 幂等成功 | AC-2.1 |

## 兼容性声明

- **已有 API 行为变更:** 否（全部新增；无 taskId 的启动行为与改动前一致，R-13）
- **配置文件格式变更:** 否（新增配置项仅增不删，默认值不改变既有行为）
- **数据存储格式变更:** 否（任务不持久化，无存量数据）
- **最低支持版本:** 7.1
- **API 版本号策略:** 新增 InnerApi 按 @systemapi 标注（inner 层无 @since 约束，标注版本号 7.1 供内部追溯）

## 架构约束

> 约束的架构依据与设计方案见 design.md。

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅系统应用可调用任务接口 | 接口入口做基础身份校验（SA/系统应用 tokenId 校验），非系统应用返回 ERR_TASK_PERMISSION_DENIED | AC-1.1, AC-1.6, AC-4.4 |
| 纳管必须经 caller 身份绑定校验 | 防仿冒：仅任务创建者或任务内进程可携带 taskId 纳管新进程（B-2） | AC-2.2 |
| 不修改 appmgr 既有 IPC 序列化 | 新增独立 ITaskManager IDL，不扩 IAppMgr | AC-2.x |
| 拦截失败不阻断启动 | Want 解析/校验失败仅不纳管，启动照常 | AC-2.2, AC-2.5 |
| RSS 经 ResourceControlProxy 代理层对接 | 任务管理模块不直接依赖 RSS 接口细节；RSS 设计由 RSS 团队主导（暂未明确），一期 ResourceControlProxy 骨架实现（账本+告警），对接后替换实现，上层零改动 | AC-5.x |
| DFX 事件定义不可破坏既有事件 | hisysevent.yaml 仅新增事件，不修改既有事件 | AC-6.2 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 任务创建/状态流转/查询响应 < 5ms（同进程 IN_PROCESS 调用）；纳管拦截附加开销不阻塞启动主流程（无 taskId 时零开销） | 单测+benchmark | `[test/unittest/task_mgr/…]` |
| 并发 | 支持 ≥16 并发任务（配置上限默认 16）；监听回调不阻塞状态机 | 并发单测 | `[…]` |
| 内存 | 任务记录轻量（≤1KB/任务） | hidumper | `[…]` |
| 安全 | 任务接口仅系统应用可调用；taskId 不可猜测；纳管需 caller 校验 | 安全用例 | `[…]` |
| 可靠性 | 豁免撤销失败重试 ≤3 次并告警；进程死亡清理延迟 ≤1s；AppMS 异常重启后任务记录丢失可接受（进程管控自动失效） | 压力/故障注入 | `[…]` |
| 可测试性 | 全部规则可经单测/集成用例验证；状态机可注入式测试 | 单测 | `[…]` |
| 自动化维测 | 任务创建/流转/终态/豁免施加撤销全覆盖 hisysevent，可自动化检索 | 自动化测试 | `[…]` |
| 定界定位 | dump 输出任务全景（状态/进程/档位/payload 摘要）；错误码可区分任务域问题 | hilog+dump | `[…]` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | — | 真机用例 | `[…]` |
| 平板 | 无差异 | — | — | `[…]` |
| 折叠屏 | 无差异 | — | — | `[…]` |

> 任务管理为系统服务机制，与设备形态无关；锁屏/后台场景验证以手机为主（锁定验证项见 design.md）。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | N/A | 任务机制无 UI 文案；系统 UI 展示侧归 UI 团队 | — |
| 大字体 | N/A | 同上 | — |
| 深色模式 | N/A | 同上 | — |
| 多窗口/分屏 | N/A | 任务按进程管控，与窗口形态无关 | — |
| 多用户 | 是 | 任务按 userId 维度隔离（创建者归属用户），跨用户不可见/不可操作；设计阶段细化 | AC-1.6 |
| 版本升级 | 是 | 全新增能力，无升级迁移；老版本无该接口不受影响 | — |
| 生态兼容 | 否 | 仅系统应用可用，不开放三方 | — |
