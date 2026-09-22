# Refute 审查记录

## 审查概要

| 指标 | 数值 |
|------|------|
| 审查范围 | security: 3 条, logic: 6 条 (含 2 条正面确认) |
| P0/P1 全审 | 4 条 (P0×1 + P1×1→降级) |
| P2 抽样审 | 3 条 |
| 维持 | 2 条 |
| 降级 | 2 条 |
| 推翻 | 0 条 |
| 合并 | 3 组 → 3 条 |
| **进入最终报告** | **5 条** (P0: 1, P1: 0, P2: 2, P3: 2) |

## 逐条审查

### ✅ 维持 — P0-001 (SEC-01 + LOG-01 + LOG-02 合并)

**发现摘要**：`ability_context_impl.cpp:1244` 引用 `uiWindow_`，但该成员不存在于 `AbilityContextImpl` 类继承链中，导致 `SUPPORT_SCREEN` 定义时编译失败。

**审查结论**：
- **第一层（触发路径证伪）**：`uiWindow_` 在 `AbilityContextImpl`(`ability_context_impl.h`)、`AbilityContext`(`ability_context.h`)、`Context`(`context.h`)、`ContextImpl`(`context_impl.h`) 中均未声明。全库 grep 确认 `uiWindow_` 仅存在于 `JsUIExtensionContentSession`、`CjUIExtensionContentSession`、`JsFillRequestCallback`、`JsSaveRequestCallback`。代码位于 `#ifdef SUPPORT_SCREEN`（line 1101-1382），设备构建 `ability_runtime_graphics=true` 时编译。**不可推翻——确认可达。**
- **第二层（影响夸大证伪）**：编译失败阻断整个 `ability_context_impl.cpp` 翻译单元，影响所有使用 `AbilityContextImpl` 的下游。**维持。**
- **第三层（跨 scanner 根因去重）**：SEC-01（security）、LOG-01（logic）、LOG-02（数据流不一致）根因同一：误用 `uiWindow_` 替代已 null-checked 的 `uiContent`。修复方案同一：恢复使用 `uiContent`。**合并为一条。**

**无变化。P0 维持。**

---

### ⬇️ 降级 — P3-001 (SEC-02 + LOG-03 合并): P1 → P3

**发现摘要**：`onAbilityErrorCode` 条件从 `phase != FOREGROUND && errorCode == 0` 改为 `phase != FOREGROUND`，导致非 FOREGROUND 相位的错误被静默丢弃，`onRelease` 可能以 `OnAbilityByTypeResult(0)` 报告假成功。

**审查结论**：
- **第一层（触发路径证伪）**：`UIExtensionOperationPhase` 枚举仅含 `FOREGROUND` 一个值（`modal_ui_extension_config.h:51-53`）。Ace 框架唯一调用点 `session_wrapper_impl.cpp:194` 硬编码 `UIExtensionOperationPhase::FOREGROUND`。**非 FOREGROUND 相位在当前代码中不可达。** 条件变更为 no-op——对 FOREGROUND 输入，`phase != FOREGROUND` 恒为 false，与旧条件 `false && ...` 结果相同。
- **第二层（影响夸大证伪）**：假成功场景要求 Ace 框架发送非 FOREGROUND 相位，但当前无此调用路径。理论风险仅在未来 Ace 框架扩展枚举值时才可触发。
- **第三层**：SEC-02 和 LOG-03 为同一根因、同一修复方案。**合并为一条。**

**降级理由**：触发路径确认不可达（上游硬编码 FOREGROUND），从 P1 降为 P3（理论风险/观察项）。

---

### ⬇️ 降级 — P3-002 (LOG-04): P2 → P3

**发现摘要**：`onAbilityErrorCode` 早返回路径无 DFX 日志，非 FOREGROUND 非零错误码被静默吞掉。

**审查结论**：
- **第一层**：同 P3-001——非 FOREGROUND 相位不可达，早返回路径不会执行。
- **第二层**：无可观测性影响因为路径不可达。

**降级理由**：依赖 P3-001 同一前提（非 FOREGROUND 不可达），从 P2 降为 P3。

---

### ✅ 维持 — P2-001 (SEC-03 + LOG-06 合并)

**发现摘要**：`ui_extension_callback.h:33` 将 `OnAbilityByTypeResult` 从纯虚函数改为虚函数默认空实现 `{}`，允许子类不 override。未 override 的子类会导致 Promise 永不 resolve/reject。

**审查结论**：
- **第一层**：当前所有子类（`JsUIExtensionCallback`、`EtsUIExtensionCallback`）均 override 了此方法，当前安全。触发需未来新增子类遗漏 override。
- **第二层**：影响为 Promise 挂死，严重但需未来代码触发。
- **第三层**：SEC-03 和 LOG-06 为同一根因。**合并为一条。**

**维持 P2。** 当前安全但 API 契约弱化，建议跟进。

---

### ✅ 维持 — P2-002 (LOG-08)

**发现摘要**：`StartWithDefaultQos` 中 `env_ = env` 位于 `execute_` 检查之前（line 242 在 243 之前），而 `Start`/`StartHighQos`/`StartLowQos` 中 `env_ = env` 位于 `execute_` 检查之后。SendNapiEvent 路径行为不一致。

**审查结论**：
- **第一层**：SendNapiEvent 路径中 `work_` 为 null，析构函数 `if (work_ && env_)` 为 false，`env_` 是否设置无实际行为差异。
- **第二层**：无可观测功能影响，仅为代码一致性/可维护性问题。

**维持 P2。** 不阻塞但建议统一。

---

## refute 耗时与统计

- 审查耗时：~3 轮工具调用
- 调用链追踪：6 次（uiWindow_ 类继承链 4 次 + Ace 框架调用链 2 次）
- 代码片段二次验证：8 次
