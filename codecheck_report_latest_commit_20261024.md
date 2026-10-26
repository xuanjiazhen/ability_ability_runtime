# 代码检视报告 — 最新提交 4a1626a5（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 生成入口：[`skills/codecheck/README.md`](skills/codecheck/README.md) → Step 5；合并逻辑见 [`skills/codecheck/orchestrator/SKILL.md`](skills/codecheck/orchestrator/SKILL.md)。
> 权威评分与门禁规则为**通用规则**，不在输出报告中呈现；生成时必须按 [`skills/codecheck/conventions.md`](skills/codecheck/conventions.md) §7（等级归一化）/ §8（必检维度）/ §9（评分与决策矩阵）计算，不得自创分值。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "commit-4a1626a5-编译整改"
  round: 1
  commit_id: "4a1626a55620203018a0ab2fb77317aa9a551441"
  change_id: "I1baa2f2fd7945551eaf2cdd15f6d72edfba31a6a"
  report_id: "I1baa2f2fd7945551eaf2cdd15f6d72edfba31a6a-R1"
  date: "2026-10-24"
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

**一句话结论**：本提交为纯 BUILD.gn 编译整改（向 41 个构建文件统一追加 `node:node_header_notice` 依赖），无 C++ 源码变更、无 API 签名变更、无安全/运行时影响；但存在一致性遗漏——5 个已发布 NAPI 模块、1 个 CJ FFI 模块、3 个 ANI 模块同样声明了 `napi:ace_napi` 却未被追加该依赖，建议补齐。

---

## 3. 必须立即处理（P0/P1）

**无。**　详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P2 | 5 个已发布 NAPI 模块声明了 `napi:ace_napi` 但未追加 `node:node_header_notice`，与本次整改目标不一致 | 向这 5 个 BUILD.gn 的 `external_deps` 追加 `"node:node_header_notice"` | 本轮或下一补档 |
| LOG-02 | P3 | CJ FFI `app_manager` 模块遗漏，同目录其余 5 个 cj/ffi 模块均已更新 | 向 `frameworks/cj/ffi/app/app_manager/BUILD.gn` 追加该依赖 | 下一补档 |
| LOG-03 | P3 | 3 个 ANI 模块遗漏，同目录其余 14 个 ANI 模块均已更新 | 向这 3 个 BUILD.gn 追加该依赖（ANI 为静态语言实现，优先级较低） | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | N/A | 本提交仅修改 BUILD.gn，无 C++ 源码变更；无 IPC、无持久化、无运行时行为变更，安全审查无适用面 |
| logic-scanner | ✅ 有发现 | 一致性分析发现 9 个 BUILD.gn 文件存在同模式遗漏（已声明 `napi:ace_napi` 但未追加 `node:node_header_notice`），详见 LOG-01/02/03 |
| api-scanner | N/A | 无 API 签名变更（`.d.ts`/`.h`/NAPI 注册入口均未修改）；BUILD.gn 仅追加构建依赖，不影响公共 API 兼容性 |

---

## 6. 关键发现详情

> P0/P1 必出全量卡片；P2/P3 按需精选或全出。

### [LOG-01] 5 个已发布 NAPI 模块遗漏 node:node_header_notice 依赖 (P2, scanner=logic-scanner)

- **位置**：
  - `frameworks/js/napi/task_manager/BUILD.gn:49`
  - `frameworks/js/napi/photo_editor_extension_ability/BUILD.gn:59`
  - `frameworks/js/napi/photo_editor_extension_context/BUILD.gn:59`
  - `frameworks/js/napi/quick_fix/BUILD.gn:49`
  - `frameworks/js/napi/extensioncontext/BUILD.gn:47`
- **触发路径**：本次提交向 41 个 BUILD.gn 统一追加 `node:node_header_notice` 到 `external_deps`，覆盖了 24 个 `frameworks/js/napi/` 模块；但上述 5 个模块同为 `ohos_shared_library`、同为 `part_name = "ability_runtime"`、同样在 `external_deps` 中声明了 `"napi:ace_napi"`，且编译 `*_module.cpp` NAPI 入口源文件，却未被追加。
- **影响**：编译整改不完整。若 `node:node_header_notice` 是许可证/声明合规依赖，这 5 个发布到设备的模块（安装路径 `module/app/agent`、`module/app/ability`、`module/application`）存在合规缺口；不影响运行时行为与安全性。
- **证据**：
  ```gn
  # frameworks/js/napi/task_manager/BUILD.gn:45-51（遗漏示例）
  external_deps = [
    "c_utils:utils",
    "hilog:libhilog",
    "ipc:ipc_core",
    "napi:ace_napi",       # ← 声明了 napi 依赖
    "samgr:samgr_proxy",
  ]                         # ← 缺少 "node:node_header_notice"

  # 对比：本次已更新的 frameworks/js/napi/extension_ability/BUILD.gn:45-49
  external_deps = [
    "napi:ace_napi",
    "node:node_header_notice",  # ← 已追加
  ]
  ```
- **建议**：向上述 5 个 BUILD.gn 的 `external_deps` 列表追加 `"node:node_header_notice"`，与本次已更新的同目录模块保持一致。

### [LOG-02] CJ FFI app_manager 模块遗漏 node:node_header_notice 依赖 (P3, scanner=logic-scanner)

- **位置**：`frameworks/cj/ffi/app/app_manager/BUILD.gn:54`
- **触发路径**：本次提交未修改任何 `frameworks/cj/ffi/` 文件，但仓库中已有 5 个 cj/ffi 模块（`want_agent`、`context`、`ark_interop_helper`、`app/errormanager`、顶层 `BUILD.gn`）声明了 `node:node_header_notice`（为更早的提交所加）。`app/app_manager` 同为 `ohos_shared_library`、`part_name = "ability_runtime"`、声明了 `napi:ace_napi`，却缺少该依赖。
- **影响**：CJ FFI 静态语言绑定模块的一致性遗漏；优先级低于 NAPI 动态模块（conventions §1 默认排除静态语言实现）。
- **证据**：
  ```gn
  # frameworks/cj/ffi/app/app_manager/BUILD.gn:50-58
  external_deps = [
    "c_utils:utils",
    "hilog:libhilog",
    "ipc:ipc_core",
    "napi:ace_napi",         # ← 声明了 napi 依赖
    "napi:cj_bind_ffi",
    "napi:cj_bind_native",
    "samgr:samgr_proxy",
  ]                           # ← 缺少 "node:node_header_notice"

  # 对比：frameworks/cj/ffi/want_agent/BUILD.gn:77-80（已有）
  external_deps = [
    ...
    "napi:ace_napi",
    "node:node_header_notice",  # ← 已有
  ]
  ```
- **建议**：向 `frameworks/cj/ffi/app/app_manager/BUILD.gn` 的 `external_deps` 追加 `"node:node_header_notice"`。

### [LOG-03] 3 个 ANI 模块遗漏 node:node_header_notice 依赖 (P3, scanner=logic-scanner)

- **位置**：
  - `frameworks/ets/ani/task_manager/BUILD.gn:56`
  - `frameworks/ets/ani/app/form_extension_context/BUILD.gn:67`
  - `frameworks/ets/ani/dialog_request_info/BUILD.gn:61`
- **触发路径**：本次提交向 14 个 `frameworks/ets/ani/` 模块追加了 `node:node_header_notice`，但上述 3 个 ANI 模块同为 `ohos_shared_library`、`part_name = "ability_runtime"`、声明了 `napi:ace_napi`，却未被追加。同一目录内的遗漏表明整改时未做全量 grep 覆盖。
- **影响**：ANI 静态语言绑定模块一致性遗漏；conventions §1 默认排除静态语言实现，优先级最低。
- **证据**：
  ```gn
  # frameworks/ets/ani/task_manager/BUILD.gn:52-59（遗漏示例）
  external_deps = [
    "c_utils:utils",
    "hilog:libhilog",
    "ipc:ipc_core",
    "napi:ace_napi",       # ← 声明了 napi 依赖
    "runtime_core:ani",
    "samgr:samgr_proxy",
  ]                         # ← 缺少 "node:node_header_notice"

  # 对比：本次已更新的 frameworks/ets/ani/wantagent/BUILD.gn:53-57
  external_deps = [
    ...
    "ipc:ipc_core",
    "node:node_header_notice",  # ← 本次追加
    "runtime_core:ani",
  ]
  ```
- **建议**：向上述 3 个 BUILD.gn 的 `external_deps` 追加 `"node:node_header_notice"`，与同目录已更新的 14 个模块保持一致。

---

## 附：观察项（不计入 findings_total）

- **`insight_intent_provider` 追加了依赖但无 `napi:ace_napi`**：`frameworks/js/napi/insight_intent/insight_intent_provider/BUILD.gn:51` 本次被追加 `node:node_header_notice`，但其 `external_deps` 中并无 `napi:ace_napi`（仅有 `hilog` 和 `ipc_single`）。该模块 `deps` 中引用了 `napi_common`，可能通过传递依赖引入 node 头文件——若如此则为正确追加；若否则为冗余依赖。无法仅凭 BUILD.gn 判定，标记为观察项，不阻塞。

## 附：Refute 记录

| ID | 判定 | 依据 |
|---|---|---|
| LOG-01 | ✅ 维持 P2 | 5 个模块均为发布到设备的 `ohos_shared_library`（`part_name=ability_runtime`），编译 NAPI 入口源文件 `*_module.cpp`，与已更新的 24 个同目录模块无本质区别；无上游 guard 或条件编译排除证据 |
| LOG-02 | ✅ 维持 P3 | `cj_app_manager_ffi` 与同目录已更新的 `cj_want_agent_ffi` 等模块同构；CJ FFI 为静态语言实现，P3 优先级适当 |
| LOG-03 | ✅ 维持 P3 | 3 个 ANI 模块与同目录已更新的 14 个模块同构；ANI 为静态语言实现，P3 优先级适当 |
