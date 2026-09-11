# SDK 接口设计文档：获焦应用变化通知

> 基于 `proposal/20260908584366-05-proposal.md` 和 `.codespec/changes/issue-app-focus-state/spec.md` 设计。
> 参照 `interface_sdk-js` 现有 `@ohos.app.ability.appManager.d.ts` 及配套 `application/*.d.ts` 模式。

## 1. 设计概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 获焦应用变化通知 |
| API 命名空间 | `appManager`（`@ohos.app.ability.appManager`） |
| 事件类型字符串 | `'appFocusState'` |
| 开放范围 | Public |
| 权限 | `ohos.permission.RUNNING_STATE_OBSERVER` |
| SystemCapability | `SystemCapability.Ability.AbilityRuntime.Core` |
| 目标版本 | API 27（OpenHarmony 7.1） |
| 模式参照 | `on('appForegroundState')` / `off('appForegroundState')` |
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

### 3.1 JS API（动态模式）

```typescript
// 注册
function on(type: 'appFocusState', observer: AppFocusStateObserver): void;

// 注销（指定 observer）
function off(type: 'appFocusState', observer?: AppFocusStateObserver): void;
```

### 3.2 JS API（静态模式）

```typescript
// 注册
function onAppFocusStateChange(observer: AppFocusStateObserver): void;

// 注销
function offAppFocusStateChange(observer?: AppFocusStateObserver): void;
```

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

### 4.1 API 命名：`appFocusState` 而非 `focusState` 或 `appFocus`

| 决策 | 理由 |
|------|------|
| 事件类型 `'appFocusState'` | 与现有 `'appForegroundState'`、`'applicationState'`、`'abilityFirstFrameState'` 命名风格一致：`app` + `Focus` + `State` |
| 回调方法 `onFocusStateChanged` | 与现有 `onAppStateChanged`（AppForegroundStateObserver）、`onAbilityFirstFrameDrawn` 命名风格一致：`on` + 主体 + 动词过去分词 |
| 静态方法 `onAppFocusStateChange` / `offAppFocusStateChange` | 与现有 `onAppForegroundStateChange` / `offAppForegroundStateChange` 命名一致：`on`/`off` + `App` + `FocusState` + `Change` |
| 数据结构 `FocusStateData` | 与现有 `AppStateData`、`AbilityStateData`、`AbilityFirstFrameStateData` 命名一致：主体 + `StateData` |

### 4.2 返回值：`void` 而非 `int`

| 对比项 | `on('applicationState')` | `on('appForegroundState')` | **`on('appFocusState')`** |
|--------|--------------------------|----------------------------|--------------------------|
| 返回值 | `int`（observer ID） | `void` | **`void`** |
| off 方式 | `off(type, observerId: int)` | `off(type, observer?)` | **`off(type, observer?)`** |
| 模式 | ID 注册/注销 | 对象注册/注销 | **对象注册/注销** |

选择 `void` + 对象注销模式，与 `appForegroundState` 一致，因为：
- focus state observer 与 foreground state observer 生命周期管理方式相同
- 不需要 ID 中间量，observer 对象直接作为注册/注销句柄
- 简化调用方代码

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
- 条件编译在一个文件内同时支持动态和静态模式
- 仍需单独的 `.static.d.ets` 文件提供静态模式的完整声明（含 `'use static'` 指令和 `declare interface`）

### 4.5 字段类型：`int` 而非 `number`

使用 `int` 类型（OpenHarmony SDK 扩展类型）而非标准 TypeScript 的 `number`，与现有 SDK 声明一致：
- `AppStateData.uid: int`
- `AppStateData.state: int`
- `AbilityFirstFrameStateData.appIndex: int`

### 4.6 访问级别：Public API

spec 指定"开放范围: Public"，但实际使用场景为 MDM 系统应用。设计为 Public API 的理由：
- proposal 明确要求"Public API"
- 与 `on('applicationState')` 一致（同为 Public + `RUNNING_STATE_OBSERVER` 权限）
- `RUNNING_STATE_OBSERVER` 为 system_grant 权限，仅系统应用可获得，实际效果等价 system-only
- JSDoc 中仍声明 `@throws 202 - Not system application`，因为底层 `VerifyAppStateObserverPermission` 会检查系统应用身份

> **API Review 待确认**：若 Review 决定改为 `@systemapi`，需在 `on`/`off` 函数和 `AppFocusStateObserver`/`FocusStateData` 类型声明上添加 `@systemapi` 标签。

## 5. 与现有 API 的关系

| 现有 API | 关系 | 说明 |
|----------|------|------|
| `on('applicationState', observer)` | 并行 | 前者监听应用**生命周期**变化（创建/前台/后台/销毁）；新增 API 监听应用**焦点**变化（获焦/失焦） |
| `on('appForegroundState', observer)` | 并行 | 前者监听应用**前台/后台切换**；新增 API 监听应用间**焦点切换**（同一应用内窗口切换不通知） |
| `on('abilityFirstFrameState', observer)` | 并行 | 前者监听 Ability **首帧渲染完成**；新增 API 监听应用**获焦**事件 |
| `ApplicationStateObserver` | 独立 | 不复用、不扩展 `ApplicationStateObserver`（spec 非目标："选定独立接口"） |

## 6. NAPI 分发模式（实现侧参考）

现有 `js_app_manager.cpp` 按字符串事件类型分发（`OnOn` at :270, `OnOff` at :404）：

```
新增常量:  ON_OFF_TYPE_APP_FOCUS_STATE = "appFocusState"  (js_app_manager.cpp:66 新增)
新增分支:  OnOn 中增加 if (type == ON_OFF_TYPE_APP_FOCUS_STATE) { return OnOnFocus(env, argc, argv); }
新增分支:  OnOff 中增加 if (type == ON_OFF_TYPE_APP_FOCUS_STATE) { return OnOffFocus(env, argc, argv); }
新增方法:  OnOnFocus / OnOffFocus (参照 OnOnAppForegroundState / OnOffAppForegroundState)
新增成员:  sptr<JSFocusStateObserver> observerFocus_ (参照 observerForeground_)
新增文件:  js_focus_state_observer.h / js_focus_state_observer.cpp (参照 js_app_foreground_state_observer)
```

## 7. 验证对照表

| Spec AC | SDK 接口覆盖 | 验证方式 |
|---------|-------------|----------|
| AC-1: 持权限注册成功 | `on('appFocusState', observer): void` 声明 + 权限注解 | JS API 单测（含权限校验 mock） |
| AC-2: 无权限返回错误码 | `@throws BusinessError 201` | JS API 单测 |
| AC-3: OnFocused 回调参数 | `FocusStateData` 接口含 `focusedUid`/`unfocusedUid`/`bundleName` | 类型编译验证 |
| AC-4: 跨应用切换通知 | `onFocusStateChanged(focusStateData)` 回调声明 | 端到端集成测试 |
| AC-5: 同应用不通知 | JSDoc: "Focus changes within the same application do not trigger this callback" | 集成测试 |
| AC-6: off 注销 | `off('appFocusState', observer?)` 声明 | JS API 单测 |
| AC-10: 不支持屏幕 | `@throws 16000050` + 条件编译 | 条件编译验证 |

## 8. 待确认项

| # | 问题 | 处理建议 |
|---|------|----------|
| 1 | API 版本号 `@since 27` 为推断值 | 需 API Review 确认实际引入版本 |
| 2 | 访问级别 Public vs `@systemapi` | spec 指定 Public，但底层权限校验含系统应用检查。若 Review 改为 `@systemapi`，需在 4 处声明添加标签 |
| 3 | `unfocusedUid = -1` 语义是否在 JSDoc 中明确 | 当前已在 `FocusStateData.unfocusedUid` 字段注释中说明 |
| 4 | 中英文翻译同步 | `zh-cn/` 目录需同步产出中文翻译文件 |
