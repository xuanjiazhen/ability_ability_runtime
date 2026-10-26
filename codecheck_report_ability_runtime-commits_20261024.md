# 代码检视报告 — ability_runtime 最近两笔提交（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 生成入口：[`README.md`](skills/codecheck/README.md) → Step 5；合并逻辑见 [`orchestrator/SKILL.md`](skills/codecheck/orchestrator/SKILL.md)。
> 权威评分与门禁规则为**通用规则**，不在输出报告中呈现；生成时必须按 [`conventions.md`](skills/codecheck/conventions.md) §7（等级归一化）/ §8（必检维度）/ §9（评分与决策矩阵）计算，不得自创分值。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "ability_runtime-最近两笔提交(洗地告警+洗地告警TDD)"
  round: 1
  commit_id: "a900267fe01929643bba18348f232d56b4440b06"
  change_id: "I87e80b1cd09b1a2cd7b5fc2b66d707fea237bb92"
  report_id: "I87e80b1cd09b1a2cd7b5fc2b66d707fea237bb92-R1"
  date: "2026-10-24"
  gate_decision: "conditional"
  risk_level: "medium"
  score: 83
  dimensions_required: ["api-scanner", "security-scanner", "logic-scanner"]
  dimensions_executed: ["api-scanner (N/A: 无 API 签名/文档变更)", "security-scanner", "logic-scanner"]
  findings_total: 2
  findings_by_severity: {P0: 0, P1: 1, P2: 1, P3: 0}
  gate_blockers: []
  must_fix: ["SEC-01"]
  followups: ["LOG-01"]
```
<!-- codecheck-report-metadata:end -->


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **conditional** |
| 风险等级 | 🟠 medium |
| 评分 | **83/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 1 项 |
| 建议跟进（P2/P3） | 1 项 |

**一句话结论**：两笔提交（"洗地告警" + "洗地告警TDD"）系统性修复了 ANI 绑定层多处未检查返回值、资源泄漏和迭代器无限循环问题（整体方向正确），但在 `EtsAbilityContext::OnRequestDialogService` 中引入了一处 P1 回归——删除了异步回调中的全局引用释放且留下未完成的死代码，需修复后方可上库。

---

## 2. 扣分原因（仅 gate_decision=block 时呈现；approve/conditional/insufficient 时本节省略）

> 本报告 gate_decision=conditional，本节省略。扣分明细见第 3/4 节。

---

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | Scanner | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|---|
| SEC-01 | P1 | security-scanner | OnRequestDialogService 全局引用泄漏 + 死代码（不完整重构回归） | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2555-2581` | ArkTS 调用 `context.requestDialogService(want, callback)` → ANI 绑定 `OnRequestDialogService` → `GlobalReference_Create` 成功创建 `callbackRef` → 异步 task lambda 执行后未调用 `GlobalReference_Delete` → 引用泄漏；若 `context==nullptr` 或 `RequestDialogService` 返回错误，task 从未执行，引用同样泄漏 | 每次调用泄漏一个 ANI 全局引用，重复调用耗尽引用表导致后续 `GlobalReference_Create` 失败级联；原代码在 lambda 成功路径中正确释放引用，本次提交构成回归 |

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P2 | `ets_ability_manager.cpp` 两处 `find_if` lambda 中 `Reference_StrictEquals` 返回值未检查，与提交"清理未检查返回值"意图不一致 | 在 lambda 内检查返回值，失败时跳过该元素（返回 false + 日志） | 本轮或下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | 1 项 P1 | `OnRequestDialogService` 全局引用泄漏回归——删除了 lambda 内的 `GlobalReference_Delete`，`callbackDeleted` 死代码未完成；其余 5 处资源释放/错误检查修复均正确 |
| logic-scanner | 1 项 P2 | `ets_ability_manager.cpp` 两处 `find_if` lambda 中 `Reference_StrictEquals` 返回值未检查（一致性遗漏）；迭代器无限循环修复（3 处）和返回值检查修复（4 处）均逻辑正确 |
| api-scanner | N/A | 无 API 签名/文档变更，均为内部实现层 bug 修复，api-scanner 无适用面 |

---

## 6. 关键发现详情

### [SEC-01] OnRequestDialogService 全局引用泄漏 + 死代码（不完整重构回归） (P1, scanner=security-scanner)

- **位置**：`frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2555-2581`
- **触发路径**：通过 ArkTS API `context.requestDialogService(want, callback)`，给定任意合法 Want 和 callback，代码路径 `OnRequestDialogService` → `GlobalReference_Create(call, &callbackRef)` 成功（line 2556）→ task lambda 以 `[vm, callbackRef]` 捕获引用 → lambda 执行 `AsyncCallback` 使用 `callbackRef` 后未调用 `GlobalReference_Delete` → 全局引用泄漏。若 `context_.lock()` 返回 nullptr（line 2584）或 `RequestDialogService` 返回错误（line 2589），task 从未执行，引用同样泄漏。
- **影响**：每次 `requestDialogService` 调用泄漏一个 ANI 全局引用。ANI 全局引用表容量有限（通常数百至数千条目），重复调用将耗尽引用表，导致后续 `GlobalReference_Create` 失败并引发级联故障（其他依赖全局引用的 ANI 绑定函数失败）。**构成回归**：变更前原代码在 lambda 成功路径中调用 `env->GlobalReference_Delete(callbackRef)` 正确释放（仅在 context 空 / RequestDialogService 失败路径泄漏），本次提交删除了该释放调用，使成功路径也泄漏。
- **证据**：
  - diff 删除行：`-        env->GlobalReference_Delete(callbackRef);`（原 lambda 体内）
  - diff 新增行：`+    auto callbackDeleted = std::make_shared<bool>(false);`（line 2561）——该变量未出现在 lambda 捕获列表 `[vm, callbackRef]` 中（line 2563），且函数后续无任何读写引用，属死代码。
  - `ets_ability_context.cpp:2563` lambda 捕获列表为 `[vm, callbackRef]`，不含 `callbackDeleted`。
  - `ets_ability_context.cpp:2577` lambda 内调用 `AsyncCallback` 使用 `callbackRef` 后，无 `GlobalReference_Delete` 调用。
  - 对比同提交中 `ets_insight_intent_driver.cpp:530` 的同类修复（GetVM 失败时正确调用 `GlobalReference_Delete(callbackRef)`），说明开发者意图正确但 `ets_ability_context.cpp` 处实现未完成。
- **建议**：
  1. 在 lambda 体内 `AsyncCallback` 调用后恢复 `env->GlobalReference_Delete(callbackRef)`（line 2578 之后、`DetachAniEnv` 之前）。
  2. 在 `context == nullptr`（line 2584）和 `RequestDialogService` 失败（line 2589）的错误分支中，增加 `env->GlobalReference_Delete(callbackRef)` 释放引用后再 return/throw。
  3. 若意图是使用 `callbackDeleted` 标志防止重复释放（如 lambda 可能被多次调用），则需：将 `callbackDeleted` 加入 lambda 捕获列表，在 lambda 和错误分支中通过标志位守卫 `GlobalReference_Delete` 调用。当前实现不完整，应补全或移除死代码。
  4. 推荐测试：构造 `requestDialogService` 调用后断言全局引用计数不增长的单元测试（参考 `ets_environment_test` 中 `MockAniEnv::state_.globalRefDeleteCount` 的计数模式）。

### [LOG-01] find_if lambda 中 Reference_StrictEquals 返回值未检查（一致性遗漏） (P2, scanner=logic-scanner)

- **位置**：`frameworks/ets/ani/ability_manager/src/ets_ability_manager.cpp:921, 995`
- **触发路径**：通过 ArkTS 调用 `NativeOffPreloadedUIExtensionAbilityLoaded` / `NativeOffPreloadedUIExtensionAbilityDestroyed`（给定非 undefined callback）→ `Reference_IsUndefined` 检查通过（line 904/978，本次提交新增）→ 进入 `find_if` 遍历回调列表 → `env->Reference_StrictEquals(callback, cb.first, &isEquals)`（line 921/995）返回值未检查 → 若调用失败，`isEquals` 保持 `ANI_FALSE` → lambda 返回 false → 回调未被匹配 → `find_if` 返回 `end()` → 日志报 "callback not found" → 函数返回，回调列表中的全局引用未被清理。
- **影响**：安全失败模式（回调未找到 → 日志报错 → 用户可重试或等 ability 销毁时清理），无崩溃/数据损坏。但与本次提交"清理所有未检查 ANI 返回值"的意图不一致——同一函数内 `Reference_IsUndefined` 已加检查，`Reference_StrictEquals` 却遗漏，属一致性缺陷。若 `Reference_StrictEquals` 持续失败，将导致回调列表中的全局引用无法通过 "off" 操作清理，形成潜在资源泄漏。
- **证据**：
  - `ets_ability_manager.cpp:919-922`：`find_if` lambda 中 `env->Reference_StrictEquals(callback, cb.first, &isEquals)` 无返回值检查。
  - `ets_ability_manager.cpp:993-996`：同一模式重复出现。
  - 对比同函数 line 904（本次提交新增 `Reference_IsUndefined` 检查）和 line 921（未检查），不一致。
  - 对比 `ets_ability_delegator.cpp:1259-1262`（本次提交为 `Reference_StrictEquals` 失败新增 `++iter; continue;`），同提交内对 `Reference_StrictEquals` 失败的处理策略不统一。
- **建议**：在 `find_if` lambda 内检查 `Reference_StrictEquals` 返回值，失败时记录日志并返回 false（跳过该元素）。或改用手动循环（参照 `ets_ability_delegator.cpp` 的模式），对失败元素 `++iter; continue`。
