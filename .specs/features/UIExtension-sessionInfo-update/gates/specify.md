# Stage 2 (Specify) Gate Checklist

**特性**：UIExtension sessionInfo 支持更新
**日期**：2026-07-02（修订：补充应用侧链路 + persistentId 修正）
**产物**：`design.md` + `spec.md`

## 进入 Stage 3 必过条件

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 1 | design.md 已写（按复杂度要求） | ✅ 通过 | 标准复杂度，design.md 已写全量：架构上下文（现有链路 + 问题分析 + 设计后链路含应用侧）、7 个 ADR、不涉及项确认表（10 项）、修改物清单（服务侧 + 应用侧 + 测试 + 构建）、架构图（含应用侧链路）、时序图、关键风险与缓解（7 项） |
| 2 | spec.md 已写（全量） | ✅ 通过 | spec.md 全量：特性概述、4 个用户故事、API 影响（含应用侧内部接口）、7 组行为规则、13 条 AC（8 P0 + 5 P1）、测试场景（13 单元测试 + 5 回归测试 + 5 集成测试） |
| 3 | 关键设计决策有取舍理由 | ✅ 通过 | 7 个 ADR 均含"决策 + 理由 + 替代方案"三段式：ADR-1 注入点、ADR-2 按 persistentId 匹配（为何不用 originalCallerInfo 匹配）、ADR-3 map 按 persistentId 清理（为何不扩展 originalCallerInfo 结构）、ADR-4 多次迁移天然支持（persistentId 不变）、ADR-5 并发保护、ADR-6 判断函数、ADR-7 应用侧迁移感知（完整 IPC 链路梳理） |
| 4 | 架构图核心结论清晰 | ✅ 通过 | design.md §5 架构图展示完整端到端链路：应用发起方 → AMS 服务侧（新增分支0 按 persistentId 匹配 + SetSessionInfo + map 清理）→ IPC → 应用侧（OnCommandWindow 迁移感知 + HandleSessionUpdate）→ window 子系统。§6 时序图展示完整消息流含应用侧 |
| 5 | 不涉及项确认表完整 | ✅ 通过 | design.md §3 列出 10 项不涉及，新增"修改 ForegroundWindow/BackgroundWindow 现有逻辑"确认不涉及；每项均有理由 |
| 6 | AC 全部 WHEN/THEN 格式可测试 | ✅ 通过 | 13 条 AC 全部 WHEN/THEN 格式，含新增 AC5a 应用侧迁移感知；每条可用 `aa dump -a`、`hilog` 过滤、单元测试命令验证 |

## 额外质量检查

| # | 检查项 | 结果 | 证据/理由 |
|---|--------|------|----------|
| 7 | 现有代码路径不受影响已确认 | ✅ 通过 | design.md §1.4 关键约束 + §3 不涉及项 + spec.md §4.6：GetAbilityRecordBySessionInfo、IsPreloadExtensionRecord、GetOrCreateExtensionRecordInner、ForegroundWindow/BackgroundWindow 逐行不变 |
| 8 | 跨仓依赖已识别且标注 | ✅ 通过 | ability_base（originalCallerInfo 前置依赖）、window_manager（Rosen::Window session 更新 API — ADR-7 跨仓依赖）、arkui（组件接口）均标注 |
| 9 | 开放设计问题已决策 | ✅ 通过 | 多次迁移策略在 ADR-4 决策：persistentId 不变 → 天然支持多次迁移，无需特殊策略；应用侧链路在 ADR-7 补全完整 IPC 链路 |
| 10 | 容错降级策略已定义 | ✅ 通过 | spec.md §4.4 定义异常场景容错：匹配失败时降级到现有逻辑、sessionInfo 为空时返回错误码、record sessionInfo 为空时跳过 |
| 11 | 测试场景覆盖度 | ✅ 通过 | 13 条单元测试覆盖 AMS 侧 + 应用侧；5 条回归测试覆盖现有路径不变；5 条集成测试覆盖跨仓联调 |

## 设计要点摘要（修订版）

### ADR 一览

| ADR | 决策 | 关键取舍理由 |
|-----|------|------------|
| ADR-1 | 迁移更新分支注入 GetOrCreateExtensionRecord 最前面 | 命中后直接 return，不影响现有逻辑 |
| ADR-2 | 按 persistentId 匹配（迁移时 persistentId 不变） | persistentId 不变可直接匹配；callerToken→callerRecordId 不可逆，无法重构 uiExtensionComponentId |
| ADR-3 | uiExtensionMap_ 按 persistentId 查找清理旧映射 | 不扩展 originalCallerInfo 结构，避免跨仓对齐成本 |
| ADR-4 | persistentId 不变 → 多次迁移天然支持 | 无需特殊策略，每次按 persistentId 匹配即可 |
| ADR-5 | 复用现有 mutex_ + 调用链串行化 | 迁移低频，不加额外防并发 |
| ADR-6 | IsMigrationUpdateRequest 封装判断 | 集中逻辑，与 SR 一致 |
| ADR-7 | 应用侧三层架构：基类检测 → JsUIExtension 执行 → ContentSession UpdateExtensionParent | 复用现有 IPC，不新增 code；统一 C++ `UpdateExtensionParent(WindowOption)` 接口（无需 napi 暴露）；跟踪并更新应用持有的 JsExtensionWindow proxy |

### 架构核心结论

- **不新增 API / 不新增 IPC**，复用 StartUIExtensionAbility + ScheduleCommandAbilityWindow
- **AMS 侧**：按 persistentId 定位原 record + 更新 callerToken + 清理旧 map 映射
- **应用侧**：OnCommandWindow 迁移检测 → JsUIExtension::HandleSessionUpdate → ContentSession::UpdateExtensionParent（更新 sessionInfo_ + uiWindow_ parent + 所有 JsExtensionWindow proxy）
- **容错降级**：匹配失败时走现有逻辑，不 crash

## 基线结论

**通过** — Stage 2 所有进入条件满足，可进入 Stage 3。

## 待用户批准

Stage 2 产物已完成（含 persistentId 修正 + 应用侧链路补充）。请审阅 `design.md` 和 `spec.md`（摘要如上），批准后进入 Stage 3。
