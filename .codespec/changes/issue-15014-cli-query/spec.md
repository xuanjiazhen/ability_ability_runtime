# 特性规格

> 本规格仅覆盖 CLI 工具查询接口行为。范围限定：`queryToolSummaries`/`getToolInfoByName`/`queryTools`/`querySession` 4 个 System API 与 `BatchQueryPermissionBySubCommand` 1 个 Inner API 的用户可见行为、参数契约、错误码与权限模型。`execTool`/`execCmd`/`SubscribeSession`/`SendMessage`/`RegisterFunction` 等同模块其他接口不在本规格范围内。
>
> **输入文档说明：** 本规格依据 `cli_tool_framework` 中查询接口的既有实现（`frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`、`services/climgr/src/cli_tool_manager_service.cpp`、`services/climgr/src/permission_query_util.cpp`、`interfaces/cli_tool/`）反推固化。待 `proposal.md` 基线化后，以下「优先级/目标版本/SIG 归属」字段应回填 proposal 值。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | CLI 工具查询（cli-query） |
| 特性编号 | FEAT-CLI-014 |
| 所属 Epic | REQ-014 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md（待基线） |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。查询接口为 `cli_tool_framework` 对外暴露的 System API 与 Inner API，此前不存在对外契约。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `queryToolSummaries` System API（JS） | 查询所有已注册工具的摘要列表 |
| ADDED | `getToolInfoByName` System API（JS） | 按名称获取工具完整信息 |
| ADDED | `queryTools` System API（JS） | 获取所有工具完整信息（raw data 优化） |
| ADDED | `querySession` System API（JS） | 按会话 ID 查询会话状态 |
| ADDED | `BatchQueryPermissionBySubCommand` Inner API | SA-only 批量查询命令权限 |
| ADDED | 权限 `ohos.permission.QUERY_CLI_TOOL` | 工具查询与批量权限查询调用前置权限 |
| ADDED | 数据结构 `ToolSummary`/`ToolInfo`/`SubCommandInfo`/`CliSessionInfo`/`Command`/`CommandPermission` | 对外行为契约，非内部实现 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `REQ-014`（未在工作区检索到） | Pending |
| Proposal | `proposal.md`（本 change 目录） | Conditional Pass |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。design.md 与本文档并行产出，互不依赖。

## 用户故事

### US-1: 查询所有已注册工具摘要

**作为** 系统应用开发者,
**我想要** 通过 `queryToolSummaries` 接口获取所有已注册 CLI 工具的摘要列表,
**以便** 在 UI 中展示可用工具列表。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN 系统应用持 `ohos.permission.QUERY_CLI_TOOL` 调用 `queryToolSummaries` THEN 接口返回 Promise，resolve 为 `ToolSummary[]`（每项含 `name`/`version`/`description`），列表内容与 KVStore 注册一致 | 正常 |
| AC-1.2 | WHEN 系统中无已注册工具 THEN resolve 为空数组 `[]` | 边界 |
| AC-1.3 | WHEN 非系统应用调用 `queryToolSummaries` THEN Promise reject 错误码 `35700008`（ERR_NOT_SYSTEM_APP） | 异常 |
| AC-1.4 | WHEN 系统应用未持有 `ohos.permission.QUERY_CLI_TOOL` THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |
| AC-1.5 | WHEN CliToolManager 系统能力未就绪或代理获取失败 THEN Promise reject 错误码 `35700000`（GET_CLI_TOOL_MGR_SERVICE_FAILED） | 恢复 |

### US-2: 按名称获取工具完整信息

**作为** 系统应用开发者,
**我想要** 通过 `getToolInfoByName` 接口按名称获取指定 CLI 工具的完整信息,
**以便** 了解工具的参数 Schema、权限要求和子命令。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-2.1 | WHEN 系统应用持 `ohos.permission.QUERY_CLI_TOOL` 调用 `getToolInfoByName`，传入已注册的 `toolName` THEN resolve 为 `ToolInfo`（含 `name`/`version`/`description`/`executablePath`/`requirePermissions`/`inputSchema`/`outputSchema`/`subcommands`/`isLockScreenExecutionAllowed`） | 正常 |
| AC-2.2 | WHEN `toolName` 为空字符串/非字符串/未传入 THEN 同步抛出 invalid parameter 异常，不进入异步链路 | 边界 |
| AC-2.3 | WHEN `toolName` 未在系统中注册 THEN Promise reject 错误码 `35700005`（ERR_TOOL_NOT_EXIST） | 异常 |
| AC-2.4 | WHEN 非系统应用调用 THEN Promise reject 错误码 `35700008`（ERR_NOT_SYSTEM_APP） | 异常 |
| AC-2.5 | WHEN 系统应用未持有 `ohos.permission.QUERY_CLI_TOOL` THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |

### US-3: 获取所有工具完整信息

**作为** 系统应用开发者,
**我想要** 通过 `queryTools` 接口获取所有已注册 CLI 工具的完整信息列表,
**以便** 批量加载工具元数据用于本地决策。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-3.1 | WHEN 系统应用持 `ohos.permission.QUERY_CLI_TOOL` 调用 `queryTools` THEN resolve 为 `ToolInfo[]`，每项含完整字段，列表与 KVStore 注册一致 | 正常 |
| AC-3.2 | WHEN 系统中无已注册工具 THEN resolve 为空数组 `[]` | 边界 |
| AC-3.3 | WHEN 非系统应用调用 THEN Promise reject 错误码 `35700008`（ERR_NOT_SYSTEM_APP） | 异常 |
| AC-3.4 | WHEN 系统应用未持有 `ohos.permission.QUERY_CLI_TOOL` THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |

### US-4: 查询会话状态

**作为** 系统应用开发者,
**我想要** 通过 `querySession` 接口按会话 ID 查询 CLI 工具执行会话的当前状态,
**以便** 管理正在运行或已完成的会话。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-4.1 | WHEN 系统应用持 `ohos.permission.EXEC_CLI_TOOL` 调用 `querySession`，传入由本应用创建的 `sessionId` THEN resolve 为 `CliSessionInfo`（含 `sessionId`/`toolName`/`status`；`status="running"` 时 `result` 为 undefined；`status="completed"`/`"failed"` 时含 `result`） | 正常 |
| AC-4.2 | WHEN `sessionId` 为空字符串/非字符串/未传入 THEN 同步抛出 invalid parameter 异常，不进入异步链路 | 边界 |
| AC-4.3 | WHEN `sessionId` 不存在或会话已清理 THEN Promise reject 错误码 `35700009`（ERR_CLI_SESSION_NOT_FOUND） | 异常 |
| AC-4.4 | WHEN `sessionId` 存在但调用方 PID 与会话创建者 PID 不匹配 THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |
| AC-4.5 | WHEN 非系统应用调用 THEN Promise reject 错误码 `35700008`（ERR_NOT_SYSTEM_APP） | 异常 |
| AC-4.6 | WHEN 系统应用未持有 `ohos.permission.EXEC_CLI_TOOL` THEN Promise reject 错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |

### US-5: 批量查询命令权限

**作为** 系统服务开发者,
**我想要** 通过 `BatchQueryPermissionBySubCommand` Inner API 批量查询命令所需权限,
**以便** 在执行前完成权限预检。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-5.1 | WHEN SA（TOKEN_NATIVE）持 `ohos.permission.QUERY_CLI_TOOL` 调用 `BatchQueryPermissionBySubCommand`，传入非空命令列表（≤100 条），每条含已注册 `toolName` THEN 返回 `CommandPermission[]`，每项含 `cmd`/`permissions`/`isLockScreenExecutionAllowed`/`queryRet`（queryRet=0 表示成功） | 正常 |
| AC-5.2 | WHEN 命令列表中某条 `toolName` 未注册 THEN 该条 `queryRet=1`（COMMAND_NOT_EXIST），`permissions` 为空列表，其余条目正常返回 | 异常 |
| AC-5.3 | WHEN 命令含 `subCommand` 且工具未声明子命令或子命令不存在 THEN 该条 `queryRet=1`（COMMAND_NOT_EXIST） | 异常 |
| AC-5.4 | WHEN 命令列表为空或大小超过 100 THEN 返回错误码 `35700002`（ERR_INVALID_PARAM） | 边界 |
| AC-5.5 | WHEN 调用方非 SA（非 TOKEN_NATIVE）THEN 返回错误码 `35700011`（ERR_NOT_SA_CALLER） | 异常 |
| AC-5.6 | WHEN SA 未持有 `ohos.permission.QUERY_CLI_TOOL` THEN 返回错误码 `35700007`（ERR_PERMISSION_DENIED） | 异常 |

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1 | TASK-Q-001 | 单测 | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` |
| AC-1.2 | R-1 | TASK-Q-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-1.3 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.4 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-1.5 | R-12 | TASK-Q-006 | 单测 | `cli_tool_mgr_client_test/` |
| AC-2.1 | R-3 | TASK-Q-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-2.2 | R-9 | TASK-Q-003 | 单测 | `cli_tool_mgr_client_test/` |
| AC-2.3 | R-4 | TASK-Q-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-2.4 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-2.5 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-3.1 | R-5 | TASK-Q-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-3.2 | R-5 | TASK-Q-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-3.3 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-3.4 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-4.1 | R-6 | TASK-Q-004 | 单测 | `cli_tool_mgr_service_test/` |
| AC-4.2 | R-9 | TASK-Q-003 | 单测 | `cli_tool_mgr_client_test/` |
| AC-4.3 | R-7 | TASK-Q-004 | 单测 | `cli_tool_mgr_service_test/` |
| AC-4.4 | R-8 | TASK-Q-004 | 单测 | `cli_tool_mgr_service_test/` |
| AC-4.5 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-4.6 | R-2 | TASK-Q-002 | 单测 | `cli_tool_mgr_client_test/` |
| AC-5.1 | R-10 | TASK-Q-005 | 单测 | `cli_tool_mgr_service_test/` |
| AC-5.2 | R-10 | TASK-Q-005 | 单测 | `cli_tool_mgr_service_test/` |
| AC-5.3 | R-10 | TASK-Q-005 | 单测 | `cli_tool_mgr_service_test/` |
| AC-5.4 | R-11 | TASK-Q-005 | 单测 | `permission_query_util_test/` |
| AC-5.5 | R-12 | TASK-Q-005 | 单测 | `cli_tool_mgr_service_test/` |
| AC-5.6 | R-12 | TASK-Q-005 | 单测 | `cli_tool_mgr_service_test/` |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 系统应用 + QUERY_CLI_TOOL + 调用 queryToolSummaries | 从 KVStore 查询所有工具摘要，resolve `ToolSummary[]` | 摘要仅含 name/version/description | AC-1.1, AC-1.2 |
| R-2 | 异常 | 调用方 Token 非系统应用，或未持 QUERY_CLI_TOOL/EXEC_CLI_TOOL 权限 | reject：非系统应用→35700008；缺权限→35700007 | 校验在 SA 入口执行 | AC-1.3, AC-1.4, AC-2.4, AC-2.5, AC-3.3, AC-3.4, AC-4.5, AC-4.6 |
| R-3 | 行为 | 系统应用 + QUERY_CLI_TOOL + 已注册 toolName | 从 KVStore 按 name 查找工具，resolve `ToolInfo`（含完整字段） | ToolInfo 含 inputSchema/outputSchema JSON 字符串 | AC-2.1 |
| R-4 | 异常 | toolName 未在 KVStore 注册 | reject 35700005（ERR_TOOL_NOT_EXIST） | — | AC-2.3 |
| R-5 | 行为 | 系统应用 + QUERY_CLI_TOOL + 调用 queryTools | 从 KVStore 获取全部工具（raw data 优化），resolve `ToolInfo[]` | raw data 共享内存减少序列化开销 | AC-3.1, AC-3.2 |
| R-6 | 行为 | 系统应用 + EXEC_CLI_TOOL + sessionId 存在 + 调用方为会话创建者 | 返回 `CliSessionInfo`：status=running→无 result；completed/failed→含 result | 会话为运行态内存对象 | AC-4.1 |
| R-7 | 异常 | sessionId 不存在或会话已清理 | reject 35700009（ERR_CLI_SESSION_NOT_FOUND） | — | AC-4.3 |
| R-8 | 异常 | sessionId 存在但调用方 PID ≠ 会话创建者 PID | reject 35700007（ERR_PERMISSION_DENIED） | IsSessionOwner 校验 callerPid | AC-4.4 |
| R-9 | 边界 | getToolInfoByName 的 toolName 为空/非串/未传入；querySession 的 sessionId 为空/非串/未传入 | 同步抛出 invalid parameter 异常（不进入异步 Promise） | NAPI 层校验先于服务端 | AC-2.2, AC-4.2 |
| R-10 | 行为 | SA(TOKEN_NATIVE) + QUERY_CLI_TOOL + 非空命令列表(≤100) | 逐命令查 KVStore：主命令取 toolInfo.requirePermissions；子命令取 subcommands[sub].requirePermissions；成功 queryRet=0，不存在 queryRet=1，DB错 queryRet=2 | isLockScreenExecutionAllowed 取工具级配置 | AC-5.1, AC-5.2, AC-5.3 |
| R-11 | 边界 | 命令列表为空或大小 > 100 | 返回 35700002（ERR_INVALID_PARAM） | MAX_QUERY_CMDS_SIZE=100 | AC-5.4 |
| R-12 | 异常 | 批量权限查询调用方非 TOKEN_NATIVE；或 SA 缺 QUERY_CLI_TOOL 权限；或 CliToolManager SA 未就绪 | 非 SA→35700011；缺权限→35700007；SA 不可用→35700000 | 批量查询不做系统应用判定（SA 非系统应用） | AC-5.5, AC-5.6, AC-1.5 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1, R-3, R-5 / AC-1.1, AC-2.1, AC-3.1 | 单测 | 正常查询返回正确数据 |
| VM-2 | R-2 / AC-1.3–1.4, AC-2.4–2.5, AC-3.3–3.4, AC-4.5–4.6 | 单测 | 系统应用判定与权限校验路径 |
| VM-3 | R-4, R-7 / AC-2.3, AC-4.3 | 单测 | 工具不存在/会话不存在错误码 |
| VM-4 | R-6, R-8 / AC-4.1, AC-4.4 | 单测 | 会话查询正常与 PID 归属校验 |
| VM-5 | R-9 / AC-2.2, AC-4.2 | 单测 | NAPI 参数同步校验 |
| VM-6 | R-10, R-11, R-12 / AC-5.1–5.6 | 单测 | 批量权限查询全路径 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `cliTool.queryToolSummaries` | System | `()` | `Promise<ToolSummary[]>` | 35700000, 35700007, 35700008 | 查询所有工具摘要 | AC-1.1–1.5 |
| `cliTool.getToolInfoByName` | System | `(toolName: string)` | `Promise<ToolInfo>` | 35700000, 35700005, 35700007, 35700008 | 按名称获取工具详情 | AC-2.1–2.5 |
| `cliTool.queryTools` | System | `()` | `Promise<ToolInfo[]>` | 35700000, 35700007, 35700008 | 获取所有工具完整信息 | AC-3.1–3.4 |
| `cliTool.querySession` | System | `(sessionId: string)` | `Promise<CliSessionInfo>` | 35700000, 35700007, 35700008, 35700009 | 按会话 ID 查询会话状态 | AC-4.1–4.6 |
| `BatchQueryPermissionBySubCommand` | Inner API | `(cmds: Command[])` | `CommandPermission[]` + int32_t | 35700002, 35700007, 35700011 | 批量查询命令权限 | AC-5.1–5.6 |

### 变更/废弃 API

无变更或废弃项。

## 接口规格

### 接口定义

**queryToolSummaries**

| 属性 | 值 |
|------|-----|
| 函数签名 | `queryToolSummaries(): Promise<ToolSummary[]>` |
| 返回值 | `Promise<ToolSummary[]>` — resolve 为工具摘要列表；reject 为错误码 |
| 开放范围 | System |
| 权限 | `ohos.permission.QUERY_CLI_TOOL`，且调用方须为系统应用 |
| 错误码 | 35700000, 35700007, 35700008 |
| 关联 AC | AC-1.1–1.5 |

**getToolInfoByName**

| 属性 | 值 |
|------|-----|
| 函数签名 | `getToolInfoByName(toolName: string): Promise<ToolInfo>` |
| 返回值 | `Promise<ToolInfo>` — resolve 为工具完整信息；reject 为错误码 |
| 开放范围 | System |
| 权限 | `ohos.permission.QUERY_CLI_TOOL`，且调用方须为系统应用 |
| 错误码 | 35700000, 35700005, 35700007, 35700008 |
| 关联 AC | AC-2.1–2.5 |

**queryTools**

| 属性 | 值 |
|------|-----|
| 函数签名 | `queryTools(): Promise<ToolInfo[]>` |
| 返回值 | `Promise<ToolInfo[]>` — resolve 为工具完整信息列表；reject 为错误码 |
| 开放范围 | System |
| 权限 | `ohos.permission.QUERY_CLI_TOOL`，且调用方须为系统应用 |
| 错误码 | 35700000, 35700007, 35700008 |
| 关联 AC | AC-3.1–3.4 |

**querySession**

| 属性 | 值 |
|------|-----|
| 函数签名 | `querySession(sessionId: string): Promise<CliSessionInfo>` |
| 返回值 | `Promise<CliSessionInfo>` — resolve 为会话信息；reject 为错误码 |
| 开放范围 | System |
| 权限 | `ohos.permission.EXEC_CLI_TOOL`，且调用方须为系统应用且为会话创建者 |
| 错误码 | 35700000, 35700007, 35700008, 35700009 |
| 关联 AC | AC-4.1–4.6 |

**BatchQueryPermissionBySubCommand（Inner API）**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t BatchQueryPermissionBySubCommand(const std::vector<Command>& cmds, std::vector<CommandPermission>& cmdPermissions)` |
| 返回值 | `int32_t` — ERR_OK 成功；否则错误码 |
| 开放范围 | Inner API（仅 TOKEN_NATIVE 即 SA 可调） |
| 权限 | `ohos.permission.QUERY_CLI_TOOL` |
| 错误码 | 35700002, 35700007, 35700011 |
| 关联 AC | AC-5.1–5.6 |

**参数约束**

| 接口 | 参数 | 类型 | 必填 | 约束条件 |
|------|------|------|------|---------|
| queryToolSummaries | — | — | — | 无参数 |
| getToolInfoByName | toolName | string | 是 | 非空；须为已注册工具名，否则 35700005 |
| queryTools | — | — | — | 无参数 |
| querySession | sessionId | string | 是 | 非空；须为本应用创建的会话 ID |
| BatchQueryPermissionBySubCommand | cmds | Command[] | 是 | 非空且 ≤100 条；每项含 toolName（非空）+subCommand（可空） |

**数据结构**

ToolSummary:
| 字段 | 类型 | 说明 |
|------|------|------|
| name | string | 工具名 |
| version | string | 版本 |
| description | string | 描述 |

ToolInfo:
| 字段 | 类型 | 说明 |
|------|------|------|
| name | string | 工具名 |
| version | string | 版本 |
| description | string | 描述 |
| executablePath | string | 可执行文件路径 |
| requirePermissions | string[] | 所需权限列表 |
| inputSchema | string (JSON) | 输入参数 Schema |
| outputSchema | string (JSON) | 输出参数 Schema |
| eventTypes | string[] | 事件类型列表 |
| eventSchemas | string (JSON) | 事件 Schema 映射 |
| hasSubCommand | boolean | 是否有子命令 |
| subcommands | Map<string, SubCommandInfo> | 子命令映射 |
| isLockScreenExecutionAllowed | boolean | 是否允许锁屏执行 |

Command:
| 字段 | 类型 | 说明 |
|------|------|------|
| toolName | string | 工具名 |
| subCommand | string | 子命令名（可空） |

CommandPermission:
| 字段 | 类型 | 说明 |
|------|------|------|
| cmd | Command | 查询的命令 |
| permissions | string[] | 所需权限列表 |
| isLockScreenExecutionAllowed | boolean | 是否允许锁屏执行 |
| queryRet | int | 查询结果码：0=成功, 1=不存在, 2=DB错误 |

CliSessionInfo:
| 字段 | 类型 | 说明 |
|------|------|------|
| sessionId | string | 会话唯一标识 |
| toolName | string | 工具名 |
| status | string | "running" / "completed" / "failed" |
| result | ExecResult (optional) | 仅 completed/failed 时存在 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 系统应用+QUERY_CLI_TOOL 调用 queryToolSummaries | resolve ToolSummary[] | AC-1.1 |
| 2 | 无注册工具 | resolve [] | AC-1.2 |
| 3 | 系统应用+QUERY_CLI_TOOL+已注册 toolName 调用 getToolInfoByName | resolve ToolInfo | AC-2.1 |
| 4 | toolName 未注册 | reject 35700005 | AC-2.3 |
| 5 | 系统应用+QUERY_CLI_TOOL 调用 queryTools | resolve ToolInfo[] | AC-3.1 |
| 6 | 系统应用+EXEC_CLI_TOOL+自有 sessionId 调用 querySession | resolve CliSessionInfo | AC-4.1 |
| 7 | sessionId 不存在 | reject 35700009 | AC-4.3 |
| 8 | 非会话创建者 PID 调用 | reject 35700007 | AC-4.4 |
| 9 | SA+QUERY_CLI_TOOL+有效命令列表 调用 BatchQuery | 返回 CommandPermission[] | AC-5.1 |
| 10 | 命令列表含未注册工具 | 该条 queryRet=1 | AC-5.2 |
| 11 | 命令列表空或>100 | 返回 35700002 | AC-5.4 |
| 12 | 非 SA 调用 BatchQuery | 返回 35700011 | AC-5.5 |

## 兼容性声明

- **已有 API 行为变更:** 否。本特性为新增 API，不改变既有公共 API 行为。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。工具元数据 KVStore 格式不变；查询为只读操作。
- **最低支持版本:** 参见 proposal.md（待基线）。
- **API 版本号策略:** 新增 API 自首个引入版本起标注 `@since`；错误码 357000xx 为本模块专属区间。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅系统应用 + `ohos.permission.QUERY_CLI_TOOL` 可查询工具 | 入口须做系统应用判定与权限校验 | AC-1.3–1.4, AC-2.4–2.5, AC-3.3–3.4 |
| 仅系统应用 + `ohos.permission.EXEC_CLI_TOOL` + 会话创建者可查询会话 | 入口须做系统应用、权限与 PID 归属校验 | AC-4.4–4.6 |
| 仅 SA（TOKEN_NATIVE）+ `ohos.permission.QUERY_CLI_TOOL` 可批量查询权限 | 不做系统应用判定（SA 非系统应用） | AC-5.5–5.6 |
| 工具元数据存储在 aimgr 进程 KVStore | 应用进程沙箱不可直接访问，须经 SA IPC | AC-1.1–3.1 |

> 架构规则适用性及设计方案见 design.md。

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 单次查询调用不应导致 CliToolManager SA 崩溃 | 单测 | `cli_tool_mgr_service_test/` |
| 安全 | 非系统应用/缺权限/非会话所有者/非SA 调用均被拒绝 | 单测 | `cli_tool_mgr_client_test/` |
| 可测试性 | 查询行为可经 mock 服务端校验错误码与返回数据 | 单测 | `cli_tool_mgr_client_test/`、`tool_info_test/` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | — | 集成 | `cli_tool_framework/test/` |
| 平板 | 无差异 | — | 集成 | `cli_tool_framework/test/` |
| 折叠屏 | 无差异 | — | 集成 | `cli_tool_framework/test/` |

> 查询行为与设备形态无关。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 查询为无界面 API | — |
| 大字体 | N/A | — | — |
| 深色模式 | N/A | — | — |
| 多窗口/分屏 | 否 | 与窗口无关 | — |
| 多用户 | 否 | 权限按调用方 Token 校验，不依赖用户切换 | — |
| 版本升级 | 否 | KVStore 数据格式不变 | — |
| 生态兼容 | 否 | 仅系统应用/SA 可用 | — |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符（"待基线"为上游 proposal 缺失的如实标注，非规格占位）
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅查询接口；execTool/execCmd 等不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致（每个 AC 至少关联一条规则，每条规则至少关联一个 AC）
- [x] 规则表每条通过 5 项质量检查（可复现/可观测/边界值/关联AC/无冲突）

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_runtime"
    query: "cli_tool_framework 查询接口的权限校验与 KVStore 读取实现"
  - repo: "openharmony/ability_runtime"
    query: "BatchQueryPermissionBySubCommand 的 SA-only 鉴权与逐命令查询逻辑"
```

**关键文档：**
- `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`（NAPI 入口 `queryToolSummaries`/`getToolInfoByName`/`queryTools`/`querySession`）
- `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`（服务端查询方法）
- `cli_tool_framework/services/climgr/src/permission_query_util.cpp`（批量权限查询工具类）
- `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h`（错误码定义）
- `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl`（IPC 接口）
- `cli_tool_framework/interfaces/cli_tool/ICliToolData.idl`（Command/CommandPermission 结构体）
