# O-4 跨仓交付件 — appMemory SDK 声明

> 目标仓：`interface/sdk-js`（与 `interface/sdk` 的 d.ts 生成工具链）。
> 本仓（ability_runtime）仅提供 C++ 绑定半边；本目录给出 SDK 侧需落位的声明与绑定契约。

## 1. 公共 API 声明

- `handoff/appMemory.d.ts` —— 公共声明（`@ohos.app.ability.appMemory`），两处落位：
  - `api/@ohos.app.ability.appMemory.d.ts`（ets）
  - `api/@ohos.app.ability.appMemory.d.ts`（js，内容一致）
- `@since` 版本号由 SDK 团队按发版节奏分配（草案中标记为 `{{TBD}}`）。
- `@kit AbilityKit`，`@syscap SystemCapability.Ability.AbilityRuntime.Core`，`@systemapi`。

## 2. Native 绑定契约（C++ 侧已实现）

ANI 侧（`frameworks/ets/ani/app_memory/src/ets_app_memory.cpp`）通过 `Namespace_BindNativeFunctions` 绑定到命名空间 `@ohos.app.ability.appMemory.appMemory`：

| Native 函数 | C 签名 | 签名串 |
|-------------|--------|--------|
| `nativeReleaseFilePages` | `(ani_env*, ani_object callback, ani_object aniFileNames)` | `C{utils.AbilityUtils.AsyncCallbackWrapper}C{std.core.Array}:` |
| `nativeReleaseRawFilePages` | `(ani_env*, ani_object callback, ani_string aniModuleName)` | `C{utils.AbilityUtils.AsyncCallbackWrapper}C{std.core.String}:` |

回调语义：`AppExecFwk::AsyncCallback(env, callback, errorObj, nullptr)` —— 成功时 `errorObj` 为无错误对象、`result` 为 null；失败时 `errorObj` 携带 `code`（202/401/16000050）。

NAPI 侧（`frameworks/js/napi/app_memory/js_app_memory.cpp`）独立实现，模块名 `application.appMemory`（FA/JS 路径），不依赖此 SDK 声明。

## 3. SDK 侧待办

1. 落位 `appMemory.d.ts`（ets + js）。
2. 按其 AsyncCallbackWrapper 工具链，将公共 `releaseFilePages(fileNames): Promise<void>` 映射到 `nativeReleaseFilePages(callback, fileNames)`；`releaseRawFilePages(moduleName)` 映射到 `nativeReleaseRawFilePages(callback, moduleName)`。
3. 在 `api abnormalities` / SysCap 清单中登记 `SystemCapability.Ability.AbilityRuntime.Core`（复用现有，无需新增 SysCap）。
4. 分配 `@since` 版本号并补中文英文 JSDoc（如需）。

## 4. 验证依赖

- ANI `FindNamespace("@ohos.app.ability.appMemory.appMemory")` 在本声明落位前会失败并打 `TAG_LOGE`（已在 C++ 中降级处理，不影响其它模块加载）。落位后 ArkTS 通道即可用。
- NAPI 通道不依赖本声明，可先行验证。
