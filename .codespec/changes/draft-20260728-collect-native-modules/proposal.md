---
target_release:
  id: OpenHarmony-7.1-Release
  status: proposed
---

# 需求文档

> 一份文档，从原始需求到基线结论。按阶段追加内容，不拆成多份独立文件。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | draft-20260728（未关联 issue） |
| 需求名称 | 启动阶段采集应用 import 的 Native 模块并保存至应用沙箱 |
| 来源 | 需求方直接提出 |
| 提出人 | yangzk |
| 目标发行版本 | OpenHarmony-7.1-Release（需求方 2026-07-28 确认） |
| 候选 Profile | none（ability_runtime 未命中 arkweb/arkui/arkgraphic/arkdata 任一 repos 声明） |
| 优先级 | P1（需求方 2026-07-28 确认） |
| 状态 | Clarifying → 澄清完成，待基线 |

### 原始描述

**原始问题：** （保留需求方原话，不做技术翻译）

> "在启动阶段，通过NativeModuleManager将应用import过的模块采集并保存到应用沙箱"
>
> "我们的native_modules.json改为保存到cacheDir下，文件名修改为更加复杂一些，避免和开发者的文件重复。以及考虑更新情况：每次启动不一定一致，需要根据一定策略更新，尽可能准确预测下一次启动会加载的模块。"

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 系统开发者 | NativeModuleManager 内部已记录应用 import 的全部 Native 模块，但链表为私有实现、无对外枚举接口，外部无法获知应用实际加载的模块集合 | 模块依赖不可视，无法支撑依赖分析、治理与启动优化 |
| 系统开发者 | 运行时无将模块信息保存至应用沙箱的既有通路 | 采集结果无法持久化、无法跨启动利用 |

**期望结果：** 应用启动阶段自动采集本进程 import 过的 Native 模块并落盘到应用沙箱 cacheDir；文件名具备系统托管命名空间，不与开发者文件冲突；文件内容跨启动按策略更新，尽可能准确预测下一次启动会加载的模块集合。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码事实 | `foundation/arkui/napi/module_manager/native_module_manager.h` | NativeModuleManager 单例维护模块链表（head/tailNativeModule_ 私有），NativeModule 节点含 name/moduleName/fileName/version/isAppModule/moduleLoaded |
| 源码事实 | `ability_runtime/frameworks/native/runtime/js_runtime.cpp:603,739` | requireNapi 绑定全局对象，import 汇聚至 ArkNativeEngine::LoadNativeModule |
| 源码事实 | `ability_runtime/frameworks/native/appkit/ability_runtime/context/context_impl.cpp:167` | GetCacheDir() 现成接口 |
| 前期分析 | `docs/sdd/collect_imported_native_modules/sdd.md`（v2） | 本流程之前的初步分析文档，作为方案探索输入 |

### 初始范围

**可能包含：**
- napi 仓新增 NativeModuleManager 已加载模块枚举接口（InnerAPI）
- ability_runtime 新增 NativeModuleCollector：双检查点采样、跨启动 EWMA 评分合并、原子写盘
- **启动早期读取消费路径**：启动框架预加载系统 so（app_startup 预加载任务）读取 predicted 中系统模块并预加载（Q-1 确认，本期范围）
- preload/fork 区分机制：仅统计 fork 后应用显式 import 的模块（Q-4 确认）；同时需区分消费方预加载 vs 应用真实 import（防自反馈回路，见风险表）
- 防冲突文件命名（隐藏点目录 + 进程哈希 + 格式版本）
- 系统 CCM 参数开关门控，缺省关闭（Q-2 确认）
- TDD 单元测试

**明确不包含：**
- 按启动场景分桶的多 profile 预测（列为演进方向）
- 系统侧集中汇总分析（无 IPC、无跨进程上报）
- Public/System API 新增
- 修改模块加载路径本身

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| NativeModuleManager 无对外枚举接口，需 napi 仓新增 | 技术 | 源码（native_module_manager.h:83-201） | 已验证 |
| 所有 import（系统+应用模块）汇聚于 NativeModuleManager::LoadNativeModule 单点 | 技术 | 源码（ark_native_engine.cpp 多处调用点） | 已验证 |
| 沙箱属应用自身，进程内写 cacheDir 无权限障碍、无需 IPC | 技术 | 源码（context_impl.cpp:167；DumpRuntimeHelper 既有先例） | 已验证 |
| 模块为按需懒加载，单检查点（PerformAppReady）会漏采 UI 相关模块 | 技术 | 启动流程源码分析 | 已验证（结论：需双检查点） |
| 每轮启动加载集合存在抖动，单快照不足以预测下轮 | 业务 | 需求方原话"每次启动不一定一致" | 已验证（需求方确认） |
| preload/fork 预加载模块不计入采集，需 PreFork 快照区分 | 技术 | 需求方 2026-07-28 确认（Q-4） | 已验证（需求方确认） |
| 消费方为启动框架预加载系统 so（app_startup PreloadSystemSoStartupTask 体系，非主线程执行，LoadNativeModule 加载系统模块） | 技术 | 源码（preload_system_so_startup_task.cpp:77-115）+ 需求方 2026-07-28 确认 | 已验证 |
| NativeModule.refCount 可区分"消费方预加载"与"应用真实 import"（cache 命中递增语义） | 技术 | napi 仓确认 refCount 语义（native_module_manager.h:68 存在该字段） | 待验证 |
| napi 仓 owner 接受新增 GetLoadedModuleInfos 枚举接口 | 外部依赖 | Owner 确认 | 待验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 改动面小且路径清晰，但跨 2 仓、涉启动性能路径，需完整澄清与方案探索 |
| 涉及仓数量 | 2 | ability_runtime（采集器+检查点接入）、arkui/napi（枚举接口） |
| 是否涉及 Public/System API | 否 | 仅新增 InnerAPI（napi 仓 native_module_manager.h，NAPI_EXPORT 内部接口） |
| 是否涉及安全/性能关键路径 | 是 | 应用启动主路径；以"内存采样 <1ms + 每轮 1 次异步写盘 + 开关默认关闭"规避 |
| 是否跨 SIG | 待确认 | ability_runtime 与 arkui/napi 的 SIG 归属待 Owner 确认 |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定（标准）

---

## 二、澄清记录

> 澄清是逐轮对话，不是一次性填表。结论确认后写入本记录。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 预测结果是否有本期消费方（如启动预加载），还是仅采集落盘？ | 决定是否需要设计读取接口与文件格式稳定性承诺 | 已澄清：**本期同时提供读取消费，消费方为启动框架预加载系统 so**——启动早期由 app_startup 预加载任务体系（现有 `PreloadSystemSoStartupTask`）读取 predicted 中系统模块（isAppModule=false）并执行 `LoadNativeModule` 预加载；属范围新增，已重评复杂度（仍为标准） |
| Q-2 | 开关默认策略？ | 影响量产版本风险评估与发布形态 | 已澄清：**走系统 CCM 参数控制；参数未下发/缺失时缺省关闭**（两轮回答合并） |
| Q-3 | EWMA 评分策略是否认可为推荐方向？ | 更新策略是需求核心，参数需需求方确认可调优 | 已澄清：**认可 EWMA 方案**（α=0.4、阈值 0.5、16 轮淘汰、升级重建，参数支持调优） |
| Q-4 | preload/fork 预加载的系统模块是否计入采集？ | 语义影响预测集内容与文件体积 | 已澄清：**不计入 preload 模块**，仅统计 fork 后应用显式 import 的模块；设计需 PreFork 快照区分 |
| Q-5 | 目标发行版本？ | proposal.target_release 基线必填 | 已澄清：**OpenHarmony-7.1-Release**（需求方回答"penHarmony-7.1"，按此标准名记录） |
| Q-6 | 需求优先级？是否有对应 issue 编号？ | 基线信息与目录命名 | 已澄清：**P1**；暂无 issue，保持 draft 目录，关联后重命名为 issue-\<number\>-\<slug\> |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-07-27 | 需求方(yangzk) | 保存位置与文件命名 | 保存到 cacheDir；文件名复杂化避免与开发者文件重复 | 已纳入方案 |
| 2026-07-27 | 需求方(yangzk) | 更新策略 | 每次启动不一定一致，需按策略更新，尽可能准确预测下一次启动加载的模块 | 形成 EWMA 方案 |
| 2026-07-28 | AI 分析 | 现状源码勘察 | NativeModuleManager 无枚举接口；import 单点汇聚；cacheDir 接口现成 | 见 K-1~K-4 |
| 2026-07-28 | 需求方(yangzk) | 澄清第 1 轮（Q-1~Q-4） | Q-1 本期含读取消费（范围新增，触发变更控制重评）；Q-2 系统 CCM 参数；Q-3 认可 EWMA；Q-4 不计入 preload 模块 | 回写 proposal；设计需含读取路径与 PreFork 快照区分 |
| 2026-07-28 | 需求方(yangzk) | 澄清第 2 轮（Q-5/Q-6 + Q-2 缺省跟进） | Q-5 OpenHarmony-7.1-Release；Q-6 P1、暂无 issue 保持 draft；CCM 参数缺失时缺省关闭 | 回写 proposal，待 /ohos-baseline 冻结 |
| 2026-07-28 | 需求方(yangzk) | Q-1 消费方具体化 | 读取消费方 = **启动框架预加载系统 so**（app_startup 现有 PreloadSystemSoStartupTask 体系）；预加载对象限定 predicted 中系统模块（isAppModule=false） | 回写 proposal；新增"自反馈回路"风险与 refCount 区分假设，入 design |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | 模块枚举接口（napi 仓）；采集器：双检查点采样、EWMA 评分合并、原子写盘；**读取消费：启动框架预加载系统 so（app_startup 预加载任务读 predicted 中系统模块并预加载）**；preload 模块排除（PreFork 快照区分）；防冲突命名；CCM 参数开关（缺省关闭）；TDD | yangzk（2026-07-28） | 已确认 |
| 明确不包含哪些？ | 场景分桶多 profile；系统侧汇总（无 IPC）；Public/System API；改动加载路径 | yangzk（2026-07-28） | 已确认 |
| 是否有分期策略？ | 一期（本期）：采集 + 预测 + 读取消费；演进：场景分桶多 profile | yangzk（2026-07-28） | 已确认 |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | 检查点快照拉取：加载路径零改动，检查点经新枚举接口一次性拉取链表快照，异步合并写盘 | 零性能侵入；失败不影响启动；实现面小 | 依赖 napi 仓新增接口（跨仓） | 推荐 |
| A-2 | 加载路径逐条钩子：在 LoadNativeModule 每次调用时记录 | 无需枚举接口；可记录精确加载时序 | 热路径每次加载都有开销；记录时序本期无用；侵入加载路径 | 放弃 |
| A-3 | AMS/AppMgr 服务侧集中采集：应用进程 IPC 上报，服务侧统一落盘 | 可跨应用集中分析 | 需 IPC 与权限设计；写非自身沙箱有权限障碍；复杂度高、超出需求范围 | 放弃 |

**取舍理由：** 需求仅要求"采集并保存到应用沙箱"，A-1 以最小侵入满足；A-2 的热路径开销无对应收益；A-3 的集中分析超出明确不包含项。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 | `arkui/napi/module_manager/native_module_manager.h` | 模块链表私有、无枚举接口；NativeModule 字段齐全（moduleName/fileName/version/isAppModule/moduleLoaded） | 高 | 范围/设计 | 命中 |
| K-2 | 源码 | ability_runtime 启动流程（main_thread.cpp / application_impl.cpp / js_runtime.cpp / ets_runtime.cpp） | Runtime 在 framework 侧 HandleLaunchApplication 创建；PerformAppReady 为"启动完成"锚点；模块懒加载 | 高 | 设计 | 命中 |
| K-3 | 源码 | `context_impl.cpp:167`、`context_deal.cpp` | GetCacheDir/GetFilesDir 现成；el2 默认加密区；DumpRuntimeHelper 写 cache 先例 | 高 | 设计 | 命中 |
| K-4 | CLAUDE.md | ability_runtime 仓开发规范 | GN 构建、TDD 目录惯例（test/unittest）、HiLog 规范、错误码规范 | 高 | 设计/测试 | 命中 |
| K-5 | 官方文档/DeepWiki | 未查询 | — | — | — | 未查询：需求为仓内特性，源码事实已充分 |

**上下文结论：**
- 高可信结论：数据源天然存在且字段齐全；单点汇聚保证采集完备性；进程内写自身沙箱无需 IPC；双检查点可覆盖懒加载。
- 待确认结论：napi 仓 owner 对枚举接口的接受度（假设表）；跨 SIG 归属。
- 未使用来源及原因：多仓知识库/DeepWiki 未查询——两仓源码已直接可读，事实充分。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | 元能力子系统（ability_runtime）、ArkUI/NAPI（arkui/napi 仓 module_manager）；SIG 归属划分待 Owner 确认 | yangzk（澄清轮次范围确认） | 已确认（SIG 归属为遗留动作） |
| 是否需要新增子系统或部件？ | 否 | yangzk（2026-07-28） | 已确认 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | yangzk（2026-07-28） | 已确认 |
| 是否需要新增 System API？ | 否 | yangzk（2026-07-28） | 已确认 |
| 是否会废弃已有 API？ | 否 | yangzk（2026-07-28） | 已确认 |
| 是否需要新增权限声明？ | 否（进程内写自身沙箱） | yangzk（2026-07-28） | 已确认 |

注：新增 `NativeModuleManager::GetLoadedModuleInfos` 为 InnerAPI（C++ 内部接口），不属于 Public/System API 开放范围。

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | CCM 参数缺省关闭，缺省时行为与存量完全一致；文件格式带 version 字段，解析失败/版本不符丢弃重建，无破坏性变更 | yangzk（2026-07-28） | 已确认 |
| 性能 | 响应时间/内存/并发要求？ | 两次内存采样合计 <1ms；每轮启动至多 1 次异步写盘（KB~百KB 级）；不阻塞启动主路径；读取消费路径在启动早期同步读小文件（<100KB） | yangzk（2026-07-28） | 已确认 |
| 安全 | 权限/隐私/加密/审计要求？ | 仅写自身沙箱 cacheDir，文件 0600；内容为模块名/路径/版本等公开信息，无用户隐私 | yangzk（2026-07-28） | 已确认 |
| 可靠性 | 崩溃率/容错/恢复要求？ | tmp+rename 原子写；JSON 损坏/缓存清理/应用升级均兜底冷启动重建；采集失败不影响启动 | yangzk（2026-07-28） | 已确认 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| arkui/napi 仓合入 GetLoadedModuleInfos | 编译 | ability_runtime 侧编译依赖该头文件接口 | 待 Owner 对齐（最大风险） |
| 系统 CCM 参数定义与下发通道 | 运行 | 开关读取 CCM 参数，缺省关闭；参数名与下发机制待 design 明确 | 已确认机制，细节入 design |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| napi 仓 owner 不接受接口形态或排期不匹配 | 外部 | 高（阻塞） | 尽早对齐接口形态；备选：ability_runtime 侧先以弱符号/动态查询解耦（不推荐，仅兜底） | 待确认 |
| EWMA 对强场景分化应用预测准确度有限 | 技术 | 中 | beta 期命中率上报标定参数；演进方向：场景分桶 | 已识别 |
| **消费方预加载自反馈回路**：启动框架预加载的模块下一轮会出现在链表中，若计为"应用真实 import 命中"则预测自我强化、误判永不淘汰 | 技术 | 中 | design 候选方案：NativeModuleInfo 携带 refCount，以"refCount > 消费方预加载次数"判定真实 import 才计命中；消费方预加载集合本轮内存记录 | 已识别，入 design |
| 缓存清理导致预测冷启动 | 技术 | 低 | 1~2 轮内快速收敛；属 cache 语义可接受行为 | 已识别 |
| 多进程应用文件互相覆盖 | 技术 | 低 | 文件名含进程名哈希，各进程独立文件 | 已规避 |

### AC 完整性

- [x] 每个用户故事有验收标准（见基线章 US/AC 草案）
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确（Q-1：本期含读取消费路径）
- [x] 子系统影响已识别（SIG 归属为遗留 Owner 动作，不阻塞基线）
- [x] API 变更已评估（无 Public/System API 变更）
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案（跨仓依赖待 Owner 对齐，已列缓解）
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（A-1/A-2/A-3 + 取舍理由）

**结论:** 通过（Q-1 ~ Q-6 已全部关闭；待需求方总结确认后由 `/ohos-baseline` 冻结基线）

---

## 三、需求基线

> 澄清完成后固化。**当前状态：未基线**，待 Q-1 ~ Q-6 关闭并经需求方/Owner 确认后，通过 `/ohos-baseline` 冻结。

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0（未冻结） |
| 基线日期 | 待定 |
| Owner | yangzk |
| 确认人 | 待需求方总结确认 + /ohos-baseline |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.1-Release |
| 版本状态 | proposed |

### 问题陈述

应用运行期实际加载的 Native 模块集合对系统不可见：NativeModuleManager 虽有完整记录但无对外通路。需要在启动阶段低成本采集并持久化到应用沙箱，且文件内容能跨启动自我修正、收敛为对下一次启动加载集合的高置信预测，为后续依赖分析与启动优化提供数据基础。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 启动阶段完整采集 import 的 Native 模块 | 文件记录覆盖启动期间实际加载模块（双检查点） | TDD + 设备实测比对 /proc/self/maps |
| 文件跨启动收敛为准确预测 | beta 期命中率 ≥95%（\|predicted ∩ actual\| / \|actual\|） | HiLog/HiSysEvent 命中率上报 |
| 对启动性能零侵扰 | 主线程采样 <1ms；写盘异步且每轮 ≤1 次 | 启动耗时对比测试 |
| 不与开发者文件冲突 | 隐藏点目录 + 系统命名空间 + 进程哈希 + 版本后缀 | TDD 用例 FileName_NoCollision |

### 用户故事与 AC（草案，待基线确认）

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统开发者，我想要应用启动时自动采集 import 的 Native 模块落盘，以便获得模块依赖事实数据 | P0 |
| US-2 | 作为系统开发者，我想要文件按跨启动策略更新并给出下轮预测集，以便数据可直接用于启动优化 | P0 |
| US-3 | 作为系统开发者，我想要启动框架在启动早期读取 predicted 并预加载其中的系统 so，以便缩短应用真实 import 时的加载耗时 | P0 |
| US-4 | 作为应用开发者，我想要采集文件不与我的沙箱文件混淆、不侵犯隐私，以便无感共存 | P1 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 应用启动且 CCM 参数开启 THEN 在检查点②后将本轮 fork 后加载的模块合并写入 cacheDir/.ohos.ability_runtime/native_modules_<processHash8>.v2.json，每轮启动写盘不超过 1 次 | 正常 | US-1 |
| AC-2 | WHEN CCM 参数未下发或缺失 THEN 缺省关闭：不采样、不读写任何文件，启动行为与存量完全一致 | 正常 | US-1 |
| AC-3 | WHEN 模块连续多轮启动均被加载 THEN 其 score 收敛趋近 1 且稳定出现在 predicted 中 | 正常 | US-2 |
| AC-4 | WHEN 模块连续 16 轮未加载或 score < 0.05 THEN 该条目从文件中淘汰 | 边界 | US-2 |
| AC-5 | WHEN 旧文件 JSON 损坏或 version 不符或被缓存清理 THEN 按冷启动重建，不产生异常、不影响启动 | 异常 | US-2 |
| AC-6 | WHEN 应用升级（appVersionCode 变化） THEN 清空历史评分按冷启动重建 | 异常 | US-2 |
| AC-7 | WHEN 应用存在多个进程 THEN 各进程写各自 <processHash8> 文件，互不覆盖 | 边界 | US-1 |
| AC-8 | WHEN 写盘中途失败 THEN 正式文件不出现残缺内容（tmp+rename 原子性） | 异常 | US-1 |
| AC-9 | WHEN 启动早期且文件存在且 CCM 参数开启 THEN 启动框架预加载任务读取 predicted 中系统模块（isAppModule=false）并执行 LoadNativeModule 预加载；文件缺失/损坏/格式不符时跳过预加载且不影响启动 | 正常 | US-3 |
| AC-10 | WHEN preload/fork 阶段预加载的系统模块存在于进程链表中 THEN 采集与预测均不包含这些模块（仅统计 fork 后应用显式 import 的模块） | 边界 | US-1 |

### 范围边界

**包含：** napi 仓枚举接口；ability_runtime 采集器（双检查点采样、EWMA 合并、原子写）；读取消费：启动框架预加载系统 so（app_startup 预加载任务，限 predicted 中系统模块）；preload 模块排除机制；防冲突命名；CCM 参数开关（缺省关闭）；TDD。
**不包含：** 应用模块（isAppModule=true）的预加载（so 路径涉及 hap 内库，机制不同，如有需要单列演进）；场景分桶多 profile（演进方向）；系统侧集中汇总（无 IPC）；Public/System API；模块加载路径改动。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ArkUI/NAPI | arkui/napi | module_manager/native_module_manager.h/.cpp | Native 模块注册与加载管理 | 新增（枚举接口） | 待确认 |
| 元能力 | ability_runtime | frameworks/native/appkit/app/（新增 native_module_collector） | 应用主线程与启动流程 | 新增 | yangzk |
| 元能力 | ability_runtime | frameworks/native/appkit/app_startup/（读取消费：新增预测预加载任务或扩展 PreloadSystemSoStartupTask，具体形态待 design） | 启动任务编排、系统 so 预加载 | 修改 | yangzk |
| 元能力 | ability_runtime | frameworks/native/appkit/app/application_impl.cpp 等 | 启动检查点接入 | 修改 | yangzk |
| 元能力 | ability_runtime | test/unittest/native_module_collector_test/ | TDD | 新增 | yangzk |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| NativeModuleManager::GetLoadedModuleInfos | 新增 | InnerAPI | 持锁遍历模块链表，深拷贝快照输出 |
| NativeModuleInfo（struct） | 新增 | InnerAPI | 模块信息纯数据快照 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 是 | 启动主路径，需采样<1ms+异步写盘约束 | design.md / spec.md |
| 安全与权限 | 是 | 沙箱内文件 0600、隐藏目录、无隐私数据；无新增权限 | design.md / spec.md |
| 兼容性 | 是 | 开关默认关闭零影响；文件版本化、损坏兜底重建 | spec.md |
| API/SDK | 是 | 仅 InnerAPI 新增，无 Public/System API | design.md / spec.md |
| IPC/跨进程 | N/A | 进程内闭环，自写自身沙箱 | — |
| 构建与部件 | 是 | 新增源文件需更新 BUILD.gn；napi 为既有依赖 | design.md |
| 国际化/无障碍 | N/A | 无 UI、无文案 | — |
| 数据迁移 | 是 | 文件格式 version 演进与损坏/升级重建策略 | design.md / spec.md |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增消费方（如预加载）或场景分桶 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改采集时点、文件格式、淘汰阈值 | 重新审批基线和 Spec |
| API 变更 | 枚举接口形态变化 | 触发设计审批（跨仓对齐） |
| 非功能指标变更 | 性能/命中率阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC（US-1~US-4，AC-1~AC-10）
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认（Q-1：本期含读取消费路径）
- [x] `proposal.target_release` 已确认（OpenHarmony-7.1-Release，status: proposed）
- [x] `manifest.profile` 已确认（none，repo 未命中任何 profile repos 声明）
- [x] 涉及仓、模块已识别（SIG 归属为遗留 Owner 动作，不阻塞）
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认（Q-1 范围新增已按规则重评复杂度）
- [x] 标准及以上复杂度的澄清问题已逐项关闭（Q-1 ~ Q-6 全部已澄清）
- [x] 上下文与知识源检索日志已填写；未查询关键来源的原因已记录（K-5）
- [x] 目标仓 Agent 指南已检查并记录关键约束（CLAUDE.md，见 K-4）

**基线结论:** 待定（澄清已闭环；待需求方总结确认后由 /ohos-baseline 冻结）
