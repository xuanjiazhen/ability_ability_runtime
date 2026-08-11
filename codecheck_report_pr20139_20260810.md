---
report_id: codecheck_pr20139_20260810
scope: "PR #20139 — cli_manager_error_utils.cpp 文案对齐"
repo: openharmony/ability_ability_runtime
commit_id: 6996103ef2e58e34b374b14366252d7d909935af
commit_subject: "0809_cli_50"
author: Luobniz21
date: 2026-08-10
change_id: N/A（patch 未见 Change-Id 行）
files_changed:
  - cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp
lines_changed: 2 (+2/-2)
risk_level: low
score: 96
gate_decision: approve
dimensions: [security, logic, api]
waived_dimensions: []
scanner_combination: [security-scanner, logic-scanner, api-scanner]
input_scanner_skipped_reason: "路径无 IPC/DB/文件/配置信号，input-scanner 不适用"
proportionate_inline_review: true
inline_review_reason: "改动仅 2 行静态字符串字面量，按比例原则内联执行三维度，未派发并行子 agent"
---

# Codecheck Report — PR #20139

## 1 基本信息

| 项 | 值 |
|---|---|
| 仓库 | openharmony/ability_ability_runtime |
| 提交 | 6996103ef2e58e34b374b14366252d7d909935af |
| Subject | 0809_cli_50 |
| 作者 | Luobniz21 |
| 日期 | 2026-08-10 |
| 变更文件 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp` |
| 变更规模 | 2 行（+2/-2），两处 `ERROR_MSG_MAP` 静态字符串字面量 |
| Change-Id | 未提供（patch body 仅 `Signed-off-by`） |

**改动内容：**
```
- ERROR_SESSION_NOT_FOUND: "The session not found."           → "The session does not exist."
- ERROR_SEND_MESSAGE:      "Sending Message to failure."     → "Failed to write message to tool."
```
枚举映射（`cli_manager_error_utils.h:32-33`）：`ERROR_SESSION_NOT_FOUND=35600032`、`ERROR_SEND_MESSAGE=35600033`。

## 2 路径特征探测与 scanner 选择

**探测信号（Step 2.1）：**
- API 信号：是（`frameworks/js/napi/` NAPI 绑定；对外 `.d.ts` 在 `interface/sdk-js` 仓定义 BusinessError message 契约）
- IPC 信号：否（静态字符串映射，无 `Parcel::Read`/`OnRemoteRequest`）
- 持久化信号：否（无 DB/文件/配置写入）

**决策（Step 2.2）：** `frameworks/js/napi/` + API 信号 → `security-scanner` + `logic-scanner` + `api-scanner`；跳过 `input-scanner`（无 IPC/DB/文件信号）。

**执行方式：** 改动仅 2 行静态字符串字面量，按比例原则内联执行三维度检查，未派发并行子 agent。各维度证据均追溯到 `file:line`。

## 3 总体评价

改动目的明确：把 `ERROR_MSG_MAP` 中 35600032/35600033 的实现文案与已发布 JSDoc（`@ohos.app.cli.cliManager.d.ts` 的 `@throws` 描述）对齐，并修正语法瑕疵。改动是纯静态字符串字面量替换，不触及任何代码路径、IPC、持久化或权限校验。

- 全仓 grep 旧文案 `"The session not found"`/`"Sending Message to failure"` → 0 命中（无代码/测试依赖旧文本）
- `CreateCliJsErrorByNativeErr` 的 12 个调用点（`js_cli_manager.cpp`/`js_function_manager.cpp`）仅 `Reject` 抛出，不依赖文案内容
- 无测试断言 cli_manager 侧文案（`ability_business_error_test.cpp` 测的是 ability 侧 `GetErrorMsg`，非 cli 侧）

无 P0/P1 问题。2 个 P3 观察项（一致性完整性缺口 + 公共契约变更记录）。**上库决策：approve（96/100，low risk）**。

## 4 问题统计

| 级别 | 数量 |
|---|---|
| P0 | 0 |
| P1 | 0 |
| P2 | 0 |
| P3 | 2 |

## 5 高优先级发现（P0/P1）

无。

## 6 分维度明细

### 6.1 security-scanner 维度
无发现。静态 const 字符串字面量，无用户输入流入、无格式串（`%s`/`%d`）、无新分配、无新 IPC/代码路径；message 经 `CreateCliManagerError`（`cli_manager_error_utils.cpp:98-107`）→ `napi_create_error`（`:104`）安全处理。

### 6.2 logic-scanner 维度

**L-1 [P3] PR 修复不完整：同文件同类语法瑕疵未一并处理**
- 位置：`cli_manager_error_utils.cpp:35-36`
- 证据：
  - `:35 {CliManagerErrorCode::ERROR_FUNCTION_EXECUTE_FAILED, "The function execute failed."}`
  - `:36 {CliManagerErrorCode::ERROR_FUNCTION_EXECUTE_TIMEOUT, "The function execute timeout."}`
- 说明：与本 PR 所修 35600032/35600033 同属"语法生硬/措辞不一致"类别（`execute` 应为 `execution`，`execute timeout` 应为 `execution timed out`）。PR 仅修了 2 处，同文件另有 2 处同类问题遗漏。
- 触发路径：`ERROR_FUNCTION_*` 经 `GetErrorMsg`（`:62-69`）→ `CreateCliManagerError`（`:98-107`）运行时输出，可达。
- 影响：仅文案可读性，无控制流/功能影响。
- 备注：本仓无 `.d.ts`，function 错误码的 JSDoc 措辞未本地核对；语法生硬为自明。
- 建议（不改源码，仅建议）：若 PR 意图为"文案对齐+语法修正"，建议 35/36 行一并处理；否则在 PR 说明中标注"仅修 35600032/35600033 的范围依据"。

### 6.3 api-scanner 维度

**A-1 [P3] 公共行为变更记录：BusinessError.message 文本变更**
- 位置：`cli_manager_error_utils.cpp:31-32`
- 说明：BusinessError 的 `message` 字段是对外可观察契约，本 PR 变更 35600032/35600033 的 message 文本属公共 API 行为变更。按 AGENTS.md「Ask before 修改已有 API 签名或语义」应做兼容性记录。
- 兼容性评估：
  - 与已发布 JSDoc 对齐（外部按文档使用的应用不受影响；仅按错误实现文案做字符串匹配的应用可能受影响，属其自身误用）
  - 仓内 grep 确认无代码/测试引用旧文案
  - 风险极低
- 结论：不阻塞，但建议 PR 描述/commit message 标注"公共 message 文案变更，与 JSDoc 对齐"以便兼容性追踪。

**一致性核对（informational，非发现）：**
- 35600032：impl `"The session does not exist."` 与 JSDoc `@throws ... The session does not exist.` 完全一致 ✅
- 35600033：impl `"Failed to write message to tool."`（首字母大写+句号，全句风格）vs JSDoc `failed to write message to tool.`（小写片段，描述风格）。风格差异，语义一致；impl 与 map 内其他文案内部一致 ✅

## 7 待跟进

- **T-1** 与上轮 DTS2026080305203 审计结论的取舍分歧：该审计对 35600032/35600033 标记为"低价值措辞差异（建议不修改）"，本 PR 选择修复。修复本身无害且提升一致性，但与既有结论存在分歧。建议 PR 说明修复依据（是否有新 DTS 单号或需求驱动），避免后续审计/合规对同一文件出现"修与不修"结论冲突。
- **T-2** Change-Id 缺失：patch body 仅有 `Signed-off-by`，未见 `Change-Id: I...` 行。若仓库门禁要求 Change-Id，需补。

## 附录 A 评分与决策矩阵

**评分公式**：`score = max(0, 100 − (30×P0 + 12×P1 + 5×P2 + 2×P3))`

| 级别 | 数量 | 扣分 |
|---|---|---|
| P0 | 0 | 0 |
| P1 | 0 | 0 |
| P2 | 0 | 0 |
| P3 | 2 | 4 |
| **合计** | 2 | **96** |

- `score = 100 − 4 = 96`
- `risk_level = low`（96 ≥ 90）
- `gate_decision = approve`（无 P0、无 P1、score ≥ 90）
- 必检维度：security✓ logic✓ api✓（input 不适用于本路径）→ 非 insufficient

## 附录 B 已排除发现（refute 结果）

无被推翻的发现。2 条 P3 均维持。Refute 详情见 `refute_log_pr20139_20260810.md`。

## 附录 C 检视范围说明

- 检视对象：PR #20139 的提交 `6996103ef2`（本地 HEAD 即该提交，工作区干净）
- `.d.ts` 不在 ability_runtime 仓，一致性比对依据上轮审计引用的 `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` JSDoc 片段
- 本报告不改源码；修复建议由用户确认后另起任务
