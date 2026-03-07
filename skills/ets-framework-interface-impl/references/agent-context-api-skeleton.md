# Agent Context API Skeleton

## Goal

Provide a minimal, copy-safe starter for adding context-bound agent ETS APIs:

- ETS class API declaration
- ANI transfer module registration
- ANI runtime context implementation entry
- BUILD wiring checklist

Use this as a scaffold, then adapt logic based on:

- `references/ets-context-patterns.md`
- `references/ets-context-cross-comparison.md`
- `references/ets-uiability-context-implementation-template.md`

## 1) ETS Class Skeleton

Suggested file location:

- `agent_runtime_framework/frameworks/ets/ets/application/AgentExtensionContext.ets`

```ts
import { BusinessError } from '@ohos.base';
import AsyncCallbackWrapper from '../utils/AgentUtils';

class Cleaner {
  constructor(private ptr: long) {}
  native clean(): void;
}

export default class AgentExtensionContext {
  static {
    loadLibraryWithPermissionCheck("agent_extension_context_ani_kit.z", "application.AgentExtensionContext");
  }

  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'AgentExtensionContext';

  private native nativeDoThing(param: string, callback: AsyncCallbackWrapper<void>): void;
  private native nativeDoThingCheck(param: string): void;

  doThing(param: string): Promise<void> {
    this.nativeDoThingCheck(param);
    return new Promise<void>((resolve, reject) => {
      const cb = new AsyncCallbackWrapper<void>((err: BusinessError | null) => {
        if (err == null || err.code == 0) {
          resolve(undefined);
        } else {
          reject(err);
        }
      });
      taskpool.execute((): void => {
        this.nativeDoThing(param, cb);
      });
    });
  }

  static transferStatic(input: Any): Object {
    let type = '';
    try {
      type = ESValue.wrap(input).getProperty('__context_impl__').getProperty('contextType').toString();
    } catch (_) {}
    return AgentExtensionContext.nativeTransferStatic(ESValue.wrap(input), type);
  }

  static transferDynamic(input: Object): Any {
    return AgentExtensionContext.nativeTransferDynamic(input).unwrap();
  }
}
```

## 2) ANI Transfer Module Skeleton

Suggested file location:

- `agent_runtime_framework/frameworks/ets/ani/app/agent_extension_context/src/ets_agent_extension_context_module.cpp`

```cpp
#include "ets_agent_extension_context_module.h"

#include "context_transfer.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *AGENT_EXT_CTX_CLASS = "application.AgentExtensionContext.AgentExtensionContext";
}

static ani_object NativeTransferStatic(ani_env *env, ani_object, ani_object input, ani_object type)
{
    // 1) unwrap ESValue -> weak_ptr<Context-derived>
    // 2) validate concrete context type
    // 3) return existing static binding if present
    // 4) otherwise ContextTransfer::GetInstance().GetStaticObject(...)
    return nullptr;
}

static ani_object NativeTransferDynamic(ani_env *env, ani_class cls, ani_object input)
{
    // 1) validate input instance
    // 2) GetStageModeContext + ConvertTo<...>
    // 3) ContextTransfer::GetInstance().GetDynamicObject(...)
    return nullptr;
}

static void ModuleInit(ani_env *env)
{
    ani_class cls = nullptr;
    if (env->FindClass(AGENT_EXT_CTX_CLASS, &cls) != ANI_OK || cls == nullptr) {
        return;
    }
    std::array funcs = {
        ani_native_function{ "nativeTransferStatic",
            "C{std.interop.ESValue}C{std.core.String}:C{std.core.Object}",
            reinterpret_cast<void *>(NativeTransferStatic) },
        ani_native_function{ "nativeTransferDynamic",
            "C{std.core.Object}:C{std.interop.ESValue}",
            reinterpret_cast<void *>(NativeTransferDynamic) },
    };
    if (env->Class_BindStaticNativeMethods(cls, funcs.data(), funcs.size()) != ANI_OK) {
        return;
    }

    ContextTransfer::GetInstance().RegisterStaticObjectCreator("AgentExtensionContext",
        [](ani_env *e, std::shared_ptr<Context> c) -> ani_object {
            // return CreateEtsAgentExtensionContext(...)
            return nullptr;
        });
    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("AgentExtensionContext",
        [](napi_env n, std::shared_ptr<Context> c) -> napi_value {
            // return GetOrCreateDynamicObject(...)
            return nullptr;
        });
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
        return ANI_NOT_FOUND;
    }
    ModuleInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
```

## 3) ANI Runtime Context Skeleton

Suggested file location:

- `agent_runtime_framework/frameworks/ets/ani/agent_extension_context/src/ets_agent_extension_context.cpp`

```cpp
#include "ets_agent_extension_context.h"

#include "ani_common_want.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AgentRuntime {

void EtsAgentExtensionContext::DoThing(ani_env *env, ani_object obj, ani_string param, ani_object callback)
{
    if (env == nullptr) {
        return;
    }
    auto etsCtx = GetEtsAgentExtensionContext(env, obj);
    if (etsCtx == nullptr) {
        return;
    }
    etsCtx->OnDoThing(env, param, callback);
}

void EtsAgentExtensionContext::DoThingCheck(ani_env *env, ani_object obj, ani_string param)
{
    if (env == nullptr || obj == nullptr) {
        return;
    }
    std::string s;
    if (!AppExecFwk::GetStdString(env, param, s) || s.empty()) {
        AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, "invalid param");
    }
}

void EtsAgentExtensionContext::OnDoThing(ani_env *env, ani_string param, ani_object callback)
{
    ani_object err = nullptr;
    auto ctx = context_.lock();
    if (ctx == nullptr) {
        err = AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, err, nullptr);
        return;
    }
    // Call native context operation.
    int32_t ret = 0;
    err = AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret);
    AppExecFwk::AsyncCallback(env, callback, err, nullptr);
}

ani_object CreateEtsAgentExtensionContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context)
{
    // 1) find ETS class
    // 2) construct object and attach native pointer
    // 3) bind native methods (DoThing, DoThingCheck, cleaner if needed)
    // 4) initialize base/extension context fields
    // 5) bind global context object if required by lifecycle
    return nullptr;
}
} // namespace AgentRuntime
} // namespace OHOS
```

## 4) BUILD Wiring Skeleton

ANI runtime library target:

- `agent_runtime_framework/frameworks/ets/ani/agent_extension_context/BUILD.gn`

ANI transfer kit target (if using app-side transfer module split):

- `agent_runtime_framework/frameworks/ets/ani/app/agent_extension_context/BUILD.gn`

ETS abc target:

- `agent_runtime_framework/frameworks/ets/ets/BUILD.gn`

Checklist:

1. Add/verify `ohos_shared_library("<module_name>")` for ANI side with source and deps.
2. Add/verify ANI module inclusion in package group (`frameworks/ets/ani/BUILD.gn` if needed).
3. Add/verify `generate_static_abc` + `ohos_prebuilt_etc` for ETS entry file.
4. Add module to ETS package group.
5. Ensure ETS `loadLibrary*("<module_name>.z", ...)` matches ANI module output name.

## 5) Minimal Validation Checklist

1. Signature parity:
- ETS declaration vs `Class_BindNativeMethods`/`Namespace_BindNativeFunctions` signatures.

2. Type and class path parity:
- class strings in ANI exactly match ETS class full names.

3. Error contract:
- invalid param and invalid context produce consistent `BusinessError` behavior.

4. Cleanup:
- every `GlobalReference_Create` has a corresponding release path.

5. Wiring:
- ETS file is in abc generation and package groups.
- ANI module is in build graph and loadable from ETS class.
