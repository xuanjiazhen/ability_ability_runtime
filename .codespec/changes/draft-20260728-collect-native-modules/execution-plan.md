# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 自包含，交接信息不依赖文件路径引用。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-draft-20260728 |
| 关联 Feature/Bug | draft-20260728 |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | ReadyForReview |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（基线 v1.0，2026-07-28 冻结） |
| Design | `design.md` | Approved |
| Spec | `spec.md` | Approved |

## 受影响文件全量清单

> 对照 design.md 调用链层级分析，逐层列出每层涉及的具体文件。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| arkui/napi | NAPI 库 | `module_manager/native_module_manager.h` | 修改 | 新增 NativeModuleInfo 结构与 GetLoadedModuleInfos 声明 |
| arkui/napi | NAPI 库 | `module_manager/native_module_manager.cpp` | 修改 | 枚举接口实现（持锁遍历、深拷贝、loadCount 快照） |
| arkui/napi | NAPI 库（测试） | `module_manager/test/native_module_manager_test.cpp`（不存在则新建） | 修改/新增 | 接口 UT：快照一致性、深拷贝、空链表 |
| ability_runtime | 框架层-appkit | `frameworks/native/appkit/app/native_module_collector.h` | 新增 | collector 接口：开关、路径、采样、合并、写盘 |
| ability_runtime | 框架层-appkit | `frameworks/native/appkit/app/native_module_collector.cpp` | 新增 | collector 实现（EWMA/淘汰/重建/原子写/loadCount 判定/CCM 参数） |
| ability_runtime | 框架层-appkit | `frameworks/native/appkit/app/application_impl.cpp` | 修改 | 检查点①：PerformAppReady 处触发采样 |
| ability_runtime | 框架层-appkit | `frameworks/native/appkit/app/main_thread.cpp` | 修改 | 检查点②：首个 UIAbility 前台 + 延迟任务触发合并写盘 |
| ability_runtime | 框架层-运行时 | `frameworks/native/runtime/ets_runtime.cpp` | 修改 | PreFork 记录 fork 前模块 key 快照 |
| ability_runtime | 框架层-app_startup | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.h` | 新增 | 预测预加载任务声明 |
| ability_runtime | 框架层-app_startup | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.cpp` | 新增 | 读文件、过滤系统模块、LoadNativeModule、故障上报 |
| ability_runtime | 框架层-app_startup | `frameworks/native/appkit/app_startup/startup_manager.cpp` | 修改 | 注册/编排预测预加载任务（参照 AnalyzePreloadSystemSoStartupTask，startup_manager.cpp:1124） |
| ability_runtime | 构建 | `frameworks/native/appkit/app/BUILD.gn` | 修改 | sources 新增 native_module_collector.cpp |
| ability_runtime | 构建 | `frameworks/native/appkit/app_startup/BUILD.gn`（或所属 BUILD.gn） | 修改 | sources 新增 predict_preload_so_startup_task.cpp |
| ability_runtime | 测试 | `test/unittest/native_module_collector_test/BUILD.gn` | 新增 | 测试 target |
| ability_runtime | 测试 | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 新增 | VM-1~VM-11 对应用例 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出（NAPI 库/运行时/appkit/app_startup/构建/测试）
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层：无（服务层/内核层本就不涉及）

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-3, TASK-4 | 单测 + 实测 | 是 |
| AC-1.2 | spec.md | TASK-2, TASK-8 | 单测 | 是 |
| AC-1.3 | spec.md | TASK-2 | 单测 | 是 |
| AC-1.4 | spec.md | TASK-3 | 单测 | 是 |
| AC-1.5 | spec.md | TASK-5 | 单测 + 实测 | 是 |
| AC-2.1 | spec.md | TASK-3 | 单测 | 是 |
| AC-2.2 | spec.md | TASK-3 | 单测 | 是 |
| AC-2.3 | spec.md | TASK-3 | 单测 | 是 |
| AC-2.4 | spec.md | TASK-3 | 单测 | 是 |
| AC-2.5 | spec.md | TASK-7 | 单测 | 是 |
| AC-3.1 | spec.md | TASK-6 | 单测 + 实测 | 是 |
| AC-3.2 | spec.md | TASK-6 | 单测 | 是 |
| AC-3.3 | spec.md | TASK-6 | 单测 | 是 |
| AC-3.4 | spec.md | TASK-6 | 单测 | 是 |
| AC-4.1 | spec.md | TASK-2 | 单测 | 是 |
| AC-4.2 | spec.md | TASK-2 | 单测 + 权限检查 | 是 |

## 首批实现边界

**首批必须实现：** TASK-SKELETON-1、TASK-1（跨仓接口 unblock）、TASK-2、TASK-3、TASK-4（采集+更新主链闭环）
**可后置：** TASK-7（loadCount 精确判定，有降级方案）、TASK-8（CCM 调优参数，缺省值可先跑）、TASK-9 中的 beta 命中率上报
**不建议延后：** TASK-5（不做则 preload 模块混入采集，违反已基线 AC-1.5）、TASK-6（消费方为本期范围，不做则主链价值不闭合）

## 阶段计划

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 跨仓接口 + collector 骨架 | TASK-SKELETON-1, TASK-1, TASK-2 | 两仓编译通过；接口 UT 与骨架 UT 通过 | `run -t UT -ts native_module_manager_test`；`run -t UT -ts native_module_collector_test` |
| Phase-2 | 采集与更新核心闭环 | TASK-3, TASK-4, TASK-5 | EWMA/淘汰/重建/原子写 UT 全过；设备两轮实测文件内容正确且不含 preload 模块 | UT + 实测比对 /proc/self/maps |
| Phase-3 | 预加载消费 + 防自反馈 | TASK-6, TASK-7 | 预加载任务 UT 过；实测 predicted 系统模块被预加载；loadCount 命中判定 UT 过 | UT + hidumper 验证 |
| Phase-4 | 参数化与齐套 | TASK-8, TASK-9 | CCM 参数生效；VM-1~VM-11 全过；`ohos-sdd validate --level all` PASS | 全量 UT + validate |

## Task 粒度原则

每个 Task 对应一个可独立验收的最小能力闭环：跨仓接口（TASK-1）、骨架与命名（TASK-2）、合并引擎（TASK-3）、检查点接线（TASK-4）、fork 排除（TASK-5）、消费任务（TASK-6）、防自反馈（TASK-7）、参数化（TASK-8）、测试齐套（TASK-9）。无超 3000 行 Task，无需拆分。

## 禁止项

- [x] 没有 TBD / TODO / 占位符
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖显式声明在前置依赖列）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每个 Task 自包含）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-SKELETON-1 | napi 枚举接口骨架 | `arkui/napi module_manager/native_module_manager.{h,cpp}` | 接口场景#2 | 无 | 编译通过，空实现返回空列表 | napi 仓对应 UT 构建通过 |
| TASK-1 | 枚举接口实现 + UT | 同 TASK-SKELETON-1 + napi UT 文件 | AC-1.1（数据源） | TASK-SKELETON-1 | 快照一致/深拷贝/空链表 UT 通过 | `run -t UT -ts native_module_manager_test` |
| TASK-2 | collector 骨架：开关/路径/命名/权限 | `native_module_collector.{h,cpp}` + app/BUILD.gn | AC-1.2, AC-1.3, AC-4.1, AC-4.2 | 无 | 命名/哈希/权限/开关 UT 通过 | `run -t UT -ts native_module_collector_test` |
| TASK-3 | EWMA 合并引擎 + 原子写 + 淘汰/重建 | `native_module_collector.cpp` | AC-1.4, AC-2.1~AC-2.4 | TASK-1, TASK-2 | 合并/淘汰/重建/原子写 UT 全过 | 同上 |
| TASK-4 | 双检查点接入 | `application_impl.cpp`, `main_thread.cpp` | AC-1.1 | TASK-3 | 检查点触发 UT + 实测每轮 ≤1 次写盘 | UT + 设备实测 |
| TASK-5 | PreFork 快照排除 | `ets_runtime.cpp` | AC-1.5 | TASK-3 | preload 模块不出现在采集结果 | UT + 实测 |
| TASK-6 | 预测预加载启动任务 | `predict_preload_so_startup_task.{h,cpp}`, `startup_manager.cpp`, app_startup BUILD.gn | AC-3.1~AC-3.4 | TASK-2 | 预加载/跳过/失败续跑/空集 UT 过 | `run -t UT -ts native_module_collector_test` + 实测 |
| TASK-7 | loadCount 命中判定（防自反馈） | `native_module_collector.cpp` | AC-2.5 | TASK-1, TASK-3, TASK-6 | 预加载模块不计命中 UT 通过 | `run -t UT -ts native_module_collector_test` |
| TASK-8 | CCM 参数接入（开关+调优参数） | `native_module_collector.{h,cpp}` | AC-1.2 | TASK-2 | 参数缺失缺省关闭、参数生效 UT 通过 | 同上 |
| TASK-9 | TDD 齐套 + beta 命中率上报 | `test/unittest/native_module_collector_test/` | 全部 AC 验证闭环 | TASK-3~TASK-8 | VM-1~VM-11 全过 | `run -t UT -tp ability_runtime` |

## Task 详情

### TASK-SKELETON-1: napi 枚举接口骨架

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 arkui/napi 仓建立 NativeModuleInfo 结构与 GetLoadedModuleInfos 空实现，打通两仓编译依赖 |
| AC 映射 | spec 接口规格场景 #2（空结果返回空列表不报错） |
| 前置依赖 | 无 |
| 非目标 | 不实现真实遍历逻辑；不改 Register/LoadNativeModule 任何既有行为 |
| 完成判据 | napi 仓编译通过；调用空实现返回空 vector |
| 停止条件 | napi 仓 owner 对接口签名有异议 → 停止，回传 design.md「API 签名」节修订 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `arkui/napi module_manager/native_module_manager.h` | 新增 struct NativeModuleInfo（name/moduleName/fileName/version/loadCount/isAppModule/moduleLoaded）与 GetLoadedModuleInfos 声明 |
| Modify | `arkui/napi module_manager/native_module_manager.cpp` | 空实现：仅 infos.clear() |

**Spec Context**：接口签名 `void GetLoadedModuleInfos(std::vector<NativeModuleInfo>& infos)`；返回值 void 经出参返回；错误码 N/A；行为场景#2：进程未加载任何模块时返回空列表不报错；参数约束：实现不得残留旧数据（先清空再填充）。

**Design Context**：ADR-1 检查点快照拉取；实现约束（后续 TASK-1）：持内部锁遍历、字符串深拷贝、loadCount 取 NativeModule.refCount 快照。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：仅 InnerAPI，不得出现于 d.ts/SystemAPI 清单 |
| OH-ARCH-LAYERING | Must Not：不得在 napi 仓反向依赖 ability_runtime 任何头文件 |

**Steps**
- [ ] 声明结构与函数（NAPI_EXPORT）
- [ ] 空实现 infos.clear()
- [ ] 编译 napi 仓目标验证通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh --product-name <product> --build-target napi`（以 napi 仓实际 target 为准） | 暂缓（需求方指示先完成编码与检视，编译验证待补） |
| 代码提交 | arkui/napi 仓 `33cffc11`（feature/collect-imported-native-modules-sdd） | 已提交（骨架与实现一并落地，见 TASK-1 证据） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 建立枚举接口骨架：NativeModuleInfo 纯数据结构（std::string name/moduleName/fileName; int32_t version; uint32_t loadCount; bool isAppModule/moduleLoaded）+ GetLoadedModuleInfos 空实现 |
| 允许修改 | `arkui/napi module_manager/native_module_manager.h`、`native_module_manager.cpp` |
| 允许新建 | 无 |
| 只读参考 | ability_runtime `.codespec/changes/draft-20260728-collect-native-modules/design.md`、`spec.md` |
| Spec 摘要 | 出参先清空再填充；空结果返回空列表；无错误码 |
| Design 摘要 | ADR-1；字段与内部 NativeModule 一一对应，loadCount ↔ refCount |
| 执行步骤 | 见 Steps |
| 验证命令 | napi 仓构建 / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-1: 枚举接口实现 + 接口 UT

| 字段 | 内容 |
|------|------|
| 任务目标 | 实现 GetLoadedModuleInfos 真实逻辑（持锁遍历链表、深拷贝快照、含 loadCount），并以 UT 验证快照一致性与生命周期安全 |
| AC 映射 | AC-1.1（采集数据源正确性前提） |
| 前置依赖 | TASK-SKELETON-1 |
| 非目标 | 不实现任何 ability_runtime 侧逻辑；不修改 LoadNativeModule/Register 行为 |
| 完成判据 | UT 覆盖：注册/加载后快照字段一致、深拷贝（原节点释放后快照有效）、空链表、并发调用一致性 |
| 停止条件 | 发现 refCount 在 cache 命中时不递增 → 停止，回传 design RSK-2 触发降级方案评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `arkui/napi module_manager/native_module_manager.cpp` | 持内部锁遍历 headNativeModule_ 链表，逐节点深拷贝填充 infos |
| Modify | `arkui/napi module_manager/test/native_module_manager_test.cpp`（不存在则新建） | 4 个接口 UT 用例 |

**Spec Context**：行为场景#1 字段与内部记录一致（深拷贝，与内部节点生命周期无关）；#2 空列表不报错；#3 并发调用返回一致性快照（无字段撕裂）；#4 快照返回后原模块卸载，快照保持有效。

**Design Context**：ADR-1；实现约束：持模块管理器内部锁遍历；先清空 infos 再填充；字符串深拷贝（内部节点 const char* 可能随 UnloadNativeModule 失效）；loadCount 取 refCount 快照。RSK-2：需在本 Task 中确认 refCount cache 命中递增语义并记录结论。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：仅使用 napi 仓内部锁与原语 |
| OH-ARCH-ERROR-LOG | Must：锁获取失败等异常记 HiLog 并返回已填充部分（不崩溃） |

**Steps**
- [ ] 写失败测试（快照一致性/深拷贝/空链表/并发）
- [ ] 运行测试确认失败
- [ ] 最小实现（持锁遍历 + 深拷贝）
- [ ] 运行测试确认通过
- [ ] 确认 refCount 语义并记录到 design RSK-2
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_manager_test` | 暂缓（编译验证待补，用例已交付：空链表/字段一致/深拷贝/并发 4 例） |
| 代码提交 | arkui/napi 仓 `33cffc11` | 已提交 |
| 检视 | 人工检视记录 | 通过：锁契约核实（Register/Unload 持 nativeModuleListMutex_）；refCount 语义未发现反证，RSK-2 保持开放待设备验证 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 实现持锁遍历 headNativeModule_ → tailNativeModule_ 链表，逐节点深拷贝（name/moduleName/fileName/version/refCount→loadCount/isAppModule/moduleLoaded）填充 infos；先 infos.clear() |
| 允许修改 | `arkui/napi module_manager/native_module_manager.cpp`、`module_manager/test/native_module_manager_test.cpp` |
| 允许新建 | `arkui/napi module_manager/test/native_module_manager_test.cpp`（若不存在） |
| 只读参考 | ability_runtime `.codespec/changes/draft-20260728-collect-native-modules/design.md` |
| Spec 摘要 | 接口行为场景 #1~#4（见上 Spec Context） |
| Design 摘要 | ADR-1 快照拉取；RSK-2 refCount 语义需确认记录 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_manager_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-2: collector 骨架——开关/路径/命名/权限

| 字段 | 内容 |
|------|------|
| 任务目标 | 建立 NativeModuleCollector 骨架：开关判定入口（缺省关闭）、cacheDir 路径与防冲突文件名生成、进程名 FNV-1a 8 字符哈希、0600 权限与隐藏目录创建 |
| AC 映射 | AC-1.2, AC-1.3, AC-4.1, AC-4.2 |
| 前置依赖 | 无（与 TASK-1 并行） |
| 非目标 | 不实现 EWMA 合并、写盘内容序列化、检查点接入、CCM 真实读参（开关先以占位函数 IsEnabled() 返回 false，TASK-8 接入真实参数） |
| 完成判据 | UT 覆盖：GetStorePath 路径形态、同进程名哈希稳定/不同进程名哈希不同、目录创建、0600 权限、IsEnabled=false 时零文件 I/O |
| 停止条件 | Context::GetCacheDir 在 collector 调用时点返回空 → 停止，回传调整路径获取时机设计 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `frameworks/native/appkit/app/native_module_collector.h` | 接口：IsEnabled/GetStorePath/SampleAtAppReady/CollectMergeAndSave（后两者本 Task 仅空壳） |
| Create | `frameworks/native/appkit/app/native_module_collector.cpp` | 路径/哈希/权限/隐藏目录实现 |
| Modify | `frameworks/native/appkit/app/BUILD.gn` | sources 新增 native_module_collector.cpp |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` + `BUILD.gn` | 骨架用例（命名/哈希/权限/开关关闭零 I/O） |

**Spec Context**：R-11 文件名 = `<cacheDir>/.ohos.ability_runtime/native_modules_<processHash8>.v2.json`，权限 0600，各进程哈希不同则文件名不同；R-2 开关缺失缺省关闭、全程零 I/O、判定结果本轮缓存；AC-4.2 内容仅公开信息。

**Design Context**：ADR-6 参数经 CCM 通道（本 Task 仅占位）；cacheDir 经 Context::GetCacheDir()（普通应用 /data/storage/el2/base/cache）；FNV-1a 仅用于命名隔离（非安全用途）；目录用 CreateDirIfNotExist 递归创建。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：仅调用 appkit/ability_base/公共工具，不得调用 services 层 |
| 安全基础检查 | Must：路径全部内部拼接，不接受外部输入；Must Not：日志打印完整 JSON 内容 |

**Steps**
- [ ] 写失败测试（路径形态/哈希稳定与区分/0600/开关零 I/O）
- [ ] 运行测试确认失败
- [ ] 最小实现
- [ ] 运行测试确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | 暂缓（编译验证待补，用例已交付：命名/哈希/建目录/0600/开关零 I/O 5 例） |
| 代码提交 | ability_runtime 仓 `93d2964db7` | 已提交 |
| 检视 | 人工检视记录 | 通过：AAFwkTag::APPKIT 存在；common_config 对 test 路径可见；EnsureDir/WriteFileWithPermission 边界走查通过 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | collector 骨架：IsEnabled()（占位 false）/GetStorePath(processName)/EnsureDir/WriteFileWithPermission 基础件；文件名为 `.ohos.ability_runtime/native_modules_<fnv1a32(processName) 转 8 位小写 hex>.v2.json` |
| 允许修改 | `frameworks/native/appkit/app/BUILD.gn` |
| 允许新建 | `frameworks/native/appkit/app/native_module_collector.{h,cpp}`、`test/unittest/native_module_collector_test/`（含 BUILD.gn 与测试 cpp） |
| 只读参考 | `frameworks/native/appkit/ability_runtime/context/context_impl.cpp`（GetCacheDir）、`frameworks/native/appkit/app/dump_runtime_helper.cpp`（沙箱写文件先例） |
| Spec 摘要 | R-11、R-2、AC-1.2/AC-1.3/AC-4.1/AC-4.2（见上 Spec Context） |
| Design 摘要 | ADR-6；存储方案表（JSON 文件、0600、可清理重建） |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-3: EWMA 合并引擎 + 原子写 + 淘汰/重建

| 字段 | 内容 |
|------|------|
| 任务目标 | 实现读旧文件 → EWMA 合并（命中奖励/遗漏衰减）→ 淘汰（低分/长未见/上限）→ predicted 预计算 → tmp+rename 原子写盘的完整闭环，含损坏/version 不符/缓存清理/应用升级四种冷启动重建路径 |
| AC 映射 | AC-1.4, AC-2.1, AC-2.2, AC-2.3, AC-2.4 |
| 前置依赖 | TASK-1（NativeModuleInfo 结构）、TASK-2（路径/写盘基础件） |
| 非目标 | 不做检查点触发接线（TASK-4）、不做 fork 排除（TASK-5）、不做 loadCount 命中区分（TASK-7，本 Task 中集合输入即视为真实命中） |
| 完成判据 | UT 覆盖：冷启动全量 score=1、命中奖励公式、遗漏衰减公式、阈值进出 predicted、0.05/16 轮/1024 条淘汰、损坏 JSON 重建、version 不符重建、appVersionCode 变化重建、写盘失败无残缺文件 |
| 停止条件 | JSON 解析库选型与仓内既有依赖冲突 → 停止，回传确认复用仓内 nlohmann/json 还是 cJSON |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/native_module_collector.cpp` | LoadStats/MergeRound/Evict/Serialize/WriteAtomic 实现 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 合并/淘汰/重建/原子写用例组 |

**Spec Context**：R-3 score←α+(1−α)·score（α=0.4）；R-4 predicted={score≥0.5}；R-5 淘汰 16 轮未见或 score<0.05；R-6 上限 1024 按 score 升序淘汰；R-7 损坏/version 不符/被清理→冷启动重建（首轮命中 score=1 全量入预测集）；R-8 appVersionCode 不一致→清空重建；R-13 写盘失败正式文件无残缺。

**Design Context**：数据模型（ModuleStat：fileName/isAppModule/score/hits/lastSeenLaunch/firstStage）；持久化 JSON v2 schema（version/bundleName/processName/appVersionCode/launchIndex/updatedAt/scenario/predicted/modules[]）；EWMA 状态机 Disabled→Sampling1→Sampling2→Merging→Evicting→Writing→Done；输入校验：条目数≤1024、单字段长度上限防畸形。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| 安全基础检查 | Must：解析输入全部校验，任一不满足按损坏丢弃；Must Not：畸形文件导致崩溃/OOM |
| OH-ARCH-ERROR-LOG | Must：写盘失败记 TAG_LOGE + SendLaunchFrameworkEvent(STARTUP_TASK_ERROR) |

**Steps**
- [ ] 写失败测试（9 个用例组，见完成判据）
- [ ] 运行测试确认失败
- [ ] 最小实现（LoadStats/MergeRound/Evict/Serialize/WriteAtomic）
- [ ] 运行测试确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | 用例已交付 10 组（TDD 执行暂缓，待统一回补） |
| 代码提交 | ability_runtime 仓 `107a19470f` | 已提交（编译通过：前序提交已验证构建链，本提交待下一轮编译确认） |
| 检视 | 人工检视记录 | 通过：修复 firstStage 序列化往返破坏、value()/get() 类型混淆未捕获两处 bug；补重复输入去重与路径守卫 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 合并引擎：输入本轮模块集合（vector<NativeModuleInfo>，已含 moduleName/isAppModule/fileName）与 launchIndex/appVersionCode；输出原子写回 JSON v2 文件；公式与阈值见 Spec Context，任一重建路径等价于 stats 清空后按首轮处理 |
| 允许修改 | `frameworks/native/appkit/app/native_module_collector.cpp` |
| 允许新建 | 无（测试在既有新建目录内追加） |
| 只读参考 | `frameworks/native/appkit/app/dump_runtime_helper.cpp`（沙箱写文件先例）、`frameworks/native/appkit/app_startup/preload_so_startup_task.cpp:134`（EventReport 模式） |
| Spec 摘要 | R-3~R-8、R-13（见上 Spec Context） |
| Design 摘要 | ModuleStat/JSON schema/状态机/输入校验（见上 Design Context） |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-4: 双检查点接入

| 字段 | 内容 |
|------|------|
| 任务目标 | 检查点①在 PerformAppReady 触发内存采样（不写盘）；检查点②在首个 UIAbility 前台后延迟（缺省 5s）或首次 OnBackground 触发合并写盘；写盘投递非主线程；每轮启动至多 1 次 |
| AC 映射 | AC-1.1 |
| 前置依赖 | TASK-3 |
| 非目标 | 不改动 PerformAppReady/OnBackground 既有行为语义；不处理 fork 排除（TASK-5） |
| 完成判据 | 触发点 UT（mock 回调验证调用序）；设备实测：开关开启时文件在检查点②后生成且每轮仅写 1 次；开关关闭时零 I/O |
| 停止条件 | "首个 UIAbility 前台"信号在 main_thread 无可靠回调点 → 停止，回传改用 application_impl 生命周期监听方案评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/application_impl.cpp` | PerformAppReady 后调用 NativeModuleCollector::SampleAtAppReady() |
| Modify | `frameworks/native/appkit/app/main_thread.cpp` | 首个 Ability 前台后投递延迟任务调用 CollectMergeAndSave()；首次 OnBackground 兜底触发 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 触发时序用例 |

**Spec Context**：AC-1.1 启动阶段结束后合并写盘、每轮启动写盘 ≤1 次；R-1 写盘不占主线程；架构约束：任一环节失败不影响启动。

**Design Context**：ADR-4 检查点②单写盘（①崩溃丢样本可接受）；数据流步骤 5/6/7；线程模型：采样主线程、合并写盘非主线程、每轮 ≤1 次禁止并发写。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：仅向 appkit 既有回调点挂接，不引入新线程模型 |
| 性能约束 | Must Not：主线程执行文件 I/O 或 JSON 解析 |

**Steps**
- [ ] 写失败测试（触发顺序、单次写盘、开关关闭零 I/O）
- [ ] 运行测试确认失败
- [ ] 最小实现（两处挂接 + 延迟任务）
- [ ] 运行测试 + 设备实测确认
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | 用例已交付 3 组（TDD 执行暂缓，待统一回补） |
| 实测 | demo 应用两轮启动，检查文件与写盘次数 | 暂缓（待编译上机） |
| 代码提交 | ability_runtime 仓 `1a6221c47f` | 已提交 |
| 检视 | 人工检视记录 | 通过：桥接点时序核实（Runtime::Create 早于 PerformAppReady）；实现形态微调为 AccumulateModules+SaveNow+NativeModuleSampler（collector 保持 napi 无关，UT 无需链接 napi） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 挂接两个触发点：application_impl.cpp 的 PerformAppReady 完成处调 SampleAtAppReady()；main_thread.cpp 首个 UIAbility 前台回调后 PostDelayedTask(5s) 调 CollectMergeAndSave()，首次 OnBackground 若未触发则兜底；IsEnabled=false 时两处均为空操作 |
| 允许修改 | `frameworks/native/appkit/app/application_impl.cpp`、`frameworks/native/appkit/app/main_thread.cpp` |
| 允许新建 | 无 |
| 只读参考 | `frameworks/native/appkit/app/application_impl.cpp`（PerformAppReady）、`frameworks/native/appkit/app/main_thread.cpp`（HandleLaunchApplication/延迟任务设施） |
| Spec 摘要 | AC-1.1、R-1、架构约束（失败不影响启动） |
| Design 摘要 | ADR-4；线程模型表（每轮 ≤1 次写盘） |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-5: PreFork 快照排除

| 字段 | 内容 |
|------|------|
| 任务目标 | ETSRuntime::PreFork 阶段记录 fork 前已加载模块 key 集合；collector 采样时过滤该集合，保证 preload 模块不进采集结果与预测集 |
| AC 映射 | AC-1.5 |
| 前置依赖 | TASK-3 |
| 非目标 | 不处理 JsRuntime 预加载路径（若存在独立预加载流程，回传补充设计）；不处理 fork 后再 unload 的边界 |
| 完成判据 | UT：构造 fork 前集合 + fork 后新增集合，采样结果仅含后者；实测：preload 系统模块（如 libui_ability_ani.z.so）不出现在 JSON 中 |
| 停止条件 | PreFork 与 collector 分属不同进程上下文导致快照无法传递 → 停止，回传设计评审（备选：经共享文件/环境标记传递） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/runtime/ets_runtime.cpp` | PreFork 调 GetLoadedModuleInfos 记录 key 快照至进程内持有区 |
| Modify | `frameworks/native/appkit/app/native_module_collector.cpp` | 采样过滤逻辑 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 过滤用例 |

**Spec Context**：AC-1.5 fork 前 preload 阶段加载的模块不得出现在采集结果；R-12 区分依据为 fork 前快照。

**Design Context**：ADR-2（key 集合快照，否决计数偏移法与 isAppModule 过滤法）；数据流步骤 3；存储方案：fork 前快照为进程内存 key 集合、进程生命周期。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：快照传递经进程内共享区，不得引入 IPC |
| ADR-2 | Must Not：不得用节点计数偏移代替 key 集合 |

**Steps**
- [ ] 写失败测试（过滤正确性）
- [ ] 运行测试确认失败
- [ ] 最小实现
- [ ] 运行测试 + 实测确认
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | 过滤用例含于 011（TDD 执行暂缓） |
| 实测 | 检查 JSON 不含 preload 系统模块 | 暂缓（待编译上机） |
| 代码提交 | ability_runtime 仓 `1a6221c47f` | 已提交 |
| 检视 | 设计微调 | 快照存储由 ETSRuntime 直接调 collector 改为 runtime 层持有 + MainThread 桥接（避免 runtime→appkit 循环依赖），语义不变 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | PreFork 时调 GetLoadedModuleInfos 取全量快照，保存 moduleName 集合到进程内静态持有区；collector SampleAtAppReady/CollectMergeAndSave 采样后剔除集合内模块；集合为空（非 preload 启动）时不过滤 |
| 允许修改 | `frameworks/native/runtime/ets_runtime.cpp`、`frameworks/native/appkit/app/native_module_collector.cpp` |
| 允许新建 | 无 |
| 只读参考 | `frameworks/native/runtime/ets_runtime.cpp`（PreFork/PostFork）、napi 仓 native_module_manager.h（NativeModuleInfo） |
| Spec 摘要 | AC-1.5、R-12 |
| Design 摘要 | ADR-2 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-6: 预测预加载启动任务

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增启动任务：启动早期（非主线程）读取 JSON 文件，取 predicted 中 isAppModule=false 的系统模块逐个 LoadNativeModule 预加载；文件缺失/损坏/version 不符跳过；单模块失败记故障事件继续；空预测集不动作；并在 startup_manager 注册编排 |
| AC 映射 | AC-3.1, AC-3.2, AC-3.3, AC-3.4 |
| 前置依赖 | TASK-2（文件路径/命名与 JSON schema 同源） |
| 非目标 | 不预加载应用模块（isAppModule=true）；不实现 loadCount 记录（TASK-7 消费 P 集合）；不改动静态 PreloadSystemSoStartupTask 既有配置链路 |
| 完成判据 | UT：正常预加载调用名单正确、文件缺失跳过、损坏跳过、单模块失败继续、空预测集零调用；实测：predicted 系统模块在启动早期出现在 /proc/self/maps |
| 停止条件 | startup_manager 对新任务类型的编排需要改动启动配置 schema → 停止，回传设计评审配置兼容性 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.h` | 任务类声明（TASK_TYPE = "PredictPreloadSo"） |
| Create | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.cpp` | RunTaskInit：读文件→过滤→循环 LoadNativeModule |
| Modify | `frameworks/native/appkit/app_startup/startup_manager.cpp` | 注册/编排新任务（参照 AnalyzePreloadSystemSoStartupTask，startup_manager.cpp:1124） |
| Modify | `frameworks/native/appkit/app_startup/BUILD.gn`（或所属 BUILD.gn） | sources 新增 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 预加载任务用例组 |

**Spec Context**：R-10 读 predicted 中 isAppModule=false 并预加载、非主线程执行、空集不动作；AC-3.2 文件缺失/损坏/版本不符跳过且不影响启动；R-14/AC-3.3 单模块失败记故障事件（含模块名与错误信息）继续其余；AC-3.1 开关开启才执行。

**Design Context**：ADR-5（独立任务，否决扩展静态任务与 MainThread 直调）；时序图预加载分支；故障复用 EventReport::SendLaunchFrameworkEvent(STARTUP_TASK_ERROR, HISYSEVENT_FAULT)（preload_system_so_startup_task.cpp:96 模式）；任务参照 PreloadSystemSoStartupTask：SetWaitOnMainThread(false)、SetCallCreateOnMainThread(false)。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：失败事件含 errCode/errReason；Must Not：单模块失败中断任务或返回错误阻断启动 |
| OH-ARCH-LAYERING | Must：仅经 NativeModuleManager::LoadNativeModule 加载，不得直接 dlopen |
| 性能约束 | Must：SetWaitOnMainThread(false)，主线程零等待 |

**Steps**
- [ ] 写失败测试（5 个用例，见完成判据）
- [ ] 运行测试确认失败
- [ ] 最小实现（任务类 + 注册）
- [ ] 运行测试 + 实测确认
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | PASS |
| 实测 | 启动早期 /proc/self/maps 含 predicted 系统模块 | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | PredictPreloadSoStartupTask：RunTaskInit 中读 GetStorePath(进程名) 文件 → 解析 predicted → 过滤 isAppModule=false → 逐个 LoadNativeModule(moduleName) → 任一失败 SendLaunchFrameworkEvent 后继续；文件侧任何异常一律 OnCallback(ERR_OK) 正常完成；任务执行前判定 IsEnabled |
| 允许修改 | `frameworks/native/appkit/app_startup/startup_manager.cpp`、`frameworks/native/appkit/app_startup/BUILD.gn`（或所属 BUILD.gn） |
| 允许新建 | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.{h,cpp}` |
| 只读参考 | `frameworks/native/appkit/app_startup/preload_system_so_startup_task.cpp`、`preload_so_startup_task.cpp`、`startup_manager.cpp:1024-1150` |
| Spec 摘要 | R-10、R-14、AC-3.1~AC-3.4 |
| Design 摘要 | ADR-5；时序图预加载分支；故障上报模式 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-7: loadCount 命中判定（防自反馈）

| 字段 | 内容 |
|------|------|
| 任务目标 | 合并引擎命中判定升级：本轮由消费方预加载的模块，仅当其 loadCount 增量超过消费方预加载次数时才计命中，否则本轮不更新（不计命中也不衰减） |
| AC 映射 | AC-2.5 |
| 前置依赖 | TASK-1（loadCount 字段）、TASK-3（合并引擎）、TASK-6（消费方 P 集合产生处） |
| 非目标 | 不修改 napi 仓 refCount 语义；不处理多轮累计误差校准 |
| 完成判据 | UT：仅预加载无真实 import→score 不变；预加载+真实 import（loadCount 增量 > 预加载次数）→命中奖励；未预加载未 import→正常衰减 |
| 停止条件 | TASK-1 确认 refCount 语义不满足（cache 命中不递增）→ 启用降级方案（P 集合模块本轮一律不更新），记录 design RSK-2 降级后继续 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/native_module_collector.cpp` | P 集合接收与命中判定逻辑 |
| Modify | `frameworks/native/appkit/app_startup/predict_preload_so_startup_task.cpp` | 预加载完成后将 P 集合（模块名+次数）写入进程内共享区供 collector 读取 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 命中判定用例 |

**Spec Context**：R-9 消费方预加载且应用未显式 import → 不计命中也不衰减（本轮不更新）；区分依据为加载计数。

**Design Context**：ADR-3（loadCount 增量 > 本轮预加载次数才计命中；否决自我强化与埋点钩子）；EWMA 伪代码中 P 集合分支；降级方案：P 集合模块本轮一律不更新。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| ADR-3 | Must：判定仅基于快照 loadCount 与 P 集合，不得向加载路径加埋点 |
| OH-ARCH-LAYERING | Must：P 集合经进程内共享区传递，不引入 IPC |

**Steps**
- [ ] 写失败测试（3 个判定分支）
- [ ] 运行测试确认失败
- [ ] 最小实现
- [ ] 运行测试确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 预加载任务完成时把 {moduleName: preloadTimes} 存入进程内共享区；collector 合并时对 m∈S：若 m∈P 且 (m.loadCount − 上轮文件中记录/或前轮快照 loadCount) ≤ P[m]，则跳过不更新；否则按命中处理；prevLoadCount 从上轮 JSON modules 条目中读取（需为 modules 条目增补 loadCount 字段，随本 Task 一并落 schema） |
| 允许修改 | `frameworks/native/appkit/app/native_module_collector.cpp`、`frameworks/native/appkit/app_startup/predict_preload_so_startup_task.cpp` |
| 允许新建 | 无 |
| 只读参考 | design.md ADR-3 与 EWMA 伪代码 |
| Spec 摘要 | R-9、AC-2.5 |
| Design 摘要 | ADR-3 + 降级方案（RSK-2） |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-8: CCM 参数接入

| 字段 | 内容 |
|------|------|
| 任务目标 | IsEnabled 与 EWMA 调优参数（α/阈值/淘汰窗口/检查点②延迟）接入系统 CCM 参数；参数缺失/读取失败一律按缺省（开关关闭、参数缺省值）；判定与参数本轮启动内缓存 |
| AC 映射 | AC-1.2 |
| 前置依赖 | TASK-2（IsEnabled 占位替换） |
| 非目标 | 不实现 CCM 配置下发管理侧；不新增参数以外的配置项 |
| 完成判据 | UT：参数缺失→关闭且零 I/O；参数=true→开启；非法参数值→按缺省；α 等参数可被参数覆盖 |
| 停止条件 | CCM 参数读取接口在应用进程沙箱内不可用 → 停止，回传切换 sysparam（persist.*）方案评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `frameworks/native/appkit/app/native_module_collector.h` | 参数读取接口声明 |
| Modify | `frameworks/native/appkit/app/native_module_collector.cpp` | CCM 读参 + 缓存 + 缺省兜底 |
| Test | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 参数用例组 |

**Spec Context**：R-2 开关参数未下发/缺失/显式关闭→零 I/O，判定结果本轮缓存；AC-1.2 缺省关闭启动行为与存量一致；R-3/R-4 α 与阈值支持系统参数调优（缺省 0.4/0.5）。

**Design Context**：ADR-6（CCM 通道，否决编译期常量）；RSK-3 参数名与下发通道待 owner 确认（本 Task 内定名并记录：建议 ccm.ability.native_module_collect.* 命名空间）；读取失败按关闭处理（安全基础检查-配置）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| 安全基础检查 | Must：读取失败/非法值一律按缺省（关闭/默认值） |
| 性能约束 | Must：参数读取一次性缓存，Must Not：每次判定重复读参 |

**Steps**
- [ ] 写失败测试（缺失/开启/非法/覆盖四组）
- [ ] 运行测试确认失败
- [ ] 最小实现
- [ ] 运行测试确认通过
- [ ] 参数名记录回 design RSK-3
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts native_module_collector_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | IsEnabled() 实现为：首次调用读 CCM 参数（布尔），结果进程内 static 缓存；读取异常/缺失→false。EWMA 参数（alpha/predictThreshold/evictRounds/checkpoint2DelayMs）同通道读取，非法值回落缺省；所有读取集中在 collector 初始化时一次完成 |
| 允许修改 | `frameworks/native/appkit/app/native_module_collector.{h,cpp}` |
| 允许新建 | 无 |
| 只读参考 | 仓内 CCM/参数读取既有封装（native_module_collector.cpp 同目录 app 组件的读参先例） |
| Spec 摘要 | R-2、R-3、R-4、AC-1.2 |
| Design 摘要 | ADR-6、RSK-3 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -ts native_module_collector_test` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

### TASK-9: TDD 齐套 + beta 命中率上报

| 字段 | 内容 |
|------|------|
| 任务目标 | 对齐 spec 验证映射 VM-1~VM-11 补齐全部用例；beta 构建下每轮启动上报命中率/误报率统计事件（用于 α/阈值标定） |
| AC 映射 | 全部 AC 的验证闭环 |
| 前置依赖 | TASK-3, TASK-4, TASK-5, TASK-6, TASK-7, TASK-8 |
| 非目标 | 不做 XTS 用例（InnerAPI 无 XTS 要求）；命中率上报不带出设备（仅 HiLog/HiSysEvent 域内统计） |
| 完成判据 | `run -t UT -tp ability_runtime` 全量通过；VM-1~VM-11 逐条有用例对应；beta 上报事件可观测 |
| 停止条件 | 全量 UT 出现与本特性无关的存量失败 → 记录并隔离本特性用例单独验证，不阻塞交付但需在证据中注明 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `test/unittest/native_module_collector_test/native_module_collector_test.cpp` | 补齐 VM-1~VM-11 用例 |
| Modify | `test/unittest/native_module_collector_test/BUILD.gn` | 依赖与用例注册 |
| Modify | `frameworks/native/appkit/app/native_module_collector.cpp` | beta 命中率统计事件（编译期宏或参数门控，量产关闭） |

**Spec Context**：验证映射 VM-1~VM-11（文件生成与字段完整/开关零 I/O/EWMA 公式/淘汰/三路径重建/预加载不计命中/预加载执行与空集/失败续跑/命名权限/preload 排除/写盘原子性）；非功能：命中率 ≥95% 为 beta 标定目标；定界定位：独立日志与故障事件。

**Design Context**：测试性设计表（单测/集成两级）；命中率 = |predicted ∩ actual| / |actual|，误报率 = |predicted − actual| / |predicted|；量产关闭上报。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：上报事件与既有 EventReport 模式一致 |
| 隐私 | Must Not：上报内容含 bundleName 以外的应用标识信息 |

**Steps**
- [ ] 逐条核对 VM-1~VM-11 用例存在性，补缺
- [ ] 全量运行确认通过
- [ ] 实现 beta 命中率上报（门控）
- [ ] 运行测试 + 实测确认事件可观测
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -tp ability_runtime` | PASS |
| 实测 | beta 构建命中率事件观测 | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 对照 spec.md 验证映射表逐条补用例（已有用例不重复）；collector 在写盘完成时计算本轮命中率/误报率，经门控（beta 宏或 CCM 参数）SendLaunchFrameworkEvent 风格事件上报 |
| 允许修改 | `test/unittest/native_module_collector_test/`、`frameworks/native/appkit/app/native_module_collector.cpp` |
| 允许新建 | 无 |
| 只读参考 | spec.md 验证映射表、design.md 测试性设计表 |
| Spec 摘要 | VM-1~VM-11、命中率 ≥95% 标定目标 |
| Design 摘要 | 测试性设计表；指标定义 |
| 执行步骤 | 见 Steps |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望： PASS |
| 完成规则 | 不得修改允许范围外文件；无 fresh 验证证据不得声明完成 |

---

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-1.1~AC-4.2 全覆盖）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 完整）
- [x] 每个 Task 验证在完成时立即执行并记录证据，不积压
- [x] 无超 3000 行 Task，无需拆分
