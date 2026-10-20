---
phase_status: approved
approval:
  status: approved
  approver: "yangzk（Owner）"
  evidence: "需求方评审批准 v1.2 修订（按模块区分 + 主进程冷启动门控 + 独立于 appStartup 配置 + 静态预加载先行）：会话回复“重新批准 + amend提交”（2026-10-16）"
  approved_at: "2026-10-16"
  baseline_digest: "6a6a6cb35067898702498fb17c1b21fa7f6cd629a64ff01cdf75c7f637be96f4"
target_release:
  id: OpenHarmony-7.1
  status: proposed
---

# 需求文档

> 主文档只保存需求输入和已确认基线。澄清过程见文末「附录：澄清与决策记录（条件启用）」（本需求涉及两仓协同 + 外部依赖，触发条件附录）。

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | RR-30096301 |
| 需求名称 | 启动阶段 Native 模块采集与跨启动预测预加载系统 SO |
| 来源 | RR 需求单 RR-30096301 |
| 提出人 | 需求方（RR-30096301）/ 承接人 yangzk |
| 目标发行版本 | OpenHarmony-7.1 |
| 候选 Profile | none |
| 优先级 | P1 |
| Define 阶段状态 | Approved |

### 原始描述

**原始问题：** 应用冷启动场景下，启动阶段实际加载的 Native 模块集合对系统不可见，无法支撑依赖分析与启动优化；启动框架预加载配置为静态列表，无法随应用实际加载行为自我演化。

**需求价值：** 补齐启动阶段模块依赖对系统的可视性，通过跨启动预测与启动框架预加载系统 so，缩短应用真实 import 时的加载耗时，优化应用冷启动。

**需求场景：** 应用冷启动场景。

**原始处理要求（需求方原话）：**
1. napi 仓 NativeModuleManager 新增已加载模块查询的 InnerAPI。
2. ability_runtime 新增 NativeModuleCollector，采集本次启动显式 import 的系统模块。开始时机暂定为 HandleLaunchApplication 中 LoadExtStartupTasks 触发，结束时机暂定 HandleLaunchApplication 完成后 5 秒。
3. 基于 EWMA 评分方案输出 predicted 预测集；tmp+rename 原子写盘至 cacheDir，文件名修改为更加复杂一些，避免和开发者的文件重复；并考虑更新情况（每次启动不一定一致，需要根据一定策略更新，尽可能准确预测下一次启动会加载的模块）。
4. 防自反馈判定：消费方本轮预加载的模块不计为应用真实 import 命中，避免预测自我强化。
5. 启动框架新增预加载消费：app_startup 新增预测预加载启动任务，启动早期读 predicted 中系统模块（isAppModule=false），在非主线程执行 LoadNativeModule 预加载；单模块失败记故障事件续跑，不影响启动；使用 third_party/musl 中提供的 dlopen_preload 能力做预加载。

**输出（需求方原话）：**
1. 能够采集到启动阶段加载的模块。
2. 基于此前采集到的模块预加载信息进行预加载。

**验收场景和标准（需求方原话）：**
1. 能够采集到启动阶段加载的模块，且正确保存到 cacheDir 下。
2. 基于此前采集到的模块预加载信息进行预加载。
3. 文件异常或不存在不影响应用启动。

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 应用开发者/最终用户 | 冷启动阶段系统 so 加载耗时不可优化，启动慢 | 应用冷启动体验 |
| 系统开发者 | 启动期实际 import 的 native 模块集合不可见，缺乏依赖分析与优化数据 | 启动优化能力缺失 |

**期望结果：** 系统能采集到应用冷启动阶段显式 import 的系统 native 模块，跨启动预测下一次启动将加载的系统模块，并在启动框架预加载这些系统 so，缩短真实 import 时的加载耗时；异常/文件缺失时不影响应用启动。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码（napi） | `foundation/arkui/napi/module_manager/native_module_manager.{h,cpp}` | `NativeModule` 结构含 `name`/`moduleName`/`systemFilePath`/`moduleLoaded`/`isAppModule`；系统模块加载时 `systemFilePath=strdup(loadPath)`、`moduleLoaded=true`（native_module_manager.cpp:1527-1601） |
| 源码（ability_runtime） | `frameworks/native/appkit/app/main_thread.cpp` | `HandleLaunchApplication` 起点 :1659，内部 :1862 调用 `LoadExtStartupTasks`（定义 :4586），`HandleLaunchApplication end` :2174 |
| 源码（ability_runtime） | `frameworks/native/appkit/app_startup/` | 已存在静态白名单预加载 `PreloadSystemSoStartupTask`（preload_system_so_startup_task.cpp:78-113）与 `startup_manager.cpp` 的 `systemPreloadHintStartupTasks`/`systemPreloadSoAllowList`（:62-64, :713, :803）；本次为**新增预测驱动**的预加载任务，不改变现有静态链路语义 |
| 外部依赖（musl） | `third_party/musl/include/linux/dlfcn_ext.h:96-101`、`libc.map.txt:568` | `int dlopen_preload(const char *file, int mode)`，首版文档注释限定 `RTLD_NOW \| RTLD_LOCAL`，只做 load/link 不跑构造函数，后续普通 dlopen 自动 finalize |
| 外部依赖分析报告 | `third_party/musl/docs/dlopen-analysis/report.md` | dlopen_preload 语义、限制与失败回退矩阵（report.md:84-95） |

## 需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.2（v1.1 已批准；本次因"按模块区分 + 主进程冷启动门控 + 触发点下移"重新基线） |
| 基线日期 | 2026-10-16 |
| Owner | yangzk |
| 确认人 | 需求方 / 模块 Owner：yangzk |
| 确认依据 | 需求方明确批准 v1.2 修订（2026-10-16 会话："重新批准 + amend提交"） |
| 复杂度 | 标准（双仓特性：ability_runtime + napi；进程内实现，无 IPC；新增 InnerAPI，非 Public API） |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.1 |
| 版本状态 | proposed |
| 基线结论 | 通过 |

> 变更记录：v1.0 于 2026-10-15 获批；v1.1 确认 S1/S3 接口修订；v1.2（2026-10-16）确认按模块区分采集/预加载、仅主进程冷启动门控（排除预加载模式与 debug）、触发点移到 `RunNativeStartupTask` 之后（静态预加载先行）。

### 问题、目标与成功指标

**问题陈述：** 启动阶段应用实际 import 的 native 系统模块集合对系统不可见，启动框架只能按静态配置预加载，无法随应用真实加载行为自我演化，导致冷启动阶段系统 so 加载耗时无法针对性优化。

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| G1 采集启动阶段系统 native 模块 | 采集窗口内显式 import 的系统模块名被记录，并在窗口结束后落盘到模块级沙箱目录（haps/<moduleName>/cache）下的本特性子目录 | 单测 + 真机日志/落盘校验 |
| G2 跨启动预测 | 基于 EWMA 生成 predicted 预测集并原子写盘，下次启动可读取上一次预测结果 | 单测 + 真机文件校验 |
| G3 启动框架预测预加载 | 存在有效预测文件时，非主线程对 predicted 系统模块调用 NativeModuleManager 的系统模块预加载接口（内部走 `dlopen_preload`），真实 import 时可复用已 load/link 的 so | 单测 + 真机冷启动日志 |
| G4 不劣化启动稳定性 | 预测文件缺失/损坏/不可写、单模块预加载失败均不影响应用启动，不崩溃 | 单测（异常路径）+ 真机验证 |
| G5 多实例并发安全 | 同应用多实例并发冷启动时预测结果合并、无丢失、任一实例不因并发失败 | 单测（并发合并）+ 真机 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统开发者，我想要采集应用冷启动阶段显式 import 的系统 native 模块并落盘，以便后续做依赖分析与启动优化 | P1 |
| US-2 | 作为系统，我想要基于历史与本次采集用 EWMA 预测下次启动将加载的系统模块，以便提前预加载 | P1 |
| US-3 | 作为启动框架，我想要在应用下次冷启动时读取预测集并预加载对应系统 so，以便缩短真实 import 的加载耗时 | P1 |
| US-4 | 作为最终用户，我想要预加载失败或预测文件异常时应用启动不受影响 | P0 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 应用主进程冷启动（非预加载模式、非 debug）且系统参数开关判定设备支持本能力 THEN 采集窗口（`HandleLaunchApplication` 中 `RunNativeStartupTask` 之后触发开始，约 5 秒结束）内被显式 import 的系统 native 模块（仅系统模块，记录模块名）被采集并写入该模块的模块级沙箱目录（`<appBase>/haps/<moduleName>/cache`）下的本特性子目录 | 正常 | US-1 |
| AC-2 | WHEN 采集窗口结束 THEN 采集结果与历史评分按 EWMA 合并，输出 predicted 预测集，并以 tmp+rename 原子写盘到该模块的模块级沙箱目录下的本特性子目录，文件名为复杂唯一命名（不与开发者文件冲突） | 正常 | US-1,US-2 |
| AC-3 | WHEN 应用下次冷启动且存在有效预测文件 THEN app_startup 的预测预加载任务在非主线程对 predicted 中系统模块调用 NativeModuleManager 的系统模块预加载接口完成预加载（load/link，不执行构造函数、不阻塞主线程） | 正常 | US-3 |
| AC-4 | WHEN 本轮已对某系统模块执行了预加载 THEN 该模块不因其被预加载而出现在本轮已加载系统模块采集结果中（防自反馈：预加载走不注册模块的 `dlopen_preload`） | 正常 | US-2 |
| AC-5 | WHEN 系统参数开关关闭、设备类型不在支持集（PC/手机/平板默认支持，其他默认不支持）、或非主进程/应用预加载模式/debug 应用 THEN 不执行采集、不写盘、不预加载，应用行为与变更前一致 | 边界 | US-4 |
| AC-6 | WHEN 预测文件或采集文件不存在、内容损坏、字段非法或写盘失败 THEN 应用启动流程不受影响、不崩溃；读取失败时降级为不预加载，写入失败时保留旧文件或跳过本次更新 | 异常 | US-4 |
| AC-7 | WHEN predicted 中某单个系统模块预加载失败（路径失效/依赖缺失等）THEN 记录故障事件并继续处理其余模块，不影响应用启动 | 异常 | US-3,US-4 |
| AC-8 | WHEN ability_runtime 查询已加载 native 模块 THEN napi `NativeModuleManager` 提供的查询 InnerAPI 仅返回已加载的**系统**模块名（不含 `isAppModule`、不含 SO 路径），且不改变现有加载行为 | 正常 | US-1 |
| AC-9 | WHEN 同一应用多个实例并发冷启动 THEN 各实例的采集结果在预测文件中合并（并发命中不丢失、采集为各实例并集），任一实例不因并发写失败、不影响启动 | 边界 | US-1,US-2 |
| AC-10 | WHEN 执行 ability_runtime 相关单元测试 THEN 覆盖采集聚合、EWMA 预测与写盘、原子写盘、预测读取、防自反馈、文件异常降级、单模块预加载失败、设备开关关闭、多实例并发等路径，且全部通过 | 正常 | US-1,US-2,US-3,US-4 |

### 范围边界

**包含：**
- ability_runtime（`frameworks/native/appkit`）：
  - 新增 NativeModuleCollector：在采集窗口内采集显式 import 的系统 native 模块（模块名）并聚合，基于 EWMA 生成预测集，tmp+rename 原子写盘到模块级沙箱目录下的本特性子目录，含防自反馈与多实例并发合并。
  - 在 `main_thread.cpp` 的采集窗口起止点接入（开始=`LoadExtStartupTasks`，结束=`HandleLaunchApplication` 完成后 5 秒）。
  - app_startup 新增「预测预加载」启动任务：读取 predicted 系统模块名，在非主线程调用 NativeModuleManager 的系统模块预加载接口；单模块失败上报故障事件并续跑。
  - 新增系统参数开关（初定 `const.abilityruntime.nativepreload.enable`，最终名以 design 为准）控制设备差异化。
  - 新增/更新 BUILD.gn。
- napi（`foundation/arkui/napi/module_manager`）：
  - `NativeModuleManager` 新增「已加载系统模块查询」InnerAPI（仅返回系统模块名）。
  - `NativeModuleManager` 新增「系统模块预加载」InnerAPI（内部拼接系统 SO 路径并调用 `dlopen_preload`）。
- 复用 third_party/musl 的 `dlopen_preload`（不改 musl 代码；mode 待与 musl 对齐）。

**不包含：**
- 修改 third_party/musl 源码、`dlopen_preload` 语义或其支持的 mode（musl 为已提供的外部依赖）。
- 修改 `LoadNativeModule` 既有行为/签名（预加载逻辑下沉到 `NativeModuleManager` 的新增接口内，不改既有加载路径）。
- 修改现有静态白名单预加载链路（`PreloadSoStartupTask`/`PreloadSystemSoStartupTask`/`systemPreloadSoAllowList`）的既有语义。
- 新增或修改公共 SDK / Public / System / JS / NAPI / ANI 对外接口。
- 修改 `hisysevent.yaml` 中已发布事件的参数或删除事件。
- 其他设备类型（非 PC/手机/平板）的适配。
- 完整 XTS 用例与系统级性能指标达标（性能仅作观测，不作为本需求强制门禁）。

| 阶段（如适用） | 范围 | 不做 | 进入条件 |
|----------------|------|------|----------|
| 骨架/完整 | 采集 + 预测 + 预加载完整功能链路 | 预测算法调优/自适应参数学习 | proposal 批准后按 spec/design/plan 推进 |

### Agent Scope Guard

> 只约束需求探索和上下文收集阶段可访问的仓库、模块与知识源，不维护实现文件清单。发现需要越界时必须停止并重新确认基线；精确代码文件范围由 execution-plan.md / task.md 持有。

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | `foundation/ability/ability_runtime`、`foundation/arkui/napi`、`third_party/musl`（只读参考其 dlopen 能力与设计报告） | 需要访问未列仓库 | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | ability_runtime：`frameworks/native/appkit/`（`app`、`app_startup`）、`interfaces/kits/native/appkit/`、`services/abilitymgr/`（仅在确需准入校验时只读参考）、`hisysevent.yaml`、`BUILD.gn`/`*.gni`；napi：`module_manager/` | 需要扩大模块/目录 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | 修改 `third_party/musl` 源码；修改公共 API 签名/错误码数值；修改 `hisysevent.yaml` 已发布事件定义；私有凭据/无关工作区 | 命中禁止项 | 标记 Blocked，不读取、不修改 |
| 外部网络/私有知识源 | 仅限本机源码 + `third_party/musl/docs/dlopen-analysis/report.md`；无外部网络检索需求 | 需要新增权限或未授权来源 | 请求人工授权；未授权前不得使用 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| 元能力运行时 | ability_ability_runtime | `frameworks/native/appkit/app_main`（main_thread） | 应用启动主流程与启动任务 | 新增（挂钩采集窗口起止） | yangzk |
| 元能力运行时 | ability_ability_runtime | `frameworks/native/appkit/app_startup` | 启动框架/启动任务编排与 so 预加载 | 新增（NativeModuleCollector + 预测预加载任务 + 系统参数开关） | yangzk |
| 元能力运行时 | ability_ability_runtime | `BUILD.gn` | 构建 | 修改/新增（新源文件） | yangzk |
| NAPI 框架 | arkui_napi | `module_manager/native_module_manager.{h,cpp}` | native 模块注册/加载与模块链表维护 | 新增（已加载系统模块查询 InnerAPI；系统模块预加载 InnerAPI） | yangzk |
| C 库 | third_party_musl | `include/linux/dlfcn_ext.h`、ldso | 动态链接器与 `dlopen_preload` | 无（仅消费，不改动） | 外部依赖 |

### API 变更项清单

> 涉及 API 变更时必填，作为 design.md 与 spec.md 的共享锚点。

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| `NativeModuleManager::GetLoadedSystemNativeModules`（初定名，最终以 design 为准） | 新增 | InnerAPI | 仅返回当前进程已加载的**系统**模块名集合；不含 `isAppModule`、不含 SO 路径；不改变既有加载行为 |
| `NativeModuleManager::PreloadSystemNativeModule`（初定名，最终以 design 为准） | 新增 | InnerAPI | 内部拼接系统 SO 路径并调用 `dlopen_preload` 完成系统模块预加载（load/link，不注册）；供启动框架预测预加载消费 |
| 复用 `dlopen_preload`（musl 已导出） | 复用（不新增） | System（libc 导出符号，已存在） | 由 `NativeModuleManager::PreloadSystemNativeModule` 内部调用；mode 待与 musl 对齐 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | 需求方明确"本需求仅承载功能，性能功耗指标不涉及"；预加载耗时为观测项不作为门禁 | design.md（观测点） |
| 安全与权限 | 否 | 仅读写应用自身模块级沙箱目录与预加载系统 so，无跨应用/跨用户/权限授予行为 | design.md（边界确认） |
| 兼容性 | 否 | 需求方明确"不影响应用兼容性"；不修改公共 API 与既有加载行为，失败降级 | spec.md（降级口径） |
| API/SDK | 是 | napi 新增 2 个 InnerAPI（已加载系统模块查询、系统模块预加载）；不涉及 Public/System SDK 对外接口 | design.md / spec.md |
| IPC/跨进程 | 否 | 采集、预测、预加载均在应用进程内完成，无新增 IPC（多实例并发经文件锁协调，非 IPC） | design.md |
| 构建与部件 | 是 | 新增源文件，需更新 ability_runtime 与 napi 的 BUILD.gn/源文件列表；napi 侧需链接 musl `dlopen_preload` | design.md |
| 国际化/无障碍 | 否 | 无 UI、无文案 | — |
| 数据迁移 | 否 | 新增本地采集/预测文件（模块级沙箱子目录），无既有数据迁移；文件含版本字段以支持演进与容错（细节在 design） | design.md |

### 依赖、风险与外部约束

| 依赖/风险 | 类型 | 影响 | 缓解措施/Owner | 状态 |
|-----------|------|------|----------------|------|
| musl `dlopen_preload` 能力 | 外部依赖 | 高 | 已由 musl 最新提交提供（dlfcn_ext.h:96-101 / libc.map.txt:568）；仅消费不改动 | 已确认 |
| `dlopen_preload` 的 mode 选择（`RTLD_NOW` vs `RTLD_LAZY`） | 外部依赖/技术 | 中 | 需与 musl 团队讨论后确定；首版 musl 文档注释限定 `RTLD_NOW \| RTLD_LOCAL`；**本轮不写死** | 待确认（与 musl 对齐） |
| 应用真实 import 的 mode 与预加载 mode 不一致 | 技术 | 中 | 过渡期依赖 dev/ino 去重复用已映射的 so（musl report.md:224-243）；mode 收口后消除 | 已确认（纳入） |
| napi 新增 2 个 InnerAPI 与 ability_runtime 的接口契约 | 技术 | 中 | 本需求一并交付两仓，接口在 design 阶段冻结，双方按同一契约 | 待确认 |
| 多实例并发写预测文件 | 技术 | 中 | 模块级沙箱子目录 + 跨进程互斥 + 原子提交；持有互斥失败则该轮跳过落盘，不影响启动（AC-9） | 已确认（纳入） |
| 静态白名单预加载（走 `LoadNativeModule`）会被计入已加载系统模块 | 技术 | 低 | 已评估：allowlist 默认可为空，影响面有限；本轮不引入打标机制（已与需求方确认可接受） | 已确认（接受） |
| SO 路径跨版本/升级失效导致预加载失败 | 运行 | 中 | 预加载前校验路径存在性；单模块失败续跑并上报（AC-7） | 待确认 |
| EWMA 参数（α、阈值、预测集上限）缺乏既有规范 | 技术 | 中 | 在 design/spec 明确参数与初始值，先保功能正确，后续可调 | 待确认 |
| 采集窗口与进程生命周期竞争（窗口内进程退出/后台） | 运行 | 中 | 写盘在窗口结束异步执行，失败静默降级；不阻塞启动 | 待确认 |
| 5 秒结束时机为暂定值 | 技术 | 低 | 作为可配置/常量，待真机数据校准；变更走变更控制 | 待确认 |
| 系统参数开关命名与默认值 | 技术 | 低 | design 定名并同步产品配置；默认关闭需显式开启（待确认默认值） | 待确认 |

### 依据与上下文引用

| 来源 | 查询/读取内容 | 基线采用的结论 | 可信度 | 证据路径/链接 |
|------|---------------|----------------|--------|---------------|
| 源码 | napi `NativeModule` 结构与已加载模块落点 | 已加载模块可从模块链表读取；系统模块可经新增 InnerAPI 以模块名枚举 | 高 | `native_module_manager.h:58-76`、`native_module_manager.cpp:1517-1601` |
| 源码 | napi 系统路径解析 | `NativeModuleManager` 已具备系统模块路径解析能力（`GetModuleFileName`/`GetNativeModulePath`），可用于内部拼接路径 | 高 | `native_module_manager.cpp:294-322,1023-1140` |
| 源码 | ability_runtime 启动主流程 | 采集窗口可挂在 `LoadExtStartupTasks`（开始）与 `HandleLaunchApplication`（结束） | 高 | `main_thread.cpp:1659,1862,2174,4586` |
| 源码 | app_startup 既有 so 预加载 | 已有静态白名单预加载链路可作模式参考，本次新增预测驱动任务 | 高 | `preload_system_so_startup_task.cpp:78-113`、`startup_manager.cpp:62-64,713,803` |
| 源码/文档 | musl 预加载 API | `dlopen_preload(file, mode)` 首版注释限定 `RTLD_NOW\|RTLD_LOCAL`，只 load/link；mode 待对齐 | 高 | `third_party/musl/include/linux/dlfcn_ext.h:96-101` |
| 文档 | musl dlopen 优化分析报告 | 预加载收益依赖真实重叠窗口；mode 不匹配时回退/残留风险与 dev/ino 去重口径 | 中 | `third_party/musl/docs/dlopen-analysis/report.md:190-193,224-243` |
| Agent 指南 | `AGENTS.md`（ability_runtime） | 公共 API/IPC/持久化格式为兼容性红线；新增源文件必须更新 BUILD.gn；DFX 事件不可破坏 | 高 | `AGENTS.md` §3 |

### 变更控制与下游条件

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和影响范围，重新确认基线 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API 或 InnerAPI 契约 | 触发设计审批；本需求 v1.1 的 API 修订已触发重新基线 |
| Agent 探索范围变更 | 需要访问未列仓库/模块、禁止项或新知识源 | 停止探索，更新 Agent Scope Guard 后重新确认基线 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认验证计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

- [ ] 所有 P0/P1 用户故事有可测试、可度量的 AC
- [ ] 范围内外、影响范围、API 变化和不涉及项已确认
- [ ] `proposal.target_release` 已确认（OpenHarmony-7.1）
- [ ] `manifest.profile` 已确认（none）
- [ ] Agent Scope Guard 已基线化，未复制 Plan/Task 的实现文件清单
- [x] 基线结论为"通过"，确认人（yangzk）和确认依据完整
- [x] 条件附录已启用，待澄清项和决策已关闭

## 附录：澄清与决策记录（条件启用）

> 触发原因：跨两仓（ability_runtime + napi）协同 + 外部依赖（musl）+ 新增跨启动持久化，满足"多仓/外部依赖"条件。

### 触发与待澄清问题

| 项目 | 内容 | 验证方式/依据 | 状态 |
|------|------|---------------|------|
| 初始假设 | napi 已加载模块可枚举系统模块名 | 源码 `native_module_manager.cpp:1517-1601` | 已验证 |
| 初始假设 | musl `dlopen_preload` 已可用 | `dlfcn_ext.h:96-101`、`libc.map.txt:568` | 已验证 |
| 复杂度触发 | 两仓 + 外部依赖 + 新增 InnerAPI/持久化文件 | 需求描述 + 源码检索 | 已确认 |

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 交付仓库边界 | 决定影响范围与提交策略 | 已澄清 |
| Q-2 | 采集对象与标识 | 决定预测集能否驱动预加载 | 已澄清（v1.1 修订） |
| Q-3 | 预加载执行路径 | 决定 napi 接口形态 | 已澄清（v1.1 修订） |
| Q-4 | 设备差异系统参数 | 决定 1+8 差异实现方式 | 已澄清 |
| Q-5 | 采集起止时机 | 决定挂钩点 | 已澄清 |
| Q-6 | 写盘位置与命名 | 决定 cacheDir 语义与防冲突 | 已澄清 |
| Q-7 | 验收/验证方式 | 决定 AC 测试入口 | 已澄清 |
| Q-8 | 多实例并发 / preload mode 不匹配 | 边界与外部依赖风险 | 已澄清（v1.1 纳入） |

### 讨论与方案决策

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-10-15 | 需求方（RR-30096301）/ yangzk | 交付仓库边界 | Q-1：**ability_runtime + napi 两仓**；musl 仅消费不改 | design 冻结接口契约 |
| 2026-10-15 | 需求方 / yangzk | 采集对象与标识（v1.1 修订） | Q-2：**仅系统模块；查询接口仅返回模块名（不含 isAppModule、不含 SO 路径）** | design 定字段 |
| 2026-10-15 | 需求方 / yangzk | 预加载执行路径（v1.1 修订） | Q-3：**预加载下沉到 `NativeModuleManager`**：新增系统模块预加载接口，内部拼接系统路径 + `dlopen_preload`；app_startup 只调用该接口 | design 定接口与路径拼接 |
| 2026-10-15 | 需求方 / yangzk | 设备差异系统参数 | Q-4：**新增专用开关参数**（初定 `const.abilityruntime.nativepreload.enable`） | design/spec 定名与默认值 |
| 2026-10-15 | 需求方 / yangzk | 采集起止时机 | Q-5：**确认按暂定**（开始 `LoadExtStartupTasks`，结束 `HandleLaunchApplication` 完成后 5 秒） | design 定线程与可配置性 |
| 2026-10-15 | 需求方 / yangzk | 写盘位置与命名 | Q-6：**cacheDir 下子目录**，复杂唯一命名 + tmp/rename 原子写 | design 定路径拼接与文件名 |
| 2026-10-15 | 需求方 / yangzk | 验收/验证方式 | Q-7：**ability_runtime 单测 + 真机冷启动日志/落盘校验** | plan 列测试任务 |
| 2026-10-15 | 需求方 / yangzk | 多实例并发 / preload mode（v1.1 纳入） | Q-8：多实例并发写合并**纳入**；mode 不匹配风险**纳入**且 mode **暂不定死**，待与 musl 讨论 | design/spec 落 AC 与开放项 |

| 方案 | 核心思路 | 优势 | 风险/代价 | 结论 |
|------|----------|------|-----------|------|
| A-1 复用 `LoadNativeModule` 预加载 | 让 `LoadNativeModule` 内部改走 `dlopen_preload` | 复用路径解析与缓存 | 改动既有加载行为，影响面大 | 放弃 |
| A-2 app_startup 直接 `dlopen_preload` | 启动框架解析 SO 路径后直接调用 musl 预加载 | 失败隔离 | 路径解析外露、跨仓职责不清 | 放弃（v1.1 修订） |
| A-3 预加载下沉 NativeModuleManager（v1.1 采纳） | `NativeModuleManager` 提供系统模块预加载接口，内部拼接系统路径 + `dlopen_preload` | 路径拼接内聚、职责清晰、app_startup 只调用 | 需在 napi 侧链接 musl `dlopen_preload` | 推荐 |

### 检索与影响分析

| 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/未查询原因 |
|------|---------------|----------|--------|------|-----------------|
| 源码 | napi 已加载模块字段 | 系统模块可枚举（模块名）；`systemFilePath`/`isAppModule` 可作为内部实现细节 | 高 | API/范围 | 命中 |
| 源码 | app_startup 既有预加载 | 静态白名单链路存在，本次为新增预测链路 | 高 | 范围/影响 | 命中 |
| 源码 | napi 路径解析能力 | `GetModuleFileName`/`GetNativeModulePath` 可支撑内部拼接路径 | 高 | 设计 | 命中 |
| 文档 | musl report.md | 预加载收益依赖重叠窗口；mode 不匹配回退口径 | 中 | 风险/非目标 | 命中 |
| Agent 指南 | `AGENTS.md` | 公共 API/IPC/持久化/DFX 红线 | 高 | 范围/约束 | 命中 |

| 维度 | 澄清结论 | 确认人 | 状态 |
|------|----------|--------|------|
| 子系统与仓 | ability_runtime + napi（musl 只消费） | 需求方/yangzk | 已确认 |
| API 与权限 | 新增 2 个 napi InnerAPI；无新增权限 | 需求方/yangzk | 已确认 |
| 兼容性与非功能 | 不影响应用兼容性；性能仅观测；mode 待 musl 对齐 | 需求方/yangzk | 已确认 |
| 依赖与风险 | musl 外部依赖已提供；mode 待定；EWMA 参数设计阶段定 | 需求方/yangzk | 已确认 |

### 澄清结论

- [x] 待澄清问题均已关闭或明确 N/A
- [x] 范围、API、兼容性、非功能需求、依赖和风险已形成基线结论
- [x] 多仓协同已记录方案取舍与需求方确认
- [x] P0/P1 AC 覆盖正常、异常和边界路径

**结论：** 通过

> 基线 v1.2（按模块拆分至模块级沙箱 `haps/<moduleName>/cache`、仅主进程冷启动门控、触发点移到 `RunNativeStartupTask` 之后、防碰撞命名）已更新，`phase_status: approved`，待需求方/Owner 重新批准 Define 后进入 Design。
