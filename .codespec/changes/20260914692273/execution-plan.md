# 执行计划

> 将 Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 用稳定 ID 和章节锚点引用上游事实，不复制 Spec/Design 正文。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260914692273 |
| 关联 Feature/Bug | FEAT-AI-TASK-MGR-001（RR 30096095） |
| 关联文档 | proposal.md / spec.md / design.md |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（2026-11-04） |
| Design | `design.md` | ReadyForReview（候选，待评审批准） |
| Spec | `spec.md` | ReadyForReview（候选，待评审批准） |

## 执行原则

- **Spec 权威：** 实现与 AC、错误码、兼容性声明冲突时，停止并修订 spec/design/plan，不能在代码里自行改语义。
- **测试/证据先行：** 每个 Task 先确认失败测试或可复现证据缺口，再做最小实现并取得 fresh PASS 证据。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环；跨接口、事件链、状态生命周期或构建边界的工作应拆分。
- **文件边界：** 只能修改 Task 的 Files/允许修改范围；需要额外文件时先修订 Plan。
- **状态所有权唯一：** 新增或修改状态必须明确 owner、key/index、创建、读取、更新、清理触发和不变量。
- **证据回填：** Task 完成当次会话回填追溯状态、实际文件、Commit、Review Evidence 和 Actual Result，不跨会话补证。
- **反伪完成：** 只补声明、只写存储、只覆盖 happy path、只跑无关测试或只填写 PASS 文本，都不能替代 AC 闭环。
- **生成文件：** 本计划无生成器产物（IDL 已交付于 20260914692191，本需求仅改其字段）。

## 受影响文件全量清单

> 对照 design.md「调用链层级分析」逐层列出。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| interface_sdk-js | SDK 声明 | api/@ohos.app.agent.taskManager.d.ts | 修改 | startTask 移除 priority 入参；移除 TaskPriority 枚举导出（跨仓提审，TASK-1） |
| ability_ability_runtime | ETS 框架 | frameworks/ets/ets/@ohos.app.agent.taskManager.ets | 修改 | startTask 框架实现移除 priority 与默认 MEDIUM 逻辑 |
| ability_ability_runtime | ANI 绑定 | frameworks/ets/ani/task_manager/include/、src/ | 修改 | NativeStartTask 移除 aniPriority 参数解析 |
| ability_ability_runtime | NAPI 绑定 | frameworks/js/napi/task_manager/js_task_manager.{h,cpp} | 修改 | OnStartTask 移除 priority 解析与校验 |
| ability_ability_runtime | IPC 客户端/服务端 | interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h、app_mgr_proxy.h、app_mgr_stub.h、app_mgr_client.h；src/appmgr/app_mgr_proxy.cpp、app_mgr_stub.cpp、app_mgr_client.cpp | 修改 | IAppMgr::StartTask 移除 priority 参数；START_TASK(147) 消息体移除 priority 序列化 |
| ability_ability_runtime | 系统服务 | services/appmgr/include/appmgr/app_mgr_service.h、src/app_mgr_service.cpp | 修改 | AppMgrService::StartTask 签名同步 |
| ability_ability_runtime | 系统服务 | services/appmgr/include/task_mgr/task_manager_service.h、src/task_mgr/task_manager_service.cpp | 修改 | StartTask/StartTaskInner 移除 priority 形参，接入判定器；CheckParam 移除 priority 校验 |
| ability_ability_runtime | 系统服务（新增） | services/appmgr/include/task_mgr/task_priority_judge.h、system_load_provider.h；src/task_mgr/task_priority_judge.cpp、system_load_provider.cpp | 新增 | 判定器与负载适配层（TASK-2） |
| ability_ability_runtime | 系统服务（新增） | services/appmgr/include/process_schedule/、src/process_schedule/ | 新增 | 双向索引、max 合并、幂等、并发上限、死亡清理（TASK-4/6） |
| ability_ability_runtime | 系统服务挂点 | services/appmgr/include/app_death_recipient.h、src/app_death_recipient.cpp（或既有挂点注册处） | 修改 | 死亡→process_schedule 清理回调（最小挂点，TASK-6） |
| ability_ability_runtime | 系统服务挂点 | services/appmgr/src/task_mgr/、应用状态感知订阅处 | 修改 | 前后台监听挂点（TASK-5） |
| ability_ability_runtime | 构建 | services/appmgr/BUILD.gn、test/unittest/task_mgr/BUILD.gn、test/unittest/process_schedule/BUILD.gn（新建） | 修改/新增 | 源文件与单测 target |
| ability_ability_runtime | 测试 | test/unittest/task_mgr/、test/unittest/process_schedule/（新建）、test/integration/、test/mock/services_appmgr_test/include/mock_app_mgr_service.h | 新增/修改 | 矩阵/兜底/重判/合并/幂等/上限单测；集成用例；StartTask mock 签名同步 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失，需回溯补充 design.md

## AC 到 Task 追溯

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-6.1 | spec.md US-2 | TASK-4 | process_schedule 双向索引/max 合并 | | | | Pending |
| AC-6.2 | spec.md US-2 | TASK-4 | process_schedule 降级重算 | | | | Pending |
| AC-6.3 | spec.md US-2 | TASK-6 | AppDeathRecipient 挂点 + process_schedule 清理 | | | | Pending |
| AC-6.4 | spec.md US-2 | TASK-4 | process_schedule 并发上限 + 并发矩阵单测 | | | | Pending |
| AC-6.5 | spec.md US-1 | TASK-1、TASK-2、TASK-3 | d.ts + 判定器 + startTask 全链路 | | | | Pending |
| AC-6.6 | spec.md US-1 | TASK-2 | 判定矩阵单测 | | | | Pending |
| AC-6.7 | spec.md US-1 | TASK-5 | 动态重判订阅与传播 | | | | Pending |

## 首批实现边界

**首批必须实现：** 判定器与 startTask 变更（AC-6.5 主链）、max 合并语义（需求"按最高优先级处理"核心）、归属矩阵/判定矩阵单测。
**可后置：** 无。
**不建议延后：** TASK-1（d.ts）与 TASK-3 同评审闭环，避免接口面漂移。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 接口与判定骨架 | TASK-1、TASK-2、TASK-3 | d.ts diff 一致；新签名调用成功、多传 401；判定矩阵单测绿 | 集成 demo + `run -t UT -tp ability_runtime` |
| Phase-2 | 归属合并与重算 | TASK-4 | 归属/降级/幂等/上限矩阵用例绿 | `run -t UT -tp ability_runtime` |
| Phase-3 | 动态重判与清理 | TASK-5、TASK-6 | 重判/防抖用例绿；死亡清理 ≤1s 且任务存续 | 单测 + 集成用例 |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环；文件范围、验证闭环和风险边界足够分离 → 拆分。
- TASK-3 跨 IPC/绑定/服务三层，但共享同一最小验证闭环（新签名 startTask 全链路调用），合并为一张 Task Card。

## 禁止项

执行计划和 Task 不得出现以下内容：

- [ ] 没有未决占位符与未填项
- [ ] 没有"根据需要实现""酌情处理"等模糊指令
- [ ] 没有跨 Task 隐式依赖（依赖必须显式声明在前置依赖列）
- [ ] 没有要求 Agent 自行寻找未列出的上下文文件
- [ ] 没有无验证方式的 AC
- [ ] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每个 Task 自包含）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | API 声明文件变更（d.ts，跨仓） | interface_sdk-js api/@ohos.app.agent.taskManager.d.ts | AC-6.5 | 无 | d.ts 移除 priority 入参与 TaskPriority 导出，@since 26.2.0 不变 | oh-api-definition（可选）+ d.ts diff 审阅 | Pending |
| TASK-2 | 优先级判定器（映射矩阵+感知+兜底） | services/appmgr/src/task_mgr/（task_priority_judge.*、system_load_provider.*）、test/unittest/task_mgr/ | AC-6.5、AC-6.6 | 任务记录模块（20260914692191，已合入 master） | 判定矩阵/兜底单测全绿 | `run -t UT -tp ability_runtime` | Pending |
| TASK-3 | startTask 接口变更与判定接入（IAppMgr IPC/NAPI/ANI/ETS/Service） | interfaces/inner_api/app_manager、frameworks/ets/ets、frameworks/ets/ani/task_manager/、frameworks/js/napi/task_manager/、services/appmgr/（app_mgr_service、task_mgr）、test/mock/、test/integration/ | AC-6.5 | TASK-1、TASK-2 | 新签名集成 demo 通过；多传入参返回 401 | 集成 demo + `run -t UT -tp ability_runtime` | Pending |
| TASK-4 | 双向索引 + max 合并 + 幂等 + 并发上限 | services/appmgr/src/process_schedule/、test/unittest/process_schedule/ | AC-6.1、AC-6.2、AC-6.4 | TASK-3；20260914692272 管控代理层 | AC 用例全绿 | `run -t UT -tp ability_runtime` | Pending |
| TASK-5 | 动态重判与档位传播 | services/appmgr/src/task_mgr/、services/appmgr/src/process_schedule/、test/unittest/task_mgr/ | AC-6.7 | TASK-3、TASK-4 | 动态重判/防抖用例全绿 | `run -t UT -tp ability_runtime` | Pending |
| TASK-6 | 进程死亡清理挂点 | services/appmgr/（AppDeathRecipient 挂点）、test/integration/ | AC-6.3 | TASK-4 | 清理 ≤1s、任务存续用例全绿 | 集成用例 + `run -t UT -tp ability_runtime` | Pending |

## Task 详情

### TASK-1: API 声明文件变更（d.ts，跨仓）

| 字段 | 内容 |
|------|------|
| 任务目标 | 按 spec.md「API 变更分析」修改声明文件：startTask 移除 priority 可选入参；移除 TaskPriority 枚举导出 |
| AC 映射 | AC-6.5 |
| 前置依赖 | 无 |
| 非目标 | appmgr 侧实现变更（TASK-3）；finishTask 声明（20260914692191 承载，不变） |
| 完成判据 | d.ts 仅 startTask 签名与 TaskPriority 枚举两处变更，@since 26.2.0/@systemapi/@stagemodelonly/@syscap 标注不变 |
| 停止条件 | 发现声明与 spec 接口规格不一致、或需变更 finishTask/其他接口 |
| Design 状态/不变量 | N/A（声明文件无运行状态） |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | d.ts 新签名 `startTask(taskId: string): Promise<void>`；TaskPriority 枚举不再导出 | TASK-1 → TASK-3 | 供 TASK-3 对齐实现 | @since 26.2.0 不变；System 级别不变 |
| Consumes | 无 | — | — | — |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `spec.md#API 变更分析` | 签名与错误码权威来源 |
| `frameworks/ets/ets/@ohos.app.agent.taskManager.ets` | 本仓框架侧对应声明（对齐 d.ts 改动） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | interface_sdk-js api/@ohos.app.agent.taskManager.d.ts | startTask 移除 priority 入参；删除 TaskPriority 枚举段 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| finishTask 声明段 | 20260914692191 承载，本需求不变 |

**Spec References**

- AC / Rules: `AC-6.5`, `R-1`
- Anchors: `spec.md#api-变更分析`, `spec.md#接口规格`

**Design References**

- IDs / Anchors: `ADR-8`, `design.md#api-签名kit-与权限`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | 必须保持 @systemapi/@since 26.2.0；枚举移除不改变 startTask 开放级别 |

**Steps**

- [ ] 生成 d.ts diff：startTask 移除 priority 入参、删除 TaskPriority 枚举段
- [ ] 如环境可用运行 oh-api-definition（可选、不阻塞）
- [ ] diff 审阅：与 spec.md「API 变更分析」签名一致
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| 20260914692191 已交付 d.ts（api/@ohos.app.agent.taskManager.d.ts） | 标注风格/@since/错误码声明 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | d.ts diff 仅两处变更且与 spec 一致（AC-6.5） |
| Scope respected | 未触碰 finishTask 与其他声明段 |
| State lifecycle complete | N/A + 理由：声明文件无运行状态 |
| Interface contract matched | 新签名与 TASK-3 Produces 引用一致 |
| Relevant verification | diff 审阅记录 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 静态检查 | d.ts diff 文件 | 仅 startTask 签名与 TaskPriority 枚举两处变更 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：d.ts 声明变更（跨仓 interface_sdk-js） |
| 允许修改 | interface_sdk-js api/@ohos.app.agent.taskManager.d.ts |
| 允许新建 | 无 |
| 只读参考 | spec.md「API 变更分析」；frameworks/ets/ets/@ohos.app.agent.taskManager.ets |
| Spec 引用 | `AC-6.5` / `R-1` / `spec.md#API 变更分析` |
| Design 引用 | `ADR-8` / `design.md#API 签名、Kit 与权限` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | d.ts diff 审阅 / 期望: 仅两处变更 |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.5→Task-1→d.ts diff；签名与 spec 精确一致 |
| Code Quality | diff 范围、标注完整性 |
| Verification | diff 审阅记录 |

### TASK-2: 优先级判定器（映射矩阵+感知+兜底）

| 字段 | 内容 |
|------|------|
| 任务目标 | task_mgr 新增 TaskPriorityJudge：DetermineLevel 按默认映射矩阵输出档位；SystemLoadProvider 适配层（默认实现+测试桩）；前后台状态经既有应用状态感知监听获取；感知失败兜底 MEDIUM |
| AC 映射 | AC-6.5、AC-6.6 |
| 前置依赖 | 任务记录模块（20260914692191，已合入 master） |
| 非目标 | startTask 链路接入（TASK-3）；动态重判事件订阅与传播（TASK-5） |
| 完成判据 | 判定矩阵四组合与兜底单测全绿 |
| 停止条件 | 映射矩阵与 design.md「关键设计决策 ADR-5」冲突；感知链路需新增侵入式钩子 |
| Design 状态/不变量 | STATE-1 / INV-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1 / INV-1 | 任务档位 | task_mgr / TaskPriorityJudge | TaskRecord.priority（tasksMutex_ 保护） | startTask 创建时判定 | process_schedule 经事件+查询只读 | 任务结束 | 档位 ∈ {100,200,300} 且等于映射矩阵求值（感知失败=MEDIUM=200） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `DetermineLevel(uid, fgState, loadLevel) -> TaskLevel`；`GetLevel(taskId)`；`OnTaskLevelChanged(taskId, level)` 事件 | TASK-2 → TASK-3/5 | 供创建链路判定与动态重判 | TaskLevel 取值 100/200/300（common.h 常量沿用） |
| Consumes | 前后台状态监听（既有应用状态感知）；SystemLoadProvider 负载等级 | 既有链路 / TASK-2 内部 | 输入为系统内部状态 | 注册监听失败按不可用处理（兜底） |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#关键设计决策`（ADR-5/6/7） | 矩阵与兜底定义 |
| `services/appmgr/src/task_mgr/` | 任务记录与锁域先例（task_manager_service.cpp:153） |
| `services/appmgr/include/task_mgr/common.h:26-29` | 档位常量 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | services/appmgr/include/task_mgr/task_priority_judge.h | 判定器接口与映射矩阵 |
| Create | services/appmgr/src/task_mgr/task_priority_judge.cpp | 判定实现+兜底 |
| Create | services/appmgr/include/task_mgr/system_load_provider.h | 负载适配层抽象 |
| Create | services/appmgr/src/task_mgr/system_load_provider.cpp | 默认实现+测试桩 |
| Modify | services/appmgr/BUILD.gn、test/unittest/task_mgr/BUILD.gn | 源文件与单测 target |
| Test | test/unittest/task_mgr/ | PriorityJudgeMatrix_0100、PriorityJudgeFallback_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| services/appmgr/src/process_schedule/ | TASK-4 范围 |
| services/appmgr/src/task_mgr/task_manager_service.cpp | TASK-3 范围（本 Task 不接入链路） |

**Spec References**

- AC / Rules: `AC-6.5`, `AC-6.6`, `R-1`, `R-7`
- Anchors: `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-5`, `ADR-7`, `STATE-1`, `INV-1`, `design.md#状态归属与不变量`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 判定器必须驻留 task_mgr 服务层，不得下沉框架层或上侵 process_schedule |
| OH-ARCH-ERROR-LOG | 感知失败必须兜底 MEDIUM 并打关键路径日志（TAG 遵循 20260914692442） |

**Steps**

- [ ] 写失败测试：四组合矩阵断言 + 感知不可用兜底 MEDIUM（Red：未实现前判定结果无状态差异）
- [ ] 运行 `run -t UT -tp ability_runtime` 聚焦用例确认 RED
- [ ] 实现 TaskPriorityJudge 映射矩阵与 SystemLoadProvider 适配层（含测试桩）
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/src/task_mgr/task_manager_service.cpp:153`（锁域） | 互斥锁保护记录风格 | 无 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp:514-522`（系统参数读取） | parameter 读取封装 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 四组合矩阵与兜底用例通过（AC-6.6） |
| Scope respected | 仅 task_mgr/ 与对应单测目录；未改 task_manager_service.cpp |
| State lifecycle complete | 档位创建（判定）/查询/清理路径覆盖（INV-1） |
| Interface contract matched | DetermineLevel/GetLevel 签名与 design 类图一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（task_mgr 单测） | 判定矩阵/兜底用例全绿 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：优先级判定器与负载适配层 |
| 允许修改 | services/appmgr/src/task_mgr/、include/task_mgr/、BUILD.gn、test/unittest/task_mgr/ |
| 允许新建 | task_priority_judge.*、system_load_provider.* |
| 只读参考 | design.md ADR-5/6/7；common.h |
| Spec 引用 | `AC-6.5`/`AC-6.6` / `R-1`/`R-7` |
| Design 引用 | `STATE-1`/`INV-1` / ADR-5/7 |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.5/6.6→Task-2→单测断言与矩阵一致 |
| Code Quality | 锁域、兜底路径、日志 |
| Verification | 单测全绿记录 |

### TASK-3: startTask 接口变更与判定接入（IAppMgr IPC/NAPI/ANI/ETS/Service）

| 字段 | 内容 |
|------|------|
| 任务目标 | IAppMgr::StartTask 及 Proxy/Stub/Client 移除 priority；NAPI/ANI/ETS 绑定移除 priority 参数校验（多传入参按 401）；TaskManagerService::StartTask 创建链路内同步调用 DetermineLevel 判定档位写入任务记录 |
| AC 映射 | AC-6.5 |
| 前置依赖 | TASK-1（d.ts 签名）、TASK-2（判定器接口） |
| 非目标 | 归属合并（TASK-4）；动态重判（TASK-5） |
| 完成判据 | 新签名集成 demo 通过（Promise 完成、任务按判定档位激活）；多传 priority 返回 401 |
| 停止条件 | 发现 IAppMgr 其他接口被迫联动修改；判定接入需改变鉴权/超时口径 |
| Design 状态/不变量 | 消费 STATE-1；无新增状态 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1（Consumes） | 任务档位 | task_mgr / TaskPriorityJudge | TaskRecord.priority | TASK-2 创建时判定（本 Task 触发） | TASK-3 写入任务记录 | 任务结束 | 同 design INV-1 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | 新 IDL 消息：START_TASK(147) 仅含 taskId（String16）；NAPI/ANI `startTask(taskId)`；Service `StartTask(const std::string &taskId)` | TASK-3 → 下游 | 供全链路编译与集成 | 事务码 147 不变；错误码 202/401/16000301 沿用 |
| Consumes | TASK-1 d.ts 新签名；TASK-2 DetermineLevel/GetLevel | TASK-1/TASK-2 | 前置 Task | 档位写入 TaskRecord.priority |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `interfaces/inner_api/app_manager/include/appmgr/app_mgr_ipc_interface_code.h:172` | START_TASK 事务码 |
| `interfaces/inner_api/app_manager/src/appmgr/app_mgr_stub.cpp:2735-2740` | 消息体现状 |
| `frameworks/js/napi/task_manager/js_task_manager.cpp:110-156` | NAPI 参数校验现状 |
| `frameworks/ets/ani/task_manager/src/ets_task_manager.cpp:86-115` | ANI 参数解析现状 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h | IAppMgr::StartTask 移除 priority |
| Modify | interfaces/inner_api/app_manager/include/appmgr/app_mgr_proxy.h、src/appmgr/app_mgr_proxy.cpp | Proxy 移除 priority |
| Modify | interfaces/inner_api/app_manager/include/appmgr/app_mgr_stub.h、src/appmgr/app_mgr_stub.cpp | Stub/HandleStartTask 移除 priority 序列化 |
| Modify | interfaces/inner_api/app_manager/include/appmgr/app_mgr_client.h、src/appmgr/app_mgr_client.cpp | Client 移除 priority |
| Modify | services/appmgr/include/appmgr/app_mgr_service.h、src/app_mgr_service.cpp | AppMgrService::StartTask 签名同步 |
| Modify | services/appmgr/include/task_mgr/task_manager_service.h、src/task_mgr/task_manager_service.cpp | StartTask/StartTaskInner 移除 priority 形参、CheckParam 移除 priority 校验、接入 DetermineLevel |
| Modify | frameworks/ets/ets/@ohos.app.agent.taskManager.ets | startTask 框架实现移除 priority |
| Modify | frameworks/ets/ani/task_manager/include/、src/ | NativeStartTask 移除 aniPriority |
| Modify | frameworks/js/napi/task_manager/js_task_manager.{h,cpp} | OnStartTask 移除 priority 解析；多传入参 401 |
| Modify | test/mock/services_appmgr_test/include/mock_app_mgr_service.h | MOCK_METHOD2→MOCK_METHOD1 同步 |
| Test | test/integration/ | StartTaskPriorityAutoJudge_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| IAppMgr 其他接口与序列化 | 高风险目录约束：不扩不修改 |
| services/uripermmgr/、services/abilitymgr/ | 超出本需求范围 |

**Spec References**

- AC / Rules: `AC-6.5`, `R-1`
- Anchors: `spec.md#接口规格`, `spec.md#api-与错误码事实`

**Design References**

- IDs / Anchors: `ADR-8`, `design.md#调用链层级分析`, `design.md#代码事实基线`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | 仅 START_TASK 消息体变更；事务码与错误码不变 |
| OH-ARCH-ERROR-LOG | 多传入参必须按 401（INVALID_PARAMETERS_ERR）拒绝；错误码值不改 |

**Steps**

- [ ] 写失败测试：无 priority 入参调用断言档位激活；多传入参断言 401（Red：未实现前接口含入参/档位缺省固定）
- [ ] 运行验证确认 RED
- [ ] IAppMgr Proxy/Stub/Client/Service 移除 priority 并同步序列化
- [ ] NAPI/ANI/ETS 绑定移除 priority 解析与默认 MEDIUM 逻辑
- [ ] TaskManagerService::StartTask 接入 DetermineLevel，档位写入 TaskRecord
- [ ] 同步 test mock 签名
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/src/task_mgr/task_manager_service.cpp:125-184` | 创建链路与错误码口径 | 移除 priority 校验，接入判定 |
| `frameworks/js/napi/task_manager/js_task_manager.cpp:120-131` | 参数校验错误风格 | 多传入参 401 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 新签名调用 demo + 多传 401 用例（AC-6.5） |
| Scope respected | 未改 IAppMgr 其他接口与 uripermmgr/abilitymgr |
| State lifecycle complete | STATE-1 创建（判定）写入路径覆盖 |
| Interface contract matched | 全链路签名与 TASK-1 d.ts、TASK-2 DetermineLevel 一致 |
| Relevant verification | 集成 demo + 单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | 集成 demo（StartTaskPriorityAutoJudge_0100） | 新签名调用成功、任务按判定档位激活 | （回填） |
| 测试 | `run -t UT -tp ability_runtime` | app_mgr_stub/proxy/client、task_manager 相关用例全绿 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-3：startTask 接口变更与判定接入（全链路） |
| 允许修改 | interfaces/inner_api/app_manager、services/appmgr（app_mgr_service、task_mgr）、frameworks/ets/ets、frameworks/ets/ani/task_manager、frameworks/js/napi/task_manager、test/mock、test/integration |
| 允许新建 | test/integration/StartTaskPriorityAutoJudge_0100 |
| 只读参考 | app_mgr_ipc_interface_code.h；js_task_manager.cpp |
| Spec 引用 | `AC-6.5` / `R-1` / `spec.md#接口规格` |
| Design 引用 | `ADR-8` / `design.md#调用链层级分析` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | 集成 demo + `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.5→Task-3→全链路代码；错误码与接口规格一致 |
| Code Quality | 序列化同步、绑定层参数处理、mock 同步 |
| Verification | demo 记录 + 单测全绿 |

### TASK-4: 双向索引 + max 合并 + 幂等 + 并发上限

| 字段 | 内容 |
|------|------|
| 任务目标 | process_schedule 维护双向索引与档位缓存：事件驱动增量重算（max）；豁免合并幂等（taskId+pid+豁免类型，pid 引用计数）；≤10 并发上限沿用 TASK_MAX_ACTIVE_TASKS + 系统参数可配 |
| AC 映射 | AC-6.1、AC-6.2、AC-6.4 |
| 前置依赖 | TASK-3；20260914692272 管控代理层 |
| 非目标 | 死亡清理挂点（TASK-6）；动态重判订阅（TASK-5） |
| 完成判据 | 归属矩阵/降级/幂等/上限用例全绿 |
| 停止条件 | 需修改 20260914692272 管控代理层语义；并发上限口径与 common.h 冲突 |
| Design 状态/不变量 | STATE-2 / INV-2、INV-3 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-2 / INV-2 | 双向索引/档位缓存 | process_schedule | taskToPids_/pidToTasks_/effectiveLevel_ | 纳管事件 | RSS 代理层只读查询 | 进程死亡/任务结束移除重算 | effectiveLevel(pid)==max(存活任务档位) |
| STATE-2 / INV-3 | 豁免引用计数 | process_schedule | 幂等键 taskId+pid+豁免类型 | 豁免申请 | RSS 代理层 | 任务结束/降档/进程死亡 | 不重复施加、按剩余任务重算无泄漏 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `OnTaskLevelChanged(taskId, level)` 重算入口（process_schedule inner API）；豁免幂等键 | TASK-4 → TASK-5/6 | 供动态重判与死亡清理调用 | 锁域内执行；不回调 task_mgr |
| Consumes | TASK-3 任务创建/结束事件与档位事实 | TASK-3 | 前置 Task | TaskLevel 100/200/300 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#状态归属与不变量` | 索引与不变量定义 |
| `services/appmgr/include/app_running_manager.h:517-518` | 索引与锁先例 |
| `services/appmgr/include/task_mgr/common.h:41-44` | 并发上限与纳管上限常量 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | services/appmgr/include/process_schedule/、src/process_schedule/ | 索引/合并/重算/幂等/上限 |
| Modify | services/appmgr/BUILD.gn | 源文件接入 |
| Test | test/unittest/process_schedule/（BUILD.gn 新建） | MultiTaskHighestPriority_0100、DegradeToNextPriority_0100、Concurrent10Tasks_0100、豁免幂等用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| services/appmgr/src/task_mgr/ | TASK-2/3 范围 |
| 20260914692272 管控代理层 | 档位施加/撤销机制归属其需求 |

**Spec References**

- AC / Rules: `AC-6.1`, `AC-6.2`, `AC-6.4`, `R-2`, `R-3`, `R-4`, `R-6`
- Anchors: `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-3`, `STATE-2`, `INV-2`, `INV-3`, `design.md#资源所有权矩阵`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 重算必须在 process_schedule 锁域内；锁内不得回调 task_mgr/RSS |
| OH-ARCH-ERROR-LOG | 并发上限读取沿用 GetIntParameter+默认兜底风格 |

**Steps**

- [ ] 写失败测试（矩阵/降档/幂等/上限）
- [ ] 运行验证确认 RED
- [ ] 实现双向索引与增量重算
- [ ] 实现幂等键与引用计数
- [ ] 实现并发上限（系统参数可配）
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/include/app_running_manager.h:517-518` | 索引+锁风格 | 双向索引为其扩展形态 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp:514-522` | 系统参数读取 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-6.1/6.2/6.4 用例全绿 |
| Scope respected | 仅 process_schedule/ 与对应单测目录 |
| State lifecycle complete | 索引/豁免创建、更新、清理覆盖（INV-2/3） |
| Interface contract matched | 重算入口签名与 design 类图一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（process_schedule 单测） | 归属/降级/幂等/上限用例全绿 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-4：双向索引、max 合并、幂等、并发上限 |
| 允许修改 | services/appmgr/src/process_schedule/、include/process_schedule/、BUILD.gn、test/unittest/process_schedule/ |
| 允许新建 | process_schedule 全部源文件与单测 |
| 只读参考 | app_running_manager.h；common.h |
| Spec 引用 | `AC-6.1`/`AC-6.2`/`AC-6.4` / `R-2`/`R-3`/`R-4`/`R-6` |
| Design 引用 | `STATE-2`/`INV-2`/`INV-3` / ADR-1/3 |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.1/6.2/6.4→Task-4→矩阵断言 |
| Code Quality | 锁域、引用计数、上限读取 |
| Verification | 单测全绿记录 |

### TASK-5: 动态重判与档位传播

| 字段 | 内容 |
|------|------|
| 任务目标 | 经注册监听感知创建者前后台状态切换（既有应用状态通知挂点）与负载等级跳变（SystemLoadProvider 转发），触发判定器重判；档位变化经 OnTaskLevelChanged 通知 process_schedule 增量重算（取最高档）并施加/撤销；档位不变不触发（天然防抖） |
| AC 映射 | AC-6.7 |
| 前置依赖 | TASK-3、TASK-4 |
| 非目标 | 负载接口语义确认（proposal 开放问题，设计评审闭环）；映射矩阵调整（版本迭代） |
| 完成判据 | 重判/重算/防抖用例全绿 |
| 停止条件 | 监听挂点需修改既有通知链主体逻辑 |
| Design 状态/不变量 | 消费 STATE-1/STATE-2；INV-4 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1/2（Consumes）+ INV-4 | 任务档位/归属索引 | task_mgr / process_schedule | 同 TASK-2/4 | 动态重判更新 | — | — | 档位不变不触发下游重算 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | 监听挂点回调（OnForegroundChanged / 负载等级监听） | TASK-5 → 判定器 | 内部 | 注册监听模型，无轮询 |
| Consumes | TASK-2 判定器接口；TASK-4 OnTaskLevelChanged 重算入口 | TASK-2/TASK-4 | 前置 Task | 档位变化才传播 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#时序设计`、`design.md#线程与并发模型` | 触发模型与防抖语义 |
| `services/appmgr/include/app_state_observer_manager.h:127`、`interfaces/inner_api/app_manager/include/appmgr/application_state_observer_stub.h:43` | 前后台事件挂点 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | services/appmgr/src/task_mgr/ | 判定器订阅挂点+重判触发 |
| Modify | services/appmgr/src/process_schedule/ | OnTaskLevelChanged 重算实现 |
| Test | test/unittest/task_mgr/ | PriorityRejudgeOnStateChange_0100 等动态重判用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| 既有通知链主体逻辑 | 最小挂点约束（design ADR-2） |
| 20260914692272 管控代理层 | 归属其需求 |

**Spec References**

- AC / Rules: `AC-6.7`, `R-1`, `R-2`
- Anchors: `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-6`, `INV-4`, `design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 重判事件串行处理于任务记录锁域，不并发重入 |

**Steps**

- [ ] 写失败测试：运行中切前后台/负载跳变断言档位重判与进程重算；档位不变断言无重算（Red：未实现前档位静止不重判）
- [ ] 运行验证确认 RED
- [ ] 挂接前后台状态与负载等级监听回调，实现重判与传播
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/include/app_state_observer_manager.h:127` | 通知链订阅风格 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-6.7 用例全绿（重判+重算+防抖断言） |
| Scope respected | 仅 task_mgr/ 与 process_schedule/ |
| State lifecycle complete | N/A + 理由：复用既有状态，无新增 |
| Interface contract matched | OnTaskLevelChanged 与 TASK-4 Produces 一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（task_mgr 重判用例） | 动态重判用例全绿 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-5：动态重判与档位传播 |
| 允许修改 | services/appmgr/src/task_mgr/、src/process_schedule/、test/unittest/task_mgr/ |
| 允许新建 | 重判相关用例 |
| 只读参考 | app_state_observer_manager.h |
| Spec 引用 | `AC-6.7` / `R-1`/`R-2` |
| Design 引用 | `INV-4` / ADR-6 |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.7→Task-5→重判/防抖断言 |
| Code Quality | 订阅挂点、事件串行 |
| Verification | 单测全绿记录 |

### TASK-6: 进程死亡清理挂点

| 字段 | 内容 |
|------|------|
| 任务目标 | 复用 AppMS 既有死亡通知链：死亡进程移除索引 + 释放豁免 + 档位重算（≤1s）；任务不自动结束 |
| AC 映射 | AC-6.3 |
| 前置依赖 | TASK-4 |
| 非目标 | CLI 进程死亡（20260914692193 会话链承载） |
| 完成判据 | 清理 ≤1s、任务存续用例全绿 |
| 停止条件 | 需修改既有通知链主体逻辑 |
| Design 状态/不变量 | 消费 STATE-2；INV-5 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-2（Consumes）+ INV-5 | 归属索引/豁免 | process_schedule | 同 TASK-4 | — | — | 进程死亡触发移除+重算 | ≤1s 清理、任务存续 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | 无 | — | — | — |
| Consumes | TASK-4 索引移除/豁免释放接口 | TASK-4 | 前置 Task | 锁域内执行 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `services/appmgr/include/app_death_recipient.h:27-29,62` | 既有通知链挂点 |
| `design.md#关键设计决策`（ADR-2） | 最小挂点约束 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | services/appmgr/include/app_death_recipient.h、src/app_death_recipient.cpp（或既有挂点注册处） | 死亡→process_schedule 清理回调 |
| Test | test/integration/ | ProcessDeathCleanup_0100、AllProcessesDeadTaskAlive_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| 既有通知链主体逻辑 | 最小挂点约束（design ADR-2） |

**Spec References**

- AC / Rules: `AC-6.3`, `R-5`, `R-8`
- Anchors: `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-2`, `INV-5`, `design.md#资源所有权矩阵`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 挂点最小化：只新增转发调用，不改通知链主体 |

**Steps**

- [ ] 写失败测试（清理 ≤1s、任务存续断言）
- [ ] 运行验证确认 RED
- [ ] 挂接既有通知链至 process_schedule 清理入口
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/include/app_death_recipient.h:62` `OnRemoteDied` | 死亡回调挂点 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-6.3 时延打点与任务存续记录（含 R-8 全部进程死亡场景） |
| Scope respected | 仅挂点文件与 integration 目录 |
| State lifecycle complete | 死亡清理覆盖索引移除+豁免释放+重算（INV-5） |
| Interface contract matched | 清理入口与 TASK-4 Produces 一致 |
| Relevant verification | 集成用例 + 时延打点 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | 集成用例 + `run -t UT -tp ability_runtime` | 清理 ≤1s、任务存续 | （回填） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-6：进程死亡清理挂点 |
| 允许修改 | services/appmgr/include/app_death_recipient.h、src/app_death_recipient.cpp、test/integration/ |
| 允许新建 | ProcessDeathCleanup_0100、AllProcessesDeadTaskAlive_0100 |
| 只读参考 | app_death_recipient.h |
| Spec 引用 | `AC-6.3` / `R-5`/`R-8` |
| Design 引用 | `INV-5` / ADR-2 |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | 集成用例 + `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-6.3→Task-6→时延/存续断言 |
| Code Quality | 挂点最小性 |
| Verification | 集成用例记录 |

## 代码范围映射

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | interface_sdk-js api/@ohos.app.agent.taskManager.d.ts | | Modify | | | | | |
| TASK-2 | services/appmgr/src/task_mgr/task_priority_judge.*、system_load_provider.*；test/unittest/task_mgr/ | | Create | | | | | |
| TASK-3 | interfaces/inner_api/app_manager（StartTask 链）、frameworks/ets/ets、frameworks/ets/ani/task_manager/、frameworks/js/napi/task_manager/、services/appmgr（app_mgr_service、task_mgr）、test/mock、test/integration/ | | Modify+Create | | | | | |
| TASK-4 | services/appmgr/src/process_schedule/；test/unittest/process_schedule/ | | Create | | | | | |
| TASK-5 | services/appmgr/src/task_mgr/、src/process_schedule/；test/unittest/task_mgr/ | | Modify+Create | | | | | |
| TASK-6 | services/appmgr/include/app_death_recipient.h、src/app_death_recipient.cpp；test/integration/ | | Modify+Create | | | | | |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-2 | dependency | 系统负载接口与资源调度领域确认（design 风险表 R1） | Open | ohos-design / yangzk |
| TASK-4 | dependency | 20260914692272 管控代理层交付时序 | Open | ohos-plan / yangzk |
