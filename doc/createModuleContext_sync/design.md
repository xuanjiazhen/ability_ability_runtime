# REQ003 createModuleContext 同步接口 — 设计文档

> 版本: v1.0
> 日期: 2026-06-04
> 作者: @zexin_c

## 1. 需求概述

在 `@ohos.app.ability.application` 模块中新增同步的 `createModuleContext` 接口，用于在 ArkTS/ETS 侧直接、同步地获取指定模块的 `Context` 对象，无需等待 Promise 异步回调。

**接口方案**：提供两个重载：
1. 同应用内按 `moduleName` 创建模块上下文。
2. 跨应用按 `bundleName` + `moduleName` 创建模块上下文（需系统应用权限）。

**调用方式**：同步，直接返回 `Context` 对象。

**权限**：同应用内无额外权限；跨应用需系统应用权限及 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED`。

## 2. 架构设计

```
ArkTS 层 (@ohos.app.ability.application.ets)
    createModuleContextSync(context, moduleName) → Context
    createModuleContextSync(context, bundleName, moduleName) → Context
              │
              ▼
ANI 原生层 (ets_application.cpp)
    CreateModuleContextSync(env, contextObj, moduleName) → ani_object
              │
              ▼
框架层 (context_impl.cpp)
    ContextImpl::CreateModuleContext(moduleName, inputContextPtr) → shared_ptr<Context>
    ContextImpl::CreateModuleContext(bundleName, moduleName, inputContextPtr) → shared_ptr<Context>
```

## 3. ArkTS API 定义

在 `frameworks/ets/ets/@ohos.app.ability.application.ets` 中新增以下同步函数：

```ets
/**
 * @brief 同步创建当前应用内指定模块的上下文。
 *
 * @param context 输入的上下文对象。
 * @param moduleName 模块名称。
 * @returns 返回创建的模块上下文对象。
 * @throws { BusinessError } 参数校验失败或模块不存在时抛出。
 * @since TBD
 */
export function createModuleContextSync(context: Context, moduleName: string): Context;

/**
 * @brief 同步创建指定应用（bundle）内指定模块的上下文。
 *
 * @param context 输入的上下文对象。
 * @param bundleName 目标应用的 bundle 名称。
 * @param moduleName 模块名称。
 * @returns 返回创建的模块上下文对象。
 * @throws { BusinessError } 参数校验失败、模块不存在或无权限时抛出。
 * @since TBD
 */
export function createModuleContextSync(context: Context, bundleName: string, moduleName: string): Context;
```

## 4. ANI 原生接口定义

在 `frameworks/ets/ani/application/include/ets_application.h` 中新增：

```cpp
static ani_object CreateModuleContextSync(ani_env *env,
    ani_object contextObj, ani_string bundleName, ani_string moduleName);
```

在 `ApplicationInit` 中注册新的 native 函数：

```cpp
ani_native_function {
    "nativeCreateModuleContextSync",
    "C{application.Context.Context}C{std.core.String}C{std.core.String}"
    "C{application.Context.Context}",
    reinterpret_cast<void *>(EtsApplication::CreateModuleContextSync)
},
```

## 5. 客户端实现

### 5.1 ArkTS 封装层

在 `@ohos.app.ability.application.ets` 中，同步函数直接调用原生接口：

```ets
export function createModuleContextSync(context: Context, moduleName: string): Context {
    application.nativeCreateModuleContextCheck(context, moduleName);
    return application.nativeCreateModuleContextSync(context, "", moduleName);
}

export function createModuleContextSync(context: Context, bundleName: string, moduleName: string): Context {
    application.nativeCreateModuleContextCheck(context, moduleName, bundleName);
    return application.nativeCreateModuleContextSync(context, bundleName, moduleName);
}
```

### 5.2 ANI C++ 实现

在 `frameworks/ets/ani/application/src/ets_application.cpp` 中新增实现：

```cpp
ani_object EtsApplication::CreateModuleContextSync(ani_env *env,
    ani_object contextObj, ani_string bundleName, ani_string moduleName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateModuleContextSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    std::string stdBundleName = "";
    std::string stdModuleName = "";
    AppExecFwk::GetStdString(env, bundleName, stdBundleName);
    AppExecFwk::GetStdString(env, moduleName, stdModuleName);

    auto context = GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed.");
        return nullptr;
    }
    auto inputContextPtr = Context::ConvertTo<Context>(context);
    if (inputContextPtr == nullptr) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Convert to context failed.");
        return nullptr;
    }

    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    contextImpl->SetProcessName(context->GetProcessName());

    std::shared_ptr<Context> moduleContext = nullptr;
    if (stdBundleName.empty()) {
        moduleContext = contextImpl->CreateModuleContext(stdModuleName, inputContextPtr);
    } else {
        if (!CheckIsSystemAppOrPermisson(env, nullptr)) {
            TAG_LOGE(AAFwkTag::APPKIT, "CheckCaller failed");
            return nullptr;
        }
        moduleContext = contextImpl->CreateModuleContext(stdBundleName, stdModuleName, inputContextPtr);
    }

    if (!moduleContext) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Create module context failed.");
        return nullptr;
    }

    // 构造 ETS Context 对象并返回
    ani_class cls {};
    ani_status status = env->FindClass(CONTEXT_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if (!SetNativeContextLong(env, moduleContext, cls, contextObj)) {
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, moduleContext);
    return contextObj;
}
```

**关键点**：
- 直接复用现有的 `CreateModuleContextCheck` 进行参数校验（stageMode、null 检查等）。
- 直接复用 `ContextImpl::CreateModuleContext` 创建上下文，无 IPC，无异步。
- 同步返回 `ani_object`，无需 `AsyncCallback` 包装。
- 错误处理统一通过 `EtsErrorUtil::ThrowXxxError` 抛出异常，调用侧通过 `try-catch` 捕获。

## 6. 修改文件清单

| 层级 | 文件路径 | 操作 | 说明 |
|------|---------|------|------|
| ArkTS | `frameworks/ets/ets/@ohos.app.ability.application.ets` | 修改 | 新增 `createModuleContextSync` 两个重载 |
| ANI 头文件 | `frameworks/ets/ani/application/include/ets_application.h` | 修改 | 新增 `CreateModuleContextSync` 声明 |
| ANI 实现 | `frameworks/ets/ani/application/src/ets_application.cpp` | 修改 | 新增 `CreateModuleContextSync` 实现及 native 函数注册 |

## 7. 安全性分析

| 安全维度 | 设计考量 |
|---------|---------|
| 权限控制 | 跨应用（bundleName + moduleName）场景复用现有权限校验：`IsSystemAppCall()` + `VerifyGetBundleInfoPrivilegedPermission()` |
| 参数校验 | 复用现有 `CreateModuleContextCheck`，校验 context 非空、stageMode、moduleName 有效性 |
| 异常安全 | 所有失败路径均通过 `EtsErrorUtil::ThrowXxxError` 抛出异常，不会返回未初始化对象 |
| 线程安全 | `ContextImpl::CreateModuleContext` 本身无全局可变状态，同步调用线程安全 |

## 8. 使用示例

```ets
import application from '@ohos.app.ability.application';
import Context from 'application.Context';

function useModuleContext(baseContext: Context) {
    try {
        // 同应用内同步获取模块上下文
        let moduleCtx: Context = application.createModuleContextSync(baseContext, "feature_module");
        console.info("moduleCtx cacheDir: " + moduleCtx.cacheDir);

        // 跨应用同步获取模块上下文（需系统权限）
        let crossCtx: Context = application.createModuleContextSync(
            baseContext, "com.example.other", "library_module");
    } catch (err) {
        console.error("createModuleContextSync failed: " + JSON.stringify(err));
    }
}
```
