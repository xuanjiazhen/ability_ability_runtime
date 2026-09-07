# 代码检视报告 — utils_0820（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。本报告遵循 [`codecheck_report_TEMPLATE.md`](codecheck_report_TEMPLATE.md) 固定格式。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "utils_0820"
  round: 1
  commit_id: "0527f53c1939cc9bc788ba7b218d26d45a2edc0f"
  change_id: "N/A"
  report_id: "0527f53c-R1"
  date: "2026-08-20"
  gate_decision: "approve"
  risk_level: "low"
  score: 91
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
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

**一句话结论**：本次提交为纯 BUILD.gn 依赖补全（19 处 `external_deps` 增补 `bounds_checking_function:libsec_shared`），逐条核对依赖名/字段/目标/c_utils 配对均正确，与仓内既有用法一致，未引入缺陷；逻辑维度发现 1 项 P2 一致性缺口（`ability_runtime` 共享库编译同一批 securec 源却未补依赖）与 2 项 P3 跟进建议，不阻塞上库。

> 路径特征：16 个变更文件全部为 `.gn`/`.gni` 构建配置，无 `.cpp`/`.h`/`.d.ts`。故 security-scanner / input-scanner / api-scanner 无适用代码面；security-scanner 按 conventions §8.2 计入 `dimensions_executed` 并标注 N/A，不计为缺失，决策不为 insufficient。

---

## 3. 必须立即处理（P0/P1）

**无。**　详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P2 | `frameworks/c/ability_runtime:ability_runtime` 共享库编译 9 个 `#include "securec.h"` 源且依赖 `c_utils:utils`，但未补 `bounds_checking_function:libsec_shared`；同一批源在对应 unittest target 已补，处理不一致 | 在该 target `external_deps` 显式补 `"bounds_checking_function:libsec_shared",`，与 test 侧对齐 | 本轮或下一补档 |
| LOG-02 | P3 | `services/dataobsmgr:dataobsms` 编译 `dataobs_mgr_changeinfo.cpp`/`dataobs_mgr_service.cpp`（均 include securec.h）且依赖 `c_utils:utils`，仅姊妹 target `dataobs_manager` 已补 | 评估 `dataobsms` 当前是否经其他 dep 间接获取 libsec_shared，收尾迁移时一并显式补齐 | 迁移收尾 |
| LOG-03 | P3 | 仓库内另有多个 include securec.h 且依赖 c_utils 的 target 未在本提交范围（如 `services/appmgr`、`frameworks/native/appkit`、`frameworks/native/ability/native` 等） | c_utils 迁移收尾时做一次全量交叉核对：`securec include ∩ c_utils 依赖 ∩ 未补 libsec_shared` | 迁移收尾 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | N/A | 变更仅含 BUILD.gn，无 C++/头文件代码面；按 §8.2 计入 executed 并标 N/A，不计缺失 |
| logic-scanner | 通过（3 项 P2/P3） | 19 处依赖增补逐条正确；发现一致性/完整性缺口 3 项，无 P0/P1 |
| input-scanner | 未要求 | 路径非 services/ IPC·DB·文件密集区 C++ 代码面，不在 dimensions_required |
| api-scanner | 未要求 | 无 interfaces/kits、NAPI/ANI/C 绑定源码变更，不在 dimensions_required |

---

## 6. 关键发现详情

### [LOG-01] ability_runtime 共享库编译同一批 securec 源却未补 libsec_shared 依赖 (P2, scanner=logic-scanner)

- **位置**：`frameworks/c/ability_runtime/BUILD.gn:56`（sources 列表，含 9 个 securec 源）、`frameworks/c/ability_runtime/BUILD.gn:93`（external_deps，含 `c_utils:utils` 但无 `bounds_checking_function:libsec_shared`）
- **触发路径**：`c_utils` 由 `public_external_deps` 改 `external_deps` 依赖 libsec_shared → 间接传递链断裂 → `ability_runtime.so` 编译的 `mo_dispatcher_type.cpp`/`mo_dispatcher_complex_type_manager.cpp`/`mo_dispatcher_metadata_manager.cpp`/`mo_dispatcher_param_codec.cpp`/`c_modular_object_utils.cpp`/`modular_object_dispatcher.cpp`/`native_ability_wrapper.cpp`/`start_options.cpp`/`start_options_impl.cpp` 引用 securec 符号 → 当其他仍间接提供 libsec_shared 的 dep（如 `ipc:ipc_core`/`samgr:samgr_proxy`）也完成迁移后 → 链接失败
- **影响**：生产 .so 当前可经其他 dep 间接解析符号而链接通过，但属“待触发的未来断裂”；且与同源 unittest target（`mo_dispatcher_*_test` 已补）处理不一致
- **证据**：`git show HEAD` 未触及 `frameworks/c/ability_runtime/BUILD.gn`；grep 命中 9 个源 `#include "securec.h"`；该 BUILD.gn external_deps（93-109 行）无 `bounds_checking_function`；对比 `test/unittest/mo_dispatcher_complex_type_test/BUILD.gn:42` 等已补
- **建议**：在 `ohos_shared_library("ability_runtime")` 的 external_deps 显式加 `"bounds_checking_function:libsec_shared",`（置于 `c_utils:utils` 前，与本提交统一风格）

### [LOG-02] dataobsms 编译 securec 源未补 libsec_shared 依赖 (P3, scanner=logic-scanner)

- **位置**：`services/dataobsmgr/BUILD.gn:42`（`sources = dataobsms_files`）、`services/dataobsmgr/BUILD.gn:56`（external_deps，含 `c_utils:utils` 无 libsec_shared）、`services/dataobsmgr/dataobsms.gni:17`（含 `dataobs_mgr_changeinfo.cpp`）
- **触发路径**：同 LOG-01 机制；`dataobs_mgr_changeinfo.cpp:19` 与 `dataobs_mgr_service.cpp:39` 均 `#include "securec.h"`
- **影响**：与已补的姊妹 target `interfaces/inner_api/dataobs_manager:dataobs_manager`（同源 `dataobs_mgr_changeinfo.cpp`）处理不一致；当前经其他 dep 间接解析，未即时断裂
- **证据**：grep 命中 `services/dataobsmgr/src/dataobs_mgr_changeinfo.cpp:19`、`services/dataobsmgr/src/dataobs_mgr_service.cpp:39`；`git show HEAD` 仅改 `interfaces/inner_api/dataobs_manager`，未改 `services/dataobsmgr:dataobsms`
- **建议**：迁移收尾时对 `dataobsms`/`dataobsms_static` 显式补齐 `bounds_checking_function:libsec_shared`

### [LOG-03] 全量迁移收尾交叉核对 (P3, scanner=logic-scanner)

- **位置**：跨仓（`services/appmgr/`、`frameworks/native/appkit/`、`frameworks/native/ability/native/` 等）
- **触发路径**：`services/appmgr/src/app_spawn_client.cpp`/`app_spawn_msg_wrapper.cpp`/`app_mgr_service_inner.cpp`、`frameworks/native/appkit/.../cj_ability_stage*.cpp`、`frameworks/native/ability/native/.../data_uri_utils.cpp`/`native_args_child_process.cpp`/`cj_insight_intent_executor_impl_object.cpp`/`cj_form_extension_object.cpp` 均 include securec.h，对应 target 多依赖 c_utils
- **影响**：本提交按外部提供的 .o 清单精确补齐 19 个 target，未覆盖仓内其余同类目标；存在与 LOG-01/02 同型的潜在未来断裂
- **证据**：全仓 grep `#include.*(securec\.h|securectype\.h)` 命中 100+ 文件，含上述 appmgr/appkit 路径
- **建议**：c_utils 迁移收尾阶段执行 `securec include ∩ c_utils 依赖 ∩ 未补 libsec_shared` 全量核对，统一显式补齐
