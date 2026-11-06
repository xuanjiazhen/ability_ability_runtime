# 特性规格

> 固化用户可见行为和验收标准。复杂交互、状态机、异常流程可补充 Gherkin 场景。
>
> **THEN 边界：** 每条 AC 的 `THEN` 均能通过 System API 返回值、错误码或经任务 dump（hidumper）与调度策略查询、hilog 检索的外部可观察结果直接判定；不描述内部数据结构、状态机、调用链、锁或算法。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 任务优先级系统判定、多任务并发与进程多任务归属（按最高优先级） |
| 特性编号 | FEAT-AI-TASK-MGR-001（20260914692273） |
| 所属 Epic | FEAT-AI-TASK-MGR-001（RR 30096095） |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-7.1-Release |
| SIG 归属 | 元能力 SIG |
| 状态 | Approved（yangzk，2026-11-05） |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | 任务优先级系统判定（前后台状态 × 系统负载 → HIGH/MEDIUM/LOW 档位，创建时判定、运行中动态重判） | 系统内部行为，经任务 dump 与调度策略查询可观测 |
| ADDED | 多任务并发基线（≤10 并发任务，系统参数可配）与任务事件并发安全 | 异常返回 ERR_INVALID_OPERATION 经 JS 层映射对外可观察 |
| ADDED | 进程多任务归属合并（实际管控档位=纳管任务中最高档）、降级重算、死亡清理 | 经调度策略查询与任务 dump 可观测 |
| MODIFIED | startTask(taskId: string, priority?: TaskPriority) → startTask(taskId: string) | System API 签名变更（@since 26.1.0 版本内变更，无跨版本兼容负担）；IPC 接口与 NAPI/ETS 绑定同步调整 |
| REMOVED | TaskPriority 枚举不再对外暴露 | 无调用面影响（该枚举仅作 startTask 入参）；内部档位保留 HIGH/MEDIUM/LOW（100/200/300） |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved（2026-11-04，批准人 yangzk） |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。design.md 与本文档并行产出，互不依赖。

## 用户故事

### US-1: 任务优先级系统判定

**作为** AI 语音助手（系统应用，任务创建者），
**我想要** 创建任务时无需指定优先级、由系统根据我的前后台状态与系统负载自动判定任务档位（运行中随状态/负载变化动态调整），
**以便** 任务管控强度与系统当前状态匹配、我无需感知系统负载细节。

典型场景：AI 语音助手经 startTask 创建任务 T1（调用方不指定优先级）：T1 创建时系统感知到助手处于前台且系统负载低，判定为 HIGH 档位并立即激活；任务运行中助手切至后台或系统负载升高，系统按映射矩阵动态重判档位（如降为 MEDIUM/LOW），纳管进程的实际管控档位随之重算；档位变化全程经任务 dump 与调度策略查询可观测。

**验收标准（AC）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-6.5 | WHEN AI 语音助手（系统应用）以合法 taskId（1~256 字符）调用不含优先级入参的 startTask THEN 调用成功（Promise 完成），系统根据其前后台状态与系统负载自动判定任务档位（HIGH/MEDIUM/LOW 之一），任务按判定档位立即激活，经任务 dump 与调度策略查询可观测 | System API（返回值）/ 任务 dump / 调度策略查询 | 正常 |
| AC-6.6 | WHEN AI 语音助手前后台状态与系统负载的不同组合（前台/后台 × 负载高/低）分别创建任务 THEN 各任务判定档位符合设计映射矩阵 | System API / 任务 dump / 调度策略查询 | 正常 |
| AC-6.7 | WHEN 任务运行中 AI 语音助手前后台状态或系统负载变化 THEN 系统按映射矩阵动态重判任务档位，进程实际管控档位随任务档位与归属实时重算（取纳管任务中最高档），经任务 dump 与调度策略查询可观测 | System API / 任务 dump / 调度策略查询 | 正常 |

### US-2: 多任务并发与进程多任务归属

**作为** AI 语音助手（系统应用，任务创建者），
**我想要** 同一进程被我的多个任务纳管时系统按最高优先级任务管控该进程（最高优先级任务结束后按次高优先级续管、全部任务结束后完全恢复原调度状态），
**以便** 多任务并发下进程调度行为可预期、不发生管控冲突与豁免泄漏。

典型场景：语音助手经 startTask 创建任务 T1（系统判定 HIGH）与 T2（系统判定 LOW），两者经纳管链路纳管同一进程 P，P 按 HIGH 档位受管控；T1 结束后 P 按 T2 的 LOW 档位续管；T2 结束后 P 完全恢复原调度状态。全程经任务 dump 与调度策略查询可观测。

**验收标准（AC）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-6.1 | WHEN 同一进程被多个任务纳管且各任务档位不同 THEN 管控执行时该进程按其中最高档位任务执行管控（实际档位=纳管任务中最高档） | System API / 调度策略查询 / 任务 dump | 正常 |
| AC-6.2 | WHEN 进程被多任务纳管且最高档任务结束 THEN 进程降至次高档位任务的管控档位；WHEN 该进程的全部纳管任务结束 THEN 进程完全恢复原调度状态（资源调度服务按原策略自然回冻） | System API / 调度策略查询 / 任务 dump | 正常 |
| AC-6.3 | WHEN 任务内纳管进程死亡（进程退出/被强制终止）且系统经既有进程死亡通知链感知（延迟 ≤1s）THEN 该进程从任务记录移除并释放其豁免，其余进程管控不受影响，任务不自动结束（创建者可继续纳管新进程） | System API / 任务 dump / 调度策略查询 | 异常 |
| AC-6.4 | WHEN 10 个并发任务（含同进程多归属组合）并发执行创建/纳管/结束操作 THEN 任务事件并发处理，无管控档位错配、无豁免泄漏，多任务/多进程矩阵单测与集成用例通过 | System API / 任务 dump / 调度策略查询 / hilog | 边界 |

## 验收追溯

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-6.5 | R-1、R-7 | System API 返回值（Promise 完成）；任务 dump 显示判定档位；调度策略查询显示档位 |
| AC-6.6 | R-1 | System API；任务 dump 与调度策略查询显示各组合判定档位符合映射矩阵 |
| AC-6.7 | R-1、R-2 | System API；任务 dump 与调度策略查询显示重判后任务档位与进程实际档位（取最高档） |
| AC-6.1 | R-2、R-4 | System API；调度策略查询与任务 dump 显示进程实际档位=纳管任务最高档 |
| AC-6.2 | R-3 | System API；调度策略查询与任务 dump 显示降次高/完全恢复的档位序列 |
| AC-6.3 | R-5、R-8 | System API；任务 dump 显示进程移出且任务存续；调度策略查询显示豁免释放（≤1s） |
| AC-6.4 | R-6 | System API；任务 dump、调度策略查询与 hilog 事件序列无竞态、无档位错配、无豁免泄漏 |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | AI 语音助手（系统应用）以合法 taskId 调用 startTask（无优先级入参）；任务运行中创建者前后台状态或系统负载变化 | 创建时按前后台状态 × 系统负载映射矩阵判定档位（HIGH/MEDIUM/LOW 之一）；运行中按同一矩阵动态重判；任务按判定档位立即激活；档位经任务 dump 与调度策略查询可观测 | 映射矩阵、评估时机、负载量化与防抖策略见 design.md；并发任务数 ≤10 | AC-6.5、AC-6.6、AC-6.7 |
| R-2 | 行为 | 同一进程被多个任务纳管且各任务档位不同；或任务档位变化 | 进程实际管控档位 = 纳管任务中最高档；档位变化触发归属重算，重算不引入管控空窗 | 归属关系为内存态，不持久化 | AC-6.1、AC-6.7 |
| R-3 | 行为 | 多归属进程的最高档任务结束；或全部纳管任务结束 | 最高档任务结束后进程降至次高档位任务档位；无次高任务则完全恢复原调度状态（自然回冻，无需显式重新冻结） | 降级重算在任务结束事件处理中完成 | AC-6.2 |
| R-4 | 行为 | 多任务对同一进程申请同类型豁免 | 豁免以最高档任务为准；同进程同类型豁免幂等（幂等键=任务标识+进程标识+豁免类型；类型维度随 20260914692272 的豁免类型体系落地，当前阶段按任务标识+进程标识计数，单类型占位） | 幂等防止重复申请 | AC-6.1 |
| R-5 | 恢复 | 任务内纳管进程退出或被强制终止 | 系统感知（≤1s）后该进程从任务记录移除并释放其豁免；其余进程管控不受影响；任务不自动结束 | 与创建者死亡自动结束区分：进程死≠任务死 | AC-6.3 |
| R-6 | 边界 | 并发任务数达上限（默认 10，系统参数可配）；10 并发任务并发创建/纳管/结束 | 达上限后再创建任务返回失败（对外可观察错误）；并发操作无管控档位错配、无豁免泄漏 | 任务记录 ≤1KB/任务 | AC-6.4 |
| R-7 | 异常 | 任务创建或动态重判时，前后台状态或系统负载感知失败（监听注册失败或回调数据获取失败） | 按设计定义的兜底档位判定，任务正常创建/存续 | 兜底档位取值在 design.md 定义 | AC-6.5、AC-6.7 |
| R-8 | 边界 | 任务内全部纳管进程死亡 | 任务不自动结束，记录保留至创建者结束/任务超时（自最后一次添加纳管进程起算、无纳管进程时自创建起算，默认 600s，系统参数可配）/创建者死亡 | 超时口径沿用 20260914692191 | AC-6.3 |

## 验证映射

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1 / AC-6.5 | 单测：`test/unittest/task_mgr/StartTaskAutoJudgeUnit_0100`（代表性接线用例：前台+负载2 → 断言任务记录档位=HIGH 并立即激活；全矩阵覆盖由 VM-2 承担，本用例不重复）；集成：`test/integration/` StartTaskPriorityAutoJudge_0100（系统应用 demo 调用无 priority 入参的 startTask） | 单测+集成+demo | 未实现前 startTask 声明含 priority 入参/档位固定为默认 MEDIUM，dump 无判定档位字段 | 单测断言任务记录档位精确等于判定值；demo 调用成功（Promise 完成），任务 dump 与调度策略查询显示判定档位 |
| VM-2 | R-1 / AC-6.6 | `test/unittest/task_mgr/` PriorityJudgeMatrix_0100（构造前台/后台 × 融合档位低（0~4)/高（5~7）组合创建任务，含边界档位 4/5） | 单测 | 未实现前判定结果无状态差异（各组合档位相同），矩阵断言失败 | 各组合判定档位与设计映射矩阵一致（前台=HIGH；后台×0~4=MEDIUM；后台×5~7=LOW；兜底 MEDIUM） |
| VM-3 | R-1、R-2 / AC-6.7 | 单测：`test/unittest/task_mgr/` PriorityRejudgeOnStateChange_0100（场景矩阵见表后附注：前后台切换 × 融合档位跳变的变化前→后组合，含档位不变防抖与多任务共管重算）；集成：`test/integration/` PriorityRejudgeIntegration_0100（代表场景经 demo + dump 复核） | 单测+集成 | 未实现前任务档位静止不重判，重判断言失败 | 单测断言各场景变化后任务档位与进程实际档位（取最高档）符合映射；档位不变场景断言无重算传播；集成代表场景 dump 观测一致 |
| VM-4 | R-2、R-4 / AC-6.1 | 单测：`test/unittest/process_schedule/` MultiTaskHighestPriority_0100（3 个不同档位任务纳管同一进程）；集成：`test/integration/` MultiTaskHighestPriorityIntegration_0100（真实纳管链路 + dump 断言 effectiveLevel） | 单测+集成 | 未实现前进程档位错配（非最高档），断言失败 | 单测断言进程实际档位=纳管任务最高档；集成经 dump 复核 effectiveLevel 一致 |
| VM-5 | R-3 / AC-6.2 | 单测：`test/unittest/process_schedule/` DegradeToNextPriority_0100（最高档位任务结束）；集成：`test/integration/` DegradeToNextPriorityIntegration_0100（dump 断言降级序列与完全恢复） | 单测+集成 | 未实现前最高档结束后档位滞留，降级断言失败 | 档位降至次高；全部结束完全恢复（集成经 dump 复核档位序列） |
| VM-6 | R-5 / AC-6.3 | 单测：`test/unittest/process_schedule/` ProcessDeathCleanupUnit_0100（模拟死亡事件，断言索引移除+豁免释放+重算+任务存续）；集成：`test/integration/` ProcessDeathCleanup_0100（纳管进程 exit） | 单测+集成 | 未实现前任务误结束或豁免滞留 | 单测断言死亡后索引/豁免/重算正确且任务存续；集成 ≤1s 清理完成且任务存续；dump 显示进程移出 |
| VM-7 | R-6 / AC-6.4 | 单测：`test/unittest/process_schedule/` Concurrent10Tasks_0100（10 并发任务全操作矩阵）；集成：并发链路用例 | 并发单测+集成 | 未实现前出现竞态（档位错配/豁免泄漏） | 矩阵用例通过，hilog 事件序列无竞态告警 |
| VM-8 | R-7 / AC-6.5、AC-6.7 | `test/unittest/task_mgr/` PriorityJudgeFallback_0100（mock 感知失败：监听注册失败/回调数据获取失败） | 单测 | 未实现前感知失败导致任务创建失败 | 按兜底档位（MEDIUM）判定，任务正常创建/存续 |
| VM-9 | R-8 / AC-6.3 | 单测：`test/unittest/process_schedule/` AllProcessesDeadTaskAliveUnit_0100（任务内全部进程死亡，断言任务存续与超时窗口口径）；集成：`test/integration/` AllProcessesDeadTaskAlive_0100 | 单测+集成 | 未实现前任务随进程死亡自动结束 | 单测断言任务存续、超时口径正确；集成记录任务存续 |

> **VM-3 单测场景矩阵（变化前 → 触发 → 变化后，任务运行中）**：
>
> | # | 变化前（任务档位） | 触发 | 变化后（期望任务档位 / 进程 effectiveLevel） |
> |---|--------------------|------|------------------------------|
> | 1 | 前台 HIGH | 切后台，融合档位 0~4 | MEDIUM / 按共管任务重算 |
> | 2 | 前台 HIGH | 切后台，融合档位 5~7 | LOW / 按共管任务重算 |
> | 3 | 后台 MEDIUM（负载 0~4） | 切前台 | HIGH / 立即升档 |
> | 4 | 后台 LOW（负载 5~7） | 切前台 | HIGH / 立即升档 |
> | 5 | 后台 MEDIUM | 融合档位 4→5（跳高） | LOW / 重算 |
> | 6 | 后台 LOW | 融合档位 5→4（跳低） | MEDIUM / 重算 |
> | 7 | 前台 HIGH | 融合档位 4→5 | 仍 HIGH（前台与负载无关）/ **防抖：断言无重算传播** |
> | 8 | 后台 MEDIUM | 融合档位 0→4 | 仍 MEDIUM / **防抖：断言无重算传播** |
> | 9 | 多任务共管同一 pid，任一任务重判 | 任务档位变化 | 进程 effectiveLevel=max（存活任务档位）：存在其他 HIGH 则不变；最后一个 HIGH 降档则降至次高 |
>
> 场景 7/8 验证 design ADR-6 防抖（档位不变不重算）；场景 9 验证 spec R-2 归属重算。集成用例选取场景 1/3/5/7 为代表经 dump 复核。

## API 变更分析

### 新增 API

无新增接口（优先级判定为系统内部行为，不新增查询/监听接口）。

### 变更/废弃 API

| API 名称 | 当前开放级别 | 目标开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|--------------|--------------|----------|----------|----------|---------|
| startTask(taskId: string, priority?: TaskPriority) → startTask(taskId: string) | System | System | 变更（参数移除） | AI 语音助手创建任务时不再传入优先级；调用方传入 priority 实参按 401 参数校验失败处理 | 调用方移除 priority 实参；无其他适配 | AC-6.5 |
| TaskPriority | System | System | 废弃（不再对外暴露，不出现在 d.ts 公共接口面） | 无调用面影响（该枚举仅作 startTask 入参） | 调用方不再引用该枚举 | AC-6.5 |

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源（文件:行/符号或 New 依据） | 关联 AC |
|--------|----------|----------|---------------|----------|----------|--------------------------------------|---------|
| startTask(taskId: string, priority?: TaskPriority) → startTask(taskId: string) | Existing API | System | 变更前：`startTask(taskId: string, priority?: TaskPriority): Promise<void>`（priority 缺省按 MEDIUM=200 处理）；变更后：`startTask(taskId: string): Promise<void>` | 系统应用创建任务 | Promise 完成/拒绝 | 变更前：`frameworks/ets/ets/@ohos.app.agent.taskManager.ets:29` `startTask`；变更后：New，依据 proposal.md「API 变更项清单」/AC-6.5 | AC-6.5 |
| TaskPriority | Existing API | System | LOW=100、MEDIUM=200、HIGH=300；目标：废弃，不再出现在 d.ts 公共接口面 | 作 startTask 入参 | 枚举值 | `frameworks/ets/ets/@ohos.app.agent.taskManager.ets:21-25` `TaskPriority`；废弃依据 proposal.md「API 变更项清单」 | AC-6.5 |
| ERR_PARAM_INVALID（401） | Existing Error Code | System | 十进制 401 | startTask 入参不合法：taskId 非 1~256 字符 | Promise 拒绝，BusinessError.code=401 | `services/appmgr/src/task_mgr/task_manager_service.cpp:643` `CheckParam`（taskId 1~256，否则 INVALID_PARAMETERS_ERR）；401 对外映射沿用 20260914692191 | AC-6.5 |
| ERR_NOT_SYSTEM_APP（202） | Existing Error Code | System | 十进制 202 | 调用方非系统应用（系统应用身份校验失败） | Promise 拒绝，BusinessError.code=202 | `interfaces/inner_api/ability_manager/include/ability_manager_errors.h:309` `ERR_NOT_SYSTEM_APP`；对外值 202 沿用 20260914692191 | AC-6.5 |
| ERR_TASK_ID_EXISTS（16000301） | Existing Error Code | System | 十进制 16000301 | taskId 对应任务已处于存活期，再次调用 startTask | Promise 拒绝，BusinessError.code=16000301，原任务不受影响 | 内部 ERR_TASK_ALREADY_EXISTS=2099501（`interfaces/inner_api/ability_manager/include/ability_manager_errors.h:1206`）；对外值 16000301 沿用 20260914692191 | AC-6.5 |

## 接口规格

### 接口定义

**startTask**

| 属性 | 值 |
|------|-----|
| 函数签名 | `startTask(taskId: string): Promise<void>` |
| 返回值 | `Promise<void>` — resolve 无载荷；失败经 BusinessError reject（错误码见 API 与错误码事实） |
| 开放级别 | System（@systemapi） |
| 错误码 | 202（非系统应用调用方）、401（参数校验失败）、16000301（taskId 已存在） |
| 关联 AC | AC-6.5 |

**公共规格属性**：@since 26.1.0（dynamic&static，沿用 20260914692191）；@syscap SystemCapability.Ability.AgentRuntime.Core；无 ohos.permission 权限项（经系统应用身份校验）；@stagemodelonly；不支持跨平台/元服务/卡片。

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| taskId | string | 是 | 无 | 1~256 字符（UTF-8 字节数），归因主键，任务存活期内全局唯一；跨应用 taskId 互不可见 |
| （额外入参） | — | 否 | — | 调用方多传 priority 等额外入参被忽略（与 finishTask 的参数解析口径一致：仅末位函数作为异步回调）；档位由系统判定，入参不影响判定结果 |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | 系统应用以合法 taskId 调用 | System API / 任务 dump | Promise 完成；系统按前后台状态与系统负载判定档位（HIGH/MEDIUM/LOW），任务按判定档位立即激活 | AC-6.5 |
| 2 | 存活期同 taskId 重复调用 | System API | Promise 拒绝（BusinessError.code=16000301），原任务不受影响 | AC-6.5 |
| 3 | 非系统应用调用 | System API | Promise 拒绝（BusinessError.code=202） | AC-6.5 |
| 4 | taskId 非 1~256 字符 | System API | Promise 拒绝（BusinessError.code=401） | AC-6.5 |
| 5 | 并发任务数达上限（默认 10）后再创建 | System API | 创建失败（对外可观察错误），既有任务不受影响 | AC-6.4 |

## 兼容性声明

- **已有 API 行为变更:** 是（startTask 移除 priority 可选入参、TaskPriority 枚举不再对外暴露；均为 @since 26.1.0 新引入接口在本版本内的变更，无已发布版本兼容负担）
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否（归属关系不持久化）
- **最低支持版本:** 26.1.0
- **API 版本号策略:** @since 26.1.0 不变（版本内签名变更）

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 复用 20260914692191 接口契约 | startTask 鉴权（系统应用校验）、taskId 规格（1~256 字符）、超时口径（默认 600s）与错误码沿用；仅移除 priority 入参 | AC-6.5 |
| IPC 兼容边界 | 仅 startTask 签名同步变更；不修改 IAppMgr 其他既有接口与序列化格式 | AC-6.5 |
| 最小挂点 | 进程死亡感知复用既有通知链，不新增侵入钩子；管控档位施加/撤销经 20260914692272 RSS 机制 | AC-6.3 |
| 判定参数边界 | 前后台状态 × 系统负载 → 档位的映射矩阵、动态重判评估时机、负载量化与防抖策略、兜底档位均在 design.md 定义并评审 | AC-6.5~AC-6.7 |
| 权限边界 | 不绕过 PermissionVerification；不新增鉴权点 | AC-6.5 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 任务创建与运行中动态重判在事件链路内同步完成，不引入可感知时延（前后台状态与系统负载为本地/低频查询） | 创建/重判链路时延打点 | 实现阶段回填 |
| 并发 | ≤10 并发任务（系统参数可配）；并发操作无管控档位错配、无豁免泄漏 | 并发单测（矩阵用例） | 实现阶段回填 |
| 内存 | 任务记录 ≤1KB/任务；归属关系内存态维护 | 内存打点 | 实现阶段回填 |
| 可靠性 | 纳管进程死亡清理延迟 ≤1s；豁免无泄漏（任务结束后登记清零） | 集成用例 + 时延打点 | 实现阶段回填 |
| 可测试性 | 档位、归属与降级经任务 dump（hidumper）与调度策略查询可观测；hilog 统一 TAG 可检索关键路径 | 单测/集成 | 实现阶段回填 |
| 定界定位 | 优先级判定/归属合并/降级重算/进程清理关键路径日志经统一 TAG 检索 | hilog | 实现阶段回填 |

## 多设备适配声明

无差异（优先级判定、并发与归属均为系统机制，与设备形态无关；全系具备 SystemCapability.Ability.AgentRuntime.Core 的设备行为一致）。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | N/A | 管控链路无 UI 文案 | — |
| 大字体 | N/A | 管控链路无 UI 文案 | — |
| 深色模式 | N/A | 管控链路无 UI | — |
| 多窗口/分屏 | N/A | 与窗口机制无关 | — |
| 多用户 | 否 | 任务归属创建者 userId，沿用 20260914692191 口径，本需求不改变 | 任务创建/结束 |
| 版本升级 | 否 | 归属关系内存态不持久化，无存量数据 | — |
| 生态兼容 | 否 | startTask 为 @since 26.1.0 System API，版本内变更无生态负担 | — |

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "services/appmgr 既有进程死亡/状态通知链（AppStateObserverManager/AppDeathRecipient）的挂点方式"
  - repo: "openharmony/ability_ability_runtime"
    query: "task_mgr 既有记录表锁域（tasksMutex_）与事件并发处理先例（EventHandlerWrap）"
  - repo: "openharmony/ability_ability_runtime"
    query: "AppMgr 既有应用/进程前后台状态感知接口（AppRunningManager / 应用状态通知链）"
  - repo: "openharmony/ability_ability_runtime"
    query: "资源调度领域系统负载信息获取接口（设计阶段与 resourceschedule SIG 确认）"
```

**关键文档：** 需求前置分析 `20260914692273/design.md`（代码事实基线）；20260914692191 spec（startTask 基线契约）；20260914692272（档位与 RSS 机制）；20260914692442（任务管理 DFX 观测面）
