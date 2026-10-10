# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 用稳定 ID 和章节锚点引用上游事实，不复制 Spec/Design 正文。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260729708982 |
| 关联 Feature/Bug | FEAT-20260729708982 |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 简单 |
| 状态 | Draft |
| Owner | Ability SIG |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved；简单变更记录关键约束和 N/A |
| Spec | `spec.md` | Approved |

## 执行原则

- **Spec 权威：** 实现与 AC、错误码、兼容性声明冲突时，停止并修订 spec/design/plan，不能在代码里自行改语义。
- **测试/证据先行：** 每个 Task 先确认失败测试或可复现证据缺口，再做最小实现并取得 fresh PASS 证据。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环；跨接口、事件链、状态生命周期或构建边界的工作应拆分。
- **文件边界：** 只能修改 Task 的 Files/允许修改范围；需要额外文件时先修订 Plan。
- **状态所有权唯一：** 新增或修改状态必须明确 owner、key/index、创建、读取、更新、清理触发和不变量。
- **证据回填：** Task 完成当次会话回填追溯状态、实际文件、Commit、Review Evidence 和 Actual Result，不跨会话补证。
- **反伪完成：** 只补声明、只写存储、只覆盖 happy path、只跑无关测试或只填写 PASS 文本，都不能替代 AC 闭环。
- **生成文件：** 只修改生成源并运行生成命令；手工改生成物不能作为最终交付。

## 受影响文件全量清单

> 对照 design.md 调用链层级分析，逐层列出每层涉及的具体文件。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_ability_runtime | 服务（Intent）- 包事件处理 | `services/intent/src/package_event_handler.cpp` | 修改 | 接收安装/更新/卸载事件，按AppIndex分发到意图存储管理 |
| ability_ability_runtime | 服务（Intent）- 意图存储管理 | `services/intent/src/intent_storage.cpp` | 修改 | Key结构扩展AppIndex字段，按AppIndex精确存储意图；安装意图逻辑适配 |
| ability_ability_runtime | 服务（Intent）- 查询意图 | `services/intent/src/intent_query.cpp` | 修改 | 返回"同名同包+当前模式"合集，内部过滤当前模式 |
| ability_ability_runtime | 服务（Intent）- 执行意图 | `services/intent/src/intent_execute.cpp` | 修改 | 默认执行当前模式意图，优先匹配当前模式应用 |
| ability_ability_runtime | 服务（Intent）- 脏数据清理 | `services/intent/src/intent_cleanup.cpp` | 新增 | 执行失败时查询数据库并删除同名同包脏数据 |
| ability_ability_runtime | 测试 | `services/intent/test/intent_storage_test.cpp` | 修改 | 新增双模式安装/更新/卸载测试用例 |
| ability_ability_runtime | 测试 | `services/intent/test/intent_query_test.cpp` | 修改 | 新增双模式查询测试用例 |
| ability_ability_runtime | 测试 | `services/intent/test/intent_execute_test.cpp` | 修改 | 新增双模式执行测试用例 |
| ability_ability_runtime | 测试 | `services/intent/test/intent_cleanup_test.cpp` | 新增 | 新增脏数据清理测试用例 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出（包事件处理→意图存储管理→查询/执行接口→脏数据清理）
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失，需回溯补充 design.md

## AC 到 Task 追溯

> AC = Acceptance Criteria（验收标准）。spec 中每个验收标准必须映射到至少一个 Task。

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-1.1 | spec.md | TASK-1 | `intent_storage.cpp / Key结构定义、InstallIntent` | 待回填 | 待回填 | 待回填 | Pending |
| AC-1.2 | spec.md | TASK-1 | `intent_storage.cpp / Key结构定义、InstallIntent` | 待回填 | 待回填 | 待回填 | Pending |
| AC-2.1 | spec.md | TASK-2 | `intent_query.cpp / QueryIntent` | 待回填 | 待回填 | 待回填 | Pending |
| AC-3.1 | spec.md | TASK-2 | `intent_execute.cpp / ExecuteIntent` | 待回填 | 待回填 | 待回填 | Pending |
| AC-4.1 | spec.md | TASK-1 | `package_event_handler.cpp / 升级事件处理` | 待回填 | 待回填 | 待回填 | Pending |
| AC-5.1 | spec.md | TASK-1 | `package_event_handler.cpp / 卸载事件处理` | 待回填 | 待回填 | 待回填 | Pending |
| AC-6.1 | spec.md | TASK-2 | `intent_cleanup.cpp / CleanupOnFailure（新增）` | 待回填 | 待回填 | 待回填 | Pending |

## 首批实现边界

**首批必须实现：** 意图存储Key结构扩展AppIndex字段（TASK-1），安装/更新/卸载逻辑适配AppIndex（TASK-1），查询/执行接口适配（TASK-2），脏数据清理逻辑（TASK-2）
**可后置：** 无
**不建议延后：** Key结构扩展是全链路基础，延后导致后续适配无法进行

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环
- 简单变更：2 张 Task Card
- TASK-1 覆盖安装/更新/卸载能力闭环（Key结构 + 包事件处理）
- TASK-2 覆盖查询/执行/清理能力闭环（查询接口 + 执行接口 + 脏数据清理）

## 禁止项

执行计划和 Task 不得出现以下内容：

- [x] 没有未决标记 / 未办标记 / 占位符
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖必须显式声明在前置依赖列）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每个 Task 自包含）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | 意图存储Key结构扩展AppIndex + 安装/更新/卸载逻辑适配 + 测试 | `intent_storage.cpp`、`package_event_handler.cpp`、`intent_storage_test.cpp` | AC-1.1, AC-1.2, AC-4.1, AC-5.1 | 无 | Key结构扩展完成，安装/更新/卸载按AppIndex区分，VM-1/VM-2/VM-5/VM-6 通过 | `pytest tests/intent_storage_test.cpp -v` | Pending |
| TASK-2 | 查询/执行接口适配 + 脏数据清理 + 测试 | `intent_query.cpp`、`intent_execute.cpp`、`intent_cleanup.cpp`、`intent_query_test.cpp`、`intent_execute_test.cpp`、`intent_cleanup_test.cpp` | AC-2.1, AC-3.1, AC-6.1 | TASK-1 完成 | 查询返回合集并过滤当前模式，执行默认当前模式，执行失败清理脏数据，VM-3/VM-4/VM-7 通过 | `pytest tests/intent_query_test.cpp tests/intent_execute_test.cpp tests/intent_cleanup_test.cpp -v` | Pending |

## Task 详情

### TASK-1: 意图存储Key结构扩展 + 安装/更新/卸载逻辑适配

| 字段 | 内容 |
|------|------|
| 任务目标 | 意图存储Key结构扩展AppIndex字段，安装意图逻辑按AppIndex区分存储，包事件处理适配升级/卸载按AppIndex精确操作 |
| AC 映射 | AC-1.1, AC-1.2, AC-4.1, AC-5.1 |
| 前置依赖 | design.md + spec.md Approved；代码事实行号已对照源码确认 |
| 非目标 | 不涉及查询/执行接口适配（TASK-2）；不涉及脏数据清理（TASK-2） |
| 完成判据 | Key结构扩展AppIndex字段；同名同包Key不变、同名不同包Key增加appIndex；升级按模式更新并删除同名同包意图；卸载按AppIndex精确删除；VM-1/VM-2/VM-5/VM-6 通过 |
| 停止条件 | 发现Key结构与现有查询逻辑不兼容；包事件处理模块接口与设计不符 |
| Design 状态/不变量 | N/A — 本特性不涉及运行时状态/不变量 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A — 本特性不涉及运行时状态管理 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | 意图存储Key结构含AppIndex字段；安装意图按AppIndex区分存储 | TASK-1 | 供 TASK-2 查询/执行接口使用 | Key结构向后兼容：同名同包不带AppIndex，同名不同包带AppIndex |
| Consumes | 无 | TASK-1 | 无前置Task | 无 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `proposal.md` | 需求基线、技术方案、Key结构定义 |
| `design.md` | 代码事实基线、调用链层级分析、关键设计决策 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/intent/src/intent_storage.cpp` | Key结构定义扩展AppIndex字段；InstallIntent逻辑按AppIndex区分存储 |
| Modify | `services/intent/src/package_event_handler.cpp` | 升级事件处理按模式更新并删除同名同包意图；卸载事件处理按AppIndex精确删除 |
| Modify | `services/intent/test/intent_storage_test.cpp` | 新增VM-1（同名同包安装Key不变）、VM-2（同名不同包安装Key增加AppIndex）、VM-5（升级按模式更新+删除同名同包）、VM-6（卸载按AppIndex精确删除）测试用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `services/intent/src/intent_query.cpp` | TASK-2 所有 |
| `services/intent/src/intent_execute.cpp` | TASK-2 所有 |
| `services/intent/src/intent_cleanup.cpp` | TASK-2 所有（新增文件） |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `AC-4.1`, `AC-5.1`, `R-1`, `R-2`, `R-5`, `R-6`
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-1`, `design.md#关键设计决策`, `design.md#代码事实基线`, `design.md#调用链层级分析`, `design.md#既有模式复用`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must — 包事件处理→意图存储管理调用方向为服务层内部，无跨层违规 |
| OH-ARCH-API-LEVEL | Must Not — 不对外暴露AppIndex，Key结构为仓内部数据结构 |
| OH-ARCH-IPC-SAF | Must Not — 不引入跨进程IPC，意图管理在进程内完成 |

**Steps**

- [ ] 对照源码确认intent_storage.cpp和package_event_handler.cpp的文件:行/符号，更新design代码事实基线
- [ ] 写失败测试：VM-1（同名同包安装Key不变）、VM-2（同名不同包安装Key增加AppIndex）、VM-5（升级按模式更新+删除同名同包）、VM-6（卸载按AppIndex精确删除）
- [ ] 运行测试，确认当前失败（Key结构未扩展，安装/更新/卸载不区分AppIndex）
- [ ] 做最小实现：Key结构定义扩展AppIndex字段；InstallIntent按AppIndex区分存储；PackageEventHandler升级/卸载事件适配
- [ ] 运行测试，确认 VM-1/VM-2/VM-5/VM-6 通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/intent/src/intent_storage.cpp:1 / Key拼接逻辑` | Key按字段顺序拼接存储，查询按Key前缀匹配 | 扩展：在现有Key末尾追加appIndex字段，同名同包appIndex=0时不追加 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.1/AC-1.2（安装Key区分）、AC-4.1（升级按模式更新）、AC-5.1（卸载按模式删除）的正向路径证据 |
| Scope respected | 实际 diff 仅涉及 intent_storage.cpp、package_event_handler.cpp 及对应测试文件 |
| State lifecycle complete | N/A — 不涉及状态生命周期 |
| Interface contract matched | Produces Key结构含AppIndex，TASK-2 Consumes 时签名一致 |
| Relevant verification | VM-1/VM-2/VM-5/VM-6 直接覆盖本 Task，非无关测试 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `pytest tests/intent_storage_test.cpp -v` | VM-1/VM-2/VM-5/VM-6 全部 PASS | 待回填 |
| 静态检查 | `build.sh` | 编译通过，无新增 error | 待回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：意图存储Key结构扩展AppIndex + 安装/更新/卸载逻辑适配 + 测试 |
| 允许修改 | `services/intent/src/intent_storage.cpp`、`services/intent/src/package_event_handler.cpp`、`services/intent/test/intent_storage_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `proposal.md`、`design.md`、`spec.md` |
| Spec 引用 | `AC-1.1` / `AC-1.2` / `AC-4.1` / `AC-5.1` / `R-1` / `R-2` / `R-5` / `R-6` / `spec.md#验收追溯` / `spec.md#规则定义` |
| Design 引用 | `ADR-1` / `design.md#关键设计决策` / `design.md#代码事实基线` / `design.md#调用链层级分析` |
| 执行步骤 | 本 Task Card 的 Steps |
| 验证命令 | `pytest tests/intent_storage_test.cpp -v` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.1/AC-1.2/AC-4.1/AC-5.1→Task→Code→Commit、Key结构边界、安装/更新/卸载行为 |
| Code Quality | 实际代码范围、Key结构兼容性、包事件处理适配质量 |
| Verification | Expected/Actual、VM-1/VM-2/VM-5/VM-6 fresh evidence |

### TASK-2: 查询/执行接口适配 + 脏数据清理 + 测试

| 字段 | 内容 |
|------|------|
| 任务目标 | 查询意图接口返回"同名同包+当前模式"合集并过滤当前模式，执行意图默认当前模式优先匹配，新增脏数据清理逻辑 |
| AC 映射 | AC-2.1, AC-3.1, AC-6.1 |
| 前置依赖 | TASK-1 完成（依赖Key结构含AppIndex字段）；代码事实行号已对照源码确认 |
| 非目标 | 不涉及Key结构定义（TASK-1）；不涉及安装/更新/卸载逻辑（TASK-1） |
| 完成判据 | 查询返回合集并过滤当前模式；执行默认当前模式优先匹配；执行失败清理同名同包脏数据；VM-3/VM-4/VM-7 通过 |
| 停止条件 | 查询接口与TASK-1 Key结构不兼容；执行接口无法获取当前模式AppIndex |
| Design 状态/不变量 | N/A — 本特性不涉及运行时状态/不变量 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A — 本特性不涉及运行时状态管理 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | 查询返回合集并过滤当前模式；执行默认当前模式；脏数据清理逻辑 | TASK-2 | 供验证使用 | 查询/执行/清理行为与Key结构含AppIndex一致 |
| Consumes | 意图存储Key结构含AppIndex字段 | TASK-2 | 来自 TASK-1 | Key结构向后兼容：同名同包不带AppIndex，同名不同包带AppIndex |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `proposal.md` | 需求基线、技术方案、查询/执行/清理逻辑 |
| `design.md` | 代码事实基线、调用链层级分析、关键设计决策 |
| `services/intent/src/intent_storage.cpp` | TASK-1 产出的Key结构定义（只读，确认AppIndex字段格式） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/intent/src/intent_query.cpp` | 查询逻辑返回"同名同包+当前模式"合集，内部过滤当前模式 |
| Modify | `services/intent/src/intent_execute.cpp` | 执行逻辑默认当前模式，优先匹配当前模式应用 |
| Create | `services/intent/src/intent_cleanup.cpp` | 新增脏数据清理逻辑：执行失败时查询数据库并删除同名同包脏数据 |
| Modify | `services/intent/test/intent_query_test.cpp` | 新增VM-3（查询返回合集并过滤当前模式）测试用例 |
| Modify | `services/intent/test/intent_execute_test.cpp` | 新增VM-4（执行默认当前模式）测试用例 |
| Create | `services/intent/test/intent_cleanup_test.cpp` | 新增VM-7（执行失败清理脏数据）测试用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `services/intent/src/intent_storage.cpp` | TASK-1 所有 |
| `services/intent/src/package_event_handler.cpp` | TASK-1 所有 |

**Spec References**

- AC / Rules: `AC-2.1`, `AC-3.1`, `AC-6.1`, `R-3`, `R-4`, `R-7`
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-2`, `ADR-3`, `ADR-4`, `design.md#关键设计决策`, `design.md#代码事实基线`, `design.md#调用链层级分析`, `design.md#既有模式复用`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must — 查询/执行/清理在服务层内部完成，无跨层违规 |
| OH-ARCH-API-LEVEL | Must Not — 不对外暴露AppIndex，查询/执行/清理为仓内部逻辑 |
| OH-ARCH-IPC-SAF | Must Not — 不引入跨进程IPC，查询/执行/清理在进程内完成 |

**Steps**

- [ ] 对照源码确认intent_query.cpp、intent_execute.cpp的文件:行/符号，更新design代码事实基线
- [ ] 写失败测试：VM-3（查询返回合集并过滤当前模式）、VM-4（执行默认当前模式）、VM-7（执行失败清理脏数据）
- [ ] 运行测试，确认当前失败（查询不返回合集、执行不区分模式、无脏数据清理）
- [ ] 做最小实现：查询逻辑返回合集并过滤当前模式；执行逻辑默认当前模式优先匹配；新增intent_cleanup.cpp脏数据清理逻辑
- [ ] 运行测试，确认 VM-3/VM-4/VM-7 通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/intent/src/intent_query.cpp:1 / IntentQuery` | 查询按Key前缀匹配返回结果 | 扩展：查询返回"同名同包+当前模式"合集，增加当前模式过滤 |
| `services/intent/test/intent_storage_test.cpp:1 / UT fixture` | 单测验证意图存储行为和查询返回值 | 直接复用：新增测试用例挂载到已有test fixture |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-2.1（查询合集）、AC-3.1（执行当前模式）、AC-6.1（清理脏数据）的正向和恢复路径证据 |
| Scope respected | 实际 diff 仅涉及 intent_query.cpp、intent_execute.cpp、intent_cleanup.cpp（新增）及对应测试文件 |
| State lifecycle complete | N/A — 不涉及状态生命周期 |
| Interface contract matched | Consumes TASK-1 的Key结构含AppIndex，查询/执行/清理行为与Key结构一致 |
| Relevant verification | VM-3/VM-4/VM-7 直接覆盖本 Task，非无关测试 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `pytest tests/intent_query_test.cpp tests/intent_execute_test.cpp tests/intent_cleanup_test.cpp -v` | VM-3/VM-4/VM-7 全部 PASS | 待回填 |
| 静态检查 | `build.sh` | 编译通过，无新增 error | 待回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：查询/执行接口适配 + 脏数据清理 + 测试 |
| 允许修改 | `services/intent/src/intent_query.cpp`、`services/intent/src/intent_execute.cpp`、`services/intent/test/intent_query_test.cpp`、`services/intent/test/intent_execute_test.cpp` |
| 允许新建 | `services/intent/src/intent_cleanup.cpp`、`services/intent/test/intent_cleanup_test.cpp` |
| 只读参考 | `proposal.md`、`design.md`、`spec.md`、`services/intent/src/intent_storage.cpp` |
| Spec 引用 | `AC-2.1` / `AC-3.1` / `AC-6.1` / `R-3` / `R-4` / `R-7` / `spec.md#验收追溯` / `spec.md#规则定义` |
| Design 引用 | `ADR-2` / `ADR-3` / `ADR-4` / `design.md#关键设计决策` / `design.md#代码事实基线` / `design.md#调用链层级分析` |
| 执行步骤 | 本 Task Card 的 Steps |
| 验证命令 | `pytest tests/intent_query_test.cpp tests/intent_execute_test.cpp tests/intent_cleanup_test.cpp -v` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-2.1/AC-3.1/AC-6.1→Task→Code→Commit、查询/执行/清理行为边界 |
| Code Quality | 实际代码范围、查询过滤逻辑质量、脏数据清理逻辑质量、新增文件规范 |
| Verification | Expected/Actual、VM-3/VM-4/VM-7 fresh evidence |

## 代码范围映射

> Plan 审批时填写计划范围；实现后如实回填实际范围和 Commit。

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | `services/intent/src/intent_storage.cpp:Key结构定义` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-1 | `services/intent/src/intent_storage.cpp:InstallIntent` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-1 | `services/intent/src/package_event_handler.cpp:升级/卸载事件处理` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-2 | `services/intent/src/intent_query.cpp:QueryIntent` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-2 | `services/intent/src/intent_execute.cpp:ExecuteIntent` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-2 | `services/intent/src/intent_cleanup.cpp:CleanupOnFailure` | 待回填 | Add | 待回填 | 待回填 | 待回填 | 待回填 | 无 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-1 | 依赖 | 代码事实行号待源码验证（design.md代码事实基线行号为估算值） | Blocked | 实现前对照源码确认 / Ability SIG |
| TASK-1 | 设计 | AppIndex值映射确认（PC/PAD模式对应的具体AppIndex值） | Blocked | proposal §7 假设确认 / Ability SIG |
| TASK-2 | 依赖 | 依赖TASK-1 Key结构含AppIndex字段 | Blocked | TASK-1完成后解除 / Ability SIG |
