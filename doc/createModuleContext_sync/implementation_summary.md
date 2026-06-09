# REQ003 createModuleContextSync — 实现总结

> 日期: 2026-06-09
> 基于: design.md v1.0 / spec.md v1.0 / proposal.md v1.0

## 变更概览

共修改 **5 个文件**，均为新增同步接口相关代码，不涉及已有异步接口行为变更。

| # | 层级 | 文件路径 | 变更说明 |
|---|------|---------|---------|
| 1 | JS/NAPI 头文件 | `frameworks/js/napi/application/js_application.h` | +2 方法声明（`CreateModuleContextSync` / `OnCreateModuleContextSync`） |
| 2 | JS/NAPI 实现 | `frameworks/js/napi/application/js_application.cpp` | +1 方法实现（`OnCreateModuleContextSync`），+1 `BindNativeFunction` 注册 |
| 3 | ANI 头文件 | `frameworks/ets/ani/application/include/ets_application.h` | +1 方法声明（`CreateModuleContextSync`） |
| 4 | ANI 实现 | `frameworks/ets/ani/application/src/ets_application.cpp` | +1 方法实现（`CreateModuleContextSync`），+1 `ani_native_function` 注册 |
| 5 | ArkTS API | `frameworks/ets/ets/@ohos.app.ability.application.ets` | +1 `nativeCreateModuleContextSync` 声明，+2 `createModuleContextSync` 重载实现 |

---

## 实现要点

### JS/NAPI 层

- **CreateModuleContextSync** / **OnCreateModuleContextSync**：
  - 复用与异步版本相同的参数校验逻辑（`IsStageContext`、`GetStageModeContext`、`ConvertTo<Context>`）
  - 直接调用 `ContextImpl::CreateModuleContext` 同步创建模块上下文，无 `NapiAsyncTask` 异步包装
  - 调用已有的 `CreateJsContext` 辅助函数将 C++ `Context` 对象转换为 JS `Context` 对象后直接返回
  - 跨应用场景内联系统应用权限校验（`CheckCallerIsSystemApp` + `CheckCallerPermission`），失败时通过 `ThrowNotSystemAppError` / `ThrowNoPermissionError` 抛异常
  - 构造失败时通过 `ThrowInvalidParamError` 抛异常，不返回 null

### ANI 层

- **CreateModuleContextSync**：
  - 复用 `IsStageContext`、`GetStageModeContext`、`ConvertTo<Context>` 进行参数校验
  - 直接调用 `ContextImpl::CreateModuleContext` 同步创建模块上下文
  - 使用 `SetNativeContextLong` + `ContextUtil::CreateEtsBaseContext` 构造 ETS `Context` 对象后直接返回 `ani_object`
  - 跨应用场景内联权限校验（`IsSystemAppCall` + `VerifyGetBundleInfoPrivilegedPermission`），失败时通过 `EtsErrorUtil::ThrowNotSystemAppError` / `ThrowNoPermissionError` 抛异常
  - 构造失败时通过 `EtsErrorUtil::ThrowInvalidParamError` 抛异常，不返回 null

### ArkTS API 层

- **`createModuleContextSync(context, moduleName)`**：同应用内同步获取模块上下文，先调用 `nativeCreateModuleContextCheck` 校验，再调用 `nativeCreateModuleContextSync`
- **`createModuleContextSync(context, bundleName, moduleName)`**：跨应用同步获取模块上下文，先调用 `nativeCreateModuleContextCheck` 校验（含权限预校验），再调用 `nativeCreateModuleContextSync`
- `nativeCreateModuleContextSync` 返回 `Context` 对象，而非 `Promise<Context>`

---

## AC 覆盖映射

| AC | 实现位置 |
|----|---------|
| AC-1.1~1.2 | `js_application.cpp` `OnCreateModuleContextSync` / `ets_application.cpp` `CreateModuleContextSync`（同应用内直接调用 `ContextImpl::CreateModuleContext`） |
| AC-2.1~2.3 | `js_application.cpp` `OnCreateModuleContextSync` / `ets_application.cpp` `CreateModuleContextSync`（跨应用内联 `IsSystemAppCall` + `VerifyGetBundleInfoPrivilegedPermission` 校验） |
| AC-3.1~3.3 | `js_application.cpp` / `ets_application.cpp`（`context` null / 非 stageMode / `moduleName` 非法 统一抛异常） |
| AC-4.1 | `@ohos.app.ability.application.ets` API 参考中需补充 "该同步接口有一定耗时" 提示（文档侧） |
| AC-4.2 | `@ohos.app.ability.application.d.ts` 中补充 `createModuleContextSync` 两个重载签名（文档侧） |

---

## 接口变更明细

### 新增 ArkTS 接口

```ets
// 同应用内同步获取模块上下文
export function createModuleContextSync(context: Context, moduleName: string): Context;

// 跨应用同步获取模块上下文（需系统权限）
export function createModuleContextSync(context: Context, bundleName: string, moduleName: string): Context;
```

### 新增 ANI Native 接口

```cpp
// 框架层注册
ani_native_function {
    "nativeCreateModuleContextSync",
    "C{application.Context.Context}C{std.core.String}C{std.core.String}"
    ":C{application.Context.Context}",
    reinterpret_cast<void *>(EtsApplication::CreateModuleContextSync)
};
```

### 新增 JS Native 接口

```cpp
// ApplicationInit 中注册
BindNativeFunction(env, exportObj, "createModuleContextSync", moduleName,
    JsApplication::CreateModuleContextSync);
```

---

## 兼容性声明

- **已有 API 行为变更:** 否。新增独立同步接口，不影响 `createModuleContext` 异步接口的任何行为
- **编译宏保护:** 无新增编译宏，所有平台均可用
- **ABI 兼容性:** 新增 JS/ANI native 函数注册，不影响已有 ABI
- **数据存储格式变更:** 否
- **配置文件格式变更:** 否
