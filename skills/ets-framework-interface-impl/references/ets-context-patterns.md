# ETS Context-Bound Patterns

## Why This Matters

Some ETS interfaces are not namespace kits. They are methods on context classes and require:

- class-level native binding
- context object transfer between static/dynamic forms
- runtime binding lifecycle (ability/extension startup and teardown)

For side-by-side differences across major context families, read:

- `references/ets-context-cross-comparison.md`

This pattern appears in `frameworks/ets` and should guide agent-side context interface work.

## Key Reference Paths

### ETS declarations

- `frameworks/ets/ets/application/Context.ets`
- `frameworks/ets/ets/application/UIAbilityContext.ets`
- `frameworks/ets/ets/application/UIExtensionContext.ets`

Typical traits:
- `loadLibrary` or `loadLibraryWithPermissionCheck` in class static block
- static:
  - `nativeTransferStatic(input: ESValue, type: string)`
  - `nativeTransferDynamic(input: Object)`
  - `contextType`
- many class-native methods bound directly to context instance

### ANI transfer modules

- `frameworks/ets/ani/app/context/src/ets_context_module.cpp`
- `frameworks/ets/ani/app/ability_context/src/ets_ability_context_module.cpp`
- `frameworks/ets/ani/app/ui_extension_context/src/ets_ui_extension_context_module.cpp`

Typical traits:
- `ANI_Constructor` calls module init
- `FindClass("application.<X>Context.<X>Context")`
- `Class_BindStaticNativeMethods` for `nativeTransferStatic/nativeTransferDynamic`
- register creators with `ContextTransfer::GetInstance()`

### Runtime-side binding

- `frameworks/ets/ani/ui_ability/src/ets_ui_ability.cpp`

Typical traits:
- create ETS context object during runtime init (`CreateEtsAbilityContext`)
- bind global ref to runtime context
- register configuration update callback to propagate config changes
- release refs and unbind on destruction

### Build wiring

- `frameworks/ets/ani/app/ability_context/BUILD.gn`
- `frameworks/ets/ani/BUILD.gn` (kit group)
- `frameworks/ets/ets/BUILD.gn` (abc + etc package targets)

## Reusable Checklist For Context APIs

1. ETS class contract
- Add/maintain static `contextType`.
- Keep transfer static methods present.
- Add native method declarations with clear async pattern wrappers.

2. ANI class-binding module
- Ensure class name constant matches ETS fully qualified class name.
- Bind static transfer functions with `Class_BindStaticNativeMethods`.
- Register `ContextTransfer` static/dynamic creators.

3. Runtime binding lifecycle
- Ensure context instance is created and bound when runtime starts.
- Ensure binding object is reused when expected and recreated when required.
- Ensure main-thread/sub-thread handling is explicitly safe.
- Ensure unbind/global-ref cleanup on teardown.

4. Type safety and conversion
- Validate input instance type (`Object_InstanceOf` where applicable).
- Convert by concrete context type (`Context::ConvertTo<...>`).
- Throw transfer errors consistently when conversion fails.

5. Build consistency
- ANI shared library module name must match ETS load library name (`*.z`).
- Include ANI module target in package group.
- Include ETS abc target and `ohos_prebuilt_etc` target in ETS package group.

## How To Apply This To Agent Runtime

When adding agent context-bound interfaces (for example future `AgentExtensionContext` APIs):

- follow `agent_runtime_framework/frameworks/ets/ani/agent_extension_context` for concrete agent context object creation
- copy transfer/binding lifecycle ideas from base `frameworks/ets/ani/app/*context*` modules
- keep agent-specific permission and service routing logic in agent manager/connection layers
- avoid mixing namespace-kit assumptions into class-bound context APIs
- for overload-heavy context APIs, apply `references/ets-uiability-context-implementation-template.md`

## Quick Verification Commands

```powershell
rg -n "contextType|nativeTransferStatic|nativeTransferDynamic|Class_BindStaticNativeMethods" frameworks/ets/ets frameworks/ets/ani
rg -n "ContextTransfer::GetInstance|RegisterStaticObjectCreator|RegisterDynamicObjectCreator" frameworks/ets/ani
rg -n "CreateEts.*Context|Bind\\(|Unbind\\(|GlobalReference_Create|GlobalReference_Delete" frameworks/ets/ani
```
