# SDK 接口设计：后台连接 AgentExtension 权限

> 基于 `spec.md`（REQ-CONNECT-AGENT-BG-001）设计。本特性不新增 API 方法，仅对现有 `connectAgentExtensionAbility` 新增 `@since 27.0.0` 版本块，体现 `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` 权限引入后的行为变更。

## 1. 变更概述

| 属性 | 值 |
|------|-----|
| 变更类型 | MODIFIED（现有 API 新增版本块） |
| 目标文件 | `api/@ohos.app.agent.agentManager.d.ts` |
| 目标方法 | `connectAgentExtensionAbility(want, agentId, callback)` |
| 新增权限 | `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND`（`system_basic`，仅系统应用 ACL 授权） |
| 新增版本 | `@since 27.0.0 dynamic&static` |

## 2. 变更内容

### 2.1 现有版本块保持不变

`@since 24` 和 `@since 26.0.0` 两个 JSDoc 块**不修改**，保持原始行为描述：

| 版本块 | 16000053 描述 | 后台权限说明 |
|--------|--------------|-------------|
| `@since 24` | "The ability is not on the top of the UI." | 无 |
| `@since 26.0.0` | 同上 | 无 |

### 2.2 新增 `@since 27.0.0` 版本块

在 `@since 26.0.0` 块之后、函数声明之前，新增第三个 JSDoc 块：

```typescript
  /**
   * Connects to an AgentExtensionAbility.
   *
   * If the caller is in the background, you need to apply for
   * permission:ohos.permission.CONNECT_AGENT_FROM_BACKGROUND to connect to the AgentExtensionAbility.
   * Otherwise, error 16000053 is returned. This permission is available only for system applications.
   *
   * @permission ohos.permission.CONNECT_AGENT
   * @param { Want } want - Indicates the want info to connect.
   * @param { string } agentId - The agent id to connect.
   * @param { AgentExtensionConnectCallback } callback - The callback of connection.
   * @returns { Promise<AgentProxy> } The promise to get AgentProxy.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 16000001 - The specified ability does not exist.
   * @throws { BusinessError } 16000002 - Incorrect ability type.
   * @throws { BusinessError } 16000004 - Cannot start an invisible component.
   * @throws { BusinessError } 16000005 - The specified process does not have the permission.
   * @throws { BusinessError } 16000008 - The crowdtesting application expires.
   * @throws { BusinessError } 16000012 - The application is controlled.
   * @throws { BusinessError } 16000013 - The application is controlled by enterprise device management (EDM).
   * @throws { BusinessError } 16000050 - Internal error. Possible causes: 1.Connect to system service failed.
   * 2.System service failed to communicate with dependency module.
   * @throws { BusinessError } 16000053 - The ability is not on the top of the UI. This error is returned when the
   * caller is in the background and does not hold the permission
   * ohos.permission.CONNECT_AGENT_FROM_BACKGROUND.
   * @throws { BusinessError } 16000055 - Installation-free timed out.
   * @throws { BusinessError } 16000073 - The app clone index is invalid.
   * @throws { BusinessError } 35600001 - The specified agentId does not exist.
   * @throws { BusinessError } 35600003 - Maximum connections from the same caller have been reached.
   * Please disconnect at least one agent extension beforehand.
   * @throws { BusinessError } 35600007 - The specified LOW_CODE agent is already active and is not yet completed.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 27.0.0 dynamic&static
   */
  function connectAgentExtensionAbility(want: Want, agentId: string,
    callback: AgentExtensionConnectCallback): Promise<AgentProxy>;
```

### 2.3 三个版本块对比

| 维度 | `@since 24` | `@since 26.0.0` | `@since 27.0.0`（新增） |
|------|-----------|----------------|----------------------|
| 必需权限 | `CONNECT_AGENT` | `CONNECT_AGENT` | `CONNECT_AGENT` |
| 可选后台权限 | 无 | 无 | `CONNECT_AGENT_FROM_BACKGROUND` |
| 16000053 描述 | "The ability is not on the top of the UI." | 同左 | "...returned when the caller is in the background and does not hold the permission ohos.permission.CONNECT_AGENT_FROM_BACKGROUND." |
| 描述文本新增 | 无 | 无 | 后台连接权限说明 |
| 新增 throws | — | 16000055, 35600007 | —（与 26.0.0 相同的错误码列表） |

## 3. 权限定义

| 属性 | 值 |
|------|-----|
| 权限名 | `ohos.permission.CONNECT_AGENT_FROM_BACKGROUND` |
| 级别 | `system_basic` |
| 授权方式 | ACL（仅系统应用可获得） |
| 作用 | 允许后台进程跳过 `APP_STATE_FOREGROUND` 前台拦截，连接 AgentExtensionAbility |
| 生效条件 | 调用方为系统应用 + 持有 `CONNECT_AGENT`（必需）+ 持有 `CONNECT_AGENT_FROM_BACKGROUND`（后台时必需） |
| 不持有时行为 | 后台调用返回 `16000053`（NOT_TOP_ABILITY），前台调用不受影响 |

## 4. 设计说明

| 设计决策 | 说明 | 关联 spec AC |
|---------|------|-------------|
| 不新增 `@permission` 标签 | `CONNECT_AGENT_FROM_BACKGROUND` 是**可选**权限（仅后台需要），不是必需权限。`@permission` 仅声明必需权限 `CONNECT_AGENT`。可选权限在描述文本中说明，参照同文件 `connectServiceExtensionAbility` 的 `START_INVISIBLE_ABILITY` / `DISTRIBUTED_DATASYNC` 写法 | AC-2, AC-3 |
| 新增版本块而非修改已有块 | 权限在 27.0.0 引入，不应回溯影响 24/26.0.0 的行为描述。每个 `@since` 块代表该版本的完整 API 行为状态 | AC-4 |
| 16000053 描述细化 | 旧版本描述 "not on the top of the UI" 不够精确。新版本明确说明触发条件：后台 + 无后台权限 | AC-2, AC-3 |
| 前台行为不变 | `@since 27.0.0` 块中前台调用仍返回 `ERR_OK`，与旧版本一致。新权限检查是**追加分支**，不替代前台检查 | AC-4 |

## 5. 文件变更清单

| # | 文件路径 | 操作 | 说明 |
|---|---------|------|------|
| 1 | `interface_sdk-js/api/@ohos.app.agent.agentManager.d.ts` | 修改 | `connectAgentExtensionAbility` 新增 `@since 27.0.0` JSDoc 块（第三个重载注释），不修改已有 `@since 24` / `@since 26.0.0` 块 |

> **注意：** 此文件变更待 spec 进入开发阶段后，统一在 `interface_sdk-js` 仓库提交。当前仅作为设计文档记录。
