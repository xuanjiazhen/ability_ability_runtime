# ETS Context Cross Comparison

## Scope

Compare context-bound framework implementation patterns across:

- `UIAbilityContext`
- `UIExtensionContext`
- `ServiceExtensionContext`

Primary references:

- ETS:
  - `frameworks/ets/ets/application/UIAbilityContext.ets`
  - `frameworks/ets/ets/application/UIExtensionContext.ets`
  - `frameworks/ets/ets/application/ServiceExtensionContext.ets`
- ANI runtime/context implementation:
  - `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp`
  - `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp`
  - `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp`
- ANI transfer modules:
  - `frameworks/ets/ani/app/ability_context/src/ets_ability_context_module.cpp`
  - `frameworks/ets/ani/app/ui_extension_context/src/ets_ui_extension_context_module.cpp`
  - `frameworks/ets/ani/app/service_extension_context/src/ets_service_extension_context_module.cpp`

## Shared Baseline (All Three)

1. Transfer model
- all define `nativeTransferStatic`, `nativeTransferDynamic`, and `contextType`
- all use app-side module registration through `ContextTransfer` creators

2. Context ownership model
- ANI stores weak context references
- static and dynamic object binding are both supported
- main-thread/sub-thread behavior is handled in transfer modules

3. Async contract
- ETS uses `AsyncCallbackWrapper` + Promise wrappers
- ANI sends completion via `AppExecFwk::AsyncCallback(...)`
- native return codes map through `EtsErrorUtil`

4. Connection model
- service/UI connect-disconnect APIs keep connection registry maps by id
- callbacks are stored with global refs and cleaned on teardown

## Key Differences By Context

### UIAbilityContext

Strengths/pattern depth:
- richest overload matrix and checker APIs
- strongest sample for large API surfaces

Notable implementation traits:
- many preflight check natives (`native*Check`) exposed in ETS
- explicit system-app gate for account-sensitive APIs
- advanced UI service extension duplicate-connect handling
- completion-handler extraction from `StartOptions`
- dense `Class_BindNativeMethods` overload binding set

When to copy from here:
- building large context APIs with many overloads
- designing check-then-call patterns
- implementing robust connection/callback lifecycle logic

### UIExtensionContext

Strengths/pattern depth:
- close to UIAbilityContext for UI service extension features
- adds split-window APIs

Notable implementation traits:
- has `StartUIAbilitiesInSplitWindowMode` plus dedicated preflight check
- has `OpenLinkCheck` and `OpenAtomicServiceCheck`
- includes account-related permission checks in selected paths

When to copy from here:
- UI extension specific APIs (split-window, overlay/page interactions)
- UI service connection from extension context side

### ServiceExtensionContext

Strengths/pattern depth:
- leaner and service-centric
- includes mission and open-link/open-atomic service workflows

Notable implementation traits:
- includes `nativePreStartMission` and service-oriented API set
- has `OpenLinkCheck` but no dedicated `OpenAtomicServiceCheck` checker API
- still uses connection registry and account variants for service extension connect/start/stop
- keeps bind helper with `ANI_ALREADY_BINDED` tolerance for class/cleaner bindings

When to copy from here:
- service extension operations and mission-oriented APIs
- APIs that are less UI-visual and more service-flow oriented

## Library Loading Differences (ETS)

- `UIAbilityContext`: `loadLibraryWithPermissionCheck("ability_context_ani_kit.z", "...")`
- `ServiceExtensionContext`: `loadLibraryWithPermissionCheck("service_extension_context_ani_kit.z", "...")`
- `UIExtensionContext`: `loadLibrary("ui_extension_context_ani_kit.z")`

Guideline:
- follow existing class family convention unless security requirement explicitly changes
- keep `.z` library name aligned with corresponding ANI `ohos_shared_library` module

## Build/Module Mapping

- Transfer kit modules (`frameworks/ets/ani/app/*_context`) provide transfer/static-dynamic bridge.
- Runtime context modules (`frameworks/ets/ani/*_ability/src/ets_*_context.cpp`) provide operational native methods.
- Both layers must be kept consistent when adding or moving APIs.

## Practical Guardrails for New Agent Context APIs

1. Decide target family early
- choose closest source family:
  - `UIAbilityContext` for broad overload/check-heavy APIs
  - `UIExtensionContext` for UI extension behaviors
  - `ServiceExtensionContext` for service/missions/open-link style APIs

2. Keep transfer and runtime layers separate
- transfer module:
  - class static binding + creator registration
- runtime context module:
  - operational APIs + connection/error/permission logic

3. Prefer checker methods for risky inputs
- add explicit `native*Check` for permission-sensitive or shape-sensitive APIs
- call checker in ETS wrapper before async scheduling

4. Keep overload bundles complete
- if ETS adds callback/options/promise variants, ANI registration must include all corresponding signatures

5. Keep cleanup deterministic
- for every global ref creation path, document and implement its release path

## Quick Audit Commands

```powershell
rg -n "native.*Check|Class_BindNativeMethods|CheckCallerIsSystemApp|IsSystemAppByFullTokenID" frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp
rg -n "loadLibraryWithPermissionCheck|loadLibrary\\(" frameworks/ets/ets/application/UIAbilityContext.ets frameworks/ets/ets/application/UIExtensionContext.ets frameworks/ets/ets/application/ServiceExtensionContext.ets
rg -n "nativeTransferStatic|nativeTransferDynamic|contextType|RegisterStaticObjectCreator|RegisterDynamicObjectCreator" frameworks/ets/ets/application frameworks/ets/ani/app
```
