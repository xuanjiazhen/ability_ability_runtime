# 特性规格：带 keepalive 防杀校验的查杀 JS API

> 本规格覆盖新增 `killProcessWithAccountSafely` @systemapi JS API 的全链路行为。范围限定：新接口的参数契约、keepalive 校验逻辑、错误码语义、IPC 全链路（Proxy→Stub→ServiceInner）。旧接口 `killProcessWithAccount` 行为不变、keepalive 设置机制、UX 交互不在本规格范围内。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 带 keepalive 防杀校验的查杀接口 |
| 特性编号 | FEAT-KILL-SAFE-001 |
| 所属 Epic | MDM 企业管控防杀 |
| 优先级 | P0 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 简单 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。在 `AmsMgrScheduler` IPC 链路和 `AppMgrServiceInner` 服务侧新增带 keepalive 校验的查杀路径，与现有 `KillProcessWithAccount` 并行存在。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `killProcessWithAccountSafely` @systemapi JS API | 系统应用调用，杀非 keepalive 应用成功，杀 keepalive 应用返回错误码 |
| ADDED | `KillProcessWithAccountSafely` IPC 方法 | `IAmsMgr` 接口新增虚方法 + Proxy + Stub + Handler |
| ADDED | IPC 事务码 `KILL_PROCESS_WITH_ACCOUNT_SAFELY` | `IAmsMgr::Message` 枚举新增项，在 END 前递增 |
| ADDED | `KillApplicationByUserIdSafely` 服务侧方法 | `AppMgrServiceInner` 新增，含 `IsKeepAliveApp()` 校验 |
| REUSED | 错误码 `KILL_PROCESS_KEEP_ALIVE`（2097184） | 复用已有错误码，不新增 |
| REUSED | `AppRunningRecord::IsKeepAliveApp()` | 复用已有 keepalive 状态查询方法 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260818138826-05-proposal-P-01.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证，proposal 中有一处关键描述需要修正：

### 发现 1：keepalive 校验当前不在 appmgr 侧

Proposal 称 `AppMgrServiceInner::KillApplicationByUserId`（`app_mgr_service_inner.cpp:3884-3899`，紧随其后是 `KillApplicationByUserIdLocked` 在 3901-3930）无 keepalive 校验——**此描述准确**。该方法仅做权限检查后委托 `KillApplicationByUserIdLocked` → `ProcessExitByBundleNameAndUid` → `WaitProcessesExitAndKill`，全链路无 keepalive 检查。

但 `KILL_PROCESS_KEEP_ALIVE`（2097184）错误码当前**仅在 `ability_manager_service.cpp`（SA 180）中返回**，而非 appmgr（SA 501）：

| 位置 | 代码 | 条件 |
|------|------|------|
| `ability_manager_service.cpp:3955-3960` | `return KILL_PROCESS_KEEP_ALIVE` | `KeepAliveUtils::IsKeepAliveBundle(bundleInfo, userId, type) && IsMemorySizeSufficient()` |
| `ability_manager_service.cpp:9364-9369` | `return KILL_PROCESS_KEEP_ALIVE` | 同上 |

**关键差异：** AMS 侧的 keepalive 拒绝是**条件性**的——仅在内存充足时拒绝杀 keepalive 进程；内存不足时仍允许杀死。本特性在 appmgr 侧新增的校验应为**无条件拒绝**（MDM 场景要求 keepalive 不被杀，不因内存状态变化）。

### 发现 2：IsKeepAliveApp 实现确认

`AppRunningRecord::IsKeepAliveApp()`（`app_running_record.cpp:1516-1526`）实现：

```cpp
bool AppRunningRecord::IsKeepAliveApp() const
{
    if (!isMainProcess_ || !isKeepAliveBundle_ || !isKeepAliveRdb_) {
        return false;
    }
    auto userId = GetUid() / BASE_USER_RANGE;
    if (userId == 0) {
        return isSingleton_;
    }
    return true;
}
```

三条件：主进程 + keepalive bundle 标记 + keepalive RDB 标记，**userId == 0（零号用户）时额外需 singleton**，非零 userId 直接返回 true。已在 19 处生产代码中被引用，但不在 `KillApplicationByUserId` 路径上。

**keepalive 保护范围说明：** `IsKeepAliveApp()` 的三条件（`isMainProcess_` + `isKeepAliveBundle_` + `isKeepAliveRdb_`）仅在 MDM 接口设置 keepalive 时同时满足。以下两种"保活"方式**不满足三条件**，`IsKeepAliveApp()` 返回 false，新接口仍可杀掉：

| 保活类型 | `isKeepAliveBundle_` | `isKeepAliveRdb_` | `IsKeepAliveApp()` | 新接口可杀 |
|----------|---------------------|---------------------|---------------------|-----------|
| MDM 接口设置 keepalive | ✅ true | ✅ true | ✅ true → **拒绝** | ❌ 不可杀 |
| 系统应用配置文件 `keepAlive: true` | ✅ true | ❌ false | ❌ false | ✅ 可以杀 |
| 托盘设置保活 | ❌ false | ❌ false | ❌ false | ✅ 可以杀 |

> 设计意图：MDM 场景的企业管控策略要求进程不可被误杀，故新接口无条件拒绝杀 MDM keepalive 进程。其他常驻类型（配置文件 keepAlive、托盘保活）仍可通过新接口查杀，与普通进程一致。

### 发现 3：完整 IPC 链路模板

`KillProcessWithAccount` 的完整 IPC 栈已验证（将作为新接口的模板）：

| 层 | 文件 | 行号 |
|----|------|------|
| 接口声明 | `ams_mgr_interface.h` | 151-160 |
| Proxy 声明 | `ams_mgr_proxy.h` | 139-148 |
| Proxy 实现 | `ams_mgr_proxy.cpp` | 411-452 |
| Stub 声明 | `ams_mgr_stub.h` | 58 |
| Stub 分发 | `ams_mgr_stub.cpp` | 106-107 |
| Stub Handler | `ams_mgr_stub.cpp` | 394-415 |
| NAPI 绑定 | `js_app_manager.cpp` | 1854 |

## 用户故事

### US-1: 系统应用通过新接口杀非 keepalive 应用

**作为** 系统应用开发者
**我想要** 调用带防杀校验的查杀接口杀非 keepalive 应用
**以便** 查杀行为与旧接口一致且支持防杀语义

### US-2: keepalive 应用通过新接口被拒绝

**作为** MDM 应用
**我想要** 设置 keepalive 后通过新接口查杀时被拒绝
**以便** 企业管控策略持续生效不会因误杀中断

### US-3: 非系统应用被拒绝

**作为** 平台安全
**我想要** 新接口仅系统应用可调用
**以便** 三方应用无法绕过防杀保护

### US-4: 旧接口行为不变

**作为** 系统应用
**我想要** 旧接口 `killProcessWithAccount` 行为不变
**以便** 强杀场景和已有代码不受影响

### US-5: 非 MDM 保活进程仍可查杀

**作为** 系统应用
**我想要** 配置文件 `keepAlive: true` 或托盘保活的进程仍可通过新接口查杀
**以便** 仅 MDM 设置的 keepalive 受到防杀保护，其他常驻类型不受影响

### US-6: 同账户查杀无需跨账户权限

**作为** 系统应用
**我想要** `accountId` 指定当前用户时无需 `INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限
**以便** 同账户内的查杀操作无额外权限要求

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 系统应用持 `KILL_APP_PROCESSES` 或 `CLEAN_BACKGROUND_PROCESSES` 权限调用 `killProcessWithAccountSafely` 杀非 keepalive 应用 THEN 进程退出，返回 `ERR_OK`（0） | 正常 | US-1 |
| AC-2 | WHEN 系统应用通过新接口查杀 keepalive 应用（`IsKeepAliveApp() == true`） THEN 进程仍在运行，返回 `KILL_PROCESS_KEEP_ALIVE`（2097184） | 异常 | US-2 |
| AC-3 | WHEN 非系统应用调用 `killProcessWithAccountSafely` THEN 返回权限拒绝错误码，进程不被杀 | 异常 | US-3 |
| AC-4 | WHEN 系统应用未持 `KILL_APP_PROCESSES` 且未持 `CLEAN_BACKGROUND_PROCESSES` 权限 THEN 返回 `ERR_PERMISSION_DENIED`，进程不被杀 | 异常 | US-3 |
| AC-5 | WHEN 通过旧接口 `killProcessWithAccount` 杀 keepalive 应用 THEN 旧接口行为不变（直接杀掉，不校验 keepalive） | 兼容 | US-4 |
| AC-6 | WHEN `bundleName` 为空或不存在、`accountId` 无效 THEN 返回 `ERR_INVALID_VALUE`，不发生崩溃 | 边界 | US-1 |
| AC-7 | WHEN 目标应用进程未运行 THEN 返回 `ERR_OK`（进程未启动视为成功，与旧接口一致） | 边界 | US-1 |
| AC-8 | WHEN AppMgrService SA 未加载或代理获取失败 THEN NAPI 层 reject 错误码（与旧接口降级行为一致） | 恢复 | US-1 |
| AC-9 | WHEN 系统应用配置文件 `keepAlive: true` 的进程通过新接口查杀 THEN 进程被杀，返回 `ERR_OK`（`IsKeepAliveApp()` 返回 false，非 MDM keepalive 不受保护） | 正常 | US-5 |
| AC-10 | WHEN 托盘设置保活的进程通过新接口查杀 THEN 进程被杀，返回 `ERR_OK`（同上，`IsKeepAliveApp()` 返回 false） | 正常 | US-5 |
| AC-11 | WHEN `accountId` 指定当前登录用户 THEN 无需 `INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限即可查杀（仅需 `KILL_APP_PROCESSES` 或 `CLEAN_BACKGROUND_PROCESSES`） | 正常 | US-6 |
| AC-12 | WHEN `accountId` 指定非当前用户且持 `INTERACT_ACROSS_LOCAL_ACCOUNTS` + `KILL_APP_PROCESSES`（或 `CLEAN_BACKGROUND_PROCESSES`）权限 THEN 正常查杀 | 正常 | US-1 |
| AC-13 | WHEN `accountId` 指定非当前用户且无 `INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限 THEN reject `201`，进程不被杀 | 异常 | US-6 |
| AC-14 | WHEN `accountId` 为负数 THEN reject `401` 参数错误，不发生查杀 | 边界 | US-1 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 集成 + 单测 | `appmgr/test/unittest/` |
| AC-2 | R-2 | 集成 + 单测 | `appmgr/test/unittest/` |
| AC-3 | R-3 | 单测 | `js_app_manager` NAPI 单测 |
| AC-4 | R-3 | 单测 | `appmgr/test/unittest/` |
| AC-5 | R-4 | 回归测试 | `appmgr/test/unittest/` |
| AC-6 | R-5 | 单测 | `appmgr/test/unittest/` |
| AC-7 | R-6 | 单测 | `appmgr/test/unittest/` |
| AC-8 | R-7 | 单测 | `app_mgr_client_test/` |
| AC-9 | R-8 | 集成 | 配置文件 `keepAlive: true` 应用查杀验证 |
| AC-10 | R-8 | 集成 | 托盘保活应用查杀验证 |
| AC-11 | R-9 | 集成 + 单测 | 同账户查杀（`accountId` = 当前用户） |
| AC-12 | R-1, R-9 | 集成 | 跨账户查杀（持 `INTERACT_ACROSS_LOCAL_ACCOUNTS`） |
| AC-13 | R-10 | 集成 + 单测 | 跨账户查杀（无 `INTERACT_ACROSS_LOCAL_ACCOUNTS`） |
| AC-14 | R-11 | 单测 | `accountId` 负数参数校验 |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 系统应用 + 持权限 + 目标非 keepalive | 执行杀进程，返回 ERR_OK | 与旧接口 `KillProcessWithAccount` 执行路径一致 | AC-1 |
| R-2 | 异常 | 目标应用 `IsKeepAliveApp() == true` | 不执行杀进程，返回 `KILL_PROCESS_KEEP_ALIVE`（2097184） | **无条件拒绝**，不因内存状态变化（与 AMS 侧条件性拒绝不同） | AC-2 |
| R-3 | 异常 | 调用方非系统应用或缺少权限 | 返回 `ERR_PERMISSION_DENIED`，不执行杀进程 | 复用 `CHECK_CALLER_IS_SYSTEM_APP` + `VerifyAccountPermission` | AC-3, AC-4 |
| R-4 | 兼容 | 通过旧接口 `killProcessWithAccount` 调用 | 行为不变，不校验 keepalive，直接执行杀进程 | 旧接口代码逐行不变 | AC-5 |
| R-5 | 边界 | `bundleName` 为空/不存在，或 `accountId` 无效 | 返回 `ERR_INVALID_VALUE` | 参数校验在入口执行 | AC-6 |
| R-6 | 边界 | 目标应用进程未运行 | 返回 `ERR_OK` | 与旧接口 `ProcessExitByBundleNameAndUid` 返回 false 时返回 ERR_OK 一致 | AC-7 |
| R-7 | 恢复 | AppMgrService SA 未加载或 IPC 代理为空 | NAPI reject 错误码 | 客户端按需拉起 SA；拉起失败即 reject | AC-8 |
| R-8 | 行为 | 目标进程为配置文件 `keepAlive: true` 或托盘保活，`IsKeepAliveApp()` 返回 false | 执行杀进程，返回 ERR_OK | 非 MDM keepalive 不受防杀保护 | AC-9, AC-10 |
| R-9 | 权限 | `accountId` == 当前登录用户 | 不要求 `INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限，仅需 `KILL_APP_PROCESSES` 或 `CLEAN_BACKGROUND_PROCESSES` | 与旧接口 `killProcessWithAccount` 同账户权限豁免一致 | AC-11 |
| R-10 | 权限 | `accountId` != 当前登录用户且无 `INTERACT_ACROSS_LOCAL_ACCOUNTS` 权限 | 返回 `ERR_PERMISSION_DENIED`（201），不执行杀进程 | 跨账户查杀需额外权限 | AC-13 |
| R-11 | 边界 | `accountId` 为负数 | 返回参数错误（401），不执行杀进程 | 参数校验在入口执行 | AC-14 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|--------|----------|---------|
| `appManager.killProcessWithAccountSafely` | @systemapi | `(bundleName: string, accountId: number): Promise<void>` | `Promise<void>` | 2097184, 权限拒绝码, 参数错误码 | 带防杀校验的查杀接口 | AC-1~AC-14 |

### 变更/废弃 API

无变更或废弃项。旧接口 `killProcessWithAccount` 签名与行为完全不变。

## 接口规格

### JS API

| 属性 | 值 |
|------|-----|
| 函数签名 | `killProcessWithAccountSafely(bundleName: string, accountId: number): Promise<void>` |
| 返回值 | `Promise<void>` — resolve 表示查杀成功或进程未运行；reject 为错误码 |
| 开放范围 | @systemapi（仅系统应用可调用） |
| 权限 | `ohos.permission.INTERACT_ACROSS_LOCAL_ACCOUNTS` + `KILL_APP_PROCESSES` 或 `INTERACT_ACROSS_LOCAL_ACCOUNTS` + `CLEAN_BACKGROUND_PROCESSES`（`accountId` 指定当前用户时免 `INTERACT_ACROSS_LOCAL_ACCOUNTS`） |
| 错误码 | 2097184（KILL_PROCESS_KEEP_ALIVE）、权限拒绝码、参数错误码 |
| d.ts 位置 | `api/@ohos.app.ability.appManager.d.ts` |
| 关联 AC | AC-1~AC-14 |

### 参数约束

| 参数 | 类型 | 必填 | 约束条件 |
|------|------|------|----------|
| bundleName | string | 是 | 非空；目标应用的 bundle name |
| accountId | number | 是 | ≥ 0；账户 ID |

### Inner API（IPC 接口）

| 属性 | 值 |
|------|-----|
| 接口 | `IAmsMgr::KillProcessWithAccountSafely(const std::string &bundleName, const int accountId, const bool clearPageStack = false, int32_t appIndex = 0)` |
| IPC 事务码 | `IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT_SAFELY`（在枚举 END 前递增） |
| 调用模式 | `MessageOption::TF_SYNC` |
| 序列化 | `WriteInterfaceToken` + `WriteString(bundleName)` + `WriteInt32(accountId)` + `WriteBool(clearPageStack)` + `WriteInt32(appIndex)` |
| 服务端实现 | `AppMgrServiceInner::KillApplicationByUserIdSafely` → 权限校验 → `GetAppRunningRecordByBundleName` → `IsKeepAliveApp()` 校验 → 拒绝或委托 `KillApplicationByUserIdLocked` |

### 服务侧处理流程

```
KillApplicationByUserIdSafely(bundleName, appCloneIndex, userId, clearPageStack, reason):
  1. CHECK_CALLER_IS_SYSTEM_APP          → 非系统应用: ERR_PERMISSION_DENIED (202)
  2. accountId < 0 参数校验              → reject 401 参数错误
  3. accountId == 当前用户?              → 跳过 INTERACT_ACROSS_LOCAL_ACCOUNTS 校验
     accountId != 当前用户?              → 校验 INTERACT_ACROSS_LOCAL_ACCOUNTS，无权限: ERR_PERMISSION_DENIED (201)
  4. 校验 KILL_APP_PROCESSES 或 CLEAN_BACKGROUND_PROCESSES  → 无权限: ERR_PERMISSION_DENIED (201)
  5. appRunningManager_ null check       → null: ERR_NO_INIT
  6. GetAppRunningRecordByBundleName → record null: ERR_OK (进程未运行)
  7. record->IsKeepAliveApp() == true    → return KILL_PROCESS_KEEP_ALIVE (2097184)
     （仅 MDM keepalive 三条件全满足时返回 true；配置文件 keepAlive/托盘保活返回 false）
  8. KillApplicationByUserIdLocked(bundleName, appCloneIndex, userId, config)  → 复用旧路径
```

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 系统应用 + 持权限 + 非 keepalive 目标 | 杀进程，resolve ERR_OK | AC-1 |
| 2 | 系统应用 + 持权限 + MDM keepalive 目标 | 不杀，reject 2097184 | AC-2 |
| 3 | 非系统应用 | reject 权限错误码 | AC-3 |
| 4 | 系统应用 + 无权限 | reject ERR_PERMISSION_DENIED | AC-4 |
| 5 | 旧接口杀 keepalive | 直接杀掉（行为不变） | AC-5 |
| 6 | bundleName 空/不存在 | reject ERR_INVALID_VALUE | AC-6 |
| 7 | 目标进程未运行 | resolve ERR_OK | AC-7 |
| 8 | SA 不可用 | reject（降级） | AC-8 |
| 9 | 系统应用配置文件 keepAlive=true 的进程 | `IsKeepAliveApp()` 返回 false，杀进程，resolve ERR_OK | AC-9 |
| 10 | 托盘设置保活的进程 | 同上，杀进程，resolve ERR_OK | AC-10 |
| 11 | accountId = 当前用户，仅持 KILL_APP_PROCESSES | 无需 INTERACT_ACROSS_LOCAL_ACCOUNTS，正常查杀 | AC-11 |
| 12 | accountId = 其他用户，持 INTERACT_ACROSS_LOCAL_ACCOUNTS + KILL_APP_PROCESSES | 正常查杀 | AC-12 |
| 13 | accountId = 其他用户，无 INTERACT_ACROSS_LOCAL_ACCOUNTS | reject 201，进程不被杀 | AC-13 |
| 14 | accountId 为负数 | reject 401，不查杀 | AC-14 |

## 兼容性声明

- **已有 API 行为变更:** 否。旧接口 `killProcessWithAccount` 代码逐行不变。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。
- **IPC 序列化格式变更:** 新增事务码，不影响已有事务码。新接口序列化格式与 `KILL_PROCESS_WITH_ACCOUNT` 一致（相同的参数集）。
- **错误码语义变更:** `KILL_PROCESS_KEEP_ALIVE`（2097184）语义不变（keepalive 进程不可杀），仅新增一个返回该错误码的路径。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 新接口为 opt-in 新增 | 不影响已有应用；系统应用需主动迁移到新接口 | AC-5 |
| keepalive 校验无条件拒绝 | 与 AMS 侧条件性拒绝（依赖 `IsMemorySizeSufficient`）不同，appmgr 侧新路径无条件拒绝 | AC-2 |
| 复用 `IsKeepAliveApp()` | 不新增 keepalive 判断逻辑，复用 `app_running_record.cpp:1516` | AC-2 |
| 仅 MDM keepalive 受保护 | `IsKeepAliveApp()` 三条件全满足时才拒绝；配置文件 `keepAlive: true`（缺 `isKeepAliveRdb_`）和托盘保活（缺 `isKeepAliveBundle_` + `isKeepAliveRdb_`）不满足条件，可被杀 | AC-9, AC-10 |
| 复用 `KillApplicationByUserIdLocked` | 杀进程的底层路径与旧接口一致，仅在校验环节增加 keepalive 检查 | AC-1 |
| 同账户免跨账户权限 | `accountId` == 当前用户时，不要求 `INTERACT_ACROSS_LOCAL_ACCOUNTS`；非当前用户时需该校验 | AC-11, AC-13 |
| IPC 事务码在 END 前递增 | 遵循 `IAmsMgr::Message` 枚举约定，不破坏已有事务码编号 | 全部 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 单次查杀不应导致 AppMgrService SA 崩溃 | 压力测试 | `appmgr/test/unittest/` |
| 安全 | 非系统应用/无权限调用均被拒绝 | 单测 | `appmgr/test/unittest/` |
| 性能 | 不涉及 | — | — |
| 可测试性 | 新接口行为可经 mock 服务端校验错误码 | 单测 | `app_mgr_client_test/` |
| 定界定位 | 查杀路径有 HiLog 日志（tag: AAFwkTag::APPMGR） | 日志检查 | — |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 |
|----------|----------|-----------|
| 手机 | 无差异 | — |
| 平板 | 无差异 | — |
| 折叠屏 | 无差异 | — |
| 企业设备（MDM） | keepalive 应用为主要受益方 | — |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 查杀接口无 UI | — |
| 多用户 | 是 | 按 accountId 校验权限 | AC-3, AC-4 |
| 版本升级 | 否 | 无持久化数据需迁移 | — |
| 生态兼容 | 是 | 旧接口不变；新接口为 opt-in | AC-5 |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅新接口；旧接口行为不变）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查

## 已记录问题（不在本规格解决）

| # | 问题 | 影响 | 处理建议 |
|---|------|------|----------|
| Q-1 | proposal 描述 `KillApplicationByUserId` 在 `app_mgr_service_inner.cpp` 中无 keepalive 校验——准确。但 `KILL_PROCESS_KEEP_ALIVE` 当前仅在 `ability_manager_service.cpp`（SA 180）中条件性返回（依赖 `IsMemorySizeSufficient`）。新接口在 appmgr 侧无条件拒绝，与 AMS 侧行为存在语义差异。 | 双路径 keepalive 语义不一致 | design.md 需明确：appmgr 新路径为无条件拒绝，AMS 旧路径保持条件性拒绝。两条路径服务不同场景（MDM 防误杀 vs 内存管理）。 |
| Q-2 | proposal 称复用错误码 `KILL_PROCESS_KEEP_ALIVE`（2097184），但该错误码当前仅在 AMS 路径返回。在 appmgr 路径返回同一错误码是否会引起调用方混淆？ | 错误码语义复用 | 可接受：错误码语义为"keepalive 进程不可杀"，与新接口场景一致。design.md 需说明双路径复用同一错误码。 |
| Q-3 | proposal 称涉及仓为 `ability_ability_runtime` 和 `interface_sdk-js`，但 `KILL_PROCESS_KEEP_ALIVE` 定义在 `ability_manager_errors.h`（ability_ability_runtime 内）。无需跨仓新增错误码。 | 无 | 确认无误。 |
| Q-4 | proposal US-5 要求"任务管理器/设置中使用新接口查杀"，属调用方迁移而非 API 行为规格。本 spec 范围限定为 API 契约与 IPC 链路，迁移工作在 execution-plan 阶段跟踪。 | US-5 覆盖缺口 | execution-plan.md 需产出迁移 Task：将任务管理器和设置应用的 `killProcessWithAccount` 调用替换为 `killProcessWithAccountSafely`，并验证 keepalive 应用在用户操作中实际受到保护。 |

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "KillProcessWithAccount 的完整 IPC 链路：ams_mgr_interface → ams_mgr_proxy → ams_mgr_stub → app_mgr_service_inner"
  - repo: "ability_ability_runtime"
    query: "AppRunningRecord::IsKeepAliveApp 的三条件校验（userId==0 时额外检查 isSingleton_）与 19 处调用场景"
  - repo: "ability_ability_runtime"
    query: "KILL_PROCESS_KEEP_ALIVE 错误码在 ability_manager_service.cpp 中的条件性返回逻辑"
  - repo: "ability_ability_runtime"
    query: "js_app_manager.cpp 中 killProcessWithAccount 的 NAPI 绑定与 Promise 封装模式"
```

**关键文档：**
- `interfaces/inner_api/app_manager/include/appmgr/ams_mgr_interface.h`（IAmsMgr 接口，新增虚方法位置）
- `interfaces/inner_api/app_manager/include/appmgr/ams_mgr_proxy.h` / `src/appmgr/ams_mgr_proxy.cpp`（Proxy 模板）
- `interfaces/inner_api/app_manager/include/appmgr/ams_mgr_stub.h` / `src/appmgr/ams_mgr_stub.cpp`（Stub 模板）
- `services/appmgr/src/app_mgr_service_inner.cpp`（`KillApplicationByUserId` 现有实现，:3884-3899 + `KillApplicationByUserIdLocked` :3901-3930）
- `services/appmgr/src/app_running_record.cpp`（`IsKeepAliveApp` 实现，:1516-1526）
- `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`（`KILL_PROCESS_KEEP_ALIVE` 定义，:181-184）
- `frameworks/js/napi/app/js_app_manager/js_app_manager.cpp`（NAPI 绑定入口，:1854）
