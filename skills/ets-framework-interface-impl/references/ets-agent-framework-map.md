# ETS Agent Framework Map

## Scope

Use this map when implementing framework-side ETS interfaces in:

- `agent_runtime_framework/frameworks/ets/ets`
- `agent_runtime_framework/frameworks/ets/ani`
- `agent_runtime_framework/frameworks/ets/ets/BUILD.gn`
- `agent_runtime_framework/frameworks/ets/ani/*/BUILD.gn`

This map focuses on agent namespace and extension flows. For context-bound class APIs, also read:

- `references/ets-context-patterns.md`

## Existing Patterns To Reuse

### 0) Choose the correct binding model first

Two models exist in the codebase:

- Namespace kit model:
  - ETS namespace module exposes `export native function ...`
  - ANI uses `FindNamespace + Namespace_BindNativeFunctions`
  - Example: `@ohos.app.agent.agentManager`
- Context class model:
  - ETS class exposes `nativeTransferStatic/nativeTransferDynamic` + class-native methods
  - ANI uses `FindClass + Class_BindStaticNativeMethods`
  - Example pattern in base runtime: `application.UIAbilityContext`

Do not mix signatures or registration APIs between these models.

### 1) ETS API declaration and Promise wrapper

Primary example:
- `agent_runtime_framework/frameworks/ets/ets/@ohos.app.agent.agentManager.ets`

Pattern:
- Declare `native` methods inside exported namespace.
- Wrap with Promise API using `AsyncCallbackWrapper`.
- Dispatch native call through `taskpool.execute`.
- Load ANI library with `loadLibraryWithPermissionCheck`.

### 2) ETS extension entry class

Primary example:
- `agent_runtime_framework/frameworks/ets/ets/@ohos.app.agent.AgentExtensionAbility.ets`

Pattern:
- Expose lifecycle callbacks.
- Keep method names aligned with C++ `CallObjectMethod` invocations.

Current expected names used by ANI side:
- `onCreate`
- `onDestroy`
- `onData`
- `onAuth`
- `onConnect`
- `onDisconnect`

### 3) ETS interface + impl pair used by ANI

Primary examples:
- `agent_runtime_framework/frameworks/ets/ets/application/AgentProxy.ets`
- `agent_runtime_framework/frameworks/ets/ets/application/AgentHostProxy.ets`
- `agent_runtime_framework/frameworks/ets/ets/application/AgentExtensionConnectCallback.ets`

Pattern:
- Define interface for public typing.
- Define `*Impl` class with native fields/methods for ANI bridge.

### 4) ANI native registration

Primary example:
- `agent_runtime_framework/frameworks/ets/ani/agent_manager/src/ets_agent_manager.cpp`

Pattern:
- Locate namespace with `FindNamespace`.
- Register with `Namespace_BindNativeFunctions`.
- Implement `ANI_Constructor` and call registry init.

Critical consistency points:
- Namespace string constant must match ETS namespace path.
- Native function names and parameter order must match ETS `native` declarations.
- ANI signature string must match ETS types.

### 5) Native<->ETS object conversion

Primary example:
- `agent_runtime_framework/frameworks/ets/ani/agent_manager/src/ets_agent_manager_utils.cpp`

Pattern:
- `FindClass` for `*Impl` class.
- Construct object by `<ctor>`.
- Fill fields with `Object_SetPropertyByName_Ref`.
- Convert arrays via `std.core.Array` and `$_set`.

Common failure mode:
- mismatch between ETS field names and `Object_SetPropertyByName_Ref` keys.

### 6) Connection and callback lifecycle

Primary example:
- `agent_runtime_framework/frameworks/ets/ani/agent_extension_ability/src/connection/ets_agent_connection.cpp`

Pattern:
- Keep connection registry with max count policy.
- Store callback/proxy as global refs.
- Attach/detach ANI env on callback threads.
- Resolve/reject pending duplicate callbacks.
- Release global refs on disconnect and destruction.

### 7) Extension runtime bridge

Primary example:
- `agent_runtime_framework/frameworks/ets/ani/agent_extension_ability/src/ets_agent_extension.cpp`

Pattern:
- Load ETS module using `ETSRuntime`.
- Bind extension context into ETS object field `context`.
- Route native events to ETS methods via `CallObjectMethod`.

## Build Wiring Checklist

### ETS abc side

File:
- `agent_runtime_framework/frameworks/ets/ets/BUILD.gn`

For each new ETS file exported as framework module:
- Add `generate_static_abc("...")`.
- Add corresponding `ohos_prebuilt_etc("..._etc")`.
- Add `..._etc` target to `group("agent_runtime_ets_packages")`.

### ANI library side

Files:
- `agent_runtime_framework/frameworks/ets/ani/agent_manager/BUILD.gn`
- `agent_runtime_framework/frameworks/ets/ani/agent_extension_ability/BUILD.gn`
- `agent_runtime_framework/frameworks/ets/ani/agent_extension_context/BUILD.gn`
- `agent_runtime_framework/frameworks/ets/ani/agent_ui_extension_ability/BUILD.gn`

For new/updated bindings:
- Ensure source file is included in `sources`.
- Ensure include directories and deps include referenced headers/libs.
- Keep `ohos_shared_library` module name consistent with ETS `loadLibraryWithPermissionCheck` library name.

## Interface-to-Framework Implementation Template

When adding a new ETS API `foo`:

1. ETS declaration:
- Add `nativeFoo(...)` in ETS namespace file.
- Add public Promise wrapper `foo(...)`.

2. ANI registration:
- Implement `Foo(...)` C++ static function.
- Register `nativeFoo` in `kitFunctions`.
- Add/adjust ANI signature string.

3. Native behavior:
- Validate env and arguments.
- Apply permission checks if API is privileged.
- Call inner manager/client.
- Return via `AsyncCallback` with `EtsErrorUtil`.

4. Optional object mapping:
- Add converter methods in `ets_agent_manager_utils.cpp`-style helper.
- Ensure ETS `*Impl` class and property names exist.

5. Build:
- Ensure ANI source is in `sources`.
- Ensure ets abc target exists for entry module.

6. Sanity grep:
- Search for API name across ETS, ANI, and BUILD.gn and confirm all links exist.

## Quick Commands

```powershell
rg -n "nativeFoo|foo\\(" agent_runtime_framework/frameworks/ets
rg -n "Namespace_BindNativeFunctions|FindNamespace|ANI_Constructor" agent_runtime_framework/frameworks/ets/ani
rg -n "generate_static_abc|agent_runtime_ets_packages|ohos_shared_library" agent_runtime_framework/frameworks/ets
```
