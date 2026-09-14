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
| Requirement | `.codespec/changes/ai-task-manager/proposal.md` | Approved（基线 v1.2，2026-09-14） |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。

## 用户故事

### US-1: 任务生命周期（最小接口）

**作为** AI 语音助手（系统应用）,
**我想要** 通过三个 System API（@systemapi）管理任务生命周期：startTask 创建并启动任务（指定类型、优先级、可选超时），finishTask/abortTask 结束任务,
**以便** 表达任务执行诉求，且系统可在超时或创建者死亡时自动收口（无任务状态机）.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN 合法调用方调用 startTask(type, priority[, timeoutSec][, payload]) THEN 返回成功与不可猜测的 taskId，任务立即激活（纳管进程开始按任务优先级受管控），超时计时启动，TASK_START 事件上报 | 正常 |
| AC-1.2 | WHEN 创建者调用 finishTask/abortTask 且任务存在 THEN 任务结束：纳管进程全部降级（豁免撤销/多任务归属按次高优先级续管）、任务记录清理、超时计时取消、TASK_END 事件（reason=FINISH/ABORT）上报 | 正常 |
| AC-1.3 | WHEN 任务存活时间超过超时时间（startTask 入参或系统默认，且不超过系统硬上限）THEN 系统自动结束任务（同 AC-1.2 清理路径）并上报 TASK_END（reason=TIMEOUT）与 TASK_ABUSE_WARNING 告警 | 异常 |
| AC-1.4 | WHEN 创建者进程死亡 THEN 其全部任务由系统自动结束（同清理路径，复用 AppDeathRecipient 链）并上报 TASK_END（reason=CREATOR_DEAD） | 异常 |
| AC-1.5 | WHEN 非任务创建者调用 finishTask/abortTask THEN 返回 ERR_TASK_PERMISSION_DENIED 且任务不受影响；不存在的 taskId（含跨用户）返回 ERR_TASK_NOT_FOUND | 异常 |
| AC-1.6 | WHEN 同一创建者活跃（未结束）任务数达到上限（可配，默认 16）THEN startTask 返回 ERR_TASK_LIMIT_EXCEEDED，不创建任务 | 边界 |
| AC-1.7 | WHEN 系统应用经 System API（@systemapi）调用 startTask/finishTask/abortTask THEN 接口可访问且行为与接口规格一致；三方应用（非系统应用）调用 THEN 返回权限错误（System API 访问控制） | 边界 |

### US-2: 工具链路纳管与进程归属（意图工具/CLI 工具/A2A）

**作为** AI 语音助手,
**我想要** 通过意图工具、CLI 工具、A2A 调用工具能力时携带 taskId，使目标进程自动纳入任务管控,
**以便** 对任务执行涉及的进程统一按任务优先级管控.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-2.1 | WHEN 任务处于存活期，任务创建者（或任务内进程）经意图工具/CLI 工具/A2A 调用工具能力且调用参数携带 taskId（CLI `ExecOptions.taskSessionId` / 意图 `ExecuteParam.taskSessionId` / A2A `A2ACallContext.taskSessionId`，task.md 4.2~4.4）THEN 系统侧校验通过（owner 归属+存活态），目标进程（含 CLI 工具进程）自动纳入任务管控 | 正常 |
| AC-2.2 | WHEN 调用携带伪造或冒用的 taskId（owner 不匹配、任务不存在或已结束/重放）THEN 拒绝执行该次工具调用并上报安全事件（fail-closed）；模型可控通道（execTool args / insightIntentParam / A2A 报文）中出现的任务标识被剥离后正常执行 | 异常 |
| AC-2.3 | WHEN 同一进程被多个任务纳管 THEN 该进程按最高优先级任务执行管控 | 边界 |
| AC-2.4 | WHEN 纳管进程死亡 THEN 该进程从任务记录移除，其余进程管控不受影响，任务不自动结束 | 异常 |
| AC-2.5 | WHEN 三链路调用参数未携带 taskSessionId 字段 THEN 调用行为与现网完全一致（存量调用方零适配） | 边界 |

### US-3: RSS 分级管控与降级恢复

**作为** 任务管理模块（系统）,
**我想要** 按任务优先级对纳管进程施加 RSS 管控（冻结豁免/解冻），并在任务结束（finish/abort/超时/创建者死亡）后恢复,
**以便** 保障锁屏/后台场景任务执行、不影响原有调度、后台执行不扰真实前台（虚拟屏调度抑制为一期分析+预留项，无一期 AC）.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-3.1 | WHEN 高优先级任务处于**存活期**（startTask~结束）THEN 任务管理模块经 RSS 管控层（ResourceControlProxy）向纳管进程下发豁免请求并记录日志（TASK_RSSCONTROL，模块级可验证）；施加后生效校验（查询冻结状态），未生效上报 TASK 告警；RSS 对接完成后端到端豁免生效，后台/锁屏场景进程不被冻结（TASK-6 门禁；锁屏端到端前提=任务数据锁屏可访问（EL3~EL5 外部依赖），未满足时仅验证冻结豁免） | 正常 |
| AC-3.2 | WHEN 任务结束（finishTask/abortTask/超时/创建者死亡）THEN 经 RSS 管控层撤销豁免并记录日志（TASK_RSSCONTROL），恢复任务前的调度状态（多任务归属时按次高优先级继续管控） | 正常 |
| AC-3.3 | WHEN 低优先级（LOW）任务 THEN 不豁免（与原系统调度行为一致）；时长约束由任务超时时间统一承载 | 边界 |
| AC-3.4 | WHEN 同一进程被多任务纳管且高优先级任务结束 THEN 降级至次高优先级任务的管控档位 | 边界 |
| AC-3.5 | WHEN 豁免数量或单任务纳管进程数超过配置上限 THEN 拒绝新增豁免并按策略降级（防滥用） | 边界 |

### US-4: DFX

**作为** 系统开发者,
**我想要** 通过 dump 与 hisysevent 查看任务执行事实,
**以便** 排查任务执行问题（DFX 为唯一可观测面）.

**验收标准（AC）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-4.1 | WHEN 执行任务相关 dump（hidumper/aa）THEN 输出活跃任务列表：taskId、类型、优先级、纳管进程（pid）、管控档位、剩余超时、创建时间（终态任务随结束即清理，不出现在 dump） | 正常 |
| AC-4.2 | WHEN 任务 start/结束（含超时/创建者死亡）/纳管/管控动作发生 THEN 上报对应 hisysevent 行为事件，可检索 | 正常 |


## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1 | TASK-lifecycle | 单测 | `[test/unittest/task_mgr/…]` |
| AC-1.2 | R-5, R-9 | TASK-lifecycle | 单测+集成 | `[…]` |
| AC-1.3 | R-25 | TASK-lifecycle | 单测 | `[…]` |
| AC-1.4 | R-26 | TASK-lifecycle | 单测+集成 | `[…]` |
| AC-1.5 | R-6 | TASK-lifecycle | 单测 | `[…]` |
| AC-1.6 | R-2 | TASK-lifecycle | 单测 | `[…]` |
| AC-1.7 | R-1, R-6 | TASK-lifecycle | 单测+API 用例 | `[…]` |
| AC-2.1 | R-7 | TASK-intercept | 集成 | `[…]` |
| AC-2.2 | R-8 | TASK-intercept | 集成 | `[…]` |
| AC-2.3 | R-10 | TASK-ressched | 单测 | `[…]` |
| AC-2.4 | R-12 | TASK-ressched | 单测+集成 | `[…]` |
| AC-2.5 | R-13 | TASK-intercept | 回归 | `[…]` |
| AC-3.1 | R-19 | TASK-ressched | 真机+集成 | `[…]` |
| AC-3.2 | R-9, R-20 | TASK-ressched | 集成+真机 | `[…]` |
| AC-3.3 | R-21 | TASK-ressched | 单测 | `[…]` |
| AC-3.4 | R-10, R-20 | TASK-ressched | 单测 | `[…]` |
| AC-3.5 | R-22 | TASK-ressched | 单测 | `[…]` |
| AC-4.1 | R-23 | TASK-dfx | 人工+单测 | `[…]` |
| AC-4.2 | R-24 | TASK-dfx | 集成 | `[…]` |

> Task 编号为占位，具体任务分解见 execution-plan.md；证据路径为占位，执行阶段填写。

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 系统应用调用 startTask，参数（type/priority/timeoutSec 可选/payload 可选）合法 | 返回 ERR_OK 与不可猜测 taskId（随机，≥128bit 熵）；任务立即激活（无状态字段，存活期= startTask 至结束）；按 priority 定档并启动超时计时；上报 TASK_START | type/priority 为枚举值，非法值返回 ERR_INVALID_PARAM；timeoutSec ≤ 系统硬上限（sysparam，默认 3600s），超限取硬上限值；payload ≤4KB（任务描述，供审计） | AC-1.1 |
| R-2 | 边界 | startTask 时该创建者活跃任务数达上限（可配，默认 16） | 返回 ERR_TASK_LIMIT_EXCEEDED，不创建任务 | **活跃 = 未结束任务**（startTask 后至 finish/abort/超时/创建者死亡前）；超时与创建者死亡自动结束（R-25/R-26）保证配额不滞留（防配额死锁） | AC-1.6 |
| R-5 | 行为 | 调用 finishTask/abortTask，任务存在且调用方为创建者 | 任务结束：双通道降级（R-9/R-20）+ 超时计时取消 + 任务记录清理（历史经 hisysevent 留痕）+ TASK_END(reason=FINISH/ABORT)；返回 ERR_OK | 幂等语义：任务已结束后记录即清理，二次调用返回 ERR_TASK_NOT_FOUND | AC-1.2 |
| R-6 | 异常 | 非任务创建者调用 finishTask/abortTask | 返回 ERR_TASK_PERMISSION_DENIED，任务不受影响 | 创建者为 startTask 时的调用方 tokenId/uid 绑定；跨用户 taskId 视为不存在（ERR_TASK_NOT_FOUND） | AC-1.5 |
| R-7 | 行为 | 任务存活期内，任务创建者（或任务内进程）经三链路调用携带 taskId——CLI `ExecOptions/ExecCmdOptions.taskSessionId`（签名不变）、意图 `ExecuteParam.taskSessionId`（系统保留字段，分发转写为 Want 兜底 `ohos.param.taskSessionId`）、A2A `A2ACallContext.taskSessionId`（带外上下文，不进协议报文） | 系统侧校验（owner 归属比对——IPC 取证身份，不可被参数伪造 + 存活态校验）通过后，目标进程（含 CLI 工具进程及其子进程）自动纳入任务；纳管不影响调用结果 | 通道隔离：taskId 仅走代码受控通道，模型可控通道（execTool args / insightIntentParam / A2A 报文 metadata·parts）出现任务标识一律剥离+安全日志；进程未创建时的补挂时机与内部纳管通道由 design.md 定稿 | AC-2.1 |
| R-8 | 异常 | 携带 taskId 但 owner 不匹配（冒用他人任务）、任务不存在或已结束（重放） | **fail-closed**：拒绝执行该次工具调用 + 安全事件上报（冒用为攻击信号）；会话不存在→拒绝并提示重建 | 分级处置（task.md §5.4）：owner 不匹配/终态重放 fail-closed；模型输出污染（标识出现在模型可控通道）→ 剥离后正常执行；taskId 不可猜测（R-1，系统 128-bit 签发） | AC-2.2 |
| R-9 | 行为 | 任务结束（finishTask/abortTask/超时 R-25/创建者死亡 R-26） | 对该任务下全部纳管进程执行降级：撤销冻结豁免（CLEAR_EXEMPT）/恢复调度优先级（CLEAR_QOS，二期 QoS 落地后）；进程归属按剩余任务最高优先级重算（无任务则完全恢复——RSS 按原策略自然回冻，无需显式 re-freeze）；**EDS 待机豁免按 uid 级引用计数撤销（该 uid 无其他存活期任务才 Unapply，F-47）**；任务级 NotifyTaskEnd 通知 RSS 清理任务级策略（R-20） | 降级在任务事件处理中同步执行；**结束同步取消超时计时（超时事件处理器校验任务仍存在，双保险）**；失败重试 ≤3 次后记录告警 | AC-1.2, AC-3.2 |
| R-10 | 边界 | 进程被多个任务纳管，各任务管控档位不同 | 进程实际管控档位 = 纳管它的任务中最高档位；高任务结束后降至次高档位 | 档位映射：任务优先级→管控档位表见 design.md（档位由 startTask 时的 priority 决定，无动态调档） | AC-2.3, AC-3.4 |
| R-11 | 边界 | 进程被多个任务纳管且最高档任务冻结豁免与次高任务冲突 | 豁免以最高档任务为准；其余任务豁免申请合并（同进程同类型豁免幂等） | 幂等键：taskId+pid+豁免类型 | AC-2.3 |
| R-12 | 异常 | 任务内纳管进程死亡（进程退出/被杀） | 该进程从任务记录移除，释放其豁免；任务其余进程不受影响；**任务不自动结束**（创建者可继续纳管新进程，结束仅由 finish/abort/超时/创建者死亡触发） | 进程死亡经 appmgr 既有死亡通知链驱动，延迟 ≤ 1s；**CLI 工具进程死亡经 cliManager 会话生命周期感知清理，与 AppMS 死亡链互补**；该任务全部纳管进程死亡且无其他存活期任务同 uid 时，EDS 待机豁免按 uid 引用计数撤销（F-47） | AC-2.4 |
| R-13 | 边界 | 三链路调用参数未携带 taskSessionId 字段 | 调用行为与现网完全一致；不产生任何纳管/管控副作用（存量调用方零适配） | 字段为既有接口的可选扩展，不改变函数签名 | AC-2.5 |
| R-19 | 行为 | 高优先级任务处于**存活期**（startTask 至结束），策略判定需豁免 | 经 RSS 管控层（ResourceControlProxy）对纳管进程下发冻结豁免请求并留存豁免登记（**内存态、不持久化、仅存活期任务**，含 taskId/pid/动作/档位/时间戳/结果——用于 RSS 重启重放与撤销重试，D-16；字段集对齐，F-17）；**施加后生效校验（查询冻结状态，未生效上报 TASK 告警）**；RSS 对接完成后豁免实际生效 | 优先级→豁免档位映射与豁免判定见 design.md 资源调度子模块；ResourceControlProxy 一期为骨架实现（内存登记+告警日志，探测开关默认关），对接策略见 design.md §6.4 | AC-3.1 |
| R-20 | 行为 | 任务结束（finish/abort/超时/创建者死亡） | **双通道清理**：① 进程级——经 ResourceControlProxy 对纳管进程撤销豁免/解冻并记录日志，进程按剩余任务最高档重算；② 任务级——NotifyTaskEnd(taskId, pids, reason) 通知 RSS 清理按任务建立的策略 | 两通道在 taskHandler 串行执行，②在①之后；失败重试 ≤3 次后告警（日志留痕供复核，D-16） | AC-3.2, AC-3.4 |
| R-21 | 边界 | 低优先级任务（LOW） | 不豁免（与原系统调度行为一致）；**时长约束由任务超时时间统一承载（R-25）**，全档位一致 | 档位映射见 design §6.1（LOW=档位 0 不豁免；MEDIUM/HIGH=豁免；CRITICAL=豁免+解冻+QoS 二期）；任务超时默认值与硬上限经 sysparam 配置（design §6.1，F-14/F-37） | AC-3.3 |
| R-22 | 边界 | 系统中活跃豁免数量或单任务纳管进程数超过配置上限 | 拒绝新增豁免，记录告警 hisysevent | 全局活跃豁免数量上限（默认 64 进程）与单任务纳管进程数上限（默认 16）——经可配参数调整（sysparam，design §6.1 F-14）；**豁免粒度按 pid（进程级）计**；时长约束由 R-25 任务超时统一承载 | AC-3.5 |
| R-23 | 行为 | 执行任务相关 dump（`hidumper -s 501 -a "<任务选项>"` / aa 命令；F-26 修正语法） | 输出**活跃任务列表**：taskId、类型、优先级、纳管进程（pid）、管控档位、剩余超时、创建时间（终态任务随结束即清理，不出现在 dump）；RSS 活跃豁免内存登记摘要 | dump 仅 shell/hidumper 可调用（复用既有校验） | AC-4.1 |
| R-24 | 行为 | 任务 start、结束（含超时/创建者死亡）、纳管、豁免施加/撤销 | 上报对应 hisysevent 行为事件（事件集=TASK_START/TASK_END/TASK_RSSCONTROL/TASK_ABUSE_WARNING，字段见 hisysevent.yaml 新增定义） | 事件定义不可破坏既有事件（AGENTS.md 约束） | AC-4.2 |
| R-25 | 行为 | 任务存活时间超过超时时间（startTask 入参 timeoutSec 或系统默认，均 ≤ 硬上限） | 系统自动结束任务：同 R-5 清理路径（双通道降级+计时取消+记录清理）+ TASK_END(reason=TIMEOUT) + TASK_ABUSE_WARNING(reason=timeout) | 超时默认值与硬上限经 sysparam（`persist.sys.appms.taskmanager.*`）可配；超时事件处理器校验任务仍存在（防与 finish/abort 竞态双保险） | AC-1.3 |
| R-26 | 行为 | 创建者进程死亡（AppMS 经既有 AppDeathRecipient 链感知） | 其全部任务由系统自动结束：同 R-5 清理路径 + TASK_END(reason=CREATOR_DEAD) | 感知延迟 ≤1s（复用进程死亡通知链）；防止创建者死亡后豁免滞留至超时（防配额/豁免滞留） | AC-1.4 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1~R-6, R-25, R-26 / AC-1.x | 单测（任务管理模块单测）+ 集成 | 任务生命周期（start/finish/abort）、超时自动结束、创建者死亡自动结束、权限校验（含 System API 访问控制）、任务上限 |
| VM-2 | R-7~R-13 / AC-2.x | 集成（三链路接口桩）+ 单测 | 三链路纳管正确性、冒用/重放拒绝（fail-closed）、多任务归属、进程死亡清理、未携带字段零影响 |
| VM-3 | R-19~R-22 / AC-3.x | 真机（锁屏/后台切换）+ 集成 | 豁免生效/撤销、多任务最高档、防滥用上限、降级恢复 |
| VM-4 | R-23~R-24 / AC-4.x | 人工 + 集成 | dump 输出、hisysevent 检索 |

## API 变更分析

> 公共接口 startTask/finishTask/abortTask 为新增 **SystemApi**（@systemapi 标注、@since 7.1，NAPI 绑定 + d.ts，面向系统应用，d.ts 在 interface_sdk_js 仓提审）；三链路纳管经既有系统接口可选字段扩展承载（签名不变，task.md 4.2~4.4）；系统侧内部纳管通道为内部实现，不在本表展开。方法签名与参数结构以 design.md 定稿为准（System API 签名以 d.ts 定稿为准）。

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `startTask`（System API；底层 `ITaskManager::startTask`） | SystemApi | TaskStartParams{type, priority, timeoutSec?, payload?} | taskId (string) | ERR_OK / ERR_INVALID_PARAM / ERR_TASK_LIMIT_EXCEEDED / ERR_TASK_PERMISSION_DENIED | 创建并启动任务（立即激活，启动超时计时），返回不可猜测 taskId | AC-1.1 |
| `finishTask`（System API；底层 `ITaskManager::finishTask`） | SystemApi | taskId | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED | 结束任务（正常完成），触发降级与记录清理 | AC-1.2 |
| `abortTask`（System API；底层 `ITaskManager::abortTask`） | SystemApi | taskId | ERR_OK | ERR_OK / ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED | 中止任务，触发降级与记录清理 | AC-1.2 |
| `cliManager.ExecOptions.taskSessionId`（CLI 链路扩展，task.md 4.2） | SystemApi（既有接口可选字段） | taskSessionId?: string（= startTask 签发 taskId） | — | 沿用 execTool 既有 | CLI 工具进程任务归因：执行前 owner+存活态校验，工具进程及子进程挂入任务；不传=现网行为 | AC-2.1 |
| `ExecuteParam.taskSessionId`（意图链路扩展，task.md 4.3） | SystemApi（系统保留字段） | taskSessionId?: string | — | 沿用 execute 既有 | 意图执行任务归因：转写为 Want 兜底 `ohos.param.taskSessionId`，目标进程挂入任务 | AC-2.1 |
| `A2ACallContext.taskSessionId`（A2A 链路扩展，task.md 4.4） | SDK 调用上下文（带外） | taskSessionId?, localTaskSessionId? | — | 沿用 connect/sendMessage 既有 | A2A 调用任务归因：端侧校验后 Agent 进程挂入任务；对端报文任务 ID 不采信（仅关联映射） | AC-2.1 |


### 变更/废弃 API

既有系统接口参数对象新增可选字段（CLI `ExecOptions`/`ExecCmdOptions`、意图 `ExecuteParam`、A2A 调用上下文 `A2ACallContext`）——**函数签名不变、不传字段行为与现网一致**（存量调用方零适配）；无既有 API 废止。

## 接口规格

> 每个接口的调用语义、参数约束与行为场景。实现细节（d.ts/桩位置、调用链）见 design.md。

### startTask

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t startTask(const TaskStartParams& params, std::string& taskId)`（native IDL）；System API 侧为 `startTask(params): Promise<string>` 风格（以 d.ts 定稿为准） |
| 返回值 | `int32_t` — 0=ERR_OK，负值为错误码 |
| 开放范围 | SystemApi（@systemapi 标注、@since 7.1；NAPI 绑定 + d.ts，经 TaskManagerClient→ITaskManager IDL 承载，面向系统应用） |
| 错误码 | ERR_INVALID_PARAM / ERR_TASK_LIMIT_EXCEEDED / ERR_TASK_PERMISSION_DENIED |
| 关联 AC | AC-1.1 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| params.type | TaskType 枚举 | 是 | — | 枚举值，非法值触发 ERR_INVALID_PARAM |
| params.priority | TaskPriority 枚举 | 是 | — | 枚举值，非法值触发 ERR_INVALID_PARAM |
| params.timeoutSec | int32 | 否 | 系统默认（sysparam） | ≤ 系统硬上限（默认 3600s，sysparam），超限取硬上限值 |
| params.payload | string/JSON | 否 | 空 | ≤4KB（任务描述，供审计/DFX） |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 系统应用调用，参数合法，未达任务数上限 | 返回 ERR_OK；taskId 为不可猜测随机串（≥128bit 熵）；任务立即激活（按 priority 定档、超时计时启动）；TASK_START 上报 | AC-1.1 |
| 2 | 参数非法（枚举越界/payload 超限） | 返回 ERR_INVALID_PARAM，不创建任务 | AC-1.1 |
| 3 | 创建者活跃任务数达上限 | 返回 ERR_TASK_LIMIT_EXCEEDED | AC-1.6 |
| 4 | 非系统应用调用 | 返回 ERR_TASK_PERMISSION_DENIED | AC-1.1 |

### finishTask / abortTask

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t finishTask(const std::string& taskId)` / `int32_t abortTask(const std::string& taskId)`（native IDL）；System API 侧为 `finishTask(taskId): Promise<void>` 风格（以 d.ts 定稿为准） |
| 返回值 | `int32_t` — 0=ERR_OK |
| 开放范围 | SystemApi（@systemapi 标注、@since 7.1；NAPI 绑定 + d.ts，经 TaskManagerClient→ITaskManager IDL 承载，面向系统应用） |
| 错误码 | ERR_TASK_NOT_FOUND / ERR_TASK_PERMISSION_DENIED |
| 关联 AC | AC-1.2, AC-1.5 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 任务存在，创建者调用 | 任务结束：双通道降级（R-9/R-20）+ 超时计时取消 + 记录清理；TASK_END(reason=FINISH/ABORT) 上报；返回 ERR_OK | AC-1.2 |
| 2 | 任务不存在（含已结束——终态记录即清理）或跨用户 | 返回 ERR_TASK_NOT_FOUND（幂等语义） | AC-1.5 |
| 3 | 非创建者调用 | 返回 ERR_TASK_PERMISSION_DENIED，任务不受影响 | AC-1.5 |

### 三链路 taskSessionId 字段扩展（CLI / 意图 / A2A，task.md 4.2~4.4）

| 属性 | 值 |
|------|-----|
| 扩展点 | ① CLI：`cliManager.ExecOptions.taskSessionId?`（`ExecCmdOptions` 同步增加同名字段，`execTool` 函数签名不变）；② 意图：`ExecuteParam.taskSessionId?`（系统保留字段，与 `insightIntentParam` 隔离——后者由模型模板渲染）；③ A2A：`A2ACallContext{ taskSessionId?, localTaskSessionId? }`（connect/sendMessage 可选调用上下文，带外传递、不进协议报文/metadata） |
| 字段语义 | 任务归因主键 = startTask 返回的系统签发 taskId（≥128bit 不可预测；应用自生成 ID 不具注册效力，系统不采信） |
| 开放范围 | SystemApi（各 Kit/SDK 既有接口的可选参数扩展，d.ts 在 interface_sdk_js 仓提审定稿） |
| 错误码 | 沿用各既有接口错误码；校验拒绝场景的错误定义以 d.ts 定稿为准 |
| 关联 AC | AC-2.1, AC-2.2, AC-2.5 |

**行为场景（系统侧统一校验，task.md §5 校验矩阵）**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 创建者（或任务内进程）在任务存活期经三链路调用携带 taskSessionId | owner 归属比对（IPC 取证身份）+ 存活态校验通过 → 目标进程（含 CLI 工具进程及其子进程）自动纳入任务管控；纳管不影响调用结果 | AC-2.1 |
| 2 | owner 不匹配（冒用他人 taskId） | 拒绝执行该次工具调用 + 安全事件上报（fail-closed） | AC-2.2 |
| 3 | taskId 不存在或任务已结束（重放） | 拒绝执行 + 提示重建 | AC-2.2 |
| 4 | 调用参数未携带 taskSessionId | 行为与现网完全一致（存量调用方零适配） | AC-2.5 |
| 5 | 模型可控通道（execTool args / insightIntentParam / A2A 报文 metadata·parts）出现任务标识字段 | 系统侧剥离/过滤覆写后正常执行 + 安全日志（通道隔离，防提示词注入） | AC-2.2 |

**通道与兜底约束**：taskSessionId 仅走代码受控通道（`ExecOptions` / `ExecuteParam` 系统保留键 / `A2ACallContext` / Want 兜底参数 `ohos.param.taskSessionId`）；Want 兜底字段系统侧校验"值与 caller 当前归因一致才采信，不一致丢弃该字段并以 IPC 取证为准"。进程未创建时的补挂时机与系统侧内部纳管通道（CLI/意图/A2A→AppMS）由 design.md 定稿，不在本规格接口面展开。

## 兼容性声明

- **已有 API 行为变更:** 否（三链路扩展为可选字段、签名不变，不传字段行为与现网一致，R-13；其余全部新增）
- **配置文件格式变更:** 否（新增配置项仅增不删，默认值不改变既有行为）
- **数据存储格式变更:** 否（任务不持久化，无存量数据）
- **最低支持版本:** 7.1
- **API 版本号策略:** 公共接口以 SystemApi 开放（@systemapi 标注、@since 7.1，d.ts 在 interface_sdk_js 仓提审）；内部 IDL 无 @since 约束（标注版本号 7.1 供内部追溯）

## 架构约束

> 约束的架构依据与设计方案见 design.md。

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅系统应用可调用任务接口 | 公共接口以 System API 开放（@systemapi 访问控制）+ 接口入口做基础身份校验（SA/系统应用 tokenId 校验），非系统应用返回 ERR_TASK_PERMISSION_DENIED | AC-1.1, AC-1.6, AC-1.7 |
| 纳管必须经 owner 归属校验 | 防仿冒（task.md 五道防线）：系统签发不可猜测 taskId + owner 归属比对（IPC 取证身份）+ 存活态校验（防重放）+ 通道隔离（模型可控通道禁止任务标识）；仅任务创建者或任务内进程可归因新进程 | AC-2.2 |
| 不修改 appmgr 既有 IPC 序列化 | 新增独立 ITaskManager IDL，不扩 IAppMgr | AC-2.x |
| 校验失败分级处置（fail-closed / fail-degrade） | 冒用/重放/会话不存在 → 拒绝执行 + 安全事件上报（fail-closed）；模型可控通道污染 → 剥离标识后正常执行；未携带字段 → 现网行为（fail-degrade，存量零适配） | AC-2.2, AC-2.5 |
| RSS 经 ResourceControlProxy 代理层对接 | 任务管理模块不直接依赖 RSS 接口细节；RSS 设计由 RSS 团队主导（暂未明确），一期 ResourceControlProxy 骨架实现（内存登记+告警日志，不持久化），对接后替换实现，上层零改动 | AC-3.x |
| 虚拟屏任务进程调度抑制为一期分析项 | 痛点：后台任务经虚拟屏执行被当前台调度，抢占真实前台（帧率/启动）；一期经资源调度子模块输出 134/221 等候选通道适用性分析与 RSS/窗口（屏幕会话）域对齐结论（开放项 O-8），ResourceControlProxy/档位扩展位预留，能力不满足列二期——**无一期 AC** | —（分析项） |
| DFX 事件定义不可破坏既有事件 | hisysevent.yaml 仅新增事件，不修改既有事件 | AC-4.2 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 任务操作（startTask/finishTask/abortTask）< 5ms（ITaskManager 跨进程 IPC，含 Binder 往返）；三链路校验只做身份比对与参数解析，附加开销不阻塞调用主流程（未携带字段时零开销） | 单测+benchmark | `[test/unittest/task_mgr/…]` |
| 并发 | 支持 ≥16 并发任务（配置上限默认 16）；任务事件处理串行（taskHandler，无状态机） | 并发单测 | `[…]` |
| 内存 | 任务记录基础字段 ≤1KB/任务（payload 为可选变长字段 ≤4KB 另计，任务描述供审计） | hidumper | `[…]` |
| 安全 | 任务 System API 仅系统应用可调用（@systemapi + 身份校验）；taskId 系统签发不可猜测；纳管经 owner 归属+存活态校验与通道隔离（task.md 五道防线） | 安全用例 | `[…]` |
| 可靠性 | 豁免撤销失败重试 ≤3 次并告警；进程死亡清理延迟 ≤1s；AppMS 异常重启后任务记录丢失可接受（foundation 重启清理全部应用进程，RSS 侧豁免随进程消亡，无孤儿豁免——C-1 结论） | 压力/故障注入 | `[…]` |
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
| 无障碍 | N/A | 任务机制无 UI 文案（呈现归 AI 助手自有 UI） | — |
| 大字体 | N/A | 同上 | — |
| 深色模式 | N/A | 同上 | — |
| 多窗口/分屏 | N/A（虚拟屏除外） | 任务按进程管控，与窗口形态无关；**虚拟屏形态相关痛点**：虚拟屏任务进程被当前台调度抢占真实前台——一期接口分析+扩展点预留（开放项 O-8），无一期行为变更 | — |
| 多用户 | 是 | 任务归属创建者 userId：跨用户 taskId 对 finishTask/abortTask 视为不存在（ERR_TASK_NOT_FOUND，R-6）；隔离语义收敛到归属校验（C-3） | AC-1.5 |
| 版本升级 | 是 | 全新增能力，无升级迁移；老版本无该接口不受影响 | — |
| 生态兼容 | 否 | 仅系统应用可用，不开放三方 | — |
