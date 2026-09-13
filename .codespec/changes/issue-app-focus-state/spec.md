# 特性规格：获焦应用变化通知

> 本规格覆盖新增 `onAppFocusStateChange` / `offAppFocusStateChange` Public API 的全链路行为。范围限定：焦点变化通知的注册/注销/回调链路（observerId 成对注册/解注册）、`FocusStateData` 数据结构、跨进程 IPC 接口、window_manager 回调签名变更。MDM 应用的输入拦截策略实现、C API 声明不在本规格范围内。

> **接口设计要点：** (1) 注册和解注册成对调用，使用直接方法命名 `onAppFocusStateChange(observer): int` / `offAppFocusStateChange(observerId: int): void`（非 `on(type, ...)` 字符串分发），`onAppFocusStateChange` 返回 observerId，`offAppFocusStateChange` 接收 observerId 精确注销单个回调，不提供解注册全部回调的接口，错误通过 throw 抛出；(2) 动静态归一，仅提供一套接口，同时适用于 dynamic 和 static。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 获焦应用变化通知 |
| 特性编号 | FEAT-FOCUS-001 |
| 所属 Epic | MDM 焦点感知能力 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability / Sig-Window |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。新增独立的 `IFocusStateObserver` IPC 接口，与现有 `IAppForegroundStateObserver` 并行存在。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `onAppFocusStateChange` / `offAppFocusStateChange` Public JS API | 应用侧注册/注销获焦变化监听（直接方法命名，`on` 返回 observerId，`off` 接收 observerId 精确注销） |
| ADDED | `IFocusStateObserver` IPC 接口（Stub/Proxy） | 焦点变化观察者跨进程通知 |
| ADDED | `FocusStateData` 通知数据结构 | focusedUid / unfocusedUid / bundleName |
| ADDED | `RegisterFocusStateObserver` / `UnregisterFocusStateObserver` IPC 方法 | IAppMgr 接口新增 |
| ADDED | IPC 事务码 `REGISTER_FOCUS_STATE_OBSERVER` / `UNREGISTER_FOCUS_STATE_OBSERVER` / `ON_FOCUS_STATE_CHANGED` | IAppMgr::Message 枚举新增项 |
| ADDED | `JSFocusStateObserver` NAPI 桥接 | js_app_manager.cpp 新增 `OnAppFocusStateChange` / `OffAppFocusStateChange` 直接方法导出（非 `on(type, ...)` 字符串分发） |
| ADDED | `AppFocusStateObserver.d.ts` observer 接口类型 | interface_sdk-js 新增 |
| MODIFIED | `IFocusChangedListener::OnFocused` 回调签名 | window_manager 增加 unfocusedUid 和 displayId 参数 |
| MODIFIED | `WindowFocusChangedListener::OnFocused` 签名适配 | ability_runtime 侧适配 |
| MODIFIED | `AppMgrServiceInner::HandleFocused` | 增加主屏幕组判断 + uid 比较 + 异步通知逻辑 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260908584366-05-proposal.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证：

### 发现 1：IAppForegroundStateObserver 是完美的 6 文件 IPC 模板

proposal 假设 `IAppForegroundStateObserver` 可作为新 observer 的 IPC 模板——**完全正确**。完整 IPC 栈已验证：

| 层 | 文件 | 说明 |
|----|------|------|
| 接口 | `app_foreground_state_observer_interface.h` | 37 行极简接口，1 个虚方法 + 1 个 Message 枚举 |
| Stub | `app_foreground_state_observer_stub.h` / `.cpp` | `IRemoteStub` 模板，dispatch `ON_APP_STATE_CHANGED` |
| Proxy | `app_foreground_state_observer_proxy.h` / `.cpp` | `IRemoteProxy` 模板，send `ON_APP_STATE_CHANGED` |
| 公共 API | `app_mgr_interface.h:864,871` | `RegisterAppForegroundStateObserver` / `UnregisterAppForegroundStateObserver` 虚方法 |
| Proxy API | `app_mgr_proxy.h:678,685` / `.cpp:2102,2119` | 客户端 Proxy 实现 |
| Stub 分发 | `app_mgr_stub.cpp:1848,1863` | `iface_cast<IAppForegroundStateObserver>` |
| Manager | `app_state_observer_manager.cpp:170-219`（注册/注销）+ `:314`（`OnAppStateChanged` 通知）+ `:414`（`OnProcessStateChanged` 通知） | 注册/注销/通知全链路 |

### 发现 2：HandleFocused 当前仅在前台状态时通知

`AppMgrServiceInner::HandleFocused`（`app_mgr_service_inner.cpp:9110`）当前实现：

```cpp
void AppMgrServiceInner::HandleFocused(const sptr<FocusChangeInfo> &focusChangeInfo)
{
    // ... null checks ...
    auto appRecord = GetAppRunningRecordByPid(focusChangeInfo->pid_);
    if (!appRecord) { return; }
    if (!appRecord->UpdateAbilityFocusState(focusChangeInfo->abilityToken_, true)) {
        return;  // 仅 Ability 焦点变化，不通知进程/应用层
    }
    bool needNotifyApp = appRunningManager_->IsApplicationFirstFocused(*appRecord);
    if (appRecord->GetState() == ApplicationState::APP_STATE_FOREGROUND) {  // ← 关键条件
        OnAppStateChanged(appRecord, ApplicationState::APP_STATE_FOREGROUND, needNotifyApp, true, false);
    }
    DelayedSingleton<AppStateObserverManager>::GetInstance()->OnProcessStateChanged(appRecord, true);
}
```

**关键约束：** 当前 `HandleFocused` 仅在 `appRecord->GetState() == APP_STATE_FOREGROUND` 时才通过 `OnAppStateChanged` 通知。新的 focus state 通知**不应**复用 `OnAppStateChanged` 路径（否则会在非前台状态下也触发，改变现有行为），而应作为独立的 `NotifyFocusChanged` 路径。

### 发现 3：FocusChangeInfo 不含 unfocusedUid

`FocusChangeInfo`（来自 `OHOS::Rosen` 命名空间，window_manager）当前暴露 `uid_`、`pid_`、`abilityToken_` 字段，但**不含 `unfocusedUid` 或 `displayId`**。proposal 的开放问题 C-02（`FocusChangeInfo` 是否需新增字段 vs 通过回调参数传递）是实际的设计决策点。

### 发现 4：所有焦点代码受 SUPPORT_SCREEN 宏约束

`HandleFocused`、`HandleUnfocused`、`WindowFocusChangedListener`、`InitFocusListener`、`RegisterFocusChangedListener` 均在 `#ifdef SUPPORT_SCREEN` 内。新特性须保持此宏约束。

### 发现 5：NAPI 层已有方法导出模式

`js_app_manager.cpp` 的 `OnOn`（:270）/ `OnOff`（:404）按字符串类型分发：

```cpp
constexpr const char* ON_OFF_TYPE = "applicationState";
constexpr const char* ON_OFF_TYPE_APP_FOREGROUND_STATE = "appForegroundState";
constexpr const char* ON_OFF_TYPE_ABILITY_FIRST_FRAME_STATE = "abilityFirstFrameState";
```

但本特性**不使用** `on(type, ...)` 字符串分发模式，而是参照 `appManager.onApplicationStateChange(observer): int` / `appManager.offApplicationStateChange(observerId): void` 的直接方法命名模式，在 `js_app_manager.cpp` 中新增 `OnAppFocusStateChange` / `OffAppFocusStateChange` NAPI 方法导出。`OnAppFocusStateChange` 返回 observerId，`OffAppFocusStateChange` 接收 observerId 精确注销单个回调。

## 用户故事

### US-1: MDM 应用注册获焦变化监听

**作为** MDM 应用开发者
**我想要** 通过 public API `onAppFocusStateChange(observer)` 注册获焦应用变化监听（需 RUNNING_STATE_OBSERVER 权限），获得 observerId 用于后续解注册
**以便** 在用户切换到危险应用时收到通知并执行拦截策略

### US-2: 跨应用切换时收到通知

**作为** MDM 应用开发者
**我想要** 系统在用户从应用 A 切换到应用 B 时通知我
**以便** 感知获焦应用变化并判断是否需要拦截

### US-3: 同应用内窗口切换不通知

**作为** MDM 应用开发者
**我想要** 同一应用内窗口切换时不被通知
**以便** 避免冗余通知带来的性能开销和误拦截

### US-4: window_manager 提供失焦信息

**作为** 系统开发者
**我想要** window_manager 在 OnFocused 回调中同时提供获焦/失焦窗口信息
**以便** ability_runtime 无需缓存 uid 即可完成比较

## 验收标准 (AC)

### P0 — 核心路径

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 应用持 `ohos.permission.RUNNING_STATE_OBSERVER` 权限调用 `onAppFocusStateChange(observer)` THEN observer 被 AppStateObserverManager 收录，注册成功返回 `observerId`（int） | 正常 | US-1 |
| AC-2 | WHEN 应用无 `RUNNING_STATE_OBSERVER` 权限调用 `onAppFocusStateChange(observer)` THEN 返回权限拒绝错误码（throw BusinessError 201），observer 不被注册 | 异常 | US-1 |
| AC-3 | WHEN window_manager 的 `OnFocused` 回调触发 THEN 回调参数含 `focusChangeInfo.uid_`、`unfocusedUid`、`displayId` | 正常 | US-4 |
| AC-4 | WHEN 跨应用切换（获焦 uid ≠ 失焦 uid）且在主屏幕组 THEN `onFocusStateChanged` 回调被触发，`FocusStateData` 含正确的 `focusedUid`、`unfocusedUid`、`focusedBundleName`、`unfocusedBundleName` | 正常 | US-2 |
| AC-5 | WHEN 同应用内窗口切换（获焦 uid == 失焦 uid） THEN `onFocusStateChanged` 不被回调 | 边界 | US-3 |
| AC-6 | WHEN 应用调用 `offAppFocusStateChange(observerId)` THEN observer 从 AppStateObserverManager 移除，不再收到回调；不提供解注册全部回调的接口 | 正常 | US-1 |

### P1 — 边界路径

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-7 | WHEN 非主屏幕组的窗口焦点变化 THEN `onFocusStateChanged` 不被回调（仅主屏幕组 uid 变化才通知） | 边界 | US-2 |
| AC-8 | WHEN observer 所在进程死亡 THEN AppStateObserverManager 的 death recipient 自动移除该 observer，不影响其他 observer | 异常 | US-1 |
| AC-9 | WHEN `HandleFocused` 被调用但 `appRecord` 为空 THEN 不发生崩溃，记 `TAG_LOGW` 日志，不触发通知 | 异常 | US-2 |
| AC-10 | WHEN 设备不支持屏幕（`SUPPORT_SCREEN` 未定义）THEN `onAppFocusStateChange` 返回不支持错误码，不注册 listener | 恢复 | US-1 |
| AC-11 | WHEN 现有 `on('applicationState')` / `on('appForegroundState')` 被调用 THEN 行为不变，不受新特性影响 | 兼容 | — |
| AC-12 | WHEN 现有 `HandleFocused` 中的 `OnAppStateChanged` / `OnProcessStateChanged` 路径被触发 THEN 现有行为不变，新通知路径独立运行 | 兼容 | — |
| AC-13 | WHEN 同一进程多次调用 `onAppFocusStateChange(observer)` 注册不同 observer 对象 THEN 每次注册成功并返回独立 `observerId`，框架侧为每个 observer 维护独立 observeId，焦点变化时各 observer 均收到回调 | 正常 | US-1 |
| AC-14 | WHEN 同一应用注册的 observer 进程数达到 10 个上限，第 11 个进程调用 `onAppFocusStateChange(observer)` THEN 返回错误码 `16000165`（当前应用注册数量超过限制），注册被拒绝 | 边界 | US-1 |
| AC-15 | WHEN `displayId` 为 -1 或无效值 THEN 不触发 `NotifyFocusChanged`，记 `TAG_LOGW` 日志（异常 displayId 视为非主屏幕组） | 异常 | US-2 |
| AC-16 | WHEN 焦点从非主屏幕组切换到主屏幕组 THEN 触发 `NotifyFocusChanged`（跨屏幕组切换到主屏幕组视为有效获焦） | 正常 | US-2 |
| AC-17 | WHEN `unfocusedUid` 为 -1（首次获焦，无前一个获焦应用）THEN `unfocusedBundleName` 为空字符串，仍触发通知 | 边界 | US-2 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + 集成 | `app_state_observer_manager_test/` |
| AC-2 | R-2 | 单测 | `app_state_observer_manager_test/` |
| AC-3 | R-3 | 单测 + 跨仓联调 | `window_focus_changed_listener_test/` |
| AC-4 | R-4 | 端到端集成 | `appmgr/test/` |
| AC-5 | R-5 | 集成 | `appmgr/test/` |
| AC-6 | R-6 | 单测 | `js_app_manager` NAPI 单测 |
| AC-7 | R-7 | 集成 | `appmgr/test/` |
| AC-8 | R-8 | 单测 | `app_state_observer_manager_test/` |
| AC-9 | R-9 | 单测 | `appmgr/test/unittest/` |
| AC-10 | R-10 | 单测 | 条件编译验证 |
| AC-11 | R-11 | 回归测试 | `js_app_manager` 现有单测 |
| AC-12 | R-11 | 回归测试 | `appmgr/test/unittest/` |
| AC-13 | R-12 | 单测 + 集成 | `js_app_manager` NAPI 单测（多 observer 分发） |
| AC-14 | R-13 | 单测 + 集成 | `app_state_observer_manager_test/`（同应用 11 进程注册拒绝） |
| AC-15 | R-14 | 单测 | `appmgr/test/unittest/`（displayId=-1） |
| AC-16 | R-15 | 集成 | `appmgr/test/`（非主屏幕组→主屏幕组切换） |
| AC-17 | R-4 | 集成 | `appmgr/test/`（首次获焦 unfocusedUid=-1） |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 持 `RUNNING_STATE_OBSERVER` 权限 + 有效 observer 对象 | `AppStateObserverManager` 通过 `VerifyAppStateObserverPermission` 校验后收录 observer，加入 `focusStateObserverMap_`，绑定 death recipient | **支持重复注册**：同一进程可注册多个 observer 对象，框架侧（NAPI）为每个 observer 维护独立的 observeId 并分别回调；服务侧每进程仅维护一个 IPC observer 代理，由框架侧多路分发。同一应用注册上限 10 个进程，超出返回 `16000165` | AC-1, AC-13, AC-14 |
| R-2 | 异常 | 无 `RUNNING_STATE_OBSERVER` 权限 | 返回 `ERR_PERMISSION_DENIED`，observer 不被收录 | 权限校验复用 `PermissionVerification::VerifyAppStateObserverPermission` | AC-2 |
| R-3 | 行为 | window_manager 焦点变化 | `IFocusChangedListener::OnFocused` 回调参数含 `FocusChangeInfo.uid_`（获焦）、`unfocusedUid`（失焦）、`displayId`（屏幕组） | 需 window_manager 侧修改回调签名 | AC-3 |
| R-4 | 行为 | 获焦 uid ≠ 失焦 uid + 主屏幕组 | `AppMgrServiceInner::HandleFocused` 中比较 uid，不同则通过 `AppStateObserverManager::NotifyFocusChanged` 异步通知所有注册 observer，`FocusStateData` 含 focusedUid/unfocusedUid/focusedBundleName/unfocusedBundleName | bundleName 由服务侧 uid 解析；通知异步投递到 TaskHandler | AC-4 |
| R-5 | 边界 | 获焦 uid == 失焦 uid（同应用窗口切换） | 不触发 `NotifyFocusChanged` | uid 比较在 `HandleFocused` 中执行 | AC-5 |
| R-6 | 行为 | 调用 `offAppFocusStateChange(observerId)` | 从 `focusStateObserverMap_` 移除 `observerId` 对应的 observer；解绑 death recipient；**不提供**解注册全部回调的接口 | 参照 `appManager.offApplicationStateChange(observerId)` 模式；错误通过 throw 抛出 | AC-6 |
| R-7 | 边界 | 焦点变化发生在非主屏幕组 | 不触发 `NotifyFocusChanged` | 主屏幕组判断由 `displayId` 决定 | AC-7 |
| R-8 | 异常 | observer 所在进程死亡 | death recipient 回调移除该 observer | 不影响其他 observer | AC-8 |
| R-9 | 异常 | `HandleFocused` 中 `GetAppRunningRecordByPid` 返回空 | 记 `TAG_LOGW`，不崩溃，不触发通知 | null check 在入口执行 | AC-9 |
| R-10 | 恢复 | `SUPPORT_SCREEN` 未定义 | `onAppFocusStateChange` 返回不支持错误码，不注册 listener | 所有焦点代码在 `#ifdef SUPPORT_SCREEN` 内 | AC-10 |
| R-11 | 兼容 | 现有 `on('applicationState')` / `on('appForegroundState')` / `HandleFocused` 中 `OnAppStateChanged` 路径 | 现有行为逐行不变，新通知路径独立运行 | 新旧路径代码隔离 | AC-11, AC-12 |
| R-12 | 行为 | 同一进程多次调用 `onAppFocusStateChange(observer)` 注册不同 observer 对象 | 框架侧（NAPI `JSFocusStateObserver`）为每个 observer 分配独立 observeId，`onAppFocusStateChange` 返回 observerId；收到服务侧回调后逐个分发；服务侧（`AppStateObserverManager`）每进程仅维护一个 IPC observer 代理 | 框架侧多路分发，服务侧单进程单代理 | AC-13 |
| R-13 | 边界 | 同一应用已注册 observer 进程数 ≥ 10 | 返回错误码 `16000165`（当前应用注册数量超过限制），拒绝该应用新进程注册 | **同一应用**注册上限 10 个进程（非全局上限），按调用方 bundleName 统计；含已注册进程死亡前的竞态窗口 | AC-14 |
| R-14 | 异常 | `displayId` 为 -1 或无效值 | 不触发 `NotifyFocusChanged`，记 `TAG_LOGW` 日志 | 异常 displayId 视为非主屏幕组，安全降级 | AC-15 |
| R-15 | 边界 | 焦点从非主屏幕组切换到主屏幕组（获焦窗口进入主屏幕组） | 触发 `NotifyFocusChanged`（跨屏幕组切换到主屏幕组视为有效获焦） | 与 AC-7（非主屏幕组不通知）互补：进入主屏幕组时通知 | AC-16 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|----------|---------|
| `appManager.onAppFocusStateChange(observer)` | Public | `observer: AppFocusStateObserver` | `int`（observerId） | 注册获焦变化监听，返回 observerId 用于 `offAppFocusStateChange` 解注册 | AC-1, AC-2 |
| `appManager.offAppFocusStateChange(observerId)` | Public | `observerId: int` | `void` | 注销获焦变化监听（通过 observerId 精确注销单个回调），错误通过 throw 抛出 | AC-6 |
| `AppFocusStateObserver` 接口类型 | Public | — | — | observer 接口类型定义 | AC-1 |

### 变更 API（Inner API / 跨仓）

| API | 变更类型 | 说明 | 关联 AC |
|-----|----------|------|---------|
| `IFocusChangedListener::OnFocused` | 签名修改（window_manager） | 增加 `unfocusedUid` 和 `displayId` 参数 | AC-3 |
| `WindowFocusChangedListener::OnFocused` | 签名适配（ability_runtime） | 适配新签名 | AC-3 |
| `AppMgrServiceInner::HandleFocused` | 扩展 | 新增主屏幕组判断 + uid 比较 + `NotifyFocusChanged` 通知 | AC-4, AC-5, AC-7 |
| `IAppMgr` | 扩展 | 新增 `RegisterFocusStateObserver` / `UnregisterFocusStateObserver` 虚方法 | AC-1, AC-6 |
| `AppStateObserverManager` | 扩展 | 新增 `focusStateObserverMap_` 和 `RegisterFocusStateObserver` / `UnregisterFocusStateObserver` / `NotifyFocusChanged` | AC-1, AC-4, AC-6 |

### 废弃 API

无废弃项。

## 接口规格

### JS API

| 属性 | 值 |
|------|-----|
| 函数签名 | `onAppFocusStateChange(observer: AppFocusStateObserver): int` |
| 返回值 | `int`（observerId，用于 `offAppFocusStateChange` 解注册） |
| 开放范围 | Public |
| 权限 | `ohos.permission.RUNNING_STATE_OBSERVER` |
| 错误处理 | `throw BusinessError`（201 权限拒绝；202 非系统应用；401 参数错误；16000050 内部错误；16000165 超限） |
| d.ts 位置 | `api/@ohos.app.ability.appManager.d.ts` |
| 动静态归一 | 同一接口同时适用于 dynamic 和 static，不提供 static-only 方法 |
| 关联 AC | AC-1, AC-2 |

| 属性 | 值 |
|------|-----|
| 函数签名 | `offAppFocusStateChange(observerId: int): void` |
| 返回值 | `void` |
| 开放范围 | Public |
| 权限 | `ohos.permission.RUNNING_STATE_OBSERVER` |
| 错误处理 | `throw BusinessError`（201 权限拒绝；202 非系统应用；401 参数错误；16000050 内部错误） |
| 解注册范围 | 仅注销 `observerId` 对应的单个回调；**不提供**省略 `observerId` 注销全部的接口 |
| 动静态归一 | 同一接口同时适用于 dynamic 和 static |
| 关联 AC | AC-6 |

### AppFocusStateObserver 接口类型

```typescript
interface AppFocusStateObserver {
    onFocusStateChanged(focusStateData: FocusStateData): void;
}

interface FocusStateData {
    focusedUid: number;
    unfocusedUid: number;
    focusedBundleName: string;
    unfocusedBundleName: string;
}
```

> `focusedBundleName` 和 `unfocusedBundleName` 由服务侧通过 uid 查询 `AppRunningRecord` 解析，不依赖 window_manager 提供 bundleName。`unfocusedBundleName` 在无前一个获焦应用时为空字符串（对应 `unfocusedUid == -1`）。

### Inner API（IPC 接口）

#### IFocusStateObserver（新增 IPC 接口）

| 属性 | 值 |
|------|-----|
| 接口 | `IFocusStateObserver : IRemoteBroker` |
| 接口描述符 | `u"ohos.appexecfwk.IFocusStateObserver"` |
| 虚方法 | `void OnFocusStateChanged(const FocusStateData &data)` |
| Message 枚举 | `ON_FOCUS_STATE_CHANGED` |
| 模板参照 | `app_foreground_state_observer_interface.h`（37 行极简模板） |

#### IAppMgr 注册/注销接口

| 属性 | 值 |
|------|-----|
| 接口 | `IAppMgr::RegisterFocusStateObserver(const sptr<IFocusStateObserver> &observer)` |
| 接口 | `IAppMgr::UnregisterFocusStateObserver(const sptr<IFocusStateObserver> &observer)` |
| IPC 事务码 | `REGISTER_FOCUS_STATE_OBSERVER` / `UNREGISTER_FOCUS_STATE_OBSERVER` |
| 调用模式 | `MessageOption::TF_SYNC` |
| 服务端委托 | `AppMgrServiceInner` → `AppStateObserverManager::RegisterFocusStateObserver` |

#### FocusStateData 数据结构

| 属性 | 值 |
|------|-----|
| 类型 | `FocusStateData : public Parcelable` |
| 字段 | `int32_t focusedUid` / `int32_t unfocusedUid` / `std::string focusedBundleName` / `std::string unfocusedBundleName` |
| 序列化 | `Marshalling`: WriteInt32(focusedUid) + WriteInt32(unfocusedUid) + WriteString(focusedBundleName) + WriteString(unfocusedBundleName) |
| 反序列化 | `Unmarshalling`: ReadInt32 + ReadInt32 + ReadString + ReadString |
| bundleName 解析 | 服务侧 `HandleFocused` 中通过 `GetAppRunningRecordByUid` 将 `focusedUid`/`unfocusedUid` 解析为对应 bundleName；`unfocusedUid == -1` 时 `unfocusedBundleName` 设为空字符串 |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 持权限 + onAppFocusStateChange(observer) | 注册成功，返回 observerId | AC-1 |
| 2 | 无权限 + onAppFocusStateChange(observer) | 返回权限拒绝（throw 201） | AC-2 |
| 3 | window_manager OnFocused 回调 | 参数含 uid_/unfocusedUid/displayId | AC-3 |
| 4 | 跨应用切换 + 主屏幕组 | 通知所有 observer | AC-4 |
| 5 | 同应用窗口切换 | 不通知 | AC-5 |
| 6 | offAppFocusStateChange(observerId) | 注销对应 observerId 的回调 | AC-6 |
| 7 | 非主屏幕组 | 不通知 | AC-7 |
| 8 | observer 进程死亡 | 自动移除 | AC-8 |
| 9 | appRecord 为空 | 记日志，不崩溃 | AC-9 |
| 10 | 不支持屏幕 | 返回不支持 | AC-10 |
| 11 | 现有 on('applicationState') | 行为不变 | AC-11 |
| 12 | 现有 HandleFocused 路径 | 行为不变 | AC-12 |
| 13 | 同进程多次注册不同 observer | 框架侧分配不同 observerId，各 observer 均收到回调；服务侧每进程单代理 | AC-13 |
| 14 | 同一应用已注册 10 个进程，第 11 个注册 | reject 16000165，注册被拒 | AC-14 |
| 15 | displayId 为 -1 或无效 | 不通知，记 WARN 日志 | AC-15 |
| 16 | 焦点从非主屏幕组切换到主屏幕组 | 触发通知（进入主屏幕组有效） | AC-16 |
| 17 | 首次获焦，unfocusedUid=-1 | 仍触发通知，unfocusedBundleName 为空串 | AC-17 |

## 兼容性声明

- **已有 API 行为变更:** 否。新增 `onAppFocusStateChange` / `offAppFocusStateChange` 不修改已有 `on`/`off` 方法。现有 `HandleFocused` 中 `OnAppStateChanged` / `OnProcessStateChanged` 路径行为不变（AC-12）。
- **IPC 序列化格式变更:** 新增事务码，不影响已有事务码。`IFocusChangedListener::OnFocused` 签名变更为跨仓破坏性变更，需 window_manager 和 ability_runtime 同步修改。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。
- **最低支持版本:** 参见 proposal.md。
- **跨仓协调:** window_manager `OnFocused` 签名变更需与窗口管理团队同步（proposal 开放问题 C-01）。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| SUPPORT_SCREEN 宏约束 | 所有焦点代码在 `#ifdef SUPPORT_SCREEN` 内，新代码须保持 | AC-10 |
| 新旧通知路径隔离 | `NotifyFocusChanged` 独立于 `OnAppStateChanged`，不改变现有 `HandleFocused` 中前台状态条件通知逻辑 | AC-12 |
| observer 注册模式一致性 | 新 observer 遵循 `AppStateObserverManager` 已有 4 种 observer 的注册模式（null check → permission → existence → lock + emplace + death recipient） | AC-1, AC-8 |
| 框架侧多路 / 服务侧单代理 | 同进程可注册多个 observer（框架侧 NAPI 维护 observerId 列表），服务侧每进程仅一个 IPC observer 代理，由框架侧多路分发回调 | AC-13 |
| 同一应用注册上限 10 个进程 | 服务侧 `focusStateObserverMap_` 按调用方 bundleName 统计，同一应用上限 10 个 observer 进程，超出返回 `16000165`（当前应用注册数量超过限制）；不含已死亡但 death recipient 未触发的竞态窗口 | AC-14 |
| 主屏幕组过滤 | 仅主屏幕组 uid 变化才通知，避免多屏冗余通知；displayId=-1 或无效值视为非主屏幕组 | AC-7, AC-15, AC-16 |
| uid 比较在服务侧 | `HandleFocused` 中比较获焦/失焦 uid，不将原始焦点事件透传给应用 | AC-4, AC-5 |
| bundleName 服务侧解析 | `focusedBundleName`/`unfocusedBundleName` 由服务侧通过 uid 查 `AppRunningRecord` 解析，window_manager 仅提供 uid | AC-4, AC-17 |
| 权限校验复用 | 复用 `PermissionVerification::VerifyAppStateObserverPermission`（校验 `RUNNING_STATE_OBSERVER`） | AC-2 |
| observerId 成对注册/解注册 | `onAppFocusStateChange` 返回 observerId，`offAppFocusStateChange` 接收 observerId 精确注销；不提供解注册全部接口；错误通过 throw 抛出 | AC-1, AC-6 |
| 动静态归一 | 仅一套 `onAppFocusStateChange(observer): int` / `offAppFocusStateChange(observerId): void` 接口，同时适用于 dynamic 和 static | 全部 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | 焦点变化通知应在窗口焦点切换完成后 ≤ 50ms 回调 | 性能测试 | — |
| 功耗 | 同应用内窗口切换不通知，避免冗余唤醒 | 集成测试 | — |
| 可靠性 | observer 进程死亡不导致 AppStateObserverManager 崩溃 | 单测 | `app_state_observer_manager_test/` |
| 安全 | 无权限应用无法注册 observer | 单测 | `app_state_observer_manager_test/` |
| 可测试性 | 焦点变化行为可经 mock 服务端校验 | 单测 | `app_mgr_client_test/` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 |
|----------|----------|-----------|----------|
| 手机 | 无差异 | — | 集成 |
| 平板 | 无差异 | — | 集成 |
| 折叠屏 | 主屏幕组判断需适配折叠/展开态 | displayId 判断需与窗口管理对齐 | 集成 |
| 无屏设备 | `SUPPORT_SCREEN` 未定义，接口返回不支持 | — | 条件编译验证 |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 通知接口无 UI | — |
| 多窗口/分屏 | 是 | 主屏幕组判断需考虑分屏场景 | AC-7 |
| 多用户 | 是 | 权限校验按调用方 Token，焦点隔离由 window_manager 保证 | AC-2 |
| 版本升级 | 否 | 无持久化数据需迁移 | — |
| 生态兼容 | 是 | 新增 Public API，应用侧需主动适配 | — |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅 appFocusState 通知；MDM 拦截策略不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查
- [x] 接口设计：`onAppFocusStateChange` 返回 observerId、`offAppFocusStateChange` 接收 observerId 精确注销、直接方法命名（非 on/off 字符串分发）、不提供解注册全部接口、错误通过 throw 抛出、动静态归一一套接口

## 已记录问题（不在本规格解决）

| # | 问题 | 影响 | 处理建议 |
|---|------|------|----------|
| Q-1 | proposal 开放问题 C-01：`IFocusChangedListener::OnFocused` 回调签名修改方案需与窗口管理团队对齐。当前 `FocusChangeInfo` 不含 `unfocusedUid` 和 `displayId`。 | 跨仓破坏性变更，需同步修改 | design.md 须产出：window_manager 侧 OnFocused 新签名 + FocusChangeInfo 是否新增字段 or 通过回调参数传递（C-02 决策）。建议方案：在 `OnFocused` 参数中直接增加 `int32_t unfocusedUid` 和 `int32_t displayId`，不改 `FocusChangeInfo` 结构体（减少序列化影响）。 |
| Q-2 | proposal 开放问题 C-03：window_manager 在焦点切换时获取失焦窗口 uid 的时序保证。若 OnFocused 回调时失焦窗口信息已失效，unfocusedUid 可能为 -1 或过期值。 | 通知数据准确性 | design.md 须明确：unfocusedUid 为 -1 时如何处理（建议：-1 表示无前一个获焦窗口，如首次获焦，此时不触发通知或触发含 unfocusedUid=-1 的通知）。 |
| Q-3 | proposal 称 `HandleFocused` 中已有焦点变化处理逻辑但无面向外部应用的 focus 变化通知——准确。但 `HandleFocused` 当前仅在前台状态时调用 `OnAppStateChanged`。新通知路径不应复用此条件，而应独立判断 uid 变化。 | 新旧通知路径耦合风险 | R-4 和 R-11 已明确：`NotifyFocusChanged` 独立于 `OnAppStateChanged`，在 `HandleFocused` 中独立执行 uid 比较 + 通知，不改变现有前台状态条件通知逻辑。 |
| Q-4 | proposal 假设 `RegisterFocusChangedListener` 已在 appms 初始化时调用（已验证：`app_mgr_service_inner.cpp:9091`）——准确。新特性无需改动注册逻辑。 | 无 | 确认无误。 |
| Q-5 | proposal 假设 `FocusChangeInfo` 含 `uid_` 字段（已验证：`app_mgr_service_inner.cpp:9116` 使用 `focusChangeInfo->uid_`）——准确。 | 无 | 确认无误。 |
| Q-6 | proposal 涉及仓为 3 个（ability_ability_runtime + interface_sdk-js + window_manager），但 window_manager 不在当前工作区。 | 跨仓实现依赖 | ability_runtime 侧先实现 IPC 接口 + Manager + NAPI 桥接 + HandleFocused 适配（预留 unfocusedUid/displayId 参数），window_manager 侧变更另行跟踪。 |

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "IAppForegroundStateObserver 的完整 IPC 栈（interface + stub + proxy + manager + NAPI）"
  - repo: "ability_ability_runtime"
    query: "AppStateObserverManager 的 register/unregister/notify 模式与 death recipient 管理"
  - repo: "ability_ability_runtime"
    query: "HandleFocused / HandleUnfocused 的实现与 OnAppStateChanged 通知条件"
  - repo: "ability_ability_runtime"
    query: "WindowFocusChangedListener 的 OnFocused/OnUnfocused 实现（SUPPORT_SCREEN 宏约束）"
  - repo: "ability_ability_runtime"
    query: "js_app_manager.cpp 的 OnOn/OnOff 字符串分发模式与 observer 成员管理"
```

**关键文档：**
- `interfaces/inner_api/app_manager/include/appmgr/app_foreground_state_observer_interface.h`（IPC 接口模板，37 行）
- `interfaces/inner_api/app_manager/include/appmgr/app_foreground_state_observer_stub.h` / `src/appmgr/app_foreground_state_observer_stub.cpp`（Stub 模板）
- `interfaces/inner_api/app_manager/include/appmgr/app_foreground_state_observer_proxy.h` / `src/appmgr/app_foreground_state_observer_proxy.cpp`（Proxy 模板）
- `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h`（IAppMgr 接口，新增虚方法位置，:864,871）
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp`（Proxy 实现，:2102,2119）
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_stub.cpp`（Stub 分发，:1848,1863）
- `services/appmgr/src/app_state_observer_manager.cpp`（Manager 模板，:170-219 注册/注销，:314 `OnAppStateChanged` 通知）
- `services/appmgr/src/app_mgr_service_inner.cpp`（HandleFocused 实现，:9110-9140）
- `services/appmgr/include/window_focus_changed_listener.h` / `src/window_focus_changed_listener.cpp`（窗口焦点监听器）
- `frameworks/js/napi/app/js_app_manager/js_app_manager.cpp`（NAPI 分发入口，:270 OnOn / :404 OnOff / :1845 on 绑定）
