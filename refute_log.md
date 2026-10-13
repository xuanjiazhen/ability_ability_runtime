# Refute 审查记录

## 审查概要
| 指标 | 数值 |
|------|------|
| 审查范围 | security: 8 条, logic: 10 条, input: 7 条 |
| P0/P1 全审 | 10 条 |
| P2/P3 抽样审 | 4 条 |
| 维持 | 14 条 |
| 降级 | 4 条 (LOG-04 P1→P2, INP-03 P1→P2, INP-04 P1→P2, LOG-05+INP-02 P1→P2) |
| 推翻 | 0 条 |
| 合并 | 6 组 → 6 条 |
| **进入最终报告** | **14 条** (P0: 0, P1: 3, P2: 10, P3: 1) |

## 逐条审查

### 🔀 合并 — SEC-02 + LOG-01 + INP-01 → CC-01
合并后：bundleVersionMap_ key 格式不匹配——加载侧用纯 bundleName，读写侧用 bundleName/appIndex
审查结论：三个 scanner 独立发现同一根因。LoadInsightIntentBundleInfos/LoadInsightIntentInfos 用 res[BUNDLE_NAME] 构建 map key，而 Save/Has 方法用 bundleName/appIndex。确认 P1。

### 🔀 合并 — SEC-05 + SEC-06 + LOG-02 + LOG-03 + INP-07 → CC-02
合并后：ResolveDualModeIfNeeded 在 BMS 失败时设 appIndex=-1 与 ResolveDualModeContext 行为不一致
审查结论：五个发现描述同一问题。ResolveDualModeIfNeeded 设 isDualMode=true/appIndex=-1 导致查询必然 miss；ResolveDualModeContext 保持默认值。确认 P1。

### ✅ 维持 — SEC-01 → CC-03
records_ map 无锁访问。CheckAndUpdateParam 新增 records_.find(intentId) 及成员赋值未持 mutex_，而 AddRecord/HandleDiedRecordsCleanup 等在 mutex_ 下 erase。确认 P1（新引入的数据竞争）。

### 🔀 合并 — SEC-03 + LOG-08 + INP-05 → CC-04
合并后：DeleteIntentByAppIndex TOCTOU——query 在锁外，delete 在锁内
审查结论：三个 scanner 发现同一 TOCTOU。影响为并发时漏删 orphaned data，不会崩溃。维持 P2。

### 🔀 合并 — SEC-04 + LOG-06 → CC-05
合并后：DeleteInsightIntentEvent 绕过 DeleteInsightIntentTotalInfo，不更新 bundleVersionMap_
审查结论：DeleteInsightIntentEvent 直接调用 storage->DeleteIntentByAppIndex，不经过 dbCache 的缓存更新逻辑。维持 P2。

### ⬇️ 降级 — SEC-07 → CC-06
appIndex 来源不匹配（bundleInfo.appIndex vs GetDualModeBundleInfo preferredAppIndex）。
降级理由：证据等级为 Likely，未确认 preferredAppIndex 与 bundleInfo.appIndex 实际会不同。P2 维持。

### ⬇️ 降级 — LOG-04 → CC-07 (P1 → P2)
DeleteInsightIntentEvent skip 条件误伤 DualMode 应用。
降级理由：经查 global_constant.h，PC_TABLET_INDEX=10000 是 DualMode 应用的专用 appIndex。`appIndex != PC_TABLET_INDEX` 正确放行了 DualMode 应用。该模式在代码库中广泛使用（start_ability_utils.cpp:78, dlp_utils.h:56 等）。当前逻辑正确，仅存在未来扩展风险。降为 P2。

### ⬇️ 降级 — LOG-05 + INP-02 → CC-08 (P1 → P2)
UnregisterInsightIntentFunctions 不支持 appIndex 精确注销。
降级理由：原代码已调用 UnregisterInsightIntentFunctions(bundleName, userId) 全量注销。本次变更使 RDB 删除 appIndex-aware 但 CliTool 侧未跟进，属设计限制而非新引入 bug。降为 P2。

### ✅ 维持 — LOG-07 → CC-09
SaveBatchInsightIntentTotalInfo 用 saveParams.front() 代表整个 batch。当前调用方保证一致性，但 API 契约脆弱。P2。

### ✅ 维持 — LOG-09 → CC-10
ResolveSrcEntry 新增 else if (!decoratorClass.empty()) 分支读取可能未设置的 INSIGHT_INTENT_SRC_ENTRANCE。P2。

### ⬇️ 降级 — INP-03 → CC-11 (P1 → P2)
SaveStorageInsightIntentData 部分失败返回 ERR_OK。
降级理由：该行为为预存问题（InsertData 失败仅日志不 return 的模式在变更前已存在），本次提交仅修改了函数签名。功能影响有限（部分条目缺失导致 intent 不完整）。降为 P2。

### ⬇️ 降级 — INP-04 → CC-12 (P1 → P2)
SaveBatchInsightIntentTotalInfo 先删后写无事务保护。
降级理由：delete-then-write 模式为预存模式，非本次提交引入。触发需 JSON 序列化失败或 BatchInsertData 失败，概率较低。降为 P2。

### ✅ 维持 — INP-06 → CC-13
RDB key 分隔符 `/` 未对 bundleName/moduleName 做注入防护。BMS 有命名约束但无代码层纵深防御。P2。

### 🔀 合并 — SEC-08 + LOG-10 → CC-14
合并后：CheckAndUpdateParam/CheckAndUpdateQueryEntityParam 新增的 appIndex/isDualMode 参数为死代码。
审查结论：参数存储到 record 但无读取方；CheckAndUpdateQueryEntityParam 完全不使用。P3。

## refute 统计
- 审查耗时：~3 轮
- 跨 scanner 去重：6 组合并为 6 条
- 降级：4 条（均从 P1 降为 P2，理由：预存问题/设计限制/模式正确性已验证）
- 推翻：0 条
