---
target_release:
  id: OpenHarmony-7.0
  status: proposed
---

# 需求文档

> 一份文档，从原始需求到基线结论。本需求依据 `cli_tool_framework` 中工具查询、会话查询与权限批量查询的既有实现反推并固化，待需求方/Owner 确认后基线。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-014 |
| 需求名称 | CLI 工具查询（cli-query）System API 与 Inner API |
| 来源 | 待确认（实现已存在于 cli_tool_framework，2026 版权） |
| 提出人 | 待确认 |
| 目标发行版本 | OpenHarmony-7.0 |
| 候选 Profile | none |
| 优先级 | P1 |
| 状态 | Clarifying |

### 原始描述

**原始问题：** 系统应用和系统服务需要查询系统中已注册的 CLI 工具元数据（名称、版本、权限、输入/输出 Schema、子命令）、查询 CLI 工具执行会话的运行状态，以及批量查询命令所需的权限列表，以便在调用执行接口前完成权限预检、工具发现与会话管理，而不应直接读取 KVStore 或绕过权限校验。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 系统应用开发者 | 无统一查询入口获取已注册工具列表与详情 | 集成成本高、依赖内部实现 |
| 系统服务（SA）开发者 | 缺少批量权限查询接口，无法在一次 IPC 中完成多命令权限预检 | 多次 IPC 往返、延迟高 |
| 系统安全/审计方 | 工具元数据查询与权限查询缺少统一入口和权限控制 | 安全边界模糊、审计困难 |

**期望结果：** 系统应用通过 System API 查询工具摘要、工具详情和会话状态；系统服务通过 Inner API 批量查询命令权限。系统统一完成权限校验与 KVStore 数据访问，确保查询入口受控。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码实现 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | NAPI 入口 `queryToolSummaries`/`getToolInfoByName`/`queryTools`/`querySession` 已实现 |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 服务端 `GetAllToolSummaries`/`GetToolInfoByName`/`GetAllToolInfos`/`QuerySession`/`BatchQueryPermissionBySubCommand` 已实现 |
| 源码实现 | `cli_tool_framework/services/climgr/src/permission_query_util.cpp` | 批量权限查询工具类已实现 |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp` | KVStore 工具注册表已实现 |
| IPC 接口 | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 18 个 IPC 方法含 5 个查询方法 |
| 进程配置 | `cli_tool_framework/etc/profile/aimgr.cfg` | SA 186（进程 aimgr，ondemand） |

### 初始范围

**可能包含：**
- `queryToolSummaries` System API（轻量工具列表）
- `getToolInfoByName` System API（按名称获取工具详情）
- `queryTools` System API（全量工具信息，raw data 优化）
- `querySession` System API（按 sessionId 查询会话状态）
- `BatchQueryPermissionBySubCommand` Inner API（SA-only 批量权限查询）
- 权限校验（系统应用 + `ohos.permission.QUERY_CLI_TOOL` / `ohos.permission.EXEC_CLI_TOOL`）
- 错误码 35700000–35700011

**明确不包含：**
- `execTool`/`execCmd`（工具执行，属 issue-15383/15382）
- `SubscribeSession`/`UnsubscribeSession`/`ClearSession`/`SendMessage`（会话交互，属 issue-15382）
- `RegisterFunction`/`BatchRegisterFunctions`/`GetAllFunctions` 等函数管理接口
- 工具注册/数据管理的内部实现（CliToolDataManager 内部）
- 跨设备工具同步

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| 工具查询须为系统应用 + `QUERY_CLI_TOOL` 权限 | 技术/安全 | 源码 `GetAllToolSummaries`/`GetToolInfoByName`/`GetAllToolInfos` | 已验证 |
| 会话查询须为系统应用 + `EXEC_CLI_TOOL` 权限 + 调用方 PID 与会话创建者匹配 | 技术/安全 | 源码 `QuerySession`/`IsSessionOwner` | 已验证 |
| 批量权限查询仅限 SA（TOKEN_NATIVE）调用 | 技术/安全 | 源码 `BatchQueryPermissionBySubCommand` | 已验证 |
| 工具元数据存储在 KVStore，查询不修改数据 | 兼容性 | 源码 `CliToolDataManager` 只读方法 | 已验证 |
| 批量查询上限为 100 条命令 | 技术 | 源码 `MAX_QUERY_CMDS_SIZE = 100` | 已验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 4 个 System API + 1 个 Inner API，行为分支有限但涉及 KVStore 读取与权限模型 |
| 涉及仓数量 | 1 | `foundation/ability/ability_runtime` |
| 是否涉及 Public/System API | 是 | 新增 4 个 System API + 1 个 Inner API |
| 是否涉及安全/性能关键路径 | 是 | 权限校验、KVStore 读取、会话所有权校验 |
| 是否跨 SIG | 否 | 全部在 Sig-Ability 内 |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [ ] 需求来源和责任人已明确（待确认）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定

---

## 二、澄清记录

> 本需求无原始对话记录，澄清项依据实现反推。标记「已澄清（实现反推）」的项需 Owner 复核确认。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 工具查询是否仅限系统应用？ | 决定 API 开放范围与权限模型 | 已澄清（实现反推：仅系统应用） |
| Q-2 | 会话查询权限是 `EXEC_CLI_TOOL` 还是 `QUERY_CLI_TOOL`？ | 决定权限模型一致性 | 已澄清（实现反推：`EXEC_CLI_TOOL`） |
| Q-3 | 批量权限查询是否仅限 SA？ | 决定 Inner API 开放范围 | 已澄清（实现反推：仅 TOKEN_NATIVE） |
| Q-4 | 批量查询上限是否固定 100？ | 决定边界约束 | 已澄清（实现反推：MAX_QUERY_CMDS_SIZE=100） |
| Q-5 | 会话查询是否校验调用方 PID 归属？ | 决定安全边界 | 已澄清（实现反推：IsSessionOwner 校验 callerPid） |
| Q-6 | 目标发行版本？ | 基线必需 | 待确认 |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-21 | AI 反推 | 查询权限模型 | 工具查询=QUERY_CLI_TOOL；会话查询=EXEC_CLI_TOOL+PID归属；批量权限=TOKEN_NATIVE+QUERY_CLI_TOOL | Owner 复核 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | queryToolSummaries/getToolInfoByName/queryTools/querySession + BatchQueryPermissionBySubCommand | 待确认 | 已确认（反推）/待确认 |
| 明确不包含哪些？ | execTool/execCmd/会话交互/函数管理/工具注册 | 待确认 | 已确认（反推）/待确认 |
| 是否有分期策略？ | 否 | 待确认 | 已确认（反推） |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | SA（186/aimgr）统一承载：NAPI→IPC→服务端做权限/KVStore 读取 | 集中权限审计、统一数据访问、可按需拉起 | 跨进程 IPC 开销 | 推荐 |
| A-2 | 应用进程内直接读 KVStore | 无 IPC 开销 | 绕过权限审计、KVStore 沙箱不可达 | 放弃 |

**取舍理由：** 工具元数据存储在 aimgr 进程的 KVStore，应用进程沙箱不可直接访问；集中到 SA 统一安全边界与数据访问控制，IPC 开销可接受。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 `js_cli_manager.cpp` | `OnQueryToolSummaries`/`OnGetToolInfoByName`/`OnQueryTools`/`OnQuerySession` | NAPI 参数解析与异步回调派发；getToolInfoByName/querySession 需非空参数 | 高 | 范围/API | 命中 |
| K-2 | 源码 `cli_tool_manager_service.cpp` | `GetAllToolSummaries`/`GetToolInfoByName`/`GetAllToolInfos`/`QuerySession`/`BatchQueryPermissionBySubCommand` | 权限链：系统应用+QUERY_CLI_TOOL（工具查询）；系统应用+EXEC_CLI_TOOL+IsSessionOwner（会话查询）；TOKEN_NATIVE+QUERY_CLI_TOOL（批量权限查询） | 高 | 设计/测试 | 命中 |
| K-3 | 源码 `permission_query_util.cpp` | `BatchQueryPermissions`/`QuerySingleCommand`/`QueryMainCommandPermission`/`QuerySubCommandPermission` | 逐命令查询；主命令取 toolInfo.requirePermissions；子命令取 subcommands[subCommand].requirePermissions；QueryResult 枚举 | 高 | 设计/测试 | 命中 |
| K-4 | 源码 `cli_tool_data_manager.h` | `GetAllTools`/`GetAllToolsRawData`/`GetToolByName`/`QueryToolSummaries` | KVStore 只读查询；raw data 共享内存优化 | 高 | 设计 | 命中 |
| K-5 | `cli_error_code.h` | 错误码枚举 | 35700000–35700018 区间 | 高 | API | 命中 |
| K-6 | `ICliToolManager.idl` | IPC 接口签名 | 5 个查询方法签名 | 高 | 设计 | 命中 |
| K-7 | `ICliToolData.idl` | Command/CommandPermission 结构体 | toolName+subCommand→permissions+isLockScreenExecutionAllowed+queryRet | 高 | 设计 | 命中 |
| K-8 | `AGENTS.md` | 目标仓 Agent 指南 | 服务层与 SDK 层解耦、PermissionVerification、hisysevent 硬约束 | 高 | 设计约束 | 命中 |

**上下文结论：**
- 高可信结论：查询行为链路、参数契约、权限模型、错误码、SA/进程模型均已由源码固化，可直接进入基线与设计。
- 待确认结论：目标发行版本、需求来源与责任人。
- 未使用来源及原因：未使用多仓知识库（本需求单仓内闭环）。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | ability（ability_runtime） | 待确认 | 已确认（反推） |
| 是否需要新增子系统或部件？ | 否 | 待确认 | 已确认（反推） |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 待确认 | 已确认（反推） |
| 是否需要新增 System API？ | 是，4 个（queryToolSummaries/getToolInfoByName/queryTools/querySession） | 待确认 | 已确认（反推） |
| 是否需要新增 Inner API？ | 是，1 个（BatchQueryPermissionBySubCommand） | 待确认 | 已确认（反推） |
| 是否会废弃已有 API？ | 否 | 待确认 | 已确认（反推） |
| 是否需要新增权限声明？ | 是，`ohos.permission.QUERY_CLI_TOOL` | 待确认 | 已确认（反推） |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？破坏性变更？ | 新增 API，无破坏性 | 待确认 | 已确认（反推） |
| 性能 | 响应时间/内存/并发要求？ | KVStore 读取延迟受数据库影响；queryTools 用 raw data 减少序列化开销 | 待确认 | 已确认（反推） |
| 安全 | 权限/隐私/审计要求？ | 系统应用+权限校验；会话查询+PID归属；批量查询仅SA | 待确认 | 已确认（反推） |
| 可靠性 | 崩溃率/容错/恢复要求？ | SA 不可用回 35700000；KVStore 未就绪回 35700016 | 待确认 | 已确认（反推） |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| `safwk`/`samgr` | 编译/运行 | SA 186 按需注册与拉起 | 已确认 |
| `access_token` | 运行 | 系统应用判定与权限校验 | 已确认 |
| `kv_store` | 运行 | 工具元数据存储与查询 | 已确认 |
| `bundle_framework` | 运行 | HAP 判定（QuerySession 所有权校验） | 已确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| 需求来源/Owner 未确认 | 进度 | 基线无法冻结 | 需求方/SIG 确认 | 待确认 |
| 目标版本未确认 | 进度 | 影响 @since 与发布 | proposal.target_release 确认 | 待确认 |
| KVStore 未初始化时查询失败 | 可靠性 | 返回 ERR_KVSTORE_NOT_READY/ERR_NO_INIT | 懒加载 EnsureToolsLoaded | 已确认 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（A-1/A-2 + 取舍理由）

**结论:** 条件通过（实现反推完整，待 Owner/需求方确认后转「通过」）

---

## 三、需求基线

> 澄清完成后固化。manifest.md 是事实源，此处为审批结论。

### 基线信息

| 字段 | 内容 |
|------|-----|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-21 |
| Owner | 待确认 |
| 确认人 | 需求方/模块Owner/SIG代表（待确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.0 |
| 版本状态 | proposed |

### 问题陈述

系统应用和系统服务缺少统一的 CLI 工具元数据查询、会话状态查询与批量权限预检入口。本需求新增 4 个 System API 和 1 个 Inner API，由 SA 186（aimgr）统一承载权限校验与 KVStore 数据访问。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 工具元数据可查询 | 合法调用返回工具摘要/详情/全量列表 | 单测 + 集成 |
| 会话状态可查询 | 合法调用返回会话状态与结果 | 单测 |
| 批量权限可预检 | SA 调用返回每命令权限列表与查询结果码 | 单测 |
| 权限/安全边界生效 | 非系统应用/缺权限/非会话所有者/非SA 均被拒绝 | 单测 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为系统应用开发者，我想要查询所有已注册 CLI 工具的摘要列表，以便在 UI 中展示可用工具 | P1 |
| US-2 | 作为系统应用开发者，我想要按名称获取指定 CLI 工具的完整信息，以便了解工具的参数 Schema 和权限要求 | P1 |
| US-3 | 作为系统应用开发者，我想要获取所有已注册 CLI 工具的完整信息，以便批量加载工具元数据 | P2 |
| US-4 | 作为系统应用开发者，我想要查询 CLI 工具执行会话的状态，以便管理正在运行或已完成的会话 | P1 |
| US-5 | 作为系统服务开发者，我想要批量查询命令所需权限，以便在执行前完成权限预检 | P1 |

> AC 共 22 条（AC-1.1–AC-5.4），详见 `spec.md`。此处不重复摘录，避免与 spec 不一致。

### 范围边界

**包含：** 5 个查询接口的行为契约、参数校验、权限模型、错误码。
**不包含：** execTool/execCmd、会话交互（Subscribe/Clear/Send）、函数管理、工具注册数据管理。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | foundation/ability/ability_runtime | cli_tool_framework/frameworks/js/napi/cli_tool_manager | NAPI 绑定 | 新增 API（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/services/climgr | 服务端实现 | 新增（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/interfaces/cli_tool | IPC 接口与数据结构 | 新增（既有实现固化契约） | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/etc/profile | SA 配置 | 不变（已注册） | — |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| `cliTool.queryToolSummaries` | 新增 | System | 查询所有已注册工具的摘要列表，返回 Promise<ToolSummary[]> |
| `cliTool.getToolInfoByName` | 新增 | System | 按名称获取工具完整信息，返回 Promise<ToolInfo> |
| `cliTool.queryTools` | 新增 | System | 获取所有工具完整信息，返回 Promise<ToolInfo[]> |
| `cliTool.querySession` | 新增 | System | 按会话 ID 查询会话状态，返回 Promise<CliSessionInfo> |
| `BatchQueryPermissionBySubCommand` | 新增 | Inner API (SA-only) | 批量查询命令权限列表，返回 CommandPermission[] |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | 无公开 P 指标；KVStore 读取延迟属内部实现 | — |
| 安全与权限 | 是 | 系统应用+QUERY_CLI_TOOL/EXEC_CLI_TOOL 权限、会话PID归属、SA-only 批量查询 | design.md / spec.md |
| 兼容性 | 否 | 新增 API，无破坏性 | spec.md |
| API/SDK | 是 | 新增 4 System API + 1 Inner API | design.md / spec.md |
| IPC/跨进程 | 是 | NAPI→IPC→SA 186 | design.md |
| 构建与部件 | 否 | 复用既有 component/parts，无新增部件 | — |
| 国际化/无障碍 | 否 | 无界面 API | — |
| 数据迁移 | 否 | 工具元数据 KVStore 格式不变 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 System/Inner API | 触发设计审批 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [ ] `proposal.target_release` 已确认或明确 TBD（待确认）
- [x] `manifest.profile` 已确认或明确 none
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭，且讨论记录包含需求方/Owner/SIG 明确确认（实现反推，待 Owner 复核）
- [x] 上下文与知识源检索日志已填写；未查询关键来源的原因已记录
- [x] 目标仓 Agent 指南已检查并记录关键约束（AGENTS.md 服务/SDK 解耦、PermissionVerification、hisysevent 硬约束）

**基线结论:** 条件通过
