# 代码检视报告 — dump_runtime_helper.cpp + dump_runtime_helper_test.cpp（Round 2 / 最新提交）

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "frameworks/native/appkit/app/dump_runtime_helper.cpp + test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp"
  round: 2
  commit_id: "bf6e979b166ef8b5688bd0a742ba74eb474aeb4b"
  change_id: "N/A"
  report_id: "bf6e979b-R2"
  date: "2026-09-01"
  gate_decision: "approve"
  risk_level: "low"
  score: 96
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  findings_total: 2
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 2}
  gate_blockers: []
  must_fix: []
  followups: ["LOG-01", "LOG-02"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **96/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 2 项 |

**一句话结论**：本提交（"fix csec warning"）含一处生产代码修复 + 新增 8 个测试。生产修复 `dump_runtime_helper.cpp:253-256` 将 `Check2DQuota` 内的 `Set2DQuota` 返回值从"忽略、无条件 return true"改为"失败即 return false"，消除了 csec 未检查返回值告警，语义更严谨（无法持久化配额扣减则不放行 dump）；无 P0/P1 危项，仅存测试侧 2 项 P3 观察项。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P3 | `Check2DQuota_0400` 未真正走到其描述的分支（负 `oomdump.maxcount`），断言为"空过"通过 | 使测试真正触达 `persist.hiview.oomdump.maxcount<0` 分支后再断言，或修正 `@tc.desc` 与测试意图一致 | 下一补档 |
| LOG-02 | P3 | `Check2DQuota_0300/0400` 写 `persist.hiview.*` 持久化参数并以 `""` 复位，未恢复原值；`persist.` 属性跨重启持久，存在测试设备环境残留 | 保存原值并在 TearDown 恢复，或用非持久化参数 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 生产改动仅封装 `Set2DQuota` 布尔返回，无新增输入面/权限/内存/IPC 影响；测试侧 `persist.` 参数残留并入 LOG-02 |
| logic-scanner | ⚠️ 2 项 P3 | 生产修复逻辑正确（见下）；测试 `Check2DQuota_0400` 分支覆盖与 `@tc.desc` 不符，参数复位缺陷 |

### 生产改动逻辑复核（lg）— 无发现

- **改动**：`dump_runtime_helper.cpp:253-256`，`Set2DQuota(...)` 失败时 `Check2DQuota` 返回 `false`。
- **正例**：`caller=appfreezeFilterCallback`（:149 `ret2D=Check2DQuota(...)`，:151 `!ret2custom && !ret2D && !ret2C` 则 no dump）。新语义下 `Set2DQuota` 失败→`ret2D=false`→若 2C/custom 也失败则不再 dump。相比原"忽略失败继续 dump"，**无法扣减配额时抑制 dump** 是更安全、正确的选择；`hidumper` 场景（`!needDecreaseQuota`，:141-147）在到达本逻辑前已提前 return，不受影响。
- **边界**：`g_oomDumpProcessQuota-1` 为负（=0 时）→ `Set2DQuota(newQuota<0)` 现返回 false→`Check2DQuota=false`；而周内 `g_oomDumpProcessQuota<=0` 已在 :796 提前拦截，负面情况仅在进程 maxcount=0 复位后出现，属合理保守行为，非缺陷。
- **排除**：`needDecreaseQuota` 形参在 `Check2DQuota` 体内未使用，改动不改变其语义。

---

## 6. 关键发现详情

### [LOG-01] Check2DQuota_0400 断言空过，未覆盖其声明的负 oomdump.maxcount 分支（P3, scanner=logic-scanner）

- **位置**：`test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp:345-356`
- **触发路径**：`Check2DQuota(true)` 在 env=`oomdump:enable`、process.maxcount=5 时：过 `Check2DOOMDumpOpt` → `Get2DQuota(OOM_QUOTA_PATH=.../rawheap)` 因 UT 中路径不存在而失败（`dump_runtime_helper.cpp:236`）→ `Init2DOOMDumpQuota`→`Set2DQuota` 的 `setxattr` 失败 → `Check2DQuota` 在 `dump_runtime_helper.cpp:239-241` 提前 `return false`，**未到达** :243-247 的负 `oomdump.maxcount` 校验。
- **影响**：`EXPECT_FALSE` 通过但验证的并非"maxcount 为负"路径，属空覆盖；对负 maxcount 边界逻辑无检测力。
- **证据**：`dump_runtime_helper.cpp:236-241` 在 :243-247 之前 return。
- **建议**：铺好 `OOM_QUOTA_PATH` xattr 前置使流程越过 `Get2DQuota` 后再断言负 `maxcount`，或修正 `@tc.desc`。

### [LOG-02] Check2DQuota_0300/0400 写持久化 persist 参数并以空串复位，产生测试环境残留（P3, scanner=logic-scanner）

- **位置**：`test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp:329-356`
- **触发路径**：`SetParameter("persist.hiview.oomdump.process.maxcount", "-1"/"5")` 与 `("persist.hiview.oomdump.maxcount", "-1")`，结束时以 `""` 复位。`persist.` 前缀为持久化参数（跨进程、跨重启），未恢复原值。
- **影响**：覆盖并清零设备预配置值，负值 `persist...maxcount=-1` 持久化后可能影响该设备后续 OOM dump 判定。
- **证据**：`:333,335,350,353` 多次 SetParameter，复位仅 `""` 非原值；`SetUp/TearDown`（:46,:51）未统一管理新增参数。
- **建议**：在 TearDown 保存/恢复原值，或改用非持久化参数。

---

## refute_log（P0 无，P3 供人工复核）

| finding | 判定 | 说明 |
|---|---|---|
| LOG-01 | ✅ 维持 | 源到源 trace 确认负 maxcount 分支不可达（提前 return），属空覆盖 |
| LOG-02 | ✅ 维持 | `persist.` 为持久化参数，`""` 复位≠恢复原值，影响成立但仅测试侧，P3 |
| （生产改动） | ✅ 维持为无发现 | csec 修复语义正确，返回 false 属合理保守，无逻辑回归 |
