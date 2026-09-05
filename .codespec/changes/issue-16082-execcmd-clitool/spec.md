# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | execCmd 工具命令模式 |
| 特性编号 | 16082 |
| 优先级 | P1 |
| 复杂度 | 标准（引用 proposal.md 初始分级判断） |
| 目标版本 | 7.1 |

## 用户故事或场景

### US-1: LLM 工具命令直通执行

**作为** AI/LLM 业务调用方, **我想要** 将拼接好的工具命令字符串直接交给 execCmd 执行, **以便** 无需自行解析为结构化参数，统一覆盖 shell 命令与工具命令两类场景。

**验收标准：**

> 使用 Given/When/Then 格式。Given 描述前置条件；When 描述用户/业务**可操作动作**（开发者调 API、用户操作、上层业务触发、外部事件），含精确输入；Then 描述通过**三层接口边界**可观测的结果（public/system/inner API 返回值/回调/错误码，或终端用户可感知行为），含精确输出。
> Then 禁止描述部件**内部实现**（内部数据结构如注册表/指针/链表、内部状态机转换、内部流程、遍历算法）——这些移 `design.md`「状态归属与不变量」。判定：Then 的行为能否仅凭公开接口判定？需查内部状态 → 越界，移 design。
> 避免模糊表述；验证映射表须给出对应验证方式。
- **AC-1.1:** Given 已注册 CLI 工具 `ohos-aa`、调用方为系统应用且持有 `ohos.permission.EXEC_CLI_TOOL` 权限 When 调用 `execCmd("ohos-aa start --bundleName=com.example", { isShellCommand: false })` Then 返回的 Promise resolve 为 `CliSessionInfo`，其 `status` 为 `running`、`toolName` 为 `"ohos-aa"`
- **AC-1.2:** Given 调用方未在 `ExecCmdOptions` 中传 `isShellCommand` When 调用 `execCmd("ls -l")` Then 走原有 shell 路径，返回的 `CliSessionInfo` 与改动前一致，无行为回退
- **AC-1.3:** Given 工具命令模式（`isShellCommand=false`） When `cmd` 提取出的 toolName 含路径分隔符或非法字符（如 `"/system/bin/ohos-aa"`、`"../ohos-aa"`、`"ohos-aa;rm"`） Then Promise reject `BusinessError` 401 `"Invalid input parameter."`，不创建子进程
- **AC-1.4:** Given 工具命令模式 When `cmd` 的 toolName 未在 CLI 工具注册表中注册（如 `"ohos-unknown start"`） Then Promise reject `BusinessError` 35600030 `"The tool does not exist."`
- **AC-1.5:** Given 工具命令模式 When `cmd` 为纯空白（如 `"   "`） Then Promise reject `BusinessError` 401 `"Invalid input parameter."`
- **AC-1.6:** Given 工具命令模式、toolName 已注册 When `cmd` 含 schema 中未定义的 flag（如 `"ohos-aa start --unknownFlag=x"`） Then Promise reject `BusinessError` 401 `"Invalid input parameter."`
- **AC-1.7:** Given 工具命令模式 When 命令参数使用长标志格式（`--key=value`、`--key value`、`--flag`、`--flag false`） Then 工具按 schema 类型恢复后的参数值执行，`CliSessionInfo` 正常返回
- **AC-1.8:** Given 旧客户端不写 `isShellCommand`/`challenge` 序列化字段 When 经 IPC 调用新服务端 `ExecCmd` Then 服务端 `ReadBool` 失败回退 `isShellCommand=true`（shell 模式），行为与旧版一致且不崩溃
- **AC-1.9:** Given 工具命令模式、并发会话数已达上限 When 调用 `execCmd` Then Promise reject `BusinessError` 35600031 `"Maximum number of processes has been reached."`

## 业务规则

> 业务规则是**约束声明**（非验收标准）：声明"什么不变量/约束成立"，约束条件列可含简短内部线索（如"唯一指针"）作 design 线索；**详细内部实现展开移 design.md「状态归属与不变量」**。AC 严格可观测，BR 放宽。

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | toolName 白名单仅允许 `[a-zA-Z0-9-_]`，拦截所有路径注入（`/`、`.`、`\`、空格、`;` `$` `` ` `` `|` `&`） | 校验在服务端 ExecCmdToolMode 入口执行 | AC-1.3 |
| BR-2 | 工具命令模式复用 `ValidateAndPrepareTool` 校验链路，不绕过 `PermissionVerification`、`GenerateSandboxConfig` 等任何安全检查 | 与 execTool 共用同一权限/沙箱链路 | AC-1.1, AC-1.7 |
| BR-3 | `isShellCommand` 默认 `true`（shell 模式），未传时行为与改动前完全一致 | 默认值在 NAPI 层与 IPC Unmarshalling 层一致 | AC-1.2, AC-1.8 |
| BR-4 | IPC 序列化字段顺序固定为 `cmd→workDir→env→policy→options→isShellCommand→challenge`，新增两字段位于 Parcel 末尾 | 旧客户端不写尾部字段时 ReadBool/ReadString 失败回退默认值 | AC-1.8 |
| BR-5 | 会话建立顺序为「先订阅后创建进程」，保证早期事件不丢失 | SubscribeSession 先于 CreateChildProcess | AC-1.1 |
| BR-6 | `param.cmd` 在日志中使用 `%{private}s`，Release 构建自动脱敏 | 防止泄露命令中的文件路径等敏感信息 | AC-1.1 |
| BR-7 | `challenge` 由调用方从 access token manager 获取并透传给 `ExecToolParam`，服务端不生成 | challenge 透传不改值 | AC-1.1 |
| BR-8 | 命令解析仅支持长标志（`--` 前缀）语法；短标志、前缀参数、条件标志、双引号、反斜杠转义不支持 | 列入后续迭代 | AC-1.6, AC-1.7 |

## 异常与边界规则

> 超时操作必须给出精确阈值（ms/s），不可写"待定义"。跨进程字段必须给出精确限制（字节数/长度）。

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EB-1 | 默认前台等待超时 | 调用方未传 `yieldMs` | 默认 0ms（立即后台化） | AC-1.1 |
| EB-2 | 工具最大执行超时 | 调用方未传 `timeout` | 默认 1800s | AC-1.1 |
| EB-3 | 命令字符串最大长度 | `cmd` 超长 | 由 IPC Parcel 与 shell/cli 沙箱参数限制约束（具体上限由沙箱层限制） | AC-1.1 |
| EB-4 | 会话数上限 | 并发会话达 `ERR_SESSION_LIMIT_EXCEEDED` | 返回 35600031 | AC-1.9 |
| EB-5 | 单引号包裹含空格的值 | `--name 'hello world'` | 分词器识别为单个值 `hello world` | AC-1.7 |
| EB-6 | 布尔 flag 显式 false | `--verbose false` | schema 为 boolean 时设为 false | AC-1.7 |
| EB-7 | 未知子命令 | `cmd` 第二 token 非 `--` 前缀且不在 `toolInfo.subcommands` | 返回 `ERR_TOOL_NOT_EXIST`（35600030） | AC-1.4 |

## 错误码定义

> 每个错误码必须给出精确的数值和含义。避免"返回错误"这种模糊描述——要写明错误类型、错误消息和触发条件。

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERROR_PERMISSION_DENIED | 201 | 权限不足：调用方未持有 `ohos.permission.EXEC_CLI_TOOL`（内部码 ERR_PERMISSION_DENIED=35700007） | AC-1.1 |
| ERROR_NOT_SYSTEM_APP | 202 | 非系统应用：调用方非系统应用（内部码 ERR_NOT_SYSTEM_APP=35700008） | AC-1.1 |
| ERROR_INVALID_PARAM | 401 | 输入参数无效：工具命令模式下 cmd 为空白、toolName 含非法字符、命令含未知 flag、参数类型恢复失败、孤立值（内部码 ERR_INVALID_PARAM=35700002） | AC-1.3, AC-1.5, AC-1.6 |
| ERROR_TOOL_NOT_FOUND | 35600030 | 工具不存在：toolName 未注册或子命令未定义（内部码 ERR_TOOL_NOT_EXIST=35700005） | AC-1.4 |
| ERROR_REACH_LIMIT | 35600031 | 进程数已达上限（内部码 ERR_SESSION_LIMIT_EXCEEDED=35700001） | AC-1.9 |
| ERROR_INNER | 35600050 | 内部错误：会话记录创建失败（ERR_NO_INIT=35700004）、沙箱生成失败、进程创建失败、监控注册失败、IPC/服务不可用、KVStore/JSON 配置错误（未映射的内部码均回退至此） | AC-1.1 |

## API 规格定义

> 本章节定义本次需求新增/修改 API 的完整规格，作为 implement 阶段修改声明文件的精确蓝图。
> 每个 API 必须覆盖以下全部维度，不可遗漏。如不涉及 API/SDK 设计，填写"不涉及"并说明理由。
> 跨平台/元服务/卡片/FA-Stage 模型等支持属性若无法从需求描述推断，须先与开发者确认，不得猜测或填写默认值。

本变更修改既有声明文件 `@ohos.app.cli.cliManager.d.ts`，不新增声明文件。格式参考既有 .d.ts（版权头、`@kit AbilityKit`、`@syscap SystemCapability.Ability.AgentRuntime.Core`、`@systemapi`、`@stagemodelonly`、`@since 26.0.0 dynamiconly`）。

| 规格项 | 值 |
|--------|-----|
| **是否新增声明文件** | 否（修改 `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` 及 `zh-cn` 版） |
| **API 类型** | System（`@systemapi`） |
| **API 命名** | `cliManager.ExecCmdOptions.isShellCommand`、`cliManager.ExecCmdOptions.challenge`（既有 `execCmd` 函数签名不变） |
| **入参** | `isShellCommand?: boolean`（可选，默认 `true`）；`challenge?: string`（可选，默认 `""`） |
| **返回值** | `execCmd` 返回 `Promise<CliSessionInfo>`（不变） |
| **权限** | `ohos.permission.EXEC_CLI_TOOL`（系统权限，grant by system） |
| **@syscap** | `SystemCapability.Ability.AgentRuntime.Core` |
| **@since** | `26.0.0 dynamiconly` |
| **错误码** | 201: 权限不足；202: 非系统应用；401: 参数无效（cmd 空白/toolName 非法/未知 flag）；35600030: 工具不存在；35600031: 进程上限；35600050: 内部错误 |
| **编程语言** | ArkTS（NAPI 模块 `@ohos.app.cli.cliManager`） |
| **跨平台/元服务/卡片** | 否 / 否 / 否（声明文件无 `@crossplatform` / `@atomicservice` / `@form` 标记） |
| **FA/Stage 模型** | `@stagemodelonly`（仅 Stage 模型） |

## 接口变更分析

> 「开放级别」取值（与 OH InnerAPI 口径一致，供下游测试按级别识别/过滤接口）：**Public**（面向三方应用）/ **System**（@systemapi，面向系统应用）/ **Inner**（InnerAPI，部件间内部接口，不对外暴露）。新增接口与变更/废弃接口两表共用同一取值。

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| `ExecCmdOptions.isShellCommand` | System | `isShellCommand?: boolean`，默认 `true`，`true`=shell 路径 / `false`=工具命令路径 | —（属性） | — | AC-1.1, AC-1.2, AC-1.8 |
| `ExecCmdOptions.challenge` | System | `challenge?: string`，默认 `""`，从 access token manager 获取的唯一标识，透传给工具进程 | —（属性） | — | AC-1.1 |

### 变更/废弃接口

| 接口名称 | 开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|----------|----------|----------|----------|---------|
| `cliManager.execCmd` | System | 文档变更（@throws 补充） | 工具命令模式新增 401、35600030 错误路径；当前 .d.ts `@throws` 仅列 201/202/35600031/35600050，**需补充 401 与 35600030** | 无须代码迁移；调用方应在 catch 中新增对 401/35600030 的处理 | AC-1.3, AC-1.4, AC-1.5, AC-1.6 |

## 兼容性声明

- **已有 API 行为变更:** 否——`execCmd` 签名不变；`ExecCmdOptions` 新增两个可选属性，未传时 `isShellCommand=true` 默认走原 shell 路径，行为与改动前一致
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否——IPC Parcel 为运行时序列化，两字段追加于末尾，旧客户端不写时回退默认值，向后兼容；无持久化数据格式变更

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1.1 | BR-2, BR-5, BR-7 | 单元测试：ExecCmd 工具命令模式调用已注册工具，断言 CliSessionInfo 返回 | cli_tool_mgr_service_test |
| AC-1.2 | BR-3 | 回归测试：不传 isShellCommand 调 execCmd，断言与原 shell 路径结果一致 | cli_tool_mgr_service_test |
| AC-1.3 | BR-1 | 安全测试：注入含 `/`、`.`、`;` 的 toolName，断言 reject 401 且无进程创建 | cli_tool_mgr_service_test |
| AC-1.4 | EB-7 | 单元测试：未注册 toolName 与未知子命令，断言 reject 35600030 | cli_tool_mgr_service_test |
| AC-1.5 | — | 单元测试：纯空白 cmd 调用，断言 reject 401 | cli_tool_mgr_client_test |
| AC-1.6 | BR-8 | 单元测试：未知 flag 命令，断言 reject 401 | tool_util_test |
| AC-1.7 | BR-8, EB-5, EB-6 | 单元测试：4 种长标志格式 + 单引号空格值，断言参数按 schema 类型恢复 | tool_util_test |
| AC-1.8 | BR-3, BR-4 | IPC 兼容测试：模拟旧客户端 Parcel（不写尾部字段），断言服务端回退 shell 模式不崩溃 | exec_cmd_param_test |
| AC-1.9 | EB-4 | 单元测试：会话达上限后调用，断言 reject 35600031 | cli_tool_mgr_service_test |

## 测试设计提示

> 面向 AI 生成测试和实现计划。每条 AC 至少给出一个测试入口；`Red 条件` 必须说明实现前为什么会失败，避免写出立即通过的无效测试。

| AC | 测试类型 | 测试文件 | 测试名称 | 输入/触发 | 期望输出/错误 | Red 条件 |
|----|----------|----------|----------|-----------|---------------|----------|
| AC-1.1 | 单元测试 | cli_tool_mgr_service_test.cpp | ExecCmdToolMode_NormalTool | `execCmd("ohos-aa start --bundleName=com.x", {isShellCommand:false})` | CliSessionInfo.status=running, toolName=ohos-aa | 实现前 ExecCmd 不识别 isShellCommand 分流，走 shell 路径返回 shell 结果而非工具会话 |
| AC-1.3 | 单元测试 | cli_tool_mgr_service_test.cpp | ExecCmdToolMode_InvalidToolName | cmd 含 `/`、`.`、`;` | reject 401，无子进程 | 实现前无白名单校验，路径注入未被拦截 |
| AC-1.4 | 单元测试 | cli_tool_mgr_service_test.cpp | ExecCmdToolMode_ToolNotExist | `"ohos-unknown start"` | reject 35600030 | 实现前 GetToolByName 失败路径未接 execCmd |
| AC-1.5 | 单元测试 | cli_tool_mgr_client_test.cpp | ExecCmd_EmptyCmdToolMode | `execCmd("   ", {isShellCommand:false})` | reject 401 | 实现前客户端未做空白校验 |
| AC-1.6 | 单元测试 | tool_util_test.cpp | ParseToolCommand_UnknownFlag | `"ohos-aa start --unknownFlag=x"` | 返回 ERR_INVALID_PARAM | 实现前 ParseToolCommand 未实现 |
| AC-1.7 | 单元测试 | tool_util_test.cpp | ParseToolCommand_LongFlagFormats | 4 种格式 + 单引号空格值 | 参数按 schema 类型恢复 | 实现前 ParseToolCommand 未实现 |
| AC-1.8 | 单元测试 | exec_cmd_param_test.cpp | Unmarshalling_OldClientCompat | Parcel 仅写到 options | isShellCommand=true(默认)，不崩溃 | 实现前 Unmarshalling 无尾部默认值回退 |
| AC-1.9 | 单元测试 | cli_tool_mgr_service_test.cpp | ExecCmdToolMode_SessionLimit | 会话达上限 | reject 35600031 | 会话上限逻辑既有，但工具命令路径未接入 |

> AC → 实现文件 + Task + 验证状态的映射见 `execution-plan.md`「AC 到 Task 追溯」+「代码范围映射」。
