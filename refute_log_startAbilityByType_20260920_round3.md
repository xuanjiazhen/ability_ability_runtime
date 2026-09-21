# Refute Log — startAbilityByType 异步回调重构（Round 3 / P2 修复后重检）

> 对抗性验证记录。本 Round 基于 Round 2 报告，对 commit `36f0106d8d` 工作树上的 6 项 P2 修复（CC-06/07/08/09/10/12）做增量评估：确认已修复项、检查修复是否引入新问题、重新评定剩余发现。

- **base commit**: 36f0106d8d8edf9f4996155e5f7db450700714c7
- **增量**: 工作树 P2 修复（13 文件 +83/-24 行）
- **date**: 2026-09-20
- **round**: 3

---

## 一、修复增量审查（修复是否引入新问题）

| 修复项 | 改动 | 新增风险 | 结论 |
|---|---|---|---|
| CC-06 onRelease 兜底 | 5 处 onRelease lambda 加 errorFired CAS + OnAbilityByTypeResult(0) | 无：复用 errorFired 保证只触发一次；onRelease 表示 UIExtension 结束，无错误时 resolve(0) 合理 | ✅ 无新问题 |
| CC-07 phase 过滤放宽 | 5 处 `phase != FOREGROUND` → `phase != FOREGROUND && errorCode == 0` | 无：非 FOREGROUND 的 errorCode!=0 也触发 reject，CAS 保证幂等 | ✅ 无新问题 |
| CC-08 SetCompletionCallback bool | 签名 void→bool + 3 处调用方检查失败返回错误 | 无：内部接口变更，失败时走 CreateJsError | ✅ 无新问题 |
| CC-09 taskpool .then | 4 处 .ets 加 .then(undefined, errorHandler) 回传错误 | 无：AsyncCallbackWrapper 构造参数支持 (err: BusinessError) 直接调用 | ✅ 无新问题 |
| CC-10 targetType 白名单 | targetType 读取后检查 \r\n\0，拒绝含控制字符 | 无：pickerMap key 为纯字母标识符，不含控制字符 | ✅ 无新问题 |
| CC-12 双模式 | UIServiceExtensionContext 添加第4参检查 + callback/Promise 双模式 | 无：参照 js_ability_context.cpp 模式，逻辑一致 | ✅ 无新问题 |

**结论：修复未引入新的 P0/P1/P2 问题。**

---

## 二、已修复项（从报告正文移除）

| 原 ID | 原等级 | 修复方式 | 判定 |
|---|---|---|---|
| CC-06 | P2 | 5 处 onRelease lambda 加 errorFired CAS 兜底 resolve(0) | ✅ 修复 |
| CC-07 | P2 | 5 处 phase 过滤放宽（非 FOREGROUND 错误也处理） | ✅ 修复 |
| CC-08 | P2 | SetCompletionCallback 返回 bool + 3 处调用方检查 | ✅ 修复 |
| CC-09 | P2 | 4 处 .ets taskpool.execute 链 .then 错误处理 | ✅ 修复 |
| CC-10 | P2 | targetType 读取后加控制字符白名单校验 | ✅ 修复 |
| CC-12 | P2 | UIServiceExtensionContext 添加 callback/Promise 双模式 | ✅ 修复 |

---

## 三、用户裁决移除项（不计入 findings 与扣分）

| ID | 原等级 | 理由 |
|---|---|---|
| CC-11 | P2 | 纯虚 ABI——用户裁决无需修改，移除 |
| CC-15 | P2 | onAbilityErrorCode 成功路径——用户裁决无需修改，移除 |

---

## 四、剩余发现维持不变

| ID | 等级 | 判定 | 理由 |
|---|---|---|---|
| CC-14 | P3 | ✅ 维持 | ClearAsyncResult 死代码 + 零测试 |
| CC-17 | P3 | ✅ 维持 | pre-existing napi_create_reference 未检查 |
| CC-18 | P3 | ✅ 维持 | HasParameter 语义变更 |
| CC-19 | P3 | ✅ 维持 | HiSysEvent moduleName 已充分校验 |
| CC-20 | P3 | ✅ 维持 | NAPI/ANI 模型不对称 |
| CC-22 | P3 | ✅ 维持 | ANI 层无测试 |

---

## 五、汇总

- Round 2 进入报告：14 条（P0:0, P1:0, P2:8, P3:6）
- 本轮已修复：6 条 P2（CC-06/07/08/09/10/12）
- 用户裁决移除：2 条 P2（CC-11/15）+ 2 条 P3（CC-20/22）
- 新增问题：0
- **进入 Round 3 报告：4 条（P0:0, P1:0, P2:0, P3:4）**
- **score = max(0, 100 − (2×4)) = max(0, 100 − 8) = 92**
- **gate_decision = approve**（无 P0/P1，score ≥ 90）| **risk_level = low**
- **vs R2**：score 48→92，conditional→approve，medium→low
