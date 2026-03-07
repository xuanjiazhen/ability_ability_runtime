---
name: ets-framework-interface-impl
description: Implement framework-side code for ETS interfaces in OpenHarmony agent runtime. Use when adding or modifying ETS API surface under agent_runtime_framework/frameworks/ets, including ETS declarations, ANI native binding, object conversion, connection flow, and BUILD.gn wiring for @ohos.app.agent.* modules.
---

# ETS Framework Interface Implementation

Implement framework-side ETS interfaces by keeping ETS declarations, ANI bindings, native runtime behavior, and context binding lifecycle consistent.

## Follow This Workflow

1. Confirm the API contract and ownership boundary.
2. Update ETS declaration files under `agent_runtime_framework/frameworks/ets/ets`.
3. Update ANI binding layer under `agent_runtime_framework/frameworks/ets/ani`.
4. Wire BUILD.gn targets for abc generation and shared libraries.
5. If interface is context-bound, verify transfer and binding lifecycle.
6. Verify call path end-to-end from ETS to native and back.

## Use This Repository Pattern

- ETS namespace entrypoint pattern:
  - `ets/@ohos.app.agent.agentManager.ets`
  - uses `loadLibraryWithPermissionCheck("<lib>.z", "<namespace>")`
  - declares `export native function ...`
  - wraps native calls with `Promise` + `AsyncCallbackWrapper` + `taskpool.execute`
- Extension class pattern:
  - `ets/@ohos.app.agent.AgentExtensionAbility.ets`
  - define lifecycle methods `onCreate`, `onDestroy`, `onData`, `onAuth`, `onConnect`, `onDisconnect`
- Proxy and callback interfaces:
  - `ets/application/AgentProxy.ets`
  - `ets/application/AgentHostProxy.ets`
  - `ets/application/AgentExtensionConnectCallback.ets`
  - keep `nativeXxx` methods in `*Impl` classes for ANI mapping

## Implement ANI Bindings Correctly

- Add or update native function implementation in `ani/*/src/*.cpp`.
- Register native methods in namespace init function with exact ANI signatures.
- Keep namespace string aligned with ETS namespace. Example:
  - ETS: `@ohos.app.agent.agentManager` with nested `agentManager`
  - ANI: `@ohos.app.agent.agentManager.agentManager`
- Use existing error and callback utilities:
  - `EtsErrorUtil::CreateError*`
  - `AsyncCallback(...)`
- Preserve permission checks where needed (for this module, system-app checks exist in manager APIs).

## Handle Context-Bound Interface Pattern

Use this pattern when ETS APIs live on context classes (for example `UIAbilityContext`-style APIs), not only namespace kits:

- ETS side usually exposes:
  - `static nativeTransferStatic(...)`
  - `static nativeTransferDynamic(...)`
  - static `contextType` string
  - class-level native methods (not namespace-level)
- ANI side usually binds static class methods via:
  - `Class_BindStaticNativeMethods(...)`
  - registration in `ANI_Constructor`
- Runtime side must ensure context object binding is valid:
  - create context object during ability/extension startup
  - keep global/static binding object and dynamic binding object behavior consistent
  - support main-thread/sub-thread object access if the pattern requires it
- Conversion helpers typically rely on:
  - `ContextTransfer` creator registration
  - `GetStageModeContext(...)`
  - context-type-specific `CreateEts*Context(...)` creation

## Implement Object Conversion and Callback Flow

- For structured return objects, map native structs to ETS `*Impl` classes.
- Set ETS object properties by exact field names from ETS definitions.
- For async connection flows:
  - store global references for callbacks/proxies
  - resolve duplicate pending callbacks
  - release global references in cleanup paths
- Reuse existing connection helper patterns in:
  - `ani/agent_extension_ability/src/connection/ets_agent_connection.cpp`

## Update Build Wiring

- ETS abc output:
  - update `agent_runtime_framework/frameworks/ets/ets/BUILD.gn`
  - add `generate_static_abc` and matching `ohos_prebuilt_etc` for new ETS entry files
  - include target in `group("agent_runtime_ets_packages")`
- ANI shared library:
  - update specific `ani/*/BUILD.gn`
  - ensure include dirs and deps cover used headers/libs
  - keep module names consistent with `loadLibraryWithPermissionCheck`
- ANI package group:
  - if a new ANI library is introduced, include it in `frameworks/ets/ani/BUILD.gn`
  - if modeling after base ability runtime patterns, confirm corresponding `frameworks/ets/ani/app/*` kit grouping

## Keep Signatures and Names Strictly Consistent

- ETS `native` declaration parameter order must match C++ registration order.
- ANI signature string must match ETS types exactly.
- Class name strings used in ANI object creation must match ETS class paths exactly.
- Property names in `Object_SetPropertyByName_Ref` must match ETS field names exactly.

## Validate Changes

- Prefer targeted grep checks before build:
  - namespace string parity
  - method name parity across ETS and ANI
  - library name parity (`*.z`)
- Build and run the relevant module tests when available.
- If full build is not feasible, at least verify that new symbols are wired in all three places:
  - ETS declaration
  - ANI registration
  - BUILD.gn targets

## Read Reference

- For concrete file-by-file mapping and copy-safe implementation checklist, read:
  - `references/agent-context-api-skeleton-todo.md`
  - `references/agent-context-api-skeleton.md`
  - `references/ets-agent-framework-map.md`
  - `references/ets-context-patterns.md`
  - `references/ets-context-cross-comparison.md`
  - `references/ets-uiability-context-implementation-template.md`
