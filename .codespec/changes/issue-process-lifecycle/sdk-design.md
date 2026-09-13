# SDK 接口设计：进程级生命周期回调

> 基于 `spec.md`（REQ-PROCESS-LIFECYCLE-001）设计。**对外仅暴露 JS/ArkTS d.ts 公共 API**，C++ 层（回调基类、ApplicationContext 注册/注销、IAppScheduler IPC）均为内部实现，不在 C++ Native SDK 中对外声明。

## 设计参照

| 参照项 | 现有接口 | 参照要点 |
|--------|---------|---------|
| JS 直接方法命名模式 | `appManager.onApplicationStateChange(observer): int` / `appManager.offApplicationStateChange(observerId): void` | 直接方法命名（非 `on(type, ...)` 字符串分发），`onApplicationStateChange` 返回 `int`（observerId），`offApplicationStateChange` 接收 `observerId` 精确注销单个回调；错误通过 `throw BusinessError` 抛出（不使用 callback） |
| 动静态归一 | 仅一套 `onProcessLifecycle(callback): int` / `offProcessLifecycle(observerId): void` | 同时适用于 dynamic 和 static，不分别提供 static-only 方法 |
| JS 回调类模式 | `ApplicationStateObserver.d.ts` | `export default class` + void 方法 + `@stagemodelonly` |
| JS 信息结构体模式 | `ChildProcessInformation.d.ts` / `ProcessInformation.d.ts` | `export interface` + `pid`/`processName` 等字段 |

## 1. JS/ArkTS 公共 API（唯一对外接口）

### 1.1 新增 d.ts 文件：`ProcessLifecycleCallback.d.ts`

**文件位置：** `interface_sdk-js/api/@ohos.app.ability.ProcessLifecycleCallback.d.ts`

```typescript
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @kit AbilityKit
 */

import type { ProcessLifecycleInfo } from './application/ProcessLifecycleInfo';

/**
 * Provides the callbacks for process lifecycle. The callback is triggered when a process of the current application
 * is created or exits.
 *
 * To listen for process lifecycle events, register this callback through
 * {@link ApplicationContext.onProcessLifecycle | ApplicationContext.onProcessLifecycle()}.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 27.0.0 dynamic&static
 */
export default class ProcessLifecycleCallback {
    /**
     * Called when a process of the current application is created.
     *
     * The process types that can trigger this callback include:
     * - Child processes started by {@link childProcessManager.startChildProcess} (APP_SPAWN_FORK mode),
     *   {@link childProcessManager.startArkChildProcess}, or {@link childProcessManager.startNativeChildProcess}.
     * - Native child processes started by {@link OH_Ability_CreateNativeChildProcess} or
     *   {@link OH_Ability_StartNativeChildProcess}.
     * - Render processes (a subtype of child processes).
     * - Preloaded template processes.
     * - Worker processes derived from preloaded templates.
     * - Independent processes (multi-process HAP modules).
     *
     * SELF_FORK processes and mirror processes are not included.
     *
     * @param { ProcessLifecycleInfo } info - Information about the created process.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    onProcessCreated(info: ProcessLifecycleInfo): void;

    /**
     * Called when a process of the current application exits, including abnormal exits such as crash or kill -9.
     *
     * The process types that can trigger this callback are the same as those described in
     * {@link onProcessCreated}. SELF_FORK processes and mirror processes are not included.
     *
     * The exiting process itself does not receive this callback. Only other surviving processes
     * of the same application that have registered this callback will be notified.
     *
     * @param { ProcessLifecycleInfo } info - Information about the exited process.
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    onProcessExited(info: ProcessLifecycleInfo): void;
}
```

### 1.2 新增 d.ts 文件：`ProcessLifecycleInfo.d.ts`

**文件位置：** `interface_sdk-js/api/application/ProcessLifecycleInfo.d.ts`

```typescript
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @kit AbilityKit
 */

/**
 * Defines the process lifecycle information.
 *
 * @syscap SystemCapability.Ability.AbilityRuntime.Core
 * @stagemodelonly
 * @since 27.0.0 dynamic&static
 */
export interface ProcessLifecycleInfo {
    /**
     * PID of the process.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    pid: number;

    /**
     * Name of the process.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    processName: string;

    /**
     * UID of the process.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    uid: number;

    /**
     * Process lifecycle state. The value can be:
     * - `'CREATED'`: The process is created.
     * - `'EXITED'`: The process exits.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @stagemodelonly
     * @since 27.0.0 dynamic&static
     */
    state: string;
}
```

### 1.3 修改 `ApplicationContext.d.ts`：新增 `onProcessLifecycle`/`offProcessLifecycle` 方法

**文件位置：** `interface_sdk-js/api/application/ApplicationContext.d.ts`

在现有 `ApplicationContext` class 中新增以下方法（直接方法命名，参照 `appManager.onApplicationStateChange` / `offApplicationStateChange` 模式）：

```typescript
import type { ProcessLifecycleCallback } from '../@ohos.app.ability.ProcessLifecycleCallback';

  /**
   * Registers a callback to listen for process lifecycle events of the current application.
   *
   * When registered, the callback will be invoked when any process of the current application is created or exits,
   * including child processes (APP_SPAWN_FORK mode), native child processes, render processes (a subtype of child
   * processes), preloaded template processes, worker processes derived from templates, and independent processes
   * (multi-process HAP modules). SELF_FORK processes and mirror processes are not included.
   *
   * The callback is invoked on all surviving processes of the same application that have registered this callback.
   * The exiting process itself does not receive the callback.
   *
   * No permission is required.
   *
   * @param { ProcessLifecycleCallback } callback - Callback to register.
   * @returns { int } ID of the observer registered. You can pass this ID to
   *     [offProcessLifecycle]{@link ApplicationContext.offProcessLifecycle} to unregister the callback.
   * @throws { BusinessError } 401 - Parameter error. Possible causes:
   *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types.
   * @throws { BusinessError } 16000050 - Internal error.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 27.0.0 dynamic&static
   */
  onProcessLifecycle(callback: ProcessLifecycleCallback): int;

  /**
   * Unregisters a callback for process lifecycle events by observerId.
   *
   * @param { int } observerId - ID of the observer registered, which is the ID returned by
   *     [onProcessLifecycle]{@link ApplicationContext.onProcessLifecycle}.
   * @throws { BusinessError } 401 - Parameter error. Possible causes:
   *     1. Mandatory parameters are left unspecified; 2. Incorrect parameter types.
   * @throws { BusinessError } 16000050 - Internal error.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @stagemodelonly
   * @since 27.0.0 dynamic&static
   */
  offProcessLifecycle(observerId: int): void;
```

### 1.4 JS API 设计说明

| 设计决策 | 说明 | 关联 spec AC |
|---------|------|-------------|
| 直接方法命名（非 on/off 字符串分发） | 使用 `onProcessLifecycle(callback)` / `offProcessLifecycle(observerId)` 直接方法命名，不使用 `on(type: 'processLifecycle', ...)` 字符串分发模式 | AC-1, AC-9 |
| observerId 成对注册/解注册模式 | 参照 `appManager.onApplicationStateChange(observer): int` / `appManager.offApplicationStateChange(observerId): void`；`onProcessLifecycle` 返回 observerId，`offProcessLifecycle` 接收 observerId 精确注销单个回调 | AC-1, AC-9 |
| 不提供解注册全部接口 | `offProcessLifecycle` 必须传入 `observerId`，不提供省略参数注销全部的接口 | AC-9 |
| 错误通过 throw 抛出 | 参数校验失败等错误通过 `throw BusinessError` 抛出，不使用 callback 返回 | AC-1, AC-9 |
| 动静态归一 | 仅一套 `onProcessLifecycle(callback): int` / `offProcessLifecycle(observerId): void`，同时适用于 dynamic 和 static，不提供 static-only 方法 | 全部 |
| `ProcessLifecycleInfo` 结构体 | 遵循 `ChildProcessInformation` / `ProcessInformation` 模式，用结构体而非散参数，便于后续扩展 | 全部 |
| `state` 为 string 而非 enum | 与 spec 定义一致：`"CREATED"` / `"EXITED"`；不透传退出信号 | AC-3, AC-11 |
| 零权限 | 无 `@permission` 标注 | AC-6 |
| `@stagemodelonly` | 与所有 ApplicationContext 方法一致 | — |
| `@since 27.0.0 dynamic&static` | 新增 API 版本，参照 `getChildProcessInfos` 的 `26.1.0` 递增 | — |

## 2. 内部实现说明（非对外 API）

> 以下内容为 JS API 的内部实现链路，不在 `interfaces/kits/native/` 或 `interface_sdk_c/` 中对外声明。仅供开发参考。

### 2.1 JS 绑定层（`frameworks/native/appkit/`）

**新增内部文件：**

| 文件 | 说明 |
|------|------|
| `ability_runtime/context/js_process_lifecycle_callback.h` | 内部 C++ 类，参照 `appManager.on('applicationState')` observerId 模式：持有 `napi_env` + `std::map<int, NativeReference>` 回调映射（observerId → JS 回调），提供 `Register(callback)→int` / `UnRegister(observerId)→bool` / `IsEmpty()` 方法 |
| `ability_runtime/context/js_process_lifecycle_callback.cpp | 实现回调分发：`CallJsMethod(info, methodName)` 遍历 JS 回调映射，构造 `ProcessLifecycleInfo` JS 对象并调用 |

**修改内部文件：**

| 文件 | 修改点 |
|------|--------|
| `ability_runtime/context/js_application_context_utils.cpp` | 新增 `OnProcessLifecycle` / `OffProcessLifecycle` NAPI 方法导出（非 `on(type, ...)` 字符串分发）：`OnProcessLifecycle` 调用 `Register(callback)` 返回 observerId（`napi_create_int32`）；`OffProcessLifecycle` 调用 `UnRegister(observerId)` 精确注销单个回调，参数错误时 throw `BusinessError(401)` |

### 2.2 C++ 内部分发层（`frameworks/native/appkit/`）

**修改内部文件：**

| 文件 | 修改点 |
|------|--------|
| `ability_runtime/context/application_context.h` | 新增**内部**方法：`RegisterProcessLifecycleCallback(weak_ptr)→int`（返回 observerId）/ `UnregisterProcessLifecycleCallback(int observerId)→bool` / `DispatchOnProcessCreated(info)` / `DispatchOnProcessExited(info)`；新增 `static vector<weak_ptr>` + `static recursive_mutex` + `static int nextObserverId_` |
| `ability_runtime/context/application_context.cpp` | 实现注册/注销/分发：仿照 `ApplicationStateChangeCallback` 的 `weak_ptr` vector + 顺序遍历 + lock/erase 过期回调；`Register` 分配递增 observerId 并返回，`Unregister` 按 observerId 精确移除 |

> **注意：** 这些方法仅声明在 `application_context.h`（`frameworks/` 内部头文件），不在 `interfaces/kits/native/appkit/ability_runtime/context/application_context.h`（SDK 公共头文件）中声明。

### 2.3 IPC 层（`interfaces/inner_api/` + `services/appmgr/`）

**修改内部文件：**

| 文件 | 修改点 |
|------|--------|
| `app_manager/include/appmgr/app_scheduler_interface.h` | `IAppScheduler` 新增纯虚 `ScheduleProcessLifecycleEvent(pid, processName, uid, state)` + `enum Message` 新增 `SCHEDULE_PROCESS_LIFECYCLE_EVENT` 事务码 |
| `app_manager/src/app_scheduler_proxy.cpp` | Proxy 序列化：`WriteInt32(pid)` + `WriteString(processName)` + `WriteInt32(uid)` + `WriteString(state)` + `SendRequest(TF_ASYNC)` |
| `appmgr/src/app_scheduler_stub.cpp`（或 `main_thread.cpp` 内联 stub） | Stub 反序列化：`ReadInt32` + `ReadString` × 2 + `ReadInt32` → 调用 `MainThread::ScheduleProcessLifecycleEvent` |
| `appmgr/src/app_mgr_service_inner.cpp` | 新增 `DispatchProcessLifecycleEvent(pid, processName, uid, state)` 统一分发方法：遍历 `GetAppRunningRecordMap()` 按 uid 过滤、排除自身 pid、逐个 `GetApplicationClient()->ScheduleProcessLifecycleEvent()`。在 4 处触发点调用（子进程含 Render 创建/退出、独立进程/预加载模板/工作进程创建/退出） |
| `appkit/app/main_thread.cpp` | Stub 接收 `ScheduleProcessLifecycleEvent` → 构造 `ProcessLifecycleInfo` → `ApplicationContext::DispatchOnProcessCreated/Exited` |

**AMS 侧触发点调用：**

| 触发时机 | 调用位置 | 参数 |
|---------|---------|------|
| 子进程创建（含 Render） | `StartChildProcessImpl` 成功后（:11204 附近）/ `AttachRenderProcess` 成功后（:8875 附近） | `pid, processName, hostRecord->GetUid(), "CREATED"` |
| 子进程退出（含 Render） | `OnChildProcessDied`（:11422）/ `OnRenderProcessDied` 中 | `pid, processName, hostRecord->GetUid(), "EXITED"` |
| 独立进程/预加载模板/工作进程创建 | `StartProcess` 成功后（:6340 附近） | `pid, processName, appRecord->GetUid(), "CREATED"` |
| 独立进程/预加载模板/工作进程退出 | `OnProcessDied`（:6765 / :13157）中 | `pid, processName, appRecord->GetUid(), "EXITED"` |

### 2.4 IPC 序列化格式

```
MessageParcel:
  [interfaceToken: string]
  [pid: int32]
  [processName: string]
  [uid: int32]
  [state: string]   // "CREATED" or "EXITED"
```

使用 `TF_ASYNC`（无需等待回复），不影响已有 IPC 事务码。

## 3. 错误码

| 错误码 | 名称 | 场景 | 抛出方式 |
|--------|------|------|----------|
| 401 | Parameter error | `onProcessLifecycle`/`offProcessLifecycle` 参数类型错误或必选参数缺失 | `throw BusinessError` |
| 16000050 | Internal error | IPC 通信异常 | `throw BusinessError` |

> 无新增错误码。零权限设计，不返回权限相关错误码（AC-6）。所有错误通过 `throw BusinessError` 抛出，不使用 callback 返回。

## 4. 文件变更清单

### SDK 公共文件（对外）

| # | 文件路径 | 操作 | 说明 |
|---|---------|------|------|
| 1 | `interface_sdk-js/api/@ohos.app.ability.ProcessLifecycleCallback.d.ts` | 新增 | JS 回调类定义 |
| 2 | `interface_sdk-js/api/application/ProcessLifecycleInfo.d.ts` | 新增 | JS 信息结构体 |
| 3 | `interface_sdk-js/api/application/ApplicationContext.d.ts` | 修改 | 新增 `onProcessLifecycle`/`offProcessLifecycle` 方法 |

### 内部实现文件（不对外）

| # | 文件路径 | 操作 | 说明 |
|---|---------|------|------|
| 4 | `frameworks/native/appkit/ability_runtime/context/js_process_lifecycle_callback.h` | 新增 | JS 绑定层回调类（内部） |
| 5 | `frameworks/native/appkit/ability_runtime/context/js_process_lifecycle_callback.cpp` | 新增 | JS 绑定层实现（内部） |
| 6 | `frameworks/native/appkit/ability_runtime/context/js_application_context_utils.cpp` | 修改 | 新增 `OnProcessLifecycle` / `OffProcessLifecycle` NAPI 方法导出 |
| 7 | `frameworks/native/appkit/ability_runtime/context/application_context.h` | 修改 | 新增内部 Register/Unregister/Dispatch 方法声明 |
| 8 | `frameworks/native/appkit/ability_runtime/context/application_context.cpp` | 修改 | 实现注册/注销/分发 |
| 9 | `interfaces/inner_api/app_manager/include/appmgr/app_scheduler_interface.h` | 修改 | 新增 `ScheduleProcessLifecycleEvent` + 事务码 |
| 10 | `interfaces/inner_api/app_manager/src/app_scheduler_proxy.cpp` | 修改 | Proxy 序列化 |
| 11 | `services/appmgr/src/app_mgr_service_inner.cpp` | 修改 | 新增 `DispatchProcessLifecycleEvent` + 4 处触发点（子进程含 Render 创建/退出、独立进程/预加载模板/工作进程创建/退出） |
| 12 | `frameworks/native/appkit/app/main_thread.cpp` | 修改 | Stub 接收 + ApplicationContext 分发 |

### BUILD.gn 变更

| 文件 | 变更 |
|------|------|
| `frameworks/native/appkit/ability_runtime/context/BUILD.gn` | 新增 `js_process_lifecycle_callback.h/.cpp` |
| `services/appmgr/BUILD.gn` | 确认新增的 `DispatchProcessLifecycleEvent` 编译到 `libappms` |

## 5. 接口调用示例

```typescript
import UIAbility from '@ohos.app.ability.UIAbility';
import ProcessLifecycleCallback from '@ohos.app.ability.ProcessLifecycleCallback';
import { BusinessError } from '@ohos.base';

class EntryAbility extends UIAbility {
    private observerId: number = -1;

    onCreate() {
        const applicationContext = this.context.getApplicationContext();
        // onProcessLifecycle returns observerId for paired offProcessLifecycle
        this.observerId = applicationContext.onProcessLifecycle({
            onProcessCreated(info) {
                console.log(`Process created: pid=${info.pid}, name=${info.processName}, uid=${info.uid}`);
            },
            onProcessExited(info) {
                console.log(`Process exited: pid=${info.pid}, name=${info.processName}, uid=${info.uid}`);
            }
        });
    }

    onDestroy() {
        const applicationContext = this.context.getApplicationContext();
        // offProcessLifecycle by observerId (paired with onProcessLifecycle)
        if (this.observerId >= 0) {
            try {
                applicationContext.offProcessLifecycle(this.observerId);
            } catch (e) {
                let err = e as BusinessError;
                console.error(`Failed to unregister: code=${err.code}, message=${err.message}`);
            }
        }
    }
}
```

## 6. 设计约束与 spec 映射

| 设计约束 | 实现方式 | 关联 spec AC |
|---------|---------|-------------|
| 统一 1:N 广播 | `DispatchProcessLifecycleEvent` 遍历 `GetAppRunningRecordMap()` 按 uid 过滤 | AC-2, AC-3, AC-10~AC-13, AC-15~AC-18 |
| 排除自身 pid | `record->GetPid() == pid` 时 continue | AC-5, AC-13 |
| uid 过滤隔离 | `record->GetUid() != uid` 时 continue | AC-7 |
| 零权限 | JS API 无 `@permission` | AC-6 |
| state 不透传信号 | `state` 固定 `"CREATED"` / `"EXITED"`，IPC 不传输 signal | AC-3, AC-11 |
| 多回调顺序执行 | `std::vector<weak_ptr>` + 顺序遍历；每次 `onProcessLifecycle` 返回独立 observerId | AC-8 |
| SELF_FORK 天然排除 | SELF_FORK 不走 AMS 路径 | AC-4 |
| 镜像进程排除 | 预加载镜像派生中间态无 AMS 独立记录 | AC-14 |
| Render 进程作为子进程的一种 | 合并到子进程触发点，不单独列出 | AC-2, AC-3 |
| observerId 成对注册/解注册 | `onProcessLifecycle` → `Register` 返回 observerId；`offProcessLifecycle` → `Unregister(observerId)` 精确注销；不提供解注册全部接口 | AC-1, AC-9 |
| 错误通过 throw 抛出 | 参数校验失败等通过 `throw BusinessError`，不使用 callback | AC-1, AC-9 |
| 动静态归一 | 仅一套 `onProcessLifecycle(callback): int` / `offProcessLifecycle(observerId): void`，同时适用于 dynamic 和 static | 全部 |
| **仅 JS 对外** | C++ 类/IPC 均在 `frameworks/`、`interfaces/inner_api/`、`services/` 内部，不在 `interfaces/kits/native/` 声明 | 全部 |
