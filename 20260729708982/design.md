# 架构设计

> 确认目标仓和模块的架构约束、关键设计决策、Spec 拆分方向。简单变更（单仓小修、无新 API）可简写关键约束、对不适用检查项填写 N/A，但 `design.md` 是四件套必需件，不得省略。

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-20260729708982 |
| 关联需求 | `proposal.md` |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-20260729708982 |
| 复杂度 | 简单 |
| 目标版本 | 引用 proposal.target_release |
| Owner | Ability SIG |
| 状态 | Draft |

## 需求基线

> 需求基线详见 proposal.md。以下仅列出设计阶段需要额外强调的要点。

| 项 | 补充说明（如需） |
|----|------------------|
| AppIndex通过inner接口获取 | 设备模式（PC/PAD）通过inner接口获取，返回两种不同AppIndex值；同名同包AppIndex=0，同名不同包AppIndex≠0。AppIndex是双模式区分的核心字段 |
| Key结构扩展向后兼容 | 同名同包应用Key不变（不带AppIndex），同名不同包应用Key增加AppIndex字段。现有查询逻辑可向后兼容（proposal §9 假设） |
| 单仓实现 | 仅ability_ability_runtime仓intent模块内部变更，不跨仓、不跨子系统 |
| 意图管理在进程内完成 | 安装/查询/执行/更新/卸载均为仓内部操作，不引入跨进程IPC |

## 上下文和现状

### 涉及仓和模块

> 仓、模块、当前职责、影响类型详见 proposal.md"影响范围"。本节补充设计层面的架构现状。

| 仓库 | 补充架构说明 |
|------|-------------|
| openharmony/ability_ability_runtime | intent模块负责意图存储管理、包事件处理适配、查询/执行接口和脏数据清理。当前意图存储Key结构为 `userID+bundleName+moduleName+intentName+version`，仅按包名区分，无法满足PC双模式同名不同包应用精确操作需求。本次改动在Key结构上增加AppIndex字段，并适配安装/查询/执行/更新/卸载/清理全链路。 |

### 代码事实基线

> **条件必填：** 变更涉及已有模块、接口、数据结构、调用链或运行时行为时填写。

| 代码引用（文件:行/符号） | 已验证事实 | 关联架构规则 | 对本设计的约束 | 验证来源/可信度 |
|-------------------------|------------|--------------|----------------|-----------------|
| `services/intent/src/intent_storage.cpp:1 / IntentStorage::GetKey` | 当前Key结构为 `userID+bundleName+moduleName+intentName+version`，仅按包名区分意图 | OH-ARCH-API-LEVEL | Key结构须扩展AppIndex字段；同名同包Key不变（向后兼容），同名不同包Key增加AppIndex | proposal §11 技术方案，中（文件:行待源码验证） |
| `services/intent/src/intent_storage.cpp:1 / IntentStorage::InstallIntent` | 安装意图时按包名拼接Key存储，不区分AppIndex | OH-ARCH-API-LEVEL | 安装逻辑须按AppIndex区分：同名同包走原有逻辑，同名不同包Key增加appIndex | proposal §11，中（文件:行待源码验证） |
| `services/intent/src/intent_query.cpp:1 / IntentQuery::QueryIntent` | 查询意图仅按包名返回结果，不区分模式 | OH-ARCH-API-LEVEL | 查询逻辑须返回"同名同包意图+当前模式意图"合集，内部自动过滤当前模式 | proposal §11，中（文件:行待源码验证） |
| `services/intent/src/intent_execute.cpp:1 / IntentExecute::ExecuteIntent` | 执行意图不区分模式，可能执行非当前模式意图 | OH-ARCH-API-LEVEL | 执行逻辑须默认执行当前模式意图，优先匹配当前模式应用 | proposal §11，中（文件:行待源码验证） |
| `services/intent/src/package_event_handler.cpp:1 / PackageEventHandler` | 接收安装/更新/卸载事件并操作意图存储 | OH-ARCH-LAYERING | 包事件处理须适配AppIndex：升级时按模式更新并删除同名同包意图；卸载时按AppIndex精确删除 | proposal §6，中（文件:行待源码验证） |
| `services/intent/src/intent_cleanup.cpp:1 / IntentCleanup` | 当前无执行失败时的脏数据清理逻辑 | OH-ARCH-API-LEVEL | 新增脏数据清理逻辑：执行失败时向数据库查询并删除同名同包脏数据 | proposal §11，中（文件:行待源码验证） |

**基线规则：**

- 设计结论与代码事实冲突时，先确认目标版本和分支；不得直接忽略现状。
- 上述代码事实基于 proposal.md 技术方案描述，具体文件:行/符号须在实现前对照源码验证确认后进入 Approved 设计约束。
- execution-plan 的文件范围必须能追溯到本表、调用链分析或明确的新建文件决策。

### 调用链层级分析

> 从最上层到最底层逐层扫描调用链路。

| 层 | 模块 | 职责 | 修改类型 |
|----|------|------|----------|
| 服务（Intent） | 包事件处理适配模块 | 接收安装/更新/卸载事件，按AppIndex分发到意图存储管理 | 修改 |
| 服务（Intent） | 意图存储管理模块 | Key结构扩展AppIndex字段，按AppIndex精确存储意图 | 修改 |
| 服务（Intent） | 查询意图接口 | 返回"同名同包+当前模式"合集，内部过滤当前模式 | 修改 |
| 服务（Intent） | 执行意图接口 | 默认执行当前模式意图，优先匹配当前模式应用 | 修改 |
| 服务（Intent） | 脏数据清理逻辑 | 执行失败时查询数据库并删除同名同包脏数据 | 新增 |

**检查项：**
- [x] 调用链每一层都已覆盖（包事件处理 → 意图存储管理 → 查询/执行接口 → 脏数据清理）
- [x] 每层职责边界清晰，无跨层违规调用（全部在intent模块服务层内完成，无跨进程IPC）
- [x] 每层修改类型明确

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | 改动全部在intent模块服务层内完成 | 调用方向为包事件处理→意图存储管理→查询/执行接口→脏数据清理，无跨层违规；不引入跨进程IPC | 架构评审/依赖检查 |
| OH-ARCH-SUBSYSTEM | 仅ability_ability_runtime单仓变更 | 不涉及跨子系统调用；AppIndex通过inner接口获取，为仓内部接口 | 代码评审/依赖检查 |
| OH-ARCH-IPC-SAF | 意图管理操作在进程内完成 | 无新增IPC通道；安装/查询/执行/更新/卸载均为进程内数据库操作 | 集成测试 |
| OH-ARCH-API-LEVEL | 不涉及Public/System/InnerAPI接口变更 | 本次为内部Key结构扩展，不对外暴露AppIndex；不涉及API新增/变更/废弃 | API 评审 |
| OH-ARCH-COMPONENT-BUILD | 不新增component，不新增外部依赖 | BUILD.gn无新增源文件（均为已有文件修改+新增脏数据清理逻辑） | 构建验证 |
| OH-ARCH-ERROR-LOG | 不涉及错误码变更 | 意图管理操作不返回错误码（内部数据库操作） | 单测 |

### 既有模式复用

> **条件必填：** 目标仓已有相邻能力、同层接口、错误处理、资源管理或测试模式可参考时填写。

| 模式 | 参考代码（文件:行/符号） | 适用架构规则 | 必须复用的约定 | 本设计的复用方式 | 偏差及理由 | 关联 Task |
|------|-------------------------|--------------|------------------|------------------|------------|-----------|
| 意图存储Key拼接模式 | `services/intent/src/intent_storage.cpp:1 / Key拼接逻辑` | OH-ARCH-API-LEVEL | Key按字段顺序拼接存储，查询按Key前缀匹配 | 扩展：在现有Key末尾追加appIndex字段，同名同包appIndex=0时不追加（保持向后兼容） | 无偏差 | TASK-1 |
| 包事件处理模式 | `services/intent/src/package_event_handler.cpp:1 / PackageEventHandler` | OH-ARCH-LAYERING | 包事件处理模块接收事件后操作意图存储 | 扩展：安装/更新/卸载事件处理增加AppIndex区分逻辑 | 无偏差 | TASK-2 |
| 查询意图模式 | `services/intent/src/intent_query.cpp:1 / IntentQuery` | OH-ARCH-API-LEVEL | 查询按Key前缀匹配返回结果 | 扩展：查询返回"同名同包+当前模式"合集，增加当前模式过滤 | 无偏差 | TASK-3 |
| 测试模式 | `services/intent/test/intent_storage_test.cpp:1 / UT fixture` | OH-ARCH-API-LEVEL | 单测验证意图存储行为和查询返回值 | 直接复用：新增测试用例挂载到已有test fixture | 无偏差 | TASK-5 |

## 不涉及项承接

> proposal.md 已完成 N/A 判定。本节仅对 proposal 中标记为"涉及"且需展开设计的维度给出结论。

| 维度 | 设计结论 |
|------|----------|
| 版本升级 | 升级场景按模式更新意图并删除同名同包意图（AC-4.1）。设计实现：包事件处理模块接收到升级事件时，按目标AppIndex更新意图存储，同时删除同名同包意图（AppIndex=0），避免升级后脏数据残留。 |
| 生态兼容 | 同名不同包应用通过AppIndex精确区分（AC-1.2）。设计实现：意图存储Key增加AppIndex字段，查询/执行/卸载均按AppIndex精确操作，不影响另一模式应用意图。 |

> proposal 中标记"不涉及"的维度（性能、安全/权限、兼容性、API/SDK、IPC/跨进程、构建/组件、国际化/无障碍、数据迁移）均确认 N/A，本设计不展开。

## 关键设计决策

> 简单变更可只填推荐方案。

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|------|------|
| ADR-1 | 如何扩展意图存储Key结构以区分双模式？ | 在现有Key末尾追加AppIndex字段：同名同包（AppIndex=0）Key不变，同名不同包（AppIndex≠0）Key增加appIndex。向后兼容 | 备选1：所有Key统一增加AppIndex字段（包括同名同包）——放弃，破坏向后兼容，现有查询逻辑需全量适配；备选2：为双模式新建独立Key命名空间——放弃，过度工程化，增加查询复杂度 | 在Key末尾追加AppIndex，同名同包不追加保持兼容；改动最小化，现有查询逻辑可向后兼容 | 影响 AC-1.1, AC-1.2 |
| ADR-2 | 查询如何处理双模式意图？ | 返回"同名同包意图+当前模式意图"合集，内部自动过滤当前模式。查询时按设备当前模式AppIndex过滤 | 备选1：返回所有模式意图，由调用方过滤——放弃，调用方需感知模式逻辑，职责泄漏；备选2：仅返回当前模式意图——放弃，丢失同名同包意图数据 | 内部自动过滤当前模式，调用方无需感知模式；返回合集保证同名同包意图不丢失 | 影响 AC-2.1 |
| ADR-3 | 执行如何默认当前模式？ | 执行意图时优先匹配当前模式AppIndex，当前模式无匹配时回退到同名同包意图 | 备选1：仅执行当前模式意图，无匹配时报错——放弃，可能导致同名同包意图无法执行；备选2：随机选择一个模式——放弃，行为不确定 | 优先匹配当前模式，无匹配时回退同名同包，行为确定且向后兼容 | 影响 AC-3.1 |
| ADR-4 | 脏数据清理策略？ | 意图执行失败时向数据库查询同名同包意图，删除执行失败产生的脏数据 | 备选1：执行失败时不清理，依赖下次安装覆盖——放弃，脏数据残留导致后续查询异常；备选2：定时清理——放弃，实时性差，需额外定时器资源 | 执行失败即时清理，保证数据库一致性；复用现有数据库查询接口 | 影响 AC-6.1 |

## 设计骨架

### 骨架范围

| 骨架项 | 目标 | 不包含 | 验证方式 |
|--------|------|--------|----------|
| 数据模型骨架 | 意图存储Key结构定义扩展AppIndex字段 | 完整查询/执行逻辑 | 编译通过 |
| 模块骨架 | 包事件处理/查询/执行接口适配AppIndex参数 | 完整双模式逻辑 | 构建通过 |
| 测试骨架 | intent模块UT新增双模式测试入口 | 全场景 | 最小用例通过 |

### 骨架 Spec 拆分

| Task ID | 目标 | 受影响文件 | AC |
|---------|------|------------|-----|
| TASK-SKELETON-1 | 建立Key结构骨架：意图存储Key定义扩展AppIndex字段 | intent模块 / Key结构定义 | WHEN Key结构定义完成 THEN 编译通过 |
| TASK-SKELETON-2 | 建立接口骨架：查询/执行接口适配AppIndex参数 | intent模块 / 查询接口、执行接口 | WHEN 接口签名变更 THEN 编译通过 |

## 后续 Task 拆分

> design.md 以 proposal.md 和已批准的 spec.md 为上游输入。本节描述 design 输出到 Plan 阶段的衔接信息。

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-1 | 意图存储Key结构扩展：Key定义增加AppIndex字段，安装意图逻辑按AppIndex区分存储 | intent模块 / Key结构定义、安装意图逻辑 | design.md + spec.md Approved；TASK-SKELETON-1 完成 |
| TASK-2 | 包事件处理适配：升级时按模式更新意图并删除同名同包意图；卸载时按AppIndex精确删除 | intent模块 / 包事件处理适配模块 | TASK-1 完成 |
| TASK-3 | 查询/执行接口适配：查询返回"同名同包+当前模式"合集并过滤当前模式；执行默认当前模式优先匹配 | intent模块 / 查询意图接口、执行意图接口 | TASK-1 完成；TASK-SKELETON-2 完成 |
| TASK-4 | 脏数据清理逻辑：执行失败时查询数据库并删除同名同包脏数据 | intent模块 / 脏数据清理逻辑（新增） | TASK-1~TASK-3 完成 |
| TASK-5 | 测试用例补齐：VM-1~VM-7 全场景单测 | test 目录 | TASK-1~TASK-4 完成 |

## API 签名、Kit 与权限

> 本节承接 spec.md"API 变更分析"中识别的 API，给出签名、权限和 d.ts 位置等实现细节。

N/A：本次不涉及Public/System/InnerAPI接口的新增、变更或废弃。AppIndex字段为意图存储内部数据结构，不对外暴露。详见 spec.md"API 变更分析"。

## 构建系统影响

### BUILD.gn 变更

```
文件路径: 无新增源文件
变更说明: 所有改动均为intent模块已有文件修改（Key结构定义、安装/查询/执行/更新/卸载逻辑适配）+新增脏数据清理逻辑。BUILD.gn无需变更，脏数据清理逻辑在已有模块内新增。
```

### bundle.json 变更

无新增 component，无新增依赖关系。所有改动在 ability_ability_runtime 部件内部完成。

---

## 可选设计扩展

> 简单变更按需展开以下章节。

### 架构图

> 涉及intent模块内多子模块协作，展开模块依赖关系。

```
[包事件处理适配模块] ──安装/更新/卸载事件──→ [意图存储管理模块（Key+AppIndex）]
                                                    │
                                                    │──查询──→ [查询意图接口] → 返回"同名同包+当前模式"合集
                                                    │
                                                    │──执行──→ [执行意图接口] → 默认当前模式
                                                    │
                                                    │──执行失败──→ [脏数据清理逻辑] → 删除同名同包脏数据
```

### 数据流/控制流

> 涉及主流程（安装/查询/执行/更新/卸载/清理）。

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | 包管理 | 包事件处理适配模块 | 安装/更新/卸载事件 + AppIndex | 包事件携带AppIndex，适配模块按AppIndex分发 |
| 2 | 包事件处理适配模块 | 意图存储管理模块 | Key（含AppIndex） + 操作类型 | 安装：同名同包Key不变，同名不同包Key增加appIndex；更新：按模式更新+删除同名同包；卸载：按AppIndex删除 |
| 3 | 查询请求 | 查询意图接口 | 当前模式AppIndex | 返回"同名同包+当前模式"合集，过滤非当前模式 |
| 4 | 执行请求 | 执行意图接口 | 当前模式AppIndex | 默认执行当前模式意图，优先匹配当前模式应用 |
| 5 | 执行失败 | 脏数据清理逻辑 | 数据库查询 | 查询同名同包意图并删除脏数据 |

### 测试性设计

> 涉及单测。

| 测试层级 | 测试目标 | Mock 策略 | 验证方式 |
|----------|----------|-----------|----------|
| 单元测试 | 同名同包安装Key不变（VM-1） | Mock包事件 | `intent_storage_test::InstallSameNameSamePackage` |
| 单元测试 | 同名不同包安装Key增加AppIndex（VM-2） | Mock包事件 | `intent_storage_test::InstallSameNameDifferentPackage` |
| 单元测试 | 查询返回合集并过滤当前模式（VM-3） | Mock设备模式 | `intent_query_test::QueryReturnsUnionOfSameNameAndCurrentMode` |
| 单元测试 | 执行默认当前模式（VM-4） | Mock设备模式 | `intent_execute_test::ExecuteDefaultsToCurrentMode` |
| 单元测试 | 升级按模式更新+删除同名同包（VM-5） | Mock包事件 | `intent_update_test::UpdateByModeAndDeleteSameName` |
| 单元测试 | 卸载按AppIndex精确删除（VM-6） | Mock包事件 | `intent_uninstall_test::UninstallByModeOnly` |
| 单元测试 | 执行失败清理脏数据（VM-7） | Mock执行失败 | `intent_cleanup_test::CleanupOnExecutionFailure` |

## 风险和开放问题

| 项 | 类型 | 影响 | 处理方式 | Owner |
|----|------|------|----------|-------|
| 代码事实行号待验证 | 架构 | 低 | 实现前对照源码确认intent模块各子模块文件:行/符号，确认后进入Approved设计约束 | Ability SIG |
| AppIndex值映射确认 | 设计 | 低 | 确认PC模式和PAD模式分别对应的AppIndex值（proposal假设为两种不同非零值） | Ability SIG |
| 各模块Owner确认 | 流程 | 低 | proposal §7 开放问题：各模块Owner待确认 | Ability SIG |

## 设计审批

- [x] 需求基线已确认，设计覆盖 P2 AC（AC-1.1~AC-6.1）
- [x] 不涉及项已承接，N/A 和展开项都有结论
- [x] 涉及仓和模块职责清楚（单仓 intent 模块 5 子模块）
- [x] 调用链层级分析完整，每层覆盖到位
- [x] 涉及既有实现时，代码事实基线使用文件:行/符号引用，且每项事实已转化为设计约束（行号待源码验证）
- [x] 已检索并记录既有模式复用
- [x] 适用架构规则已识别并形成设计结论
- [x] 分层和子系统边界合规
- [x] API 变更有签名、权限、错误码和兼容性说明（N/A，不涉及API变更）
- [x] BUILD.gn/bundle.json 影响明确
- [x] 设计输出和后续 Task 拆分明确
- [x] 关键设计决策有理由和影响说明
- [x] 风险和开放问题有 Owner

**结论:** 通过
