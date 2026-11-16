# 特性规格

> 固化调度代理外部可观察行为和验收标准。具体内部接口名称、签名、数据结构、状态机、计时器与并发实现归 design.md；真实 RSS 调控效果、性能功耗和资源配额不属于本需求。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 任务进程 RSS 分级调度决策与代理调用（含防抖合并） |
| 特性编号 | FEAT-AI-TASK-MGR-001（20260914692272） |
| 所属 Epic | FEAT-AI-TASK-MGR-001（RR 30096095） |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-7.1-Release |
| SIG 归属 | 元能力 SIG |
| 状态 | Approved（yangzk，2026-11-16） |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | 每次进程优先级重算对应的完整事件上下文、START/END 动作及触发原因 | 通过代理事件和关键路径日志观察 |
| ADDED | 新建/升档立即通知、降档及非死亡结束固定 1000ms 合并、纳管进程死亡立即结束 | 只验证事件动作、顺序和最终状态，不验证真实 RSS 调控效果 |
| MODIFIED | 调度代理由仅记录 pid 与档位的占位日志，改为携带完整上下文的 START/END 空实现日志 | 不新增真实 RSS 调用、动态策略或资源数量限制 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved（yangzk，2026-11-16） |
| 依赖需求设计 | `20260914692273/design.md` | 已合入实现；ADR-9 提供初始防抖契约 |

> 需求基线、范围边界、不涉及项和影响范围以 Approved proposal.md 为准。design.md 以本文档及 proposal.md 为输入，在后续 Design 阶段编写。

## 用户故事

### US-1: 完整、可核对的调度事件上下文

**作为** 系统调度开发者，
**我想要** 每次进程优先级重算对应的上下文日志都包含完整归因信息，实际代理事件在产生时也包含同样的完整信息，
**以便** 在多任务、多创建者、多 AttributionTool 场景下准确核对调度决策。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-5.1 | WHEN 任一纳管进程发生新建归属或 effectiveLevel 重算 THEN 每次重算的上下文日志完整列出纳管 pid/uid、共管任务集合、每个任务的 taskId 与创建者 pid/uid、每个任务的 AttributionTool 集合、变化前后 effectiveLevel 和触发原因；WHEN 按 AC-5.3～AC-5.7 产生实际代理事件 THEN 该事件还包含 START/END 动作及上述全部字段，且各项与本次任务事实一致 | InnerAPI（代理事件、上下文日志） | 正常 |
| AC-5.2 | WHEN 同一进程被多个任务、不同创建者或多个 AttributionTool 共管 THEN 上下文日志和实际代理事件逐一保留全部共管任务及各自工具集合，不发生单创建者或单工具覆盖；WHEN 最后任务移除或纳管进程死亡导致 END THEN END 事件仍完整列出变化前的创建者、纳管对象 uid 和 AttributionTool 集合 | InnerAPI（代理事件、上下文日志） | 边界 |
| AC-5.3 | WHEN 仅新增/移除共管任务或仅 AttributionTool 集合变化，且 effectiveLevel 不变且管控未进入开始/结束迁移 THEN 上下文日志反映最新集合且代理事件计数不增加；下一次实际 START/END 事件携带最新集合 | InnerAPI（代理事件计数、上下文日志） | 边界 |
| AC-5.8 | WHEN 调度代理收到符合本规格的 START/END 事件 THEN 仅输出包含完整事件内容的可核对日志，真实冻结、调度分组、QoS、speed、生效查询、恢复及撤销重试接口的调用次数均为 0 | InnerAPI（代理日志、真实资源调度接口 mock 调用计数） | 正常 |

### US-2: 与最终管控状态一致的事件动作和时序

**作为** 系统调度开发者，
**我想要** 代理事件按明确的动作、立即通知和固定窗口合并规则输出，
**以便** 在档位变化、任务结束和进程死亡时核对事件序列与最终管控状态。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-5.4 | WHEN 进程由未纳管变为纳管或 effectiveLevel 上升 THEN 在当前外部事件处理序列中直接输出包含最新状态的 START，不进入 1000ms 合并窗口；WHEN 纳管进程死亡 THEN 在当前死亡事件处理序列中直接输出 END，且该 pid 尚未输出的合并事件不再输出 | InnerAPI（代理事件序列） | 正常/异常 |
| AC-5.5 | WHEN effectiveLevel 下降，或主动注销、任务正常结束、任务超时、创建者死亡导致降档或最终结束 THEN 对应事件进入固定 1000ms 合并窗口且不读取系统参数；窗口到期输出按最新状态形成的事件，降档记 START，最终结束记 END | InnerAPI（代理事件及可注入时钟的到期序列） | 正常 |
| AC-5.6 | WHEN 合并窗口内又发生新建、升档、继续降档、任务结束或进程死亡 THEN 不输出基于旧档位或旧共管集合的过期事件；最终事件序列只包含各触发点的有效事件和窗口到期时的最新状态，重算结果无变化时不额外输出事件 | InnerAPI（代理事件序列） | 边界 |
| AC-5.7 | WHEN 任务结束、超时、创建者死亡或主动注销使最高档任务被移除但同一 pid 仍有其他任务共管 THEN 输出反映剩余任务最新 effectiveLevel 的降档 START；WHEN 最后一个任务/归因被移除 THEN 输出 END；WHEN 纳管进程死亡 THEN 该 pid 仅输出一个最终 END，其他 pid 的事件不受影响 | InnerAPI（代理事件及事件计数） | 正常/异常 |

### US-3: 既有行为兼容

**作为** 现有应用及任务调用方，
**我想要** 未纳管进程和既有前端纳管约束不受本需求影响，
**以便** 保持现有任务和进程行为兼容。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-5.9 | WHEN 进程从未被任务纳管或与本需求无关 THEN 代理事件计数为 0；WHEN 已纳管进程因最后任务/归因移除或死亡而转为未纳管 THEN 仅允许 AC-5.7 定义的最终 END，END 之后该 pid 不再产生后续 START/END；上述任一情况均不出现本需求新增的独立资源数量上限、超限拒绝或新错误码，且既有前端任务与归因上限的行为与改动前一致 | System API（既有任务/归因结果）/ InnerAPI（代理事件计数） | 边界 |

## 验收追溯

> 本表只维护 Spec 内部的 AC → 规则 → 可观察表面事实。AC → Task 映射由 execution-plan.md 维护；测试入口、Red 条件和通过标准由“验证映射”维护；实际执行证据由 evidence/reviews/ 维护。

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-5.1 | R-1 | InnerAPI（代理事件、上下文日志） |
| AC-5.2 | R-1 | InnerAPI（代理事件、上下文日志） |
| AC-5.3 | R-2 | InnerAPI（代理事件计数、上下文日志） |
| AC-5.4 | R-3 | InnerAPI（代理事件序列） |
| AC-5.5 | R-4 | InnerAPI（代理事件及可注入时钟的到期序列） |
| AC-5.6 | R-5 | InnerAPI（代理事件序列） |
| AC-5.7 | R-6 | InnerAPI（代理事件及事件计数） |
| AC-5.8 | R-7 | InnerAPI（代理日志、真实资源调度接口 mock 调用计数） |
| AC-5.9 | R-8 | System API（既有任务/归因结果）/ InnerAPI（代理事件计数） |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 新建归属、effectiveLevel 重算、多任务/多创建者/多工具共管、最后任务移除或纳管进程死亡 | 每次重算的上下文日志完整列出纳管 pid/uid、全部 taskId、各任务创建者 pid/uid 与 AttributionTool 集合、变化前后 effectiveLevel 和触发原因；实际代理事件还包含 START/END 动作；END 保留变化前完整上下文 | 共管集合逐项表达，不得用单创建者或单工具覆盖；触发原因至少区分新建归属、主动注销、任务正常结束、任务超时、创建者死亡、纳管进程死亡、前后台状态变化、系统负载变化及窗口到期；合并事件同时保留原始触发原因和窗口到期事实；变化后集合为空时仍保留变化前上下文 | AC-5.1、AC-5.2 |
| R-2 | 边界 | 仅共管任务或 AttributionTool 集合变化，effectiveLevel 不变且未进入管控开始/结束迁移 | 上下文日志使用最新集合，代理事件计数不增加；下一实际事件使用最新集合 | 新增和移除均适用；不得以更新上下文为由额外制造 START/END | AC-5.3 |
| R-3 | 行为/异常 | 进程由未纳管变为纳管、effectiveLevel 上升，或纳管进程死亡 | 新建/升档直接输出 START；死亡直接输出 END；均不进入 1000ms 合并窗口，死亡同时使旧合并事件失效 | effectiveLevel 取 LOW(100)/MEDIUM(200)/HIGH(300)；死亡事件不影响其他 pid | AC-5.4 |
| R-4 | 行为 | effectiveLevel 下降，或主动注销、任务正常结束、任务超时、创建者死亡导致降档或最终结束 | 对应事件经固定 1000ms 窗口合并，到期按最新状态输出；降档记 START，最终结束记 END | 窗口固定为 1000ms，不读取系统参数；测试使用可注入时钟验证到期行为 | AC-5.5 |
| R-5 | 边界 | 合并窗口内发生新建、升档、继续降档、任务结束或进程死亡 | 丢弃旧档位/旧共管集合对应的过期输出，只保留有效触发事件及窗口到期时的最新状态；无变化时不额外输出 | 不得把窗口内每次变化都作为独立事件排队输出 | AC-5.6 |
| R-6 | 行为/异常 | 最高档任务被移除但仍有共管任务、最后一个任务/归因被移除，或纳管进程死亡 | 仍有共管任务时输出降档 START；最后任务/归因移除时输出 END；死亡 pid 只输出一个最终 END，其他 pid 不受影响 | 动作由进程最终管控状态决定，任务结束原因由触发原因字段区分 | AC-5.7 |
| R-7 | 行为 | 调度代理收到 START/END 事件 | 仅输出完整事件日志，不调用真实冻结、调度分组、QoS、speed、生效查询、恢复或撤销重试接口 | 真实资源调度接口 mock 调用计数必须为 0；不据此宣称真实调控已生效 | AC-5.8 |
| R-8 | 边界 | 进程从未纳管或与本需求无关、已纳管进程转为未纳管，或既有任务/归因数量变化 | 从未纳管或无关进程的代理事件计数为 0；转为未纳管时仅允许最终 END，END 后该 pid 不再产生后续 START/END；不新增独立配额、超限拒绝或错误码；既有前端上限行为不变 | 最后任务/归因移除和死亡只允许 AC-5.7 定义的最终 END；不取消、不提高、不降低既有前端任务/归因限制 | AC-5.9 |

## 验证映射

> 每个 AC 至少关联一个可执行测试入口。Red 条件基于当前源码中缺失的字段、事件语义或合并机制；AC-5.9 是兼容性防回归项，不制造假 Red。实现后的实际结果和证据由 execution-plan / Verification Evidence 回填。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1 / AC-5.1 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_EventContext_0100`、`ProcessSchedule_TriggerReasonMatrix_0100` | 单测：事件记录测试替身 + 上下文日志与原因矩阵断言 | 当前管控占位日志只有 pid 与变化前后档位，缺少纳管 uid、共管任务、创建者、AttributionTool 集合、动作和原因，且无法区分前后台与负载原因或同时表达原始原因和窗口到期，字段与原因矩阵断言失败 | 每条实际事件及对应日志的完整字段与输入事实一致；新建、注销、正常结束、超时、创建者死亡、进程死亡、前后台、负载及窗口到期均可区分，合并事件同时保留原始原因与到期事实 |
| VM-2 | R-1 / AC-5.2 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_MultiCreatorMultiToolContext_0100`、`ProcessSchedule_EndEventPreviousContext_0100` | 单测：多任务/多创建者/多工具及最后结束矩阵 | 当前调度侧归属与事件上下文未保存完整创建者和工具集合，最后结束日志也不含变化前完整上下文，集合并集与 END 字段断言失败 | 所有共管任务和各自工具均被保留；最后任务移除和死亡 END 均保留变化前完整上下文 |
| VM-3 | R-2 / AC-5.3 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_ContextOnlyChangeMatrix_0100` | 单测：新增同档/低档共管、同 task/pid 新增工具、移除重复工具、移除非最高任务且档位不变，及后续 START/END 上下文断言 | 新增同档/低档共管任务当前会输出占位管控事件，且后续事件无法携带新增任务/工具全集；无事件计数与最新上下文断言失败 | 四类不变档集合变化均不增加代理事件；后续实际 START 和保留变化前上下文的 END 均使用对应最新全集 |
| VM-4 | R-3 / AC-5.4 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_ImmediateStartAndDeathEnd_0100` | 单测：可注入时钟 + 事件顺序与原因断言 | 当前没有 START/END 动作语义和死亡使旧合并事件失效的机制，动作、原因及事件序列断言失败 | 新建/升档不经过合并窗口即输出 START 并区分前后台与负载原因；死亡立即输出 END，旧合并事件输出计数为 0，其他 pid 不受影响 |
| VM-5 | R-4 / AC-5.5 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_DebounceDownAndEnd_0100` | 单测：可注入时钟 + 固定窗口与原始原因断言 | 当前降档和结束直接输出占位日志，没有固定 1000ms 合并窗口；窗口前已有输出且到期无法得到正确动作和原始原因，事件序列断言失败 | 窗口前无对应事件；1000ms 到期按最新状态输出一次降档 START 或最终 END，保留正常结束/超时/创建者死亡等原始原因及到期事实，且策略/窗口系统参数读取次数为 0 |
| VM-6 | R-5 / AC-5.6 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_DebounceLatestState_0100` | 单测：反复升降档、新建、结束、死亡与到期序列矩阵 | 当前没有合并窗口内最新状态重算和过期事件取消机制，旧状态输出及重复事件计数断言失败 | 无旧档位/旧集合事件；最终序列与最新状态一致；无变化时新增事件计数为 0 |
| VM-7 | R-6 / AC-5.7 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_FinalStateAction_0100` | 单测：任务结束/超时/创建者死亡/主动注销/死亡动作矩阵 | 当前缺少按最终管控状态区分降档 START 与最终 END 的事件契约，动作、原因和最终事件计数断言失败 | 有共管任务时为最新档位 START；最后移除时 END；死亡 pid 只有一个最终 END，其他 pid 序列不变 |
| VM-8 | R-7 / AC-5.8 | `test/unittest/process_schedule_test/resource_control_proxy_test.cpp::ResourceControlProxy_StubOnly_0100` | 单测：代理日志 + 真实资源调度接口 mock | 当前尚无满足完整事件契约的资源管控代理，测试入口编译/链接失败，且无法断言真实接口零调用 | START/END 日志内容完整；冻结、分组、QoS、speed、生效查询、恢复和撤销重试接口调用次数均为 0 |
| VM-9 | R-8 / AC-5.9 | `test/unittest/process_schedule_test/process_schedule_test.cpp::ProcessSchedule_UnmanagedCompatibility_0100`；`test/unittest/task_manager_service_test/task_manager_service_test.cpp::TaskManagerService_StartTask_0700`、`TaskManagerService_AttributeProcessForAbility_0800`、`TaskManagerService_RegisterManagedProcessQuota_0900` | 单测 + 既有回归 | N/A：兼容性防回归项。从未纳管或无关进程无本需求事件、既有配额行为已有用例覆盖；最终 END 的时序和载荷由 VM-5/VM-7 验证，本项不制造假 Red | 从未纳管或无关进程事件计数为 0；转为未纳管只允许一个最终 END，之后无后续 START/END；无本需求新增配额/超限拒绝/错误码；三个既有上限用例结果与改动前一致 |

**统一构建入口（实现阶段执行）：** 在 `/srv/workspace/openharmony_master_default_dayu600_7885_ai_20261114203447_huawei_9e27330f7/code` 下执行 `./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test --no-prebuilt-sdk --gn-args use_thin_lto=false`；聚焦验证使用 `process_schedule_test` 对应测试对象。

## API 变更分析

### 新增 API

N/A：无 Public/System API 新增。具体 InnerAPI 名称、参数和签名仅在 design.md 中设计，不在本文展开。

### 变更/废弃 API

N/A：无 Public/System API 变更或废弃；既有任务/归因接口行为保持不变。

### API 与错误码事实

N/A：本需求不新增、修改或废弃 Public/System API，不新增公开错误码。具体 InnerAPI 的签名、事件类型和取值在 design.md 中定义，不在本文列出。

## 接口规格

N/A：无对外接口规格。代理事件的可观察字段、动作和时序已在用户故事、验收追溯和规则定义中给出；具体 InnerAPI 接口定义由后续 design.md 承载。

## 兼容性声明

- **已有 API 行为变更:** 否。无 Public/System API 新增、修改或废弃，既有任务/归因结果及错误码不变。
- **配置文件格式变更:** 否。不新增动态策略或防抖窗口配置，不读取相关系统参数。
- **数据存储格式变更:** 否。事件上下文和合并状态不持久化，无存量数据迁移。
- **最低支持版本:** OpenHarmony-7.1-Release（本需求目标版本；无对外 API 版本变更）。
- **API 版本号策略:** N/A：无 Public/System API 或 @since 标注变化。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| Approved 需求边界 | 只验证调度决策、事件载荷、动作和时序；不验收真实 RSS 调控效果、性能功耗或资源配额 | AC-5.1～AC-5.9 |
| 既有事实复用 | 任务创建者、共管关系、effectiveLevel、任务结束/超时/死亡事实沿用已合入依赖需求，不重新定义其语义 | AC-5.1、AC-5.2、AC-5.7、AC-5.9 |
| 固定功能时序 | 新建/升档和死亡不经过合并窗口；降档及非死亡结束使用固定 1000ms 窗口，不读取动态配置 | AC-5.4～AC-5.6 |
| 代理隔离 | 本期代理仅输出日志，真实资源调度接口调用次数为 0；不得以日志通过宣称真实调控生效 | AC-5.8 |
| 内部接口边界 | 具体 InnerAPI 名称、签名、数据结构和并发实现只在 design.md 定义 | AC-5.1～AC-5.9 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 死亡事件不经过合并窗口；合并事件不得输出旧档位或旧共管集合；最后结束不得丢失变化前上下文 | VM-2、VM-4、VM-6、VM-7 | 实现阶段生成 |
| 兼容性 | 从未纳管或无关进程代理事件计数为 0；最终 END 后无后续 START/END；既有任务/归因上限、Public/System API 和错误码行为不变 | VM-9 + 既有回归用例 | 实现阶段生成 |
| 可测试性 | AC-5.1～AC-5.9 均有自动化验证入口，固定窗口可通过注入时钟确定性验证 | `ability_runtime_test` 构建及聚焦用例 | 实现阶段生成 |
| 定界定位 | 日志可区分触发原因、变化前后 effectiveLevel、动作和完整归因上下文，能够定位过期事件来源 | hilog 检索与 VM-1～VM-7 | 实现阶段生成 |

## 多设备适配声明

无差异。调度决策、完整事件上下文、START/END 动作和固定 1000ms 合并规则均为系统机制，不按手机、平板、折叠屏、PC、手表、电视或车机读取不同策略；真实 RSS 效果不在本需求验收范围。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | N/A | 无 UI 或无障碍交互 | — |
| 大字体 | N/A | 无 UI 文案 | — |
| 深色模式 | N/A | 无 UI | — |
| 多窗口/分屏 | N/A | 不改变窗口机制 | — |
| 多用户 | 否 | 创建者及纳管对象的 userId 语义沿用既有任务记录，本需求不改变 | 共管上下文采集与结束事件 |
| 版本升级 | 否 | 事件上下文和合并状态不持久化，无存量数据迁移 | — |
| 生态兼容 | 否 | 无 Public/System API、d.ts、Kit 或公开错误码变化 | — |

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "services/appmgr/src/task_mgr/process_schedule.cpp 当前归属重算、占位日志与不变档不传播行为"
  - repo: "openharmony/ability_ability_runtime"
    query: "TaskRecord/AttributionRecord 创建者和工具字段，以及任务结束、超时、创建者死亡、纳管进程死亡事件事实"
  - repo: "openharmony/ability_ability_runtime"
    query: "test/unittest/process_schedule_test 现有多任务、降档、死亡和不变档回归用例"
  - repo: "abilitykit_public/abilitykit_sdd"
    query: "specifications/ability task_flow 生命周期、归因、容量和死亡清理既有规格"
```

**关键文档：** Approved `proposal.md`；`20260914692273/design.md` ADR-9；`services/appmgr/src/task_mgr/process_schedule.cpp:41-65,76-101,134-188`；`services/appmgr/src/task_mgr/task_manager_service.cpp:197-230,233-312,405-445,625-685`；`test/unittest/process_schedule_test/process_schedule_test.cpp`；本仓 `AGENTS.md`、`CLAUDE.md`。
