# Stage 2 (Specify) Gate Checklist

**特性**：嵌入式元服务隐私弹框改为模应用体验
**日期**：2026-06-30
**产物**：`design.md` + `spec.md`
**复杂度**：标准

## Stage 2 退出条件检查

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 1 | design.md 已写（标准复杂度要求关键决策） | ✅ 通过 | design.md 包含 7 个 ADR（接入点、IPC、C++ 接口、Context 方法、ModalCallback、Want 常量、命名约定），每个 ADR 含决策/理由/替代方案 |
| 2 | spec.md 已写（全量） | ✅ 通过 | spec.md 包含特性概述、3 个用户故事、API 影响（新增 Inner API + Native API + Want 常量 + 兼容性 + JS SDK）、行为规则、14 条 AC、测试场景（14 单元 + 4 回归 + 3 集成） |
| 3 | 关键设计决策有取舍理由 | ✅ 通过 | 每个 ADR 均含"理由"+"替代方案"；尤其 ADR-2（IPC code 不复用）和 ADR-3（不修改 Extension 基类）明确说明取舍 |
| 4 | 架构图核心结论清晰 | ✅ 通过 | design.md §5 提供完整新旧链路对照架构图；§6 提供关键场景时序图（onPageShow → 新链路 → 模应用呈现 → 退出 key 回调） |
| 5 | 不涉及项确认表完整 | ✅ 通过 | design.md §3 列出 10 项不涉及项，包括现有分支代码、现有类/方法、JS 绑定、应用市场侧、ArkUI 底层、跨仓变更 |
| 6 | AC 全部 WHEN/THEN 格式可测试 | ✅ 通过 | spec.md §5 共 14 条 AC（P0×7 + P1×7），全部 WHEN/THEN 格式，每条可用 `aa dump`/`hilog` 过滤/单元测试命令验证 |
| 7 | YAGNI — 不涉及项标记 N/A | ✅ 通过 | design.md §3 不涉及项确认表；spec.md §3.4 现有接口变更标注"无变更"，§3.5 JS/ArkTS 公共 SDK 变更标注"无变更" |
| 8 | 复杂度裁剪对应（标准） | ✅ 通过 | 标准：design.md 关键决策 + spec.md 全量；不涉及场景库（复杂级才需要）、合规审查（关键级才需要） |
| 9 | 修改物清单完整 | ✅ 通过 | design.md §4 列出 4 大类共 ~17 个修改点（AMS 服务端、Inner API、框架层、Want 常量）+ 单元测试 + BUILD.gn |
| 10 | 命名隔离约定明确 | ✅ 通过 | ADR-7 以表格形式列出 7 对新旧命名对照，强制使用 `EmbeddablePrivacy` 前缀 |

## 额外质量检查

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 11 | 与 proposal.md 基线一致 | ✅ 通过 | 范围、排除项、复杂度、AC 数量与 proposal.md 完全对齐；新增 AC8-AC14 是 proposal 的 P1 扩展细化 |
| 12 | 关键风险识别 | ✅ 通过 | design.md §7 列出 4 项风险（IPC code 兼容、Callback 生命周期、隐私弹框未识别新 key、ArkUI 行为）+ 缓解措施 |
| 13 | 借鉴项识别清晰 | ✅ 通过 | design.md §1.1 明确标注"现有链路（借鉴项，保持不动）"；§1.2 明确标注"仅 ArkUI 底层复用" |

## Stage 2 结论

**通过** — Stage 2 所有退出条件满足，可请求用户批准进入 Stage 3。

## 待用户批准事项

请审阅：
- `design.md` — 7 个 ADR + 新旧链路架构图 + 时序图 + 修改物清单 + 风险缓解
- `spec.md` — 3 个用户故事 + API 影响 + 14 条 AC + 测试场景矩阵

批准后将进入 Stage 3，编写 `execution-plan.md` 和 `task.md`，然后开始代码实现。
