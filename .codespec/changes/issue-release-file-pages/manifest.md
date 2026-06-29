# Manifest — issue-release-file-pages

| 字段 | 值 |
|------|-----|
| 变更 ID | issue-release-file-pages |
| 标题 | 应用按需释放文件页 (Release File Pages On Demand) |
| 复杂度 | 标准 |
| 当前阶段 | Stage 3 — 实现 (Implement) |
| 仓 | ability_runtime（主） |

## 阶段状态

| 阶段 | 产物 | 状态 | 审批证据 |
|------|------|------|----------|
| Stage 1 Define | proposal.md, gates/define.md | **Approved** | 用户 2026-06-16 显式批准"进入 Stage 2" |
| Stage 2 Specify | design.md, spec.md, gates/specify.md | **Approved** | 用户 2026-06-16 显式批准"进入 Stage 3" |
| Stage 3 Implement | execution-plan.md, task.md, review.md | **Approved** | T-1~T-6 代码就绪；T-0/T-7 设备实测通过；O-4 跨仓 SDK 声明由 SDK 团队落位 |
| Stage 4 Release | 验证+合入 | **Approved** | 编译通过 + 单测全绿 + T-0 方案 A 确认 + AC-1~AC-9 hidumper 全通过（用户设备，2026-06-29）；O-4 SDK 仓落位由 SDK 团队执行，不阻塞本仓 |

## 知识源检索日志

- 读取仓根 `CLAUDE.md`：记录构建/测试命令、目录结构、错误码、日志规范、特性标志（`ability_runtime.gni`）等约束。
- 读取 `MADVISE_COLD_PAGE_ENHANCEMENT_PLAN.md`、`SO_MEMORY_OPTIMIZATION_PLAN_V2.md`：既有 madvise 基础设施现状与已规划增强方向。
- 探索 `frameworks/native/ability/native/ability_runtime/madvise/`、`interfaces/kits/native/`、`frameworks/js/napi/`、`frameworks/ets/ani/`：确认可复用工具链与绑定注册位置。
