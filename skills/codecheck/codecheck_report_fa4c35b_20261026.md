# 代码检视报告 — fa4c35b 编译整改（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 生成入口：[`README.md`](README.md) → Step 5；合并逻辑见 [`orchestrator/SKILL.md`](orchestrator/SKILL.md)。
> 权威评分与门禁规则为**通用规则**，不在输出报告中呈现；生成时必须按 [`conventions.md`](conventions.md) §7（等级归一化）/ §8（必检维度）/ §9（评分与决策矩阵）计算，不得自创分值。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "fa4c35b_编译整改"
  round: 1
  commit_id: "fa4c35b36732ff6a7eca33eb30197518e14d8d10"
  change_id: "I1baa2f2fd7945551eaf2cdd15f6d72edfba31a6a"
  report_id: "I1baa2f2fd7945551eaf2cdd15f6d72edfba31a6a-R1"
  date: "2026-10-26"
  gate_decision: "approve"
  risk_level: "low"
  score: 91
  dimensions_required: ["security-scanner", "logic-scanner", "api-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "api-scanner"]
  findings_total: 3
  findings_by_severity: {P0: 0, P1: 0, P2: 1, P3: 2}
  gate_blockers: []
  must_fix: []
  followups: ["LOG-01", "LOG-02", "LOG-03"]
```
<!-- codecheck-report-metadata:end -->


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **91/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 3 项 |

**一句话结论**：本提交为纯 BUILD.gn 编译整改（向 40 个 BUILD.gn 的 external_deps 统一添加 `node:node_header_notice`），无源代码变更，无安全/逻辑/API 签名风险；发现 1 项 P2（修改文件内漏改一个 target）和 2 项 P3（覆盖范围遗漏），建议跟进补齐。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P2 | `cj_insightintentcontext` 目标在本提交修改的文件内漏改 `node:node_header_notice` | 向 `frameworks/native/insight_intent/insight_intent_context/BUILD.gn:101-112` 的 `cj_insightintentcontext` 目标 external_deps 添加 `"node:node_header_notice"` | 本轮或下一补档 |
| LOG-02 | P3 | 5 个 NAPI BUILD.gn 文件（`task_manager`、`photo_editor_extension_ability`、`photo_editor_extension_context`、`quick_fix`、`extensioncontext`）有 `napi:ace_napi` 但缺 `node:node_header_notice` | 向这 5 个文件的 external_deps 添加 `"node:node_header_notice"` | 下一补档 |
| LOG-03 | P3 | 3 个 ANI BUILD.gn 文件（`task_manager`、`form_extension_context`、`dialog_request_info`）有 `napi:ace_napi` 但缺 `node:node_header_notice`（ANI 属静态语言排除范围，观察项） | 按需补充，保持与 NAPI 侧一致 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | N/A | 无源代码变更（.cpp/.h），不存在内存安全/并发/UAF/权限绕过等攻击面 |
| logic-scanner | 有发现 | 逻辑一致性分析发现 `node:node_header_notice` 添加覆盖不完整：1 个修改文件内 target 漏改 + 5 个 NAPI 文件 + 3 个 ANI 文件遗漏 |
| api-scanner | N/A | 无 API 签名变更（.d.ts/.h 接口未改动），仅 BUILD.gn external_deps 新增构建依赖，不影响对外 API 兼容性 |

---

## 6. 关键发现详情

### [LOG-01] cj_insightintentcontext 目标在已修改文件内漏改 node:node_header_notice (P2, scanner=logic-scanner)

- **位置**：`frameworks/native/insight_intent/insight_intent_context/BUILD.gn:67`
- **触发路径**：本提交修改了 `frameworks/native/insight_intent/insight_intent_context/BUILD.gn`，向 `insightintentcontext` 目标（line 24-65）的 external_deps 添加了 `"node:node_header_notice"`（line 60）。但同一文件内的 `cj_insightintentcontext` 目标（line 67-117）同样有 `"napi:ace_napi"`（line 109）依赖，却未添加 `"node:node_header_notice"`。两个目标共享源文件 `insight_intent_context.cpp`（line 31 vs line 82），且 `cj_insightintentcontext` 额外依赖 `napi:cj_bind_ffi`/`napi:cj_bind_native`。
- **影响**：构建合规检查覆盖不完整。`node:node_header_notice` 是编译期 Node.js 头文件版权声明合规检查目标，缺失不影响运行时功能，但导致 `cj_insightintentcontext` 目标不满足 header notice 合规要求。由于该文件被本提交明确修改，同文件内一个目标已改、另一个漏改属于提交内部一致性问题。
- **证据**：
  - `frameworks/native/insight_intent/insight_intent_context/BUILD.gn:60` — `insightintentcontext` 目标已添加 `"node:node_header_notice"`
  - `frameworks/native/insight_intent/insight_intent_context/BUILD.gn:109` — `cj_insightintentcontext` 目标有 `"napi:ace_napi"` 但无 `"node:node_header_notice"`
  - git diff 确认本提交仅在该文件 line 60 处添加，未触及 line 67-117 的 `cj_insightintentcontext` 目标
- **建议**：向 `cj_insightintentcontext` 目标的 external_deps（line 101-112）按字母序在 `"napi:cj_bind_native"` 之后添加 `"node:node_header_notice"`。

### [LOG-02] 5 个 NAPI BUILD.gn 文件完全遗漏 node:node_header_notice (P3, scanner=logic-scanner)

- **位置**：
  - `frameworks/js/napi/task_manager/BUILD.gn`
  - `frameworks/js/napi/photo_editor_extension_ability/BUILD.gn`
  - `frameworks/js/napi/photo_editor_extension_context/BUILD.gn`
  - `frameworks/js/napi/quick_fix/BUILD.gn`
  - `frameworks/js/napi/extensioncontext/BUILD.gn`
- **触发路径**：本提交向 24 个 NAPI BUILD.gn 文件添加了 `node:node_header_notice`，但上述 5 个文件同样有 `"napi:ace_napi"` 依赖却未被触及。统计：NAPI 侧 84 个含 `napi:ace_napi` 的 BUILD.gn 中 79 个已有 `node:node_header_notice`，5 个遗漏。
- **影响**：构建合规检查覆盖范围不完整。这些 NAPI 模块使用 Node/NAPI 头文件（源文件含 napi 相关代码），但缺少 header notice 合规检查。不影响运行时功能。
- **证据**：
  - `frameworks/js/napi/task_manager/BUILD.gn` — external_deps 含 `"napi:ace_napi"` 无 `"node:node_header_notice"`，源文件 `js_task_manager.cpp`/`task_manager_module.cpp` 使用 NAPI API
  - `frameworks/js/napi/photo_editor_extension_ability/BUILD.gn` — `external_deps = [ "napi:ace_napi" ]` 单行，未扩展为多行添加
  - `frameworks/js/napi/extensioncontext/BUILD.gn` — 同上
  - `frameworks/js/napi/quick_fix/BUILD.gn` — external_deps 含 `"napi:ace_napi"` 无 `"node:node_header_notice"`
- **建议**：向这 5 个文件的 external_deps 添加 `"node:node_header_notice"`，与已修改的 24 个 NAPI 文件保持一致。

### [LOG-03] 3 个 ANI BUILD.gn 文件遗漏 node:node_header_notice（静态语言排除范围，观察项） (P3, scanner=logic-scanner)

- **位置**：
  - `frameworks/ets/ani/task_manager/BUILD.gn`
  - `frameworks/ets/ani/app/form_extension_context/BUILD.gn`
  - `frameworks/ets/ani/dialog_request_info/BUILD.gn`
- **触发路径**：本提交向 14 个 ANI BUILD.gn 文件添加了 `node:node_header_notice`，但上述 3 个文件同样有 `"napi:ace_napi"` 依赖却未被触及。统计：ANI 侧 29 个含 `napi:ace_napi` 的 BUILD.gn 中 26 个已有 `node:node_header_notice`，3 个遗漏。
- **影响**：与 LOG-02 同——构建合规覆盖不完整。注意：ANI 目录属静态语言排除范围（conventions.md §1），本项为观察记录，不计入 P0/P1。
- **证据**：
  - `frameworks/ets/ani/task_manager/BUILD.gn` — 含 `"napi:ace_napi"` 无 `"node:node_header_notice"`
  - `frameworks/ets/ani/app/form_extension_context/BUILD.gn` — 同上
  - `frameworks/ets/ani/dialog_request_info/BUILD.gn` — 同上
- **建议**：按需补充，保持与已修改的 14 个 ANI 文件一致。

---

## 附：检视范围与方法

- **提交**：`fa4c35b36732ff6a7eca33eb30197518e14d8d10`（编译整改，40 文件 +84/-21）
- **变更性质**：纯 BUILD.gn 变更，向 `frameworks/ets/ani/`（14 文件）、`frameworks/js/napi/`（24 文件）、`frameworks/native/`（2 文件）的 external_deps 统一添加 `"node:node_header_notice"` 构建依赖。无 .cpp/.h/.js/.ts/.d.ts 源代码变更。
- **路径特征探测**：API 信号（`frameworks/js/napi/`）→ 必检维度 = security + logic + api
- **scanner 执行**：
  - security-scanner：N/A（无源代码，不存在内存/并发/UAF/权限攻击面）
  - logic-scanner：执行一致性分析，发现覆盖遗漏
  - api-scanner：N/A（无 API 签名变更，不影响对外兼容性）
- **Refute 日志**：[`refute_log.md`](refute_log.md)
