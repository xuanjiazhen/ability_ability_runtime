# 支持系统Agent调用系统 CLI 工具规格

> 来源：REQ-010（关联 Issue #15023）
> 代码路径：`cli_tool_framework/`
> 模板：`skills/sdd/templates/spec.md`

## 功能规格

| 编号 | 规格 |
|------|------|
| FR-1 | 提供 `execTool` 接口，系统 Agent 通过指定工具名（toolName）和子命令（subcommand）执行已注册的 CLI 工具 |
| FR-2 | 提供 `execCmd` 接口，系统 Agent 执行原始 shell 命令，支持 workDir、env、policy 参数 |
| FR-3 | `execTool` 和 `execCmd` 通过 fork 创建子进程，子进程内调用 execve 加载目标工具执行 |
| FR-4 | `execTool` 通过 ToolInfo 的 requirePermissions 校验调用方权限，无权限时返回 `ERR_PERMISSION_DENIED` |
| FR-5 | 仅系统应用（System HAP）可调用 `execTool` 和 `execCmd`，非系统应用返回 `ERR_NOT_SYSTEM_APP` |
| FR-6 | ExecOptions 支持配置后台执行（background）、超时时间（timeout，单位 ms）、让出时间（yieldMs） |
| FR-7 | 工具执行超时时，子进程收到信号终止，ExecResult.timeout 标记为 true |
| FR-8 | 后台执行模式下，工具在后台运行，通过 Session 机制异步获取输出和事件 |
| FR-9 | 提供 `GetToolInfoByName` 查询单个工具信息，提供 `GetAllToolSummaries` 查询全部工具摘要列表 |
| FR-10 | 提供 `GetAllToolInfos` 获取全部工具完整信息（含 requirePermissions、inputSchema、outputSchema、subcommands） |
| FR-11 | 提供 `QuerySession` 查询会话状态（running/completed/failed）和执行结果（ExecResult） |
| FR-12 | 提供 `SubscribeSession` / `UnsubscribeSession` 订阅/取消订阅会话事件，通过 ICliToolManagerScheduler 异步回调 |
| FR-13 | 提供 `SendMessage` 向运行中的会话发送输入文本，支持交互式工具 |
| FR-14 | 提供 `ClearSession` 清理指定会话，终止关联子进程并释放资源 |
| FR-15 | 提供 `BatchQueryPermissionBySubCommand` 批量查询命令所需权限和锁屏执行许可（Inner API，SA 专用） |
| FR-16 | 提供 Function 管理接口：RegisterFunction / BatchRegisterFunctions / GetFunctionInfo / UnregisterFunction / GetAllFunctions，支持意图函数注册和查询 |
| FR-17 | ToolInfo 通过 JSON 配置文件加载，支持工具名、可执行路径、权限、输入/输出 Schema、事件类型、子命令的校验 |
| FR-18 | ExecResult 返回退出码（exitCode）、标准输出（outputText）、标准错误（errorText）、信号编号（signalNumber）、超时标记（timeout）、执行耗时（executionTime） |
| FR-19 | ICliToolManagerScheduler 通过 SchedulerExecToolReplyEvent 异步回调执行结果，通过 SchedulerSessionEvent 回调会话事件，通过 SchedulerInputReplyEvent 回调消息发送结果 |
| FR-20 | 本需求仅支持动态执行接口，静态接口由另外的需求跟踪 |

## 返回提示

| 场景 | 错误码 | 返回提示 |
|------|--------|----------|
| 获取 CliToolManagerService 失败 | 35700000 | 获取 CLI 工具管理服务失败 |
| 会话数超限 | 35700001 | 会话数量超过上限 |
| 参数非法（空工具名、空命令等） | 35700002 | 参数非法 |
| 调用方非 HAP 应用 | 35700003 | 调用方非 HAP 应用 |
| 服务未初始化 | 35700004 | 服务未初始化 |
| 工具不存在 | 35700005 | 工具不存在 |
| 内部参数无效（JS CliSessionInfo 创建失败等） | 35700006 | 内部参数无效 |
| 权限不足 | 35700007 | 权限不足 |
| 调用方非系统应用 | 35700008 | 调用方非系统应用 |
| 会话不存在 | 35700009 | 会话不存在 |
| 消息发送失败 | 35700010 | 消息发送失败 |
| 调用方非 SA（BatchQueryPermissionBySubCommand 专用） | 35700011 | 调用方非 SA |
| 终止进程失败 | 35700012 | 终止进程失败 |
| 函数不存在 | 35700013 | 函数不存在 |
| JSON 解析失败 | 35700014 | JSON 解析失败 |
| 文件未找到 | 35700015 | 文件未找到 |
| KVStore 未就绪 | 35700016 | KVStore 未就绪 |
| 函数已存在 | 35700017 | 函数已存在 |
| KVStore 操作错误 | 35700018 | KVStore 操作错误 |
| 函数执行失败 | 35700019 | 函数执行失败 |
| 函数执行超时 | 35700020 | 函数执行超时 |
| 执行成功 | 0 | 成功 |

说明：错误码基址为 35700000，定义于 `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h`。

## 验收标准

| AC编号 | 验收标准 | 对应 FR |
|--------|----------|---------|
| AC-1 | 调用 `execTool` 接口能够启动已注册的 CLI 工具进程，ExecResult.exitCode 返回工具退出码 | FR-1, FR-3 |
| AC-2 | 调用 `execCmd` 接口能够执行原始 shell 命令，返回 stdout/stderr 输出 | FR-2, FR-3 |
| AC-3 | 权限校验准确：调用方需通过 ToolInfo.requirePermissions 的 AccessToken 校验，无权限返回 ERR_PERMISSION_DENIED（35700007） | FR-4 |
| AC-4 | 非系统应用调用 `execTool` / `execCmd` 时返回 ERR_NOT_SYSTEM_APP（35700008） | FR-5 |
| AC-5 | ExecOptions.timeout 生效：超时后子进程被终止，ExecResult.timeout 为 true | FR-6, FR-7 |
| AC-6 | 后台执行模式（background=true）下，工具异步运行，通过 SubscribeSession + SchedulerSessionEvent 获取输出 | FR-8, FR-12, FR-19 |
| AC-7 | `GetToolInfoByName` 返回正确的工具信息，`GetAllToolSummaries` 返回全部已注册工具摘要 | FR-9 |
| AC-8 | `GetAllToolInfos` 返回工具完整信息（含 permissions、schema、subcommands） | FR-10 |
| AC-9 | `QuerySession` 返回正确的会话状态（running/completed/failed）和 ExecResult | FR-11 |
| AC-10 | `SendMessage` 向运行中会话发送输入文本成功，SchedulerInputReplyEvent 回调结果码 | FR-13, FR-19 |
| AC-11 | `ClearSession` 清理会话后，关联子进程终止，资源释放 | FR-14 |
| AC-12 | `BatchQueryPermissionBySubCommand` 批量返回命令所需权限和锁屏执行许可，仅 SA 可调用 | FR-15 |
| AC-13 | Function 管理接口（Register/Get/Unregister/BatchRegister/GetAll）功能正确，重复注册返回 ERR_FUNCTION_EXIST | FR-16 |
| AC-14 | ToolInfo 从 JSON 配置文件正确加载，ValidateName / ValidateExecutablePath / ValidateRequirePermissions 校验通过 | FR-17 |
| AC-15 | ExecResult 包含 exitCode、outputText、errorText、signalNumber、timeout、executionTime 全部字段 | FR-18 |
| AC-16 | SchedulerExecToolReplyEvent 异步回调执行结果，SchedulerSessionEvent 回调会话事件 | FR-19 |
| AC-17 | 错误码覆盖全部 21 种错误场景，每种错误返回对应码值 | 返回提示表 |
| AC-18 | 无内存泄漏（Valgrind/ASan 检测通过） | — |
