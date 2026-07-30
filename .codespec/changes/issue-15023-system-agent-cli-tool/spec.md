# 支持系统Agent调用系统 CLI 工具规格

> 来源：REQ-010（关联 Issue #15023）
> 代码路径：`cli_tool_framework/`
> 模板：`skills/sdd/templates/spec.md`
> 范围：仅 `execTool` 接口

## 功能规格

| 编号 | 规格 |
|------|------|
| FR-1 | 提供 `execTool` 接口，系统 Agent 通过 ExecToolParam（toolName、subcommand、challenge、options、args）执行已注册的 CLI 工具 |
| FR-2 | `execTool` 通过 fork 创建子进程，子进程内调用 execve 加载目标工具执行 |
| FR-3 | `execTool` 校验调用方为系统应用（System HAP），非系统应用返回 `ERR_NOT_SYSTEM_APP` |
| FR-4 | `execTool` 通过 ToolInfo.requirePermissions 校验调用方 AccessToken 权限，无权限返回 `ERR_PERMISSION_DENIED` |
| FR-5 | `execTool` 校验工具名非空且工具已注册，未注册返回 `ERR_TOOL_NOT_EXIST` |
| FR-6 | ExecOptions.timeout 生效：超时后子进程被信号终止，ExecResult.timeout 标记为 true |
| FR-7 | ExecOptions.background=true 时工具后台异步运行，通过 ICliToolManagerScheduler.SchedulerExecToolReplyEvent 回调结果 |
| FR-8 | ExecResult 返回退出码（exitCode）、标准输出（outputText）、标准错误（errorText）、信号编号（signalNumber）、超时标记（timeout）、执行耗时（executionTime） |
| FR-9 | `execTool` 通过 ICliToolManagerScheduler.SchedulerExecToolReplyEvent 异步回调执行结果（resultCode + CliSessionInfo） |
| FR-10 | 本需求仅支持动态执行接口，静态接口由另外的需求跟踪 |

## 返回提示

| 场景 | 错误码 | 返回提示 |
|------|--------|----------|
| 获取 CliToolManagerService 失败 | 35700000 | 获取 CLI 工具管理服务失败 |
| 会话数超限 | 35700001 | 会话数量超过上限 |
| 参数非法（空工具名等） | 35700002 | 参数非法 |
| 调用方非 HAP 应用 | 35700003 | 调用方非 HAP 应用 |
| 服务未初始化 | 35700004 | 服务未初始化 |
| 工具不存在 | 35700005 | 工具不存在 |
| 内部参数无效（JS CliSessionInfo 创建失败等） | 35700006 | 内部参数无效 |
| 权限不足 | 35700007 | 权限不足 |
| 调用方非系统应用 | 35700008 | 调用方非系统应用 |
| 终止进程失败 | 35700012 | 终止进程失败 |
| 执行成功 | 0 | 成功 |

说明：错误码定义于 `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h`，仅列出 execTool 接口可达的错误码。

## 验收标准

| AC编号 | 验收标准 | 对应 FR |
|--------|----------|---------|
| AC-1 | 调用 `execTool` 接口能够启动已注册的 CLI 工具进程，ExecResult.exitCode 返回工具退出码 | FR-1, FR-2 |
| AC-2 | 非系统应用调用 `execTool` 时返回 ERR_NOT_SYSTEM_APP（35700008） | FR-3 |
| AC-3 | 权限校验准确：调用方需通过 ToolInfo.requirePermissions 的 AccessToken 校验，无权限返回 ERR_PERMISSION_DENIED（35700007） | FR-4 |
| AC-4 | 工具名不存在时返回 ERR_TOOL_NOT_EXIST（35700005），空工具名返回 ERR_INVALID_PARAM（35700002） | FR-5 |
| AC-5 | ExecOptions.timeout 生效：超时后子进程被终止，ExecResult.timeout 为 true | FR-6 |
| AC-6 | 后台执行模式（background=true）下，工具异步运行，通过 SchedulerExecToolReplyEvent 回调结果 | FR-7, FR-9 |
| AC-7 | ExecResult 包含 exitCode、outputText、errorText、signalNumber、timeout、executionTime 全部字段 | FR-8 |
| AC-8 | SchedulerExecToolReplyEvent 异步回调执行结果（resultCode + CliSessionInfo），前台和后台模式均生效 | FR-9 |
