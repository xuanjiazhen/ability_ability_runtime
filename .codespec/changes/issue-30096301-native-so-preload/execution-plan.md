# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 用稳定 ID 和章节锚点引用上游事实，不复制 Spec/Design 正文。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-30096301 |
| 关联 Feature/Bug | FEAT-30096301 |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved |
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
| ability_runtime | 框架（采集组件） | `interfaces/kits/native/appkit/app_startup/native_module_collector.h` | 新增 | NativeModuleCollector 头文件 |
| ability_runtime | 框架（采集组件） | `frameworks/native/appkit/app_startup/native_module_collector.cpp` | 新增 | 采集聚合 + EWMA + 原子落盘 + 跨进程互斥 |
| ability_runtime | 框架（启动框架） | `interfaces/kits/native/appkit/app_startup/startup_manager.h` | 修改 | 预测预加载任务与采集调度声明、开关成员 |
| ability_runtime | 框架（启动框架） | `frameworks/native/appkit/app_startup/startup_manager.cpp` | 修改 | 预测预加载任务、采集调度、参数开关、故障事件 |
| ability_runtime | 框架（启动主流程） | `frameworks/native/appkit/app/main_thread.cpp` | 修改 | 采集窗口锚点挂接（如需） |
| ability_runtime | 框架（启动主流程） | `interfaces/kits/native/appkit/app/main_thread.h` | 修改 | 挂接声明（如需） |
| ability_runtime | 框架（构建） | `frameworks/native/appkit/BUILD.gn` | 修改 | 新增 collector 源文件 |
| ability_runtime | 测试 | `test/unittest/frameworks_kits_appkit_native_test/native_module_collector_test.cpp` | 新增 | collector/预加载单测 |
| ability_runtime | 测试 | `test/unittest/frameworks_kits_appkit_native_test/BUILD.gn` | 修改 | 加入新测试源文件 |
| ability_runtime | 测试 | `test/mock/frameworks_kits_appkit_native_test/include/mock_native_module_manager.h` | 修改 | mock 注入已加载系统模块与预加载失败 |
| ability_runtime | 测试 | `test/mock/frameworks_kits_appkit_native_test/include/mock_native_module_manager.cpp` | 修改 | mock 实现 |
| napi | NAPI 部件 | `foundation/arkui/napi/module_manager/native_module_manager.h` | 修改 | 新增 2 个 InnerAPI 声明 |
| napi | NAPI 部件 | `foundation/arkui/napi/module_manager/native_module_manager.cpp` | 修改 | 枚举系统模块名 + 预加载实现 |
| napi | NAPI 部件 | `foundation/arkui/napi/module_manager/BUILD.gn` | 修改 | 链接 musl `dlopen_preload`（如需） |
| napi | 测试 | `foundation/arkui/napi/test/unittest/` | 修改/新增 | InnerAPI 单测 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失，需回溯补充 design.md

## AC 到 Task 追溯

> AC = Acceptance Criteria（验收标准）。spec 中每个验收标准必须映射到至少一个 Task。

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-1 | spec.md | TASK-1, TASK-3, TASK-5, TASK-6 | `native_module_manager.{h,cpp}; native_module_collector.{h,cpp}; startup_manager.cpp; main_thread.cpp` | 待回填 | 待回填 | 待回填 | Pending |
| AC-2 | spec.md | TASK-3, TASK-6 | `native_module_collector.{h,cpp}` | 待回填 | 待回填 | 待回填 | Pending |
| AC-3 | spec.md | TASK-2, TASK-4, TASK-6 | `native_module_manager.{h,cpp}; startup_manager.{h,cpp}` | 待回填 | 待回填 | 待回填 | Pending |
| AC-4 | spec.md | TASK-2, TASK-3, TASK-6 | `native_module_manager.cpp; native_module_collector.cpp` | 待回填 | 待回填 | 待回填 | Pending |
| AC-5 | spec.md | TASK-4, TASK-6 | `startup_manager.{h,cpp}` | 待回填 | 待回填 | 待回填 | Pending |
| AC-6 | spec.md | TASK-3, TASK-6 | `native_module_collector.cpp` | 待回填 | 待回填 | 待回填 | Pending |
| AC-7 | spec.md | TASK-4, TASK-6 | `startup_manager.cpp` | 待回填 | 待回填 | 待回填 | Pending |
| AC-8 | spec.md | TASK-1, TASK-6 | `native_module_manager.{h,cpp}` | 待回填 | 待回填 | 待回填 | Pending |
| AC-9 | spec.md | TASK-3, TASK-6 | `native_module_collector.cpp` | 待回填 | 待回填 | 待回填 | Pending |
| AC-10 | spec.md | TASK-6 | `test/unittest/frameworks_kits_appkit_native_test/*` | 待回填 | 待回填 | 待回填 | Pending |

## 首批实现边界

**首批必须实现：** napi 两个 InnerAPI（TASK-1/2）、collector 采集+预测+落盘（TASK-3）、启动框架预测预加载与调度（TASK-4）、参数开关。
**可后置：** EWMA 参数调优、采集窗口时长校准。
**不建议延后：** 防自反馈（AC-4）、文件异常降级（AC-6）、多实例并发合并（AC-9）——延后会导致主链不闭合。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 骨架/基础接线（napi InnerAPI + collector 骨架） | TASK-1, TASK-2, TASK-3 | 编译通过、最小用例可跑 | `./build.sh --product-name <p> --build-target ability_runtime` |
| Phase-2 | 规则层/完整层（EWMA/并发/预加载/开关/测试） | TASK-4, TASK-5, TASK-6 | 全量单测通过 | 单测命令见 Task |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环
- 两项改动共享同一规则上下文和同一最小验证闭环 → 合并为一张 Task Card
- 文件范围、验证闭环和风险边界足够分离 → 拆分为多张 Task Card
- 简单变更：1-2 张 Task Card；复杂变更：按接线层/规则层/恢复层拆分
- 每个 Task 是可独立验证的最小能力闭环，任务边界由能力闭环决定，不单纯按代码行数或预估时间划定
- 单 Task 上下文（Spec + Design + 代码 + 测试）超过 3000 行时优先沿 AC/文件边界拆分；Handoff Summary 只引用本 Task 直接相关的 AC、规则和设计锚点

## 禁止项

执行计划和 Task 不得出现以下内容：

- [ ] 没有 TBD / TODO / 占位符
- [ ] 没有"根据需要实现""酌情处理"等模糊指令
- [ ] 没有跨 Task 隐式依赖（依赖必须显式声明在前置依赖列）
- [ ] 没有要求 Agent 自行寻找未列出的上下文文件
- [ ] 没有无验证方式的 AC
- [ ] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每个 Task 自包含）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | napi `GetLoadedSystemNativeModules` 枚举已加载系统模块名 | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}` | AC-1, AC-8 | 无 | 接口可用且只返回系统模块名 | napi 单测 | Pending |
| TASK-2 | napi `PreloadSystemNativeModule`（拼接路径 + `dlopen_preload`） | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}` | AC-3, AC-4 | TASK-1 | 预加载走 dlopen_preload、不注册模块 | napi 单测 | Pending |
| TASK-3 | NativeModuleCollector：采集 + EWMA + 原子落盘 + 跨进程互斥 | `interfaces/kits/native/appkit/app_startup/native_module_collector.h`、`frameworks/native/appkit/app_startup/native_module_collector.cpp` | AC-1, AC-2, AC-4, AC-6, AC-9 | TASK-1 | 采集/预测/落盘/并发/降级闭环 | ability_runtime 单测 | Pending |
| TASK-4 | startup_manager 预测预加载任务 + 采集调度 + 参数开关 + 故障事件 | `frameworks/native/appkit/app_startup/startup_manager.{h,cpp}` | AC-3, AC-5, AC-7 | TASK-2, TASK-3 | 非主线程预加载、开关生效、失败续跑 | ability_runtime 单测 | Pending |
| TASK-5 | main_thread 采集窗口锚点挂接 | `frameworks/native/appkit/app/main_thread.cpp`、`interfaces/kits/native/appkit/app/main_thread.h` | AC-1 | TASK-4 | 窗口起止挂接生效 | ability_runtime 单测 | Pending |
| TASK-6 | 单测与 mock 扩展（采集/预测/落盘/降级/并发/开关/预加载失败） | `test/unittest/frameworks_kits_appkit_native_test/*`、`test/mock/frameworks_kits_appkit_native_test/*` | AC-1, AC-2, AC-3, AC-4, AC-5, AC-6, AC-7, AC-8, AC-9, AC-10 | TASK-1..TASK-5 | 用例全部通过 | ability_runtime 单测 | Pending |

## Task 详情

### TASK-1: napi 枚举已加载系统模块

| 字段 | 内容 |
|------|------|
| 任务目标 | `NativeModuleManager` 新增 `GetLoadedSystemNativeModules()`，返回已加载系统模块名集合 |
| AC 映射 | AC-1, AC-8 |
| 前置依赖 | design.md 代码事实基线（模块链表结构） |
| 非目标 | 不返回 isAppModule/路径；不改既有加载路径 |
| 完成判据 | 加锁快照下返回系统模块名；并发加载不崩溃 |
| 停止条件 | 需改动既有加载语义时停止并回修 spec/design |
| Design 状态/不变量 | INV-2 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-3 | 已加载模块链表 | NativeModuleManager | headNativeModule_ | 模块注册/加载 | 读：本接口；写：加载路径 | 进程退出 | 互斥加锁，返回一致快照 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `std::vector<std::string> NativeModuleManager::GetLoadedSystemNativeModules() const` | TASK-1 | 供 TASK-3 消费 | 仅系统模块名；空集表示无数据 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `foundation/arkui/napi/module_manager/native_module_manager.h:58-76` | 确认 NativeModule 字段（name/moduleLoaded/isAppModule） |
| `foundation/arkui/napi/module_manager/native_module_manager.cpp:1517-1601` | 确认已加载系统模块落点 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `foundation/arkui/napi/module_manager/native_module_manager.h` | 声明 `GetLoadedSystemNativeModules` |
| Modify | `foundation/arkui/napi/module_manager/native_module_manager.cpp` | 实现：加锁遍历链表，收集 isAppModule=false 且 moduleLoaded=true 的模块名 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `foundation/arkui/napi/module_manager/native_module_manager.cpp` 的既有加载路径 | 不得改变既有加载语义 |

**Spec References**

- AC / Rules: `AC-1`, `AC-8`, `R-1`, `R-10`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-3`, `INV-2`, `design.md#类图`, `design.md#代码事实基线`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must 保持 InnerAPI，不面向应用 |
| OH-ARCH-LAYERING | Must Not 越层访问框架内部结构 |

**Steps**

- [ ] 写失败测试或定义验证用例（枚举返回系统模块名）
- [ ] 运行测试/验证，确认当前失败或缺失
- [ ] 做最小实现（加锁遍历模块链表，过滤系统模块）
- [ ] 运行测试/验证，确认通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `foundation/arkui/napi/module_manager/native_module_manager.cpp:161 / GetInstance` | 单例与加锁约定 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 正向（返回系统模块名）、边界（无系统模块返回空）证据 |
| Scope respected | 实际 diff 限于本 Task Files |
| State lifecycle complete | STATE-3 读路径加锁，不变量 INV-2 覆盖 |
| Interface contract matched | 与 TASK-3 Consumes 签名一致 |
| Relevant verification | napi 单测直接覆盖本接口 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `foundation/arkui/napi/test/unittest` 中枚举用例 | 返回系统模块名集合，应用模块被排除 | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：枚举已加载系统模块名 |
| 允许修改 | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}` |
| 允许新建 | 无 |
| 只读参考 | `native_module_manager.h:58-76`、`native_module_manager.cpp:1517-1601` |
| Spec 引用 | `AC-1`, `AC-8`, `R-1`, `R-10`, `spec.md#用户故事`, `spec.md#规则定义` |
| Design 引用 | `ADR-3`, `INV-2`, `design.md#类图` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | napi 单测（枚举用例） / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1/AC-8 与枚举行为、边界 |
| Code Quality | 加锁、链表遍历、内存 |
| Verification | 枚举用例 PASS 证据 |

### TASK-2: napi 系统模块预加载

| 字段 | 内容 |
|------|------|
| 任务目标 | `NativeModuleManager` 新增 `PreloadSystemNativeModule(moduleName)`，内部拼接系统路径并调用 `dlopen_preload` |
| AC 映射 | AC-3, AC-4 |
| 前置依赖 | TASK-1 |
| 非目标 | 不注册模块、不改变应用真实 import 路径、mode 不写死 |
| 完成判据 | 返回 0/-1；预加载后该模块不出现在已加载系统模块枚举中 |
| 停止条件 | musl `dlopen_preload` mode/符号不可用时停止并上报 |
| Design 状态/不变量 | INV-3, STATE-4 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-4 | 预加载 pending 记录 | musl ldso | dlopen_preload 内部 | 首次 dlopen_preload | 应用真实 dlopen 消费 | 进程退出 | 预加载不注册模块（INV-3） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `int NativeModuleManager::PreloadSystemNativeModule(const char* moduleName)` | TASK-2 | 供 TASK-4 消费 | 0/-1；mode 与 musl 对齐后固化 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `foundation/arkui/napi/module_manager/native_module_manager.cpp:294-322` | 复用系统路径解析逻辑 |
| `third_party/musl/include/linux/dlfcn_ext.h:96-101` | `dlopen_preload` 签名与 mode 约束 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `foundation/arkui/napi/module_manager/native_module_manager.h` | 声明 `PreloadSystemNativeModule` |
| Modify | `foundation/arkui/napi/module_manager/native_module_manager.cpp` | 实现：拼接系统 SO 路径 + `dlopen_preload` |
| Modify | `foundation/arkui/napi/module_manager/BUILD.gn` | 链接 musl `dlopen_preload`（如需） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `third_party/musl` 任何文件 | 外部依赖，不改 |
| `LoadNativeModule` 既有实现 | 不改既有加载语义 |

**Spec References**

- AC / Rules: `AC-3`, `AC-4`, `R-5`, `R-9`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`, `spec.md#兼容性声明`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `INV-3`, `design.md#代码事实基线`, `design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-SUBSYSTEM | Must 经 NativeModuleManager 封装 musl 能力 |
| OH-ARCH-API-LEVEL | Must Not 改既有 API 签名 |

**Steps**

- [ ] 写失败测试（预加载接口不存在/未实现）
- [ ] 运行测试/验证，确认当前失败
- [ ] 做最小实现（拼接路径 + dlopen_preload）
- [ ] 运行测试/验证，确认通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `foundation/arkui/napi/module_manager/native_module_manager.cpp:294-322 / GetModuleFileName` | 不加载解析路径 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 正向（预加载成功）、异常（路径失效返回 -1）、边界（不注册模块）证据 |
| Scope respected | 实际 diff 限于本 Task Files |
| State lifecycle complete | STATE-4 不变量 INV-3 覆盖 |
| Interface contract matched | 与 TASK-4 Consumes 签名一致 |
| Relevant verification | napi 单测直接覆盖 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `foundation/arkui/napi/test/unittest` 中预加载用例 | 预加载成功返回 0；失败返回 -1；该模块不进枚举 | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：系统模块预加载 InnerAPI |
| 允许修改 | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}`、`module_manager/BUILD.gn` |
| 允许新建 | 无 |
| 只读参考 | `native_module_manager.cpp:294-322`、`dlfcn_ext.h:96-101` |
| Spec 引用 | `AC-3`, `AC-4`, `R-5`, `R-9`, `spec.md#兼容性声明` |
| Design 引用 | `ADR-1`, `ADR-2`, `INV-3`, `design.md#线程与并发模型` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | napi 单测（预加载用例） / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-3/AC-4 与防自反馈边界 |
| Code Quality | 路径拼接、错误处理 |
| Verification | 预加载用例 PASS 证据 |

### TASK-3: NativeModuleCollector 采集/预测/落盘/并发

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增 NativeModuleCollector：采集窗口结束枚举系统模块、EWMA 合并、原子落盘到模块级沙箱目录下的本特性子目录、跨进程互斥合并 |
| AC 映射 | AC-1, AC-2, AC-4, AC-6, AC-9 |
| 前置依赖 | TASK-1 |
| 非目标 | 不做预测算法调优；不做云端同步 |
| 完成判据 | 采集/预测/落盘/并发/降级闭环可用，文件含 version 字段 |
| 停止条件 | 需改 cacheDir 语义或跨进程方案时停止并回修 design |
| Design 状态/不变量 | INV-1, INV-2, INV-5, STATE-2, STATE-3 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-2 | 采集/预测文件 | NativeModuleCollector | `<appBase>/haps/<moduleName>/cache/native_module_collect/` | Init 后首次落盘 | 读：LoadPredictedModules；写：CollectAndPersist | 文件损坏按无历史；进程退出不清理 | 原子更名 + 跨进程 flock（INV-1/INV-5） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | `std::vector<std::string> NativeModuleManager::GetLoadedSystemNativeModules() const` | TASK-1 | 来自 TASK-1 | 仅系统模块名 |
| Produces | `std::vector<std::string> NativeModuleCollector::LoadPredictedModules()`、`void NativeModuleCollector::CollectAndPersist()`、`void NativeModuleCollector::Init(cacheDir, moduleName, config)`、`void NativeModuleCollector::SetEnabled(bool)` | TASK-3 | 供 TASK-4 消费 | 文件格式含 version |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `frameworks/native/appkit/ability_runtime/context/application_context.cpp:1085` | 模块级沙箱目录获取（haps/<moduleName>/cache） |
| `design.md#数据模型设计` | 文件结构与版本字段 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `interfaces/kits/native/appkit/app_startup/native_module_collector.h` | 类声明 |
| Create | `frameworks/native/appkit/app_startup/native_module_collector.cpp` | 实现：采集/EWMA/原子写/flock |
| Modify | `frameworks/native/appkit/BUILD.gn` | 加入源文件 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `startup_manager.cpp` | 本 Task 不接入调度（TASK-4 负责） |
| `interfaces/kits/native/appkit/app_startup/startup_utils.h` | 既有错误码，不改 |

**Spec References**

- AC / Rules: `AC-1`, `AC-2`, `AC-4`, `AC-6`, `AC-9`, `R-1`, `R-2`, `R-3`, `R-4`, `R-7`, `R-8`, `R-11`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`, `spec.md#架构约束`

**Design References**

- IDs / Anchors: `ADR-4`, `ADR-5`, `INV-1`, `INV-2`, `INV-5`, `design.md#状态归属与不变量`, `design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must 原子写盘、失败降级 |
| OH-ARCH-LAYERING | Must 仅经 napi InnerAPI 获取模块 |

**Steps**

- [ ] 写失败测试（采集/落盘/并发/降级）
- [ ] 运行测试/验证，确认当前失败
- [ ] 做最小实现（枚举→EWMA→原子写盘）
- [ ] 运行测试/验证，确认通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| no prior art found（原子写盘） | 新引入 tmp+rename+flock | 无既有同类实现，属本特性新增 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 采集/预测/落盘/并发/降级正/异/边界证据 |
| Scope respected | 实际 diff 限于本 Task Files |
| State lifecycle complete | STATE-2 不变量 INV-1/INV-5 覆盖 |
| Interface contract matched | Consumes 与 TASK-1 Produces 一致 |
| Relevant verification | collector 单测直接覆盖 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.*` | 采集/预测/落盘/并发/降级用例全通过 | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-3：采集器（采集+预测+落盘+并发） |
| 允许修改 | `interfaces/kits/native/appkit/app_startup/native_module_collector.h`、`frameworks/native/appkit/app_startup/native_module_collector.cpp`、`frameworks/native/appkit/BUILD.gn` |
| 允许新建 | 上述两个新文件 |
| 只读参考 | `application_context.cpp:1085`、`design.md#数据模型设计` |
| Spec 引用 | `AC-1`, `AC-2`, `AC-4`, `AC-6`, `AC-9`, `R-1..R-4, R-7, R-8, R-11`, `spec.md#架构约束` |
| Design 引用 | `ADR-4`, `ADR-5`, `INV-1`, `INV-2`, `INV-5`, `design.md#状态归属与不变量` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | collector 单测 / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1/2/4/6/9 闭环与边界 |
| Code Quality | 并发、资源、内存、错误处理 |
| Verification | 采集器用例 PASS 证据 |

### TASK-4: startup_manager 预测预加载与调度

| 字段 | 内容 |
|------|------|
| 任务目标 | startup_manager 新增预测预加载任务（读 predicted、非主线程逐模块预加载、失败记故障事件续跑）、采集调度、参数开关 |
| AC 映射 | AC-3, AC-5, AC-7 |
| 前置依赖 | TASK-2, TASK-3 |
| 非目标 | 不改既有静态 allowlist 预加载语义 |
| 完成判据 | 开关关闭/设备不支持时不生效；单模块失败续跑；不阻塞主线程 |
| 停止条件 | 需改既有静态预加载语义时停止并回修 spec |
| Design 状态/不变量 | INV-4, STATE-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1 | 采集调度状态 | StartupManager | nativeModuleCollectScheduled_ | 启动任务首次调度 | 启动任务 | 进程退出/调度失败 | 原子单次调度（INV-4） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | `NativeModuleCollector::Init/LoadPredictedModules/SetEnabled` | TASK-3 | 来自 TASK-3 | 文件格式含 version |
| Consumes | `NativeModuleManager::PreloadSystemNativeModule(moduleName)` | TASK-2 | 来自 TASK-2 | 0/-1；失败记故障事件 |
| Produces | `int32_t StartupManager::RunPredictPreloadSystemSoTask()`、`void StartupManager::ScheduleNativeModuleCollect()` | TASK-4 | 供 TASK-5 挂接 | 不阻塞主线程 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `frameworks/native/appkit/app_startup/startup_manager.cpp:120,131-143` | 复用 preload 线程 |
| `interfaces/kits/native/appkit/app_startup/startup_utils.h:36` | 复用 ERR_LOAD_NATIVE_MODULE |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interfaces/kits/native/appkit/app_startup/startup_manager.h` | 新增任务/调度/开关声明 |
| Modify | `frameworks/native/appkit/app_startup/startup_manager.cpp` | 实现预测预加载任务、采集调度、参数开关、故障事件 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `preload_system_so_startup_task.cpp` | 既有静态预加载链路，不改语义 |
| `hisysevent.yaml` | 复用既有事件，不新增定义 |

**Spec References**

- AC / Rules: `AC-3`, `AC-5`, `AC-7`, `R-5`, `R-6`, `R-9`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`, `spec.md#多设备适配声明`

**Design References**

- IDs / Anchors: `ADR-6`, `ADR-7`, `INV-4`, `design.md#线程与并发模型`, `design.md#状态归属与不变量`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must 单模块失败记故障事件并续跑 |
| OH-ARCH-LAYERING | Must 复用 preload 线程，不阻塞主线程 |

**Steps**

- [ ] 写失败测试（开关关闭/单模块失败续跑/非主线程）
- [ ] 运行测试/验证，确认当前失败
- [ ] 做最小实现（读预测 + 循环预加载 + 调度）
- [ ] 运行测试/验证，确认通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `frameworks/native/appkit/app_startup/startup_manager.cpp:757-793 / AddAppAutoPreloadSoTask` | 启动任务注册约定 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 开关、非主线程、单模块失败续跑证据 |
| Scope respected | 实际 diff 限于本 Task Files |
| State lifecycle complete | STATE-1 不变量 INV-4 覆盖 |
| Interface contract matched | Consumes 与 TASK-2/3 Produces 一致 |
| Relevant verification | startup 单测直接覆盖 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `test/unittest/frameworks_kits_appkit_native_test::PredictPreloadTaskTest.*` | 开关关闭不生效；单模块失败续跑；非主线程执行 | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-4：预测预加载任务与采集调度 |
| 允许修改 | `interfaces/kits/native/appkit/app_startup/startup_manager.h`、`frameworks/native/appkit/app_startup/startup_manager.cpp` |
| 允许新建 | 无 |
| 只读参考 | `startup_manager.cpp:120,131-143`、`startup_utils.h:36` |
| Spec 引用 | `AC-3`, `AC-5`, `AC-7`, `R-5`, `R-6`, `R-9`, `spec.md#多设备适配声明` |
| Design 引用 | `ADR-6`, `ADR-7`, `INV-4`, `design.md#线程与并发模型` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | startup 单测 / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-3/5/7 闭环与边界 |
| Code Quality | 线程安全、错误处理 |
| Verification | startup 用例 PASS 证据 |

### TASK-5: main_thread 采集窗口锚点挂接

| 字段 | 内容 |
|------|------|
| 任务目标 | 在启动主流程挂接采集窗口起止（开始 LoadExtStartupTasks，结束 HandleLaunchApplication 完成后 5s） |
| AC 映射 | AC-1 |
| 前置依赖 | TASK-4 |
| 非目标 | 不改变主流程既有调度语义 |
| 完成判据 | 窗口起止按设计生效，不引入启动回归 |
| 停止条件 | 需要改动主流程既有行为时停止并回修 design |
| Design 状态/不变量 | INV-4, STATE-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1 | 采集调度状态 | StartupManager | nativeModuleCollectScheduled_ | 挂接点触发 | 启动任务 | 进程退出 | 原子单次调度（INV-4） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | `StartupManager::ScheduleNativeModuleCollect()` | TASK-4 | 来自 TASK-4 | 幂等，单次生效 |
| Produces | 无对外接口 | — | — | — |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `frameworks/native/appkit/app/main_thread.cpp:1659,1862,2174` | 采集窗口锚点 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/main_thread.cpp` | 挂接采集窗口起止 |
| Modify | `interfaces/kits/native/appkit/app/main_thread.h` | 挂接声明（如需） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| 主流程其他函数 | 不改变既有调度语义 |

**Spec References**

- AC / Rules: `AC-1`, `R-3`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-7`, `INV-4`, `design.md#代码事实基线`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must Not 改变主流程既有调度语义 |

**Steps**

- [ ] 定义采集窗口起止挂接点
- [ ] 运行验证，确认未挂接时不触发
- [ ] 做最小实现（挂接起止）
- [ ] 运行测试/验证，确认通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `frameworks/native/appkit/app/main_thread.cpp:1862 / LoadExtStartupTasks` | 启动任务挂接点 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 窗口起止按设计触发的证据 |
| Scope respected | 实际 diff 限于本 Task Files |
| State lifecycle complete | STATE-1 不变量 INV-4 覆盖 |
| Interface contract matched | Consumes 与 TASK-4 Produces 一致 |
| Relevant verification | 挂接用例直接覆盖 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | ability_runtime 单测（采集窗口挂接） | 窗口起止按设计触发 | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-5：采集窗口锚点挂接 |
| 允许修改 | `frameworks/native/appkit/app/main_thread.cpp`、`interfaces/kits/native/appkit/app/main_thread.h` |
| 允许新建 | 无 |
| 只读参考 | `main_thread.cpp:1659,1862,2174` |
| Spec 引用 | `AC-1`, `R-3`, `spec.md#规则定义` |
| Design 引用 | `ADR-7`, `INV-4`, `design.md#代码事实基线` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | 采集窗口挂接用例 / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1 窗口挂接边界 |
| Code Quality | 主流程侵入最小 |
| Verification | 挂接用例 PASS 证据 |

### TASK-6: 单测与 mock 扩展

| 字段 | 内容 |
|------|------|
| 任务目标 | 扩展 mock 并为采集/预测/落盘/降级/并发/开关/预加载失败补齐单元测试 |
| AC 映射 | AC-1, AC-2, AC-3, AC-4, AC-5, AC-6, AC-7, AC-8, AC-9, AC-10 |
| 前置依赖 | TASK-1..TASK-5 |
| 非目标 | 不做系统级 XTS、不做性能压测 |
| 完成判据 | 本特性相关单测全部通过 |
| 停止条件 | 发现实现与 spec/design 冲突时停止并回修 |
| Design 状态/不变量 | INV-1..INV-5（回归） |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| INV-1..INV-5 | 全不变量回归 | NativeModuleCollector/StartupManager | 单测用例 | 测试初始化 | 用例执行 | 用例结束 | 覆盖 INV-1..INV-5 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | TASK-1..TASK-5 的全部接口 | TASK-1..TASK-5 | 前置 Task | 与各自 Produces 签名一致 |
| Produces | 单测用例集 + mock 扩展 | TASK-6 | 供回归使用 | 用例稳定、可重复 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `test/mock/frameworks_kits_appkit_native_test/include/mock_native_module_manager.cpp:28` | mock 注入约定 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `test/unittest/frameworks_kits_appkit_native_test/native_module_collector_test.cpp` | 采集器/预加载用例 |
| Modify | `test/unittest/frameworks_kits_appkit_native_test/BUILD.gn` | 加入测试源 |
| Modify | `test/mock/frameworks_kits_appkit_native_test/include/mock_native_module_manager.{h,cpp}` | mock 扩展 |
| Test | `foundation/arkui/napi/test/unittest/` | napi 接口用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| 生产代码 | 本 Task 仅测试，不改实现 |

**Spec References**

- AC / Rules: `AC-1`, `AC-2`, `AC-3`, `AC-4`, `AC-5`, `AC-6`, `AC-7`, `AC-8`, `AC-9`, `AC-10`, `R-12`
- Anchors: `spec.md#验证映射`, `spec.md#用户故事`

**Design References**

- IDs / Anchors: `INV-1`, `INV-2`, `INV-3`, `INV-4`, `INV-5`, `design.md#测试性设计`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must 覆盖异常路径与故障事件 |

**Steps**

- [ ] 扩展 mock（注入已加载系统模块、预加载失败）
- [ ] 编写各 AC 对应的用例（含 Red 条件确认实现前失败）
- [ ] 运行用例，确认全部通过
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result 和计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `test/mock/frameworks_kits_appkit_native_test/include/mock_native_module_manager.cpp:28 / MockSetNativeModule` | mock 注入约定 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 各 AC 正/异/边界用例 PASS 证据 |
| Scope respected | 仅改测试与 mock |
| State lifecycle complete | INV-1..INV-5 均有用例覆盖 |
| Interface contract matched | mock 与真实接口签名一致 |
| Relevant verification | 单测命令直接覆盖本特性 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts frameworks_kits_appkit_native_test` 或对应模块测试 | 本特性用例全部 PASS | 实现后回填（PASS + 输出摘要） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-6：单测与 mock 扩展 |
| 允许修改 | `test/unittest/frameworks_kits_appkit_native_test/*`、`test/mock/frameworks_kits_appkit_native_test/*`、`foundation/arkui/napi/test/unittest/` |
| 允许新建 | `native_module_collector_test.cpp` |
| 只读参考 | `mock_native_module_manager.cpp:28` |
| Spec 引用 | `AC-1..AC-10`, `R-12`, `spec.md#验证映射` |
| Design 引用 | `INV-1..INV-5`, `design.md#测试性设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | ability_runtime 单测 / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC 覆盖完整性 |
| Code Quality | 用例质量、mock 正确性 |
| Verification | 全部用例 PASS 证据 |

## 代码范围映射

> Plan 审批时填写计划范围；实现后如实回填实际范围和 Commit。

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}::GetLoadedSystemNativeModules` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-2 | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}::PreloadSystemNativeModule` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-3 | `interfaces/kits/native/appkit/app_startup/native_module_collector.h`、`frameworks/native/appkit/app_startup/native_module_collector.cpp` | 待回填 | Create | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-4 | `frameworks/native/appkit/app_startup/startup_manager.{h,cpp}::RunPredictPreloadSystemSoTask/ScheduleNativeModuleCollect` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-5 | `frameworks/native/appkit/app/main_thread.{h,cpp}::采集窗口挂接` | 待回填 | Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |
| TASK-6 | `test/unittest/frameworks_kits_appkit_native_test/native_module_collector_test.cpp` 等 | 待回填 | Create/Modify | 待回填 | 待回填 | 待回填 | 待回填 | 无 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-2 | dependency | `dlopen_preload` 的 mode 待与 musl 对齐 | Blocked | Owner（yangzk）与 musl 团队对齐 |
| TASK-4 | design | 系统参数开关最终名/默认值待产品配置对齐 | Blocked | Owner（yangzk） |
| TASK-5 | design | 采集窗口结束锚点误差（5000ms 常量近似） | Resolved | design ADR-7 |
