# 特性规格

> 固化用户可见行为和验收标准。本规格固化 `agenthook_design.txt` 粗方案与 proposal.md 基线中的行为契约。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | AgentHook 能力 |
| 特性编号 | FEAT-AGENT-HOOK |
| 所属 Epic | 无 |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-6.0-Release |
| SIG 归属 | ability SIG |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | ICliToolManager 新增 6 个方法：RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/BeforeInvokeFunction/AfterInvokeFunction | SystemAPI，授权调用方使用 |
| DELETED | ICliToolManager::GetFunctionHook 移除（v1.0 定义，由 BeforeInvokeFunction/AfterInvokeFunction 替代） | v2.0 变更 CHG-3 |
| ADDED | 新增 ICliHookInterface 回调接口：BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd | 由 Hook 注册方实现 |
| ADDED | 新增 IFunctionHookInterface 回调接口：BeforeInvokeFunction/AfterInvokeFunction | 由 Hook 注册方实现 |
| ADDED | ExecTool/ExecCmd 执行链路插入 cliHook 回调点（dev mode 门控） | service 侧 |
| ADDED | InvokeFunctionExecutor 调用 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction（service 内部分发） | 调用方侧 |
| ADDED | NAPI 层：JS 对象封装为 IPC Stub + JS 线程同步回调 | 扩展 cli_tool_manager + function_manager 现有 NAPI 模块 |
| ADDED | SDK JS：3 个 Hook interface（CliHook、FunctionHook、cliManager 注册函数） | interface/sdk-js/ |
| CHANGED | 门控方式：`system::GetBoolParameter("const.security.developermode.state")` 运行时判断（移除宏隔离） | v2.0 变更 CHG-1 |
| CHANGED | 注册语义：非开发模式返回 ERR_NOT_DEVELOPER_MODE(35700021)，已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式，非覆盖式） | v2.0 变更 CHG-2；v3.0 拆分错误码 |
| CHANGED | DeathRecipient 架构：cliHook_ 与 functionHook_ 各自独立 DeathRecipient（cliHookDeathRecipient_ + functionHookDeathRecipient_） | v3.0 变更 CHG-5 |
| ADDED | Hook 注册安全校验：系统应用校验（`IsSystemAppByFullTokenID`）+ 权限校验（`ohos.permission.REGISTER_AGENT_HOOK`） | v3.0 变更 CHG-6 |
| ADDED | 所有 hook 回调接口 + ICliToolManager Before/AfterInvokeFunction 新增 `HookOption` 参数（`int32_t dmSessionId = -1`, `int32_t toolCallId = -1`） | v3.0 变更 CHG-7 |
| CHANGED | NAPI 层 Stub 缓存：同类型 Stub 已存在则直接返回异常不 IPC 到服务侧；增加 JS 参数校验 | v3.0 变更 CHG-8 |
| ADDED | SDK .d.ts 新增 @permission 标注（`ohos.permission.REGISTER_AGENT_HOOK`）和 @throws 错误码说明 | v3.0 变更 CHG-9 |
| CHANGED | JS 侧错误码调整：不暴露 `ERR_INVALID_PARAM(35700002)`；系统内部原因（非调用方原因）导致的异常返回 `35600050`（system error）；修正 unregisterCliHook/unregisterFunctionHook `@throws` 错误码与 spec 一致 | v3.0 变更 CHG-10 |
| CHANGED | Hook 方法可选：RegisterCliHook/RegisterFunctionHook 新增 `int activeMethods` 位掩码（0=全部有效）；service 按掩码跳过未激活方法；JS 校验改为"至少 1 个方法存在"；.d.ts 方法改为可选 | v4.0 变更 CHG-11 |
| CHANGED | Hook 调用异步化：service 用 FFRT 任务包裹同步 IPC + condition_variable + 6s 超时；超时用原始数据继续；NAPI Stub 超时从 500ms 统一为 5000ms | v4.0 变更 CHG-12 |
| CHANGED | HookOption JS 字段可选：`.d.ts` 改为 `dmSessionId?: number` + `toolCallId?: number`；NAPI 构建 JS 对象时仅当值 ≠ -1 才设置 | v4.0 变更 CHG-13（v5.0 CHG-16 已移除 HookOption） |
| CHANGED | 移除 @throws 401 标注：401 是默认参数错误码，不需显式标注 | v5.0 变更 CHG-14 |
| CHANGED | unregister 改为传 hook 对象 + NAPI 用 napi_strict_equals 校验同一性 | v5.0 变更 CHG-15 |
| DELETED | HookOption 参数从所有接口移除（.d.ts/IDL/service/NAPI） | v5.0 变更 CHG-16 |
| CHANGED | Hook 方法名改 on 前缀：onBeforeCallTool/onAfterCallTool/onBeforeCallCmd/onAfterCallCmd/onBeforeInvokeFunction/onAfterInvokeFunction | v5.0 变更 CHG-17 |
| CHANGED | FunctionHook 参数封装：onBeforeInvokeFunction 接收 InvokeFunctionParam 结构体；onAfterInvokeFunction 只接收 InvokeResult | v5.0 变更 CHG-18 |
| CHANGED | Hook 参数结构体对齐原始函数签名：ExecToolParam（toolName+subCommand+args+challenge+execOptions?）/ ExecCmdParam（cmd+execCmdOptions?） | v5.0 变更 CHG-19 |
| CHANGED | C++ 接口重命名：ICliHookScheduler→ICliHookInterface、IFunctionHookScheduler→IFunctionHookInterface | v6.0 变更 CHG-20/21 |
| CHANGED | C++ AfterCall* 参数封装为 ExecResultWrap；BeforeInvokeFunction 参数封装为 InvokeFunctionParam；AfterInvokeFunction 参数封装为 FunctionResultWrap + 去掉 namespace/functionName | v6.0 变更 CHG-22/23/24 |
| CHANGED | NAPI Stub 类重命名：JsCliHookSchedulerStub→JsCliHook、JsFunctionHookSchedulerStub→JsFunctionHook | v6.0 变更 CHG-25/26 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `.specs/features/agent-hook/proposal.md` | 条件通过（待 Q-1~Q-5 确认） |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。

## 用户故事

### US-1: 注册 cliHook 拦截 tool/cmd 执行

**作为** Agent 框架开发者,
**我想要** 注册 cliHook 拦截 ExecTool/ExecCmd 执行前后的参数与结果,
**以便** 实现命令审计、参数改写、策略阻断等横切逻辑。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN 开发模式下调用方调用 RegisterCliHook(cliHook) 且 cliHook_ 未注册 THEN service 保存 Hook 并返回 ERR_OK | 正常 |
| AC-1.2 | WHEN RegisterCliHook 成功后任意调用方调用 ExecTool(param) THEN BeforeCallTool 在 ExecTool 执行前被回调，入参为当前 ExecToolParam | 正常 |
| AC-1.3 | WHEN BeforeCallTool 回调返回改写后的 ExecToolParam（无论返回 ERR_OK 还是非零） THEN service 使用改写后的参数执行 ExecTool（全部 hook 模式：返回值不影响 Hook 应用） | 正常 |
| AC-1.4 | WHEN ExecTool 执行完成 THEN AfterCallTool 被回调，入参为执行结果 ExecResultWrap | 正常 |
| AC-1.5 | WHEN AfterCallTool 回调返回改写后的 ExecResultWrap（无论返回 ERR_OK 还是非零） THEN service 将改写后的结果封装回 CliSessionInfo 返回给原调用方（全部 hook 模式） | 正常 |
| AC-1.6 | WHEN RegisterCliHook 成功后任意调用方调用 ExecCmd(param) THEN BeforeCallCmd/AfterCallCmd 在 ExecCmd 执行前后被回调 | 正常 |
| AC-1.7 | WHEN 授权调用方调用 UnregisterCliHook THEN 后续 ExecTool/ExecCmd 不再触发回调 | 正常 |
| AC-1.8 | WHEN 非开发模式（`const.security.developermode.state`=false）THEN RegisterCliHook 返回 ERR_NOT_DEVELOPER_MODE(35700021)，不保存 Hook | 异常 |
| AC-1.9 | WHEN cliHook 已注册 THEN 再次调用 RegisterCliHook 返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式注册，必须先 Unregister） | 异常 |
| AC-1.10 | WHEN 无 Hook 注册或非开发模式 THEN ExecTool/ExecCmd/invokeFunction 执行链路与未引入 Hook 前完全一致（无额外 IPC 开销，回调点跳过） | 边界 |
| AC-1.11 | WHEN Hook 注册方进程死亡 THEN service 自动注销 cliHook，后续 ExecTool/ExecCmd 不触发回调 | 异常 |

### US-2: 注册 functionHook 拦截 invokeFunction

**作为** Agent 框架开发者,
**我想要** 注册 functionHook 拦截 invokeFunction 执行前后的参数与结果,
**以便** 实现函数调用审计、参数改写、结果脱敏。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-2.1 | WHEN 开发模式下调用方调用 RegisterFunctionHook(functionHook) 且 functionHook_ 未注册 THEN service 保存 Hook 并返回 ERR_OK | 正常 |
| AC-2.2 | WHEN RegisterFunctionHook 成功后任意调用方调用 invokeFunction THEN 调用方调用 BeforeInvokeFunction（IPC 到 service），service 内部分发到 functionHook 回调 | 正常 |
| AC-2.3 | WHEN BeforeInvokeFunction 回调返回改写后的 param（无论返回值） THEN 调用方使用改写后的参数执行 invokeFunction（全部 hook 模式） | 正常 |
| AC-2.4 | WHEN invokeFunction 执行完成 THEN 调用方调用 AfterInvokeFunction（IPC 到 service），service 内部分发到 functionHook 回调入参为 InvokeFunctionResult | 正常 |
| AC-2.5 | WHEN AfterInvokeFunction 回调返回改写后的结果（无论返回值） THEN 调用方将改写后的结果返回给上层（全部 hook 模式） | 正常 |
| AC-2.6 | WHEN 授权调用方调用 UnregisterFunctionHook THEN 后续 invokeFunction 的 BeforeInvokeFunction/AfterInvokeFunction 返回 ERR_OK（零开销） | 正常 |
| AC-2.7 | WHEN 非开发模式 THEN RegisterFunctionHook 返回 ERR_NOT_DEVELOPER_MODE(35700021) | 异常 |
| AC-2.8 | WHEN functionHook 已注册 THEN 再次调用 RegisterFunctionHook 返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式注册） | 异常 |
| AC-2.9 | WHEN functionHook 注册方进程死亡 THEN service 侧注销 functionHook；后续 BeforeInvokeFunction/AfterInvokeFunction service 直接返回 ERR_OK（零开销） | 异常 |
| AC-2.10 | WHEN 无 functionHook 注册 THEN BeforeInvokeFunction/AfterInvokeFunction 返回 ERR_OK，invokeFunction 执行链路与引入前一致 | 边界 |

### US-3: 开发模式门控与安全校验

**作为** 系统管理员,
**我想要** 限制 Hook 仅在开发模式生效且仅授权系统应用可注册,
**以便** 生产环境零开销且无安全风险。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-3.1 | WHEN `const.security.developermode.state`=false（非开发模式）THEN RegisterCliHook/RegisterFunctionHook 返回 ERR_NOT_DEVELOPER_MODE(35700021)，不保存 Hook | 异常 |
| AC-3.2 | WHEN `const.security.developermode.state`=true（开发模式）THEN RegisterCliHook/RegisterFunctionHook 正常工作 | 正常 |
| AC-3.3 | WHEN 非开发模式 + 已注册的 Hook 存在（开发模式切换为非开发模式前已注册）THEN ExecTool/ExecCmd/BeforeInvokeFunction/AfterInvokeFunction 回调点跳过（零开销） | 边界 |
| AC-3.4 | WHEN 非系统应用调用 RegisterCliHook/RegisterFunctionHook/UnregisterCliHook/UnregisterFunctionHook THEN 返回 ERR_NOT_SYSTEM_APP(35700008) | 异常 |
| AC-3.5 | WHEN 系统应用但无 `ohos.permission.REGISTER_AGENT_HOOK` 权限调用 Register/Unregister Hook THEN 返回 ERR_PERMISSION_DENIED(35700007) | 异常 |
| AC-3.6 | WHEN 系统应用 + 有权限 + 开发模式 THEN Register/Unregister Hook 正常工作 | 正常 |

### US-4: NAPI 层 JS 接口（v2.0 新增）

**作为** Agent 框架开发者,
**我想要** 通过 JS 接口注册 CliHook/FunctionHook 回调对象,
**以便** 在 JS 层实现命令审计、参数改写、结果脱敏。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-4.1 | WHEN JS 调用方调用 `cliManager.registerCliHook(hook)` 传入实现 CliHook interface 的 JS 对象 THEN NAPI 层将 JS 对象封装为 IPC Stub 并注册到 CliToolManagerService，返回成功 | 正常 |
| AC-4.2 | WHEN JS CliHook 对象的 onBeforeCallTool 被回调 THEN 回调在 JS 线程执行，JS 修改 param 后 NAPI 层将改写后 param 通过 IPC 返回给 service | 正常 |
| AC-4.3 | WHEN JS 调用方调用 `cliManager.registerFunctionHook(hook)` 传入实现 FunctionHook interface 的 JS 对象 THEN NAPI 层封装为 IPC Stub 并注册成功 | 正常 |
| AC-4.4 | WHEN JS FunctionHook 对象的 onBeforeInvokeFunction/onAfterInvokeFunction 被回调 THEN 回调在 JS 线程执行，改写后的 param/result 通过 IPC 返回 | 正常 |
| AC-4.5 | WHEN JS 回调方法抛出异常或执行超时（>5s）THEN NAPI 层返回原 param/result（未改写），不阻塞 IPC 1s 默认超时 | 异常 |
| AC-4.6 | WHEN JS 调用方调用 `cliManager.unregisterCliHook(hook)`/`unregisterFunctionHook(hook)` THEN service 注销 Hook，后续不再触发 JS 回调 | 正常 |
| AC-4.7 | WHEN JS 调用方调用 `registerCliHook(hook)` 且同类型 Stub 已存在 THEN NAPI 层直接返回异常（不 IPC 到服务侧） | 异常 |
| AC-4.8 | WHEN JS 调用方调用 `registerFunctionHook(hook)` 且同类型 Stub 已存在 THEN NAPI 层直接返回异常（不 IPC 到服务侧） | 异常 |
| AC-4.9 | WHEN JS 调用方调用 `registerCliHook`/`registerFunctionHook` 传入非对象或无任何回调方法 THEN NAPI 层返回参数错误（不 IPC 到服务侧） | 异常 |
| AC-4.10 | WHEN JS 调用方调用 `unregisterCliHook(hook)`/`unregisterFunctionHook(hook)` 但本地无 Stub 缓存 THEN NAPI 层直接返回 `ERR_HOOK_NOT_REGISTERED(35700023)`（不 IPC 到服务侧） | 异常 |
| AC-4.11 | WHEN registerCliHook/registerFunctionHook/unregisterCliHook/unregisterFunctionHook 因系统内部原因（非调用方原因，如 IPC 失败、service 未启动等）失败 THEN NAPI 层返回 `35600050`（system error），不暴露 `ERR_INVALID_PARAM(35700002)` | 异常 |
| AC-4.12 | WHEN JS 调用方调用 `registerCliHook(hook)` 传入仅含 `onBeforeCallTool` 方法的对象 THEN NAPI 层根据 JS 对象方法存在性构建位掩码传入 service；service 仅对已激活方法发起 IPC 回调，未激活方法跳过 | 正常 |
| AC-4.13 | WHEN hook 回调在 6s 内未返回 THEN service 使用原始数据继续执行，不阻塞主链路；FFRT task 后续完成的结果被丢弃 | 异常 |
| ~~AC-4.14~~ | ~~已废弃（v5.0 CHG-16 HookOption 移除）~~ | — |

### US-5: HookOption 追踪参数（v3.0 新增，v5.0 已移除）

> **v5.0 CHG-16：HookOption 参数已从所有接口中移除。** dmSessionId/toolCallId 追踪需求后续如需要再单独设计。本节保留历史记录，AC-5.1~5.3 标记为废弃。

**作为** Agent 框架开发者,
**我想要** 在 Hook 回调中获取 dmSessionId 和 toolCallId 追踪信息,
**以便** 关联 Hook 回调与具体的 DM 会话和工具调用上下文。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| ~~AC-5.1~~ | ~~已废弃（v5.0 HookOption 移除）~~ | — |
| ~~AC-5.2~~ | ~~已废弃（v5.0 HookOption 移除）~~ | — |
| ~~AC-5.3~~ | ~~已废弃（v5.0 HookOption 移除）~~ | — |

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1 | TASK-AH-002 | 单测 | `test/unittest/cli_tool_mgr_service_test/` |
| AC-1.2 | R-2 | TASK-AH-003 | 单测 | 同上 |
| AC-1.3 | R-3 | TASK-AH-003 | 单测 | 同上 |
| AC-1.4 | R-4 | TASK-AH-003 | 单测 | 同上 |
| AC-1.5 | R-5 | TASK-AH-003 | 单测 | 同上 |
| AC-1.6 | R-6 | TASK-AH-004 | 单测 | 同上 |
| AC-1.7 | R-7 | TASK-AH-005 | 单测 | 同上 |
| AC-1.8 | R-8 | TASK-AH-012 | 单测 | 同上 |
| AC-1.9 | R-9 | TASK-AH-002 | 单测 | 同上 |
| AC-1.10 | R-10 | TASK-AH-011 | 单测 | 同上 |
| AC-1.11 | R-11 | TASK-AH-005 | 单测 | 同上 |
| AC-2.1 | R-12 | TASK-AH-006 | 单测 | `test/unittest/invoke_function_executor_test/` |
| AC-2.2 | R-13 | TASK-AH-007/008 | 单测 | 同上 |
| AC-2.3 | R-14 | TASK-AH-008 | 单测 | 同上 |
| AC-2.4 | R-15 | TASK-AH-007/008 | 单测 | 同上 |
| AC-2.5 | R-16 | TASK-AH-008 | 单测 | 同上 |
| AC-2.6 | R-17 | TASK-AH-006 | 单测 | 同上 |
| AC-2.7 | R-8 | TASK-AH-012 | 单测 | 同上 |
| AC-2.8 | R-9 | TASK-AH-006 | 单测 | 同上 |
| AC-2.9 | R-18 | TASK-AH-009 | 单测 | 同上 |
| AC-2.10 | R-19 | TASK-AH-011 | 单测 | 同上 |
| AC-3.1 | R-8 | TASK-AH-012 | 单测 | 同上 |
| AC-3.2 | R-20 | TASK-AH-012 | 单测 | 同上 |
| AC-3.3 | R-21 | TASK-AH-012 | 单测 | 同上 |
| AC-4.1 | R-22 | TASK-AH-015 | 单测 | `test/unittest/cli_tool_manager_napi_test/` |
| AC-4.2 | R-23 | TASK-AH-016 | 单测 | 同上 |
| AC-4.3 | R-24 | TASK-AH-015 | 单测 | 同上 |
| AC-4.4 | R-25 | TASK-AH-016 | 单测 | 同上 |
| AC-4.5 | R-26 | TASK-AH-016 | 单测 | 同上 |
| AC-4.6 | R-27 | TASK-AH-015 | 单测 | 同上 |
| AC-4.7 | R-28 | TASK-AH-019 | 单测 | 同上 |
| AC-4.8 | R-29 | TASK-AH-019 | 单测 | 同上 |
| AC-4.9 | R-30 | TASK-AH-019 | 单测 | 同上 |
| AC-4.10 | R-31 | TASK-AH-019 | 单测 | 同上 |
| AC-4.11 | R-36 | TASK-AH-019 | 单测 | 同上 |
| AC-3.4 | R-32 | TASK-AH-017 | 单测 | 同上 |
| AC-3.5 | R-33 | TASK-AH-017 | 单测 | 同上 |
| AC-3.6 | R-34 | TASK-AH-017 | 单测 | 同上 |
| AC-5.1 | R-35 | TASK-AH-018 | 单测 | 同上 |
| AC-5.2 | R-35 | TASK-AH-018 | 单测 | 同上 |
| AC-5.3 | R-35 | TASK-AH-018 | 单测 | 同上 |
| AC-4.12 | R-37 | TASK-AH-022 | 单测 | 同上 |
| AC-4.13 | R-38 | TASK-AH-023 | 单测 | 同上 |
| AC-4.14 | R-39 | TASK-AH-024 | 单测 | 同上 |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 开发模式 + cliHook_ 未注册 + 调用 RegisterCliHook(cliHook) | service 保存 Hook Proxy，注册 DeathRecipient，返回 ERR_OK | Hook Proxy 非空 | AC-1.1 |
| R-2 | 行为 | cliHook 已注册 + 任意调用方调用 ExecTool(param) | 在 ValidateExecToolPermissions 成功之后回调 BeforeCallTool(currentParam) | 回调为同步 IPC + 6s 超时；dev mode 门控；hook 在鉴权后执行 | AC-1.2 |
| R-3 | 行为 | BeforeCallTool 回调返回（无论 ERR_OK 还是非零） | 全部 hook 模式：service 使用回调改写后的 param 执行 | 改写值需通过 ValidateAndPrepareTool 校验 | AC-1.3 |
| R-4 | 行为 | ExecTool 执行完成 | 回调 AfterCallTool(currentExecResultWrap) | 入参为 ExecResultWrap；同步 IPC | AC-1.4 |
| R-5 | 行为 | AfterCallTool 回调返回 | 全部 hook 模式：service 将改写后 ExecResultWrap 填充回 CliSessionInfo.result | 改写值需可序列化 | AC-1.5 |
| R-6 | 行为 | cliHook 已注册 + 调用 ExecCmd(param) | 在 ValidateExecToolPermissions 成功之后回调 BeforeCallCmd，执行完成后回调 AfterCallCmd | ExecCmd 无 skill 分派，直接 CreateShellProcess；hook 在鉴权后执行 | AC-1.6 |
| R-7 | 行为 | 调用 UnregisterCliHook | hook 匹配时 service 移除 Hook + 注销 DeathRecipient；hook 不匹配时返回 ERR_HOOK_NOT_REGISTERED | 无 | AC-1.7 |
| R-8 | 异常 | 非开发模式调用 Register*Hook | 返回 ERR_NOT_DEVELOPER_MODE(35700021)，不保存 Hook | `const.security.developermode.state`=false | AC-1.8/AC-2.7/AC-3.1 |
| R-9 | 异常 | 已注册时再次调用 Register*Hook | 返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式注册） | 必须先 Unregister | AC-1.9/AC-2.8 |
| R-10 | 边界 | 无 Hook 注册或非开发模式 | ExecTool/ExecCmd/BeforeInvokeFunction/AfterInvokeFunction 回调点跳过，零开销 | 无额外 IPC | AC-1.10/AC-2.10 |
| R-11 | 恢复 | cliHook 注册方进程死亡 | DeathRecipient 触发，service 移除 Hook | 恢复到 R-10 状态 | AC-1.11 |
| R-12 | 行为 | 开发模式 + functionHook_ 未注册 + 调用 RegisterFunctionHook | service 保存 Hook，注册 DeathRecipient，返回 ERR_OK | Hook 非空 | AC-2.1 |
| R-13 | 行为 | functionHook 已注册 + 调用 invokeFunction | 调用方调 BeforeInvokeFunction（IPC 到 service），service 校验 ACCESS_FUNCTION 权限后分发到 functionHook 回调 | service 内部判断有 functionHook 则 IPC 分发；PermissionUtil::CheckSystemAndPermission(ACCESS_FUNCTION) 校验系统应用或 SA + ACCESS_FUNCTION | AC-2.2 |
| R-14 | 行为 | BeforeInvokeFunction 回调返回改写后 param | 全部 hook 模式：调用方使用改写后 param 执行 | 改写值需通过 functionType 校验 | AC-2.3 |
| R-15 | 行为 | invokeFunction 执行完成 | 调用方调 AfterInvokeFunction（IPC 到 service），service 分发到 functionHook 回调 | 同步 IPC | AC-2.4 |
| R-16 | 行为 | AfterInvokeFunction 回调返回改写后 result | 全部 hook 模式：调用方将改写后 result 返回给上层 | 改写值需可序列化 | AC-2.5 |
| R-17 | 行为 | 调用 UnregisterFunctionHook | service 移除 Hook；后续 Before/After 返回 ERR_OK | 无 | AC-2.6 |
| R-18 | 恢复 | functionHook 注册方进程死亡 | DeathRecipient 触发，service 移除 Hook；后续 Before/After 返回 ERR_OK | 恢复到 R-19 状态 | AC-2.9 |
| R-19 | 边界 | 无 functionHook 注册 | BeforeInvokeFunction/AfterInvokeFunction 返回 ERR_OK，invokeFunction 链路与引入前一致 | service 直接返回 ERR_OK（零开销） | AC-2.10 |
| R-20 | 正常 | 开发模式 + 调用 Register*Hook | 正常工作 | `const.security.developermode.state`=true | AC-3.2 |
| R-21 | 边界 | 非开发模式 + 已注册 Hook 存在 | 回调点跳过（零开销） | dev mode 切换前已注册的 Hook 不回调 | AC-3.3 |
| R-22 | 行为 | JS 调用 registerCliHook(hook) 传入 CliHook 对象 | NAPI 封装为 IPC Stub，注册到 service | napi_ref + threadsafe_function | AC-4.1 |
| R-23 | 行为 | JS CliHook onBeforeCallTool 被回调 | 回调在 JS 线程执行，改写后 param 通过 IPC 返回 | threadsafe_function 同步分发 | AC-4.2 |
| R-24 | 行为 | JS 调用 registerFunctionHook(hook) 传入 FunctionHook 对象 | NAPI 封装为 IPC Stub，注册到 service | 同 R-22 | AC-4.3 |
| R-25 | 行为 | JS FunctionHook onBeforeInvokeFunction/onAfterInvokeFunction 被回调 | 回调在 JS 线程执行，改写后通过 IPC 返回 | 同 R-23 | AC-4.4 |
| R-26 | 异常 | JS 回调抛异常或超时（>5s） | NAPI 返回原 param/result，不阻塞 IPC 1s 超时 | future.wait_for(5000ms) 兜底 | AC-4.5 |
| R-27 | 行为 | JS 调用 unregisterCliHook(hook)/unregisterFunctionHook(hook) 传入 hook 对象 | NAPI 用 napi_strict_equals 校验与本地 g_cliHookStub/g_functionHookStub 同一性，匹配则 IPC 到 service 注销，清空本地 Stub | 传 hook 对象 + napi_strict_equals | AC-4.6 |
| R-28 | 异常 | NAPI 层同类型 Stub 已存在时调用 registerCliHook | NAPI 层直接返回异常，不 IPC 到服务侧 | Stub 缓存检查在 NAPI 层完成 | AC-4.7 |
| R-29 | 异常 | NAPI 层同类型 Stub 已存在时调用 registerFunctionHook | NAPI 层直接返回异常，不 IPC 到服务侧 | 同 R-28 | AC-4.8 |
| R-30 | 异常 | JS 调用 registerCliHook/registerFunctionHook 传入非对象或缺少回调方法 | NAPI 层返回参数错误，不 IPC 到服务侧 | argc + napi_typeof + 方法存在性检查 | AC-4.9 |
| R-31 | 异常 | JS 调用 unregisterCliHook(hook)/unregisterFunctionHook(hook) 但本地无 Stub 缓存 | NAPI 层直接返回 ERR_HOOK_NOT_REGISTERED(35700023)，不 IPC 到服务侧 | g_cliHookStub/g_functionHookStub 为 null | AC-4.10 |
| R-32 | 异常 | 非系统应用且非 SA 调用 Register/Unregister Hook | 返回 ERR_NOT_SYSTEM_APP(35700008) | `IsSystemAppByFullTokenID` 或 `TOKEN_NATIVE` 校验 | AC-3.4 |
| R-33 | 异常 | 系统应用或 SA 但无 `ohos.permission.REGISTER_AGENT_HOOK` 权限 | 返回 ERR_PERMISSION_DENIED(35700007) | `PermissionUtil::VerifyAccessToken` 校验 | AC-3.5 |
| R-34 | 正常 | 系统应用 + 有权限 + 开发模式 | Register/Unregister 正常工作 | 三层校验依次通过 | AC-3.6 |
| ~~R-35~~ | ~~行为~~ | ~~hook 回调被触发~~ | ~~回调入参含 HookOption~~ | ~~已废弃（v5.0 CHG-16 HookOption 移除）~~ | ~~AC-5.1/5.2/5.3~~ |
| R-36 | 异常 | registerCliHook/registerFunctionHook/unregisterCliHook/unregisterFunctionHook 因系统内部原因失败（IPC 失败、service 未启动等，非调用方原因） | NAPI 层返回 35600050（system error），不暴露 ERR_INVALID_PARAM(35700002) | `CreateCliJsErrorByNativeErr` 映射：未匹配到特定业务错误码的原生错误统一返回 35600050 | AC-4.11 |
| R-37 | 行为 | RegisterCliHook/RegisterFunctionHook 传入 `activeMethods` 位掩码 | service 按掩码跳过未激活方法——未激活方法不发 IPC 回调，对应方法保持原始数据 | 位掩码 0=全部有效；NAPI 层根据 JS 对象方法存在性构建掩码 | AC-4.12 |
| R-38 | 异常 | hook 回调在 6s 内未返回（FFRT task 超时） | service 使用原始数据继续执行，不阻塞主链路；FFRT task 后续完成时结果被丢弃（通过 shared_ptr 生命周期管理） | `condition_variable::wait_for(6s)` 超时分支；NAPI Stub 内部 5s 超时（服务端 > 客户端确保不竞态） | AC-4.13 |
| ~~R-39~~ | ~~边界~~ | ~~JS HookOption 对象未设置字段~~ | ~~NAPI 层不报错~~ | ~~已废弃（v5.0 CHG-16 HookOption 移除）~~ | ~~AC-4.14~~ |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1/R-12/AC-1.1/AC-2.1 | 单测 | dev mode + 未注册时注册成功 |
| VM-2 | R-2~R-6/AC-1.2~1.6 | 单测 | cliHook 回调触发与改写生效 |
| VM-3 | R-7/R-17/AC-1.7/AC-2.6 | 单测 | 注销后不触发 |
| VM-4 | R-8/AC-1.8/AC-2.7/AC-3.1 | 单测 | 非开发模式注册失败 |
| VM-5 | R-9/AC-1.9/AC-2.8 | 单测 | 已注册时失败式注册 |
| VM-6 | R-10/R-19/AC-1.10/AC-2.10 | 单测 | 无 Hook 时零开销 |
| VM-7 | R-11/R-18/AC-1.11/AC-2.9 | 单测 | 进程死亡自动注销 |
| VM-8 | R-13~R-16/AC-2.2~2.5 | 单测 | functionHook service 分发与回调 |
| VM-9 | R-20/R-21/AC-3.2/AC-3.3 | 单测 | dev mode 门控 |
| VM-10 | R-22~R-31/R-36~R-39/AC-4.1~4.14 | 单测 | NAPI 层 JS 回调 + Stub 缓存 + JS 参数校验 + 系统内部错误 35600050 + 方法可选 + 异步超时 + HookOption 可选 |
| VM-11 | R-32~R-34/AC-3.4~3.6 | 单测 | 系统应用 + 权限校验 |
| VM-12 | R-35/R-39/AC-5.1~5.3/AC-4.14 | 单测 | HookOption 透传 + JS 字段可选 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `ICliToolManager::RegisterCliHook` | SystemAPI | `ICliHookInterface hook, int activeMethods` | `int32_t` ERR_OK/ERR_INVALID_PARAM/ERR_NOT_SYSTEM_APP/ERR_PERMISSION_DENIED/ERR_NOT_DEVELOPER_MODE/ERR_HOOK_ALREADY_REGISTERED | 0/35700002/35700008/35700007/35700021/35700022 | 注册 cliHook（dev mode + 系统应用或 SA + 权限 + 失败式 + 位掩码方法可选） | AC-1.1/1.8/1.9/3.4~3.6/4.12 |
| `ICliToolManager::UnregisterCliHook` | SystemAPI | `ICliHookInterface hook` | `int32_t` | 0/35700008/35700007 | 注销 cliHook（系统应用 + 权限） | AC-1.7/3.4/3.5 |
| `ICliToolManager::RegisterFunctionHook` | SystemAPI | `IFunctionHookInterface hook, int activeMethods` | `int32_t` | 0/35700002/35700008/35700007/35700021/35700022 | 注册 functionHook（dev mode + 系统应用 + 权限 + 失败式 + 位掩码方法可选） | AC-2.1/2.7/2.8/3.4~3.6/4.12 |
| `ICliToolManager::UnregisterFunctionHook` | SystemAPI | `IFunctionHookInterface hook` | `int32_t` | 0/16001001/35700008/35700007 | 注销 functionHook（系统应用 + 权限） | AC-2.6/3.4/3.5 |
| `ICliToolManager::BeforeInvokeFunction` | SystemAPI | `[in,out] InvokeFunctionParam param` | `int32_t` | 0/35700008/35700007 | function 执行前回调（service 内部分发到 functionHook；权限 ACCESS_FUNCTION） | AC-2.2/2.3 |
| `ICliToolManager::AfterInvokeFunction` | SystemAPI | `[in,out] FunctionResultWrap functionResultWrap` | `int32_t` | 0/35700008/35700007 | function 执行后回调（service 内部分发；权限 ACCESS_FUNCTION） | AC-2.4/2.5 |
| `ICliHookInterface::BeforeCallTool` | SystemAPI | `[in,out] ExecToolParam param` | `void` | - | tool 执行前回调 | AC-1.2/AC-1.3 |
| `ICliHookInterface::AfterCallTool` | SystemAPI | `[in,out] ExecResultWrap execResultWrap` | `void` | - | tool 执行后回调 | AC-1.4/AC-1.5 |
| `ICliHookInterface::BeforeCallCmd` | SystemAPI | `[in,out] ExecCmdParam param` | `void` | - | cmd 执行前回调 | AC-1.6 |
| `ICliHookInterface::AfterCallCmd` | SystemAPI | `[in,out] ExecResultWrap execResultWrap` | `void` | - | cmd 执行后回调 | AC-1.6 |
| `IFunctionHookInterface::BeforeInvokeFunction` | SystemAPI | `[in,out] InvokeFunctionParam param` | `void` | - | function 执行前回调（注册方实现） | AC-2.2/AC-2.3 |
| `IFunctionHookInterface::AfterInvokeFunction` | SystemAPI | `[in,out] FunctionResultWrap functionResultWrap` | `void` | - | function 执行后回调 | AC-2.4/AC-2.5 |
| `cliManager.registerCliHook` | SystemAPI (JS) | `CliHook hook` | `Promise<void>` | 201/202/35700021/35700022/35600050 | JS 注册 cliHook（@permission REGISTER_AGENT_HOOK；系统内部错误返回 35600050） | AC-4.1/4.7/4.9/4.11 |
| `cliManager.unregisterCliHook` | SystemAPI (JS) | `CliHook hook` | `Promise<void>` | 201/202/35700023/35600050 | JS 注销 cliHook（传 hook 对象 + napi_strict_equals 校验；系统内部错误返回 35600050） | AC-4.6/4.10/4.11 |
| `cliManager.registerFunctionHook` | SystemAPI (JS) | `FunctionHook hook` | `Promise<void>` | 201/202/35700021/35700022/35600050 | JS 注册 functionHook（系统内部错误返回 35600050） | AC-4.3/4.8/4.9/4.11 |
| `cliManager.unregisterFunctionHook` | SystemAPI (JS) | `FunctionHook hook` | `Promise<void>` | 201/202/35700023/35600050 | JS 注销 functionHook（传 hook 对象 + napi_strict_equals 校验；系统内部错误返回 35600050） | AC-4.6/4.10/4.11 |
| `interface CliHook` | SystemAPI (JS) | 4 个 on 前缀回调方法（onBeforeCallTool/onAfterCallTool/onBeforeCallCmd/onAfterCallCmd），无 HookOption 参数 | - | - | JS 回调 interface | AC-4.2 |
| `interface FunctionHook` | SystemAPI (JS) | 2 个 on 前缀回调方法（onBeforeInvokeFunction/onAfterInvokeFunction），无 HookOption 参数 | - | - | JS 回调 interface | AC-4.4 |
| ~~`struct HookOption`~~ | ~~SystemAPI~~ | ~~`int32_t dmSessionId, int32_t toolCallId`~~ | - | - | ~~已移除（v5.0 CHG-16）~~ | ~~AC-5.1~5.3~~ |
| `ohos.permission.REGISTER_AGENT_HOOK` | SystemAPI | N/A | N/A | N/A | 管控 Hook 注册/注销行为 | AC-3.5/3.6 |

### 删除 API

| API 名称 | 原开放范围 | 删除原因 |
|----------|----------|----------|
| `ICliToolManager::GetFunctionHook` | SystemAPI | v2.0 CHG-3：由 BeforeInvokeFunction/AfterInvokeFunction 替代，调用方不再查询 Hook |

### 变更/废弃 API

| API 名称 | 变更类型 | 说明 |
|----------|----------|------|
| `ICliToolManager::RegisterCliHook` | 修改（v3.0） | 新增系统应用 + 权限校验（CHG-6） |
| `ICliToolManager::UnregisterCliHook` | 修改（v3.0） | 新增系统应用 + 权限校验（CHG-6） |
| `ICliToolManager::RegisterFunctionHook` | 修改（v3.0） | 新增系统应用 + 权限校验（CHG-6） |
| `ICliToolManager::UnregisterFunctionHook` | 修改（v3.0） | 新增系统应用 + 权限校验（CHG-6） |
| `ICliToolManager::BeforeInvokeFunction` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `ICliToolManager::AfterInvokeFunction` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `ICliHookInterface::BeforeCallTool` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `ICliHookInterface::AfterCallTool` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `ICliHookInterface::BeforeCallCmd` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `ICliHookInterface::AfterCallCmd` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `IFunctionHookInterface::BeforeInvokeFunction` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |
| `IFunctionHookInterface::AfterInvokeFunction` | 修改（v3.0） | 新增 `HookOption` 参数（CHG-7） |

> **前置依赖：InvokeFunctionResult Parcelable 化。** `AfterInvokeFunction([in,out] InvokeFunctionResult result)` IDL 参数要求 Parcelable。当前 `InvokeFunctionResult`（`invoke_function_executor.h:36`）为普通 struct，需改为继承 `Parcelable` + 新增 `Marshalling`/`Unmarshalling` + IDL `sequenceable` 声明。详见 design.md ADR-9。

> API 签名、IDL 位置、权限要求等实现细节见 design.md。

## 接口规格

### 接口定义

**ICliToolManager::RegisterCliHook**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t ICliToolManager::RegisterCliHook(const sptr<ICliHookInterface> &hook, int32_t activeMethods)` |
| 返回值 | `int32_t` — ERR_OK(0) 成功；ERR_INVALID_PARAM(35700002) hook 为空；ERR_NOT_SYSTEM_APP(35700008) 非系统应用且非 SA；ERR_PERMISSION_DENIED(35700007) 无权限；ERR_NOT_DEVELOPER_MODE(35700021) 非开发模式；ERR_HOOK_ALREADY_REGISTERED(35700022) 已注册 |
| 开放范围 | SystemAPI |
| 权限要求 | `ohos.permission.REGISTER_AGENT_HOOK`（v3.0 新增） |
| 错误码 | 0 / 35700002 / 35700007 / 35700008 / 35700021 / 35700022 |
| 关联 AC | AC-1.1/1.8/1.9/3.4/3.5/3.6/4.12 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| hook | `sptr<ICliHookInterface>` | 是 | N/A | 非空；已注册则返回失败（必须先 Unregister） |
| activeMethods | `int32_t` | 是 | 0 | 位掩码：bit0=BeforeCallTool, bit1=AfterCallTool, bit2=BeforeCallCmd, bit3=AfterCallCmd；0=全部有效；NAPI 层根据 JS 对象方法存在性自动构建 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 系统应用 + 有权限 + 开发模式 + hook 非空 + cliHook_ 未注册 | 保存 hook + cliHookActiveMethods_，注册 cliHookDeathRecipient_，返回 ERR_OK | AC-1.1 |
| 2 | 非系统应用 | 返回 ERR_NOT_SYSTEM_APP | AC-3.4 |
| 3 | 系统应用但无权限 | 返回 ERR_PERMISSION_DENIED | AC-3.5 |
| 4 | 非开发模式 | 返回 ERR_NOT_DEVELOPER_MODE(35700021)，不保存 | AC-1.8 |
| 5 | hook 为空 | 返回 ERR_INVALID_PARAM | AC-1.1 |
| 6 | cliHook_ 已注册 | 返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式，必须先 Unregister） | AC-1.9 |
| 7 | activeMethods 非 0 且某 bit 未设置 | service 在对应 Invoke* 方法中检查掩码跳过（不发 IPC） | AC-4.12 |

**ICliToolManager::BeforeInvokeFunction**

| 属性 | 值 |
|------|-----|
| 函数签名 | `int32_t ICliToolManager::BeforeInvokeFunction(const std::string &functionNamespace, const std::string &functionName, AAFwk::WantParams &param, const HookOption &option)` |
| 返回值 | `int32_t` — ERR_OK(0)（无论是否有 Hook）；ERR_CLITOOL_SERVICE_NOT_READY(16001002) service 未启动 |
| 开放范围 | SystemAPI |
| 错误码 | 0 / 16001002 |
| 关联 AC | AC-2.2/2.3/2.10/5.2 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 开发模式 + functionHook 已注册 | service 调用 functionHook_->BeforeInvokeFunction（IPC 到注册方，含 option 透传），返回改写后 param | AC-2.2/5.2 |
| 2 | 无 functionHook 或非开发模式 | 直接返回 ERR_OK，param 不变（零开销） | AC-2.10 |

**ICliHookInterface::BeforeCallTool**

| 属性 | 值 |
|------|-----|
| 函数签名 | `void ICliHookInterface::BeforeCallTool(ExecToolParam &param, const HookOption &option)` |
| 返回值 | `void`（全部 hook 模式：修改后的 param 始终生效） |
| 开放范围 | SystemAPI（由 Hook 注册方实现） |
| 关联 AC | AC-1.2/AC-1.3/AC-5.3 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| param | `ExecToolParam&` | 是 | 调用方传入的原始值 | Hook 可改写 toolName/subcommand/challenge/options/args；改写值需通过 ValidateAndPrepareTool 校验 |
| option | `HookOption` | 是 | dmSessionId=-1, toolCallId=-1 | 追踪参数，当前 -1 占位；后续调用方生成对应值 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | ExecTool 被调用 + cliHook 已注册 | 同步回调 BeforeCallTool(param, option)，service 等待返回 | AC-1.2/5.3 |
| 2 | 回调返回 + 改写 param | service 用改写后的 param 执行（全部 hook 模式） | AC-1.3 |
| 3 | 无 cliHook | 不触发回调 | AC-1.9 |

**ICliHookInterface::AfterCallTool**

| 属性 | 值 |
|------|-----|
| 函数签名 | `void ICliHookInterface::AfterCallTool(ExecResult &result, const HookOption &option)` |
| 返回值 | `void`（全部 hook 模式：修改后的 result 始终生效） |
| 开放范围 | SystemAPI（由 Hook 注册方实现） |
| 关联 AC | AC-1.4/AC-1.5/AC-5.3 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| result | `ExecResult&` | 是 | 执行完成后的原始结果 | Hook 可改写 exitCode/outputText/errorText/signalNumber/timeout/executionTime；改写值需可序列化 |
| option | `HookOption` | 是 | dmSessionId=-1, toolCallId=-1 | 追踪参数，当前 -1 占位 |

**行为场景**

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | ExecTool 执行完成 + cliHook 已注册 | 同步回调 AfterCallTool(result, option)，service 等待返回 | AC-1.4/5.3 |
| 2 | 回调返回 + 改写 result | service 将改写后的 ExecResult 封装回 CliSessionInfo 返回给原调用方（全部 hook 模式） | AC-1.5 |

## 兼容性声明

- **已有 API 行为变更:** 否。全新增方法，既有 ExecTool/ExecCmd/invokeFunction 行为在无 Hook 时不变。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。
- **最低支持版本:** OpenHarmony-6.0-Release
- **API 版本号策略:** 新增 API 标注 `@since 6.0`

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 仅开发模式生效 | `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断；非开发模式注册返回 ERR_NOT_DEVELOPER_MODE(35700021)，回调点跳过（零开销） | AC-1.1/1.8/2.1/2.7/3.1~3.3 |
| 失败式注册 | 已注册则返回 ERR_HOOK_ALREADY_REGISTERED(35700022)，必须先 Unregister 再 Register | AC-1.9/2.8 |
| Hook 回调为同步 IPC（service 侧 6s 超时 + NAPI 侧 5000ms 超时） | Hook 注册方实现 Stub，service/调用方持有 Proxy；回调为同步 IPC，FFRT 包裹 + 6s 超时兜底；IPC 默认 1s 兜底 | AC-1.2/AC-2.2 |
| cliHook 在 service 进程内触发 | ExecTool/ExecCmd 执行链路在 CliToolManagerService 内，Hook 回调由 service 发起 | AC-1.2/AC-1.4/AC-1.6 |
| functionHook 在 service 内部分发 | 调用方调 BeforeInvokeFunction/AfterInvokeFunction（IPC 到 service），service 内部分发到 functionHook | AC-2.2/AC-2.4 |
| 无 Hook 时零开销 | Hook 为空时 service 直接返回 ERR_OK，调用方无感知 | AC-1.10/AC-2.10 |
| NAPI JS 回调同步分发 | JS 对象封装为 IPC Stub，binder 线程→JS 线程同步（future+threadsafe_function），超时 5s 兜底（v4.0 从 500ms 统一为 5s） | AC-4.2/4.4/4.5/4.13 |
| 每个 hook 独立 DeathRecipient | cliHook_ 和 functionHook_ 各自独立 DeathRecipient（cliHookDeathRecipient_ + functionHookDeathRecipient_），各自仅清除自身 Hook | AC-1.11/AC-2.9 |
| 系统应用 + 权限校验 | Register/Unregister Hook 入口校验 `IsSystemAppByFullTokenID` + `PermissionUtil::VerifyAccessToken(REGISTER_AGENT_HOOK)` | AC-3.4/3.5/3.6 |
| ~~HookOption 透传~~ | ~~已废弃（v5.0 CHG-16 HookOption 移除）~~ | ~~AC-5.1/5.2/5.3~~ |
| NAPI Stub 缓存 | NAPI 层保存 Stub 引用，同类型已存在则直接返回 ERR_HOOK_ALREADY_REGISTERED(35700022) 不 IPC；unregister 传 hook 对象，NAPI 用 napi_strict_equals 校验同一性后 IPC 注销（v5.0 CHG-15）；无 Stub 则返回 ERR_HOOK_NOT_REGISTERED(35700023) 不 IPC | AC-4.6/4.7/4.8/4.10 |
| JS 侧系统内部错误码 | JS 侧不暴露 `ERR_INVALID_PARAM(35700002)`；系统内部原因导致的异常 NAPI 层返回 `35600050`（system error）；@throws 401 不显式标注（v5.0 CHG-14，401 是默认参数错误码） | AC-4.11 |
| Hook 方法可选（位掩码） | RegisterCliHook/RegisterFunctionHook 新增 `int activeMethods` 位掩码（0=全部有效）；service 按掩码跳过未激活方法的 IPC 调用；NAPI 层根据 JS 对象方法存在性构建掩码；.d.ts 方法改为可选（`?`）；方法名统一 on 前缀（v5.0 CHG-17） | AC-4.12 |
| Hook 调用异步化 + 6s 超时 | service 用 `InvokeHookAsync` 模板：`shared_ptr<ParamType> paramCopy` + `shared_ptr<ffrt::mutex/cv/atomic<bool>>` + `ffrt::submit` + `cv->wait_for(chrono::seconds(6))`；超时用原始数据继续；FFRT task 后续完成时 paramCopy 被 shared_ptr 安全丢弃；NAPI `HOOK_TIMEOUT_MS` 5000ms；IDL 不变 | AC-4.13 |
| FunctionHook 参数封装（v5.0） | onBeforeInvokeFunction 接收 InvokeFunctionParam 结构体（functionNamespace + functionName + args + invokeOptions）；onAfterInvokeFunction 只接收 InvokeResult | — |
| Hook 参数对齐原始函数（v5.0） | ExecToolParam 镜像 execTool 签名（toolName + subCommand + args + challenge + execOptions?）；ExecCmdParam 镜像 execCmd 签名（cmd + execCmdOptions?） | — |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | Hook 回调 IPC 时延 P95 < 50ms（正常负载）；超时阈值 5s，超时后用原始数据继续 | benchmark + 单测 | 待补 |
| 性能 | BeforeInvokeFunction/AfterInvokeFunction 两跳 IPC 时延 P95 < 50ms（有 Hook 时）；无 Hook 时零开销 | benchmark | 待补 |
| 安全 | 仅开发模式 + 系统应用 + `ohos.permission.REGISTER_AGENT_HOOK` 权限可注册 Hook | 权限扫描 + 单测 | 待补 |
| 可靠性 | Hook 注册方进程死亡后 1s 内自动注销 | 压力测试 | 待补 |
| 可测试性 | Hook 回调用 mock Stub 可在单测中验证 | 单测 | `test/unittest/` |
| 定界定位 | Hook 回调触发/超时/注销记录 HiSysEvent | hilog | 待补 |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | N/A | 同主验证 | N/A |
| 平板 | 无差异 | N/A | 同主验证 | N/A |
| 折叠屏 | 无差异 | N/A | 同主验证 | N/A |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否/N-A | Hook 能力无 UI | N/A |
| 大字体 | 否/N/A | 无 UI | N/A |
| 深色模式 | 否/N/A | 无 UI | N/A |
| 多窗口/分屏 | 否/N/A | Hook 能力与窗口模式无关 | N/A |
| 多用户 | 是 | Hook 为系统级，跨用户生效；仅开发模式可注册 | 多用户场景下 Hook 跨用户拦截 |
| 版本升级 | 否/N/A | 全新能力 | N/A |
| 生态兼容 | 否/N/A | 不影响既有应用 | N/A |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符（Owner 字段在 manifest 中，此处不涉及）
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（做什么/不做什么清晰）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致（每个 AC 至少关联一条规则，每条规则至少关联一个 AC）
- [x] 规则表每条通过 5 项质量检查（可复现/可观测/边界值/关联AC/无冲突）

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "cli_tool_framework ExecTool/ExecCmd 执行链路、InvokeFunctionExecutor::DoExecute、ICliToolManager IDL、PermissionUtil 权限校验模式"
```

**关键文档：** `agenthook_design.txt`、`AGENTS.md`、`.codespec/changes/issue-15382-cli-shell/design.md`
