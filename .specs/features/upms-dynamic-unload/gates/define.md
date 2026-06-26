# Gate — Stage 1 Define（定义阶段）

> item: `upms-dynamic-unload` | 阶段切换：Stage 1 → Stage 2
> 规则依据：SKILL.md「Stage 1 进入 Stage 2 条件」+「审批即状态」+「证据先于声明」。
> 本文件记录 Stage 1 门禁逐项检查结果与审批证据。

## 一、Stage 1 退出 / Stage 2 进入门禁

| # | 检查项 | 结果 | 证据 / 理由 |
|---|--------|------|------------|
| D1 | proposal.md 原始需求已记录 | ✅ 通过 | proposal.md §一，引用两份来源分析文档与硬性内存目标动机 |
| D2 | 澄清记录完整（逐轮问答 + 选项 + 决策 + 理由） | ✅ 通过 | proposal.md §二，4 轮澄清（范围/驱动力/触发策略/验收口径）均闭环 |
| D3 | 范围 (In scope) 已明确 | ✅ 通过 | proposal.md §3.2，4 项明确（延时任务/触发点/ondemand 配置/SelfClean 校核） |
| D4 | 排除项 (Out of scope) 已明确 | ✅ 通过 | proposal.md §3.3，5 项排除 + §3.7 不涉及项确认表 |
| D5 | 涉及子系统 / 仓已识别 | ✅ 通过 | proposal.md §3.4，单仓 `ability_runtime`（services/uripermmgr + services/sa_profile/183.json） |
| D6 | 复杂度级别已判断 | ✅ 通过 | standard（单仓单特性，2-3 个关键设计决策，触发条件已规避安全风险） |
| D7 | 每个 P0/P1 AC 以 WHEN/THEN 写出且可测 | ✅ 通过 | proposal.md §3.5，3 个 P0 + 2 个 P1，每条含验证手段 |
| D8 | 不涉及项确认表已完成（YAGNI） | ✅ 通过 | proposal.md §3.7，7 行 N/A 已填，无留空 |
| D9 | Profile 判定完成 | ✅ 通过 | 无 `.claude/ohos-sdd/profiles/` 目录，无匹配子系统 profile，manifest.profile = none |
| D10 | 待解决问题已登记并指派 owner | ✅ 通过 | proposal.md §3.8，OQ-1~OQ-5（target_release / 超时阈值 / 取消口径 / ondemand 事件 / SelfClean），均 Stage 2 处理 |
| D11 | 基线理解已被需求方确认 | ✅ 通过 | 见下方「审批证据」 |

**结论：Stage 1 门禁全部通过（11/11）。**

## 二、审批证据 (Approval Evidence)

- **形式**：理解确认草稿（核心/范围含/排除项/涉及仓/复杂度/P0-P1 AC）已于本轮对话向需求方完整输出。
- **需求方表态**：2026-06-25，需求方（owner: wangzhen）回复 **"确认"**，确认理解准确、范围/排除项无增删、复杂度判 **standard** 可接受。
- **判定**：Stage 1 基线**通过**，状态字段已在 manifest.md 更新为 `Stage 1 Approved`，registry.md 已登记。

## 三、带入 Stage 2 的移交事项

1. design.md 需决策 OQ-2~OQ-5（超时阈值、取消口径、ondemand 事件定义、SelfClean 是否升级）。
2. design.md 需给出关键 ADR：触发点放置（账本清空路径尾部）、计时器实现（EventHandler）、ondemand 配置写法。
3. spec.md 需把 §3.5 的 AC 展开为完整规格 + 兼容性/可追溯。
4. `target_release`(OQ-1) 在 Stage 2 期间向 owner 确认。

> HARD-GATE：design.md / spec.md 在本 gate 通过 + 需求方批准 Stage 1 后方可创建。本 gate 已通过，**等待需求方批准进入 Stage 2**。
