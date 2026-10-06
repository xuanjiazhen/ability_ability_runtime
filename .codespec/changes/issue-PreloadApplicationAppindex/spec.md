# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 分身应用预加载支持（JS 接口 appManager.preloadApplication） |
| 特性编号 | FEAT-30095352 |
| 优先级 | P1 |
| 复杂度 | 简单（≤5 人月），引用 proposal.md §2 分级判断 |
| 目标版本 | OpenHarmony 7.1 |

## 用户故事或场景

### US-1: 预加载分身应用

**作为** 应用开发者，**我想要** 通过 JS 接口 `appManager.preloadApplication` 传入 `appIndex` 预加载分身应用，**以便** 分身应用启动时可复用预加载进程加速启动。

**验收标准：**

- **AC-1:** Given PC 平板分身应用已安装且 appIndex=PC_TABLET_INDEX(10000) 存在、调用方为系统应用且持有预加载权限（`VerifyPreloadApplicationPermission()=true`） When 调用 JS 接口 `appManager.preloadApplication(bundleName, userId, preloadMode, appIndex=PC_TABLET_INDEX)` Then 返回 `ERR_OK`（0），且通过 AppMgr inner API 可查询到该 appIndex 对应的分身预加载进程记录存在
- **AC-4:** Given appIndex=PC_TABLET_INDEX(10000) 的分身应用已预加载成功（预加载进程存在） When 正式启动该分身应用（`StartAbility` 携带相同 appIndex） Then 预加载进程被复用，`isPreloadStart` 复用标志生效，跳过完整进程初始化流程

### US-2: 多分身预加载记录互不冲突

**作为** 系统开发者，**我想要** 不同 appIndex 的预加载记录互不冲突，**以便** 支持同时预加载多个分身实例。

**验收标准：**

- **AC-2:** Given appIndex=1 的分身已预加载成功且记录存在 When 预加载同一 bundleName 的 appIndex=PC_TABLET_INDEX(10000) 分身 Then 返回 `ERR_OK`，appIndex=1 和 appIndex=10000 两条预加载记录均存在且互不覆盖
- **AC-3:** Given appIndex=PC_TABLET_INDEX(10000) 的分身已预加载成功 When 再次调用 `appManager.preloadApplication(bundleName, userId, preloadMode, appIndex=PC_TABLET_INDEX)` Then 返回 `ERR_PRELOAD_APP_RECORD_ALREADY_EXIST`（2097383），不创建重复进程
- **AC-6:** Given 无任何预加载记录 When 并发调用 `appManager.preloadApplication(bundleName, userId, preloadMode, appIndex=1)` 和 `appManager.preloadApplication(bundleName, userId, preloadMode, appIndex=PC_TABLET_INDEX)` Then 均返回 `ERR_OK`，产生两条独立的预加载记录

### US-3: 主应用预加载向后兼容

**作为** 系统开发者，**我想要** appIndex=0 的主应用预加载行为不受影响，**以便** 保证向后兼容。

**验收标准：**

- **AC-5:** Given appIndex=0 的主应用、调用方为系统应用且持有预加载权限 When 调用 `appManager.preloadApplication(bundleName, userId, preloadMode, appIndex=0)` Then 返回 `ERR_OK`，行为与修改前一致（预加载流程、进程复用、错误码均不变）

## 业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 预加载支持的 appIndex 有效范围为 [0, MAX_APP_CLONE_INDEX(1000)] ∪ {PC_TABLET_INDEX(10000)}：0 表示主应用，1-1000 表示普通分身，10000 表示 PC 平板分身 | 当前 `AppMgrServiceInner::PreloadApplication`（app_mgr_service_inner.cpp:668）`if (appIndex != 0 && appIndex != PC_TABLET_INDEX) return ERR_INVALID_VALUE` 阻断 [1,1000]；需移除该阻断并替换为范围校验：仅拒绝 appIndex < 0 或 appIndex > 1000 且 ≠ 10000 | AC-1, AC-2 |
| BR-2 | 不同 appIndex 的预加载记录相互独立，以 (bundleName, userId, appIndex) 三元组唯一标识 | 当前 JS 接口调用链（AppMgrServiceInner::PreloadApplication line 677 → GeneratePreloadRequest → HandlePreloadApplication）无预加载记录去重检查；需在该路径补充去重逻辑，复用 `CheckPreloadAppRecordExist` 或等价机制 | AC-2, AC-3, AC-6 |
| BR-3 | appIndex > 0 时须查询分身专属 BundleInfo/ApplicationInfo，不可使用主应用信息替代 | `AppPreloader::GetBundleAndHapInfo`（app_preloader.cpp:216-218）appIndex > 0 时调用 `GetCloneBundleInfo` 获取分身信息 | AC-1, AC-4 |
| BR-4 | 权限校验逻辑不变：所有 appIndex 均经过 `CHECK_CALLER_IS_SYSTEM_APP` + `VerifyPreloadApplicationPermission` | 无权限返回 `ERR_PERMISSION_DENIED`（201）；非系统应用返回调用方校验错误 | AC-1, AC-5 |

## 异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-1 | 分身应用未安装 | appIndex>0 但包管理无对应分身 BundleInfo（`GetCloneBundleInfo` 失败） | 返回 `GET_BUNDLE_INFO_FAILED`（`GeneratePreloadRequest` 内 `GetBundleAndHapInfo` 返回 false） | AC-1 |
| EX-2 | 非系统应用调用 | 调用方未通过 `CHECK_CALLER_IS_SYSTEM_APP` | 返回系统应用校验错误（JS 接口仅允许系统应用调用） | AC-1, AC-5 |
| EX-3 | appIndex 超出有效范围 | appIndex < 0 或 appIndex > 1000 且 appIndex ≠ PC_TABLET_INDEX(10000) | 返回 `ERR_INVALID_VALUE`（401）（校验位于 `AppMgrServiceInner::PreloadApplication` line 656 范围校验） | AC-1 |
| EX-4 | 预加载阶段无效 | `appPreloadPhase` 不在有效枚举范围 | 返回 `ERR_INVALID_APP_PRELOAD_PHASE`（2097388） | AC-1 |
| EX-5 | 预加载记录已存在 | 同一 (bundleName, userId, appIndex) 已有预加载记录 | 返回 `ERR_PRELOAD_APP_RECORD_ALREADY_EXIST`（2097383） | AC-3 |
| EX-6 | ResSched 不允许预加载 | `appPreloader_->PreCheck` 返回 false（`IsAllowedAppPreload` 拒绝） | 返回 `ERR_NOT_ALLOW_PRELOAD_BY_RSS` | AC-1 |

> **注**：JS 接口调用链 `AppMgrServiceInner::PreloadApplication`（line 656）不经过 `IsPreloadApplicationEnabled()` 检查（该检查仅在 `PreloadApplicationByPhase` line 647 和 AMS 路径 PreloadManagerService 中存在）。本需求不新增该检查到 line 656。

## 错误码定义

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERR_OK | 0 | 操作成功 | AC-1, AC-2, AC-4, AC-5, AC-6 |
| ERR_INVALID_VALUE | 401 | 参数无效（appIndex 超出有效范围：appIndex < 0 或 appIndex > 1000 且 ≠ PC_TABLET_INDEX） | AC-1, EX-3 |
| ERR_PERMISSION_DENIED | 201 | 权限校验失败（`VerifyPreloadApplicationPermission` 返回 false） | AC-1, AC-5 |
| ERR_PRELOAD_APP_RECORD_ALREADY_EXIST | 2097383 | 预加载记录已存在（同一 appIndex 重复预加载） | AC-3, EX-5 |
| ERR_INVALID_APP_PRELOAD_PHASE | 2097388 | 预加载阶段无效 | AC-1, EX-4 |
| GET_BUNDLE_INFO_FAILED | — | 获取 BundleInfo 失败（分身未安装、`GetCloneBundleInfo` 失败） | AC-1, EX-1 |
| ERR_NOT_ALLOW_PRELOAD_BY_RSS | — | ResSched 不允许预加载 | AC-1, EX-6 |

## API 规格定义

不涉及：JS 接口 `appManager.preloadApplication` 签名已存在于 `frameworks/js/napi/app/js_app_manager/js_app_manager.cpp:1870`（`BindNativeFunction(env, exportObj, "preloadApplication", ...)`），`appIndex` 参数已预留并透传至 `IAppMgr::PreloadApplication(bundleName, userId, preloadMode, appIndex)`。本次需求仅移除服务端 `AppMgrServiceInner::PreloadApplication`（app_mgr_service_inner.cpp:668）的 `appIndex != 0 && appIndex != PC_TABLET_INDEX` 限制并补齐分身查询逻辑，不涉及公共 API 签名变更、错误码数值变更或生命周期语义变更。

## 接口变更分析

### 新增接口

无。

### 变更/废弃接口

无。本次变更不修改任何接口签名；JS 接口 `appManager.preloadApplication` 的 `appIndex` 参数已存在，仅行为从「appIndex ∈ [1,1000] 返回 `ERR_INVALID_VALUE`」变为「appIndex [0, 1000] ∪ {PC_TABLET_INDEX} 正常预加载」。PC_TABLET_INDEX(10000) 在 line 668 原本已放行，本次变更新增对 [1, 1000] 普通分身的支持。

## 兼容性声明

- **已有 API 行为变更:** 是 — JS 接口 `appManager.preloadApplication` 的 `appIndex` 参数行为变更：appIndex ∈ [1, 1000] 从返回 `ERR_INVALID_VALUE`（line 668 `if (appIndex != 0 && appIndex != PC_TABLET_INDEX)`）变为正常预加载。该变更是向后兼容的（appIndex=0 行为不变，PC_TABLET_INDEX 原本已支持，appIndex ∈ [1,1000] 从拒绝变为支持）。
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | BR-1, BR-3, BR-4 | 单元测试：调用 `AppMgrServiceInner::PreloadApplication(bundleName, userId, preloadMode, appIndex=PC_TABLET_INDEX)`，断言返回 ERR_OK；mock `GetCloneBundleInfo` 返回分身 BundleInfo | UT 返回值 + 分身 BundleInfo 使用断言 |
| AC-2 | BR-2 | 单元测试：先预加载 appIndex=1，再预加载 appIndex=PC_TABLET_INDEX(10000)，断言均返回 ERR_OK 且两条记录独立存在 | UT 双记录断言 |
| AC-3 | BR-2 | 单元测试：预加载 appIndex=PC_TABLET_INDEX(10000) 后再次预加载同一 appIndex，断言返回 ERR_PRELOAD_APP_RECORD_ALREADY_EXIST | UT 错误码断言 |
| AC-4 | BR-3 | 单元测试：预加载 appIndex=PC_TABLET_INDEX(10000) 后正式启动分身，断言 isPreloadStart 复用标志生效 | UT 复用标志断言 |
| AC-5 | BR-4 | 回归测试：调用 appIndex=0，断言行为与修改前一致（返回 ERR_OK，预加载流程不变） | 回归 UT |
| AC-6 | BR-2 | 单元测试：并发预加载 appIndex=1 和 appIndex=PC_TABLET_INDEX(10000)，断言均 ERR_OK 且记录独立 | UT 并发断言 |

## 测试设计提示

| AC | 测试类型 | 测试文件 | 测试名称 | 输入/触发 | 期望输出/错误 | Red 条件 |
|----|----------|----------|----------|-----------|---------------|----------|
| AC-1 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_PCTabletIndex_Success | appIndex=PC_TABLET_INDEX(10000), mock GetCloneBundleInfo 返回分身 BundleInfo | ERR_OK | 实现前 line 668 已放行 PC_TABLET_INDEX，但 GetBundleAndHapInfo 调用 GetBundleInfo（非 GetCloneBundleInfo），分身信息不正确；需 TASK-2 的 GetCloneBundleInfo 适配 |
| AC-2 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_DifferentAppIndex_NoConflict | 先 appIndex=1 再 appIndex=PC_TABLET_INDEX | 均 ERR_OK | 实现前 appIndex=1 被 line 668 `appIndex != 0 && appIndex != PC_TABLET_INDEX` 阻断返回 ERR_INVALID_VALUE |
| AC-3 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_SameAppIndex_AlreadyExist | appIndex=PC_TABLET_INDEX 重复预加载 | ERR_PRELOAD_APP_RECORD_ALREADY_EXIST | 实现前 line 677 路径无去重检查（CheckPreloadAppRecordExist 未在该路径调用），需补充去重逻辑 |
| AC-4 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_Clone_ProcessReuse | appIndex=PC_TABLET_INDEX 预加载后启动 | isPreloadStart=true | 实现前分身预加载信息不正确导致进程复用失败 |
| AC-5 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_AppIndex0_Unchanged | appIndex=0 | ERR_OK，行为不变 | 此 AC 不依赖实现变更（appIndex=0 原本就支持），应可直接通过 |
| AC-6 | UT | app_mgr_service_inner_test.cpp | PreloadApplication_Concurrent_DifferentAppIndex | 并发 appIndex=1, PC_TABLET_INDEX | 均 ERR_OK | 实现前 appIndex=1 被 line 668 阻断返回 ERR_INVALID_VALUE |

> AC → 实现文件 + Task + 验证状态的映射见 `execution-plan.md`「AC 到 Task 追溯」+「代码范围映射」。
