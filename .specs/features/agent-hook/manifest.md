---
id: agent-hook
type: feature
title: "AgentHook 能力：授权调用方注册 Hook 拦截 tool/cmd/function 参数与结果"
spec_schema: ohos-sdd/v1
profile: security-sensitive
complexity: standard
lineage: new
status: designing
owner: ""
source_issue: ""
created_at: 2026-08-25
updated_at: 2026-09-03
related: []
related_tasks: []
related_decisions: []
subprofiles: []
profile_source: owner
code_refs:
  - foundation/ability/ability_runtime/cli_tool_framework
  - interface/sdk-js
commits: []
---

## 备注

- 主类型 `security-sensitive`：本能力允许授权调用方修改其它调用方的 tool/cmd/function 参数与结果，属于安全敏感路径。
- **v2.0 基线变更（2026-09-03）：** CHG-1 门控改为 `system::GetBoolParameter` 运行时判断（移除宏隔离）；CHG-2 注册改为失败式（已注册返回失败）；CHG-3 functionHook 调用路径改为 service 直接分发（移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction）；CHG-4 新增 NAPI 层（JS 系统接口，3 个 JS interface，JS 对象封装为 RemoteObject，回调到 JS 线程）。
- 仓内路径：`cli_tool_framework/**` + `interface/sdk-js/**`（v2.0 新增 SDK JS 层）。
- 澄清状态：Q-1~Q-5（v1.0）+ Q-6~Q-8（v2.0 新增）全部已确认。
- 适用范围：仅开发模式（`const.security.developermode.state`）生效，非开发模式注册失败 + 回调跳过。
