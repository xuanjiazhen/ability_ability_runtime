# 元能力接口错误信息增强 — ability_runtime 侧详细设计

> Feature: FEAT-30092574 / RR-30092574 / Proposal P1
> Scope: ability_runtime 仓库

## 1. 问题概述

startAbilityByType 异步路径异常场景统一返回 `"Internal error."`（JS error code 16000050），应用开发者无法定位根因。错误信息在四层被吞没：

| 层 | 位置 | 行为 |
|----|------|------|
| Window | `extension_session_manager.cpp:161` | `callback(ret)` 仅传 WSError 枚举，丢弃 AMS 真实 errorCode |
| ArkUI | `session_wrapper_impl.cpp:197-199` | 覆写为 `ERROR_CODE_UIEXTENSION_FOREGROUND_FAILED`(100018) + 固定字符串 |
| Ability | 5 个构造站点 `callback.onError` lambda | `OnError(arg)` 仅传 1 参，丢弃 str1(name)/str2(message) |
| Callback | `js_ui_extension_callback.cpp:212-215`、`ets_ui_extension_callback.cpp:63-66` | CallJsError/OnError 覆写 code=16000050、name/message 为固定值 |

## 2. 全量 startAbilityByType 对外 API 入口

排除 Cangjie 后共 **5 个独立的 `ModalUIExtensionCallbacks` 构造站点**：

| # | 构造站点 | 文件 | 覆盖的对外 API | 回调类型 |
|---|---------|------|--------------|---------|
| 1 | `AbilityContextImpl::StartAbilityByType` | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | JsAbilityContext(NAPI)、EtsAbilityContext(ANI)、InsightIntentDriver | `shared_ptr<UIExtensionCallback>` |
| 2 | `UIExtensionContext::StartAbilityByType` | `frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp` | JsUIExtensionContext(NAPI) | `shared_ptr<JsUIExtensionCallback>` |
| 3 | `UIServiceExtensionContext::StartAbilityByType` | `frameworks/native/appkit/ability_runtime/ui_service_extension_context.cpp` | JSUIServiceExtensionContext(NAPI)、EtsUIServiceExtensionContext(ANI) | `shared_ptr<UIExtensionCallback>` |
| 4 | `JsUIExtensionContentSession::OnStartAbilityByType` | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp` | JS UIExtensionContentSession（内联构造） | `shared_ptr<JsUIExtensionCallback>` |
| 5 | `EtsUIExtensionContentSession::StartAbilityByTypeSync` | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp` | ETS UIExtensionContentSession（内联构造） | `shared_ptr<EtsUIExtensionCallback>` |

ETS(ANI) 入口为同步返回 + `onError` 回调模式，无 Promise 需持有。`EtsUIExtensionCallback::OnAbilityErrorCode` 映射错误并调用 ETS `onError`。

## 3. UEC 侧已提供的接口

```cpp
// modal_ui_extension_config.h (arkui 侧已修改)
enum class UIExtensionOperationPhase {
    FOREGROUND,
};

struct ModalUIExtensionCallbacks {
    std::function<void(int32_t, const std::string&, const std::string&)> onError;
    std::function<void(int32_t, const UIExtensionOperationPhase&)> onAbilityErrorCode;  // 新增
};
```

- FOREGROUND（成功和失败）→ `onAbilityErrorCode(errorCode, FOREGROUND)` ← 增强路径
- BACKGROUND/DESTRUCTION 失败 → 原 `FireOnErrorCallback` → `callback.onError` ← 原路径

## 4. 同步路径：错误码映射（FR-1）

### 4.1 错误码来源分析

`startAbilityByType` → AMS 完整错误码路径：

```
App → AbilityContextImpl → [arkui] → [window] → AbilityManagerProxy::StartUIExtensionAbility
  │
  ├  Proxy: WriteParcelable 失败（Want 超 IPC 限制）
  │    → ERR_NATIVE_IPC_PARCEL_FAILED (已有映射: "IPC failed. Try again later.")
  │
  ├  Proxy: SendRequest 失败
  │    → DEAD_OBJECT (-32, ipc_types.h: DEAD_OBJECT = -EPIPE)
  │    → 其他 IPC 框架错误码（TRANSACTION_ERR=1 等，无法定界）
  │
  └  Stub → AbilityManagerService::StartUIExtensionAbility
       │
       ├  SetPickerElementNameAndParams: targetType 在 pickerMap 查不到
       │    → ERR_INVALID_EXTENSION_TYPE (新增, 2099423)
       │
       ├  extensionType == UNSPECIFIED (UIEXTENSION_TYPE_KEY 被设为无效字符串)
       │    → ERR_INVALID_VALUE (22, 无法定界, 保持 "Internal error.")
       │
       ├  权限/调用方/可见性/拦截器
       │    → CHECK_PERMISSION_FAILED / ERR_INVALID_CALLER / RESOLVE_ABILITY_ERR 等
       │    → 已在 INNER_TO_JS_ERROR_CODE_MAP 中映射
       │
       ├  INNER_ERR (通用内部错误, 2097167)
       │    → 无法定界, 保持 "Internal error."
       │
       └  ERR_INVALID_VALUE (22, 多种参数校验, 非 type 专用)
            → 无法定界, 保持 "Internal error."
```

### 4.2 IPC 错误码

`SendRequest` 返回 C++ IPC 框架（`ipc_types.h`）的错误码。仅 `DEAD_OBJECT`(-32) 可明确定界：

```cpp
static constexpr int32_t IPC_ERR_DEAD_OBJECT = -32;
// 对应 ipc_types.h: DEAD_OBJECT = -EPIPE = -32
```

### 4.3 type 参数无效专用错误码

`startAbilityByType` 传入的 `targetType` 经 `SetPickerElementNameAndParams` 在 `pickerMap` 中查找。找不到时原代码仅 `return;`（void），不设 `UIEXTENSION_TYPE_KEY`，后续默认为 `UI` 类型继续执行——错误被吞没。

`ERR_INVALID_VALUE`(22) 被 AMS 用于多种错误，无法定界。新增专用错误码并修改返回点：

**`ability_manager_errors.h`**：
```cpp
ERR_INVALID_EXTENSION_TYPE = 2099423,  // StartUIExtensionAbility: invalid extension ability type
```

**`ability_manager_service.h`**：`SetPickerElementNameAndParams` 返回值 `void` → `ErrCode`

**`ability_manager_service.cpp` — `SetPickerElementNameAndParams` pickerMap 查找失败处**：
```cpp
auto it = pickerMap.find(targetType);
if (it == pickerMap.end()) {
    TAG_LOGE(AAFwkTag::ABILITYMGR, "can not find targetType:%{public}s", targetType.c_str());
    return ERR_INVALID_EXTENSION_TYPE;  // 原: return; (void, 不报错)
}
```

**`ability_manager_service.cpp` — `StartUIExtensionAbility` 调用处**：
```cpp
auto pickerRet = SetPickerElementNameAndParams(extensionSessionInfo, userId);
if (pickerRet != ERR_OK) {
    return pickerRet;  // ERR_INVALID_EXTENSION_TYPE 透传
}
```

> 注：line 5053 的 `extensionType == UNSPECIFIED` 检查保持 `ERR_INVALID_VALUE`——这是 `UIEXTENSION_TYPE_KEY` 被设为非空无效字符串的场景，与 `startAbilityByType` 的 `targetType` 在 `pickerMap` 查找不到是不同路径。

### 4.4 新增映射条目

```cpp
// ability_business_error.cpp — INNER_ERROR_MSG_BY_NATIVE_CODE 新增
{IPC_ERR_DEAD_OBJECT,          "Internal error. IPC peer process dead."},
{ERR_INVALID_EXTENSION_TYPE,   "Internal error. Invalid extension ability type."},
```

### 4.5 错误码映射完整表

| 错误码 | 值 | 来源 | 映射路径 | 最终消息 |
|--------|---|------|---------|---------|
| `DEAD_OBJECT` | -32 | `ipc_types.h` (SendRequest) | `INNER_ERROR_MSG_BY_NATIVE_CODE` | "Internal error. IPC peer process dead." |
| `ERR_INVALID_EXTENSION_TYPE` | 2099423 | `ability_manager_errors.h` (新增, AMS type 校验) | `INNER_ERROR_MSG_BY_NATIVE_CODE` | "Internal error. Invalid extension ability type." |
| `ERR_NATIVE_IPC_PARCEL_FAILED` | — | Proxy WriteParcelable 失败 | `INNER_ERROR_MSG_BY_NATIVE_CODE` (已有) | "Internal error. IPC failed. Try again later." |
| `CHECK_PERMISSION_FAILED` | — | AMS 权限校验 | `INNER_TO_JS_ERROR_CODE_MAP` → 201 | "The application does not have permission..." |
| `ERR_INVALID_CALLER` | — | AMS 调用方校验 | → 16200001 | "The specified process does not have the permission." |
| `RESOLVE_ABILITY_ERR` | 2097152 | AMS ability 解析 | → 16000001 | "The specified ability does not exist." |
| `ERR_CROSS_USER` | — | AMS 多用户 | → 16000006 | "Cross-user operations are not allowed." |
| 拦截器码 | — | AMS 拦截器 | → 各种 | 各种具体消息 |
| `INNER_ERR` | 2097167 | AMS 通用内部错误 | 不在任一映射表 | "Internal error."（无法定界） |
| `ERR_INVALID_VALUE` | 22 | AMS 多种参数校验 | 不在任一映射表 | "Internal error."（无法定界） |

> `ERR_FREQ_START_ABILITY` 已有映射（行628），但 `StartUIExtensionAbility` 路径不返回此码（无频率限制）。

## 5. 回调基础设施

### 5.1 UIExtensionCallback 基类（ui_extension_callback.h）

新增两个虚函数（默认空实现），不修改已有 `OnError(int32_t)` 签名：

```cpp
virtual void OnAbilityErrorCode(int32_t errorCode) {}
virtual void ResolveDeferred() {}
```

- `OnAbilityErrorCode`：接收原始 native 错误码，子类内部完成 native→JS error code + msg 映射
- `ResolveDeferred`：NAPI Promise 持有模式下 resolve deferred，ETS 默认 no-op

### 5.2 JsUIExtensionCallback（js_ui_extension_callback.h/cpp）

新增成员和方法：
```cpp
napi_deferred deferred_ = nullptr;
void SetDeferred(napi_deferred deferred) { deferred_ = deferred; }
void RejectDeferred(int32_t jsErrCode, const std::string& innerMsg);
void ResolveDeferred() override;
void OnAbilityErrorCode(int32_t errorCode) override;
```

**`OnAbilityErrorCode` 实现**：
1. `GetJsErrorCodeByNativeError(errorCode)` → JS 错误码（如 16000050）
2. `GetErrorMsgByNativeError(errorCode)` → 具体消息
3. 异步调度 `RejectDeferred(jsErrCode, innerMsg)` 到 JS 线程（经 `NapiAsyncTask::Schedule`）
4. `CloseModalUIExtension()`

**`RejectDeferred` 实现**：`if (deferred_ != nullptr) { napi_reject_deferred(...); deferred_ = nullptr; }`

**`ResolveDeferred` 实现**：经 `NapiAsyncTask::Schedule` 调度到 JS 线程，双重检查 `deferred_ != nullptr` 后 `napi_resolve_deferred`

**`CallJsError` 改造**：
- 签名扩展为 `(int32_t number, const std::string& name = "", const std::string& message = "")`
- 新增 `number == 0` 守卫
- `finalMessage` 非空时优先透传，空时回退固定值（向后兼容 1 参路径）
- 末尾增加 `RejectDeferred(number, finalMessage)` — BACKGROUND/DESTRUCTION 错误同样 reject Promise

### 5.3 EtsUIExtensionCallback（ets_ui_extension_callback.h/cpp）

override `OnAbilityErrorCode`：内部完成映射，直接调用 ETS `onError(jsErrCode, name, msg)` + `CloseModalUIExtension()`。`ResolveDeferred` 使用基类默认 no-op（ETS 无 Promise）。

## 6. onAbilityErrorCode 注册（5 站点统一模式）

每个站点新增 `shared_ptr<atomic<bool>> errorFired`，**仅**由 `onAbilityErrorCode` 消费：

```cpp
auto errorFired = std::make_shared<std::atomic<bool>>(false);

callback.onError = [uiExtensionCallback](int32_t arg, const std::string &str1,
    const std::string &str2) {
    uiExtensionCallback->OnError(arg);
};

callback.onAbilityErrorCode = [uiExtensionCallback, errorFired](int32_t errorCode,
    const Ace::UIExtensionOperationPhase& phase) {
    if (phase != Ace::UIExtensionOperationPhase::FOREGROUND) {
        return;
    }
    bool expected = false;
    if (!errorFired->compare_exchange_strong(expected, true)) {
        return;
    }
    if (errorCode == 0) {
        uiExtensionCallback->ResolveDeferred();
    } else {
        uiExtensionCallback->OnAbilityErrorCode(errorCode);
    }
};
```

### 去重机制

| 路径 | errorFired | 说明 |
|------|------------|------|
| `onAbilityErrorCode(0)` 成功 | ✅ 消费 | 首次 FOREGROUND 结果 → `ResolveDeferred` |
| `onAbilityErrorCode(≠0)` 失败 | ✅ 消费 | 首次 FOREGROUND 结果 → `OnAbilityErrorCode` → `RejectDeferred` |
| `callback.onError` 运行时错误 | ❌ 不检查 | 始终触发 → `CallJsError` → JS onError + `RejectDeferred`(no-op if settled) |

两层守卫：
1. `errorFired` CAS — `onAbilityErrorCode` 仅首次 FOREGROUND 结果为准
2. `deferred_ != nullptr` — `ResolveDeferred`/`RejectDeferred` 一次性，Promise settle 后后续 no-op

## 7. NAPI 入口 Promise 持有

4 个 NAPI 入口改为 Promise 持有模式：

1. `napi_create_promise` 手动创建 Promise，`deferred` 存入 `callback->SetDeferred()`
2. `NapiAsyncTask(deferred, execute, complete)` 调度 worker 线程执行 `StartAbilityByType`
3. `complete` 回调：`innerErrCode == ERR_OK` → **不 resolve**（持有 deferred）；`!= ERR_OK` → `task.Reject`
4. 异步结果通过 `onAbilityErrorCode` resolve/reject deferred

利用 `NapiAsyncTask::~NapiAsyncTask()` 不 reject deferred 的特性（仅删除 async_work），deferred 在 `complete` 不 resolve 时安全持有。

`lastParam != nullptr`（callback 模式）保持原行为（立即 resolve/reject）。

| NAPI 入口 | 文件 | 特点 |
|-----------|------|------|
| `JsAbilityContext::OnStartAbilityByType` | `js_ability_context.cpp` | `lastParam` 分支：callback 模式立即 resolve，Promise 模式持有 |
| `JsUIExtensionContext::OnStartAbilityByType` | `js_ui_extension_context.cpp` | 同上 |
| `JSUIServiceExtensionContext::OnStartAbilityByType` | `js_ui_service_extension_context.cpp` | 始终 Promise 持有（原 `lastParam` 固定 nullptr） |
| `JsUIExtensionContentSession::OnStartAbilityByType` | `js_ui_extension_content_session.cpp` | `holdDeferred` 标志，无 execute，complete 内联构造 callback |

## 8. 数据流

### 8.1 异步激活失败

```
[window] callback(WSError, errorCode=-32)
[arkui]  HandleExtensionError → onAbilityErrorCode(-32, FOREGROUND)
[ability] phase==FOREGROUND → errorFired CAS → 首次放行
  └─ OnAbilityErrorCode(-32)
       jsErrCode = GetJsErrorCodeByNativeError(-32) = 16000050
       innerMsg = GetErrorMsgByNativeError(-32) = "Internal error. IPC peer process dead."
       └─ RejectDeferred(16000050, "Internal error. IPC peer process dead.")
            → napi_reject_deferred → Promise reject
            → 开发者: await → catch(error) → error.code=16000050, error.message="IPC peer process dead."
```

### 8.2 type 参数无效

```
[AMS] extensionType == UNSPECIFIED → return ERR_INVALID_EXTENSION_TYPE (2099423)
[window] callback(WSError, 2099423)
[arkui]  onAbilityErrorCode(2099423, FOREGROUND)
[ability] OnAbilityErrorCode(2099423)
       jsErrCode = GetJsErrorCodeByNativeError(2099423) = 16000050
       innerMsg = GetErrorMsgByNativeError(2099423) = "Internal error. Invalid extension ability type."
       └─ RejectDeferred(16000050, "Internal error. Invalid extension ability type.")
```

### 8.3 同步失败

```
[execute] StartAbilityByType → ERR_INVALID_VALUE
[complete] innerErrCode != ERR_OK → task.Reject → Promise reject
```

### 8.4 成功

```
[window] callback(WS_OK, errorCode=0)
[arkui]  HandleExtensionError → onAbilityErrorCode(0, FOREGROUND)
[ability] phase==FOREGROUND → errorFired CAS → 首次放行
  └─ errorCode == 0 → ResolveDeferred()
       → napi_resolve_deferred → Promise resolve
       → 开发者: await → resolve(undefined)
```

### 8.5 成功后运行时错误

```
[ability] callback.onError(100020, ...) → 不检查 errorFired → OnError(100020)
  └─ CallJsError(100020) → JS onError 回调
  └─ RejectDeferred → deferred_==nullptr → no-op（Promise 已 settle）
```

## 9. ModalUIExtensionCallbacks 使用方影响分析

### 9.1 已修改站点（5 个 startAbilityByType）

均注册 `onAbilityErrorCode`，使用 `UIExtensionCallback`（或子类 `JsUIExtensionCallback`/`EtsUIExtensionCallback`）。

### 9.2 未修改站点（无影响）

| 文件 | 函数 | 回调类 | 继承 `UIExtensionCallback`? | 影响 |
|------|------|--------|---------------------------|------|
| `ability_context_impl.cpp:1334` | `CreateModalUIExtensionWithApp` | `DialogUIExtensionCallback` | ❌ 独立类 | 无 |
| `ui_extension_context.cpp:1060` | `SetupModalCallbacks` | `UIExtensionModalCallback` | ❌ 独立类 | 无 |
| `auto_fill_manager.cpp:162` | `BindModalUIExtensionCallback` | `AutoFillExtensionCallback` | ❌ 独立类 | 无 |
| `start_vertical_panel.cpp:109` | `StartVerticalPanel` | `PanelStartCallback` | ❌ 独立类 | 无 |
| `cj_*.cpp` | (CJ) | `CjUIExtensionCallback` | ❌ 独立类 | 无（排除） |

`onAbilityErrorCode` 为 `nullptr` → arkui `HandleExtensionError` 走原 `FireOnErrorCallback` 路径，行为不变。`UIExtensionCallback` 基类虚函数新增仅影响 `JsUIExtensionCallback` 和 `EtsUIExtensionCallback` 两个子类。

## 10. 修改文件清单

| # | 文件 | 修改内容 |
|---|------|---------|
| 1 | `ability_business_error.cpp` | +`IPC_ERR_DEAD_OBJECT` 局部 constexpr, +2 条映射 |
| 2 | `ability_manager_errors.h` | +`ERR_INVALID_EXTENSION_TYPE = 2099423` |
| 3 | `ability_manager_service.h` | `SetPickerElementNameAndParams` 返回值 `void` → `ErrCode` |
| 4 | `ability_manager_service.cpp` | `SetPickerElementNameAndParams` pickerMap 失败返回 `ERR_INVALID_EXTENSION_TYPE`；调用处检查返回值；`UNSPECIFIED` 检查保持 `ERR_INVALID_VALUE` |
| 4 | `ui_extension_callback.h` | +`OnAbilityErrorCode` +`ResolveDeferred` 虚函数 |
| 5 | `js_ui_extension_callback.h` | +`deferred_` 成员, +`SetDeferred`/`RejectDeferred`/`ResolveDeferred`/`OnAbilityErrorCode`, CallJsError 扩展签名 |
| 6 | `js_ui_extension_callback.cpp` | +`RejectDeferred`/`ResolveDeferred`/`OnAbilityErrorCode` 实现, CallJsError 改造 |
| 7 | `ets_ui_extension_callback.h` | +`OnAbilityErrorCode` override |
| 8 | `ets_ui_extension_callback.cpp` | +`OnAbilityErrorCode` 实现 |
| 9 | `ability_context_impl.cpp` | +onAbilityErrorCode +errorFired (站点#1) |
| 10 | `ui_extension_context.cpp` | +onAbilityErrorCode +errorFired (站点#2) |
| 11 | `ui_service_extension_context.cpp` | +onAbilityErrorCode +errorFired (站点#3) |
| 12 | `js_ui_extension_content_session.cpp` | +onAbilityErrorCode +errorFired +Promise 持有 (站点#4) |
| 13 | `ets_ui_extension_content_session.cpp` | +onAbilityErrorCode +errorFired (站点#5) |
| 14 | `js_ability_context.cpp` | Promise 持有 (NAPI 入口#1) |
| 15 | `js_ui_extension_context.cpp` | Promise 持有 (NAPI 入口#2) |
| 16 | `js_ui_service_extension_context.cpp` | Promise 持有 (NAPI 入口#3) |
