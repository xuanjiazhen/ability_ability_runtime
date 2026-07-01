# Stage 1 (Define) Gate Checklist

**特性**：嵌入式元服务隐私弹框改为模应用体验
**日期**：2026-06-30
**产物**：`proposal.md`

## 进入 Stage 2 必过条件

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 1 | 范围和排除项已明确 | ✅ 通过 | proposal.md "需求基线 → 范围" 区段列出 5 项包含、6 项排除。包含项明确独立新路径、新 IPC、新 C++ 接口、ModalCallback 退出 key；排除项明确不动 `disposed_observer.cpp` 现有代码、不改 JS 绑定、不改应用市场侧、不跨仓 |
| 2 | 涉及子系统/仓已识别 | ✅ 通过 | proposal.md "涉及子系统/仓" 表格列出 4 项，全部在 ability_runtime 单仓内：services/abilitymgr、interfaces/inner_api、frameworks/native/ability/native、interfaces/kits/native |
| 3 | 复杂度级别已判断 | ✅ 通过 | 判定为 **标准**。理由：单仓特性 + 新 IPC + 新 C++ 接口 + 回调协议，无跨 SIG 协调，无安全/性能关键路径 |
| 4 | 每个 P0/P1 AC 以 WHEN/THEN 格式写出且可测试 | ✅ 通过 | 7 条 P0 (AC1-AC7) + 5 条 P1 (AC8-AC12)，全部 WHEN/THEN 格式；每条均可用 `aa dump`、`hilog` 过滤、单元测试命令验证 |

## 额外质量检查

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 5 | 澄清记录完整（每轮问答留痕） | ✅ 通过 | "澄清记录" 表格列出 8 项关键澄清，覆盖入口路径、交付边界、调用方、概念澄清、归属组件、JS 绑定、退出 key、跨仓范围 |
| 6 | YAGNI — 不涉及项标记 N/A 或在排除项中 | ✅ 通过 | 排除项明确：JS 绑定、UIExtensionContext 既有方法、应用市场侧、跨仓、退出 key 语义变更，均不在本次范围 |
| 7 | 概念术语一致 | ✅ 通过 | 全文统一使用"嵌入式元服务 = EmbeddableUIAbility（C++ 本体 UIExtension）"；隐私弹框 = 应用市场 UIExtensionAbility；与用户澄清口径一致 |

## 基线结论

**通过** — Stage 1 所有进入条件满足，可进入 Stage 2。

## 待用户批准

请审阅 `proposal.md` 全文。批准后进入 Stage 2 编写 `design.md` 和 `spec.md`。
