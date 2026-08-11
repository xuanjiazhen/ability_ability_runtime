---
report_id: codecheck_pr20114_20260810
scope: "PR #20114 — branch_protector 属性名修正（BTI 修复）"
repo: openharmony/ability_ability_runtime
commit_id: 2277fc753730821fb7db1f8a2683f3d8f43c39ed
commit_subject: "bti"
author: Luobniz21
date: 2026-08-07
change_id: N/A（patch 未见 Change-Id 行）
ai_authored: true
ai_authorship: "AI[100%] AI Adopted[100%], Co-authored-by: opencode (glm-5.2)"
files_changed: 15
lines_changed: 28
lines_deleted: 28
risk_level: low
score: 100
gate_decision: approve
dimensions: [security, logic]
waived_dimensions: [api, input]
waive_reason: "纯 BUILD.gn 构建配置变更，不触及 .d.ts/API 签名/IPC/DB/文件"
scanner_combination: [security-scanner, logic-scanner]
input_scanner_skipped_reason: "路径无 IPC/DB/文件/配置信号"
api_scanner_skipped_reason: "无 .d.ts/公共 API 签名变更，仅 branch_protector 属性名"
proportionate_inline_review: true
inline_review_reason: "改动为 28 行机械属性名重命名，按比例原则内联执行 security+logic 两维度"
---

# Codecheck Report — PR #20114

## 1 基本信息

| 项 | 值 |
|---|---|
| 仓库 | openharmony/ability_ability_runtime |
| 提交 | 2277fc753730821fb7db1f8a2683f3d8f43c39ed |
| Subject | bti |
| 作者 | Luobniz21 |
| 日期 | 2026-08-07 |
| AI 作者 | 是（AI 100%，Co-authored-by opencode glm-5.2） |
| 变更规模 | 15 文件，+28/-28 |
| 变更类型 | 构建配置（BUILD.gn）属性名重命名 |
| Change-Id | 未提供 |

**改动语义：** 把 PR #20058 引入的 28 处错误写法
```
  branch_protector_ret = "pac_ret"
  branch_protector_ret = "bti"   ← 同名后赋值覆盖前赋值，PAC-RET 被静默丢弃
```
修正为
```
  branch_protector_ret = "pac_ret"   # PAC-RET（返回地址签名，后向边/防 ROP）保留
  branch_protector_frt = "bti"        # BTI（分支目标识别，前向边/防 JOP）经专用属性叠加
```
涉及 15 个 BUILD.gn 的 28 处 `ohos_shared_library` 目标。

## 2 路径特征探测与 scanner 选择

**探测信号（Step 2.1）：**
- IPC 信号：否（纯 BUILD.gn，无 `Parcel::Read`/`OnRemoteRequest`）
- 持久化信号：否（无 DB/文件/配置写入）
- API 信号：否（无 `.d.ts`/公共 API 签名变更，仅编译期 branch protector 标志）
- 安全信号：是（branch protection = ARMv8.3-A CFI 硬件特性，属安全加固配置）

**决策（Step 2.2）：** 通用路径 + 安全信号 → `security-scanner` + `logic-scanner`（必检双维度）；跳过 `api-scanner`（无公共 API 变更）与 `input-scanner`（无 IPC/DB/文件信号）。

**执行方式：** 改动为 28 行机械属性名重命名，按比例原则内联执行 security+logic 两维度，未派发并行子 agent。

## 3 总体评价

此 PR 是对 PR #20058 安全降级缺陷的正确修复。PR #20058 因在 28 处目标内对**同名属性** `branch_protector_ret` 二次赋值（`"pac_ret"` 后写 `"bti"`），导致 GN 后赋值覆盖前赋值，**PAC-RET 返回地址签名被静默丢弃**，仅剩 BTI。本 PR 将第二行属性名改为专用的 `branch_protector_frt`，使 PAC-RET 与 BTI 经两个独立属性并存，恢复并补齐了双向边 Control-Flow Integrity 防护。

**修复正确性验证（全量）：**
- 残留错误写法 `branch_protector_ret = "bti"` 全仓 grep → **0 命中**（28 处全修复）
- `branch_protector_frt = "bti"` 总数 = **58**（30 处既有正确用法 + 28 处本次修复，无重复/过度替换）
- 目标块共存检查：抽查 `frameworks/native/ability/native/BUILD.gn:296-297`（`abilitykit_native`）、`:3280-3281`（`auto_startup_callback`）均确认 `branch_protector_ret = "pac_ret"` 紧接 `branch_protector_frt = "bti"`，无"反向错误"（仅有 BTI 而丢 PAC-RET）
- 属性名 `branch_protector_frt` 正确性：仓内 5 处既有用法佐证（`agent_ui_extension_ability`、`ets_environment`、`js_environment`、`ani_base_context`、`page_config_manager`），为 OpenHarmony GN 模板的标准 BTI 属性

无 P0/P1/P2 代码发现。无 P3 代码发现。**上库决策：approve（100/100，low risk）**。

## 4 问题统计

| 级别 | 数量 |
|---|---|
| P0 | 0 |
| P1 | 0 |
| P2 | 0 |
| P3 | 0 |

## 5 高优先级发现（P0/P1）

无。

## 6 分维度明细

### 6.1 security-scanner 维度
无发现。本 PR 是安全**改进**而非退化：
- **恢复 PAC-RET**（后向边，防 ROP）：PR #20058 的同名覆盖曾使其在 28 个目标内失效，本 PR 恢复
- **正确叠加 BTI**（前向边，防 JOP）：经专用 `branch_protector_frt` 属性，不再覆盖 `pac_ret`
- 属性名 `branch_protector_frt` 为仓内既有标准用法，无新属性风险
- 不触及权限校验、IPC 鉴权、AppSpawn 安全参数、Mission 持久化等任何安全边界

### 6.2 logic-scanner 维度
无发现。
- **GN 语义正确**：`branch_protector_ret` 与 `branch_protector_frt` 是两个独立属性，不互相覆盖；二者并存 → 编译期 `-mbranch-protection` 同时生效 PAC-RET + BTI
- **完整性**：28/28 处修复（grep 残留 0）
- **一致性**：每个修复目标均保留 `branch_protector_ret = "pac_ret"` 并新增 `branch_protector_frt = "bti"`（抽查佐证）
- **无过度替换**：`_frt` 总数 58 = 30 既有 + 28 修复，无目标产生重复 `_frt` 行
- **范围正确**：仅在 PR #20058 已添加 BTI 意图的目标内修正属性名，未向原本无 BTI 意图的目标扩散

## 7 待跟进

- **T-1 commit message 过简**：Subject 仅为 "bti"，未说明本 PR 是修复 PR #20058 的同名属性覆盖缺陷、未说明安全影响（PAC-RET 恢复 + BTI 正确叠加）。建议补 `Fixes #20058` 或在 body 说明"恢复被 #20058 静默丢弃的 PAC-RET，并将 BTI 迁移到专用 branch_protector_frt 属性"。
- **T-2 Change-Id 缺失**：patch body 仅有 `Signed-off-by` + `Co-authored-by`，未见 `Change-Id: I...` 行。若仓库门禁要求 Change-Id（gerrit hooks），需补。
- **T-3 AI 作者透明度**：patch header 已标注 `AI[100%] AI Adopted[100%]` + `Co-authored-by: opencode (glm-5.2)`，透明度合规。改动为机械属性名重命名，AI 作者风险低。
- **T-4 编译验证未本地执行**：本环境无 OpenHarmony 根目录构建链，未跑 `./build.sh --product-name <product> --build-target ability_runtime`。GN 模板对 `branch_protector_frt` 的支持已由仓内 5 处既有用法佐证，编译风险低；建议合入前在 CI 跑一次全量 `ability_runtime` 编译。

## 附录 A 评分与决策矩阵

**评分公式**：`score = max(0, 100 − (30×P0 + 12×P1 + 5×P2 + 2×P3))`

| 级别 | 数量 | 扣分 |
|---|---|---|
| P0 | 0 | 0 |
| P1 | 0 | 0 |
| P2 | 0 | 0 |
| P3 | 0 | 0 |
| **合计** | 0 | **0** |

- `score = 100 − 0 = 100`
- `risk_level = low`（100 ≥ 90）
- `gate_decision = approve`（无 P0、无 P1、score ≥ 90）
- 必检维度：security✓ logic✓（api/input 不适用于本路径，已 waive）→ 非 insufficient

## 附录 B 已排除发现（refute 结果）

无发现可 refute。Refute 详情见 `refute_log_pr20114_20260810.md`。

## 附录 C 检视范围说明

- 检视对象：PR #20114 提交 `2277fc753730821fb7db1f8a2683f3d8f43c39ed`（本地 HEAD 即该提交）
- 验证手段：`git show --stat` 确认改动范围 + 全仓 grep 确认残留/总数/共存 + 抽查目标块结构
- `.d.ts`/GN 模板定义不在本仓（在 `interface/sdk-js` 与 `build` 仓），属性名正确性以仓内既有用法为权威佐证
- 本报告不改源码
