# Feature: Agent Hook（CliHook/FunctionHook 拦截能力）

## 背景

OpenHarmony Agent 框架需要在 cli_tool_framework 的 tool/cmd/function 执行链路中插入"Hook"能力，允许授权调用方在执行前后修改参数与结果、实现命令审计、参数改写、结果脱敏、策略阻断等横切逻辑。当前无此机制，必须侵入式改源码才能插入横切逻辑，维护成本高且无法运行时动态配置。

## 目标

- 在 tool/cmd/function 执行链路中提供 Hook 注册/注销能力
- 授权调用方可在执行前后拦截并改写参数与结果
- 仅系统应用 + 权限 + 开发模式可注册，防止恶意劫持

## 非目标

- Hook 链式注册（多 Hook 按顺序执行）
- Hook 持久化
- 跨设备 Hook 同步
- C++ 静态接口

## 涉及范围

- ability_ability_runtime（主仓）：NAPI 层 + IPC 接口 + 回调链路
- interface/sdk-js：JS System API 声明

## 关联 Proposal

- `.codespec/changes/issue-16098-agent-hook/proposal.md`
