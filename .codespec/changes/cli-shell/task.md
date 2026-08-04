# FEAT-003-1 Task

> 任务索引。详细 Steps（RED→GREEN）/ Verification / Anti-Fake Completion / Review Handoff / 代码范围映射见 `execution-plan.md`。
> 本表为快速索引，与 execution-plan.md 的 TASK 一一对应。

## 实现任务（详见 execution-plan.md Task 详情）

| TASK ID | 目标 | 文件范围 | AC 映射 | 状态 |
|---------|------|----------|---------|------|
| TASK-1 | 客户端接口与数据结构 | cli_tool_framework/interfaces/cli_tool/include | AC-1.1/1.2/2.1/2.2 | ✅ |
| TASK-2 | session 状态机与并发清理 | cli_tool_framework/services/climgr/session_record | AC-2.3 | ✅ |
| TASK-3 | 进程孵化与安全链路 | cli_tool_framework/services/climgr/process_manager + services/common/permission_util | AC-1.1/2.2 | ✅ |
| TASK-4 | sandbox 配置与 sessionId 生成 | cli_tool_framework/services/climgr/tool_util | AC-2.1/2.2 | ✅ |
| TASK-5 | 单测 | cli_tool_framework/test/unittest/tool_util_test | 支撑 | ✅ |

## 跨仓依赖任务（由对应仓跟踪）

| # | 任务 | 仓库 | 状态 |
|---|------|------|------|
| X1 | ATM 接口生成包装调用方 tokenId | security_access_token | ✅（已对齐） |
| X2 | 内核新接口生成 tokenId 并设置 parentHapTokenId | kernel | ✅（已对齐） |

## 验收任务

| # | 验收 | AC | 状态 |
|---|------|----|------|
| V1 | 前台命令 demo | AC-1.1/1.2 | ✅ |
| V2 | 后台命令 + 交互 + 清理 demo | AC-2.1/2.2/2.3 | ✅ |
