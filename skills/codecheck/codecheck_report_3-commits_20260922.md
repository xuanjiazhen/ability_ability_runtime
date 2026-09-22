# 代码检视报告 — 最近 3 笔提交（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "3-commits: 1ec03fc4 / 8707cf0dcf / 8e39c7db77"
  round: 1
  commit_id: "1ec03fc4ea9595ab8a87fc5b4755268b62fe7980"
  change_id: "I442549b0146a799e695e08e6099f50623a4e99de"
  report_id: "I442549b0146a799e695e08e6099f50623a4e99de-R1"
  date: "2026-09-22"
  gate_decision: "block"
  risk_level: "high"
  score: 56
  dimensions_required: ["security-scanner", "logic-scanner", "api-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "api-scanner"]
  findings_total: 5
  findings_by_severity: {P0: 1, P1: 0, P2: 2, P3: 2}
  gate_blockers: ["P0-001"]
  must_fix: []
  followups: ["P2-001", "P2-002", "P3-001", "P3-002"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **block** |
| 风险等级 | 🔴 high |
| 评分 | **56/100** |
| 阻塞项 | P0-001（`uiWindow_` 未声明导致编译失败） |
| 必须修复（P0/P1） | 1 项 |
| 建议跟进（P2/P3） | 4 项 |

**一句话结论**：Commit 8e39c7db77 在 `AbilityContextImpl::StartAbilityByType` 中误用 `uiWindow_`（该类无此成员），导致 `SUPPORT_SCREEN` 构建时编译失败，阻断上库；其余变更（泄漏修复、条件对齐、测试补充）本身正确但附带了 API 契约弱化和代码一致性小问题。

---

## 2. 扣分原因

> 共扣 **44 分**，由 1 个 P0（−30）、2 个 P2（−10）、2 个 P3（−4）组成。

| 扣分项 | 扣分数 | 问题 | 位置 |
|---|---|---|---|
| P0-001 | −30 | `uiWindow_` 未声明，编译失败 | `ability_context_impl.cpp:1244` |
| P2-001 | −5 | 纯虚→默认空实现，API 契约弱化 | `ui_extension_callback.h:33` |
| P2-002 | −5 | `env_` 赋值位置与同类方法不一致 | `js_runtime_utils.cpp:242` |
| P3-001 | −2 | 非 FOREGROUND 错误被丢弃（理论） | 3 文件 `onAbilityErrorCode` |
| P3-002 | −2 | 丢弃路径无 DFX 日志（理论） | 3 文件 `onAbilityErrorCode` |

**如果想快速提分**：优先修复 **P0-001（−30 分）**——将 `uiWindow_->GetUIContent()` 恢复为已 null-checked 的 `uiContent`，即可解除编译阻断，分数从 56 提升至 86。

---

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | Scanner | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|---|
| P0-001 | P0 | security + logic | `uiWindow_` 不存在于 `AbilityContextImpl` 继承链，编译失败 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1244` | `SUPPORT_SCREEN` 定义时（设备标准构建）→ 编译 `StartAbilityByType` → `uiWindow_->GetUIContent()` → 标识符未声明 → **编译错误** | 整个 `ability_context_impl.cpp` 翻译单元编译失败，阻断所有依赖 `AbilityContextImpl` 的下游 |

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| P2-001 | P2 | `OnAbilityByTypeResult` 从纯虚改为默认空实现，未来子类遗漏 override 时 Promise 永不 resolve | 恢复 `= 0` 或在接口注释中标注"生产子类必须 override" | 本轮或下一补档 |
| P2-002 | P2 | `StartWithDefaultQos` 中 `env_ = env` 位于 `execute_` 检查之前，与 `Start`/`StartHighQos`/`StartLowQos` 不一致 | 将 `env_ = env` 移至 `execute_` 检查之后，统一四个方法 | 下一补档 |
| P3-001 | P3 | 非 FOREGROUND 相位错误被静默丢弃（当前不可达：Ace 框架仅发送 FOREGROUND） | 若 Ace 未来扩展 `UIExtensionOperationPhase`，需补充非 FOREGROUND 错误处理 | 登记跟进 |
| P3-002 | P3 | 早返回路径无 DFX 日志（当前不可达） | 在早返回路径补充 `TAG_LOGD` 记录非零 errorCode | 登记跟进 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | 发现 3 条（合并后 2 条） | P0×1（编译失败）、P2×1（API 契约弱化）；`StartWithDefaultQos` 泄漏修复确认正确无新缺陷 |
| logic-scanner | 发现 6 条（合并后 3 条） | P0×1（与 security 同根因合并）、P2×1（env_ 位置）、P3×2（理论性条件变更影响） |
| api-scanner | N/A（手动检查） | `ui_extension_callback.h` 变更为二进制兼容（纯虚→虚函数默认实现），现有子类均 override，一致性无破坏；契约弱化见 P2-001 |

---

## 6. 关键发现详情

### [P0-001] `uiWindow_` 未声明导致编译失败 (P0, scanner=security+logic)

- **位置**：`frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1244`
- **触发路径**：设备标准构建（`ability_runtime_graphics = true`）→ `SUPPORT_SCREEN` 定义 → 编译 `#ifdef SUPPORT_SCREEN` 块（line 1101-1382）→ `StartAbilityByType` line 1244 `uiWindow_->GetUIContent()` → 标识符 `uiWindow_` 未声明 → **编译错误**
- **影响**：整个 `ability_context_impl.cpp` 翻译单元编译失败，阻断 `AbilityContextImpl` 及所有下游消费者。`ohos-sdk` 产品构建（`ability_runtime_graphics` 可能为 false）可能不受影响，但设备构建必然失败。
- **证据**：
  - `ability_context_impl.h:37`：`class AbilityContextImpl : public AbilityContext` — 私有成员（line 450-487）无 `uiWindow_`
  - `ability_context.h:54`：`class AbilityContext : public Context` — 无 `uiWindow_`
  - `context.h:42`：`class Context` — 无 `uiWindow_`
  - `context_impl.h` — 无 `uiWindow_`
  - 全库 grep `uiWindow_`（`.h` 文件）：仅 `JsUIExtensionContentSession`、`CjUIExtensionContentSession`、`JsFillRequestCallback`、`JsSaveRequestCallback` 有此成员
  - 原始代码 `uiContent->CreateModalUIExtension(...)` 使用 line 1223 的 `uiContent`（来自 `GetUIContent()`，已在 line 1224-1227 null-checked）
  - 修复代码改为 `uiWindow_->GetUIContent()`，疑似从 `EtsUIExtensionContentSession`（有 `uiWindow_` 成员）copy-paste
- **建议**：恢复使用已 null-checked 的 `uiContent`：
  ```cpp
  int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
  ```
  如确实需要 window-based UIContent，需先在 `AbilityContextImpl` 中添加并初始化 `uiWindow_` 成员。

### [P2-001] 纯虚函数改为默认空实现，API 契约弱化 (P2, scanner=security+logic)

- **位置**：`interfaces/kits/native/ability/native/ui_extension_callback.h:33`
- **触发路径**：未来新增 `UIExtensionCallback` 子类未 override `OnAbilityByTypeResult` → `onRelease` 调用 `OnAbilityByTypeResult(0)` → 空实现 no-op → Promise/deferred 永不 resolve/reject → **异步操作挂死**
- **影响**：当前所有子类（`JsUIExtensionCallback`、`EtsUIExtensionCallback`）均 override 此方法，当前安全。但 API 契约从"强制实现"弱化为"可选实现"，与同文件中仍为纯虚的 `OnError`/`OnResult` 不对称。
- **证据**：
  ```cpp
  virtual void OnError(int32_t number) = 0;                    // 仍为纯虚
  virtual void OnAbilityByTypeResult(int32_t errorCode) {}     // 改为默认空实现（不对称）
  virtual void OnResult(int32_t resultCode, const AAFwk::Want &want) = 0;  // 仍为纯虚
  ```
  `JsUIExtensionCallback::OnAbilityByTypeResult`（`js_ui_extension_callback.cpp:188-213`）负责 resolve/reject Promise。
- **建议**：恢复 `= 0`（纯虚），在需要 no-op 的特定子类中提供空 override。若保留默认实现，添加注释标注"生产子类必须 override 此方法以交付异步结果"。

### [P2-002] `env_` 赋值位置与同类方法不一致 (P2, scanner=logic)

- **位置**：`frameworks/native/runtime/js_runtime_utils.cpp:242` vs `:392`/`:410`/`:431`
- **触发路径**：无功能影响。在 `StartWithDefaultQos` 中 `env_ = env`（line 242）位于 `execute_ == nullptr` 检查（line 243）之前；而 `Start`（line 392）、`StartHighQos`（line 410）、`StartLowQos`（line 431）中 `env_ = env` 位于 `execute_` 检查之后。SendNapiEvent 路径（`execute_ == nullptr`）中 `StartWithDefaultQos` 设置 `env_` 而其他三个方法不设置。
- **影响**：无功能差异（SendNapiEvent 路径中 `work_` 为 null，析构函数 `if (work_ && env_)` 为 false）。但代码不一致可能误导维护者。
- **证据**：
  - `StartWithDefaultQos:242` — `env_ = env;` 在 `if (execute_ == nullptr)` 之前
  - `Start:392` — `env_ = env;` 在 `if (execute_ == nullptr)` 之后
  - `StartHighQos:410` — `env_ = env;` 在 `if (execute_ == nullptr)` 之后
  - `StartLowQos:430` — `env_ = env;` 在 `if (execute_ == nullptr)` 之后
- **建议**：统一四个方法中 `env_ = env` 的位置（移至 `execute_` 检查之后或之前，选其一）。

### [P3-001] 非 FOREGROUND 相位错误被静默丢弃（理论性） (P3, scanner=security+logic)

- **位置**：`js_ui_extension_content_session.cpp:997`、`ui_extension_context.cpp:816`、`ui_service_extension_context.cpp:126`
- **触发路径**：条件从 `phase != FOREGROUND && errorCode == 0` 改为 `phase != FOREGROUND`。若 Ace 框架发送非 FOREGROUND 相位 + 非零 errorCode → 回调返回早退 → `errorFired` 保持 false → 后续 `onRelease` → `OnAbilityByTypeResult(0)` → **假成功**
- **影响**：**当前不可达**——`UIExtensionOperationPhase` 枚举仅含 `FOREGROUND`（`modal_ui_extension_config.h:51-53`），Ace 框架唯一调用点 `session_wrapper_impl.cpp:194` 硬编码 `UIExtensionOperationPhase::FOREGROUND`。条件变更为 no-op。
- **证据**：`session_wrapper_impl.cpp:194`:
  ```cpp
  pattern->FireOnAbilityErrorCodeCallback(UIExtensionOperationPhase::FOREGROUND, abilityCode);
  ```
- **建议**：若 Ace 未来扩展 `UIExtensionOperationPhase` 枚举值，需重新评估非 FOREGROUND 错误的处理路径。当前登记为观察项。

### [P3-002] 丢弃路径无 DFX 日志（理论性） (P3, scanner=logic)

- **位置**：同 P3-001
- **触发路径**：`onAbilityErrorCode` 早返回路径（`phase != FOREGROUND`）无 `TAG_LOGD`/`TAG_LOGW` 日志，非零 errorCode 被静默吞掉。
- **影响**：**当前不可达**（同 P3-001，非 FOREGROUND 相位不发生）。若未来可达，丢失 DFX 可观测性。
- **建议**：在早返回路径补充 `TAG_LOGD` 或 `TAG_LOGW` 记录 errorCode，登记为观察项。
