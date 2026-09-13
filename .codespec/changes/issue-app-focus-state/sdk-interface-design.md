# SDK 接口设计文档：获焦应用变化通知

> 基于 `proposal/20260908584366-05-proposal.md` 和 `.codespec/changes/issue-app-focus-state/spec.md` 设计。
> 参照 `interface_sdk-js` 现有 `@ohos.app.ability.appManager.d.ts` 及配套 `application/*.d.ts` 模式。

## 1. 设计概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 获焦应用变化通知 |
| API 命名空间 | `appManager`（`@ohos.app.ability.appManager`） |
| 方法名 | `onAppFocusStateChange` / `offAppFocusStateChange` |
| 开放范围 | Public |
| 权限 | `ohos.permission.RUNNING_STATE_OBSERVER` |
| SystemCapability | `SystemCapability.Ability.AbilityRuntime.Core` |
| 目标版本 | API 27（OpenHarmony 7.1） |
| 模式参照 | `appManager.onApplicationStateChange(observer): int` / `appManager.offApplicationStateChange(observerId): void` |
| 动静态归一 | 仅一套接口，同时适用于 dynamic 和 static，不分别提供 static-only 方法 |
| 数据结构参照 | `AbilityFirstFrameStateData` |
| Observer 模板参照 | `AbilityFirstFrameStateObserver`（条件编译 import + `export interface`） |

## 2. 产出文件清单

| 文件 | 目标路径（interface_sdk-js 仓） | 用途 |
|------|------|------|
| `AppFocusStateObserver.d.ts` | `api/application/AppFocusStateObserver.d.ts` | Observer 接口类型（动态+静态条件编译） |
| `FocusStateData.d.ts` | `api/application/FocusStateData.d.ts` | 通知数据结构 |
| `AppFocusStateObserver.static.d.ets` | `api/application/AppFocusStateObserver.static.d.ets` | ArkTS 静态类型声明 |
| `FocusStateData.static.d.ets` | `api/application/FocusStateData.static.d.ets` | ArkTS 静态类型声明 |
| `appManager.d.ts.snippet.md` | —（设计参考） | 需添加到 `@ohos.app.ability.appManager.d.ts` 的声明片段 |

> 以上 4 个 `.d.ts` / `.static.d.ets` 文件已在本目录（`.codespec/changes/issue-app-focus-state/`）产出，合入时复制到 `interface_sdk-js/api/application/` 目录。

## 3. 接口契约

### 3.1 JS API（动静态归一，直接方法命名）

```typescript
// 注册（返回 observerId）
function onAppFocusStateChange(observer: AppFocusStateObserver): int;

// 注销（通过 observerId 精确注销单个回调，不提供解注册全部的接口）
function offAppFocusStateChange(observerId: int): void;
```

> **接口设计要点：**
> - **直接方法命名：** 使用 `onAppFocusStateChange(observer)` / `offAppFocusStateChange(observerId)` 直接方法命名，不使用 `on(type: 'appFocusState', ...)` 字符串分发模式。
> - **注册/解注册成对调用：** `onAppFocusStateChange` 返回 `observerId`，`offAppFocusStateChange` 接收 `observerId` 精确注销单个回调。参照 `appManager.onApplicationStateChange(observer): int` / `appManager.offApplicationStateChange(observerId): void` 模式。
> - **不提供解注册全部回调的接口：** `offAppFocusStateChange` 必须传入 `observerId`，不提供省略参数注销全部的接口。
> - **错误码用 throw 方式：** 参数校验失败等错误通过 `throw BusinessError` 抛出，不使用 callback 返回。
> - **动静态归一：** 仅提供一套接口，同时适用于 dynamic 和 static，不分别提供 static-only 方法。

### 3.3 AppFocusStateObserver 接口类型

```typescript
export interface AppFocusStateObserver {
    onFocusStateChanged(focusStateData: FocusStateData): void;
}
```

### 3.4 FocusStateData 数据结构

```typescript
export interface FocusStateData {
    focusedUid: int;              // 获焦应用 UID
    unfocusedUid: int;            // 失焦应用 UID（-1 表示无前一个获焦应用）
    focusedBundleName: string;    // 获焦应用 bundle name
    unfocusedBundleName: string;  // 失焦应用 bundle name（空串表示无前一个获焦应用）
}
```

### 3.5 错误码

| 错误码 | 含义 | 触发条件 |
|--------|------|----------|
| 201 | Permission denied | 调用方无 `RUNNING_STATE_OBSERVER` 权限 |
| 202 | Not system application | 调用方非系统应用 |
| 401 | Parameter error | 参数缺失/类型错误/校验失败 |
| 16000050 | Internal error | 系统服务内部错误 |
| 16000165 | The number of registered observers for the current application has reached the upper limit | 同一应用注册的 observer 进程数达到 10 个上限 |

## 4. 设计决策

### 4.1 API 命名：`onAppFocusStateChange` / `offAppFocusStateChange`

| 决策 | 理由 |
|------|------|
| 方法名 `onAppFocusStateChange` / `offAppFocusStateChange` | 与现有 `onApplicationStateChange` / `offApplicationStateChange` 命名风格一致：`on`/`off` + `App` + `FocusState` + `Change`；使用直接方法命名而非 `on(type, ...)` 字符串分发 |
| 回调方法 `onFocusStateChanged` | 与现有 `onAppStateChanged`、`onAbilityFirstFrameDrawn` 命名风格一致：`on` + 主体 + 动词过去分词 |
| 数据结构 `FocusStateData` | 与现有 `AppStateData`、`AbilityStateData`、`AbilityFirstFrameStateData` 命名一致：主体 + `StateData` |

### 4.2 返回值：`int`（observerId）+ observerId 注销模式

| 对比项 | `on('applicationState')` | `on('appForegroundState')` | **`onAppFocusStateChange`** |
|--------|--------------------------|----------------------------|------------------------------|
| 返回值 | `int`（observer ID） | `void` | **`int`（observerId）** |
| off 方式 | `off(type, observerId: int)` | `off(type, observer?)` | **`offAppFocusStateChange(observerId: int)`** |
| 模式 | ID 注册/注销 | 对象注册/注销 | **ID 注册/注销（直接方法命名）** |
| 动静态 | 分别提供 | 分别提供 | **归一，一套接口** |
| 错误处理 | callback | 无 throw | **throw BusinessError** |
| 解注册全部 | 不支持 | 支持（省略 observer） | **不支持（必须传 observerId）** |

选择 `int` + observerId 注销模式，参照 `appManager.onApplicationStateChange` / `offApplicationStateChange`，因为：
- observerId 成对注册/解注册，精确管理单个回调
- 不提供解注册全部的接口，防止误操作
- 错误通过 throw 抛出，不使用 callback
- 动静态归一，仅一套接口

### 4.3 `unfocusedUid` = -1 语义

| 场景 | `unfocusedUid` 值 | 含义 |
|------|-------------------|------|
| 跨应用切换（A→B） | A 的 UID | 正常失焦 |
| 首次获焦（开机后第一个应用获焦） | -1 | 无前一个获焦应用 |
| window_manager 无法提供失焦 UID | -1 | 降级标记 |

> spec Q-2 的处理建议：unfocusedUid 为 -1 时不抑制通知（调用方自行判断），但 design.md 需明确此语义。

### 4.4 条件编译模式

采用 `AbilityFirstFrameStateObserver.d.ts` 的条件编译模式（而非 `AppForegroundStateObserver.d.ts` 的 `export default class` 模式）：

```typescript
/*** if arkts dynamic */
import type FocusStateData from './FocusStateData';
/*** endif */
/*** if arkts static */
import { FocusStateData } from './FocusStateData';
/*** endif */

export interface AppFocusStateObserver {
    onFocusStateChanged(focusStateData: FocusStateData): void;
}
```

理由：
- `export interface` 比 `export default class` 更适合纯回调接口（无字段、无构造逻辑）
- 条件编译在一个文件内同时支持动态和静态模式的 import 语法
- 仍需单独的 `.static.d.ets` 文件提供静态模式的完整声明（含 `'use static'` 指令和 `declare interface`）
- d.ts 文件内仍需条件编译处理 import 语法差异，但 **API 方法本身动静态归一**（`@since 27 dynamic&static`），不分别提供 dynamic-only 和 static-only 方法

### 4.5 字段类型：`int` 而非 `number`

使用 `int` 类型（OpenHarmony SDK 扩展类型）而非标准 TypeScript 的 `number`，与现有 SDK 声明一致：
- `AppStateData.uid: int`
- `AppStateData.state: int`
- `AbilityFirstFrameStateData.appIndex: int`

### 4.6 访问级别：Public API

spec 指定"开放范围: Public"，但实际使用场景为 MDM 系统应用。设计为 Public API 的理由：
- proposal 明确要求"Public API"
- 与 `onApplicationStateChange` 一致（同为 Public + `RUNNING_STATE_OBSERVER` 权限）
- `RUNNING_STATE_OBSERVER` 为 system_grant 权限，仅系统应用可获得，实际效果等价 system-only
- JSDoc 中仍声明 `@throws 202 - Not system application`，因为底层 `VerifyAppStateObserverPermission` 会检查系统应用身份

> **API Review 待确认**：若 Review 决定改为 `@systemapi`，需在 `onAppFocusStateChange`/`offAppFocusStateChange` 函数和 `AppFocusStateObserver`/`FocusStateData` 类型声明上添加 `@systemapi` 标签。

## 5. 与现有 API 的关系

| 现有 API | 关系 | 说明 |
|----------|------|------|
| `onApplicationStateChange(observer): int` | 参照模式 | 前者监听应用**生命周期**变化（创建/前台/后台/销毁）；新增 API 监听应用**焦点**变化（获焦/失焦）；二者均使用直接方法命名 + observerId 返回 |
| `onAppForegroundStateChange(observer)` | 并行 | 前者监听应用**前台/后台切换**；新增 API 监听应用间**焦点切换**（同一应用内窗口切换不通知） |
| `onAbilityFirstFrameStateChange(observer)` | 并行 | 前者监听 Ability **首帧渲染完成**；新增 API 监听应用**获焦**事件 |
| `ApplicationStateObserver` | 独立 | 不复用、不扩展 `ApplicationStateObserver`（spec 非目标："选定独立接口"） |

## 6. NAPI 分发模式（实现侧参考）

本特性**不使用** `on(type, ...)` 字符串分发模式，而是在 `js_app_manager.cpp` 中直接导出 `OnAppFocusStateChange` / `OffAppFocusStateChange` NAPI 方法：

```
新增方法导出:  OnAppFocusStateChange (参照 OnApplicationStateChange 模式，返回 observerId)
新增方法导出:  OffAppFocusStateChange (参照 OffApplicationStateChange 模式，接收 observerId)
新增成员:  sptr<JSFocusStateObserver> observerFocus_ (参照 observerForeground_)
新增文件:  js_focus_state_observer.h / js_focus_state_observer.cpp (参照 js_app_foreground_state_observer)
```

`OnAppFocusStateChange` 内部调用 `RegisterFocusStateObserver` 注册 observer 并返回 observerId（`napi_create_int32`）；`OffAppFocusStateChange` 接收 observerId 调用 `UnregisterFocusStateObserver` 精确注销，参数错误时 throw `BusinessError(401)`。

## 7. 验证对照表

| Spec AC | SDK 接口覆盖 | 验证方式 |
|---------|-------------|----------|
| AC-1: 持权限注册成功 | `onAppFocusStateChange(observer): int` 声明 + 权限注解 + 返回 observerId | JS API 单测（含权限校验 mock） |
| AC-2: 无权限返回错误码 | `@throws BusinessError 201` | JS API 单测 |
| AC-3: OnFocused 回调参数 | `FocusStateData` 接口含 `focusedUid`/`unfocusedUid`/`bundleName` | 类型编译验证 |
| AC-4: 跨应用切换通知 | `onFocusStateChanged(focusStateData)` 回调声明 | 端到端集成测试 |
| AC-5: 同应用不通知 | JSDoc: "Focus changes within the same application do not trigger this callback" | 集成测试 |
| AC-6: off 注销 | `offAppFocusStateChange(observerId: int): void` 声明 | JS API 单测 |
| AC-10: 不支持屏幕 | `@throws 16000050` + 条件编译 | 条件编译验证 |

## 8. 待确认项

| # | 问题 | 处理建议 |
|---|------|----------|
| 1 | API 版本号 `@since 27` 为推断值 | 需 API Review 确认实际引入版本 |
| 2 | 访问级别 Public vs `@systemapi` | spec 指定 Public，但底层权限校验含系统应用检查。若 Review 改为 `@systemapi`，需在 4 处声明添加标签 |
| 3 | `unfocusedUid = -1` 语义是否在 JSDoc 中明确 | 当前已在 `FocusStateData.unfocusedUid` 字段注释中说明 |
| 4 | 中英文翻译同步 | `zh-cn/` 目录需同步产出中文翻译文件 |
| 5 | 动静态归一后 d.ts 文件结构 | d.ts 文件内仍需条件编译处理 import 语法（`import type` vs `import { }`），但 API 方法本身使用 `@since 27 dynamic&static` 统一标注，不分别提供 dynamic-only 和 static-only 方法。`appManager.d.ts.snippet.md` 中 export type 也已统一为 `@since 27 dynamic&static` |
