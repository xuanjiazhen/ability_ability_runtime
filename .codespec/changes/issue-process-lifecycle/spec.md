# 特性规格：进程级生命周期回调

> 本规格覆盖 ApplicationContext 层新增 `ProcessLifecycleCallback` 进程级生命周期监听能力。范围限定：回调注册/注销/分发链路、AMS 侧 uid 过滤触发机制、JS/ETS 桥接。进程自动恢复/重启机制、SELF_FORK 进程覆盖不在本规格范围内。

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
| ADDED | `ProcessLifecycleCallback` 回调基类 | 仿照 `ApplicationStateChangeCallback` 模式，`OnProcessCreated`/`OnProcessExited` 两个纯虚方法 |
| ADDED | `ApplicationContext::Register/UnregisterProcessLifecycleCallback` | 仿照 `RegisterAbilityLifecycleCallback` 模式 |
| ADDED | `on('processLifecycle')` / `off('processLifecycle')` JS API | `js_application_context_utils.cpp` 新增事件类型分发 |
| MODIFIED | `AppMgrServiceInner` 触发点 | `StartChildProcess` / `LoadAbility` 成功路径新增 `OnProcessCreated` 通知 |
| MODIFIED | `AppMgrServiceInner` 退出路径 | `OnProcessDied` / `OnChildProcessDied` 路径新增 uid 过滤分发 |
| MODIFIED | `MainThread` IPC 接收 | 新增 `ScheduleProcessLifecycleEvent` 接收与 Dispatch |

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

## 用户故事

### US-1: 注册进程生命周期回调

**作为** 应用开发者
**我想要** 注册进程生命周期回调，感知子进程的启动与退出
**以便** 及时初始化/清理相关资源

### US-2: 子进程启动通知

**作为** 应用开发者
**我想要** 在子进程启动时收到通知（pid/processName/uid/state="CREATED"）
**以便** 初始化相关资源

### US-3: 子进程退出通知（含异常退出）

**作为** 应用开发者
**我想要** 在子进程退出时收到通知（pid/processName/uid/state="EXITED"），包括异常退出
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

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 应用调用 `on('processLifecycle', callback)` THEN callback 被注册到 ApplicationContext 的回调列表中，无权限拒绝 | 正常 | US-1, US-6 |
| AC-2 | WHEN AMS 通过 `StartChildProcess` 成功启动子进程 THEN 目标应用收到 `OnProcessCreated` 回调，参数含 pid/processName/uid/state="CREATED" | 正常 | US-2 |
| AC-3 | WHEN 子进程退出（正常或异常 crash） THEN 目标应用收到 `OnProcessExited` 回调，参数含 pid/processName/uid/state="EXITED" | 正常 | US-3 |
| AC-4 | WHEN SELF_FORK 进程退出 THEN 不触发 `OnProcessExited` 回调（`childProcessRecordMap_` 天然不含 SELF_FORK 进程） | 边界 | US-4 |
| AC-5 | WHEN 注册回调的进程自身退出 THEN 不触发自身回调（进程已死亡无法接收） | 边界 | US-5 |
| AC-6 | WHEN 普通应用（非系统应用）调用 `on('processLifecycle')` THEN 注册成功，无权限拒绝 | 正常 | US-6 |
| AC-7 | WHEN 应用 A 的子进程启动/退出 THEN 仅应用 A 收到回调，应用 B 不收到（AMS 按 uid 过滤分发） | 异常 | US-7 |
| AC-8 | WHEN 同一应用注册多个 callback THEN 按注册顺序依次执行 | 正常 | US-8 |
| AC-9 | WHEN 应用调用 `off('processLifecycle', callback)` 或 `off('processLifecycle')` THEN 对应 callback 或全部 callback 被移除 | 正常 | US-1 |
| AC-10 | WHEN `LoadAbility` 成功（非 `StartChildProcess` 路径，新进程首次加载 Ability） THEN 触发 `OnProcessCreated` 回调 | 正常 | US-2 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + XTS | `js_application_context_utils` NAPI 单测 |
| AC-2 | R-2 | 集成测试 | 启动 UIAbility/Extension/子进程 |
| AC-3 | R-3 | 集成测试 | 正常退出 + crash 场景 |
| AC-4 | R-4 | 集成测试 | 应用自行 fork 后退出 |
| AC-5 | R-5 | 集成测试 | 主进程退出验证 |
| AC-6 | R-1 | 权限测试 | 普通应用无权限拒绝 |
| AC-7 | R-6 | 多应用并发测试 | 不同 uid 应用互不干扰 |
| AC-8 | R-7 | 单元测试 | 多回调顺序验证 |
| AC-9 | R-8 | 单测 | `off` 注销验证 |
| AC-10 | R-2 | 集成测试 | LoadAbility 新进程回调 |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 应用调用 `on('processLifecycle', callback)` | callback 加入 ApplicationContext 回调列表 | 零权限，无 `VerifyCallingPermission` | AC-1, AC-6 |
| R-2 | 行为 | `AppMgrServiceInner::StartChildProcess` 成功 或 `LoadAbility` 首次加载 | AMS 通过 `AppStateObserverManager` 向目标应用进程的 `MainThread` 发送 `OnProcessCreated` IPC | 按 uid 过滤：仅向目标应用所属进程分发 | AC-2, AC-10 |
| R-3 | 行为 | 子进程退出（正常或 crash） | AMS 通过 `OnProcessDied` / `OnChildProcessDied` 路径向目标应用进程分发 `OnProcessExited` | 按 uid 过滤；state 固定为 "EXITED"，不透传退出信号 | AC-3 |
| R-4 | 边界 | SELF_FORK 进程退出 | 不触发回调 | `childProcessRecordMap_` 天然不含 SELF_FORK 进程 | AC-4 |
| R-5 | 边界 | 注册回调的进程自身退出 | 不触发自身回调 | 进程已死亡无法接收 IPC | AC-5 |
| R-6 | 异常 | 不同 uid 的应用子进程事件 | 仅向匹配 uid 的应用分发 | AMS 侧 uid 过滤，无跨应用信息泄露 | AC-7 |
| R-7 | 行为 | 同一应用注册多个 callback | 按注册顺序依次执行 | 仿照 `ApplicationStateChangeCallback` 顺序遍历模式 | AC-8 |
| R-8 | 行为 | 调用 `off('processLifecycle', callback?)` | 移除指定 callback 或全部 callback | 仿照 `UnregisterAbilityLifecycleCallback` | AC-9 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|----------|---------|
| `on('processLifecycle', callback)` | Public | `callback: ProcessLifecycleCallback` | `void` | 注册进程生命周期监听 | AC-1 |
| `off('processLifecycle', callback?)` | Public | `callback?: ProcessLifecycleCallback` | `void` | 注销进程生命周期监听 | AC-9 |

### 回调数据结构

```typescript
interface ProcessLifecycleCallback {
    onProcessCreated(pid: number, processName: string, uid: number, state: string): void;
    onProcessExited(pid: number, processName: string, uid: number, state: string): void;
}
```

> `state` 值为 `"CREATED"` 或 `"EXITED"`。

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
| 触发点 | `AppMgrServiceInner::StartChildProcess`（:11012）/ `LoadAbility`（:2157）成功路径 |
| uid 过滤 | `IPCSkeleton::GetCallingUid()` → 匹配目标应用 uid → 仅向同 uid 进程分发 |
| IPC 通道 | 复用 `IAppScheduler`（MainThread stub）新增 `ScheduleProcessLifecycleEvent` |
| 退出触发 | `AppMgrServiceInner` 的 `OnProcessDied` / `OnChildProcessDied` 路径 |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | on('processLifecycle', callback) | 注册成功，零权限 | AC-1 |
| 2 | StartChildProcess 成功 | OnProcessCreated 回调 | AC-2 |
| 3 | LoadAbility 首次加载 | OnProcessCreated 回调 | AC-10 |
| 4 | 子进程正常退出 | OnProcessExited 回调 | AC-3 |
| 5 | 子进程 crash 退出 | OnProcessExited 回调 | AC-3 |
| 6 | SELF_FORK 进程退出 | 不回调 | AC-4 |
| 7 | 主进程自身退出 | 不回调 | AC-5 |
| 8 | 不同 uid 应用事件 | 仅同 uid 应用收到回调 | AC-7 |
| 9 | 多 callback 注册 | 按顺序执行 | AC-8 |
| 10 | off('processLifecycle') | 注销成功 | AC-9 |

## 兼容性声明

- **已有 API 行为变更:** 否。新增 `on('processLifecycle')` 不修改已有 `on` 类型。现有 `AbilityLifecycleCallback` 行为不变。
- **IPC 序列化格式变更:** 新增 `ScheduleProcessLifecycleEvent` 到 `IAppScheduler`，不影响已有 IPC 事务码。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| uid 过滤分发 | AMS 侧按 uid 过滤，仅向目标应用所属进程分发，无跨应用信息泄露 | AC-7 |
| 复用 IApplicationStateObserver | 不新增 IPC 接口，复用已有 `OnProcessCreated`/`OnProcessDied` 方法 | 全部 |
| childProcessRecordMap_ 天然排除 SELF_FORK | SELF_FORK 进程不走 `StartChildProcess` 路径，不在 Map 中 | AC-4 |
| 多回调模式参照 ApplicationStateChangeCallback | 基类 + weak_ptr vector + 顺序遍历 | AC-8 |
| state 字段不透传信号 | 退出 state 固定 "EXITED"，不包含退出信号信息 | AC-3 |
| 零权限 | 不调用 `VerifyCallingPermission`，uid 过滤由 AMS 保证安全 | AC-6 |

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
- [x] 范围边界明确（仅进程级回调；SELF_FORK/自身退出/信号透传不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "ApplicationStateChangeCallback 的多回调顺序执行模式与 ApplicationContext 注册/分发"
  - repo: "ability_ability_runtime"
    query: "IApplicationStateObserver 的 OnProcessCreated/OnProcessDied IPC 链路"
  - repo: "ability_ability_runtime"
    query: "AppMgrServiceInner::StartChildProcess / LoadAbility 进程创建路径"
  - repo: "ability_ability_runtime"
    query: "js_application_context_utils.cpp 的 on/off 事件类型分发模式"
  - repo: "ability_ability_runtime"
    query: "MainThread 的 IAppScheduler Schedule* IPC 接收模式"
```

**关键文档：**
- `interfaces/kits/native/appkit/ability_runtime/context/application_context.h`（Register/Unregister 模板，:47-58）
- `interfaces/kits/native/appkit/ability_runtime/context/application_state_change_callback.h`（多回调基类模板，:28）
- `interfaces/inner_api/app_manager/include/appmgr/iapplication_state_observer.h`（OnProcessCreated/OnProcessDied，:60,:88）
- `services/appmgr/src/app_mgr_service_inner.cpp`（StartChildProcess :11012，LoadAbility :2157）
- `services/appmgr/include/app_running_record.h`（childProcessRecordMap_ :1410）
- `frameworks/native/appkit/ability_runtime/context/js_application_context_utils.cpp`（on/off 事件分发，:1644-1656）
- `frameworks/native/appkit/app/main_thread.cpp`（IAppScheduler IPC 接收，:436,:507,:877）
