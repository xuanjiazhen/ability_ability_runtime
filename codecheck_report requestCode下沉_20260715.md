# 代码检视报告 — requestCode下沉AbilityContextImpl

> **提交**: `f1d12b5c36` — requestCode下沉AbilityContextImpl
> **范围**: `frameworks/native/ability/ability_runtime/` + `interfaces/kits/native/ability/ability_runtime/` + `test/`
> **日期**: 2026-07-15
> **检视维度**: deep-scan (high-impact-bug-audit → logic_analyzer → security_review)
> **变更文件**: 6 个（+176 / -23 行）

---

## 1. 总览

| 维度 | 执行的 skill | 发现数 |
|------|-------------|--------|
| 高影响缺陷 | high-impact-bug-audit | 5 |
| 逻辑影响 | logic_analyzer | 7 |
| 安全审查 | security_review | 5 |
| **跨维度合并去重后** | — | **8** |

| 优先级 | 数量 |
|--------|------|
| P0（致命） | 1 |
| P1（严重） | 3 |
| P2（一般） | 3 |
| P3/提示 | 1 |

> api-audit 未执行：本次变更为**内部实现重构**（requestCode 生成逻辑下沉），`interfaces/kits/` 中新增的虚方法在 `AbilityContext` 基类上是带默认实现的非纯虚方法，不属于公开 API 签名变更，不触发 api-audit。

---

## 2. 高优先级发现（P0/P1，跨维度去重后）

### 2.1 [P0] `resultCallbacks_` 无锁保护 — 数据竞争可致崩溃

| 字段 | 值 |
|------|-----|
| **维度来源** | high-impact-bug-audit · logic_analyzer · security_review |
| **位置** | `ability_context_impl.cpp:276,289,302,322,476,488,1040,1046,1052` |
| **文件行** | `ability_context_impl.h:439`（声明） |

**问题描述**：`resultCallbacks_`（`std::map<int, RuntimeTask>`，每实例拥有）在以下路径中被无锁并发访问：

- **写入者**：`StartAbilityForResult` 4 个重载、`RegisterResultCallback`、`InsertResultCallbackTask`、`OpenAtomicService`
- **读取+删除**：`OnAbilityResultInner`（line 476-493）
- **删除**：`RemoveResultCallbackTask`（line 1046）

`requestCodeMutex_` 是 **static** 类成员，仅保护 `curRequestCode_` 计数器，不保护 `resultCallbacks_`。原 `JsAbilityContext` 中也存在同样的无锁插入（`InsertResultCallbackTask`），但本次提交新增了 `RegisterResultCallback` 这条新的写入路径且同样无锁，扩大了竞争窗口。

**触发路径**：NAPI 回调线程池中的多个并发 JS Promise 同时调用 `StartAbilityForResult(want, task)`，或 `OnAbilityResult` 从 AMS IPC 回调线程触发时与写入者并发。

**影响**：`std::map` 并发修改导致迭代器失效、UAF、堆损坏 → **进程崩溃**。对比 `UIExtensionContext`（`ui_extension_context.cpp:480`）已正确使用 `std::lock_guard<std::mutex>` 保护 `resultCallbacks_`。

**修复建议**：
```cpp
// ability_context_impl.h 新增成员：
std::mutex resultCallbacksMutex_;

// 所有 resultCallbacks_ 访问路径加锁：
std::lock_guard<std::mutex> lock(resultCallbacksMutex_);
resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
```

---

### 2.2 [P1] 基类虚方法默认返回 `ERR_OK` — 静默吞没未实现的子类

| 字段 | 值 |
|------|-----|
| **维度来源** | high-impact-bug-audit · logic_analyzer · security_review |
| **位置** | `ability_context.h:163-213, 521-525` |

**问题描述**：新增的 5 个不带 `requestCode` 的虚方法 + `RegisterResultCallback` 在 `AbilityContext` 基类中提供默认实现，返回 `ERR_OK`（或 `0`）。而原有的带 `requestCode` 的方法是**纯虚**（`= 0`）。若任何子类（含测试 mock、FA 模型 Context、第三方扩展）未 override 这些新方法：

1. 调用方收到 `ERR_OK`（成功），但**没有启动任何 Ability**
2. `RuntimeTask` 被 move 进方法后丢失，对应的 JS Promise **永远不会 resolve/reject** → 永久挂起
3. `RegisterResultCallback` 默认返回 `0`，与合法 requestCode `0` 语义冲突（after wrap-around）

**对比**：同文件中 `InsertResultCallbackTask` 仍保持纯虚（`= 0`），风格不一致。

**修复建议**：将默认返回改为 `AAFwk::ERR_NOT_SUPPORTED`（或负数错误码），与 `InsertResultCallbackTask` 保持一致的纯虚风格；`RegisterResultCallback` 默认返回 `-1`。

---

### 2.3 [P1] requestCode 溢出回绕至 0 后，`std::map::insert` 静默丢弃新回调

| 字段 | 值 |
|------|-----|
| **维度来源** | high-impact-bug-audit · logic_analyzer |
| **位置** | `ability_context_impl.cpp:338-343`（GenerateRequestCode）、`1051`（RegisterResultCallback） |

**问题描述**：`GenerateRequestCode` 在 `curRequestCode_ == INT_MAX` 时重置为 `0`。此后 requestCode 从 `0, 1, 2, ...` 重新递增。当 requestCode 与 `resultCallbacks_` 中尚未清理的旧条目键重复时，`std::map::insert` **不覆盖已有键**，新回调被静默丢弃。调用方收到 requestCode 并传给 AMS，但本地回调映射失败 → **Ability 结果永远无法回调到调用方**。

**触发条件**：进程生命周期内累计约 21.5 亿次 `GenerateRequestCode` 调用（长期运行的 kiosk 应用可达），且存在未清理的旧 requestCode 条目。

**修复建议**：
```cpp
int32_t AbilityContextImpl::GenerateRequestCode()
{
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ >= INT_MAX) ? 1 : (curRequestCode_ + 1);
    return curRequestCode_;
}
// 跳过 requestCode 0，避免与默认值/旧条目冲突
// 或在 insert 前检查键是否已存在并处理
```

---

### 2.4 [P1] `EtsAbilityContext` 仍使用独立 requestCode 计数器 — 与 `AbilityContextImpl` 计数器碰撞风险

| 字段 | 值 |
|------|-----|
| **维度来源** | logic_analyzer |
| **位置** | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:915-921` |

**问题描述**：本次提交将 NAPI 路径（`JsAbilityContext`）的 requestCode 生成统一到 `AbilityContextImpl::GenerateRequestCode()`（static 计数器），但 **ETS 路径**（`EtsAbilityContext`）仍保留自己的 `static int32_t curRequestCode_`（函数级 static 变量），生成 requestCode 后传入 `AbilityContextImpl::StartAbilityForResult(want, requestCode, task)`。

两个独立计数器向同一个 `resultCallbacks_` map 写入，回绕后可能产生相同的 requestCode → 回调静默丢弃（同 2.3）。

**修复建议**：将 ETS 路径也迁移到无 requestCode 的重载，统一使用 `AbilityContextImpl` 的计数器。

---

## 3. 分维度明细

### 3.1 high-impact-bug-audit（经 deep-scan）

| # | 位置 | 风险等级 | 问题 |
|---|------|---------|------|
| L1-1 | `ability_context_impl.cpp:1051` | 致命 | `resultCallbacks_.insert()` 无锁保护（→ 合并至 §2.1） |
| L1-2 | `ability_context.h:163-210` | 严重 | 基类默认 `ERR_OK` 静默吞没（→ 合并至 §2.2） |
| L1-3 | `ability_context_impl.cpp:339` | 严重 | INT_MAX 回绕后 `insert` 不覆盖旧键（→ 合并至 §2.3） |
| L1-4 | `ability_context_impl_test.cpp` | 严重 | 测试 `NoRequestCode_0200` 断言 `!= ERR_OK` 与实现矛盾（→ §3.2 L2-7） |
| L1-5 | `ability_context.h:519` | 一般 | `InsertResultCallbackTask` 未被废弃，与 `RegisterResultCallback` 并存致requestCode碰撞窗口 |

### 3.2 logic_analyzer（经 deep-scan）

| # | 位置 | 风险等级 | 问题类型 | 问题 |
|---|------|---------|---------|------|
| L2-1 | `ability_context.h:163-184` | P1 | 控制流 | 基类默认 `ERR_OK` → 合并至 §2.2 |
| L2-2 | `ability_context.h:521` | P1 | 数据流 | `RegisterResultCallback` 默认返回 `0` 与合法 requestCode 歧义 |
| L2-3 | `ability_context_impl.cpp:341` | P3 | 边界条件 | 回绕至 `0` 后 `insert` 静默失败 → 合并至 §2.3 |
| L2-4 | `ets_ability_context.cpp:915` | P2 | 并发控制 | ETS 独立计数器与 Impl 计数器碰撞 → 合并至 §2.4 |
| L2-5 | `js_ability_context.cpp:1030` | P3 | 业务规则 | `CreateOpenLinkTask` 行为改善（非 bug）— 旧代码在 context 失效时浪费 requestCode，新代码不浪费 |
| L2-6 | `ability_context_impl.cpp` 多处 | P2 | 并发控制 | `resultCallbacks_` 无锁读写 → 合并至 §2.1 |
| L2-7 | `ability_context_impl_test.cpp:731` | P2 | 错误处理 | `NoRequestCode_0200` 断言 `!= ERR_OK` 与基类默认 `ERR_OK` 矛盾 |

### 3.3 security_review（经 deep-scan）

| # | 位置 | 严重等级 | 问题类别 | 问题 |
|---|------|---------|---------|------|
| S-1 | `ability_context_impl.cpp` 多处 | High | 并发安全 | `resultCallbacks_` 数据竞争 → 合并至 §2.1 |
| S-2 | `ability_context_impl.cpp:341` | Medium | 注入/输入验证 | requestCode 回绕可预测 → 合并至 §2.3 |
| S-3 | `ability_context.h:163-213` | High | 权限绕过/状态混淆 | 基类默认 `ERR_OK` → 合并至 §2.2 |
| S-4 | `ability_context_impl.cpp:1051` | Medium | 内存安全/输入验证 | `RuntimeTask` 为 null 时 `std::bad_function_call` 崩溃 |
| S-5 | `ability_context_impl.h:460` | Low | 信息泄漏 | 静态 requestCode 计数器跨实例持久化，日志 `%{public}d` 泄漏 requestCode |

---

## 4. 待跟进（需进一步确认）

| # | 问题 | 待确认项 |
|---|------|---------|
| F-1 | `ability_context_impl.cpp:275` `TAG_LOGI` 以 `%{public}d` 打印 requestCode | 确认 requestCode 是否属于敏感信息，是否应改为 `%{private}d` |
| F-2 | `InsertResultCallbackTask` 与 `RegisterResultCallback` 并存 | 确认是否有外部模块（非 NAPI/ETS 路径）仍使用 `InsertResultCallbackTask` 手动传入 requestCode |
| F-3 | 测试 `NoRequestCode_0200` 期望 `!= ERR_OK` | 确认测试 fixture 中 `MockServiceAbilityManagerService` 的 `StartAbility(want, startOptions, ...)` 行为是否符合预期 |

---

## 5. 附录：各 skill 原始产出

| skill | 产出 |
|-------|------|
| high-impact-bug-audit | 5 findings (P0×1, P1×3, P2×1) |
| logic_analyzer | 7 findings (P1×2, P2×3, P3×2) |
| security_review | 5 findings (High×2, Medium×2, Low×1) |

---

*本报告由 AI 自动扫描生成（deep-scan 三层串联），所有结论需结合具体调用场景二次确认。引用的文件:行号基于 `f1d12b5c36` 版本。*
