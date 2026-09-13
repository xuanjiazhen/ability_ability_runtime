# 特性规格：后台连接 AgentExtension 权限

> 本规格覆盖新增 `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限定义和 `CheckPermissionForAgentExtension` 前台拦截豁免逻辑。范围限定：权限定义注册、拦截点修改、权限校验分支。AgentExtension 自身生命周期管理逻辑、通用后台拉起路径不在本规格范围内。

> **权限校验完整链路（本规格的核心规格）：**
> 1. 校验是否系统应用 → 非系统应用返回 `202`（`ERR_NOT_SYSTEM_APP`）
> 2. 校验 `ohos.permission.CONNECT_AGENT` 权限 → 无权限返回 `201`（`ERR_PERMISSION_DENIED`）
> 3. 校验调用方是否前台 topAbility → 前台则直接放行（拉起）
> 4. 非前台 → 校验 `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限 → 有权限则放行（拉起），无权限返回 `16000053`（`NOT_TOP_ABILITY`）

> **跨用户与分身规格：** ConnectAgentExtension 不支持跨用户拉起 AgentExtension（`CheckCrossUser` 对 AGENT 类型返回 `false`，返回 `ERR_CROSS_USER`）；不支持拉起分身应用的 AgentExtension。上述行为与未引入 `CONNECT_AGENT_FROM_BACKGROUND` 权限时一致，新增后台权限不改变跨用户/分身限制。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 后台连接 AgentExtension 权限 |
| 特性编号 | REQ-CONNECT-AGENT-BG-001 |
| 所属 Epic | 系统应用后台 AgentExtension 连接 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability / Sig-Security |
| 状态 | Draft |
| 复杂度 | 简单 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。当前 `CheckPermissionForAgentExtension` 对后台拉起 Extension 类组件有统一拦截策略——非前台进程直接返回 `NOT_TOP_ABILITY`，未区分系统应用与普通应用。本特性在现有校验链路（系统应用校验 → `CONNECT_AGENT` 权限校验 → 前台校验）的基础上，于前台校验失败分支增加 `CONNECT_AGENT_FROM_BACKGROUND` 权限豁免，同时明确跨用户与分身限制不随新权限放开。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限定义 | `system_basic` 级别，注册到权限框架 |
| ADDED | `PERMISSION_CONNECT_AGENT_FROM_BACKGROUND` 常量 | `permission_constants.h` 新增 |
| MODIFIED | `CheckPermissionForAgentExtension` 前台检查 | 在 `processInfo.state_ != APP_STATE_FOREGROUND` 返回 `NOT_TOP_ABILITY` 前增加新权限检查分支（~8 行）：持有 `CONNECT_AGENT_FROM_BACKGROUND` 则跳过前台拦截继续放行 |
| REUSED | `PermissionVerification::VerifyCallingPermission` | 复用已有校验函数，无修改 |
| REUSED | `PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI` | 复用系统应用校验（:13116），无修改 |
| REUSED | `CheckCrossUser`（:18338） | 复用跨用户校验，AGENT 类型不在允许列表中，不随新权限放开 |
| KEPT | 跨用户 AgentExtension 限制 | `CheckCrossUser` 对 AGENT 类型返回 `false` → `ERR_CROSS_USER`，与未引入新权限时一致 |
| KEPT | 分身 AgentExtension 限制 | 不支持拉起分身应用的 AgentExtension，与未引入新权限时一致 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260905542161\05-proposal.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证：

### 发现 1：CheckPermissionForAgentExtension 实际行号与完整校验链路

`CheckPermissionForAgentExtension`（`ability_manager_service.cpp:13099-13139`）实现。完整校验链路如下：

```
:13102-13106  非 AGENT 类型 → ERR_OK 跳过
:13107-13110  类型不匹配 → ERR_WRONG_INTERFACE_CALL
:13111-13114  缺 AGENTEXTENSIONHOSTPROXY_KEY → ERR_WRONG_INTERFACE_CALL
:13116-13119  非系统应用 → ERR_NOT_SYSTEM_APP（SDK 202）        ← 第1步：系统应用校验
:13120-13124  校验 PERMISSION_CONNECT_AGENT → ERR_PERMISSION_DENIED（SDK 201）  ← 第2步：老权限校验
:13126-13133  获取 RunningProcessInfo
:13134-13137  进程状态 != APP_STATE_FOREGROUND → NOT_TOP_ABILITY（SDK 16000053）  ← 第3步：前台校验
             ↑ 本特性在此分支内增加 CONNECT_AGENT_FROM_BACKGROUND 豁免：持有则跳过拦截继续放行
:13138       全部通过 → ERR_OK
```

> **本特性修改点：** 在 `:13134` 的 `processInfo.state_ != APP_STATE_FOREGROUND` 判定分支内，原逻辑直接 `return NOT_TOP_ABILITY`。修改为：先检查 `VerifyCallingPermission(PERMISSION_CONNECT_AGENT_FROM_BACKGROUND)`，持有则不 return、继续走到 `:13138` 的 `ERR_OK`；未持有则维持原 `return NOT_TOP_ABILITY`。

> **Proposal 修正：** Proposal 称该方法在 `:13146`，实际在 `:13099`。`:13146` 属于下一个方法 `CheckCallServiceExtensionPermission`。

### 发现 2：错误码内部值与 SDK 公开值映射

| 常量 | 内部值 | SDK 公开值 | SDK 常量 | 文件 | 行号 |
|------|--------|-----------|----------|------|------|
| `ERR_NOT_SYSTEM_APP` | `2097209` | `202` | `ERROR_CODE_NOT_SYSTEM_APP` | `ability_business_error.h` | :31 |
| `ERR_PERMISSION_DENIED` | （全局 `errors.h`） | `201` | `ERROR_CODE_PERMISSION_DENIED` | `ability_business_error.h` | :28 |
| `NOT_TOP_ABILITY` | `0x500001` | `16000053` | `ERROR_CODE_NOT_TOP_ABILITY` | `ability_business_error.h` | :103 |

> 内部使用 `ERR_NOT_SYSTEM_APP`（`2097209`）、`ERR_PERMISSION_DENIED` 和 `NOT_TOP_ABILITY`（`0x500001`），在 SDK 边界分别映射为 `202`、`201`、`16000053`。spec 中 AC 使用 SDK 公开值。

### 发现 3：permission_constants.h 已有 PERMISSION_CONNECT_AGENT

`permission_constants.h`（`services/common/include/permission_constants.h`）:25 定义：
```cpp
constexpr const char* PERMISSION_CONNECT_AGENT = "ohos.permission.CONNECT_AGENT";
```

需在同文件新增：
```cpp
constexpr const char* PERMISSION_CONNECT_AGENT_FROM_BACKGROUND = "ohos.permission.CONNECT_AGENT_FROM_BACKGROUND";
```

### 发现 4：VerifyCallingPermission 可直接复用

`PermissionVerification::VerifyCallingPermission`（`permission_verification.h:46`，`permission_verification.cpp:59-75`）签名：
```cpp
bool VerifyCallingPermission(const std::string &permissionName, const uint32_t specifyTokenId = 0) const;
```
内部调用 `AccessTokenKit::VerifyAccessToken(callerToken, permissionName, false)`，返回 `PERMISSION_GRANTED` 时为 true。已被 244 处引用。**无需修改，直接传入新权限名即可**。

### 发现 5：AgentExtension 类型已完整集成

`ExtensionAbilityType::AGENT` 在代码库中有 116+ 处引用，`agent_runtime_framework/` 目录有完整的 Agent 管理服务。`CheckPermissionForAgentExtension` 是 AgentExtension 连接的核心拦截点，从 `ConnectAbilityCommon`（:6070）的 `ConnectLocalAbility`（:6441）:6486 调用。

### 发现 6：跨用户 AgentExtension 已被 CheckCrossUser 拦截

`CheckCrossUser`（`ability_manager_service.cpp:18338-18351`）仅允许 `DATASHARE` 和 `SERVICE` 类型在 `IsConnectSupportCrossUser()` 或 SA 调用时跨用户，AGENT 类型不在允许列表中，返回 `false` → `ERR_CROSS_USER`。该校验在 `ConnectLocalAbility:6451` 执行，**先于** `CheckPermissionForAgentExtension:6486`。引入 `CONNECT_AGENT_FROM_BACKGROUND` 权限不修改 `CheckCrossUser`，跨用户限制保持不变。

### 发现 7：分身应用 AgentExtension 不支持

AgentExtension 连接路径（`ConnectAbilityCommon` → `ConnectLocalAbility`）中，`CheckPermissionForAgentExtension` 不处理 `appIndex`/clone 逻辑。分身应用（sandbox clone）的 AgentExtension 拉起不支持，该限制与是否持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限无关，引入新权限不改变分身限制。

## 用户故事

### US-1: 系统应用后台连接 AgentExtension

**作为** 系统应用开发者
**我想要** 在后台状态下，持有 `CONNECT_AGENT` + `CONNECT_AGENT_FROM_BACKGROUND` 双权限后连接 AgentExtension
**以便** 系统应用后台业务流程不受阻断

### US-2: 非系统应用被拦截

**作为** 安全审查者
**我想要** 非系统应用调用 `ConnectAbilityCommon` 连接 AgentExtension 时返回 `202`（`ERR_NOT_SYSTEM_APP`）
**以便** 三方应用无法通过 AgentExtension 连接进入系统级后台拉起路径

### US-3: 未持有 CONNECT_AGENT 权限被拦截

**作为** 安全审查者
**我想要** 系统应用未持有 `ohos.permission.CONNECT_AGENT` 权限时返回 `201`（`ERR_PERMISSION_DENIED`）
**以便** 缺少老权限的调用方在任何场景下都无法连接 AgentExtension

### US-4: 后台未持有 CONNECT_AGENT_FROM_BACKGROUND 权限被拦截

**作为** 安全审查者
**我想要** 后台系统应用持有 `CONNECT_AGENT` 但未持有 `CONNECT_AGENT_FROM_BACKGROUND` 时返回 `16000053`（`NOT_TOP_ABILITY`）
**以便** 后台拉起权限不扩大攻击面

### US-5: 前台行为不受影响

**作为** 系统应用开发者
**我想要** 前台系统应用（持有 `CONNECT_AGENT`）连接 AgentExtension 行为不变
**以便** 现有前台路径无回归

### US-6: 跨用户 AgentExtension 不支持

**作为** 安全审查者
**我想要** 跨用户连接 AgentExtension 返回 `ERR_CROSS_USER`，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND`
**以便** 新增后台权限不放开跨用户 AgentExtension 拉起

### US-7: 分身应用 AgentExtension 不支持

**作为** 安全审查者
**我想要** 分身应用（sandbox clone）的 AgentExtension 拉起不被支持，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND`
**以便** 新增后台权限不放开分身 AgentExtension 拉起

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 权限定义文件注册到权限框架 THEN `availableLevel=system_basic`，可通过 ACL 授权 | 正常 | US-1 |
| AC-2 | WHEN 后台系统应用持有 `CONNECT_AGENT` + `CONNECT_AGENT_FROM_BACKGROUND` 权限调用 `ConnectAbilityCommon` 连接 AgentExtension THEN 返回 `ERR_OK`（0），跳过前台拦截 | 正常 | US-1 |
| AC-3 | WHEN 后台系统应用持有 `CONNECT_AGENT` 但未持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 THEN 返回 `16000053`（NOT_TOP_ABILITY），进程不被拉起 | 异常 | US-4 |
| AC-4 | WHEN 前台系统应用持有 `CONNECT_AGENT`（无论是否持新权限）调用 `ConnectAbilityCommon` THEN 返回 `ERR_OK`，行为不变 | 兼容 | US-5 |
| AC-5 | WHEN 非系统应用调用 THEN 返回 `202`（`ERR_NOT_SYSTEM_APP`），在 `CONNECT_AGENT` 权限校验之前拦截 | 异常 | US-2 |
| AC-6 | WHEN 系统应用未持有 `ohos.permission.CONNECT_AGENT` 权限 THEN 返回 `201`（`ERR_PERMISSION_DENIED`），在前台校验之前拦截 | 异常 | US-3 |
| AC-7 | WHEN 跨用户调用连接 AgentExtension THEN 返回 `ERR_CROSS_USER`，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 异常 | US-6 |
| AC-8 | WHEN 分身应用（sandbox clone）尝试拉起 AgentExtension THEN 不支持，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 异常 | US-7 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 权限配置文件审查 | security_permission 权限定义文件 |
| AC-2 | R-2 | 单元测试 | `ability_manager_service_test/`（持有双权限 + 后台） |
| AC-3 | R-3 | 单元测试 | `ability_manager_service_test/`（持 CONNECT_AGENT + 无新权限 + 后台） |
| AC-4 | R-4 | 回归测试 | `ability_manager_service_test/`（前台场景） |
| AC-5 | R-5 | 单元测试 | `ability_manager_service_test/`（非系统应用，断言返回 202） |
| AC-6 | R-6 | 单元测试 | `ability_manager_service_test/`（系统应用 + 无 CONNECT_AGENT，断言返回 201） |
| AC-7 | R-7 | 单元测试 | `ability_manager_service_test/`（跨用户 + AGENT 类型，断言 ERR_CROSS_USER） |
| AC-8 | R-8 | 单元测试 | `ability_manager_service_test/`（分身 appIndex + AGENT 类型，断言不支持） |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 权限定义注册 | `CONNECT_AGENT_FROM_BACKGROUND` 注册为 `system_basic` 级别 | 仅系统应用可通过 ACL 获得该权限 | AC-1 |
| R-2 | 行为 | 后台进程 + 持有 `CONNECT_AGENT` + 持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 跳过 `APP_STATE_FOREGROUND` 检查，继续执行 `ConnectAbilityCommon` | 在 `CheckPermissionForAgentExtension` :13134 分支内增加豁免，持有则不 return 继续走到 :13138 ERR_OK | AC-2 |
| R-3 | 异常 | 后台进程 + 持有 `CONNECT_AGENT` + 未持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 返回 `NOT_TOP_ABILITY`（`0x500001` / SDK `16000053`），不执行连接 | 与现有行为一致 | AC-3 |
| R-4 | 兼容 | 前台进程 + 持有 `CONNECT_AGENT` 调用 | 行为不变，新权限检查分支不影响前台路径 | 前台路径 `APP_STATE_FOREGROUND` 检查在新权限分支之前（:13134 判定为 false 时直接走到 :13138 ERR_OK） | AC-4 |
| R-5 | 异常 | 非系统应用调用（`JudgeCallerIsAllowedToUseSystemAPI` 返回 false） | 返回 `ERR_NOT_SYSTEM_APP`（SDK `202`），在 `CONNECT_AGENT` 权限校验之前拦截 | 校验顺序：系统应用（:13116）先于 `CONNECT_AGENT`（:13120）先于前台校验（:13134） | AC-5 |
| R-6 | 异常 | 系统应用 + 未持有 `ohos.permission.CONNECT_AGENT` 权限 | 返回 `ERR_PERMISSION_DENIED`（SDK `201`），在前台校验之前拦截 | 校验顺序：`CONNECT_AGENT`（:13120）先于前台校验（:13134） | AC-6 |
| R-7 | 异常 | 跨用户调用连接 AgentExtension | 返回 `ERR_CROSS_USER`，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND` | `CheckCrossUser`（:18338）对 AGENT 类型返回 `false`，在 `ConnectLocalAbility:6451` 先于权限校验执行；不随新权限放开 | AC-7 |
| R-8 | 异常 | 分身应用（sandbox clone）尝试拉起 AgentExtension | 不支持，无论是否持有 `CONNECT_AGENT_FROM_BACKGROUND` | 分身限制与未引入新权限时一致；新权限不修改分身处理逻辑 | AC-8 |

## 接口规格

### 权限定义

| 属性 | 值 |
|------|-----|
| 权限名 | `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` |
| 级别 | `system_basic` |
| 授权方式 | ACL（仅系统应用可通过 ACL 获得该权限） |
| 注册仓库 | security_permission（待定）/ ability_ability_runtime 常量声明 |
| 常量声明 | `permission_constants.h` 新增 `PERMISSION_CONNECT_AGENT_FROM_BACKGROUND` |

### 代码修改点

| 属性 | 值 |
|------|-----|
| 修改文件 | `services/abilitymgr/src/ability_manager_service.cpp` |
| 修改位置 | `CheckPermissionForAgentExtension` :13134 `processInfo.state_ != APP_STATE_FOREGROUND` 分支内 |
| 修改内容 | 在该分支内，原逻辑直接 `return NOT_TOP_ABILITY`。修改为：先 `VerifyCallingPermission(PERMISSION_CONNECT_AGENT_FROM_BACKGROUND)`，持有则不 return、继续走到 :13138 `ERR_OK`；未持有则维持 `return NOT_TOP_ABILITY` |
| 修改量 | ~8 行 |
| 复用函数 | `PermissionVerification::VerifyCallingPermission`（无修改） |

> **校验顺序（修改后完整链路）：**
> 1. :13116 `JudgeCallerIsAllowedToUseSystemAPI()` → false → `ERR_NOT_SYSTEM_APP`（202）
> 2. :13120 `VerifyCallingPermission(PERMISSION_CONNECT_AGENT)` → false → `ERR_PERMISSION_DENIED`（201）
> 3. :13134 `processInfo.state_ != APP_STATE_FOREGROUND` → false（前台）→ :13138 `ERR_OK`
> 4. :13134 `processInfo.state_ != APP_STATE_FOREGROUND` → true（后台）→ 新增：`VerifyCallingPermission(PERMISSION_CONNECT_AGENT_FROM_BACKGROUND)` → true → :13138 `ERR_OK`；false → `NOT_TOP_ABILITY`（16000053）

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 后台 + 系统应用 + 有 CONNECT_AGENT + 有 FROM_BACKGROUND | ERR_OK，连接成功 | AC-2 |
| 2 | 后台 + 系统应用 + 有 CONNECT_AGENT + 无 FROM_BACKGROUND | 16000053，拦截 | AC-3 |
| 3 | 前台 + 系统应用 + 有 CONNECT_AGENT（无论 FROM_BACKGROUND） | ERR_OK，行为不变 | AC-4 |
| 4 | 系统应用 + 无 CONNECT_AGENT（无论前台/后台） | 201（ERR_PERMISSION_DENIED） | AC-6 |
| 5 | 非系统应用（无论前台/后台） | 202（ERR_NOT_SYSTEM_APP） | AC-5 |
| 6 | 跨用户 + AGENT 类型（无论权限） | ERR_CROSS_USER | AC-7 |
| 7 | 分身应用（sandbox clone）+ AGENT 类型（无论权限） | 不支持 | AC-8 |

## 兼容性声明

- **已有 API 行为变更:** 否。新增权限为 opt-in，原有前台路径不受影响。未持有新权限的后台行为与当前一致。
- **IPC 序列化格式变更:** 否。权限校验在 AMS 进程内完成，不涉及 IPC 协议变更。
- **配置文件格式变更:** 是（新增权限定义文件），但不修改已有权限定义。
- **跨用户兼容性:** 不变。`CheckCrossUser` 对 AGENT 类型保持 `false`，新增 `CONNECT_AGENT_FROM_BACKGROUND` 权限不放开跨用户 AgentExtension 拉起。
- **分身兼容性:** 不变。分身应用 AgentExtension 拉起保持不支持，新增 `CONNECT_AGENT_FROM_BACKGROUND` 权限不放开分身 AgentExtension 拉起。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 权限限定 system_basic | 收敛攻击面，仅系统应用可通过 ACL 获得该权限 | AC-1, AC-3 |
| 校验顺序固定 | 系统应用（:13116）→ `CONNECT_AGENT`（:13120）→ 前台校验（:13134）→ `CONNECT_AGENT_FROM_BACKGROUND` 豁免（新增） | AC-2 ~ AC-6 |
| 新权限检查为追加豁免 | 不修改 `PERMISSION_CONNECT_AGENT` 检查逻辑和前台路径；新权限仅在前台校验失败的分支内生效 | AC-4 |
| 复用 VerifyCallingPermission | 不新增权限校验函数 | 全部 |
| 校验在 AMS 进程内完成 | 不涉及 IPC，无跨进程协议变更 | 全部 |
| 跨用户限制不变 | `CheckCrossUser` 对 AGENT 类型返回 `false`，不随新权限放开 | AC-7 |
| 分身限制不变 | 分身 AgentExtension 不支持，不随新权限放开 | AC-8 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 新增权限分支不应导致 AMS 崩溃 | 单测 | `ability_manager_service_test/` |
| 安全 | 未授权应用仍被拦截 | 权限校验单测 | — |
| 性能 | 不涉及 | — | — |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 |
|----------|----------|-----------|
| 手机 | 无差异 | — |
| 平板 | 无差异 | — |
| 折叠屏 | 无差异 | — |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 系统级权限机制，无 UI | — |
| 多用户 | 是 | 跨用户 AgentExtension 拉起不支持（`CheckCrossUser` 对 AGENT 返回 `false` → `ERR_CROSS_USER`），新权限不放开此限制 | AC-7 |
| 分身/克隆 | 是 | 分身应用 AgentExtension 拉起不支持，新权限不放开此限制 | AC-8 |
| 版本升级 | 否 | 无持久化数据 | — |
| 生态兼容 | 是 | 新增权限为 opt-in，未授权行为与当前一致 | AC-3 |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅新增权限 + 拦截豁免；AgentExtension 生命周期/通用后台路径不涉及）
- [x] 完整校验链路体现：系统应用(202) → CONNECT_AGENT(201) → 前台校验 → CONNECT_AGENT_FROM_BACKGROUND 豁免(16000053)
- [x] 跨用户/分身限制明确：不支持且不随新权限放开
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致（AC-1~AC-8 ↔ R-1~R-8）
- [x] 规则表每条通过 5 项质量检查

## 已记录问题（不在本规格解决）

| # | 问题 | 影响 | 处理建议 |
|---|------|------|----------|
| Q-1 | 权限定义文件格式和注册流程（security_permission 仓）待 Phase 2 验证 | 权限注册路径 | 开发阶段确定具体注册路径 |
| Q-2 | ACL 相关团队对齐 | 授权流程 | 需与安全团队对齐 ACL 配置 |
| Q-3 | 相关团队正式审查新增后台拉起权限的安全影响 | 安全审查 | 预审已提示需正式审查 |
| Q-4 | Proposal 称涉及仓含 security_permission（待定），实际代码修改仅在 ability_ability_runtime | 仓库范围 | 权限常量声明在 ability_ability_runtime，权限定义文件注册在 security_permission |

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "CheckPermissionForAgentExtension 的完整实现与 ConnectAbilityCommon 调用链"
  - repo: "ability_ability_runtime"
    query: "permission_constants.h 的权限常量定义模式"
  - repo: "ability_ability_runtime"
    query: "PermissionVerification::VerifyCallingPermission 实现与 VerifyAccessToken 调用"
  - repo: "ability_ability_runtime"
    query: "NOT_TOP_ABILITY 内部值 0x500001 与 SDK 值 16000053 的映射关系"
  - repo: "ability_ability_runtime"
    query: "ERR_NOT_SYSTEM_APP 与 ERR_PERMISSION_DENIED 的 SDK 映射值"
  - repo: "ability_ability_runtime"
    query: "CheckCrossUser 对 AGENT 类型的跨用户拦截逻辑"
  - repo: "ability_ability_runtime"
    query: "ConnectLocalAbility 中 CheckCrossUser 与 CheckPermissionForAgentExtension 的调用顺序"
  - repo: "ability_ability_runtime"
    query: "分身应用 sandbox clone appIndex 在 ConnectAbilityCommon 路径中的处理"
```

**关键文档：**
- `services/abilitymgr/src/ability_manager_service.cpp`（`CheckPermissionForAgentExtension` :13099-13139，`ConnectAbilityCommon` :6070，`ConnectLocalAbility` :6441，`CheckCrossUser` :18338-18351）
- `services/common/include/permission_constants.h`（`PERMISSION_CONNECT_AGENT` :25，权限常量模式）
- `services/common/include/permission_verification.h`（`VerifyCallingPermission` :46，`JudgeCallerIsAllowedToUseSystemAPI`）
- `services/common/src/permission_verification.cpp`（:59-75）
- `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`（`NOT_TOP_ABILITY` = `0x500001`，:1310；`ERR_NOT_SYSTEM_APP` = `2097209`，:310；`ERR_CROSS_USER`，:300）
- `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h`（`ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP` = `202`，:90；`ERR_ABILITY_RUNTIME_EXTERNAL_NOT_TOP_ABILITY` = `16000053`，:48）
- `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（`ERROR_CODE_PERMISSION_DENIED` = `201`，:28；`ERROR_CODE_NOT_SYSTEM_APP` = `202`，:31；`ERROR_CODE_NOT_TOP_ABILITY` = `16000053`，:103）
- `services/abilitymgr/include/ability_manager_service.h`（`CheckPermissionForAgentExtension` 声明，:3715）
