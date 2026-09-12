# SDK 接口设计文档：带 keepalive 防杀校验的查杀接口

> 基于 `proposal/20260818138826-05-proposal-P-01.md` 和 `.codespec/changes/issue-kill-process-safely/spec.md` 设计。
> 参照 `interface_sdk-js` 现有 `@ohos.app.ability.appManager.d.ts` 中 `killProcessWithAccount` 声明模式。

## 1. 设计概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 带 keepalive 防杀校验的查杀接口 |
| API 命名空间 | `appManager`（`@ohos.app.ability.appManager`） |
| 函数名 | `killProcessWithAccountSafely` |
| 开放范围 | `@systemapi`（仅系统应用可调用） |
| 权限 | `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS` + `KILL_APP_PROCESSES` 或 `CLEAN_BACKGROUND_PROCESSES` |
| SystemCapability | `SystemCapability.Ability.AbilityRuntime.Core` |
| 返回模式 | `Promise<void>`（仅 Promise，无 Callback 版本） |
| 目标版本 | API 27（OpenHarmony 7.1） |
| 模式参照 | `killProcessWithAccount(bundleName, accountId): Promise<void>`（since 9） |
| 新增类型文件 | 无（不引入新接口类型/数据结构） |

## 2. 产出文件清单

| 文件 | 用途 |
|------|------|
| `appManager.d.ts.snippet.md` | 需添加到 `@ohos.app.ability.appManager.d.ts` 的函数声明片段 |

> 与 `appFocusState` 特性不同，本特性**不新增独立 .d.ts 类型文件**，仅在 `appManager.d.ts` 中追加一个函数声明。无需新增导入、无需新增 `export type`。

## 3. 接口契约

### 3.1 JS API

```typescript
function killProcessWithAccountSafely(bundleName: string, accountId: int): Promise<void>;
```

### 3.2 参数约束

| 参数 | 类型 | 必填 | 约束条件 |
|------|------|------|----------|
| `bundleName` | `string` | 是 | 非空；目标应用的 bundle name |
| `accountId` | `int` | 是 | ≥ 0；系统账户 ID，参见 `osAccount.AccountManager.getOsAccountLocalId` |

### 3.3 返回值

| 结果 | 行为 |
|------|------|
| 查杀成功或进程未运行 | `Promise` resolve（无返回值） |
| 目标为 keepalive 应用 | `Promise` reject，`err.code = 2097184` |
| 权限不足 | `Promise` reject，`err.code = 201` |
| 非系统应用 | `Promise` reject，`err.code = 202` |
| 参数错误 | `Promise` reject，`err.code = 401` |
| 内部错误 | `Promise` reject，`err.code = 16000050` |

### 3.4 错误码

| 错误码 | 含义 | 触发条件 | 关联 AC |
|--------|------|----------|---------|
| 201 | Permission denied | 调用方无 `KILL_APP_PROCESSES` / `CLEAN_BACKGROUND_PROCESSES` 权限或跨账户权限 | AC-3, AC-4 |
| 202 | Not system application | 调用方非系统应用（`@systemapi` 约束） | AC-3 |
| 401 | Parameter error | `bundleName` 为空/类型错误，`accountId` 类型错误 | AC-6 |
| 16000050 | Internal error | 系统服务内部错误或 SA 不可用 | AC-8 |
| 2097184 | The target process is a keep-alive application and cannot be killed | 目标进程 `IsKeepAliveApp() == true` | AC-2 |

> 错误码 2097184 = `KILL_PROCESS_KEEP_ALIVE`，定义于 `ability_manager_errors.h`。该错误码已在 AMS（SA 180）路径中使用，本接口在 appmgr（SA 501）路径中复用同一错误码值，语义一致（spec Q-2 确认可接受）。

## 4. 设计决策

### 4.1 命名：`killProcessWithAccountSafely`

| 决策 | 理由 |
|------|------|
| 后缀 `Safely` 而非 `Safe` / `WithCheck` | `Safely` 表达"安全地执行查杀"语义；与 `killProcessWithAccount` 形成"不安全(强杀) vs 安全(防杀校验)"对照 |
| 不用 `killProcessSafely` | 保留 `WithAccount` 表明仍需指定账户，与旧接口签名对齐 |
| proposal R-4 注明"最终命名以 API Review 审批结论为准" | 暂定名，可调整 |

### 4.2 返回模式：仅 Promise，无 Callback

| 对比 | 旧接口 `killProcessWithAccount` | **新接口 `killProcessWithAccountSafely`** |
|------|-------------------------------|------------------------------------------|
| Promise 版 | ✓ since 9 | **✓ since 27** |
| Callback 版 | ✓ since 9 | **✗ 不提供** |

不提供 Callback 版本的理由：
- API 14 以后新增的查杀类接口（`isAppRunning`、`clearUpAppData`、`terminateMission`、`setKeepAliveForBundle` 等）均仅提供 Promise 版本
- Callback 版本为 API 9 遗留模式，新接口无需兼容旧模式
- 减少 NAPI 绑定代码量（无需实现 `AsyncCallback` 分支）
- spec 明确仅 Promise

> **API Review 待确认**：若 Review 要求提供 Callback 版本，需追加 `killProcessWithAccountSafely(bundleName: string, accountId: int, callback: AsyncCallback<void>): void` 声明。

### 4.3 参数：仅 `bundleName` + `accountId`

spec IPC 接口有 4 个参数：`bundleName`、`accountId`、`clearPageStack`(默认 false)、`appIndex`(默认 0)。JS API 仅暴露前两个。

| 参数 | JS API 暴露 | IPC 层默认值 | 理由 |
|------|------------|-------------|------|
| `bundleName` | ✓ | — | 必需 |
| `accountId` | ✓ | — | 必需 |
| `clearPageStack` | ✗ | `false` | 新接口语义为"安全查杀"，默认不清栈；如需清栈后续可增加重载 |
| `appIndex` | ✗ | `0` | 默认主应用；克隆应用场景后续可增加重载 |

这与 since-9 的 `killProcessWithAccount(bundleName, accountId): Promise<void>` 一致。如需扩展，可后续添加 `killProcessWithAccountSafely(bundleName, accountId, clearPageStack, appIndex?)` 重载。

### 4.4 访问级别：`@systemapi`

spec 和 proposal 均指定 `@systemapi`。与现有 `killProcessWithAccount` 一致（同为 `@systemapi` + `@throws 202`）。

### 4.5 权限声明

复用 `killProcessWithAccount` 的权限组合：

```
@permission ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS and ohos.permission.KILL_APP_PROCESSES
    or ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS and ohos.permission.CLEAN_BACKGROUND_PROCESSES
```

spec 写"权限: `KILL_APP_PROCESSES` 或 `CLEAN_BACKGROUND_PROCESSES`"，这是指 `VerifyAccountPermission` 内部的检查逻辑。但 SDK 声明需完整声明所有权限（含跨账户权限），与现有接口保持一致。同时补充说明：`accountId` 指定当前用户时免 `INTERACT_ACROSS_LOCAL_ACCOUNTS`（AC-11）。

> **NOTE** 保留"当 `accountId` 指定当前用户时，`INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限不要求"的说明，与旧接口一致。

### 4.6 无需新增类型文件

本接口不引入新的接口类型或数据结构：
- 参数仅 `string` + `int`（基本类型）
- 返回 `Promise<void>`
- 错误码为 `BusinessError`（已有通用类型）
- 无 observer/observer data 需要定义

因此无需创建 `application/` 目录下的新 `.d.ts` 文件，仅在 `appManager.d.ts` 中追加函数声明。

## 5. 与现有 API 的关系

| 现有 API | 关系 | 说明 |
|----------|------|------|
| `killProcessWithAccount(b, a): Promise<void>` | **并行（安全版）** | 旧接口无 keepalive 校验，直接杀进程；新接口先校验 keepalive 再决定是否杀 |
| `killProcessWithAccount(b, a, c, i?): Promise<void>` | **并行** | 同上，含 `clearPageStack` + `appIndex` 扩展参数 |
| `killProcessWithAccount(b, a, cb): void` | **并行** | Callback 版本，无安全版 |
| `killProcessesByBundleName(b): Promise<void>` | 独立 | 不按账户查杀，不在本特性范围 |
| `killProcessesInBatch(pids)` | 独立 | 批量查杀，不在本特性范围 |

### 安全版 vs 旧版行为对照

| 场景 | `killProcessWithAccount` (旧) | `killProcessWithAccountSafely` (新) |
|------|-------------------------------|--------------------------------------|
| 非 keepalive 应用 | 杀进程，resolve | 杀进程，resolve |
| keepalive 应用 | **杀进程**，resolve | **拒绝**，reject 2097184 |
| 非系统应用 | reject 202 | reject 202 |
| 无权限 | reject 201 | reject 201 |
| 参数错误 | reject 401 | reject 401 |
| 进程未运行 | resolve | resolve |
| SA 不可用 | reject 16000050 | reject 16000050 |

## 6. NAPI 绑定侧参考

现有 `js_app_manager.cpp` 中 `killProcessWithAccount` 的 NAPI 实现（约 :1854）可作为模板：

```
新增 NAPI 绑定:
  - 属性描述符: {"killProcessWithAccountSafely", ..., napi_kill_process_with_account_safely, ...}
  - 实现: napi_kill_process_with_account_safely(env, info)
    → 解析 bundleName (string) + accountId (int)
    → 创建 Promise
    → 调用 AppMgrClient::KillProcessWithAccountSafely(bundleName, accountId)
    → resolve/reject 回调
  → 参照 napi_kill_process_with_account 的 Promise 分支（不含 callback 参数解析）
```

## 7. 验证对照表

| Spec AC | SDK 接口覆盖 | 验证方式 |
|---------|-------------|----------|
| AC-1: 杀非 keepalive 成功 | `Promise<void>` resolve 声明 | 集成测试 |
| AC-2: keepalive 被拒绝 | `@throws BusinessError 2097184` | 集成测试 |
| AC-3: 非系统应用被拒绝 | `@systemapi` + `@throws 202` | 集成测试 |
| AC-4: 无权限被拒绝 | `@permission` + `@throws 201` | 集成测试 |
| AC-5: 旧接口不变 | 旧接口声明不变 | 回归测试 |
| AC-6: 参数错误 | `@throws 401` | JS API 单测 |
| AC-7: 进程未运行 resolve | `Promise<void>` resolve 语义 | 集成测试 |
| AC-8: SA 不可用 | `@throws 16000050` | 集成测试 |

## 8. 待确认项

| # | 问题 | 处理建议 |
|---|------|----------|
| 1 | API 版本号 `@since 27` 为推断值 | 需 API Review 确认实际引入版本 |
| 2 | 函数名 `killProcessWithAccountSafely` 为暂定名 | proposal R-4 注明"以 API Review 审批为准" |
| 3 | 是否需要 Callback 版本 | spec 仅 Promise；若 Review 要求需追加声明 |
| 4 | 是否需要 `clearPageStack` + `appIndex` 重载 | IPC 层已有参数；JS API 暂不暴露，后续可扩展 |
| 5 | 错误码 2097184 是否需在 SDK 中增加命名常量 | 现有模式均使用数值在 `@throws` 中声明，无需常量 |
| 6 | 中英文翻译同步 | `zh-cn/` 目录需同步产出中文翻译 |
