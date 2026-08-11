---
target_release:
  id: TBD
  status: proposed
---

# 需求文档：应用启动链路包信息重复查询/存储治理

> 一份文档，从原始需求到基线结论。按阶段追加内容，不拆成多份独立文件。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-DUP-BUNDLEINFO |
| 需求名称 | 应用启动过程中重复查询/存储包信息问题的分析与修复 |
| 来源 | 性能优化分析（启动路径） |
| 提出人 | 待定（Owner 确认） |
| 目标发行版本 | TBD |
| 候选 Profile | none |
| 优先级 | P1 |
| 状态 | Clarifying |

### 原始描述

**原始问题：** 分析应用启动过程中，存在哪些重复查询包信息、存储包信息，以及提供修复方案。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 最终用户 | 冷启动链路上对 BMS 的同步 IPC 查询 5~8 次、4 次携带完整包信息结构体的跨进程传输，拖慢应用启动 | 启动耗时增加 |
| 系统开发者 | 109 处 BMS 查询调用点全部直连 IPC、无统一缓存；去重手段仅为 thread_local 单次启动缓存与点状补丁 | 重复问题反复出现，维护成本高 |
| 系统开发者 | BundleInfo/HapModuleInfo 查到后被丢弃或拆散存储，后续需要任何字段只能重查 BMS | 重复 IPC 的根因无法消除 |

**期望结果：** 给出重复查询/存储点清单（含证据），并通过修复方案使单次冷启动的 BMS 同步查询次数与包信息拷贝份数可度量地下降，且行为语义不变。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码分析 | `services/abilitymgr/src/utils/start_ability_utils.cpp:45-46,263` | 既有 thread_local 单次启动缓存（StartAbilityInfoWrap），`QueryAbilityInfo` 为启动链路首个 BMS 查询 |
| 源码分析 | `services/abilitymgr/src/ability_manager_service.cpp:3657-3666` | 沙箱克隆特判缓存，注释明写"Cache the queried abilityInfo to avoid redundant BMS query"——点状补丁证明痛点已被感知 |
| 源码分析 | `services/abilitymgr/src/ability_record.cpp:359 → utils/main_element_utils.cpp:74` | LoadAbility 阶段对刚查过的 bundle 再做全量 `GetBundleInfoV9`，仅为判断 mainElement |
| 源码分析 | `services/appmgr/src/app_mgr_service_inner.cpp:2214,5780; module_running_record.cpp:392` | 一次冷启动 appmgr 侧对同一 bundle 至少 3 次 BMS IPC（GetBundleAndHapInfo×2 + attach 后 GetCloneBundleInfo） |
| 源码分析 | `services/appmgr/src/module_running_record.cpp`（Init/GetHapModuleInfo） | ModuleRunningRecord::Init 收到 HapModuleInfo 只存 moduleName_，之后每次重新向 BMS 查全量 BundleInfo——"手里有过又扔掉再查" |
| 源码分析 | `services/abilitymgr/include/ability_record/ability_request.h:58-59; ability_manager_service.cpp:9048` | 同一 ApplicationInfo 在一次启动中至少 4 份拷贝（thread_local 缓存、AbilityRequest::abilityInfo.applicationInfo、AbilityRequest::appInfo、AbilityRecord::abilityInfo_.applicationInfo） |
| 源码分析 | `frameworks/native/appkit/ability_bundle_manager_helper/bundle_mgr_helper.cpp` | BundleMgrHelper 只缓存代理对象，不缓存任何包数据；全仓无 uid↔bundleName、bundleName→BundleInfo 的服务端数据缓存 |
| 源码分析 | `services/abilitymgr/src/ability_manager_service.cpp:4207; ability_bundle_event_callback.cpp` | AMS 已订阅 BMS 包变化事件（用于杀进程/清 URI 权限），失效机制基础设施已具备但未用于数据缓存 |

### 初始范围

**可能包含：**
- 冷启动主链路（StartAbility → 拦截器链 → GenerateAbilityRequest → AppMgr LoadAbility/StartProcess → attach 后 AddAbilityStage）重复 BMS 查询的消除
- 包信息结构体（ApplicationInfo/AbilityInfo/HapModuleInfo/BundleInfo）在启动链路上的重复拷贝治理
- 高频小查询（uid↔bundleName）缓存
- 利用既有包事件订阅建立缓存失效机制

**明确不包含：**
- BMS（bundle_framework 仓）侧修改
- Public/System API 变更
- 热启动、后台调度、卸载/安装流程的行为变更
- 应用进程内（AppKit 侧）的包信息缓存

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| 单次冷启动可消减约 4~6 次 BMS 同步 IPC | 技术 | 源码证据 + 实施后打点统计 | 待验证（量化基线需实测） |
| 包事件回调（SubscribeBundleEventCallback）足以作为缓存失效触发 | 技术 | 源码确认事件覆盖面（安装/更新/卸载/分身） | 待验证 |
| 消除重复查询不改变任何行为语义（含权限校验时序） | 兼容性 | 单元测试 + 全量 ability_runtime_test 回归 | 待验证 |
| 启动期间包信息不变（安装/更新与启动互斥由 BMS 保证） | 技术 | Owner 确认 | 待验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单仓特性，无 API 变更，无跨 SIG；但涉及启动性能敏感路径，需专项非功能验证 |
| 涉及仓数量 | 1 | ability_runtime |
| 是否涉及 Public/System API | 否 | 仅 InnerAPI/进程内重构 |
| 是否涉及安全/性能关键路径 | 是（性能） | 应用冷启动路径 |
| 是否跨 SIG | 否 | 仅 applications/ability SIG |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确（Owner 待定）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断

---

## 二、澄清记录

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 本次交付边界：仅"分析+修复方案文档"，还是含代码实现？ | 决定 execution-plan 是否含代码任务、AC 是否含实测指标 | 已澄清：**文档 + 一期代码实现** |
| Q-2 | 优化策略选型：点状去重 / 统一进程内缓存 / 数据下沉复用，单选还是分期组合？ | 决定风险等级与设计规模 | 已澄清：**一期 A-1+A-2** |
| Q-3 | 是否允许修改 AMS↔AppMgr 的 InnerAPI IPC 参数（如 LoadAbility 携带包信息使 AppMgr 免重查）？ | InnerAPI 变更影响面与兼容性评估不同 | 已澄清：**允许一期纳入 A-4** |
| Q-4 | 是否接受引入带包事件失效订阅的进程内包信息缓存（服务端数据缓存从 0 到 1）？ | 缓存一致性是本需求最大技术风险 | 已澄清：**放弃 A-3**。AMS/AppMgr/BMS 同驻 foundation 进程（IN_PROCESS_CALL），BMS 侧（bundle_framework）自身已维护包数据缓存；在 ability_runtime 再建一层缓存是重复状态且引入一致性风险，收益低。若后续 profiling 证明 BMS 查询路径本身是热点，缓存应做在包管理侧（跨仓，本需求排除） |
| Q-5 | 量化目标口径：以"单次冷启动 BMS 同步查询次数"为主指标是否认可？基线值以实测为准？ | AC 需要可度量 | 已澄清：认可；同进程下代价主因是大结构体深拷贝与重复查询逻辑而非跨机 IPC，打点统计 BMS 查询次数与大对象拷贝次数 |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-10 | AI + 需求方 | 三轮并行源码分析（abilitymgr 启动链路 / appmgr 孵化链路 / 横切缓存与拷贝） | 形成重复查询点、重复存储字段、既有缓存缺陷三份清单（见 K-1~K-3） | — |
| 2026-08-10 | 需求方 | Q-1~Q-5 澄清确认 | Q-1：文档+一期代码实现；Q-2：一期 A-1+A-2；Q-3：允许一期纳入 A-4；Q-4：放弃 A-3（AMS/AppMgr/BMS 同进程，缓存应做在包管理侧，跨仓排除）；Q-5：以 BMS 同步查询次数为主指标，实测基线 | 进入 spec/design |
| 2026-08-10 | 需求方 | 追加范围：应用进程内启动阶段的包信息查询/存储重复分析 | 纳入一期范围（分析+修复）；应用进程与 foundation 为真实跨进程，Q-4 结论不适用，修复手段以复用已下发数据为主 | 应用进程侧源码分析（进行中） |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | ① 重复查询点/存储点清单（已产出）；② 一期修复：A-1 点状去重 + A-2 数据下沉复用 + A-4 InnerAPI 参数复用；③ 打点统计与回归测试 | 需求方 | 已确认 |
| 明确不包含哪些？ | BMS 侧修改（含"缓存做在包管理侧"的建议，跨仓排除）、Public/System API、应用进程内缓存、热启动路径行为变更、进程内统一缓存（A-3 放弃） | 需求方 | 已确认 |
| 是否有分期策略？ | 一期：A-1+A-2+A-4 + 打点；二期：仅当实测显示 BMS 查询路径仍为热点时，推动包管理侧缓存（跨仓，单独立项） | 需求方 | 已确认 |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | **点状去重**：将既有 thread_local 缓存复用到所有拦截器（IsStartIncludeAtomicService、screen_unlock、extension_control）；消除同一函数内字面重复查询（StartNativeProcessForDebugger 的 GetBundleAndHapInfo 双查、VerifyPermission 三连查）；GetNameForUid 结果沿调用链传递 | 改动小、语义零风险、无需失效机制 | 收益有限；跨函数/跨阶段重复仍在 | 推荐（一期） |
| A-2 | **数据下沉复用**：ModuleRunningRecord::Init 保存 HapModuleInfo（不再丢弃后回查）；LoadAbility 查到的 BundleInfo/HapModuleInfo 传入 CreateStartMsg/StartProcess 复用（appmgr 侧 3 次 IPC 降为 1 次）；AppRunningRecord 整体持有 BundleInfo 供后续字段需求 | 直接消除"手里有过又扔掉再查"的根因；不依赖缓存一致性 | 涉及 appmgr 多个函数签名扩展；BundleInfo 大对象持有增加单进程内存（KB 级，可接受） | 推荐（一期） |
| A-3 | ~~统一进程内包信息缓存~~ | ~~根治直连查询~~ | AMS/AppMgr/BMS 同驻 foundation 进程，BMS 自身已有数据缓存；ability_runtime 再建缓存 = 重复状态 + 一致性风险，收益低 | **放弃**（Q-4 结论：如需缓存应做在包管理侧，跨仓排除） |
| A-4 | **IPC 参数复用**：修改 AmsMgrProxy::LoadAbility 等 InnerAPI，把 AMS 已查到的包信息随调用传给 AppMgr，AppMgr 不再重查 | 消除 AMS→AppMgr 间信息断层；同进程下代价可控 | InnerAPI 变更，需兼容同进程/跨进程两条路径，回归面较大 | **推荐（一期，Q-3 确认）** |

**取舍理由：** 一期 A-1+A-2+A-4 以"复用已查结果、消灭丢弃后回查"为核心，不引入新的缓存状态，规避一致性风险；A-3 经澄清确认放弃（缓存的正确位置在包管理侧，属跨仓事项）。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 | abilitymgr 启动链路全量 BMS 查询扫描（StartAbility→拦截器链→GenerateAbilityRequest→LoadAbility） | 主链路 11 个查询点 + 拦截器链 11 个查询点 + 隐式启动 6 个查询点；高优重复点 3 个（LoadAbility 全量重查、IsStartIncludeAtomicService 绕过缓存、screen_unlock/extension_control 重查同一 want）；ApplicationInfo 单次启动 4 份拷贝 | 高 | 范围/设计 | 命中 |
| K-2 | 源码 | appmgr 孵化/加载链路全量 BMS 查询扫描（LoadAbility/StartProcess/CreateStartMsg/attach 后 AddAbilityStage） | 一次冷启动 appmgr 侧同一 bundle ≥3 次 BMS IPC；ModuleRunningRecord 丢弃 HapModuleInfo 后回查；StartNativeProcessForDebugger 同参数双查；BundleInfo 拆散存标量是重复查询根因之一；GetBundleAndHapInfo 被 8 处调用 | 高 | 范围/设计 | 命中 |
| K-3 | 源码 | 横切缓存机制与拷贝面扫描（BundleMgrHelper、services/common、包事件订阅、uid↔name 调用点） | 全仓无服务端包数据缓存；BundleMgrHelper 只缓存代理；包事件订阅已具备但未用于失效；uid↔bundleName 30+ 调用点；services/ 下 BMS 数据查询调用点共 109 处 | 高 | 范围/设计 | 命中 |
| K-4 | CLAUDE.md（仓 Agent 指南） | 构建/测试命令、错误处理、日志规范、特性标志 | 构建用 GN（build.sh --build-target）；测试 run -t UT；日志 TAG_LOGI(LABEL)；错误码 ERR_* | 高 | 计划/实现约束 | 命中 |
| K-5 | 源码 | 应用进程启动链路扫描（MainThread/OHOSApplication/ContextImpl/js_runtime，frameworks/native） | 冷启动应用进程对 BMS 主线程同步 IPC 序列：全量 GetBundleInfoForSelf → GetBaseSharedBundleInfos →（每 module 首个 ability）GetHapModuleInfo；最明确优化点 R10（AddAbilityStage(abilityRecord) 路径可复用已下发 HapModuleInfo，兄弟重载 :893 已证明可行）；ApplicationInfo 三份拷贝（S7）；应用进程内无 BMS 查询缓存 | 高 | 范围/设计 | 命中 |
| K-6 | 多仓知识库/DeepWiki | — | 未查询：本需求证据全部来自本仓源码，BMS 侧行为以假设 H2/H4 记录待 Owner 确认 | — | — | 未查询（无权限/不涉及跨仓修改） |

**上下文结论：**
- 高可信结论：重复查询/存储点清单（三份报告，全部附文件:行号）；既有去重机制仅 thread_local 单次启动缓存与点状补丁；包事件失效基础设施已存在。
- 待确认结论：BMS 侧是否已有可被复用的缓存（假设 H4：启动期间包信息不变）；量化基线需真机实测。
- 未使用来源及原因：多仓知识库/DeepWiki 未查询——单仓分析，证据自足。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | applications 子系统 / ability_runtime 仓（abilitymgr、appmgr、services/common） | 待确认 | 待确认 |
| 是否需要新增子系统或部件？ | 否（二期缓存若立项，新增 services/common 内组件，不新增部件） | 待确认 | 待确认 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 待确认 | 待确认 |
| 是否需要新增 System API？ | 否 | 待确认 | 待确认 |
| 是否会废弃已有 API？ | 否 | 待确认 | 待确认 |
| 是否需要新增权限声明？ | 否 | 待确认 | 待确认 |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | 行为语义零变更；A-4 InnerAPI 参数扩展需保持同进程/跨进程两路径兼容（新增参数带默认值/兼容分支） | 需求方 | 已确认 |
| 性能 | 响应时间/内存/并发要求？ | 冷启动 BMS 同步 IPC 次数下降（目标见基线成功指标）；AMS/AppMgr 单进程内存增量 < 数百 KB（持有 BundleInfo）；并发：缓存读写线程安全 | 待确认 | 待确认 |
| 安全 | 权限/隐私/加密/审计要求？ | 不涉及新增权限；权限校验结果不得因去重而跳过（时序语义保持） | 待确认 | 待确认 |
| 可靠性 | 崩溃率/容错/恢复要求？ | 缓存失效失败时降级为直查 BMS；不得引入新 crash 路径 | 待确认 | 待确认 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| bundle_framework（BMS） | 运行 | 查询接口与包事件通知语义不变；不修改 BMS | 已确认 |
| SubscribeBundleEventCallback | 运行 | 二期缓存失效依赖既有包事件回调（ability_manager_service.cpp:4207） | 已确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| 缓存不一致导致用过期包信息（二期） | 技术 | 高 | 包事件失效订阅 + 启动关键校验点保留直查 + 灰度开关 | 待确认 |
| 去重改变权限校验时序/结果 | 技术 | 中 | 校验类查询（VerifyPermission 等）一期只做同函数内去重，不跨阶段复用结果 | 待确认 |
| 持有 BundleInfo 增加常驻内存 | 技术 | 低 | 仅 AppRunningRecord 维度持有（每进程一份），KB 级 | 已确认 |
| 量化收益不及预期 | 进度 | 低 | 一期先落地打点统计（hilog 计数），以实测验证 | 已确认 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确（待需求方确认 Q-1~Q-5 后关闭）
- [x] 子系统影响已识别
- [x] API 变更已评估（无）
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（A-1~A-4 + 取舍理由）

**结论:** 通过（Q-1~Q-5 已经需求方确认）

---

## 三、需求基线

> 待 Q-1~Q-5 澄清确认后固化。

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-10 |
| Owner | 待定 |
| 确认人 | 需求方（2026-08-10 澄清确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | TBD |
| 版本状态 | proposed |

### 问题陈述

应用冷启动链路上，AMS 与 AppMgr 两侧对同一 bundle 的包信息存在系统性重复查询（单次冷启动对 BMS 的同步 IPC 5~8 次，其中至少 4~6 次可消除）与重复存储（ApplicationInfo 单次启动 ≥4 份拷贝、HapModuleInfo 丢弃后回查、BundleInfo 拆散存标量导致后续重查）。全仓 109 处 BMS 查询直连 IPC，无统一缓存，去重仅靠 thread_local 单次启动缓存和点状补丁。这直接拖慢应用启动并抬升维护成本。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 消除同函数/同阶段字面重复查询 | StartNativeProcessForDebugger、VerifyPermission、IsStartIncludeAtomicService 等已识别重复点消减为单次查询（代码审查可证） | 代码审查 + 单元测试 |
| 消除"丢弃后回查" | 冷启动 attach 后 AddAbilityStage 不再向 BMS 回查 HapModuleInfo（ModuleRunningRecord 持有） | 单元测试 + hilog 打点计数 |
| 降低单次冷启动 BMS 同步 IPC 次数 | 一期目标：普通冷启动从 ~8 次降为 ≤4 次（实测基线后校准） | hilog 打点统计 + 启动 trace |
| 行为语义零变化 | 全量 ability_runtime_test 通过；权限校验、拦截器判定结果与变更前一致 | 回归测试 |
| 减少包信息拷贝 | ApplicationInfo 在 AMS 侧由 4 份降为共享持有（thread_local 缓存 + 引用传递） | 代码审查 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统开发者，我想要一份含证据的重复查询/存储清单，以便确认优化对象 | P0 |
| US-2 | 作为系统开发者，我想要消除启动链路上证据充分的重复 BMS 查询，以便降低启动耗时 | P0 |
| US-3 | 作为系统开发者，我想要包信息在启动链路上被复用而非丢弃后重查，以便根治重复查询 | P0 |
| US-4 | 作为测试者，我想要行为语义不变的回归保障，以便安全合入 | P0 |
| US-5 | 作为系统开发者，我想要可观测的打点统计，以便量化优化收益 | P1 |
| US-6 | 作为系统开发者，我想要消除应用进程内启动阶段的重复包信息查询（复用 AppLaunchData 等已下发数据），以便减少真实跨进程 IPC | P0 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 完成源码分析 THEN 产出重复查询点清单与重复存储字段清单，每条附文件:行号证据 | 正常 | US-1 |
| AC-2 | WHEN 同一函数/同一拦截器执行内需多次使用同一包信息 THEN 只发起一次 BMS 查询，后续使用复用结果 | 正常 | US-2 |
| AC-3 | WHEN 拦截器（jump/screen_unlock/extension_control 等）需要目标包 applicationInfo/abilityInfo THEN 优先命中既有 thread_local 启动缓存，未命中才直查 BMS | 正常 | US-2 |
| AC-4 | WHEN AppMgr LoadAbility 已查到 BundleInfo/HapModuleInfo THEN 同次启动的 CreateStartMsg/StartProcess/AddAbilityStage 复用该结果，不再向 BMS 重查同一 bundle 的相同信息 | 正常 | US-3 |
| AC-5 | WHEN ModuleRunningRecord 初始化时已获得 HapModuleInfo THEN 后续 GetHapModuleInfo 直接返回持有副本，不再回查 BMS | 正常 | US-3 |
| AC-6 | WHEN 执行权限校验类路径（VerifyPermission 等）THEN 校验结果与变更前逐点一致（不跨阶段复用校验输入） | 正常 | US-4 |
| AC-7 | WHEN 执行 `run -t UT -tp ability_runtime` 全量单元测试 THEN 全部通过 | 正常 | US-4 |
| AC-8 | WHEN BMS 查询失败或缓存未命中 THEN 回退为直查 BMS 的原有行为，无新 crash 路径、有错误日志 | 异常 | US-4 |
| AC-9 | WHEN 应用分身/沙箱/插件（appIndex≠0、plugin）场景启动 THEN 复用逻辑按 (bundleName, appIndex, userId) 维度区分，不串数据 | 边界 | US-3 |
| AC-10 | WHEN 冷启动完成 THEN 打点日志可统计本次启动的 BMS 同步查询次数（优化前后可对比） | 正常 | US-5 |
| AC-11 | WHEN 应用进程启动阶段某组件需要的包信息已随 AppLaunchData/ScheduleAbilityStage 下发或已在本次启动中查询过 THEN 直接使用已有数据，不再向 BMS 发起重复 IPC 查询（重点：`OHOSApplication::AddAbilityStage(abilityRecord)` 路径不再经 `ContextImpl::InitHapModuleInfo` 重查 GetHapModuleInfo；runtime 侧 InitSourceMap/GetPresetAppHapPath 复用 HandleLaunchApplication 已展开的模块路径信息） | 正常 | US-6 |
| AC-12 | WHEN 应用进程内同一启动阶段多处使用 ApplicationInfo THEN 共享同一 shared_ptr 持有（消除 AppLaunchData→MainThread→ContextImpl 的重复 make_shared 拷贝） | 正常 | US-6 |
| AC-13 | WHEN 混合 arkTSMode（1.0 应用拉起 1.2 ability）触发 CheckAndUpdateRuntime THEN 复用 HandleLaunchApplication 已查询的 BundleInfo/HSP 列表，不原样重查 | 边界 | US-6 |

### 范围边界

**包含：**
- 一期（方案 A-1+A-2+A-4）：abilitymgr/appmgr 启动链路的点状去重、数据下沉复用、InnerAPI 参数复用；打点统计；单元测试
- **应用进程（AppKit/框架侧）启动阶段的重复查询/存储分析与修复**（2026-08-10 需求方追加；该侧与 foundation 跨进程，Q-4"同进程无需缓存"结论不适用于此，修复手段以复用 AppLaunchData/已下发数据为主，是否引入进程内缓存视分析结果定）
- 重复查询/存储清单文档（本 proposal 证据 + design 详表）

**不包含：**
- BMS（bundle_framework）侧修改——含 Q-4 澄清结论"如需统一缓存应做在包管理侧"，跨仓排除
- Public/System API 变更
- 进程内统一缓存（A-3 放弃）；应用进程侧是否引入轻量缓存待分析结论（若引入仅限本进程内、无跨进程一致性需求）
- 热启动/后台调度路径行为变更

**分期策略（如适用）：**

| 阶段 | 范围 | 不做 | 进入条件 |
|------|------|------|----------|
| 一期 | A-1 点状去重 + A-2 数据下沉复用 + A-4 InnerAPI 参数复用 + 应用进程侧去重复用 + 打点 | 任何新缓存层、BMS 侧修改 | 本基线确认 |
| 二期 | 仅当实测显示 BMS 查询路径仍为热点时，推动包管理侧缓存 | — | 一期实测数据（跨仓，单独立项） |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| applications/ability | ability_runtime | services/abilitymgr（interceptor/、utils/、ability_record.cpp、ability_manager_service.cpp） | 启动调度与拦截 | 修改 | 待定 |
| applications/ability | ability_runtime | services/appmgr（app_mgr_service_inner.cpp、module_running_record.cpp、app_running_record.cpp、app_preloader.cpp） | 进程孵化与加载 | 修改 | 待定 |
| applications/ability | ability_runtime | services/abilitymgr/include/utils/（start_ability_utils.h 等） | 启动信息缓存 | 修改 | 待定 |
| applications/ability | ability_runtime | interfaces/inner_api/app_manager（IAppMgr/AmsMgrProxy 等） | AMS↔AppMgr InnerAPI | 修改（A-4 参数扩展） | 待定 |
| applications/ability | ability_runtime | frameworks/native/appkit（main_thread、context_impl、application_context 等） | 应用进程启动与上下文 | 修改（复用已下发包信息） | 待定 |

### API 变更项清单

N/A — 基线时曾列入 `IAppMgr` 参数扩展（A-4），实现期核对数据依赖后取消（见 design.md ADR-7 修正记录）；最终无任何 API（Public/System/InnerAPI）变更。

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 是 | 本需求即启动性能优化 | design.md / spec.md |
| 安全与权限 | 是（受限） | 权限校验路径去重需保证语义不变（AC-6） | spec.md |
| 兼容性 | 是 | 行为语义零变更约束 | spec.md |
| API/SDK | 否/N/A | A-4 经实现期核对后取消，最终无 API 变更 | — |
| IPC/跨进程 | 是（仅查询次数/载荷优化） | 减少 AMS/AppMgr→BMS 查询次数与载荷；应用进程侧减少真实跨进程 IPC；不改任何 IPC 接口 | design.md |
| 构建与部件 | 否/N/A | 不新增部件，BUILD.gn 仅可能新增测试源文件 | — |
| 国际化/无障碍 | 否/N/A | 不涉及 UI 文案 | — |
| 数据迁移 | 否/N/A | 无持久化数据 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 纳入二期缓存或 InnerAPI 变更 | 重新评估复杂度，单独立项 |
| AC 变更 | 修改可观察行为或指标口径 | 重新审批基线和 Spec |
| API 变更 | 任何 Public/System/InnerAPI 变更提议 | 触发设计审批，升级复杂度 |
| 非功能指标变更 | IPC 次数目标、内存阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认（Q-1~Q-5 + 应用进程侧追加范围）
- [ ] `proposal.target_release` 已确认或明确 TBD（TBD，Owner 排期时回填）
- [x] `manifest.profile` 已确认或明确 none（仓内无 manifest.md、无 git remote，兜底为 none）
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭，且讨论记录包含需求方明确确认（2026-08-10）
- [x] 上下文与知识源检索日志已填写；未查询关键来源的原因已记录
- [x] 目标仓 Agent 指南已检查并记录关键约束（K-4）

**基线结论:** 通过（2026-08-10 需求方确认 Q-1~Q-5；应用进程侧追加分析范围已纳入）

---

## 附录：分析结论摘要（证据详见各文件:行号）

### 重复查询点（高优）

| # | 重复点 | 证据 |
|---|--------|------|
| R1 | LoadAbility 阶段对刚查过的 bundle 再做全量 `GetBundleInfoV9`，仅为判断 mainElement | `services/abilitymgr/src/ability_record.cpp:359` → `utils/main_element_utils.cpp:74` |
| R2 | `IsStartIncludeAtomicService` 每次拦截直查 BMS（GetApplicationInfo+GetNameForUid 3 次 IPC），绕过 thread_local 缓存 | `services/abilitymgr/include/utils/ability_util.h:271,284` |
| R3 | screen_unlock / extension_control 拦截器对同一 want 重查 QueryAbilityInfo | `interceptor/screen_unlock_interceptor.cpp:269`、`interceptor/extension_control_interceptor.cpp:158` |
| R4 | AppMgr 一次冷启动同一 bundle ≥3 次 BMS IPC（GetBundleAndHapInfo×2 + attach 后 GetCloneBundleInfo） | `app_mgr_service_inner.cpp:2214,5780`、`module_running_record.cpp:392` |
| R5 | StartNativeProcessForDebugger 同参数双查 GetBundleAndHapInfo（4 次 IPC） | `app_mgr_service_inner.cpp:9934,5780` |
| R6 | 同一 callingUid 的 GetNameForUid 多次调用（jump 拦截器内 2 次、UpdateCallerInfo 链 2 次、ERMS 1 次） | `ability_util.h:284`、`ability_jump_interceptor.cpp:93`、`update_caller_info_util.cpp:102,425` |
| R7 | debug 启动下 QueryAbilityInfo 两次（CheckAppProvisionMode 未写回 thread_local） | `start_ability_utils.cpp:119-141 vs 143-170` |
| R8 | VerifyPermission 三连查（GetNameForUid→GetBundleInfo→GetNameForUid） | `ability_manager_service.cpp:14015-14050` |
| R9 | 预加载查一遍（4 次 IPC），正式 LoadAbility 又查一遍 | `app_preloader.cpp:54-96,204-221`、`app_mgr_service_inner.cpp:2214` |
| R10 | 应用进程：同一 module 的 HapModuleInfo 冷启动获取 3 次——Q1 全量 GetBundleInfoForSelf 已含、AMS ScheduleAbilityStage 已下发、`OHOSApplication::AddAbilityStage(abilityRecord)` 仍经 `ContextImpl::InitHapModuleInfo` 第 3 次 IPC 重查（真实跨进程） | `main_thread.cpp:1694,855/2700`、`ohos_application.cpp:555-673`、`context_impl.cpp:1319-1334`；对照可复用路径 `ohos_application.cpp:893` |
| R11 | 应用进程：runtime 侧 InitSourceMap 与 GetPresetAppHapPath 重查 hapPath 映射，而 Q1 结果已展开进 Runtime::Options | `js_runtime.cpp:791/1016`、`js_module_reader.cpp:263,290`、`main_thread.cpp:1925-1938` |
| R12 | 应用进程：混合 arkTSMode 时 CheckAndUpdateRuntime 原样重查 Q1 的 GetBundleInfoForSelf+GetBaseSharedBundleInfos | `main_thread.cpp:1581/1589,4811` |

### 重复存储点

| # | 信息 | 拷贝位置 |
|---|------|----------|
| S1 | ApplicationInfo（单次启动 ≥4 份） | thread_local `startAbilityInfo->abilityInfo.applicationInfo`；`AbilityRequest::abilityInfo.applicationInfo`；`AbilityRequest::appInfo`（ability_manager_service.cpp:9048 显式再拷）；`AbilityRecord::abilityInfo_.applicationInfo` |
| S2 | ApplicationInfo（AppMgr 侧 ≥4 处持有） | `AppRunningRecord::appInfo_`（app_running_record.h:1414）；`AppRunningRecord::appInfos_`（:1402）；`ModuleRunningRecord::appInfo_`（module_running_record.h:188）；`AbilityInfo.applicationInfo` 内嵌 |
| S3 | HapModuleInfo 丢弃后回查 | `ModuleRunningRecord::Init` 只存 moduleName_；`GetHapModuleInfo` 每次查全量 BundleInfo（module_running_record.cpp:378-395） |
| S4 | BundleInfo 拆散存标量 | `app_running_manager.cpp:96-102` 逐字段 Set；后续字段需求只能重查 BMS |
| S5 | customProcess 三处冗余 | `StartAbilityInfo::customProcess`、`AbilityRequest::customProcess`、`AbilityRecord::customProcessFlag_` 均 = `abilityInfo.process` |
| S6 | BundleInfo 全量值拷贝 3 处 | `PreloadRequest`（app_preloader.h:35-36）、`CreateStartMsgParam`（app_spawn_client.h:130）、`ChildProcessInfo` |
| S7 | 应用进程 ApplicationInfo 3 份拷贝 | `AppLaunchData::applicationInfo_`（app_launch_data.h:398）、`MainThread::applicationInfo_`（main_thread.h:795，:1183 make_shared）、`ContextImpl::applicationInfo_`（context_impl.h:586，main_thread.cpp:1780 再次 make_shared 而非共享） |
| S8 | 应用进程 HapModuleInfo 双路径各存一份 | `ContextImpl::hapModuleInfo_`（context_impl.h:589）：ScheduleAbilityStage 路径 :1391 拷贝 / abilityRecord 路径 :1330 IPC 结果填充 |

### 既有去重机制与缺陷

| 机制 | 位置 | 缺陷 |
|------|------|------|
| thread_local StartAbilityInfo 缓存 | `start_ability_utils.h:76-77` | 仅线程内单次启动有效；多个拦截器未使用 |
| BundleMgrHelper | `bundle_mgr_helper.cpp` | 只缓存代理对象，不缓存包数据 |
| 包事件订阅 | `ability_manager_service.cpp:4207` | 未用于任何数据缓存失效 |
