# 代码检视报告 — InsightIntent 双模式适配（Round 5 / 最终）

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
  scope: "services/abilitymgr/src/insight_intent (InsightIntent DualMode)"
  round: 5
  commit_id: "3337c2f6aa8d2ad5fb099303656f2ee26fe9acd4"
  change_id: "I945681bcd6b6ba94b567f5ce16e19c634f405975"
  report_id: "I945681bcd6b6ba94b567f5ce16e19c634f405975-R5"
  date: "2026-10-13"
  gate_decision: "approve"
  risk_level: "low"
  score: 92
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 4
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 4}
  gate_blockers: []
  must_fix: []
  followups: ["CC-06", "CC-07", "CC-08", "CC-09"]
```
<!-- codecheck-report-metadata:end -->


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | ✅ low |
| 评分 | **92/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 4 项（均为 P3 观察项） |

**一句话结论**：全部 P0/P1/P2 已修复或处置，CC-10~CC-14 经确认非问题，TDD mock 已补齐编译，剩余 4 项 P3 观察项，评分 92，可上库。

### 审查范围

| 项目 | 值 |
|---|---|
| 提交范围 | `a70bcc1392`..`3337c2f6aa`（6 笔提交） |
| 核心变更 | `services/abilitymgr/src/insight_intent/` InsightIntent 双模式（DualMode）适配 |
| 最新提交 | `3337c2f6aa` 双模式意图查询fix（含 CC-04/CC-05 修复 + TDD mock 补齐） |

### 已处置项（全轮次变更）

| ID | 原始等级 | 处置 | 修复提交 | 说明 |
|---|---|---|---|---|
| CC-01 | P1 | **已修复** | `22808dcfe2` | `LoadInsightIntentBundleInfos`/`LoadInsightIntentInfos` 中 bundleVersionMap_ key 格式按段数判断 |
| CC-02 | P1 | **设计如此** | — | `ResolveDualModeIfNeeded` 在 BMS 不可用时设 `appIndex=-1` 是有意设计——防止跨模式查询 |
| CC-03 | P1 | **已修复** | `22808dcfe2` | `appIndex`/`isDualMode` 赋值移入 `AddRecord`（锁内），消除 `records_` 无锁数据竞争 |
| CC-04 | P2 | **已修复** | `3337c2f6aa` | `DeleteIntentByAppIndex` 中将 `QueryDataBeginWithKey` 移入 `rdbStorePtrMutex_` 锁内 |
| CC-05 | P2 | **已修复** | `3337c2f6aa` | 新增 `RemoveBundleVersionCacheEntry`，在 RDB 删除后同步清理 `bundleVersionMap_` |
| CC-10 | P3 | **非问题** | — | ResolveSrcEntry 新增分支经确认非问题 |
| CC-11 | P3 | **非问题** | — | SaveStorageInsightIntentData 部分失败处理经确认非问题 |
| CC-12 | P3 | **非问题** | — | SaveBatchInsightIntentTotalInfo 事务保护经确认非问题 |
| CC-13 | P3 | **非问题** | — | RDB key 分隔符防护经确认非问题 |
| CC-14 | P3 | **非问题** | — | CheckAndUpdateQueryEntityParam 参数经确认非问题 |

### 评分变化

| 轮次 | P0 | P1 | P2 | P3 | 评分 | 决策 |
|---|---|---|---|---|---|---|
| Round 1 | 0 | 3 | 10 | 1 | 12 | conditional |
| Round 2 | 0 | 0 | 2 | 9 | 72 | conditional |
| Round 3 | 0 | 0 | 0 | 9 | 82 | conditional |
| Round 4 | 0 | 0 | 0 | 4 | 92 | approve |
| **Round 5** | **0** | **0** | **0** | **4** | **92** | **approve** |

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议跟进（P3 观察项）

| ID | 问题 | 建议行动 | 排期 |
|---|---|---|---|
| CC-06 | appIndex 来源可能不匹配（bundleInfo.appIndex vs preferredAppIndex） | 统一保存和查询路径的 appIndex 来源；证据等级 Likely，待确认 | 下一补档 |
| CC-07 | DeleteInsightIntentEvent skip 条件对非 10000 的 DualMode appIndex 可能误伤 | 当前 PC_TABLET_INDEX=10000 正确覆盖；扩展时需更新 | 下一补档 |
| CC-08 | UnregisterInsightIntentFunctions 全量注销影响同名其他模式 | CliTool 支持按 appIndex 精确注销，或注销后触发其他模式重新注册 | 下一补档 |
| CC-09 | SaveBatchInsightIntentTotalInfo 用 front() 代表整个 batch 的 map key 格式 | 遍历所有 saveParam 分别更新 map key，或添加 batch 一致性断言 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | 8 条原始 → 0 条进入报告 | CC-01/CC-03/CC-04(已修复)、CC-10~CC-14(非问题)；appIndex 来源不匹配(P3) |
| logic-scanner | 10 条原始 → 0 条进入报告 | CC-01/CC-03/CC-05(已修复)、CC-02(设计如此)、CC-10~CC-14(非问题)；skip 条件/front() 假设/Unregister(P3) |
| input-scanner | 7 条原始 → 0 条进入报告 | CC-01/CC-02/CC-04/CC-05(已修复/设计如此)、CC-11~CC-13(非问题)；无剩余 P3 |

---

## 6. 关键发现详情

> 以下 P3 项为观察项，不阻塞上库，按需跟进。

### [CC-06] appIndex 来源可能不匹配 (P3, scanner=SEC)

- **位置**：`insight_intent_sys_event_receiver.cpp:216` vs `insight_intent_db_cache.cpp:56-59`
- **触发路径**：保存路径用 `bundleInfo.appIndex`，查询路径用 `GetDualModeBundleInfo` 返回的 `preferredAppIndex`（可能不同）→ RDB key 前缀不匹配 → 查询返回空。
- **影响**：如果两个值不一致，保存的 intent 数据无法被查询到。证据等级 **Likely**，未确认实际会不同。
- **建议**：统一 appIndex 来源，保存路径也使用 `GetDualModeBundleInfo` 的 `preferredAppIndex`，或在查询路径中使用 `bundleInfo.appIndex` 作为 fallback。

### [CC-07] DeleteInsightIntentEvent skip 条件 (P3, scanner=LOG)

- **位置**：`services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp:247`
- **触发路径**：DualMode 应用卸载 → appIndex 非 0 且非 10000 → 跳过删除。
- **影响**：当前 `PC_TABLET_INDEX=10000` 正确覆盖所有 DualMode 应用，逻辑**当前正确**。仅存在未来扩展风险。
- **建议**：当前无需修改；扩展时需更新判断条件。

### [CC-08] UnregisterInsightIntentFunctions 全量注销 (P3, scanner=LOG+INP)

- **位置**：`services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp:272`
- **触发路径**：DeleteInsightIntentEvent(appIndex=0) → DeleteIntentByAppIndex(只删 appIndex=0) → `UnregisterInsightIntentFunctions(bundleName, userId)`(全量注销) → 同名 appIndex=10000 的函数注册也被注销。
- **影响**：其他 DualMode 变体的函数注册被误删，intent 功能在下次注册前不可用。属设计限制——CliTool 注册系统不支持 appIndex 维度。
- **建议**：CliTool 支持按 appIndex 精确注销，或注销后触发其他模式重新注册。

### [CC-09] SaveBatchInsightIntentTotalInfo front() 假设 (P3, scanner=LOG)

- **位置**：`services/abilitymgr/src/insight_intent/insight_intent_db_cache.cpp:155-157`
- **触发路径**：`SaveBatchInsightIntentTotalInfo` → `saveParams.front().isDualMode` 判断 map key 格式 → 如果 batch 混合 DualMode/非 DualMode → 只有第一个 saveParam 的模式反映在 map key 中。
- **影响**：当前所有调用方为 batch 设置相同的 `appIndex`/`isDualMode`（未触发），但 API 契约脆弱。
- **建议**：遍历所有 saveParam 分别更新 map key，或添加 batch 一致性断言。
