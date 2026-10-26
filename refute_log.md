# Refute 审查记录

## 审查概要

| 指标 | 数值 |
|------|------|
| 审查范围 | security: 2 条, logic: 1 条 |
| P0/P1 全审 | 2 条 |
| P2/P3 抽样审 | 1 条 |
| 维持 | 2 条 |
| 降级 | 0 条 |
| 推翻 | 0 条 |
| 合并 | 1 组 → 1 条 |
| **进入最终报告** | **2 条** (P0: 0, P1: 1, P2: 1, P3: 0) |

## 逐条审查

### ✅ 维持 — SEC-01 (P1): OnRequestDialogService 全局引用泄漏 + 死代码

**发现摘要**：`ets_ability_context.cpp:2555-2581`，提交新增了 `GlobalReference_Create` 返回值检查（正确），但同时从异步回调 lambda 中**删除了** `env->GlobalReference_Delete(callbackRef)`，且新增的 `callbackDeleted` shared_ptr 未被 lambda 捕获、从未使用——属于不完整重构。所有代码路径（context 为空、RequestDialogService 失败、lambda 正常执行）均不删除 `callbackRef`，造成 ANI 全局引用泄漏。

**第一层：触发路径证伪** — "真的能被触发吗？"
- `OnRequestDialogService` 是暴露给 ArkTS 的 ANI 绑定函数，用户代码调用 `context.requestDialogService(want, callback)` 时触发。
- 无上游 guard 阻止该路径：`GlobalReference_Create` 检查通过后（line 2556），`callbackRef` 在所有后续路径均无删除点。
- **结论**：可达，不可证伪。✅ 维持。

**第二层：影响夸大证伪** — "后果真的这么严重吗？"
- 每次调用泄漏一个 ANI 全局引用。ANI 全局引用表容量有限（通常数百至数千）。
- `requestDialogService` 是用户交互级 API（对话框），非高频循环调用，累积速度较慢。
- 但属确定性泄漏（每次必漏），非单次崩溃（不升 P0），P1（严重：持续性资源泄漏）评级合理。
- **对比变更前**：原代码在 lambda 成功路径中正确删除引用（仅在 context 空 / RequestDialogService 失败路径泄漏）；新代码在**所有路径**均泄漏——构成回归。
- **结论**：影响评估准确，非夸大。✅ 维持 P1。

**第三层：跨 scanner 根因去重**
- scanner 原始产出中 SEC-01（全局引用泄漏）与 SEC-02（`callbackDeleted` 死代码）位于同一函数、同一根因（不完整重构），修复方案为同一处改动。
- **结论**：🔀 合并 SEC-02 至 SEC-01，统一为一条 P1 发现。

---

### ✅ 维持 — LOG-01 (P2): find_if lambda 中 Reference_StrictEquals 返回值未检查

**发现摘要**：`ets_ability_manager.cpp:921, 995`，提交为 `NativeOffPreloadedUIExtensionAbilityLoaded` 和 `NativeOffPreloadedUIExtensionAbilityDestroyed` 新增了 `Reference_IsUndefined` 返回值检查，但同一函数的 `find_if` lambda 中 `env->Reference_StrictEquals(callback, cb.first, &isEquals)` 仍未检查返回值。与提交"清理未检查返回值"的意图不一致。

**第一层：触发路径证伪**
- `find_if` lambda 在每次"off"操作时遍历回调列表执行。可达。
- 若 `Reference_StrictEquals` 失败，`isEquals` 保持 `ANI_FALSE`，lambda 返回 false → 回调未找到 → 日志报错 + 函数返回。
- **结论**：可达。✅ 维持。

**第二层：影响夸大证伪**
- 安全失败模式：回调未找到 → 日志报错 → 用户可重试或等 ability 销毁时清理。无崩溃、无数据损坏。
- 但与提交意图（检查所有 ANI 返回值）不一致，属遗漏。
- **结论**：P2 评级合理（一致性缺陷 + 低实际影响）。✅ 维持 P2。

## refute 耗时与统计
- 审查耗时：~3 轮调用链追踪
- 代码片段二次验证：6 处
- 跨 scanner 合并：1 组（SEC-01 + SEC-02 → 1 条）
