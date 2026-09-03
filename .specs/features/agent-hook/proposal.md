---
target_release:
  id: OpenHarmony-6.0-Release
  status: changed
---

# 需求文档

> 从 `agenthook_design.txt` 粗方案固化而来的需求基线。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-AGENT-HOOK |
| 需求名称 | AgentHook：授权调用方注册 Hook 拦截 tool/cmd/function 参数与结果 |
| 来源 | 内部设计稿 `agenthook_design.txt` |
| 提出人 | 待确认 |
| 目标发行版本 | OpenHarmony-6.0-Release |
| 候选 Profile | security-sensitive |
| 优先级 | P1 |
| 状态 | Draft |

### 原始描述

**原始问题：** 当前 cli_tool_framework 的 `ExecTool`/`ExecCmd`（在 CliToolManagerService 内执行）和 `invokeFunction`（在调用方进程内执行）都是直通路径——调用方传参 → 服务/执行器执行 → 返回结果。没有一种机制允许"授权第三方"在执行前后插入自定义逻辑：修改入参、修改出参、甚至根据策略阻断执行。Agent 框架需要这种"Hook"能力来实现命令审计、参数改写、结果脱敏、策略阻断等横切场景。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| Agent 框架开发者 | 无法在不修改 cli_tool_framework 源码的前提下插入执行前/后逻辑 | 必须侵入式改源码，维护成本高 |
| 应用开发者（调用 tool/cmd） | 无法被授权第三方审计或改写其命令参数 | 缺少合规/安全审计能力 |
| 系统管理员 | 无法对敏感命令做策略阻断或结果脱敏 | 安全合规缺口 |

**期望结果：** 授权调用方可通过 CliToolManagerService 注册 Hook；Hook 在 tool/cmd/function 执行前后被回调，可修改参数与结果；cliHook 在 service 内回调，functionHook 在 invokeFunction 调用方进程内回调（需每次查询）。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 设计稿 | `agenthook_design.txt` | 粗方案，定义接口签名与 cliHook/functionHook 区分 |
| 既有实现 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | ExecTool/ExecCmd/RegisterFunction 等接口已实现 |
| 既有实现 | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp` | invokeFunction 在调用方进程执行的证据 |
| 既有 IPC 接口 | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 现有 ICliToolManager 契约 |

### 初始范围

**可能包含（v1.0 原始，以 section 四 v2.0 变更为准）：**
- CliToolManagerService 新增 `RegisterCliHook`/`UnregisterCliHook`/`RegisterFunctionHook`/`UnregisterFunctionHook` + `BeforeInvokeFunction`/`AfterInvokeFunction` 六个接口（v2.0：移除 `GetFunctionHook`，新增 Before/AfterInvokeFunction）
- ICliToolManager IDL 新增对应 IPC 方法
- ICliHookScheduler / IFunctionHookScheduler 两个回调 IPC 接口（Hook 注册方实现 Stub，service/调用方持有 Proxy）
- CliHook 在 `ExecTool`/`ExecCmd` 执行链路插入 before/after 回调点
- FunctionHook 在 service 内部分发（v2.0：调用方调 `BeforeInvokeFunction`/`AfterInvokeFunction` 到 service，service 分发到 functionHook）
- 开发模式门控：`system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断（v2.0：移除宏隔离）
- NAPI 层：JS 系统接口，3 个 JS interface（v2.0 新增）
- 注册失败语义：已注册返回 `ERR_INVALID_VALUE`（v2.0：从覆盖改为失败式）

**明确不包含：**
- 宏隔离 / 特性开关 `ability_runtime_agent_hook`（v2.0：移除，改为运行时 dev mode 门控）
- Hook 的 JS/NAPI 绑定层的延迟（v2.0：已包含 NAPI 层）
- Hook 链式注册（多 Hook 顺序执行）—— v1 仅支持单 Hook 注册
- Hook 的持久化（注册方进程死亡即注销）
- 跨设备 Hook 同步

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| tool/cmd 执行均在 CliToolManagerService 进程内完成 | 技术 | 源码 `cli_tool_manager_service.cpp` 的 `ExecTool`/`ExecCmd` 实现 | 已验证 |
| invokeFunction 在调用方进程内完成，仅通过 `GetFunctionInfo` IPC 查询函数元数据 | 技术 | 源码 `invoke_function_executor.cpp` 的 `DoExecute` | 已验证 |
| Hook 注册方与 service/调用方分属不同进程，需 IPC 回调 | 技术 | 既有 `ICliToolManagerScheduler` 回调模式 | 已验证 |
| 仅系统应用可注册 Hook | 业务/安全 | v2.0 变更：改为 dev mode 门控（`const.security.developermode.state`），不校验权限 | 已验证（v2.0） |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 单仓（ability_ability_runtime），新增 IPC 接口 + 回调链路，无跨仓依赖 |
| 涉及仓数量 | 1 | ability_ability_runtime |
| 是否涉及 Public/System API | 是 | 新增 System API（ICliToolManager 扩展） |
| 是否涉及安全/性能关键路径 | 是 | Hook 可修改其它调用方的参数与结果，属安全敏感路径 |
| 是否跨 SIG | 否 | 仅 ability SIG |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [x] 需求来源和责任人已明确（设计稿，Owner 待确认）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断（标准）

---

## 二、澄清记录

> 粗方案已有明确接口签名与架构区分。以下为 AI 一次性补全字段后请需求方确认（标准级允许 2 轮内确认）。如有歧义将升级为逐轮澄清。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | `afterCallCli(toolParam)` 中参数名仍为 toolParam，是否表示修改的是"工具执行后的最终输出参数"而非"执行结果对象"？还是笔误应为 toolResult？ | 影响 Hook 接口签名设计——参数类型决定 Hook 能改写什么 | 已澄清：原始输入写错，afterCallCli 入参为结果（ExecResult） |
| Q-2 | cliHook 的四个方法是方法重载（同名 `beforeCallCli`/`afterCallCli` 按参数类型区分）还是四个不同名方法？ | 影响 IDL 定义（IDL 不支持重载，需不同名） | 已澄清：异名（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd） |
| Q-3 | Hook 返回值语义：before 钩子返回非 ERR_OK 是否阻断执行？after 钩子返回非 ERR_OK 是否覆盖原结果？ | 影响执行链路控制流 | 已澄清：全部 hook 模式——非零也 hook，返回值不影响 Hook 应用（修改后的 param/result 始终生效） |
| Q-4 | functionHook 的 `GetFunctionHook` 返回的是单个 Hook 还是 Hook 列表？ | v1 是否支持多 Hook | 已澄清：单 Hook，最多只存在一个 functionHook。v2.0：移除 GetFunctionHook，注册语义改为失败式（已注册返回 ERR_INVALID_VALUE） |
| Q-5 | 权限名建议：`ohos.permission.REGISTER_CLI_HOOK` 是否合适？ | 安全边界 | 已澄清：按照推荐（REGISTER_CLI_HOOK + ACCESS_FUNCTION） |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | Hook 回调用独立 IPC 接口（ICliHookScheduler / IFunctionHookScheduler），注册方实现 Stub，service/调用方持有 Proxy | 职责清晰，与既有 ICliToolManagerScheduler 模式一致 | 新增 2 个 IDL 文件 | 推荐 |
| A-2 | Hook 回调复用 ICliToolManagerScheduler，通过 event type 区分 | 不新增 IDL | 回调接口臃肿，event 携带 Hook 语义复杂 | 放弃 |

**取舍理由：** A-1 与既有 `ICliToolManagerScheduler` 回调模式一致，职责清晰，IDL 可读性好。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 `cli_tool_manager_service.h` | ExecTool/ExecCmd 是否在 service 内 | 是，`ExecTool`/`ExecCmd` 为 `CliToolManagerService` 成员 | 高 | 设计 cliHook 回调点 | 命中 |
| K-2 | 源码 `invoke_function_executor.cpp` | invokeFunction 执行位置 | `DoExecute` 在 ffrt 任务中执行，通过 `CliToolMGRClient::GetFunctionInfo` IPC 查询后本地执行 | 高 | 设计 functionHook 查询+回调点 | 命中 |
| K-3 | IDL `ICliToolManager.idl` | 现有 IPC 契约 | 已有 ExecTool/ExecCmd/RegisterFunction 等 | 高 | 扩展新接口 | 命中 |
| K-4 | IDL `ICliToolManagerScheduler.idl` | 回调模式 | `[oneway] void Scheduler*ReplyEvent(...)` 回调模式 | 高 | Hook 回调接口设计参考 | 命中 |
| K-5 | AGENTS.md | 安全约束 | `services/common/include/permission_verification.h` 是全局安全入口；cli_tool_framework 有独立 PermissionUtil | 高 | 权限校验设计 | 命中 |
| K-6 | 既有 design `.codespec/changes/issue-15382-cli-shell/design.md` | 设计模式 | 四层分层架构；回调用独立 Scheduler Stub | 高 | 设计风格一致性 | 命中 |

**上下文结论：**
- 高可信结论：cliHook 在 service 回调、functionHook 在调用方进程回调、Hook 回调用独立 IPC 接口——均可直接进入设计
- 已确认结论：Q-1 afterCallCli 入参为结果（ExecResult）；Q-2 异名；Q-3 全部 hook 模式（非零也 hook，返回值不影响 Hook 应用）；Q-4 单 Hook（最多一个 functionHook）；Q-5 权限名按推荐（REGISTER_CLI_HOOK + ACCESS_FUNCTION）
- 待确认结论：无（5 个澄清问题已全部确认）
- 未使用来源：无

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | AppFwk（cli_tool_framework） | 待确认 | 已识别 |
| 是否需要新增子系统或部件？ | 否 | 待确认 | 已识别 |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 待确认 | 已识别 |
| 是否需要新增 System API？ | 是，6 个 C++ 方法 + 4 个 JS 函数 + 2 个回调接口 + 2 个 JS interface（v2.0：移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction + NAPI 层） | 待确认 | 已识别 |
| 是否会废弃已有 API？ | 否 | 待确认 | 已识别 |
| 是否需要新增权限声明？ | 否（v2.0：改为 dev mode 门控，不校验权限） | 待确认 | 已识别（v2.0 变更） |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | 向前/向后兼容要求？ | 全新增方法，不破坏既有 IPC 序列化格式 | 待确认 | 已识别 |
| 性能 | 回调时延/并发要求？ | Hook 回调为 IPC oneway/sync，不应阻塞主链路超 100ms；functionHook 每次查询增加一次 IPC | 待确认 | 待确认 |
| 安全 | 权限/隐私/审计要求？ | 仅授权调用方可注册 Hook；Hook 可修改参数/结果需审计日志 | 待确认 | 已识别 |
| 可靠性 | 注册方进程死亡时 Hook 清理？ | 复用既有 DeathRecipient 模式，进程死亡自动注销 | 待确认 | 已识别 |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| ICliToolManager IDL 扩展 | 编译 | 新增 5 方法 + 2 回调接口 | 已识别 |
| PermissionUtil 扩展 | 运行 | 新增 Hook 注册权限校验 | 已识别 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| Hook 回调阻塞主链路 | 技术 | 中 | 回调为同步 IPC，不设超时；由注册方保证回调快速返回；IPC 默认 1s 兜底 | 已识别 |
| functionHook 每次查询增加 IPC 时延 | 性能 | 中 | 可加进程内缓存（注册方死亡时失效）| 已识别 |
| Hook 注册方与执行方竞态 | 技术 | 中 | Hook 注册/注销用 mutex；执行链路持 shared_ptr 快照 | 已识别 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估
- [x] 兼容性和非功能需求已确认（性能阈值待 benchmark；本能力仅开发环境启用，生产环境不编译）
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索

**结论:** 条件通过（待 Q-1~Q-5 确认后转通过）

---

## 三、需求基线

> 澄清完成后固化。manifest.md 是事实源，此处为审批结论。

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-25 |
| Owner | 待确认 |
| 确认人 | 待确认 |
| 复杂度 | 标准 |
| Profile | security-sensitive |
| 目标发行版本 | OpenHarmony-6.0-Release |
| 版本状态 | proposed |

### 问题陈述

cli_tool_framework 缺少"授权第三方在执行前后插入自定义逻辑"的 Hook 能力。Agent 框架需要此能力实现命令审计、参数改写、结果脱敏、策略阻断。当前必须侵入式改源码才能插入横切逻辑，维护成本高且无法运行时动态配置。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 授权调用方可注册 cliHook 拦截 tool/cmd 执行 | 注册后 ExecTool/ExecCmd 前后触发回调 | 单测 mock Hook 回调 |
| 授权调用方可注册 functionHook 拦截 invokeFunction | 注册后 invokeFunction 前后触发回调 | 单测 mock Hook 回调 |
| Hook 可修改参数与结果 | before 回调入参被改写后服务端用改写值执行；after 回调返回值覆盖原结果 | 单测断言改写生效 |
| 非授权调用方注册 Hook 被拒绝 | 返回权限错误码 | 单测无权限场景 |
| 注册方进程死亡后 Hook 自动注销 | 进程死亡后 ExecTool 不再触发回调 | 单测模拟 death |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为 Agent 框架开发者，我想要注册 cliHook 拦截 tool/cmd 执行前后，以便实现命令审计与策略阻断 | P0 |
| US-2 | 作为 Agent 框架开发者，我想要注册 functionHook 拦截 invokeFunction 执行前后，以便实现函数调用审计与参数改写 | P0 |
| US-3 | 作为系统管理员，我想要限制仅授权调用方可注册 Hook，以便防止恶意应用劫持命令执行 | P0 |
| US-4 | 作为 Hook 注册方，我想要进程死亡后自动注销 Hook，以便避免回调悬空 | P1 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 授权调用方调用 RegisterCliHook(cliHook) THEN service 保存 Hook 并返回 ERR_OK | 正常 | US-1 |
| AC-2 | WHEN 授权调用方调用 RegisterCliHook(cliHook) 后任意调用方调用 ExecTool(param) THEN beforeCallCli 在 ExecTool 执行前被回调，参数为当前 ExecToolParam | 正常 | US-1 |
| AC-3 | WHEN beforeCallCli 回调返回改写后的 ExecToolParam THEN service 使用改写后的参数执行 ExecTool | 正常 | US-1 |
| AC-4 | WHEN ExecTool 执行完成 THEN afterCallCli 被回调，参数为执行结果（CliSessionInfo） | 正常 | US-1 |
| AC-5 | WHEN afterCallCli 返回改写后的结果 THEN service 将改写后的结果返回给原调用方 | 正常 | US-1 |
| AC-6 | WHEN 授权调用方调用 RegisterCliHook(cliHook) 后任意调用方调用 ExecCmd(param) THEN beforeCallCli/afterCallCli 在 ExecCmd 执行前后被回调（cmdParam/cmdResult 重载） | 正常 | US-1 |
| AC-7 | WHEN 授权调用方调用 UnregisterCliHook(cliHook) THEN 后续 ExecTool/ExecCmd 不再触发回调 | 正常 | US-1 |
| AC-8 | WHEN 授权调用方调用 RegisterFunctionHook(functionHook) THEN service 保存 Hook 并返回 ERR_OK | 正常 | US-2 |
| AC-9 | WHEN RegisterFunctionHook 成功后任意调用方调用 invokeFunction THEN 调用方先经 GetFunctionHook 查询 Hook，再在 beforeInvokeFunction 回调后执行 | 正常 | US-2 |
| AC-10 | WHEN beforeInvokeFunction 回调返回改写后的 functionParam THEN 调用方使用改写后的参数执行 invokeFunction | 正常 | US-2 |
| AC-11 | WHEN invokeFunction 执行完成 THEN afterInvokeFunction 被回调，参数为执行结果 | 正常 | US-2 |
| AC-12 | WHEN afterInvokeFunction 返回改写后的结果 THEN 调用方将改写后的结果返回给上层 | 正常 | US-2 |
| AC-13 | WHEN 非授权调用方调用 RegisterCliHook/RegisterFunctionHook THEN 返回权限错误码（ERR_PERMISSION_DENIED） | 异常 | US-3 |
| AC-14 | WHEN Hook 注册方进程死亡 THEN service 自动注销 cliHook，后续 ExecTool/ExecCmd 不触发回调 | 异常 | US-4 |
| AC-15 | WHEN Hook 注册方进程死亡 THEN functionHook 在 service 侧注销；调用方 GetFunctionHook 返回"无 Hook" | 异常 | US-4 |
| AC-16 | WHEN 无 Hook 注册 THEN ExecTool/ExecCmd/invokeFunction 执行链路与未引入 Hook 能力前完全一致（无额外 IPC 开销） | 边界 | US-1/US-2 |
| AC-17 | WHEN beforeCallCli 回调超时（>100ms） THEN service 跳过 Hook，继续执行原链路，记录 HiSysEvent | 边界 | US-1 |

### 范围边界

**包含：** ICliToolManager IDL 扩展 5 方法 + 2 回调接口；CliToolManagerService 实现 RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/GetFunctionHook；ExecTool/ExecCmd 链路插入 cliHook 回调点；InvokeFunctionExecutor 插入 functionHook 查询与回调点；PermissionUtil 新增权限校验；DeathRecipient 自动注销。

**不包含：** NAPI 绑定层；Hook 链式注册；Hook 持久化；跨设备 Hook 同步；静态接口。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| AppFwk | ability_ability_runtime | `cli_tool_framework/interfaces/cli_tool/` | ICliToolManager IPC 契约 | 新增方法+新增回调 IDL | 待确认 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/interfaces/cli_tool/include/` | 客户端 CliToolMGRClient | 新增 5 方法 | 待确认 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/services/climgr/` | CliToolManagerService | 新增实现+ExecTool/ExecCmd 插入回调点 | 待确认 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/frameworks/js/napi/function_manager/` | InvokeFunctionExecutor | 插入 functionHook 查询与回调点 | 待确认 |
| AppFwk | ability_ability_runtime | `cli_tool_framework/services/common/` | PermissionUtil | 新增权限校验 | 待确认 |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| ICliToolManager::RegisterCliHook | 新增 | SystemAPI | 注册 cliHook（tool/cmd 执行前后回调） |
| ICliToolManager::UnregisterCliHook | 新增 | SystemAPI | 注销 cliHook |
| ICliToolManager::RegisterFunctionHook | 新增 | SystemAPI | 注册 functionHook（invokeFunction 前后回调） |
| ICliToolManager::UnregisterFunctionHook | 新增 | SystemAPI | 注销 functionHook |
| ICliToolManager::GetFunctionHook | 新增 | SystemAPI | 查询当前 functionHook（供 invokeFunction 调用方进程使用） |
| ICliHookScheduler | 新增 | SystemAPI | cliHook 回调 IPC 接口（beforeCallCli/afterCallCli 重载 tool/cmd） |
| IFunctionHookScheduler | 新增 | SystemAPI | functionHook 回调 IPC 接口（beforeInvokeFunction/afterInvokeFunction） |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 是 | Hook 回调时延、functionHook 查询 IPC 开销 | design.md |
| 安全与权限 | 是 | 仅授权调用方可注册 Hook；Hook 可改写参数/结果 | design.md + spec.md |
| 兼容性 | 否 | 全新增方法，不破坏既有 IPC | N/A |
| API/SDK | 是 | 5 个新 SystemAPI + 2 个回调接口 | design.md + spec.md |
| IPC/跨进程 | 是 | Hook 回调为跨进程 IPC；functionHook 查询为跨进程 IPC | design.md |
| 构建与部件 | 是 | 新增特性开关 `ability_runtime_agent_hook`（默认 false，仅开发环境启用）；复用 cli_tool_framework 既有部件 | design.md |
| 国际化/无障碍 | 否 | 无 UI 文案 | N/A |
| 数据迁移 | 否 | 全新能力 | N/A |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增 NAPI 绑定或多 Hook 链 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改 Hook 返回值语义或阻断语义 | 重新审批基线和 spec |
| API 变更 | 修改 Public API | 触发设计审批 |
| 非功能指标变更 | Hook 回调超时阈值变化 | 重新确认测试计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [x] `proposal.target_release` 已确认或明确 TBD
- [x] `manifest.profile` 已确认或明确 security-sensitive
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭（Q-1~Q-5 全部已确认）
- [x] 上下文与知识源检索日志已填写
- [x] 目标仓 Agent 指南已检查（AGENTS.md 已读）

**基线结论:** 通过（Q-1~Q-5 全部已确认）

---

## 四、基线变更记录（v2.0 — 2026-09-03 重新基线）

> 本节记录基线 v1.0 通过后的范围变更。触发原因：用户提出 4 项方案调整，涉及门控方式、注册语义、functionHook 调用路径和 NAPI 层新增。按变更控制规则"AC 变更 → 重新审批基线和 Spec"执行重新基线。

### 变更项

| 编号 | 变更内容 | 原方案 | 新方案 | 影响范围 |
|------|----------|--------|--------|----------|
| CHG-1 | 门控方式 | `#ifdef ABILITY_RUNTIME_AGENT_HOOK` 宏隔离 + `ability_runtime_agent_hook=false` 特性开关 | `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断，仅开发模式生效 | design ADR-8、spec AC-3.5/3.6、ability_runtime.gni、BUILD.gn |
| CHG-2 | 注册语义 | 单 Hook 覆盖语义（重复注册覆盖旧 Hook） | 如果已存在已注册的 Hook 则返回失败（`ERR_INVALID_VALUE`），必须先 Unregister 再 Register | design ADR-4、spec AC-1.1/2.1/接口规格行为场景 4 |
| CHG-3 | functionHook 调用路径 | 调用方经 `GetFunctionHook` IPC 查询 Hook Proxy → 调用方本地回调 `BeforeInvokeFunction`/`AfterInvokeFunction` | 移除 `GetFunctionHook`；在 `ICliToolManager` 直接提供 `BeforeInvokeFunction`/`AfterInvokeFunction` 两个方法，调用方 IPC 到 service，service 内部分发到已注册 functionHook | design ADR-5、spec AC-2.2/2.9/3.4、ICliToolManager.idl、invoke_function_executor |
| CHG-4 | NAPI 层新增 | 不包含 NAPI 绑定层 | 新增 NAPI 层：JS 系统接口，参数复用已有 JS API 类型；新增 3 个 Hook JS interface；JS 对象内部封装为 RemoteObject（IPC Stub），回调时到 JS 线程执行 | design ADR-9（新增）、spec 新增 AC、sdk-js 目录、cli_tool_framework NAPI 层 |

### 变更后的范围边界

**包含（v2.0 更新）：**
- ICliToolManager IDL：`RegisterCliHook`/`UnregisterCliHook`/`RegisterFunctionHook`/`UnregisterFunctionHook` + `BeforeInvokeFunction`/`AfterInvokeFunction`（移除 `GetFunctionHook`）
- ICliHookScheduler/IFunctionHookScheduler 回调 IDL
- CliToolManagerService：Hook 存储 + `ExecTool`/`ExecCmd`/`BeforeInvokeFunction`/`AfterInvokeFunction` 回调点 + `system::GetBoolParameter` 开发模式门控
- InvokeFunctionExecutor：调用 `CliToolMGRClient::BeforeInvokeFunction`/`AfterInvokeFunction`（IPC 到 service）
- NAPI 层：JS callback 对象封装为 IPC Stub（`JsCliHookSchedulerStub`/`JsFunctionHookSchedulerStub`），同步回调到 JS 线程
- SDK JS：3 个新 interface（`CliHook`、`FunctionHook`、`cliManager` 注册函数）+ `@kit.AbilityKit` 导出
- 注册失败语义：已注册则返回 `ERR_INVALID_VALUE`

**不包含：**
- 宏隔离 / 特性开关（`ability_runtime_agent_hook`）
- Hook 链式注册（多 Hook）
- Hook 持久化
- 跨设备 Hook 同步

### 变更后的 API 清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| ICliToolManager::RegisterCliHook | 新增 | SystemAPI | 注册 cliHook（已存在则失败） |
| ICliToolManager::UnregisterCliHook | 新增 | SystemAPI | 注销 cliHook |
| ICliToolManager::RegisterFunctionHook | 新增 | SystemAPI | 注册 functionHook（已存在则失败） |
| ICliToolManager::UnregisterFunctionHook | 新增 | SystemAPI | 注销 functionHook |
| ICliToolManager::BeforeInvokeFunction | 新增 | SystemAPI | function 执行前回调（service 内部分发到 functionHook） |
| ICliToolManager::AfterInvokeFunction | 新增 | SystemAPI | function 执行后回调（service 内部分发到 functionHook） |
| ~~ICliToolManager::GetFunctionHook~~ | 删除 | ~~SystemAPI~~ | v1.0 中定义，v2.0 移除（由 Before/After 方法替代） |
| ICliHookScheduler | 新增 | SystemAPI | cliHook 回调接口（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd） |
| IFunctionHookScheduler | 新增 | SystemAPI | functionHook 回调接口（BeforeInvokeFunction/AfterInvokeFunction） |
| cliManager.registerCliHook | 新增 | SystemAPI (JS) | JS 注册 cliHook |
| cliManager.unregisterCliHook | 新增 | SystemAPI (JS) | JS 注销 cliHook |
| cliManager.registerFunctionHook | 新增 | SystemAPI (JS) | JS 注册 functionHook |
| cliManager.unregisterFunctionHook | 新增 | SystemAPI (JS) | JS 注销 functionHook |
| CliHook (interface) | 新增 | SystemAPI (JS) | JS 回调接口：beforeCallTool/afterCallTool/beforeCallCmd/afterCallCmd |
| FunctionHook (interface) | 新增 | SystemAPI (JS) | JS 回调接口：beforeInvokeFunction/afterInvokeFunction |

### 变更后的澄清状态

| 编号 | 问题 | 变更后状态 |
|------|------|-----------|
| Q-1 | afterCallCli 入参 | 不变：ExecResult |
| Q-2 | 方法命名 | 不变：异名 |
| Q-3 | Hook 返回值语义 | 不变：全部 hook 模式 |
| Q-4 | 单 Hook 还是多 Hook | 变更：仍为单 Hook，但注册语义从"覆盖"改为"已存在则失败" |
| Q-5 | 权限名 | 不变：REGISTER_CLI_HOOK + ACCESS_FUNCTION |
| Q-6（新增） | 门控方式 | 已确认：`system::GetBoolParameter("const.security.developermode.state", false)`，仅开发模式生效，不用宏隔离 |
| Q-7（新增） | functionHook 调用路径 | 已确认：移除 GetFunctionHook，直接提供 BeforeInvokeFunction/AfterInvokeFunction 在 ICliToolManager 上 |
| Q-8（新增） | NAPI 层 | 已确认：提供 NAPI 层，JS 系统接口，参数复用已有 JS API 类型，3 个 JS interface，JS 对象内部封装为 RemoteObject，回调到 JS 线程 |

### 变更后的基线结论

**结论:** 条件通过（v2.0 变更项 CHG-1~CHG-4 已明确，待用户确认后转通过）
