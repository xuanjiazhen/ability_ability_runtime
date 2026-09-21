---
id: agent-hook
type: feature
title: "AgentHook 能力：授权调用方注册 Hook 拦截 tool/cmd/function 参数与结果"
spec_schema: ohos-sdd/v1
profile: security-sensitive
complexity: standard
lineage: new
status: implementing
owner: ""
source_issue: ""
created_at: 2026-08-25
updated_at: 2026-09-17
related: []
related_tasks: []
related_decisions: []
subprofiles: []
profile_source: owner
code_refs:
  - foundation/ability/ability_runtime/cli_tool_framework
  - interface/sdk-js
commits:
  - ed511499220481262b1465face549ec25315588b  # Add agentHook
  - 173bff3741bb15a93bce11d027cd5e0b10fe369f  # Tdd fix
  - 63b07b300502acd5bebdcf648c6e4f2bbfd254fd  # Code review fix
---

## 备注

- 主类型 `security-sensitive`：本能力允许授权调用方修改其它调用方的 tool/cmd/function 参数与结果，属于安全敏感路径。
- **v2.0 基线变更（2026-09-03）：** CHG-1 门控改为 `system::GetBoolParameter` 运行时判断（移除宏隔离）；CHG-2 注册改为失败式（已注册返回失败）；CHG-3 functionHook 调用路径改为 service 直接分发（移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction）；CHG-4 新增 NAPI 层（JS 系统接口，3 个 JS interface，JS 对象封装为 RemoteObject，回调到 JS 线程）。
- **v5.0 基线变更（2026-09-11）：** CHG-14 移除 @throws 401；CHG-15 unregister 改为传 hook 对象 + napi_strict_equals 校验同一性；CHG-16 HookOption 参数完全移除（.d.ts/IDL/service/NAPI）；CHG-17 hook 方法名改 on 前缀；CHG-18 FunctionHook beforeInvokeFunction 封装为 InvokeFunctionParam 结构体 + afterInvokeFunction 简化为只接收 InvokeResult；CHG-19 ExecToolParam/ExecCmdParam 对齐原始函数签名。
- **v6.0 基线变更（2026-09-11）：** CHG-20 ICliHookScheduler→ICliHookInterface；CHG-21 IFunctionHookScheduler→IFunctionHookInterface；CHG-22 AfterCall*参数封装为 ExecResultWrap；CHG-23 BeforeInvokeFunction参数封装为 InvokeFunctionParam；CHG-24 AfterInvokeFunction参数封装为 FunctionResultWrap + 去掉 namespace/functionName；CHG-25 JsCliHookSchedulerStub→JsCliHook；CHG-26 JsFunctionHookSchedulerStub→JsFunctionHook。
- 仓内路径：`cli_tool_framework/**` + `interface/sdk-js/**`。
- 澄清状态：Q-1~Q-5（v1.0）+ Q-6~Q-8（v2.0 新增）+ Q-9~Q-11（v3.0 新增）全部已确认。
- 适用范围：仅开发模式（`const.security.developermode.state`）生效，非开发模式注册失败 + 回调跳过。v3.0 新增系统应用 + 权限校验作为第二层安全门控。
