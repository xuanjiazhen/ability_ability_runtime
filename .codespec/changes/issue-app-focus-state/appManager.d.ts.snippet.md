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

在现有 `off('abilityFirstFrameState')` 声明之后（约 line 1020）、`getForegroundApplications` 之前（约 line 1022）插入以下函数声明：

```typescript
  /**
   * Registers an observer to listen for application focus state changes. The observer is used by a system application
   * to observe the focus state changes of all applications.
   *
   * This callback is triggered only when the focus switches between different applications (different UIDs).
   * Focus changes within the same application (same UID) do not trigger the callback.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { 'appFocusState' } type - Event type. It is fixed at **'appFocusState'**.
   * @param { AppFocusStateObserver } observer - Observer used to listen for application focus state changes.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 401 - Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;
   *     2. Incorrect parameter types; 3. Parameter verification failed.
   * @throws { BusinessError } 16000050 - Internal error.
   * @throws { BusinessError } 16000165 - The number of registered observers for the current application has reached
   *     the upper limit.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic
   */
  function on(type: 'appFocusState', observer: AppFocusStateObserver): void;

  /**
   * Register application focus state observer.
   *
   * This callback is triggered only when the focus switches between different applications (different UIDs).
   * Focus changes within the same application (same UID) do not trigger the callback.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { AppFocusStateObserver } observer - The application focus state observer.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 16000050 - Internal error.
   * @throws { BusinessError } 16000165 - The number of registered observers for the current application has reached
   *     the upper limit.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 static
   */
  function onAppFocusStateChange(observer: AppFocusStateObserver): void;

  /**
   * Unregisters the observer used to listen for application focus state changes.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { 'appFocusState' } type - Event type. It is fixed at **'appFocusState'**.
   * @param { AppFocusStateObserver } [observer] - Observer used to listen for application focus state changes. If this
   *     parameter is left blank, all subscriptions to the specified event are canceled.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 401 - Parameter error. Possible causes: 1. Mandatory parameters are left unspecified;
   *     2. Incorrect parameter types; 3. Parameter verification failed.
   * @throws { BusinessError } 16000050 - Internal error.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic
   */
  function off(type: 'appFocusState', observer?: AppFocusStateObserver): void;

  /**
   * Unregister application focus state observer.
   *
   * @permission ohos.permission.RUNNING_STATE_OBSERVER
   * @param { AppFocusStateObserver } [observer] - The application focus state observer. If this parameter is left
   *     blank, all subscriptions are canceled.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 16000050 - Internal error.
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 static
   */
  function offAppFocusStateChange(observer?: AppFocusStateObserver): void;
```

---

## 3. export type 声明

在现有 `export type AbilityFirstFrameStateData` 声明之后（约 line 2047）、`export type RunningMultiAppInfo` 之前（约 line 2049）插入：

```typescript
  /**
   * Defines the observer used to listen for application focus state changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic
   */
  export type AppFocusStateObserver = _AppFocusStateObserver.default;

  /**
   * Defines the observer used to listen for application focus state changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 static
   */
  export type AppFocusStateObserver = _AppFocusStateObserver;

  /**
   * Defines the focus state data reported when the application focus changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 dynamic
   */
  export type FocusStateData = _FocusStateData.default;

  /**
   * Defines the focus state data reported when the application focus changes.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @since 27 static
   */
  export type FocusStateData = _FocusStateData;
```

---

## 4. 使用示例

```typescript
import appManager from '@ohos.app.ability.appManager';

// 定义 observer
const observer: appManager.AppFocusStateObserver = {
  onFocusStateChanged(focusStateData: appManager.FocusStateData) {
    console.log(`Focus changed: ${JSON.stringify(focusStateData)}`);
    console.log(`  focusedUid: ${focusStateData.focusedUid}`);
    console.log(`  unfocusedUid: ${focusStateData.unfocusedUid}`);
    console.log(`  bundleName: ${focusStateData.bundleName}`);
  }
};

// 注册监听
appManager.on('appFocusState', observer);

// 注销监听（指定 observer）
appManager.off('appFocusState', observer);

// 注销监听（注销全部）
appManager.off('appFocusState');
```

### ArkTS 静态模式使用示例

```typescript
import appManager from '@ohos.app.ability.appManager';

// 注册监听
appManager.onAppFocusStateChange({
  onFocusStateChanged(focusStateData: appManager.FocusStateData) {
    console.log(`Focus changed: focused=${focusStateData.focusedUid}, unfocused=${focusStateData.unfocusedUid}`);
  }
});

// 注销监听
appManager.offAppFocusStateChange();
```
