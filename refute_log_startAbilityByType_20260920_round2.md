# Refute Log — startAbilityByType 异步回调重构（Round 2 / 修复后重检）

> 对抗性验证记录。本 Round 基于 Round 1 全量四维扫描结果，对 commit `36f0106d8d`（含 CC-01/CC-02/CC-03/CC-05 修复，不含 CC-04）做增量评估：确认已修复项、检查修复是否引入新问题、重新评定剩余发现等级。

- **commit**: 36f0106d8d8edf9f4996155e5f7db450700714c7（Round 1 commit 2fd02d65fc + 197 行修复）
- **date**: 2026-09-20
- **round**: 2

---

## 一、修复增量审查（修复是否引入新问题）

对 +197 行修复逐项审查，确认未引入新的 P0/P1 问题：

| 修复项 | 改动 | 新增风险 | 结论 |
|---|---|---|---|
| CC-02 #else 回退（5 处） | `#else` 设 `ERROR_CODE_INNER` → complete 走 RejectAsyncResult/AsyncCallback | 无：仅 SUPPORT_SCREEN 未定义时触发，行为从"挂死"变"reject"，正向 | ✅ 无新问题 |
| CC-03 NAPI OnError reject | complete lambda 中 CallJsError 后检查 deferred_/completionCallback_ 非空则 RejectAsyncResult | 无：与 OnAbilityByTypeResult 的 complete 回调均经 NapiAsyncTask::Schedule 在 JS 线程串行执行，RejectAsyncResult 先执行置空后 OnAbilityByTypeResult 检查 nullptr 跳过，无竞态 | ✅ 无新问题 |
| CC-03 ANI OnError reject | OnError 中检查 asyncCallback_ 非空则构造 error + AsyncCallback + 释放引用 | 无：与原 CallObjectMethod 同步执行同线程，asyncCallback_ 置空后 OnAbilityByTypeResult 的 InvokeAsyncCallback 检查 nullptr 跳过 | ✅ 无新问题 |
| CC-01/CC-05 测试（+168 行） | 新增 AssertCompletionCallback + 5 个用例 | 无：纯测试代码，不涉及生产路径 | ✅ 无新问题 |

**结论：修复未引入新的 P0/P1/P2 问题。**

---

## 二、已修复项（从报告正文移除，记录于此）

| 原 ID | 原等级 | scanner | 修复方式 | 判定 |
|---|---|---|---|---|
| CC-01 | P0 | api | 补 OnAbilityByTypeResult_001/002/003（成功/失败/Promise 三路径）+ RejectAsyncResult_002/003 | ✅ 修复 |
| CC-02 | P1 | logic+security+api | 5 处 `#ifdef SUPPORT_SCREEN` 补 `#else` 设 ERROR_CODE_INNER（3 处 execute lambda + 2 处 content_session） | ✅ 修复 |
| CC-03 | P1 | logic | NAPI OnError complete lambda 补 RejectAsyncResult + ANI OnError 补 asyncCallback_ 触发 | ✅ 修复 |
| CC-05 | P1 | api | 补 RejectAsyncResult_003（SetDeferred + Promise reject 幂等） | ✅ 修复 |
| CC-13 | P2 | api | 补 RejectAsyncResult_002 用 AssertCompletionCallback 校验 callCount==1 + errorCode==16000050 | ✅ 修复 |
| CC-21 | P3 | api | 合并冲突标记本 commit 已清理（R1 已确认） | ✅ 修复 |

---

## 三、剩余发现重新评定

### CC-06（原 P2）无超时兜底 — CC-03 修复后部分缓解

- **变更**：CC-03 修复后 OnError 路径会 reject Promise，覆盖"Ace 触发 onError 但不触发 onAbilityErrorCode"场景
- **残余风险**：Ace 层完全不回调（既不 onError 也不 onAbilityErrorCode）时 Promise 仍永久 pending
- **判定**：⬇️ 维持 P2，但触发面收窄（仅 Ace 完全无回调时）

### CC-14+CC-16 合并 — ClearAsyncResult 死代码 + 零测试

- **变更**：CC-01 修复补了 OnAbilityByTypeResult/RejectAspectRatio 测试，但 ClearAsyncResult 仍无测试且无调用点
- **判定**：🔀 合并为一条 P3（死代码导致测试缺失是合理关联，同根因）

### 其余发现维持不变

| ID | 原等级 | 判定 | 理由 |
|---|---|---|---|
| CC-04 | P1 | ❌ 移除 | 用户整体裁决移除，不计入 findings 与扣分 |
| CC-07 | P2 | ✅ 维持 P2 | phase 过滤未改 |
| CC-08 | P2 | ✅ 维持 P2 | napi_create_reference 失败未处理 |
| CC-09 | P2 | ✅ 维持 P2 | taskpool 无 .catch() 未补 |
| CC-10 | P2 | ✅ 维持 P2 | hilog CRLF 未过滤 |
| CC-11 | P2 | ✅ 维持 P2 | 纯虚 ABI 未改 |
| CC-12 | P2 | ✅ 维持 P2 | UIServiceExtensionContext 仅 Promise 未改 |
| CC-15 | P2 | ✅ 维持 P2 | onAbilityErrorCode 成功路径未验证 |
| CC-17 | P3 | ✅ 维持 P3 | pre-existing napi_create_reference 未检查 |
| CC-18 | P3 | ✅ 维持 P3 | HasParameter 语义变更未确认 |
| CC-19 | P3 | ✅ 维持 P3 | HiSysEvent moduleName 已充分校验 |
| CC-20 | P3 | ✅ 维持 P3 | NAPI/ANI 模型不对称 |
| CC-22 | P3 | ✅ 维持 P3 | ANI 层仍无测试 |

---

## 四、汇总

- Round 1 原始发现：22 条（P0:1, P1:4, P2:10, P3:7）
- 已修复：6 条（CC-01 P0, CC-02/03/05 P1×3, CC-13 P2, CC-21 P3）
- 用户裁决移除：1 条（CC-04 P1，不计入 findings 与扣分）
- 合并：1 组（CC-14+CC-16 → P3）
- 新增问题：0
- **进入 Round 2 报告：14 条（P0:0, P1:0, P2:8, P3:6）**
- **score = max(0, 100 − (5×8 + 2×6)) = max(0, 100 − 52) = 48**
- **gate_decision = conditional**（无 P0/P1，P2/P3 为建议跟进项，门禁复核人书面裁决后可放行）| **risk_level = medium**
- **vs R1**：score 0→48，block→conditional，high→medium，P0 1→0，P1 4→0

### 用户裁决（2026-09-20）

CC-04（ABILITY_SERVICE_NOT_CONNECTED 错误码-消息不一致）：用户整体裁决移除，不修改代码，不计入 findings 与扣分。后续若需修复，方案 A（在 `INNER_TO_JS_ERROR_CODE_MAP` 增加 1 行映射 `{ABILITY_SERVICE_NOT_CONNECTED, ERROR_CODE_SERVICE_BUSY}`）即可。
