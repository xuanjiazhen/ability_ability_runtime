# 代码检视报告 — dump_runtime_helper.cpp@c9dfedc3（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。章节顺序、字段名、报告元数据块、评分与门禁规则均为固定格式，便于门禁脚本解析与历史对比。
> 编排入口：[README.md](skills/codecheck/README.md) → Step 5；合并逻辑见 [orchestrator/SKILL.md](skills/codecheck/orchestrator/SKILL.md)。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "frameworks/native/appkit/app/dump_runtime_helper.cpp (commit c9dfedc3)"
  round: 1
  commit_id: "c9dfedc32031e66462c295452d329879dce44422"
  change_id: "N/A"
  report_id: "c9dfedc3-R1"
  date: "2026-09-20"
  gate_decision: "approve"
  risk_level: "low"
  score: 98
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  findings_total: 1
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 1}
  gate_blockers: []
  must_fix: []
  followups: ["FIND-01"]
```
<!-- codecheck-report-metadata:end -->


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **98/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 1 项 |

**一句话结论**：本提交为单行常量变更（`SO_NAME` 由 `libmemleak.z.so` 改为 `liborphanmem.z.so`），安全域 11 项 Core Scan 与逻辑域 7 项维度全部通过，无 P0/P1 危项；唯一观察项为库名与上游产物改名的跨模块同步契约风险（P3，建议合入前跨仓确认），可正常上库。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| FIND-01 | P3 | dlopen 库名改为 `liborphanmem.z.so`，属跨模块外部契约，仓内无法校验与上游产物改名是否同步；若错配则 native 泄漏/快照 dump 静默降级 | 合入前与库侧模块（hiviewdfx）确认产物名同步；失败路径已 fail-closed，无需代码改动 | 合入前确认 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 11 项 Core Scan 全部通过/不适用；G 类模式库无 dlopen 类目，全库无遗漏改名点 |
| logic-scanner | ⚠️ 1 项 P3 | 变更类型=常量值修改；dlopen 失败路径错误处理完整（:430-448 / :461-476 / :489-521）；同步契约风险并入 FIND-01 |
| input-scanner | N/A | 本提交仅改编译期常量，不触碰外部输入→持久化链路，无适用面 |
| api-scanner | N/A | 变更不涉及 `interfaces/kits/`、NAPI/ANI/C 绑定，无对外 API 面 |

---

## 6. 关键发现详情

### [FIND-01] dlopen 库名改为 liborphanmem.z.so 的跨模块契约同步风险（P3, scanner=security+logic）

- **位置**：`frameworks/native/appkit/app/dump_runtime_helper.cpp:65`（dlopen 使用点 :436）
- **触发路径**：`DumpMem(NATIVE)` → `GetDumpResult`(:454) / `GetSnapshot`(:482) → `LoadMemLeakFunc`(:436) `dlopen("liborphanmem.z.so")`。若设备上实际产物仍为旧名 `libmemleak.z.so`（版本/合入顺序错配），dlopen 失败 → 两路 dump fail-closed 返回 false，仅 HILOG 提示，native 内存泄漏抓取与快照功能静默降级。
- **影响**：仅 DFX 诊断功能降级；无崩溃、无安全边界影响。SO_NAME 为编译期常量，无注入攻击面（已排除反序列化四件套与敏感信息类）。
- **证据**：本次 diff 将 :65 常量改为 `"liborphanmem.z.so"`；:436 `dlopen(SO_NAME, RTLD_LAZY)`；:430-448 失败路径返回 nullptr 并在 dlsym 失败时 dlclose；:461-476、:489-521 各失败路径 free+dlclose 配对、错误处理完整（本次变更未破坏任何资源配对）。全库 grep `libmemleak`/`liborphanmem` 仅 :65 一处引用，无遗漏的旧名引用点。
- **建议**：合入前与库侧 hiviewdfx 模块确认 `liborphanmem.z.so` 产物名已同步（提交为仓外契约，本仓无法自证）；如担心线上错配可保留加载失败时的可归因日志（现有 :437-440 已满足）。UT 建议覆盖同名桩库下 dlopen 成功/失败两条路径。

---

## refute_log

见 [codecheck_reports/c9dfedc3_raw/refute_log.md](codecheck_reports/c9dfedc3_raw/refute_log.md)：SEC-01 与 LOG-01 同根因（库名为跨模块契约、修复动作相同）→ 🔀 合并为 FIND-01（P3）；无 P0/P1，未触发强证据推翻要求。