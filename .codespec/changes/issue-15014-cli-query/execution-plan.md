# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。本计划针对 CLI 工具查询接口既有实现进行契约固化与测试覆盖验证：实现已存在，Task 聚焦「验证既有链路符合 spec + 补齐测试缺口 + 跨仓协调」。

## Plan 元数据

| 字段 | 内容 |
|------|-----|
| Plan ID | PLAN-CLI-014 |
| 关联 Feature/Bug | FEAT-CLI-014（REQ-014） |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | 待确认 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（条件通过，待 Owner 转正） |
| Design | `design.md` | Approved（条件通过，待 Owner 转正） |
| Spec | `spec.md` | Approved（Draft→待转 Approved） |

## 受影响文件全量清单

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_runtime | NAPI | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 验证/补测试 | `queryToolSummaries`/`getToolInfoByName`/`queryTools`/`querySession` 参数解析与回调 |
| ability_runtime | NAPI | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_manager.h` | 只读参考 | 声明 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 只读参考 | 5 个查询方法签名 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/ICliToolData.idl` | 只读参考 | Command/CommandPermission 结构体 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` | 验证 | 客户端查询方法声明 + BatchQueryPermissionBySubCommand |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.cpp` | 验证 | 客户端查询方法实现 |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/tool_info.h` | 验证 | ToolInfo Parcelable |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/tool_summary.h` | 验证 | ToolSummary Parcelable |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/sub_command_info.h` | 验证 | SubCommandInfo Parcelable |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_session_info.h` | 验证 | CliSessionInfo Parcelable |
| ability_runtime | InnerAPI/IPC | `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h` | 验证 | 错误码 357000xx |
| ability_runtime | 服务 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 验证 | 服务端查询方法全链路 |
| ability_runtime | 服务/数据 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp` | 验证 | KVStore 查询实现 |
| ability_runtime | 服务/数据 | `cli_tool_framework/services/climgr/include/cli_tool_data_manager.h` | 只读参考 | 数据管理器声明 |
| ability_runtime | 服务/工具 | `cli_tool_framework/services/climgr/src/permission_query_util.cpp` | 验证 | 批量权限查询逻辑 |
| ability_runtime | 服务/工具 | `cli_tool_framework/services/climgr/include/permission_query_util.h` | 只读参考 | 声明 |
| ability_runtime | 服务/公共 | `cli_tool_framework/services/common/src/permission_util.cpp` | 只读参考 | 权限校验工具 |
| ability_runtime | SA | `cli_tool_framework/etc/profile/aimgr.cfg` | 只读参考 | 进程/权限/caps |
| ability_runtime | SA | `cli_tool_framework/etc/profile/aimgr_trust.json` | 只读参考 | SA 186 注册 |
| 跨仓（权限定义仓） | 权限 | `ohos.permission.QUERY_CLI_TOOL` 定义 | 跨仓协调 | 权限登记（不在本仓） |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层缺失

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-1.2 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-1.3 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-1.4 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-1.5 | spec.md | TASK-Q-006 | 单测 | 是 |
| AC-2.1 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-2.2 | spec.md | TASK-Q-003 | 单测 | 是 |
| AC-2.3 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-2.4 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-2.5 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-3.1 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-3.2 | spec.md | TASK-Q-001 | 单测 | 是 |
| AC-3.3 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-3.4 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-4.1 | spec.md | TASK-Q-004 | 单测 | 是 |
| AC-4.2 | spec.md | TASK-Q-003 | 单测 | 是 |
| AC-4.3 | spec.md | TASK-Q-004 | 单测 | 是 |
| AC-4.4 | spec.md | TASK-Q-004 | 单测 | 是 |
| AC-4.5 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-4.6 | spec.md | TASK-Q-002 | 单测 | 是 |
| AC-5.1 | spec.md | TASK-Q-005 | 单测 | 是 |
| AC-5.2 | spec.md | TASK-Q-005 | 单测 | 是 |
| AC-5.3 | spec.md | TASK-Q-005 | 单测 | 是 |
| AC-5.4 | spec.md | TASK-Q-005 | 单测 | 是 |
| AC-5.5 | spec.md | TASK-Q-005 | 单测 | 是 |
| AC-5.6 | spec.md | TASK-Q-005 | 单测 | 是 |

## 首批实现边界

**首批必须实现：** TASK-Q-001（正常查询链路）+ TASK-Q-002（权限/非系统应用），构成主链与安全边界。
**可后置：** TASK-Q-006（SA 不可用恢复路径）。
**不建议延后：** TASK-Q-004（会话查询）和 TASK-Q-005（批量权限查询）延后会导致核心查询能力不闭合。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 工具查询主链+安全 | TASK-Q-001,002,003 | 正常查询与权限异常单测通过 | `run -t UT -ts cli_tool_mgr_service_test` |
| Phase-2 | 会话查询+批量权限 | TASK-Q-004,005 | 会话查询与批量权限全路径通过 | `run -t UT -ts permission_query_util_test` |
| Phase-3 | SA 不可用恢复 | TASK-Q-006 | SA 不可用覆盖 | `run -t UT -ts cli_tool_mgr_client_test` |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环
- 文件范围、验证闭环和风险边界足够分离 → 拆分
- 本计划按工具查询/权限异常/NAPI校验/会话查询/批量权限/SA不可用拆为 6 张
- 每个 Task 自包含，不依赖外部文件路径引用

## 禁止项

- [x] 没有 TBD / TODO / 占位符（「待确认」为需 Owner 决议项，非规格占位）
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖显式声明在前置依赖列）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-Q-001 | 工具查询正常链路验证（摘要/详情/全量） | climgr/cli_tool_data_manager | AC-1.1,1.2,2.1,2.3,3.1,3.2 | 无 | 正常查询返回正确数据；空注册返回空数组；未注册工具返回 35700005 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-Q-002 | 权限/非系统应用异常路径验证 | cli_tool_manager_service/cli_tool_mgr_client | AC-1.3,1.4,2.4,2.5,3.3,3.4,4.5,4.6 | TASK-Q-001 | 非系统/缺权限返回正确错误码 | `run -t UT -ts cli_tool_mgr_client_test` |
| TASK-Q-003 | NAPI 参数同步校验验证 | js_cli_manager | AC-2.2,4.2 | TASK-Q-001 | 必填缺失/类型错/空串 同步抛错 | `run -t UT -ts cli_tool_mgr_client_test` |
| TASK-Q-004 | 会话查询（正常/不存在/PID归属）验证 | cli_tool_manager_service | AC-4.1,4.3,4.4 | TASK-Q-001 | 正常返回 CliSessionInfo；不存在返回 35700009；非创建者返回 35700007 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-Q-005 | 批量权限查询全路径验证 | permission_query_util/cli_tool_manager_service | AC-5.1–5.6 | TASK-Q-001 | 正常返回 CommandPermission[]；不存在 queryRet=1；空/超限返回 35700002；非SA返回 35700011 | `run -t UT -ts permission_query_util_test` |
| TASK-Q-006 | SA 不可用恢复路径验证 | cli_tool_mgr_client | AC-1.5 | TASK-Q-002 | SA 不可用返回 35700000 | `run -t UT -ts cli_tool_mgr_client_test` |

## Task 详情

### TASK-Q-001: 工具查询正常链路验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证既有工具查询链路符合 spec AC-1.1/1.2/2.1/2.3/3.1/3.2 |
| AC 映射 | AC-1.1, AC-1.2, AC-2.1, AC-2.3, AC-3.1, AC-3.2 |
| 前置依赖 | 无 |
| 非目标 | 不验证权限异常路径（属 TASK-Q-002） |
| 完成判据 | queryToolSummaries 返回 ToolSummary[]；无工具返回空数组；getToolInfoByName 返回 ToolInfo；未注册返回 35700005；queryTools 返回 ToolInfo[] |
| 停止条件 | 发现既有实现与 spec 行为不一致且无法在 Task 范围内修复 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 查询链路用例 |
| Test | `cli_tool_framework/test/unittest/tool_info_test/` | ToolInfo Parcelable 用例 |
| Test | `cli_tool_framework/test/unittest/tool_summary_test/` | ToolSummary Parcelable 用例 |

**Spec Context**

AC-1.1：WHEN 系统应用持 QUERY_CLI_TOOL 调用 queryToolSummaries THEN resolve 为 ToolSummary[]。
AC-2.1：WHEN 系统应用持 QUERY_CLI_TOOL 调用 getToolInfoByName 传入已注册 toolName THEN resolve 为 ToolInfo。
AC-2.3：WHEN toolName 未注册 THEN reject 35700005。
AC-3.1：WHEN 系统应用持 QUERY_CLI_TOOL 调用 queryTools THEN resolve 为 ToolInfo[]。

R-1/R-3/R-5：工具查询正常路径。

**Design Context**

调用链：NAPI→cli_tool_client Proxy→SA 186 查询方法（同步 IPC）→PermissionUtil（系统应用+QUERY_CLI_TOOL）→CliToolDataManager（KVStore 只读查询）→返回 [out] 参数。设计 ADR-1（SA 集中承载）、ADR-2（raw data 优化）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：NAPI 不直接调 services 内部；经 InnerAPI/IPC |
| OH-ARCH-IPC-SAF | Must：查询为同步 IPC；返回 [out] 参数 |

**Steps**

- [ ] 写失败测试：合法调用断言返回正确数据
- [ ] 运行测试，确认当前结果
- [ ] 若不一致，做最小实现修正
- [ ] 运行测试，确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |
| 测试 | `run -t UT -ts tool_info_test` | PASS |
| 测试 | `run -t UT -ts tool_summary_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证工具查询正常链路返回正确数据（AC-1.1/1.2/2.1/2.3/3.1/3.2） |
| 允许修改 | `cli_tool_manager_service.cpp`、`cli_tool_data_manager.cpp`、对应测试 |
| 允许新建 | 测试用例文件 |
| 只读参考 | `spec.md` AC-1.1–3.2、`design.md` ADR-1/ADR-2、`ICliToolManager.idl` |
| Spec 摘要 | AC-1.1/1.2/2.1/2.3/3.1/3.2 + R-1/R-3/R-5 |
| Design 摘要 | NAPI→IPC→SA 同步查询→CliToolDataManager KVStore |
| 执行步骤 | 写失败测试→运行→最小修正→运行通过→记录证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不得修改允许范围外文件；如需扩大范围停止并修订 Plan；无 fresh evidence 不得声明完成 |

### TASK-Q-002: 权限/非系统应用异常路径验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证非系统应用、缺权限调用查询接口返回 35700008/35700007（AC-1.3/1.4/2.4/2.5/3.3/3.4/4.5/4.6） |
| AC 映射 | AC-1.3, AC-1.4, AC-2.4, AC-2.5, AC-3.3, AC-3.4, AC-4.5, AC-4.6 |
| 前置依赖 | TASK-Q-001 |
| 非目标 | 不验证工具不存在（TASK-Q-001） |
| 完成判据 | 非系统应用返回 35700008；缺 QUERY_CLI_TOOL 返回 35700007；缺 EXEC_CLI_TOOL 返回 35700007 |
| 停止条件 | 权限判定与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | 权限异常用例 |

**Spec Context**

AC-1.3：非系统应用→reject 35700008。AC-1.4：缺权限→reject 35700007。
R-2：非系统应用/缺权限→35700008/35700007。

**Design Context**

GetAllToolSummaries/GetToolInfoByName/GetAllToolInfos/QuerySession 入口：IsSystemAppByFullTokenID→ERR_NOT_SYSTEM_APP；VerifyAccessToken(QUERY_CLI_TOOL/EXEC_CLI_TOOL)→ERR_PERMISSION_DENIED。设计「安全基础检查」信任边界。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：System API + 系统应用 + QUERY_CLI_TOOL/EXEC_CLI_TOOL |
| OH-ARCH-ERROR-LOG | Must：错误码 35700008/35700007 |

**Steps**

- [ ] 写失败测试：非系统应用/缺权限断言对应错误码
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证权限/非系统应用异常路径错误码（AC-1.3/1.4/2.4/2.5/3.3/3.4/4.5/4.6） |
| 允许修改 | `cli_tool_manager_service.cpp`、`cli_tool_mgr_client.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.3–4.6 + R-2、design 安全基础检查 |
| Spec 摘要 | 非系统→35700008；缺权限→35700007 |
| Design 摘要 | IsSystemAppByFullTokenID + VerifyAccessToken |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-Q-003: NAPI 参数同步校验验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 getToolInfoByName 的 toolName 为空/非串/未传入、querySession 的 sessionId 为空/非串/未传入均同步抛错（AC-2.2/4.2） |
| AC 映射 | AC-2.2, AC-4.2 |
| 前置依赖 | TASK-Q-001 |
| 非目标 | 不验证服务端校验 |
| 完成判据 | 各缺失/类型错/空串均同步抛 invalid parameter 异常，不进入异步 |
| 停止条件 | NAPI 校验与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | NAPI 校验用例 |

**Spec Context**

AC-2.2：toolName 空/非串/未传入→同步抛错。AC-4.2：sessionId 空/非串/未传入→同步抛错。
R-9：NAPI 层校验先于服务端。

**Design Context**

OnGetToolInfoByName：argc<INDEX_ONE→ThrowTooFewParametersError；UnwrapStringFromJS2(toolName) 失败或空→ThrowInvalidParamError("Tool name is required")。OnQuerySession：argc<INDEX_ONE→ThrowTooFewParametersError；UnwrapStringFromJS2(sessionId) 失败或空→ThrowInvalidParamError("sessionId is required")。调用链层级 NAPI 层。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：NAPI 层做同步参数校验先于 IPC |

**Steps**

- [ ] 写失败测试：各缺失/类型错/空串断言同步抛错
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 NAPI 参数同步校验（AC-2.2/4.2） |
| 允许修改 | `js_cli_manager.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-2.2/4.2 + R-9、design 调用链 NAPI 层 |
| Spec 摘要 | 必填缺失/类型错/空串→同步抛错 |
| Design 摘要 | OnGetToolInfoByName/OnQuerySession 参数解析与 ThrowInvalidParamError |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-Q-004: 会话查询验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 querySession 正常返回 CliSessionInfo、sessionId 不存在返回 35700009、非创建者返回 35700007（AC-4.1/4.3/4.4） |
| AC 映射 | AC-4.1, AC-4.3, AC-4.4 |
| 前置依赖 | TASK-Q-001 |
| 非目标 | 不验证权限路径（TASK-Q-002） |
| 完成判据 | 正常调用返回 CliSessionInfo（含 status/result）；不存在返回 35700009；非创建者 PID 返回 35700007 |
| 停止条件 | 会话查找或 PID 校验与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 会话查询用例 |
| Test | `cli_tool_framework/test/unittest/session_record_test/` | BuildSessionInfo 用例 |

**Spec Context**

AC-4.1：系统应用+EXEC_CLI_TOOL+自有 sessionId→resolve CliSessionInfo。AC-4.3：sessionId 不存在→reject 35700009。AC-4.4：非创建者 PID→reject 35700007。
R-6/R-7/R-8。

**Design Context**

QuerySession：GetSessionRecord(sessionId)→nullptr→ERR_CLI_SESSION_NOT_FOUND；IsSessionOwner(record,"QuerySession")→callerPid≠record->callerPid→ERR_PERMISSION_DENIED；record->BuildSessionInfo(session)。设计 ADR-3（EXEC_CLI_TOOL+PID 归属）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：EXEC_CLI_TOOL + 系统应用 + PID 归属 |
| OH-ARCH-ERROR-LOG | Must：错误码 35700009/35700007 |

**Steps**

- [ ] 写失败测试：正常/不存在/非创建者三种场景
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |
| 测试 | `run -t UT -ts session_record_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证会话查询正常/不存在/PID 归属（AC-4.1/4.3/4.4） |
| 允许修改 | `cli_tool_manager_service.cpp`、`session_record.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-4.1/4.3/4.4 + R-6/R-7/R-8、design ADR-3 |
| Spec 摘要 | 正常→CliSessionInfo；不存在→35700009；非创建者→35700007 |
| Design 摘要 | GetSessionRecord + IsSessionOwner + BuildSessionInfo |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-Q-005: 批量权限查询全路径验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 BatchQueryPermissionBySubCommand 正常返回 CommandPermission[]、未注册工具 queryRet=1、空/超限返回 35700002、非 SA 返回 35700011（AC-5.1–5.6） |
| AC 映射 | AC-5.1, AC-5.2, AC-5.3, AC-5.4, AC-5.5, AC-5.6 |
| 前置依赖 | TASK-Q-001 |
| 非目标 | 不验证工具查询路径（TASK-Q-001） |
| 完成判据 | 正常返回 CommandPermission[]（queryRet=0）；未注册 queryRet=1；空/超限 35700002；非SA 35700011；缺权限 35700007 |
| 停止条件 | 批量查询逻辑与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/permission_query_util_test/` | 批量查询逻辑用例 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | SA 入口校验用例 |

**Spec Context**

AC-5.1：SA+QUERY_CLI_TOOL+有效命令→CommandPermission[]（queryRet=0）。AC-5.2：未注册 toolName→queryRet=1。AC-5.3：子命令不存在→queryRet=1。AC-5.4：空/超限→35700002。AC-5.5：非SA→35700011。AC-5.6：缺权限→35700007。
R-10/R-11/R-12。

**Design Context**

BatchQueryPermissionBySubCommand：cmds.empty()||cmds.size()>=MAX_QUERY_CMDS_SIZE(100)→ERR_INVALID_PARAM；GetTokenTypeFlag≠TOKEN_NATIVE→ERR_NOT_SA_CALLER；VerifyAccessToken(QUERY_CLI_TOOL)→ERR_PERMISSION_DENIED；PermissionQueryUtil::BatchQueryPermissions 逐命令查 KVStore。设计 ADR-4（SA-only）、ADR-5（逐命令 queryRet）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：Inner API + TOKEN_NATIVE + QUERY_CLI_TOOL |
| OH-ARCH-ERROR-LOG | Must：错误码 35700002/35700007/35700011 |

**Steps**

- [ ] 写失败测试：正常/未注册/子命令不存在/空/超限/非SA/缺权限
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts permission_query_util_test` | PASS |
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证批量权限查询全路径（AC-5.1–5.6） |
| 允许修改 | `permission_query_util.cpp`、`cli_tool_manager_service.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-5.1–5.6 + R-10/R-11/R-12、design ADR-4/ADR-5 |
| Spec 摘要 | 正常→CommandPermission[]；未注册→queryRet=1；空/超限→35700002；非SA→35700011 |
| Design 摘要 | TOKEN_NATIVE 校验 + PermissionQueryUtil::BatchQueryPermissions + 逐命令 queryRet |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts permission_query_util_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-Q-006: SA 不可用恢复路径验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 CliToolManager SA 未就绪或代理获取失败时返回 35700000（AC-1.5） |
| AC 映射 | AC-1.5 |
| 前置依赖 | TASK-Q-002 |
| 非目标 | 不验证正常路径 DFX |
| 完成判据 | SA 拉起失败/代理空→reject 35700000 |
| 停止条件 | 恢复路径与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/` | SA 不可用用例 |

**Spec Context**

AC-1.5：CliToolManager SA 未就绪/代理获取失败→reject 35700000。
R-12（SA 不可用部分）。

**Design Context**

cli_tool_client LoadCliToolMgrService 失败→GET_CLI_TOOL_MGR_SERVICE_FAILED。客户端按需拉起 SA；拉起失败即返回 35700000。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：SA 按需拉起，失败回 35700000 |
| OH-ARCH-ERROR-LOG | Must：错误码 35700000 |

**Steps**

- [ ] 写失败测试：SA 不可用断言 35700000
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 SA 不可用恢复路径 35700000（AC-1.5） |
| 允许修改 | `cli_tool_mgr_client.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.5 + R-12、design 异常传播时序图 |
| Spec 摘要 | SA 不可用→35700000 |
| Design 摘要 | LoadCliToolMgrService 失败→GET_CLI_TOOL_MGR_SERVICE_FAILED |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-1.1–AC-5.6 全覆盖）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符（「待确认」为 Owner 决议项）
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 完整）
- [x] 每个 Task 验证在完成时立即执行并记录证据
- [x] 超 3000 行阈值的 Task 已说明不拆分理由（本计划 Task 均聚焦验证+补测试，上下文未超阈值）
