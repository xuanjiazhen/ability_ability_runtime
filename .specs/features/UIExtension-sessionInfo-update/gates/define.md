# Stage 1 (Define) Gate Checklist

**特性**：UIExtension sessionInfo 支持更新
**日期**：2026-07-02
**产物**：`proposal.md`

## 进入 Stage 2 必过条件

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 1 | 范围和排除项已明确 | ✅ 通过 | proposal.md "需求基线 → 范围" 区段列出 6 项包含、6 项排除。包含项明确 GetOrCreateExtensionRecord 迁移更新分支、originalCallerInfo 定位、sessionInfo 更新、uiExtensionMap_ 一致性、单元测试；排除项明确 ability_base 结构体变更、window_manager 接口、arkui 接口、跨进程迁移、多次迁移策略（留 design）、不破坏现有匹配主路径 |
| 2 | 涉及子系统/仓已识别 | ✅ 通过 | proposal.md "涉及子系统/仓" 表格列出 5 项，全部在 ability_runtime 单仓内：ability_manager_service.cpp、extension_record_manager.cpp/.h、ui_extension_ability_manager.cpp、test/unittest。另列出 3 项跨仓依赖（ability_base / window_manager / arkui）标注为非本次范围 |
| 3 | 复杂度级别已判断 | ✅ 通过 | 判定为 **标准**。理由：单仓特性 + GetOrCreateExtensionRecord 核心调度路径新增分支 + sessionInfo 更新 + map 一致性维护，无跨 SIG 协调，无安全/性能关键路径。一个开放设计问题（多次迁移策略）已标记留待 design.md |
| 4 | 每个 P0/P1 AC 以 WHEN/THEN 格式写出且可测试 | ✅ 通过 | 7 条 P0 (AC1-AC7) + 5 条 P1 (AC8-AC12)，全部 WHEN/THEN 格式。每条均可用 `aa dump -a`、`hilog` 过滤、单元测试命令验证 |

## 额外质量检查

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 5 | 澄清记录完整（每轮问答留痕） | ✅ 通过 | "澄清记录" 表格列出 7 项关键澄清，覆盖仓范围边界、originalCallerInfo 语义、迁移场景范围、AC#2 对齐、老接口加新参数含义、现有匹配机制分析、多次迁移开放问题 |
| 6 | YAGNI — 不涉及项标记 N/A 或在排除项中 | ✅ 通过 | 排除项明确：ability_base 结构体变更、window_manager 接口、arkui 接口、跨进程/分布式迁移、多次迁移策略、现有主路径修改，均不在本次范围 |
| 7 | 概念术语一致 | ✅ 通过 | 全文统一使用"originalCallerInfo = 迁移前原始调用方信息"、"迁移更新 = 通过 originalCallerInfo 定位原 record 再更新 sessionInfo"；与用户澄清口径一致 |
| 8 | 前期代码调研已记录 | ✅ 通过 | proposal.md "现有逻辑分析" 区段记录了 GetAbilityRecordBySessionInfo 匹配机制、uiExtensionComponentId 生成公式、迁移场景核心问题（ID 变化导致无法匹配）、现有 sessionInfo 更新链路，为 design.md 提供基础 |

## 基线结论

**通过** — Stage 1 所有进入条件满足，可进入 Stage 2。

## 待用户批准

请审阅 `proposal.md` 全文。批准后进入 Stage 2 编写 `design.md` 和 `spec.md`。
