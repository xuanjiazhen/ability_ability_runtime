# 特性规格：进程级生命周期回调

> 本规格覆盖 ApplicationContext 层新增进程级生命周期监听能力（JS 公共 API `on('processLifecycle')` / `off('processLifecycle')`），支持监听**子进程**（`StartChildProcess` 路径）和**独立进程**（`StartProcess`/`LoadAbility` 多进程 HAP 路径）的创建与退出。**对外仅暴露 JS/ArkTS d.ts 接口，不新增 C++ Native 对外 API。** 范围限定：回调注册/注销/分发链路、AMS 侧 uid 过滤触发机制、JS/ETS 桥接。进程自动恢复/重启机制、SELF_FORK 进程覆盖不在本规格范围内。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 进程级生命周期回调 |
| 特性编号 | REQ-PROCESS-LIFECYCLE-001 |
| 所属 Epic | 应用进程生命周期感知 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。当前代码库中不存在 `ProcessLifecycleCallback` 或 `process_lifecycle_callback.h` 符号，进程级生命周期仅通过 `IApplicationStateObserver` 内部 IPC 接口管理，无面向应用侧的查询/监听接口。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `on('processLifecycle')` / `off('processLifecycle')` JS API | `js_application_context_utils.cpp` 新增事件类型分发，**本特性唯一的对外公共 API** |
| ADDED | `ProcessLifecycleCallback` / `ProcessLifecycleInfo` d.ts 类型 | JS 侧类型定义，对应用开放 |
| MODIFIED | `AppMgrServiceInner` 创建触发点 | `StartChildProcess`（子进程，:11012）/ `StartProcess`（独立进程，:6217）成功路径新增 `OnProcessCreated` 通知（内部实现） |
| MODIFIED | `AppMgrServiceInner` 退出路径 | 子进程退出经 pidfd（`OnChildProcessExited` :11430）；独立进程退出经 binder death recipient（`OnRemoteDied` :6695 → `OnProcessDied` :6765）。两条路径均新增 uid 过滤分发（内部实现） |
| MODIFIED | `MainThread` IPC 接收 | 新增 `ScheduleProcessLifecycleEvent` 接收与 Dispatch（内部实现） |

> **对外接口范围声明：** 本特性仅通过 JS/ArkTS d.ts 对外暴露 `on('processLifecycle')` / `off('processLifecycle')` 公共 API。C++ 层的 `ProcessLifecycleCallback` 基类、`ApplicationContext::Register/UnregisterProcessLifecycleCallback`、`IAppScheduler::ScheduleProcessLifecycleEvent` 均为**内部实现**，不在 C++ Native SDK（`interfaces/kits/native/`）中对外声明。

## 进程类型覆盖范围

本特性覆盖以下所有进程类型的创建与退出通知（SELF_FORK 模式除外）：

| # | 进程类型 | 创建接口/路径 | 记录类型 | 死亡检测 | 分发机制 |
|---|---------|-------------|---------|---------|---------|
| 1 | **ArkTS 子进程（APP_SPAWN_FORK）** | JS: `startChildProcess(StartMode.APP_SPAWN_FORK)` | `ChildProcessRecord` | pidfd (`PidFdType::CHILD`) | 1:N 广播 via uid |
| 2 | **ArkTS 子进程（带参数）** | JS: `startArkChildProcess(srcEntry, args, options)` | `ChildProcessRecord` | pidfd (`PidFdType::CHILD`) | 1:N 广播 via uid |
| 3 | **Native 子进程** | JS: `startNativeChildProcess(entryPoint, args, options)`<br>C: `OH_Ability_CreateNativeChildProcess` / `OH_Ability_CreateNativeChildProcessWithConfigs` / `OH_Ability_StartNativeChildProcess` / `OH_Ability_StartNativeChildProcessWithConfigs` | `ChildProcessRecord` | pidfd (`PidFdType::CHILD`) | 1:N 广播 via uid |
| 4 | **Render 进程** | Web/图形栈内部创建（非应用 API） | `RenderRecord` | pidfd (`PidFdType::RENDER`) | 1:N 广播 via uid |
| 5 | **预加载进程** | `StartProcess(isPreload=true)`（:6217，:6329 post attach timeout） | `AppRunningRecord` | binder death recipient | 1:N 广播 via uid |
| 6 | **UIAbility/Extension 独立进程** | `LoadAbility` → `StartProcess`（:6217，非主进程 `isMainProcess_=false`） | `AppRunningRecord` | binder death recipient | 1:N 广播 via uid |
| 7 | **多进程 HAP 模块进程** | `LoadAbility` → `StartProcess`（processName ≠ bundleName） | `AppRunningRecord` | binder death recipient | 1:N 广播 via uid |
| — | ~~SELF_FORK 进程~~ | JS: `startChildProcess(StartMode.SELF_FORK)` | 无 AMS 记录 | 无 | 不监听（AC-4） |

> **统一分发原则：** 所有进程类型均采用 **1:N 广播**——遍历 `AppRunningManager::GetAppRunningRecordMap()` 按 `uid` 过滤，向同 uid 所有存活进程（排除事件进程自身）发送 `ScheduleProcessLifecycleEvent` IPC。每个收到 IPC 的进程在自身 `MainThread` 中检查是否注册了 `ProcessLifecycleCallback`，注册了就回调，没注册就忽略。
>
> **三类记录的 uid 来源不同，但分发路径统一：**
> - `ChildProcessRecord`（#1-3）：`childRecord->GetHostRecord()->GetUid()` 获取宿主应用 uid
> - `RenderRecord`（#4）：`renderRecord->GetHostRecord()->GetUid()` 获取宿主应用 uid
> - `AppRunningRecord`（#5-7）：`appRecord->GetUid()` 直接获取自身 uid
>
> **两类死亡检测机制不同，但不影响分发：**
> - pidfd（#1-4）：内核级进程退出通知，感知 `kill -9` / crash / 正常退出
> - binder death recipient（#5-7）：binder 驱动进程死亡通知，同样感知 `kill -9` / crash / 正常退出

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260807943065\05-proposal-P-01.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证：

### 发现 1：ApplicationContext 已有完整的回调注册模式

`ApplicationContext`（`interfaces/kits/native/appkit/ability_runtime/context/application_context.h`）已有多种回调注册方法：

| 方法 | 行号 | 参照模式 |
|------|------|----------|
| `RegisterAbilityLifecycleCallback` | :47 | 回调基类 + weak_ptr vector + 顺序分发 |
| `UnregisterAbilityLifecycleCallback` | :48 | — |
| `RegisterEnvironmentCallback` | :53 | — |
| `RegisterApplicationStateChangeCallback` | :55-56 | `ApplicationStateChangeCallback` 基类 |
| `RegisterApplicationUpdateCallback` | :57-58 | — |

新增 `RegisterProcessLifecycleCallback` 可直接复用此模式。

### 发现 2：ApplicationStateChangeCallback 是多回调顺序执行模板

`ApplicationStateChangeCallback`（`application_state_change_callback.h:28`）是抽象基类，`ApplicationContext` 用 `static std::vector<std::weak_ptr<ApplicationStateChangeCallback>>` 存储（`application_context.h:270`），在 `application_context.cpp:702-719` 中顺序遍历调用。

`ProcessLifecycleCallback` 可直接镜像此设计：基类 + weak_ptr vector + 顺序分发。

### 发现 3：IApplicationStateObserver 已有 OnProcessCreated / OnProcessDied

`IApplicationStateObserver`（`interfaces/inner_api/app_manager/include/appmgr/iapplication_state_observer.h`）已包含：

| 方法 | 行号 | 类型 |
|------|------|------|
| `OnProcessCreated` | :60 | pure virtual |
| `OnProcessDied` | :88 | pure virtual |
| `OnProcessStateChanged` | :67 | default impl |

IPC 事务码 `TRANSACT_ON_PROCESS_CREATED`（:180）和 `TRANSACT_ON_PROCESS_DIED`（:182）已定义。Stub/Proxy/Manager 全链路已实现（`application_state_observer_stub.cpp` / `application_state_observer_proxy.cpp` / `app_state_observer_manager.cpp`）。

> **关键发现：** AMS 侧的进程生命周期通知 IPC 链路**已完整存在**，无需新增 IPC 接口。但当前 `OnProcessCreated` 仅在 `AppMgrServiceInner` 中对系统观察者（`AppStateObserverManager`）分发，不向应用侧进程分发。本特性需在 `MainThread`（应用侧 IPC 接收端）新增接收逻辑，并由 `AppMgrServiceInner` 在 uid 过滤后向目标应用进程分发。

### 发现 4：childProcessRecordMap_ 在 AppRunningRecord 中

`childProcessRecordMap_`（`app_running_record.h:1410`）存储子进程记录，`AppMgrServiceInner` 通过 `GetChildProcessRecordMap()` 访问。该 Map 天然不含 SELF_FORK 进程（SELF_FORK 进程不走 AMS 的 `StartChildProcess` 路径），因此遍历该 Map 即可精确限定监听范围。

### 发现 5：进程创建/退出路径在 AppMgrServiceInner

| 方法 | 文件 | 行号 | 说明 |
|------|------|------|------|
| `StartChildProcess` | `app_mgr_service_inner.cpp` | :11012 | 子进程启动入口 |
| `StartChildProcessImpl` | `app_mgr_service_inner.cpp` | :11125 | 子进程启动实现 |
| `LoadAbility` | `app_mgr_service_inner.cpp` | :2157 | Ability 加载（进程已存在时） |
| `OnProcessDied` | `app_state_observer_manager.cpp` | — | 进程死亡通知 |
| `OnChildProcessDied` | `app_mgr_service_inner.cpp` | — | 子进程死亡处理 |

> **Proposal 修正：** Proposal 将 `StartChildProcess`/`LoadAbility` 归于 `ability_manager_service.cpp`（SA 180），实际在 `app_mgr_service_inner.cpp`（SA 501）。AMS = AbilityManagerService，AppMgrService = AppManagerService，两者为不同 SA。

### 发现 6：js_application_context_utils.cpp 已有事件分发模式

`js_application_context_utils.cpp` 的 `on()`（:1644-1656）按字符串事件类型分发：`"abilityLifecycle"` / `"environment"` / `"applicationStateChange"` 等。新增 `"processLifecycle"` 需在此 switch 中增加分支。

### 发现 7：MainThread 已有 IPC 接收模式

`main_thread.cpp` 实现 `IAppScheduler` stub（:436 `iface_cast<IAppMgr>`），接收 `Schedule*` 系列 IPC 回调（如 `ScheduleForegroundApplication` :507、`ScheduleLaunchAbility` :877）。新增 `ScheduleProcessLifecycleEvent` 可复用此模式。

### 发现 8：子进程死亡检测使用 pidfd + epoll

`AppPidFdManager`（`services/appmgr/src/app_pidfd_manager.cpp`）在子进程创建成功后**立即注册** pidfd 监控（`app_mgr_service_inner.cpp:11200`）：

- `AddWatcher(pid, PidFdType::CHILD)`（:77）调用 `syscall(SYS_pidfd_open, pid, PIDFD_NONBLOCK)` 获取内核 pidfd
- 将 pidfd 加入 `ffrt_epoll_ctl`（:116），注册回调 `OnChildPidfdEvent`（:132）
- 进程死亡时内核自动使 pidfd 可读（EPOLLIN），epoll 线程触发回调
- **对 `kill -9`（SIGKILL）同样可靠感知**——内核级保证，不依赖进程内代码执行
- 即使进程在 `AddWatcher` 调用前已退出（`pidfd_open` 返回 ESRCH），`AddWatcher` 也直接调用 `DispatchCleanup` 兜底（:84-89）

`PidFdType` 枚举仅有 `CHILD` 和 `RENDER`，**不覆盖独立进程和主进程**。

### 发现 9：主进程/独立进程死亡检测使用 binder death recipient

主进程和独立进程在 `AttachApplication`（`app_mgr_service_inner.cpp:2967-2972`）时注册 `AppDeathRecipient` 到 `appScheduler->AsObject()`：

- 进程死亡时 binder 驱动触发 `AppDeathRecipient::OnRemoteDied`（`app_death_recipient.cpp:26`）
- 同样对 `kill -9` 可靠感知——binder 驱动在内核层监控 IPC 对端进程存活
- 独立进程的 `AppRunningRecord` 与主进程走**完全相同**的 `OnRemoteDied` → `ClearAppRunningData` → `OnProcessDied` 链路

> **attach 前窗口期：** 独立进程在 `StartProcess` 成功后到 `AttachApplication` 调用前，既无 pidfd 也无 death recipient。此窗口期内 `kill -9` 无法即时感知，需依赖后续 `LoadAbility` 超时或显式清理兜底。子进程无此问题（pidfd 在创建后立即注册）。

### 发现 10：统一 1:N 广播分发——所有进程类型均遍历同 uid 存活进程

子进程、Render 进程和独立进程的**死亡检测机制不同**（pidfd vs binder death recipient），但**分发机制统一为 1:N 广播**。原因是 `ProcessLifecycleCallback` 是进程级 API（`ApplicationContext.on(...)`），任何同应用进程都可以注册监听，不应仅限于创建者/宿主进程。

| | 子进程 / Render 进程 | 独立进程 |
|---|---|---|
| **记录类型** | `ChildProcessRecord` / `RenderRecord` | `AppRunningRecord` |
| **关联结构** | `hostRecord_` 指向宿主 `AppRunningRecord` | 自身就是 `AppRunningRecord` |
| **uid 来源** | `childRecord->GetHostRecord()->GetUid()` 或 `renderRecord->GetHostRecord()->GetUid()` | `appRecord->GetUid()` |
| **死亡检测** | pidfd（`PidFdType::CHILD` / `RENDER`） | binder death recipient |
| **分发机制** | **统一 1:N 广播**：遍历 `GetAppRunningRecordMap()` 按 uid 过滤 | **统一 1:N 广播**：同上 |
| **分发范围** | 同 uid 所有存活进程（排除事件进程自身） | 同 uid 所有存活进程（排除自身） |
| **任何进程都能监听** | ✅ 主进程 + 独立进程 + 其他存活进程均可收到 | ✅ 同上 |

> **关键设计决策：** 不使用 `childRecord->GetHostRecord()->GetApplicationClient()` 1:1 分发，因为那只能通知创建子进程的宿主进程。一个应用可能同时有主进程和多个独立进程运行，任何一个都可以注册 `ProcessLifecycleCallback` 并期望收到同应用所有进程（包括其他进程创建的子进程）的生命周期事件。因此**统一采用 1:N 广播**：遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程发送 IPC，由各进程自行决定是否回调。

`GetAppRunningRecordMap()`（`app_running_manager.h:222`）返回所有 `AppRunningRecord` 的副本，可遍历按 `GetUid()` 过滤实现同应用进程查找。`WrapChildProcessData`（`app_state_observer_manager.cpp:1141`）和 `WrapProcessData`（:1085）填充的 `data->uid` 可作为统一过滤键。

## 用户故事

### US-1: 注册进程生命周期回调

**作为** 应用开发者
**我想要** 注册进程生命周期回调，感知子进程和独立进程的启动与退出
**以便** 及时初始化/清理相关资源

### US-2: 子进程启动通知

**作为** 应用开发者
**我想要** 在同应用任意进程创建子进程时收到通知（pid/processName/uid/state="CREATED"）
**以便** 初始化相关资源

### US-3: 子进程退出通知（含异常退出）

**作为** 应用开发者
**我想要** 在同应用任意子进程退出时收到通知（pid/processName/uid/state="EXITED"），包括异常退出
**以便** 清理资源或触发恢复

### US-4: SELF_FORK 进程不通知

**作为** 应用开发者
**我想要** SELF_FORK 进程退出不触发回调
**以便** 避免非 AMS 孵化进程的误通知

### US-5: 自身退出不通知

**作为** 应用开发者
**我想要** 注册回调的进程自身退出不触发回调
**以便** 进程死亡不产生无效回调

### US-6: 零权限

**作为** 应用开发者
**我想要** 无需申请任何权限即可使用该接口
**以便** 降低使用门槛

### US-7: uid 过滤隔离

**作为** 应用开发者
**我想要** 只收到本应用进程的事件
**以便** 不会收到其他应用的信息

### US-8: 多回调顺序执行

**作为** 应用开发者
**我想要** 注册多个回调并按注册顺序执行
**以便** 灵活扩展监听逻辑

### US-9: 独立进程启动通知

**作为** 应用开发者
**我想要** 在同应用的独立进程（多进程 HAP 模块）启动时收到通知
**以便** 主进程和独立进程间协调资源

### US-10: 独立进程退出通知（含 kill -9）

**作为** 应用开发者
**我想要** 在独立进程退出时收到通知，包括被 `kill -9` 杀死的情况
**以便** 及时清理资源或触发恢复

### US-11: 独立进程也能监听

**作为** 应用开发者
**我想要** 在独立进程中也能注册 `ProcessLifecycleCallback` 监听同应用其他进程的生命周期
**以便** 独立进程间互相感知，而非只有主进程能监听

### US-12: Render 进程通知

**作为** 应用开发者
**我想要** 在 Render 进程（GPU/Web 渲染进程）创建和退出时收到通知
**以便** 监控渲染子进程的资源占用与异常退出

### US-13: 预加载进程通知

**作为** 应用开发者
**我想要** 在预加载进程创建和退出时收到通知
**以便** 感知系统预加载行为，协调应用资源状态

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 应用调用 `on('processLifecycle', callback)` THEN callback 被注册到 ApplicationContext 的回调列表中，无权限拒绝 | 正常 | US-1, US-6 |
| AC-2 | WHEN AMS 通过 `StartChildProcess` 成功启动子进程 THEN 同 uid 的所有注册了回调的存活进程收到 `OnProcessCreated` 回调，参数含 pid/processName/uid/state="CREATED" | 正常 | US-2 |
| AC-3 | WHEN 子进程退出（正常或异常 crash，含 `kill -9`） THEN 同 uid 的所有注册了回调的存活进程收到 `OnProcessExited` 回调，参数含 pid/processName/uid/state="EXITED" | 正常 | US-3 |
| AC-4 | WHEN SELF_FORK 进程退出 THEN 不触发 `OnProcessExited` 回调（`childProcessRecordMap_` 天然不含 SELF_FORK 进程） | 边界 | US-4 |
| AC-5 | WHEN 注册回调的进程自身退出 THEN 不触发自身回调（进程已死亡，`GetApplicationClient()` 为 nullptr，无法接收 IPC） | 边界 | US-5 |
| AC-6 | WHEN 普通应用（非系统应用）调用 `on('processLifecycle')` THEN 注册成功，无权限拒绝 | 正常 | US-6 |
| AC-7 | WHEN 应用 A 的子进程/独立进程启动/退出 THEN 仅应用 A 同 uid 的进程收到回调，应用 B 不收到（AMS 按 uid 过滤分发） | 异常 | US-7 |
| AC-8 | WHEN 同一应用注册多个 callback THEN 按注册顺序依次执行 | 正常 | US-8 |
| AC-9 | WHEN 应用调用 `off('processLifecycle', callback)` 或 `off('processLifecycle')` THEN 对应 callback 或全部 callback 被移除 | 正常 | US-1 |
| AC-10 | WHEN `StartProcess` 成功启动独立进程（多进程 HAP 模块） THEN 同 uid 的所有存活进程收到 `OnProcessCreated` 回调 | 正常 | US-9 |
| AC-11 | WHEN 独立进程退出（含 `kill -9`） THEN 同 uid 的其他存活进程收到 `OnProcessExited` 回调 | 正常 | US-10 |
| AC-12 | WHEN 独立进程注册了 `ProcessLifecycleCallback` THEN 该独立进程能收到同应用其他进程的创建/退出通知（1:N 广播覆盖独立进程） | 正常 | US-11 |
| AC-13 | WHEN 独立进程退出 THEN 该退出进程自身不收到回调（排除自身 pid），同应用其他存活进程收到回调 | 边界 | US-5, US-10 |
| AC-14 | WHEN AMS 通过 Render 进程创建路径（`AttachRenderProcess` :8875）成功启动 Render 进程 THEN 同 uid 的所有注册了回调的存活进程收到 `OnProcessCreated` 回调 | 正常 | US-12 |
| AC-15 | WHEN Render 进程退出（含 `kill -9`） THEN 同 uid 的所有注册了回调的存活进程收到 `OnProcessExited` 回调（pidfd `PidFdType::RENDER` 检测） | 正常 | US-12 |
| AC-16 | WHEN `StartProcess(isPreload=true)` 成功启动预加载进程 THEN 同 uid 的所有存活进程收到 `OnProcessCreated` 回调（1:N 广播） | 正常 | US-13 |
| AC-17 | WHEN 预加载进程退出（含 `kill -9`、attach 超时被 kill） THEN 同 uid 的其他存活进程收到 `OnProcessExited` 回调 | 正常 | US-13 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + XTS | `js_application_context_utils` NAPI 单测 |
| AC-2 | R-2 | 集成测试 | 启动 UIAbility/Extension/子进程 |
| AC-3 | R-3 | 集成测试 | 正常退出 + crash + kill -9 场景 |
| AC-4 | R-4 | 集成测试 | 应用自行 fork 后退出 |
| AC-5 | R-5, R-9 | 集成测试 | 主进程/独立进程退出验证 |
| AC-6 | R-1 | 权限测试 | 普通应用无权限拒绝 |
| AC-7 | R-6 | 多应用并发测试 | 不同 uid 应用互不干扰 |
| AC-8 | R-7 | 单元测试 | 多回调顺序验证 |
| AC-9 | R-8 | 单测 | `off` 注销验证 |
| AC-10 | R-9 | 集成测试 | 多进程 HAP 启动独立进程回调 |
| AC-11 | R-10 | 集成测试 | 独立进程正常退出 + kill -9 |
| AC-12 | R-9, R-10 | 集成测试 | 独立进程注册回调收到通知 |
| AC-13 | R-9, R-10 | 集成测试 | 独立进程退出排除自身 pid |
| AC-14 | R-11 | 集成测试 | Render 进程创建回调 |
| AC-15 | R-12 | 集成测试 | Render 进程退出（含 kill -9） |
| AC-16 | R-9 | 集成测试 | 预加载进程创建回调 |
| AC-17 | R-10 | 集成测试 | 预加载进程退出 |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 应用调用 `on('processLifecycle', callback)` | callback 加入 ApplicationContext 回调列表 | 零权限，无 `VerifyCallingPermission` | AC-1, AC-6 |
| R-2 | 行为 | `AppMgrServiceInner::StartChildProcess` 成功 | AMS 通过 `childRecord->GetHostRecord()->GetUid()` 获取 uid，遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程发送 `OnProcessCreated` IPC | 统一 1:N 广播；排除事件进程自身 | AC-2 |
| R-3 | 行为 | 子进程退出（正常或 crash 或 kill -9） | AMS 通过 pidfd 回调链路（`OnChildProcessExited` → `OnChildProcessDied`）获取 uid，遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程分发 `OnProcessExited` | 统一 1:N 广播；state 固定为 "EXITED"，不透传退出信号 | AC-3 |
| R-4 | 边界 | SELF_FORK 进程退出 | 不触发回调 | `childProcessRecordMap_` 天然不含 SELF_FORK 进程 | AC-4 |
| R-5 | 边界 | 注册回调的进程自身退出 | 不触发自身回调 | 进程已死亡，`GetApplicationClient()` 为 nullptr，无法接收 IPC | AC-5 |
| R-6 | 异常 | 不同 uid 的应用进程事件 | 仅向匹配 uid 的应用分发 | AMS 侧 uid 过滤，无跨应用信息泄露 | AC-7 |
| R-7 | 行为 | 同一应用注册多个 callback | 按注册顺序依次执行 | 仿照 `ApplicationStateChangeCallback` 顺序遍历模式 | AC-8 |
| R-8 | 行为 | 调用 `off('processLifecycle', callback?)` | 移除指定 callback 或全部 callback | 仿照 `UnregisterAbilityLifecycleCallback` | AC-9 |
| R-9 | 行为 | `AppMgrServiceInner::StartProcess` 成功启动独立进程 | AMS 遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程（排除自身）发送 `OnProcessCreated` IPC | 独立进程 1:N 广播分发；每个收到 IPC 的进程检查自身是否注册了回调 | AC-10, AC-12, AC-13 |
| R-10 | 行为 | 独立进程退出（含 kill -9） | AMS 通过 binder death recipient（`OnRemoteDied` → `OnProcessDied`）遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 其他存活进程分发 `OnProcessExited` | 独立进程 1:N 广播分发；排除自身 pid；state 固定为 "EXITED"，不透传退出信号 | AC-11, AC-12, AC-13, AC-17 |
| R-11 | 行为 | `AttachRenderProcess` 成功启动 Render 进程 | AMS 通过 `renderRecord->GetHostRecord()->GetUid()` 获取 uid，遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程发送 `OnProcessCreated` IPC | 统一 1:N 广播；排除事件进程自身 | AC-14 |
| R-12 | 行为 | Render 进程退出（含 kill -9） | AMS 通过 pidfd 回调链路（`OnRenderPidfdEvent` → `OnRenderProcessExited` → `OnRenderProcessDied`）获取 uid，遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程分发 `OnProcessExited` | 统一 1:N 广播；state 固定为 "EXITED"，不透传退出信号 | AC-15 |

## API 变更分析

### 新增 API（仅 JS/ArkTS 公共 API，不新增 C++ Native 对外接口）

| API 名称 | 开放范围 | 入参概要 | 返回值 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|----------|---------|
| `on('processLifecycle', callback)` | Public (JS/ArkTS) | `callback: ProcessLifecycleCallback` | `void` | 注册进程生命周期监听 | AC-1 |
| `off('processLifecycle', callback?)` | Public (JS/ArkTS) | `callback?: ProcessLifecycleCallback` | `void` | 注销进程生命周期监听 | AC-9 |

### 回调数据结构

```typescript
interface ProcessLifecycleInfo {
    pid: number;
    processName: string;
    uid: number;
    state: string;  // "CREATED" | "EXITED"
}

interface ProcessLifecycleCallback {
    onProcessCreated(info: ProcessLifecycleInfo): void;
    onProcessExited(info: ProcessLifecycleInfo): void;
}
```

## 接口规格

### JS API

| 属性 | 值 |
|------|-----|
| 函数签名 | `on(type: 'processLifecycle', callback: ProcessLifecycleCallback): void` |
| 返回值 | `void` |
| 开放范围 | Public |
| 权限 | 无（零权限设计） |
| d.ts 位置 | `api/@ohos.app.ability.applicationContext.d.ts`（新增到现有 namespace） |
| 关联 AC | AC-1, AC-6 |

### Inner API（IPC 分发）

| 属性 | 值 |
|------|-----|
| 统一分发原则 | 所有进程类型（子进程/Render/独立进程/预加载）均采用 1:N 广播：遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程（排除事件进程自身）发送 IPC |
| 子进程创建触发 | `AppMgrServiceInner::StartChildProcessImpl`（:11125）成功后，`childRecord->GetHostRecord()->GetUid()` 取 uid，遍历 Map 广播 |
| 独立进程创建触发 | `AppMgrServiceInner::StartProcess`（:6217）成功后（:6340），`appRecord->GetUid()` 取 uid，遍历 Map 广播 |
| Render 进程创建触发 | `AttachRenderProcess`（:8875）成功后，`renderRecord->GetHostRecord()->GetUid()` 取 uid，遍历 Map 广播 |
| 子进程退出触发 | pidfd 回调链路：`AppPidFdManager::OnChildPidfdEvent` → `OnChildProcessExited`（:11430）→ `OnChildProcessDied`（:11422）→ 取 uid → 遍历 Map 广播 |
| Render 进程退出触发 | pidfd 回调链路：`OnRenderPidfdEvent` → `OnRenderProcessExited` → `OnRenderProcessDied` → 取 uid → 遍历 Map 广播 |
| 独立进程退出触发 | binder death recipient 链路：`AppDeathRecipient::OnRemoteDied` → `OnRemoteDied`（:6695）→ `ClearAppRunningData` → `OnProcessDied`（:6765）→ 取 uid → 遍历 Map 广播 |
| uid 过滤 | 遍历 `GetAppRunningRecordMap()`，`record->GetUid() == data->uid` 匹配同应用进程；排除 `record->GetPid() == data->pid`（事件进程自身） |
| IPC 通道 | 复用 `IAppScheduler`（MainThread stub）新增 `ScheduleProcessLifecycleEvent` |

### 行为场景

| # | 触发条件 | 预期行为 | 分发机制 | 关联 AC |
|---|----------|----------|---------|---------|
| 1 | on('processLifecycle', callback) | 注册成功，零权限 | — | AC-1 |
| 2 | StartChildProcess 成功 | 同 uid 所有注册了回调的存活进程收到 OnProcessCreated | 统一 1:N 广播 (uid) | AC-2 |
| 3 | StartProcess 成功（独立进程） | 同 uid 所有注册了回调的存活进程收到 OnProcessCreated | 统一 1:N 广播 (uid) | AC-10 |
| 4 | 子进程正常退出 | 同 uid 所有注册了回调的存活进程收到 OnProcessExited | 统一 1:N 广播 (pidfd + uid) | AC-3 |
| 5 | 子进程 crash / kill -9 | 同 uid 所有注册了回调的存活进程收到 OnProcessExited | 统一 1:N 广播 (pidfd + uid) | AC-3 |
| 6 | 独立进程退出（含 kill -9） | 同 uid 其他存活进程收到 OnProcessExited | 统一 1:N 广播 (binder death recipient + uid) | AC-11 |
| 7 | SELF_FORK 进程退出 | 不回调 | — | AC-4 |
| 8 | 进程自身退出 | 自身不回调，其他同 uid 进程收到 | 排除自身 pid | AC-5, AC-13 |
| 9 | 不同 uid 应用事件 | 仅同 uid 进程收到回调 | uid 过滤 | AC-7 |
| 10 | 多 callback 注册 | 按顺序执行 | — | AC-8 |
| 11 | 独立进程注册回调 | 独立进程也能收到同应用其他进程通知 | 1:N 广播覆盖所有同 uid 进程 | AC-12 |
| 12 | off('processLifecycle') | 注销成功 | — | AC-9 |
| 13 | Render 进程创建 | 同 uid 所有注册了回调的存活进程收到 OnProcessCreated | 统一 1:N 广播 (uid) | AC-14 |
| 14 | Render 进程退出（含 kill -9） | 同 uid 所有注册了回调的存活进程收到 OnProcessExited | 统一 1:N 广播 (pidfd + uid) | AC-15 |
| 15 | 预加载进程创建 | 同 uid 所有注册了回调的存活进程收到 OnProcessCreated | 统一 1:N 广播 (uid) | AC-16 |
| 16 | 预加载进程退出（含 attach 超时 kill） | 同 uid 其他存活进程收到 OnProcessExited | 统一 1:N 广播 (binder death recipient + uid) | AC-17 |

## 兼容性声明

- **已有 API 行为变更:** 否。新增 `on('processLifecycle')` 不修改已有 `on` 类型。现有 `AbilityLifecycleCallback` 行为不变。
- **IPC 序列化格式变更:** 新增 `ScheduleProcessLifecycleEvent` 到 `IAppScheduler`，不影响已有 IPC 事务码。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 统一 1:N 广播分发 | 所有进程类型（子进程/Render/独立进程/预加载）均遍历 `GetAppRunningRecordMap()` 按 uid 过滤，向同 uid 所有存活进程（排除事件进程自身）广播；每个收到 IPC 的进程自行检查是否注册了回调 | AC-2, AC-3, AC-10, AC-11, AC-12, AC-13, AC-14, AC-15, AC-16, AC-17 |
| 子进程死亡检测：pidfd + epoll | `AppPidFdManager` 创建后立即注册（:11200），`PidFdType::CHILD`，内核级保证感知 kill -9 / crash / 正常退出 | AC-3 |
| Render 进程死亡检测：pidfd + epoll | `AppPidFdManager` 在 `AttachRenderProcess` 时注册（:8875），`PidFdType::RENDER`，同子进程机制 | AC-15 |
| 独立进程死亡检测：binder death recipient | `AppDeathRecipient` 在 `AttachApplication` 时注册（:2967），binder 驱动内核级保证感知 kill -9 / crash / 正常退出 | AC-11, AC-17 |
| 独立进程 attach 前窗口期 | `StartProcess` 成功到 `AttachApplication` 前无 pidfd 也无 death recipient；此窗口期 kill -9 需依赖 LoadAbility 超时或显式清理兜底 | AC-11 |
| uid 过滤隔离 | AMS 侧按 uid 过滤，仅向同应用进程分发，无跨应用信息泄露 | AC-7 |
| 复用 IAppScheduler IPC 通道 | 不新增 IPC 接口，复用 `IAppScheduler` 新增 `ScheduleProcessLifecycleEvent` | 全部 |
| childProcessRecordMap_ 天然排除 SELF_FORK | SELF_FORK 进程不走 `StartChildProcess` 路径，不在 Map 中 | AC-4 |
| 多回调模式参照 ApplicationStateChangeCallback | 基类 + weak_ptr vector + 顺序遍历 | AC-8 |
| state 字段不透传信号 | 退出 state 固定 "EXITED"，不包含退出信号信息 | AC-3, AC-11 |
| 零权限 | 不调用 `VerifyCallingPermission`，uid 过滤由 AMS 保证安全 | AC-6 |
| 排除自身 pid | 独立进程创建/退出时分发排除自身 pid，避免向自身发送 IPC | AC-13 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 回调注册/分发不应导致 AMS 崩溃 | 压力测试 | `appmgr/test/unittest/` |
| 安全 | 零权限设计，AMS 按 uid 过滤无信息泄露 | 多应用并发测试 | — |
| 性能 | 不涉及 | — | — |
| 可测试性 | 回调行为可经 mock AMS 校验 | 单测 | `app_mgr_client_test/` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 |
|----------|----------|-----------|
| 手机 | 无差异 | — |
| 平板 | 无差异 | — |
| 折叠屏 | 无差异 | — |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 回调接口无 UI | — |
| 多用户 | 是 | uid 过滤天然支持多用户隔离 | AC-7 |
| 版本升级 | 否 | 无持久化数据需迁移 | — |
| 生态兼容 | 是 | 新增 Public API，应用侧需主动适配 | — |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（子进程 + 独立进程；SELF_FORK/自身退出/信号透传/进程自动恢复不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查
- [x] 两类死亡检测（pidfd / binder death recipient）均有 AC 和规则覆盖
- [x] 统一 1:N 广播分发：所有进程类型（子进程/Render/独立进程/预加载）均遍历同 uid 存活进程广播，不再有 1:1 分发
- [x] 进程类型覆盖范围表列明全部 7 类进程（子进程 3 类 + Render + 预加载 + 独立进程 2 类）及排除项（SELF_FORK）
- [x] Render 进程（1:1 分发 + pidfd 检测）有 AC-14/AC-15 和 R-11/R-12 覆盖
- [x] 预加载进程（1:N 广播 + binder death recipient）有 AC-16/AC-17 覆盖

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "ApplicationStateChangeCallback 的多回调顺序执行模式与 ApplicationContext 注册/分发"
  - repo: "ability_ability_runtime"
    query: "IApplicationStateObserver 的 OnProcessCreated/OnProcessDied IPC 链路"
  - repo: "ability_ability_runtime"
    query: "AppMgrServiceInner::StartChildProcess / StartProcess 进程创建路径"
  - repo: "ability_ability_runtime"
    query: "AppPidFdManager 的 pidfd + epoll 子进程死亡检测机制"
  - repo: "ability_ability_runtime"
    query: "AppDeathRecipient binder death recipient 主进程/独立进程死亡检测"
  - repo: "ability_ability_runtime"
    query: "AppRunningManager::GetAppRunningRecordMap / FindMainProcessAppRunningRecord uid 查找机制"
  - repo: "ability_ability_runtime"
    query: "ChildProcessRecord::hostRecord_ 子进程到宿主的结构化引用"
  - repo: "ability_ability_runtime"
    query: "js_application_context_utils.cpp 的 on/off 事件类型分发模式"
  - repo: "ability_ability_runtime"
    query: "MainThread 的 IAppScheduler Schedule* IPC 接收模式"
```

**关键文档：**
- `interfaces/kits/native/appkit/ability_runtime/context/application_context.h`（Register/Unregister 模板，:47-58）
- `interfaces/kits/native/appkit/ability_runtime/context/application_state_change_callback.h`（多回调基类模板，:28）
- `interfaces/inner_api/app_manager/include/appmgr/iapplication_state_observer.h`（OnProcessCreated/OnProcessDied，:60,:88）
- `services/appmgr/src/app_mgr_service_inner.cpp`（StartChildProcess :11012，StartProcess :6217，AttachRenderProcess :8875，OnRemoteDied :6695，OnChildProcessExited :11430，OnProcessDied :6765, :13157）
- `services/appmgr/src/app_pidfd_manager.cpp`（pidfd + epoll 子进程/Render 进程死亡检测，:77 AddWatcher，:132 OnChildPidfdEvent，:143 OnRenderPidfdEvent）
- `services/appmgr/src/app_death_recipient.cpp`（binder death recipient，:26 OnRemoteDied）
- `services/appmgr/include/render_record.h`（RenderRecord，hostPid_ :75，GetHostRecord :58）
- `services/appmgr/include/app_running_record.h`（childProcessRecordMap_ :1410，isMainProcess_ :1527，GetApplicationClient :322）
- `services/appmgr/include/child_process_record.h`（hostRecord_ :83）
- `services/appmgr/src/app_running_manager.cpp`（FindMainProcessAppRunningRecord :317，GetAppRunningRecordMap :804）
- `services/appmgr/src/app_state_observer_manager.cpp`（HandleOnProcessCreated :895，HandleOnChildProcessCreated :869，HandleOnProcessDied :1064）
- `frameworks/native/appkit/ability_runtime/context/js_application_context_utils.cpp`（on/off 事件分发，:1644-1656）
- `frameworks/native/appkit/app/main_thread.cpp`（IAppScheduler IPC 接收，:436,:507,:877）
