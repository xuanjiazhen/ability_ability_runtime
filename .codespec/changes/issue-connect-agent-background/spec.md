# 特性规格：后台连接 AgentExtension 权限

> 本规格覆盖新增 `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限定义和 `CheckPermissionForAgentExtension` 前台拦截豁免逻辑。范围限定：权限定义注册、拦截点修改、权限校验分支。AgentExtension 自身生命周期管理逻辑、通用后台拉起路径不在本规格范围内。

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

本特性为新增能力（lineage: new）。当前 `CheckPermissionForAgentExtension` 对后台拉起 Extension 类组件有统一拦截策略——非前台进程直接返回 `NOT_TOP_ABILITY`，未区分系统应用与普通应用。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限定义 | `system_basic` 级别，注册到权限框架 |
| ADDED | `PERMISSION_CONNECT_AGENT_FROM_BACKGROUND` 常量 | `permission_constants.h` 新增 |
| MODIFIED | `CheckPermissionForAgentExtension` 前台检查 | 在 `APP_STATE_FOREGROUND` 检查前增加新权限检查分支（~8 行） |
| REUSED | `PermissionVerification::VerifyCallingPermission` | 复用已有校验函数，无修改 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260905542161\05-proposal.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证：

### 发现 1：CheckPermissionForAgentExtension 实际行号

`CheckPermissionForAgentExtension`（`ability_manager_service.cpp:13099-13139`）实现：

```
:13102-13106  非 AGENT 类型 → ERR_OK 跳过
:13107-13110  类型不匹配 → ERR_WRONG_INTERFACE_CALL
:13111-13114  缺 AGENTEXTENSIONHOSTPROXY_KEY → ERR_WRONG_INTERFACE_CALL
:13116-13119  非系统应用 → ERR_NOT_SYSTEM_APP
:13120-13124  校验 PERMISSION_CONNECT_AGENT → ERR_PERMISSION_DENIED
:13126-13133  获取 RunningProcessInfo
:13134-13137  进程状态 != APP_STATE_FOREGROUND → NOT_TOP_ABILITY (0x500001)
:13138       全部通过 → ERR_OK
```

> **Proposal 修正：** Proposal 称该方法在 `:13146`，实际在 `:13099`。`:13146` 属于下一个方法 `CheckCallServiceExtensionPermission`。

### 发现 2：NOT_TOP_ABILITY 内部值与公开值不同

| 常量 | 值 | 文件 | 行号 |
|------|-----|------|------|
| `NOT_TOP_ABILITY`（内部） | `0x500001` | `ability_manager_errors.h` | :1310 |
| `ERROR_CODE_NOT_TOP_ABILITY`（SDK） | `16000053` | `ability_business_error.h` | :103 |

内部使用 `0x500001`，在 SDK 边界映射为 `16000053`。spec 中 AC 使用 SDK 公开值 `16000053`。

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

`ExtensionAbilityType::AGENT` 在代码库中有 116+ 处引用，`agent_runtime_framework/` 目录有完整的 Agent 管理服务。`CheckPermissionForAgentExtension` 是 AgentExtension 连接的核心拦截点，从 `ConnectAbilityCommon`（:6070）的 :6486 调用。

## 用户故事

### US-1: 系统应用后台连接 AgentExtension

**作为** 系统应用开发者
**我想要** 在后台状态下连接 AgentExtension
**以便** 系统应用后台业务流程不受阻断

### US-2: 未授权应用仍被拦截

**作为** 安全审查者
**我想要** 未授权应用仍被拦截
**以便** 后台拉起权限不扩大攻击面

### US-3: 前台行为不受影响

**作为** 系统应用开发者
**我想要** 前台系统应用连接 AgentExtension 行为不变
**以便** 现有前台路径无回归

### US-4: 后台无性能劣化

**作为** 系统应用最终用户
**我想要** 后台功能正常工作且无明显延迟
**以便** 用户体验不受损

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 权限定义文件注册到权限框架 THEN `availableLevel=system_basic`，可通过 ACL 授权 | 正常 | US-1 |
| AC-2 | WHEN 后台系统应用持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限调用 `ConnectAbilityCommon` 连接 AgentExtension THEN 返回 `ERR_OK`（0），跳过前台拦截 | 正常 | US-1 |
| AC-3 | WHEN 后台系统应用未持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 THEN 返回 `16000053`（NOT_TOP_ABILITY），进程不被拉起 | 异常 | US-2 |
| AC-4 | WHEN 前台系统应用（无论是否持新权限）调用 `ConnectAbilityCommon` THEN 返回 `ERR_OK`，行为不变 | 兼容 | US-3 |
| AC-5 | WHEN 后台非系统应用调用 THEN 返回 `16000053` 或 `ERR_NOT_SYSTEM_APP`（取决于 `JudgeCallerIsAllowedToUseSystemAPI` 校验先后） | 异常 | US-2 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 权限配置文件审查 | security_permission 权限定义文件 |
| AC-2 | R-2 | 单元测试 | `ability_manager_service_test/`（持有权限 + 后台） |
| AC-3 | R-3 | 单元测试 | `ability_manager_service_test/`（无权限 + 后台） |
| AC-4 | R-4 | 回归测试 | `ability_manager_service_test/`（前台场景） |
| AC-5 | R-3 | 单元测试 | `ability_manager_service_test/`（非系统应用 + 后台） |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 权限定义注册 | `CONNECT_AGENT_FROM_BACKGROUND` 注册为 `system_basic` 级别 | 仅系统应用可通过 ACL 获得该权限 | AC-1 |
| R-2 | 行为 | 后台进程 + 持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 跳过 `APP_STATE_FOREGROUND` 检查，继续执行 `ConnectAbilityCommon` | 在 `CheckPermissionForAgentExtension` :13134 前增加分支 | AC-2 |
| R-3 | 异常 | 后台进程 + 未持有 `CONNECT_AGENT_FROM_BACKGROUND` 权限 | 返回 `NOT_TOP_ABILITY`（`0x500001` / SDK `16000053`），不执行连接 | 与现有行为一致 | AC-3, AC-5 |
| R-4 | 兼容 | 前台进程调用 | 行为不变，新权限检查分支不影响前台路径 | 前台路径 `APP_STATE_FOREGROUND` 检查先于新权限分支或并行 | AC-4 |

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
| 修改位置 | `CheckPermissionForAgentExtension` :13134 前插入新分支 |
| 修改内容 | 在 `APP_STATE_FOREGROUND` 检查前增加 `VerifyCallingPermission(PERMISSION_CONNECT_AGENT_FROM_BACKGROUND)` 分支：持有权限则跳过前台拦截 |
| 修改量 | ~8 行 |
| 复用函数 | `PermissionVerification::VerifyCallingPermission`（无修改） |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 后台 + 系统应用 + 有权限 | ERR_OK，连接成功 | AC-2 |
| 2 | 后台 + 系统应用 + 无权限 | 16000053，拦截 | AC-3 |
| 3 | 前台 + 系统应用（无论权限） | ERR_OK，行为不变 | AC-4 |
| 4 | 后台 + 非系统应用 | NOT_SYSTEM_APP 或 16000053 | AC-5 |
| 5 | 前台 + 非系统应用 | NOT_SYSTEM_APP（:13116-13119 校验） | — |

## 兼容性声明

- **已有 API 行为变更:** 否。新增权限为 opt-in，原有前台路径不受影响。未持有新权限的后台行为与当前一致。
- **IPC 序列化格式变更:** 否。权限校验在 AMS 进程内完成，不涉及 IPC 协议变更。
- **配置文件格式变更:** 是（新增权限定义文件），但不修改已有权限定义。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 权限限定 system_basic | 收敛攻击面，仅系统应用可通过 ACL 获得该权限 | AC-1, AC-3 |
| 新权限检查为追加分支 | 不修改 `PERMISSION_CONNECT_AGENT` 检查逻辑和前台路径 | AC-4 |
| 复用 VerifyCallingPermission | 不新增权限校验函数 | 全部 |
| 校验在 AMS 进程内完成 | 不涉及 IPC，无跨进程协议变更 | 全部 |

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
| 多用户 | 否 | 权限校验按调用方 Token | — |
| 版本升级 | 否 | 无持久化数据 | — |
| 生态兼容 | 是 | 新增权限为 opt-in，未授权行为与当前一致 | AC-3 |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅新增权限 + 拦截豁免；AgentExtension 生命周期/通用后台路径不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
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
```

**关键文档：**
- `services/abilitymgr/src/ability_manager_service.cpp`（`CheckPermissionForAgentExtension` :13099-13139，`ConnectAbilityCommon` :6070）
- `services/common/include/permission_constants.h`（`PERMISSION_CONNECT_AGENT` :25，权限常量模式）
- `services/common/include/permission_verification.h`（`VerifyCallingPermission` :46）
- `services/common/src/permission_verification.cpp`（:59-75）
- `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`（`NOT_TOP_ABILITY` = `0x500001`，:1310）
- `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（`ERROR_CODE_NOT_TOP_ABILITY` = `16000053`，:103）
- `services/abilitymgr/include/ability_manager_service.h`（`CheckPermissionForAgentExtension` 声明，:3715）
