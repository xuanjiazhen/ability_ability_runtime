# 架构设计：应用启动链路包信息重复查询/存储治理

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-DUP-BUNDLEINFO |
| 关联需求 | `.codespec/changes/dup-bundle-info-query/proposal.md`（基线 v1.0） |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-DUP-BUNDLEINFO |
| 复杂度 | 标准 |
| 目标版本 | TBD（引用 proposal.target_release） |
| Owner | 待定 |
| 状态 | Draft |

## 需求基线

| 项 | 补充说明（如需） |
|----|------------------|
| 一期范围 = A-1 点状去重 + A-2 数据下沉复用 + A-4 InnerAPI 参数复用 + 应用进程侧去重 | 不引入任何跨请求缓存层（A-3 已放弃） |
| 行为语义零变更 | 权限校验、拦截判定、生命周期时序逐点一致（spec AC-6/7/8） |
| 复用键值维度 | (bundleName, appIndex, userId)，复用对象只读（spec R-3/R-7、架构约束） |

## 上下文和现状

### 涉及仓和模块

| 仓库 | 补充架构说明 |
|------|-------------|
| ability_runtime | 单仓。AMS/AppMgr/BMS 同驻 foundation 进程（IN_PROCESS_CALL），BMS 查询代价主因是大结构体（BundleInfo 含全部 hapModuleInfos/permissions/signature）深拷贝与重复查询逻辑；应用进程与 foundation 为真实跨进程 binder IPC，重复查询代价更高 |

### 调用链层级分析

| 层 | 模块 | 职责 | 修改类型 |
|----|------|------|----------|
| 服务 | abilitymgr（interceptor/、utils/、ability_record.cpp、scene_board/） | 启动调度、拦截器链、启动信息 thread_local 缓存 | 修改 |
| 胶水（InnerAPI） | interfaces/inner_api/app_manager（IAppMgr/AppMgrProxy/Stub） | AMS↔AppMgr 调用接口 | 修改（参数扩展，兼容） |
| 服务 | appmgr（app_mgr_service_inner.cpp、module_running_record.cpp、app_running_record.cpp） | 进程孵化、加载、模块登记 | 修改 |
| 框架 | frameworks/native/appkit（main_thread.cpp、ohos_application.cpp、context_impl.cpp） | 应用进程启动、上下文初始化 | 修改 |
| 框架 | frameworks/native/runtime（js_runtime.cpp、js_module_reader.cpp） | JS 运行时模块路径解析 | 修改 |

**检查项：**
- [x] 调用链每一层都已覆盖（服务 AMS → 胶水 InnerAPI → 服务 AppMgr →（孵化）→ 框架 appkit → 运行时）
- [x] 每层职责边界清晰：去重均发生在本层内部或相邻层参数传递，无跨层违规调用
- [x] 每层修改类型明确（全部为"修改"，无新增部件/删除）

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | 涉及服务/框架/胶水多层 | 复用数据沿既有调用方向传递（上→下），禁止下层回调上层取数 | 代码评审 |
| OH-ARCH-SUBSYSTEM | 不跨子系统 | 仅 ability_runtime 仓内 | 依赖检查 |
| OH-ARCH-IPC-SAF | AMS↔AppMgr 调用 | **（实现期修正）不修改任何 InnerAPI**；检查类调用通过 AppMgr 侧轻量查询降载（ADR-7） | 集成测试 |
| OH-ARCH-API-LEVEL | 仅 InnerAPI 静态方法新增 | **（实现期）** 新增 `JsRuntime::SetModuleHapPathList` 转发方法（与 SetAppLibPath 同模式）；无 Public/System API、无 SysCap、无 AMS↔AppMgr/AMS↔应用 接口语义变更 | API 评审 |
| OH-ARCH-COMPONENT-BUILD | 无新增部件 | 仅新增单元测试源文件入既有 test target | 构建验证 |
| OH-ARCH-ERROR-LOG | 回退路径日志 | 回退/失败路径 TAG_LOGW/TAG_LOGE（含 bundleName/appIndex/userId），沿用既有 ERR_* 错误码 | 单测/hilog |

## 不涉及项承接

| 维度 | 设计结论 |
|------|----------|
| 性能 | 主指标：单次冷启动三侧 BMS 同步查询次数合计下降 ≥40%。手段全部为"复用/降载"，无新缓存层 |
| 安全与权限（受限涉及） | 权限校验类路径仅同函数内去重，校验输入不跨阶段复用（ADR-2）；见「安全基础检查」 |
| 兼容性 | InnerAPI 参数扩展保持未携带时行为与现状一致（ADR-7）；无配置/存储格式变更 |
| IPC/跨进程 | 减少 AMS/AppMgr→BMS 查询次数；A-4 修改 AMS↔AppMgr InnerAPI（ADR-7）；应用进程侧减少真实 binder IPC（ADR-8/9/10） |

## 关键设计决策

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|----------|------|
| ADR-1 | 拦截器（jump/screen_unlock/extension_control）重复查询目标 abilityInfo/applicationInfo（proposal R2/R3，AC-3） | 统一改走 `StartAbilityUtils` 的 thread_local 缓存优先入口（既有 `GetApplicationInfo` 缓存版 start_ability_utils.cpp:71-75 + 为 abilityInfo 增加同款缓存优先查询），缓存键含 (bundleName, appIndex, userId) | 备选1：各拦截器各自比对缓存（ability_jump_interceptor.cpp:52-55 的现有模式复制）——放弃，逻辑分散易漏；备选2：跨请求缓存——放弃（A-3 已否决） | 收敛到单一入口，后续拦截器自动受益；thread_local 生命周期已由 StartAbilityInfoWrap RAII 管理，无一致性新风险 | services/abilitymgr/utils + 3 个拦截器 |
| ADR-2 | 同函数/同链字面重复（R6 GetNameForUid×2、R7 debug 双查、R8 VerifyPermission 三连查，AC-2/AC-6） | 同函数内去重：UpdateCallerInfo 将 callerBundleName 传给 ClearProtectedWantParam；CheckAppProvisionMode 查询结果写回 thread_local 缓存；VerifyPermission 内 GetNameForUid 去重；GetCallerUidAndToken 的 GetBundleInfo(WITH_ABILITIES) 降为 GetUidByBundleName | 备选：跨阶段复用校验输入——**禁止**（spec R-5），仅同函数内去重 | 权限语义零风险前提下的最大去重；重查询降载返回数据同源（uid） | abilitymgr utils/ability_manager_service.cpp |
| ADR-3 | LoadAbility 判断 mainElement 全量查 GetBundleInfoV9（R1，AC-2） | 降为 `GetHapModuleInfo`（单模块），取 mainElementName 比较；查询失败回退现有全量查询 | 备选1：AbilityRecord 持有 HapModuleInfo——放弃，AMS 侧 GenerateAbilityRequest 仅在 allowMultiProcess 时才有 hapModuleInfo，持有需扩 AbilityRequest；备选2：维持现状——放弃，全包结构体拷贝是启动热点 | 载荷从全包降单模块，逻辑等价；失败回退保证语义 | ability_record.cpp/main_element_utils.cpp |
| ADR-4 | ModuleRunningRecord 丢弃 HapModuleInfo 后每次回查全量 BundleInfo（R4/S3，AC-5） | `Init` 增加持有 `std::shared_ptr<const HapModuleInfo>`；`GetHapModuleInfo` 直接返回持有副本；`UpdateApplicationInfoInstalled`（app_mgr_service_inner.cpp:3257）时刷新 | 备选：AppRunningRecord 统一持有所有模块 HapModuleInfo——放弃，ModuleRunningRecord 是天然归属点，改动最小 | 直接消除"手里有过又扔掉再查"；每模块一份 KB 级持有 | appmgr module_running_record |
| ADR-5 | AppMgr 查询类辅助接口（IsSpecifiedModuleLoaded/IsCorrespondingProcessAttachDebug/预加载后 LoadAbility）重复 GetBundleAndHapInfo（R4/R9，AC-4） | **（实现期修正）** ① LoadAbility selfPid 分支下游只消费 hapModuleInfo，改为仅查 HapModuleInfo（失败回退全量），直接省掉 BundleInfo 查询；② 两个检查接口 appIndex==0 主路径改为轻量查询（GetBundleInfoV9(WITH_APPLICATION) [+ GetHapModuleInfo]），分身/沙箱路径保持全量 | 备选1：AppRunningRecord 整体持有 BundleInfo——放弃（实现期核对：各消费点或被 TASK-6 轻量化覆盖、或根本不需要 bundleInfo，持有将无消费者）；备选2：继续持有散标量——放弃 | 核对真实消费点后以"按需轻量查询"替代"持有全量"，零新增状态、零失效问题 | appmgr app_mgr_service_inner |
| ADR-6 | StartNativeProcessForDebugger 同参数双查 GetBundleAndHapInfo（R5，AC-4） | `CreatNewStartMsg` 增加可选入参（已查到的 bundleInfo/hapModuleInfo/appIndex 指针，默认 nullptr）；调用方（9956）传入 9934 已查结果；nullptr 时走现有自查 | 备选：重构 CreatNewStartMsg 签名强制传入——放弃，其他调用方无已查结果，强制签名会扩散改动 | 可选参数保持全部既有调用点行为不变 | app_mgr_service_inner.cpp |
| ADR-7 | AMS↔AppMgr 检查类调用在 AppMgr 侧为查表而重查包信息（A-4，AC-4/8/9） | **（实现期修正）不改 InnerAPI**：核对 `CheckAppRunningRecordIsExist` 真实消费（jointUserId/appId→signCode）与 `MakeProcessName` 依赖（hapModuleInfo.process/isolation）后，这些数据 AMS 侧不可得，"调用方传 processName"无法消除查询；改为 AppMgr 侧轻量查询替代（appIndex==0 主路径：`IsCorrespondingProcessAttachDebug` 仅查 GetBundleInfoV9(WITH_APPLICATION)，`IsSpecifiedModuleLoaded` 查轻量 BundleInfo+单模块 HapModuleInfo），2 次重量级查询降为 1~2 次轻量查询 | 备选1：InnerAPI 追加 processName 可选参数（原方案）——放弃，所需数据不全在调用方，参数扩展消除不了查询，徒增五层签名与 parcel 变更；备选2：MakeProcessName 上移 AMS——放弃，逻辑双份风险 | 在不改任何接口的前提下获得大部分收益；接口零兼容风险 | services/appmgr（仅实现，不涉及 interfaces） |
| ADR-8 | 应用进程 AddAbilityStage(abilityRecord) 路径 InitHapModuleInfo 重查（R10，AC-11） | `MainThread::HandleLaunchApplication` 已持全量 BundleInfo：将其 `hapModuleInfos` 以 `map<moduleName, shared_ptr<HapModuleInfo>>` 持久化到 OHOSApplication（`SetHapModuleInfoMap`，受 runtimeMutex_ 保护）；**查找归属 map 持有者**——`AddAbilityStage(abilityRecord)` 命中 `GetHapModuleInfoByModule` 后调 `stageContext->InitHapModuleInfo(HapModuleInfo)`（复用 ContextImpl 既有重载），未命中回退现有 IPC | 备选1：abilityRecord 携带 HapModuleInfo 下发——放弃，需改 ScheduleLaunchAbility InnerAPI 与序列化，面大；备选2：查找放 ContextImpl（经 parentContext_ 链）——放弃，map 持有者是 OHOSApplication，查找放持有者更内聚，context_impl 无需改动 | 数据本进程已有，仅缺持久化与查找入口；回退保证任意时序/异常下行为不变 | appkit main_thread/ohos_application（context_impl 复用既有重载，不改） |
| ADR-9 | runtime 侧 InitSourceMap/GetPresetAppHapPath 重查模块路径（R11，AC-11） | `JsRuntime` 将已展开的 moduleName→hapPath 映射（options.hapModulePath/pkgContextInfoJsonStringMap_，main_thread.cpp:1925-1938）传入 `JsModuleReader`/`HybridJsModuleReader`（构造或 setter）；映射命中即用，未命中回退现有 BMS 查询 | 备选：模块加载时按需查 BMS 保持现状——放弃，ABC 加载路径上的同步 IPC 是启动热点 | 复用已展开数据，零新状态 | frameworks/native/runtime |
| ADR-10 | 混合 arkTSMode 下 CheckAndUpdateRuntime 原样重查（R12，AC-13） | `MainThread` 将 Q1 的 BundleInfo 与 Q2 的 hspList 存为成员（仅本次启动生命周期）；`GetBundleAndHspListForUpdateRuntime` 优先用成员，为空回退现有查询 | 备选：保持重查——放弃，同函数语义数据源完全相同 | 成员级复用，无跨请求状态 | appkit main_thread |
| ADR-11 | 应用进程 ApplicationInfo 三份拷贝（S7，AC-12） | MainThread 创建唯一 `shared_ptr<ApplicationInfo>`；ContextImpl/OHOSApplication 共享该指针（main_thread.cpp:1780 不再 make_shared 新对象）；更新（UpdateApplicationInfoInstalled）整体替换指针 | 备选：保持拷贝——放弃，KB 级 × N context；备选：全仓引用计数重构——放弃，面大收益小 | 替换指针语义与现有更新路径兼容；持有期间只读 | appkit main_thread/context_impl |
| ADR-12 | 打点统计（AC-10） | `BundleMgrHelper` 各 Get* 查询入口加进程内原子计数（DEBUG 级开关）；AMS（StartAbilityInfoWrap 析构）、AppMgr（LoadAbility 完成）、应用进程（HandleLaunchApplication 完成）三处各输出一行计数日志（TAG_LOGD，含 bundleName） | 备选：hisysevent 上报——放弃，一期仅需可对比日志，不引入事件上报 | 零依赖、可开关、不产生额外查询 | bundle_mgr_helper + 三处打点 |
| ADR-13 | AMS 侧 AbilityRequest::appInfo 与 abilityInfo.applicationInfo 双份存储（S1） | **一期不做**：AbilityRequest 为值语义结构体且读点众多，重构为共享指针改动面大、收益小（同进程拷贝）；记录为开放问题，二期结合 AMS 侧结构治理评估 | 备选：request.appInfo 改 shared_ptr——放弃（本期），读点遍布 abilitymgr | 不影响任何 AC；proposal 成功指标中"AMS 侧 4 份降共享"以应用进程侧（AC-12）与 AppMgr 侧共享持有（ADR-5）为主要达成路径 | 开放问题 |

## 设计骨架

### 骨架范围

| 骨架项 | 目标 | 不包含 | 验证方式 |
|--------|------|--------|----------|
| 复用持有骨架 | ModuleRunningRecord.hapModuleInfo_ 成员与更新失效点 | 全部消费点切换 | 编译通过 |
| 打点骨架 | BundleMgrHelper 计数器 + 三处输出点 | 全量查询点覆盖 | 日志验证 |
| 测试骨架 | mock BMS 计数 fixture（统计查询次数） | 全场景 | 最小用例通过 |

### 骨架 Spec 拆分

| Task ID | 目标 | 受影响文件 | AC |
|---------|------|------------|-----|
| TASK-SKELETON-1 | 打点骨架 | bundle_mgr_helper.* | WHEN 编译并运行既有 UT THEN 全部通过（行为不变） |

## 后续 Task 拆分

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-1 | 打点基础设施（ADR-12，AC-10） | frameworks/native/appkit/ability_bundle_manager_helper/*、start_ability_utils.cpp、app_mgr_service_inner.cpp、main_thread.cpp | design + spec Approved |
| TASK-2 | AMS 拦截器缓存复用（ADR-1，AC-3） | start_ability_utils.*、ability_util.h、screen_unlock_interceptor.cpp、extension_control_interceptor.cpp | TASK-1 |
| TASK-3 | AMS 同函数去重（ADR-2，AC-2/AC-6） | update_caller_info_util.cpp、start_ability_utils.cpp、ability_manager_service.cpp | TASK-1 |
| TASK-4 | mainElement 降载（ADR-3，AC-2） | ability_record.cpp、main_element_utils.cpp | TASK-1 |
| TASK-5 | ModuleRunningRecord 持有 HapModuleInfo（ADR-4，AC-5） | module_running_record.*、app_running_record.cpp、app_mgr_service_inner.cpp | TASK-1 |
| TASK-6 | AppMgr 轻量查询化（ADR-5/6/7 修正版，AC-4/8/9） | app_mgr_service_inner.cpp（LoadAbility selfPid 分支、CreatNewStartMsg 可选参数、IsCorrespondingProcessAttachDebug/IsSpecifiedModuleLoaded 轻量查询） | TASK-1 |
| TASK-7 | ~~InnerAPI 检查接口参数扩展~~（已取消，见 ADR-7 修正；其 AC-4/8/9 由 TASK-6 轻量查询覆盖） | — | — |
| TASK-8 | 应用进程 HapModuleInfo 复用（ADR-8，AC-11） | main_thread.cpp、ohos_application.cpp/h、context_impl.cpp/h | TASK-1 |
| TASK-9 | runtime 模块路径复用 + 混合模式复用（ADR-9/10，AC-11/13） | js_runtime.cpp、js_module_reader.*、hybrid_js_module_reader.cpp、main_thread.cpp | TASK-8 |
| TASK-10 | ApplicationInfo 单一指针共享（ADR-11，AC-12） | main_thread.cpp、context_impl.*、ohos_application.cpp | TASK-8 |
| TASK-11 | 单元测试与全量回归（AC-7/8/9 + 各 AC 计数验证） | test/unittest/ 对应套件 | TASK-2~10 |

## API 签名、Kit 与权限

### 新增 API

| API 签名 | 类型 | Kit | d.ts 位置 | 权限要求 | SysCap |
|----------|------|-----|-----------|----------|--------|
| `static void JsRuntime::SetModuleHapPathList(const std::vector<std::pair<std::string,std::string>> &hapPathList)` | InnerApi | 运行时框架 | 无（InnerAPI 无 d.ts） | - | - |

> 注：该方法为**纯转发**实现（转发到 `JsModuleReader::SetHapPathMap`），把应用进程启动期已展开的 module→hapPath 列表注入运行时层，供模块路径解析复用。与既有 InnerAPI 静态方法 `JsRuntime::SetAppLibPath` 同模式（同文件、同签名风格）。**不涉及 AMS↔AppMgr、AMS↔应用 的任何接口语义变更**；Public/System API 零变更。

### 变更/废弃 API

N/A — 经核对 `CheckAppRunningRecordIsExist`/`MakeProcessName` 的真实数据依赖后，AMS↔AppMgr InnerAPI 参数扩展无法消除目标查询（见 ADR-7），故 `IAppMgr` 系列接口签名零变更；无 Public/System API 变更。

## 构建系统影响

### BUILD.gn 变更

```
文件路径: test/unittest/ 对应模块 BUILD.gn（如 module_running_record 相关测试 target）
变更说明: 新增/扩展单测源文件入既有 test target；无新增部件、无新依赖
```

### bundle.json 变更

无。

---

## 可选设计扩展

### 数据流/控制流

冷启动包信息查询流（变更前 → 变更后，同步查询计数）：

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 变更前 | 变更后 |
|------|--------|----------|-----------|--------|--------|
| 1 | AMS StartAbilityInner | BMS | QueryAbilityInfo | 1 次 | 1 次（不变） |
| 2 | AMS 拦截器链 | BMS | GetApplicationInfo/QueryAbilityInfo/GetNameForUid | 1~4 次（jump/screen_unlock/extension_control/UpdateCallerInfo） | ≤1 次（缓存命中，ADR-1/2） |
| 3 | AMS LoadAbility(mainElement) | BMS | GetBundleInfoV9 全量 | 1 次（全包载荷） | 1 次（单模块载荷，ADR-3） |
| 4 | AppMgr LoadAbility | BMS | GetBundleInfoV9 + GetHapModuleInfo | 2 次 | 2 次（不变，信息下传复用） |
| 5 | AppMgr attach 后 AddAbilityStage | BMS | GetCloneBundleInfo 全量 | 1 次 | 0 次（ADR-4） |
| 6 | 应用进程 HandleLaunchApplication | BMS | GetBundleInfoForSelf + GetBaseSharedBundleInfos | 2 次 | 2 次（不变，结果持久化复用） |
| 7 | 应用进程 AddAbilityStage(非 entry module) | BMS | GetHapModuleInfo | 每模块 1 次 | 0 次（ADR-8） |
| 8 | 应用进程 runtime InitSourceMap/模块加载 | BMS | GetBundleInfoForSelf/GetHapModuleInfo | 1~2 次 | 0 次（ADR-9，命中映射） |

典型场景合计：变更前 8~12 次 → 变更后 ≤5 次（满足 spec ≥40% 降幅指标；以打点实测为准）。

### 线程与并发模型

| 操作 | 发起线程 | 回调线程 | 跨进程边界 | 线程安全 | 重入约束 |
|------|----------|----------|------------|----------|----------|
| thread_local 启动缓存读写 | AMS 主事件线程 | — | 否 | thread_local 天然隔离 | StartAbilityInfoWrap RAII 管理生命周期 |
| AppRunningRecord::bundleInfo_ 读 | AppMgr 主线程/任务线程 | — | 否 | 沿用 appRunningManager_ 既有锁；持有为 shared_ptr<const>，写时整体替换 | 替换仅发生在 UpdateApplicationInfoInstalled |
| ModuleRunningRecord::hapModuleInfo_ 读 | AppMgr 主线程 | — | 否 | 同上 | 同上 |
| OHOSApplication hapModuleInfo 映射 | 应用主线程 | — | 否 | 主线程单线程访问 | HandleLaunchApplication 时填充 |

**并发场景：**

| 场景 | 竞争对象 | 保护机制 | 预期行为 |
|------|----------|----------|----------|
| 应用更新与启动并发 | AppRunningRecord::bundleInfo_ | shared_ptr<const> 整体替换 + 既有锁；更新事件后新启动读新值，进行中启动用旧值完成（与现状查询时序语义等价） | 不 crash、不读半更新数据 |
| 多用户并发启动 | 复用键值 (bundleName, appIndex, userId) | 键值维度校验不匹配即回退直查 | 不串用户数据（AC-9） |

### 资源所有权矩阵

| 资源 | 创建方 | 持有方 | 销毁触发 | 实际释放 | 异常回收 |
|------|--------|--------|----------|----------|----------|
| BundleInfo（启动时查询结果） | AppMgrServiceInner::LoadAbility | AppRunningRecord（shared_ptr<const>） | 进程 record 销毁 | AppRunningRecord 析构 | record 异常移除时随析构释放 |
| HapModuleInfo（模块级） | GetBundleAndHapInfo | ModuleRunningRecord | 模块 record 销毁 | ModuleRunningRecord 析构 | 同上 |
| HapModuleInfo 映射（应用进程） | MainThread::HandleLaunchApplication | OHOSApplication | 应用退出 | OHOSApplication 析构 | 进程退出回收 |
| ApplicationInfo（应用进程唯一指针） | MainThread | MainThread/ContextImpl/OHOSApplication 共享 | 应用退出或整体替换 | shared_ptr 计数归零 | 进程退出回收 |

### 异常传播时序图

| 异常场景 | 触发层 | 传播路径 | 最终处理 |
|----------|--------|----------|----------|
| 复用数据为空/维度不匹配 | 服务/框架各层 | 就地回退为直查 BMS 的原有调用 | 行为与变更前一致，TAG_LOGW（AC-8） |
| BMS 查询失败 | 各层 | 沿用现有错误处理（错误码/日志） | 与变更前一致，无新错误码 |
| 包更新发生在复用窗口内 | 服务层 | 更新事件刷新持有值；进行中启动用启动时刻数据完成 | 与现状"查询发生在不同时刻"语义等价 |

### 安全基础检查

> proposal「安全与权限」= 是（受限：权限校验路径去重需保证语义不变）。

#### 信任边界交叉分析

| 边界类型 | 跨越的交互 | 风险 | 约束 |
|---|---|---|---|
| 本地/远程 | 应用进程→foundation 的 BMS 查询减少 | 无新增信任边界；减少的查询均为读操作 | 不新增任何跨进程接口 |
| 不同 SELinux 域 | 无变化 | — | — |

#### 基础安全要求

| 检查项 | 结论 | 说明/措施 |
|---|---|---|
| 输入验证 | 复用前校验维度键值 (bundleName, appIndex, userId) 与数据非空 | 不匹配即回退直查（AC-8/AC-9） |
| 错误处理 | 不新增错误码；回退路径均有日志 | 沿用 ERR_* 体系 |
| 敏感数据处理 | 包信息（含签名）仅在既有持有范围内复用，不新增日志打印敏感字段 | 打点日志只含 bundleName/计数 |
| 权限校验语义 | 校验类路径仅同函数内去重，校验输入不跨阶段复用 | spec R-5 / AC-6 单测逐点比对 |

### 深度威胁分析(如需)

未命中高风险判据（无敏感数据新增暴露面、无网络暴露、无认证授权变更、无合规项）：本需求为只读数据的进程内/进程间复用去重，不产出 threat-model.md。

## 风险和开放问题

| 项 | 类型 | 影响 | 处理方式 | Owner |
|----|------|------|----------|-------|
| S1（AbilityRequest::appInfo 双份存储）一期不做 | 架构 | 低 | ADR-13 记录，二期结合 AMS 结构治理评估 | 待定 |
| ADR-7 的 processName 一致性风险 | 技术 | — | **已消除**：ADR-7 修正为轻量查询替代，不再涉及调用方计算 processName | — |
| 量化收益依赖实测 | 测试 | 低 | TASK-1 打点先行，优化前后对比 | 待定 |
| 应用更新与进行中启动的复用窗口 | 技术 | 低 | shared_ptr<const> 整体替换，语义与现状等价（见并发场景） | 待定 |

## 设计审批

- [x] 需求基线已确认，设计覆盖 P0/P1 AC（AC-2~6、AC-11~13 均有 ADR 对应）
- [x] 不涉及项已承接，N/A 和展开项都有结论
- [x] 涉及仓和模块职责清楚
- [x] 调用链层级分析完整，每层覆盖到位
- [x] 适用架构规则已识别并形成设计结论
- [x] 分层和子系统边界合规
- [x] API 变更有签名、权限、错误码和兼容性说明（InnerAPI 两接口可选参数）
- [x] BUILD.gn/bundle.json 影响明确（仅测试源文件）
- [x] 设计输出和后续 Task 拆分明确
- [x] 关键设计决策有理由和影响说明
- [x] 风险和开放问题有 Owner（待定，排期时指派）

**结论:** 通过（待 Owner 评审确认）
