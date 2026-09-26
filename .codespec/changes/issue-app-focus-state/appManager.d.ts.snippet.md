# appManager.d.ts 新增声明片段

> 本文件描述需要添加到 `interface_sdk-js/api/@ohos.app.ability.appManager.d.ts` 中的新增声明。
> 按 **导入区** → **on/off 函数声明** → **export type 声明** 三部分给出，每部分标注在现有文件中的插入位置。

---

## 1. 导入区新增

在现有导入区（`/*** if arkts dynamic */` / `/*** if arkts static */` 块内）追加以下导入：

### arkts dynamic 块（约 line 26-32 之间）

```typescript
import * as _AppFocusStateObserver from './application/AppFocusStateObserver';
import * as _FocusStateData from './application/FocusStateData';
```

### arkts static 块（约 line 34-42 之间）

```typescript
import { AppFocusStateObserver as _AppFocusStateObserver } from './application/AppFocusStateObserver';
import { FocusStateData as _FocusStateData } from './application/FocusStateData';
```

### 完整导入区示意（变更后）

```typescript
/*** if arkts dynamic */
import * as _ApplicationStateObserver from './application/ApplicationStateObserver';
import type * as _AppForegroundStateObserver from './application/AppForegroundStateObserver';
import * as _AbilityStateData from './application/AbilityStateData';
import * as _AppStateData from './application/AppStateData';
import type * as _ProcessData from './application/ProcessData';
import * as _AbilityFirstFrameStateObserver from './application/AbilityFirstFrameStateObserver';
import * as _AbilityFirstFrameStateData from './application/AbilityFirstFrameStateData';
import * as _AppFocusStateObserver from './application/AppFocusStateObserver';
import * as _FocusStateData from './application/FocusStateData';
/*** endif */
/*** if arkts static */
import _ApplicationStateObserver from './application/ApplicationStateObserver';
import _AbilityStateData from './application/AbilityStateData';
import _AppStateData from './application/AppStateData';
import _ProcessData from './application/ProcessData';
import _AppForegroundStateObserver from './application/AppForegroundStateObserver';
import { AbilityFirstFrameStateObserver as _AbilityFirstFrameStateObserver } from './application/AbilityFirstFrameStateObserver';
import { AbilityFirstFrameStateData as _AbilityFirstFrameStateData } from './application/AbilityFirstFrameStateData';
import { AppFocusStateObserver as _AppFocusStateObserver } from './application/AppFocusStateObserver';
import { FocusStateData as _FocusStateData } from './application/FocusStateData';
/*** endif */
```

---

## 2. on/off 函数声明

在现有 `offAbilityFirstFrameStateChange` 声明之后（约 line 1020）、`getForegroundApplications` 之前（约 line 1022）插入以下函数声明（动静态归一，直接方法命名）：

```typescript
  /**
   * Registers an observer to listen for application focus state changes. The observer is used by a system application
   * to observe the focus state changes of all applications.
   *
   * This callback is triggered only when the focus switches between different applications (different UIDs).
   * Focus changes within the same application (same UID) do not trigger the callback.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { AppFocusStateObserver } observer - Observer used to listen for application focus state changes.
   * @returns { int } ID of the observer registered. You can pass this ID to
   *     [offAppFocusStateChange]{@link appManager.offAppFocusStateChange} to unregister the observer.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 401 - Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;
   *     2. Incorrect parameter types; 3. Parameter verification failed.
   * @throws { BusinessError } 16000050 - Internal error.
   * @throws { BusinessError } 16000165 - The number of registered observers for the current application has reached
   *     the upper limit.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  function onAppFocusStateChange(observer: AppFocusStateObserver): int;

  /**
   * Unregisters an observer for application focus state changes by observerId.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { int } observerId - ID of the observer registered, which is the ID returned by
   *     [onAppFocusStateChange]{@link appManager.onAppFocusStateChange}.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 401 - Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;
   *     2. Incorrect parameter types; 3. Parameter verification failed.
   * @throws { BusinessError } 16000050 - Internal error.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  function offAppFocusStateChange(observerId: int): void;
```

---

## 3. export type 声明

在现有 `export type AbilityFirstFrameStateData` 声明之后（约 line 2047）、`export type RunningMultiAppInfo` 之前（约 line 2049）插入：

```typescript
  /**
   * Defines the observer used to listen for application focus state changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  export type AppFocusStateObserver = _AppFocusStateObserver;

  /**
   * Defines the focus state data reported when the application focus changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic&static
   */
  export type FocusStateData = _FocusStateData;
```

---

## 4. 使用示例

```typescript
import appManager from '@ohos.app.ability.appManager';
import { BusinessError } from '@ohos.base';

// 定义 observer
const observer: appManager.AppFocusStateObserver = {
  onFocusStateChanged(focusStateData: appManager.FocusStateData) {
    console.log(`Focus changed: ${JSON.stringify(focusStateData)}`);
    console.log(`  focusedUid: ${focusStateData.focusedUid}`);
    console.log(`  unfocusedUid: ${focusStateData.unfocusedUid}`);
    console.log(`  focusedBundleName: ${focusStateData.focusedBundleName}`);
  }
};

// 注册监听（返回 observerId）
let observerId = appManager.onAppFocusStateChange(observer);

// 注销监听（通过 observerId 精确注销）
try {
  appManager.offAppFocusStateChange(observerId);
} catch (e) {
  let err = e as BusinessError;
  console.error(`Failed to unregister: code=${err.code}, message=${err.message}`);
}
```
