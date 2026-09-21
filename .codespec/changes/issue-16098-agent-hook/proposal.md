---
target_release: "7.0"
change_type: new-feature
issue: "https://gitcode.com/openharmony/ability_ability_runtime/issues/16098"
author: "@zivzhen"
date: "2026-09-09"
status: Draft
---

# Proposal

## 背景与问题

OpenHarmony Agent 框架需要在 cli_tool_framework 的 tool/cmd/function 执行链路中插入"Hook"能力，允许授权调用方在执行前后修改参数与结果、实现命令审计、参数改写、结果脱敏、策略阻断等横切逻辑。当前无此机制，必须侵入式改源码才能插入横切逻辑，维护成本高且无法运行时动态配置。

## 用户场景与业务触发

### US-1: 注册 CliHook 拦截 tool/cmd 执行

- **角色**：Agent 框架开发者
- **业务触发**：授权调用方通过 JS 接口 `cliManager.registerCliHook(hook)` 注册 CliHook 对象
- **业务价值**：在 tool/cmd 执行前后插入自定义逻辑（审计、参数改写、结果脱敏）

验收标准：

| AC | 验收标准 | 类型 |
|----|----------|------|
| AC-1.1 | WHEN 系统应用 + 有权限 + 开发模式 + 传入合法 CliHook THEN registerCliHook 返回 Promise resolved | 正常 |
| AC-1.7 | WHEN 调用 unregisterCliHook(hook) 传入相同对象 THEN Promise resolved，后续 execTool/execCmd 不再触发回调 | 正常 |
| AC-3.1 | WHEN 非系统应用调用 registerCliHook THEN reject 错误码 202 | 异常 |

### US-2: 注册 FunctionHook 拦截 function 执行

- **角色**：Agent 框架开发者
- **业务触发**：授权调用方通过 JS 接口 `functionManager.registerFunctionHook(hook)` 注册 FunctionHook 对象
- **业务价值**：在 invokeFunction 执行前后插入自定义逻辑

验收标准：

| AC | 验收标准 | 类型 |
|----|----------|------|
| AC-2.1 | WHEN 系统应用 + 有权限 + 开发模式 + 传入合法 FunctionHook THEN registerFunctionHook 返回 Promise resolved | 正常 |
| AC-2.5 | WHEN 调用 unregisterFunctionHook(hook) 传入相同对象 THEN Promise resolved，后续 invokeFunction 不再触发回调 | 正常 |

### US-3: 安全门控

- **角色**：系统管理员
- **业务触发**：非系统应用或无权限调用方尝试注册 Hook
- **业务价值**：防止恶意应用劫持命令执行链路

验收标准：

| AC | 验收标准 | 类型 |
|----|----------|------|
| AC-3.2 | WHEN 系统应用但无 REGISTER_AGENT_HOOK 权限 THEN reject 错误码 201 | 异常 |
| AC-3.3 | WHEN 系统应用 + 有权限但非开发模式 THEN reject 错误码 35600034 | 异常 |
| AC-3.4 | WHEN 系统应用 + 有权限 + 开发模式 THEN register 返回 Promise resolved、unregister 返回 Promise resolved | 正常 |

## 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 变更类型 | new-feature | 新增 Hook 注册/注销 JS API + NAPI 层 + IPC 回调链路 |
| 复杂度 | 标准 | 单仓（ability_ability_runtime + interface/sdk-js），新增 IPC 接口 + NAPI 层 + 回调链路 |
| 涉及仓数量 | 2 | ability_ability_runtime（主）、interface/sdk-js |
| 是否涉及 System API | 是 | 新增 JS System API（registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook + CliHook/FunctionHook/HookOption interface） |
| 是否涉及安全关键路径 | 是 | Hook 可修改其它调用方的参数与结果 |
| 是否跨 SIG | 否 | 仅 ability SIG |

## 目标

1. 授权调用方可通过 JS 接口 `cliManager.registerCliHook(hook)` 注册 CliHook 拦截 execTool/execCmd 执行前后的参数与结果。
2. 授权调用方可通过 JS 接口 `functionManager.registerFunctionHook(hook)` 注册 FunctionHook 拦截 invokeFunction 执行前后的参数与结果。
3. Hook 回调方法为可选（至少实现一个），通过 `activeMethods` 位掩码仅调用已实现的方法。
4. 仅系统应用 + `ohos.permission.REGISTER_AGENT_HOOK` 权限 + 开发模式可注册 Hook。
5. 同类型 Hook 已注册时 register 返回 35600035；unregister 需传入相同 Hook 对象校验一致性。
6. 公共错误码为 356xxxx，NAPI 层将内部错误码映射为公共错误码。
7. API 文档含 @permission 标注和 @throws 错误码说明。
8. unregister 接口需要传入与 register 时相同的 Hook 对象（非无参数）。

## 成功标准

| 编号 | 成功标准 | 验证方式 |
|------|----------|----------|
| SC-1 | 授权调用方可注册 CliHook/FunctionHook 拦截 tool/cmd/function 执行 | register 后 execTool/invokeFunction 触发回调，参数与结果可改写 |
| SC-2 | 非系统应用/无权限/非开发模式调用被拒绝 | 分别 reject 202/201/35600034 |
| SC-3 | 无 Hook 时执行链路与引入前一致（零开销） | 不注册 Hook → execTool/invokeFunction 结果不受影响 |
| SC-4 | 注册方进程死亡后 Hook 自动注销 | 终止进程 → 其他调用方不再触发回调 |

## 非目标

- Hook 链式注册（多 Hook 按顺序执行）——v1 单 Hook。
- Hook 持久化——注册方进程死亡即注销。
- 跨设备 Hook 同步。
- HookOption 追踪参数（dmSessionId + toolCallId）——最终实现未包含。
- C++ 静态接口。

## 安全约束

- 权限：`ohos.permission.REGISTER_AGENT_HOOK`（管控 Register/Unregister 行为）。
- 系统应用校验：`PermissionUtil::CheckSystemAndPermission`（IsSystemApp + VerifyAccessToken 合一）。
- 开发模式门控：`system::GetBoolParameter("const.security.developermode.state", false)`。
- 校验顺序：系统应用（202）→ 权限（201）→ 开发模式（35600034）。
- 非开发模式下回调点也跳过（零开销）。

## 兼容性

- 全新增方法，不破坏既有 execTool/execCmd/invokeFunction 行为。
- 无 Hook 时执行链路与引入前完全一致（零开销）。
- 最低支持版本：OpenHarmony-7.0-Release（JS API @since 26.0.1）。

## 1+8设备差异规格

不涉及。本特性为 Agent 框架 Hook 能力，与设备形态（手机/平板/穿戴/智慧屏等）差异无关，所有设备形态行为一致。

## DFX设计

不涉及。本特性不引入可靠性/可测试性/可维测性专属设计；Hook 回调超时与异常恢复由业务规则 BR-8/EX-3/EX-4 覆盖。

## 不涉及项确认

| 维度 | 是否涉及 | 依据 |
|------|----------|------|
| 性能 | 是 | 无 Hook 时执行链路零开销；Hook 回调超时 6s/5s 后返回原值 |
| 安全与权限 | 是 | ohos.permission.REGISTER_AGENT_HOOK 权限 + 系统应用校验 + 开发模式门控，三层校验 |
| 兼容性 | 是 | 全新增方法，不破坏既有行为；最低支持版本 7.0；JS API @since 26.0.1 |
| API/SDK | 是 | 新增 JS System API：registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook + CliHook/FunctionHook 等接口 |
| IPC/跨进程 | 是 | 新增 IPC 接口 + 同步 IPC 回调链路 |
| 构建与部件 | 是 | ability_ability_runtime + interface/sdk-js 两仓构建，新增 NAPI 层 |
| 国际化/无障碍 | 否 | Hook 拦截为系统能力，无用户可见 UI，不涉及国际化与无障碍 |
| 数据迁移 | 否 | 全新增能力，无历史数据需迁移 |
