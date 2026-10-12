# 代码检视报告 — dump_runtime_helper_test.cpp（Round 1 / 最新提交）

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp"
  round: 1
  commit_id: "8db502706733b823dfb67974186de557c7c77411"
  change_id: "N/A"
  report_id: "8db50270-R1"
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

**一句话结论**：本提交为纯测试变更（`dump_runtime_helper_test.cpp` 新增 8 个 HWTEST，未改动任何生产代码文件），测试断言整体与 `dump_runtime_helper.cpp` 实现一致，无 P0/P1 危项；仅存在测试逻辑覆盖不达预期、测试环境参数残留等 P3 级观察项。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P3 | `Check2DQuota_0400` 未真正走到其描述的分支（负 `oomdump.maxcount`），断言为"空过"通过 | 使测试真正触达 `persist.hiview.oomdump.maxcount<0` 分支后再断言，或修改 `@tc.desc` 与测试意图一致（先搭好 `OOM_QUOTA_PATH` / xattr 前置） | 下一补档 |
| LOG-02 | P3 | `Check2DQuota_0300/0400` 写 `persist.hiview.*` 持久化参数并以 `""` 复位，未恢复原值；`persist.` 属性跨重启持久，存在测试设备环境残留 | 保存原值并在 TearDown 恢复，或用非持久化参数避免污染 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 纯测试代码，无生产输入面/资源所有权/IPC 面；但检出测试侧 `persist.` 参数持久化残留（并入 LOG-02） |
| logic-scanner | ⚠️ 2 项 P3 | 测试断言与实现一致，但 `Check2DQuota_0400` 分支覆盖与 `@tc.desc` 不符；测试参数复位缺陷 |

---

## 6. 关键发现详情

### [LOG-01] Check2DQuota_0400 断言空过，未覆盖其声明的负 oomdump.maxcount 分支（P3, scanner=logic-scanner）

- **位置**：`test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp:345-356`
- **触发路径**：`Check2DQuota(true)` 在 env=`DFX_RESOURCE_OVERLIMIT_OPTIONS=oomdump:enable`、`persist.hiview.oomdump.process.maxcount=5` 时：先过 `Check2DOOMDumpOpt()`（进程 maxcount=5 不小于 0）→ 调用 `Get2DQuota(OOM_QUOTA_PATH, ...)`，而 `OOM_QUOTA_PATH=/data/storage/el2/base/cache/rawheap` 在 UT 中不存在 → `getxattr` 失败 → `Init2DOOMDumpQuota` → `Set2DQuota` 的 `setxattr` 失败 → `Check2DQuota` 在 dump_runtime_helper.cpp:239-241 提前 `return false`。**未到达** dump_runtime_helper.cpp:243-247 的 `persist.hiview.oomdump.maxcount<0` 校验分支。
- **影响**：`EXPECT_FALSE` 通过但验证的并不是测试描述所谓"oomdump maxcount 为负"的路径，属空覆盖（false 因路径不存在而非因负 maxcount）。降低对 `Check2DQuota` 负 maxcount 边界逻辑的可信度。
- **证据**：`dump_runtime_helper.cpp:236` `Get2DQuota` 失败 → `:238` `Init2DOOMDumpQuota` → `:742` `Set2DQuota` 对不存在路径 `setxattr` 失败返回 false → `:239-241` `return false`；因此 `:243-247` 的负值校验不可达。
- **建议**：让测试真实铺底（创建 `OOM_QUOTA_PATH` 目录并写入可解析的 `user.oomdump.quota` xattr 前置），使流程越过 `Get2DQuota` 分支后再断言负 `maxcount`；或至少修正 `@tc.desc` 表明当前仅验证"流程提前返回"，避免误导后续维护者。

### [LOG-02] Check2DQuota_0300/0400 写持久化 persist 参数并以空串复位，产生测试环境残留（P3, scanner=logic-scanner）

- **位置**：`test/unittest/frameworks_kits_appkit_native_test/dump_runtime_helper_test.cpp:329-356`
- **触发路径**：测试调用 `OHOS::system::SetParameter("persist.hiview.oomdump.process.maxcount", "-1"/"5")` 与 `("persist.hiview.oomdump.maxcount", "-1")`，结束时以 `""` 复位。`persist.` 前缀属性为**持久化**参数（跨进程、跨重启保留），且未恢复测试执行前的原始值。
- **影响**：若测试设备/仿真环境预先配置了这两个属性，测试会覆盖并清零，且负值 `persist.hiview.oomdump.process.maxcount=-1` 在被持久化后可能影响该设备后续 OOM dump 判定。属测试对环境状态的破坏性副作用（持久的、非本进程隔离的）。
- **证据**：`dump_runtime_helper_test.cpp:333,335,350,353` 多次 SetParameter；复位值仅 `""` 而非原值。对比 `SetUp/TearDown`（:46, :51）仅处理 `hiview.oomdump.switch`（非 persist），未统一管理本批新增参数。
- **建议**：在 `TearDown` 中保存/恢复这些 `persist.` 属性的**原始值**，或改用非持久化（无 `persist.` 前缀）参数避免污染跨重启环境。

---

## refute_log（P3 无需必审，供人工复核）

| finding | 判定 | 说明 |
|---|---|---|
| LOG-01 | ✅ 维持 | 基于源到源 trace（`Get2DQuota`→`Init2DOOMDumpQuota`→`Set2DQuota` 路径提前 return）确认负 maxcount 分支不可达；属"空覆盖"而非断言错误 |
| LOG-02 | ✅ 维持 | `persist.` 前缀确为持久化参数，`""` 复位不等于恢复原值，影响成立但仅测试侧，评 P3 |
