# UIAbilityContext Implementation Template

## Purpose

Use this template when implementing or extending ETS context class APIs with many overloads and callbacks, following:

- `frameworks/ets/ets/application/UIAbilityContext.ets`
- `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp`

This is the most complete in-tree pattern for:

- overload triads (`callback`, `callback+options`, `promise`)
- preflight `*Check` native methods
- async callback bridging from native runtime tasks
- connection lifecycle management

## Pattern 1: ETS API Surface Layout

For each feature API, keep a consistent shape in ETS:

1. Native declarations:
- operation methods (`nativeXxx(...)`)
- optional preflight checker (`nativeXxxCheck(...)`)

2. TS wrappers:
- callback overload
- callback with options overload
- Promise overload

3. Wrapper behavior:
- call `nativeXxxCheck(...)` first when validation/permission is required
- use `AsyncCallbackWrapper<T>`
- execute actual native call in `taskpool.execute(...)` when async

Examples in `UIAbilityContext.ets`:
- `startAbilityForResultWithAccount(...)` + `nativeStartAbilityForResultWithAccountCheck()`
- `openLink(...)` + `nativeOpenLinkCheck(...)`
- `openAtomicService(...)` + `nativeOpenAtomicServiceCheck()`
- UI service connect/disconnect with `nativeWantCheck(...)` and proxy check

## Pattern 2: ANI Registration for Overloads

Register all overloads explicitly with exact signatures.

In `ets_ability_context.cpp`:
- same method name appears multiple times with different signatures
- mapping is centralized in one `Class_BindNativeMethods` array

Rules:
- keep overload set complete; missing one silently breaks one ETS call path
- keep parameter order and type string exactly aligned with ETS declaration
- reuse signature constants for complex signatures to avoid drift

## Pattern 3: Preflight Check Methods

Preflight checks are first-class APIs, not optional helpers.

Use check methods to fail early for:
- permission gates (for example system-app only APIs)
- parameter type constraints
- object shape constraints (proxy validity)
- context availability

Failure style:
- throw immediately via `EtsErrorUtil::ThrowInvalidParamError(...)` or `ThrowError(...)`

Examples:
- `OpenLinkCheck`
- `OpenAtomicServiceCheck`
- `StartAbilityForResultWithAccountCheck`
- `DisconnectUIServiceExtensionCheck`

## Pattern 4: Async Callback Contract

For native async operations:

1. Build error/data pair consistently:
- success: `CreateError(...ERROR_OK)` + data/null
- failure: `CreateErrorByNativeErr(...)` + null/empty object

2. Use `AppExecFwk::AsyncCallback(...)` for delivery.

3. For deferred callbacks:
- persist callback with `GlobalReference_Create`
- use runtime task/lambda to call back later
- release reference after callback is sent

4. On task callback from other thread:
- attach ANI env as needed
- marshal results (`WrapAbilityResult`, etc.)
- handle conversion failure as inner error path

## Pattern 5: Connection Lifecycle Template

Connection APIs use a registry + id pattern:

1. Connect:
- parse want
- create connection object
- store ETS callback reference
- insert into global connection map and assign id
- call native connect
- on failure: callback error + remove registry entry

2. Disconnect:
- find by connection id/proxy-derived id
- remove from registry
- call native disconnect
- callback success/error

3. Duplicate connect handling:
- detect existing connection by want+callback identity
- if connection pending, queue duplicated callback
- if proxy exists, resolve duplicated callback immediately

Do not skip cleanup:
- remove registry entry on failure and on disconnect completion
- delete global refs in destructors or explicit cleanup methods

## Pattern 6: Error Mapping Rules

Use predictable mapping:

- parameter parse/type error -> `ThrowInvalidParamError` or `CreateInvalidParamError`
- missing/invalid context -> `ERROR_CODE_INVALID_CONTEXT`
- permission failure -> `ERROR_CODE_NOT_SYSTEM_APP`
- native runtime failure -> `CreateErrorByNativeErr` and JS error-code mapping helper when needed

Keep one API style per function:
- throw-style APIs should not also callback success/error in same branch
- callback-style APIs should avoid silent return on invalid input unless legacy behavior is required

## Pattern 7: Feature Checklist for a New Context API

1. ETS declarations
- Add `nativeXxx` and optional `nativeXxxCheck`
- Add overload wrappers and Promise variant

2. ANI bind
- Add static entry function(s)
- Add registration entries in `Class_BindNativeMethods` array
- Verify signatures for all overloads

3. Native implementation
- Parse/validate args
- Apply permission checks
- Resolve `context_.lock()` and fail consistently if null
- Call runtime API
- Return via throw or async callback contract

4. Lifecycle/resource safety
- Use global refs only when callback outlives stack
- release refs on completion/error/destruction

5. Build wiring
- Ensure relevant ANI module target sources and deps are updated
- Ensure loaded library name in ETS matches produced `.z` library target

## Focused Sanity Commands

```powershell
rg -n "native.*Check|Class_BindNativeMethods|AsyncCallback\\(|GlobalReference_Create|GlobalReference_Delete" frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp
rg -n "private native native|native.*Check|taskpool\\.execute|Promise<" frameworks/ets/ets/application/UIAbilityContext.ets
```
