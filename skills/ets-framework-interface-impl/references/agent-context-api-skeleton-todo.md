# Agent Context API Skeleton (TODO Variant)

## Purpose

Use this variant when you want a paste-ready scaffold with explicit implementation anchors.

All placeholders use the grep-friendly marker:

- `TODO(agent-context): ...`

## 1) ETS Class Template (TODO)

Suggested path:

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
    loadLibrary("agent_extension_context_ani_kit.z");
  }

  private static native nativeTransferStatic(input: ESValue, type: string): Object;
  private static native nativeTransferDynamic(input: Object): ESValue;
  private static contextType: string = 'AgentExtensionContext';

  private native nativeDoThing(param: string, callback: AsyncCallbackWrapper<void>): void;
  private native nativeDoThingCheck(param: string): void;

  doThing(param: string): Promise<void> {
    // TODO(agent-context): add preflight rules (empty string, permission precheck routing, etc.).
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

Note:
- `TODO(agent-context): use loadLibraryWithPermissionCheck(...)` only for namespace-kit style modules that require namespace verification on import.
- `TODO(agent-context): for context classes created/bound with ability lifecycle, prefer `loadLibrary(...)` unless an existing family explicitly requires permission-check loading.

## 2) ANI Transfer Module Template (TODO)

Suggested path:

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
    // TODO(agent-context): unwrap ESValue to weak_ptr<...> and validate concrete context type.
    // TODO(agent-context): return existing static binding object when present.
    // TODO(agent-context): otherwise create via ContextTransfer::GetInstance().GetStaticObject(...).
    return nullptr;
}

static ani_object NativeTransferDynamic(ani_env *env, ani_class cls, ani_object input)
{
    // TODO(agent-context): validate Object_InstanceOf for AgentExtensionContext class.
    // TODO(agent-context): resolve stage mode context and ConvertTo<AgentExtensionContext>.
    // TODO(agent-context): bridge napi object to esvalue via hybridgref path.
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
            // TODO(agent-context): convert Context -> AgentExtensionContext and call CreateEtsAgentExtensionContext.
            return nullptr;
        });

    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("AgentExtensionContext",
        [](napi_env n, std::shared_ptr<Context> c) -> napi_value {
            // TODO(agent-context): convert Context -> AgentExtensionContext and return dynamic binding object.
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

## 3) ANI Runtime Context Template (TODO)

Suggested path:

- `agent_runtime_framework/frameworks/ets/ani/agent_extension_context/src/ets_agent_extension_context.cpp`

```cpp
#include "ets_agent_extension_context.h"

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
    // TODO(agent-context): define strict parse/validation logic and message text.
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

    // TODO(agent-context): call actual native context API.
    int32_t ret = 0;

    // TODO(agent-context): if needed, map native error to JS error code family.
    err = AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, ret);
    AppExecFwk::AsyncCallback(env, callback, err, nullptr);
}

ani_object CreateEtsAgentExtensionContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context)
{
    // TODO(agent-context): find class "application.AgentExtensionContext.AgentExtensionContext".
    // TODO(agent-context): construct object and attach native pointer / nativeContext long.
    // TODO(agent-context): bind class native methods (DoThing/DoThingCheck + cleaner).
    // TODO(agent-context): initialize base/extension context fields.
    // TODO(agent-context): create and bind global reference if lifecycle requires stable binding object.
    return nullptr;
}
} // namespace AgentRuntime
} // namespace OHOS
```

## 4) BUILD Template (TODO)

Use as checklist, not literal code.

1. ANI runtime target:
- `TODO(agent-context): add/extend ohos_shared_library in agent_runtime_framework/frameworks/ets/ani/agent_extension_context/BUILD.gn`
- `TODO(agent-context): include new source files and deps`

2. ANI transfer target (if split model is used):
- `TODO(agent-context): add app-side transfer module target under frameworks/ets/ani/app/agent_extension_context`
- `TODO(agent-context): include target in frameworks/ets/ani/BUILD.gn package group`

3. ETS abc target:
- `TODO(agent-context): add generate_static_abc + ohos_prebuilt_etc for new ETS entry in frameworks/ets/ets/BUILD.gn`
- `TODO(agent-context): add etc target into ets package group`

4. Library name consistency:
- `TODO(agent-context): keep ETS loadLibrary*("... .z") consistent with ANI module output`

## 5) Grep-First Execution Plan

```powershell
# Find all TODO anchors in this template after copy.
rg -n "TODO\\(agent-context\\):" agent_runtime_framework/frameworks/ets

# Confirm signatures are wired in all layers.
rg -n "nativeDoThing|nativeDoThingCheck|DoThing\\(|DoThingCheck\\(" agent_runtime_framework/frameworks/ets

# Confirm transfer contract is complete.
rg -n "nativeTransferStatic|nativeTransferDynamic|contextType|RegisterStaticObjectCreator|RegisterDynamicObjectCreator" agent_runtime_framework/frameworks/ets
```
