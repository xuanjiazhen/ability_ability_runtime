# FEAT-003-1 Manifest

> 交付件索引。父需求见 `../`（FEAT-003）。
> 代码与交付件落点：ability_ability_runtime（本地 `D:\Code\SDD\ability_ability_runtime`）

## 元数据

| 属性 | 值 |
|------|-----|
| 需求 ID | FEAT-003-1 |
| 标题 | cli SA 沙箱内执行 shell 命令 - 命令执行与会话管理（FEAT-003 子需求1） |
| 父需求 | FEAT-003（cli SA 提供应用沙箱内执行 shell 命令能力） |
| 涉及仓库 | ability_ability_runtime（主）、security_access_token、kernel |
| 责任人 | @SKY2001 |
| 插件组合 | spec-for-ai |
| Agent | Claude |
| 当前阶段 | Release |
| 更新时间 | 2026-08-03 |

## 交付件清单

| 交付件 | 路径（相对仓库根） | 状态 |
|--------|------|------|
| Manifest | `doc/cli_sa_shell/feat-003-1/manifest.md` | ✅ |
| Proposal | `doc/cli_sa_shell/feat-003-1/proposal.md` | ✅ |
| Design | `doc/cli_sa_shell/feat-003-1/design.md` | ✅ |
| Spec | `doc/cli_sa_shell/feat-003-1/spec.md` | ✅ |
| Execution Plan | `doc/cli_sa_shell/feat-003-1/execution-plan.md` | ✅ |
| Task | `doc/cli_sa_shell/feat-003-1/task.md` | ✅ |
| Gate (Define) | `doc/cli_sa_shell/feat-003-1/gates/define.md` | ✅ |
| Gate (Specify) | `doc/cli_sa_shell/feat-003-1/gates/specify.md` | ✅ |
| Threat Model | `doc/cli_sa_shell/feat-003-1/threat-model.md` | ✅ |
| Validation Spec | `doc/cli_sa_shell/feat-003-1/spec-for-validation.md` | ✅ |

## 验收标准（AC）

| AC | 描述 |
|----|------|
| AC-1 | 前台命令：demo 调用 ExecCmd（background=false），命令执行完 shell 自动销毁，接口回调返回正确 ExecResult |
| AC-2 | 后台命令：demo 调用 ExecCmd（background=true）立即返回 sessionId；通过 SubscribeSession 回调获取 CliSessionInfo（含 ExecResult）；通过 SendMessage 发送后续交互指令结果正确；调用 ClearSession 销毁 shell 进程 |

## 代码落点

| 仓库 | 模块 | 关键文件 |
|------|------|---------|
| ability_ability_runtime | cli SA 客户端 | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` |
| ability_ability_runtime | 执行参数/结果 | `exec_cmd_param.h`、`exec_options.h`、`exec_result.h`、`cli_session_info.h` |
| ability_ability_runtime | 服务侧 session | `cli_tool_framework/services/climgr/include/session_record.h` |
| ability_ability_runtime | 进程/sandbox | `cli_tool_framework/services/climgr/include/process_manager.h` |
| ability_ability_runtime | 工具/sandbox 生成 | `cli_tool_framework/services/climgr/include/tool_util.h` |
| ability_ability_runtime | ATM 权限 | `cli_tool_framework/services/common/include/permission_util.h` |
| security_access_token | ATM 接口 | 生成包装调用方 tokenId（依赖） |
| kernel | 内核新接口 | 生成 tokenId 并设置 parentHapTokenId（依赖） |
