# 特性规格

> 固化用户可见行为和验收标准。本文只描述外部可观察行为、API/InnerAPI 行为契约与验收标准；内部实现结构、调用链、并发与算法归 design.md。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 启动阶段 Native 模块采集与跨启动预测预加载系统 SO |
| 特性编号 | FEAT-30096301 |
| 所属 Epic | 无（独立特性） |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-7.1 |
| SIG 归属 | 元能力 SIG（napi 仓协同） |
| 状态 | Approved |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

> 本次为全新特性（manifest.lineage: new），Delta 表用于记录新增能力，便于评审时快速定位变化面。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | 启动阶段系统 native 模块采集（ability_runtime NativeModuleCollector） | 新增，应用进程内 |
| ADDED | EWMA 跨启动预测 + 预测文件原子落盘（应用 cacheDir 子目录）+ 多实例并发合并 | 新增，应用进程内 |
| ADDED | app_startup 预测预加载启动任务（非主线程消费 predicted，调用 NativeModuleManager 系统模块预加载接口） | 新增，应用进程内 |
| ADDED | napi `NativeModuleManager` 已加载系统模块查询 InnerAPI（仅系统模块名） | 新增，InnerAPI |
| ADDED | napi `NativeModuleManager` 系统模块预加载 InnerAPI（内部拼接路径 + `dlopen_preload`） | 新增，InnerAPI |
| ADDED | 设备差异化系统参数开关 `const.abilityruntime.nativepreload.enable` | 新增，系统参数 |
| MODIFIED | 无 | — |
| REMOVED | 无 | — |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `.codespec/changes/issue-30096301-native-so-preload/proposal.md` | AwaitingApproval（v1.1 修订待批准；v1.0 已批准） |
| 外部依赖分析 | `third_party/musl/docs/dlopen-analysis/report.md` | 参考 |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。

## 用户故事

### US-1: 采集启动阶段显式 import 的系统 native 模块

**作为** 系统开发者，
**我想要** 采集应用冷启动阶段显式 import 的系统 native 模块并保存到应用 cacheDir，
**以便** 后续做依赖分析与启动优化。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1 | WHEN 应用主进程冷启动（非预加载模式、非 debug）、系统参数开关开启且设备类型受支持，应用在采集窗口内显式 import 系统 native 模块 THEN 该模块 cacheDir 下按模块名区分的采集子目录中可观察到本次采集到的系统模块名记录（仅系统模块，不含应用模块与 SO 路径） | 终端用户/InnerAPI | 正常 |
| AC-8 | WHEN ability_runtime 查询已加载 native 模块 THEN napi 提供的查询 InnerAPI 仅返回已加载的系统模块名集合（不含 isAppModule、不含 SO 路径），且不改变既有模块加载行为 | InnerAPI | 正常 |

### US-2: 跨启动预测与防自反馈

**作为** 系统，
**我想要** 基于历史与本次采集用 EWMA 预测下次启动将加载的系统模块，并避免预加载自我强化，
**以便** 提前预加载以缩短真实 import 的加载耗时。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-2 | WHEN 采集窗口结束 THEN 应用 cacheDir 子目录下可观察到按 EWMA 合并本次采集与历史评分后的预测结果（predicted 预测集），且以临时文件更名方式原子落盘、文件名为复杂唯一命名，不与开发者文件冲突 | 终端用户/InnerAPI | 正常 |
| AC-4 | WHEN 本轮已对某系统模块执行了预加载 THEN 该模块不因其被预加载而出现在本轮已加载系统模块采集结果中（防自反馈） | InnerAPI | 正常 |
| AC-9 | WHEN 同一应用多个实例并发冷启动 THEN 各实例的采集结果在预测文件中合并（并发命中不丢失、采集为并集），任一实例不因并发写失败 | InnerAPI | 边界 |

### US-3: 启动框架预测预加载系统 SO

**作为** 启动框架，
**我想要** 在应用下次冷启动时读取预测集并预加载对应系统 SO，
**以便** 缩短真实 import 的加载耗时。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-3 | WHEN 应用下次冷启动、系统参数开关开启、设备受支持且存在有效预测文件 THEN 启动框架在非主线程对预测集中系统模块调用 NativeModuleManager 系统模块预加载接口完成预加载（load/link，不执行构造函数），不阻塞主线程 | System API | 正常 |
| AC-7 | WHEN 预测集中某单个系统模块预加载失败（路径失效/依赖缺失等）THEN 记录故障事件并继续处理其余模块，应用正常启动 | 终端用户 | 异常 |

### US-4: 异常与设备差异降级

**作为** 最终用户，
**我想要** 预加载失败或预测文件异常时应用启动不受影响，
**以便** 不因优化能力引入启动风险。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-5 | WHEN 系统参数开关关闭、设备类型不在支持集（PC/手机/平板默认支持，其他默认不支持）、或非主进程/应用预加载模式/debug 应用 THEN 不采集、不落盘、不预加载，应用行为与变更前一致 | System API | 边界 |
| AC-6 | WHEN 预测文件或采集文件不存在、内容损坏、字段非法或落盘失败 THEN 应用正常启动、不崩溃；读取失败时降级为不预加载，落盘失败时保留旧文件或跳过本次更新 | 终端用户 | 异常 |
| AC-10 | WHEN 执行本特性相关单元测试 THEN 覆盖采集聚合、EWMA 预测与落盘、原子落盘、预测读取、防自反馈、文件异常降级、单模块预加载失败、设备开关关闭、多实例并发等路径且全部通过 | InnerAPI | 正常 |

## 验收追溯

> 本表只维护 Spec 内部的 AC → 规则 → 可观察表面事实。AC → Task 由 execution-plan 维护；测试入口与 Red 条件由“验证映射”维护；执行证据由 `evidence/reviews/` 维护。

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-1 | R-1, R-3 | 终端用户/InnerAPI |
| AC-2 | R-2, R-3 | 终端用户/InnerAPI |
| AC-3 | R-5 | System API |
| AC-4 | R-4 | InnerAPI |
| AC-5 | R-6 | System API |
| AC-6 | R-7, R-8 | 终端用户 |
| AC-7 | R-9 | 终端用户 |
| AC-8 | R-10 | InnerAPI |
| AC-9 | R-11 | InnerAPI |
| AC-10 | R-12 | InnerAPI |

## 规则定义

> 类型标签：行为（正常路径）、边界（临界点）、异常（非法输入/异常状态）、恢复（异常后恢复策略）。

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 系统参数开关为开启、设备类型受支持，应用冷启动且在采集窗口内显式 import 系统 native 模块 | 采集到这些系统模块的模块名，形成本次采集集 | 仅系统模块；不含应用模块/路径 | AC-1 |
| R-2 | 行为 | 采集窗口正常结束 | 以 EWMA 合并本次采集与历史评分得到预测集，并以临时文件更名方式原子落盘到应用 cacheDir 子目录 | 预测集元素为系统模块名 | AC-2 |
| R-3 | 边界 | 采集窗口起止：开始=`LoadExtStartupTasks` 触发，结束=`HandleLaunchApplication` 完成后 5 秒 | 窗口内采集，窗口结束触发预测与落盘 | 时长 5s（常量，可后续校准） | AC-1, AC-2 |
| R-4 | 行为 | 本轮已对系统模块 M 执行预加载，且 M 在本轮又被采集 | M 不计入本轮预测命中（预加载走不注册的 `dlopen_preload`，不出现在已加载系统模块枚举中） | 不引入打标机制 | AC-4 |
| R-5 | 行为 | 应用下次冷启动、开关开启、设备受支持且存在有效预测文件 | 非主线程对预测集中系统模块调用 NativeModuleManager 预加载接口（内部 `dlopen_preload`），不阻塞主线程、不执行构造函数 | mode 待与 musl 对齐 | AC-3 |
| R-6 | 边界 | 系统参数开关为关闭，或设备类型不在 {手机, 平板, PC} | 全链路不生效：不采集、不落盘、不预加载 | 开关默认值待 design 定 | AC-5 |
| R-7 | 异常 | 预测/采集文件不存在或内容损坏/字段非法 | 跳过预加载、按无历史处理，应用正常启动 | 不抛异常、不崩溃 | AC-6 |
| R-8 | 异常 | 预测结果落盘失败（空间不足/权限/中断）或目录不可写 | 保留旧文件或跳过本次更新，应用正常启动 | 失败静默降级 | AC-6 |
| R-9 | 异常 | 预测集中某单个系统模块预加载失败 | 记录故障事件并继续处理其余模块，应用正常启动 | 单模块失败不影响整体 | AC-7 |
| R-10 | 行为 | 调用已加载系统模块查询 InnerAPI | 返回已加载系统模块名集合（不含 isAppModule/路径），不改既有加载行为 | 只读查询 | AC-8 |
| R-11 | 并发 | 同一应用多实例并发冷启动并读写预测文件 | 在跨进程互斥内"读最新状态→应用本轮命中→原子提交"；任一实例持锁失败则跳过本轮落盘 | 命中合并为并集、不丢失 | AC-9 |
| R-12 | 行为 | 执行本特性单元测试 | 覆盖采集/预测/落盘/读取/防自反馈/异常降级/单模块失败/开关关闭/并发 | 用例全部通过 | AC-10 |

## 验证映射

> 每个 AC 至少关联一个可执行测试入口。Red 条件描述实现前测试为何必然失败及可观察到的失败信号。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1 / AC-1 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.CollectAndPersist_0100` | 单测 | 采集器与查询 InnerAPI 均未实现，查询接口无法返回已加载系统模块名，采集集为空 | 采集集包含窗口内显式 import 的系统模块名，系统模块纳入、应用模块排除 |
| VM-2 | R-2, R-3 / AC-2 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.EwmaPredict_0500` | 单测 | 未实现预测与落盘，cacheDir 子目录下不存在预测文件，读不到预测集 | 子目录下存在预测文件，内容为 EWMA 合并后的 predicted 集，存在临时文件更名痕迹 |
| VM-3 | R-5 / AC-3 | `test/unittest/frameworks_kits_appkit_native_test::PredictPreloadTaskTest.RunOnNonMainThread_0100` | 单测 | 未新增预测预加载任务与 napi 预加载接口，启动框架不产生基于 predicted 的预加载调用，主线程被占用 | 预加载在非主线程发起，对 predicted 系统模块调用 NativeModuleManager 预加载接口，主线程未被阻塞 |
| VM-4 | R-4 / AC-4 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.ExcludePreloaded_0200` | 单测 | 未实现隔离，预加载模块出现在已加载系统模块枚举/采集结果中 | 仅被预加载的模块不出现在采集结果中，评分未被预加载抬高 |
| VM-5 | R-6 / AC-5 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.DisabledByParam_0400` | 单测 | 开关判断未实现，关闭开关时仍执行采集/预加载 | 开关关闭或设备不支持时不采集、不落盘、不预加载 |
| VM-6 | R-7, R-8 / AC-6 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.CorruptDegrade_0300` | 单测 | 未做容错，损坏/缺失文件触发异常或崩溃 | 文件缺失/损坏/落盘失败时无异常、不崩溃，降级为不预加载/跳过更新 |
| VM-7 | R-9 / AC-7 | `test/unittest/frameworks_kits_appkit_native_test::PredictPreloadTaskTest.SingleModuleFailureContinue_0200` | 单测 | 未做单模块失败隔离，单模块失败中断整体或影响启动 | 单模块失败记录故障事件后继续处理其余模块，应用正常启动 |
| VM-8 | R-10 / AC-8 | `foundation/arkui/napi/module_manager/test/unittest/module_manager_test/module_manager_test.cpp::ModuleManagerTest.GetLoadedSystemNativeModules_001` | 单测 | InnerAPI 不存在，无法查询已加载系统模块名 | 查询返回已加载系统模块名集合（不含 isAppModule/路径），既有加载用例不受影响 |
| VM-9 | R-11 / AC-9 | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.ConcurrentMerge_0600` | 单测 | 未实现子目录与跨进程互斥，并发写互相覆盖（断言并集失败） | 实例 A 命中 {m1}、实例 B 命中 {m2} 后，预测文件同时包含 m1、m2；无临时文件残留 |
| VM-10 | R-12 / AC-10 | `test/unittest/frameworks_kits_appkit_native_test`（本特性新增/扩展用例集） | 单测 | 用例不存在或无实现，全部失败 | 上述路径用例全部通过 |
| VM-11 | R-1, R-3 / AC-1（端到端采集落盘） | 真机：开启参数开关后冷启动含 native import 的测试应用 | 人工/集成 | 无该能力时模块沙箱下无 `haps/<module>/cache/native_module_collect/collected.json` | 冷启动后该文件生成，内容 JSON 合法且含启动期 import 的系统模块名 |
| VM-12 | R-2, R-4 / AC-2, AC-4（端到端跨启动预测） | 真机：同一应用连续冷启动两次，检查预测集演化 | 人工/集成 | 无该能力时无 `predict.json` 或内容不随启动更新 | 第二次冷启动后 `predict.json` 含按 EWMA 合并的 predicted 集；仅被预加载的模块不计入命中 |
| VM-13 | R-5 / AC-3（端到端预测预加载生效） | 真机：已有预测文件后冷启动，观察日志与加载行为；并用 HiTrace 观察相关系统模块的加载时机 | 人工/集成 | 无该能力时无预测预加载日志，trace 中模块加载落在真实 import 时刻 | 日志可见在非主线程对 predicted 系统模块执行预加载（`predict preload done`），且 HiTrace 上这些系统模块的 load/link 时机提前（命中预加载）、真实 import 时刻的加载耗时缩短；应用启动不受影响 |
| VM-14 | R-7, R-8 / AC-6（端到端异常降级） | 真机：人为置损坏/缺失预测文件后冷启动 | 人工/集成 | 无容错时启动崩溃或报错 | 应用正常启动，损坏文件按无历史处理且本次重新生成有效文件 |
| VM-15 | R-11 / AC-9（端到端多实例并发） | 真机：同应用多实例并发冷启动 | 人工/集成 | 无并发合并时预测结果被互相覆盖丢失 | 各实例命中合并为并集，预测文件完整，任一实例不因并发失败 |
| VM-16 | R-6 / AC-5（端到端门控不生效） | 真机：参数开关关闭或设备不支持时冷启动 | 人工/集成 | 无门控时仍产生文件与预加载 | 不产生采集/预测文件、无预加载行为，应用行为与未支持本特性一致 |
| VM-17 | R-5 / AC-3（停止机制） | `test/unittest/frameworks_kits_appkit_native_test::PredictPreloadTaskTest.StoppedSkipsPreload_0300` | 单测 | 未接入 autoPreloadSoStopped_ 时，被停止后仍执行预加载 | 开关开启且 autoPreloadSoStopped_=true 时不发起预加载调用 |
| VM-18 | R-2 / AC-2（debug 门控落盘） | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.CollectedOnlyInDebug_0700` | 单测 | 未做 debug 门控时默认也写 collected.json | debug 开关关闭时不写 collected.json 但仍写 predict.json，开启时写 collected.json |
| VM-19 | R-2 / AC-2（参数可调与默认值兜底） | `test/unittest/frameworks_kits_appkit_native_test::NativeModuleCollectorTest.CollectConfigSanitize_0800` | 单测 | 无校验时非法参数被采用导致预测异常 | 非法参数回退默认值；系统参数覆盖路径生效 |

## API 变更分析

### 新增 API

| API 名称 | 开放级别 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `NativeModuleManager::GetLoadedSystemNativeModules` | InnerAPI | 无 | 已加载系统模块名集合（`std::vector<std::string>`） | 无（无数据返回空集合） | 枚举当前进程已加载的系统 native 模块名，供 ability_runtime 采集 | AC-8, AC-1 |
| `NativeModuleManager::PreloadSystemNativeModule` | InnerAPI | 系统模块名 | 成功 0 / 失败 -1 | 无固定错误码 | 内部拼接系统 SO 路径并调用 `dlopen_preload` 完成系统模块预加载（load/link，不注册） | AC-3 |

### 变更/废弃 API

N/A：本次无变更或废弃的 API（既有 `LoadNativeModule` 系列签名与语义不变）。

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源（文件:行/符号或 New 依据） | 关联 AC |
|--------|----------|----------|---------------|----------|----------|--------------------------------------|---------|
| `NativeModuleManager::GetLoadedSystemNativeModules` | New API | InnerAPI | `std::vector<std::string> NativeModuleManager::GetLoadedSystemNativeModules() const` | 调用方查询已加载系统模块时 | 返回已加载系统模块名集合（不含 isAppModule/路径）；无数据返回空集合 | New: proposal.md API 变更项清单 / AC-8 | AC-8, AC-1 |
| `NativeModuleManager::PreloadSystemNativeModule` | New API | InnerAPI | `int NativeModuleManager::PreloadSystemNativeModule(const char* moduleName)` | 启动框架对预测系统模块发起预加载时 | 内部拼接系统 SO 路径并调用 `dlopen_preload`；成功返回 0、失败返回 -1 | New: proposal.md API 变更项清单 / AC-3 | AC-3 |
| `dlopen_preload` | Existing API | System | `int dlopen_preload(const char *file, int mode)` | 被 `PreloadSystemNativeModule` 内部调用 | 成功返回 0；失败返回 -1；仅完成 load/link，不执行构造函数 | `third_party/musl/include/linux/dlfcn_ext.h:101` / `dlopen_preload` | AC-3 |

错误码：N/A：本次无新增错误码（查询接口无数据返回空集合；预加载失败返回 -1 并上报故障事件）。

> 说明：`PreloadSystemNativeModule` 内部调用 `dlopen_preload` 的 mode（`RTLD_NOW` 或 `RTLD_LAZY`）**待与 musl 团队讨论后确定**，本 spec 不写死；首版 musl 文档注释限定 `RTLD_NOW | RTLD_LOCAL`（`dlfcn_ext.h:97`）。

## 接口规格

### 接口定义

**NativeModuleManager::GetLoadedSystemNativeModules**

| 属性 | 值 |
|------|-----|
| 函数签名 | `std::vector<std::string> NativeModuleManager::GetLoadedSystemNativeModules() const` |
| 返回值 | `std::vector<std::string>` — 已加载系统模块名集合，空表示无数据 |
| 开放级别 | InnerAPI |
| 错误码 | N/A（无错误码，无数据返回空） |
| 关联 AC | AC-8 |

**NativeModuleManager::PreloadSystemNativeModule**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int NativeModuleManager::PreloadSystemNativeModule(const char* moduleName)` |
| 返回值 | `int` — 成功 0；失败 -1 |
| 开放级别 | InnerAPI |
| 错误码 | N/A（以返回值表征成功/失败） |
| 关联 AC | AC-3 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| moduleName | const char* | 是 | — | 非空系统模块名；解析失败或预加载失败返回 -1 |
| （GetLoadedSystemNativeModules）无参数 | — | — | — | 只读查询 |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | 进程已加载若干系统模块后调用查询 | InnerAPI | 返回非空系统模块名集合，不含应用模块 | AC-8 |
| 2 | 无已加载系统模块时调用查询 | InnerAPI | 返回空集合，不报错 | AC-8 |
| 3 | 对有效系统模块名调用预加载 | System API | 返回 0；对应系统 so 完成 load/link，主线程不阻塞 | AC-3 |
| 4 | 对不可加载/路径失效模块名调用预加载 | System API | 返回 -1；不崩溃，记录故障事件 | AC-7 |

## 兼容性声明

- **已有 API 行为变更:** 否（不修改 `LoadNativeModule` 与既有静态预加载链路行为）
- **配置文件格式变更:** 否（新增独立预测/采集文件，不改既有配置）
- **数据存储格式变更:** 否（新增应用沙箱 cacheDir **子目录**下的采集/预测文件，含版本字段；无存量数据迁移；旧版本无该文件按“文件不存在”处理）
- **最低支持版本:** 本特性合入版本起（target_release：OpenHarmony-7.1）
- **API 版本号策略:** 新增 InnerAPI 不面向应用，无 `@since` 标注；采集/预测文件格式以文件内 version 字段演进

## 数据存储格式

**落盘目录结构**（写入被启动**模块**的模块级沙箱，而非应用顶层 cacheDir；按模块隔离）：

```
<appBase>/haps/<moduleName>/cache/native_module_collect/
├── predict.json     # 预测集与评分（跨启动持久化）
├── collected.json   # 本轮采集窗口采集到的系统模块快照（仅 debug 开关开启时生成）
└── .lock            # 跨进程互斥文件（flock）
```

- `<appBase>`：应用沙箱基础目录（普通应用为 `/data/storage/el2/base`；系统应用为 `/data/app/el2/<userId>/base/<bundleName>`）。
- `haps/<moduleName>`：模块级沙箱路径（被启动模块的独立目录）；不同模块各自隔离，互不影响。
- `cache/native_module_collect`：本特性专用子目录（复杂命名，避免与模块自身缓存文件碰撞；`mkdir` 冲突时按降级处理，不影响应用）。
- `collected.json` 仅为可观测性的本轮原始快照，**仅在 debug 开关开启时落盘**（`const.abilityruntime.nativepreload.debug`）；预测链路不依赖它，默认关闭以减少启动期 IO。

**文件格式**（均为 JSON，含版本字段；版本不支持或内容损坏时按"无历史数据"降级处理）：

`predict.json`：

```json
{
  "version": 1,
  "scores": { "<moduleName>": 500 },
  "predicted": ["<moduleName>", "..."]
}
```

| 字段 | 类型 | 含义 | 约束 |
|------|------|------|------|
| version | number | 文件格式版本号 | 当前为 1；不支持则按无历史处理 |
| scores | object（map：模块名→评分） | 各系统模块的 EWMA 评分 | 毫定标整数，范围 0..1000 |
| predicted | string[] | 入选预测集的系统模块名 | 按评分降序、不超上限（默认 32） |

`collected.json`：

```json
{
  "version": 1,
  "collected": ["<moduleName>", "..."]
}
```

| 字段 | 类型 | 含义 | 约束 |
|------|------|------|------|
| version | number | 文件格式版本号 | 当前为 1 |
| collected | string[] | 本轮采集窗口内显式 import 的系统模块名 | 仅系统模块（isAppModule=false） |

**写盘原子性**：先写唯一临时文件（含进程号）再 `rename` 原子替换，任何失败不留半成品文件；多实例并发经 `.lock` 跨进程互斥后合并。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅在受支持设备启用 | 通过系统参数 `const.abilityruntime.nativepreload.enable` 与设备类型共同判定 | AC-1, AC-3, AC-5 |
| 仅在主进程冷启动启用 | 非主进程（extension/子进程）、应用预加载模式、debug 应用不采集不预加载；触发点挂在 `main_thread` 的 `HandleLaunchApplication` 中 `RunNativeStartupTask` 之后（静态预加载先行），独立于 app_startup 配置 | AC-1, AC-5 |
| 按启动模块区分 | 采集/预测结果写入应用 cacheDir 下按模块名拆分的子目录 | AC-1, AC-2 |
| 预加载不得阻塞主线程 | 预加载必须在非主线程执行 | AC-3 |
| 失败必须降级不影响启动 | 文件异常/落盘失败/单模块失败均不得影响应用启动 | AC-6, AC-7 |
| 不改变既有加载与静态预加载语义 | 不修改 `LoadNativeModule` 与既有白名单预加载链路行为 | AC-8 |
| EWMA 参数可经系统参数调优 | 采集窗口/评分 α/入选阈值/预测集上限支持经 `const.abilityruntime.nativepreload.*` 系统参数覆盖，便于测试调优；缺省用默认值 | AC-2 |
| 预加载不注册模块 | 预加载经 `dlopen_preload` 只做 load/link、不注册，因此不出现在已加载系统模块枚举中（结构性防自反馈） | AC-4 |
| 多实例并发合并 | 预测文件读写经跨进程互斥 + 原子提交，命中合并为并集 | AC-9 |
| 写盘原子性 | 临时文件（唯一名）加更名原子替换；失败不留半成品文件 | AC-6 |
| mode 待定 | `dlopen_preload` 的 mode 待与 musl 对齐；过渡期靠 dev/ino 去重复用映射 | AC-3 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | N/A：本需求仅承载功能，性能仅作观测、不作门禁；预加载线程低优先级执行 | 真机冷启动日志 | 待真机验证 |
| 功耗 | N/A：无新增常驻/后台任务，采集窗口结束后无额外占用 | 待真机验证 | — |
| 内存 | 预测集元素数有上限（上限值在 design 固化）；采集/预测为启动期短生命周期对象 | 单测 | 待补 |
| 安全 | 仅读写应用自身沙箱 cacheDir 子目录，不涉及权限授予；读入做格式/版本校验，损坏即弃用 | 单测（损坏注入） | VM-6 |
| 可靠性 | 文件异常/单模块失败均降级，应用启动不受影响 | 单测异常路径 | VM-6, VM-7 |
| 可测试性 | 采集/预测/预加载/降级/并发路径均可单测覆盖 | 单测 | VM-1..VM-10 |
| 自动化维测 | 单模块预加载失败上报故障事件，可观测 | hilog/事件 | VM-7 |
| 定界定位 | 预加载失败与文件异常有对应日志/事件 | hilog | VM-6, VM-7 |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 默认支持 | 通过系统参数开关控制 | 单测 + 真机 | VM-5 |
| 平板 | 默认支持 | 通过系统参数开关控制 | 单测 | VM-5 |
| PC | 默认支持 | 通过系统参数开关控制 | 单测 | VM-5 |
| 其他设备 | 默认不支持 | 开关关闭时全链路不生效 | 单测 | VM-5 |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 无 UI 变化 | — |
| 大字体 | 否 | 无 UI 变化 | — |
| 深色模式 | 否 | 无 UI 变化 | — |
| 多窗口/分屏 | 否 | 与窗口无关 | — |
| 多用户 | 否 | 进程内能力，随应用沙箱隔离 | — |
| 版本升级 | 是 | 采集/预测文件含版本字段，跨版本不兼容时按异常降级 | AC-6 |
| 生态兼容 | 否 | 不影响三方应用兼容性 | — |
