# 代码检视报告 — startAbilityByType 异步回调重构（Round 3 / 最终版）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。本报告为 Round 3 最终版。Refute 记录见 `refute_log_startAbilityByType_20260920_round3.md`。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "startAbilityByType_异步回调重构"
  round: 3
  commit_id: "36f0106d8d8edf9f4996155e5f7db450700714c7"
  change_id: "I483d4ae39dcadd3d226996cf34d06fdc6595af86"
  report_id: "I483d4ae39dcadd3d226996cf34d06fdc6595af86-R3"
  date: "2026-09-20"
  gate_decision: "approve"
  risk_level: "low"
  score: 92
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner", "api-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner", "api-scanner"]
  findings_total: 4
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 4}
  gate_blockers: []
  must_fix: []
  followups: ["CC-14", "CC-17", "CC-18", "CC-19"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **92/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P3） | 4 项 |

**一句话结论**：Round 1 全部 P0/P1 已解决，Round 2/3 共修复 12 项 P2，剩余 P2（CC-11/15）及 P3（CC-20/22）经用户裁决移除不计扣分；当前仅剩 4 项 P3 观察跟进项，无任何阻塞项，score 92 ≥ 90 达到 approve 门槛，可上库。

### 三轮改善对比

| 指标 | R1 (2fd02d65) | R2 (36f0106d8d) | R3 (含 P2 修复) | R1→R3 |
|---|---|---|---|---|
| score | 0 | 48 | 92 | +92 |
| gate_decision | block | conditional | **approve** | 升级 |
| risk_level | high | medium | **low** | 降级 |
| P0 | 1 | 0 | 0 | −1 |
| P1 | 4 | 0 | 0 | −4 |
| P2 | 10 | 8 | 0 | −10 |
| P3 | 7 | 6 | 4 | −3 |

### 已解决项汇总（19 项）

| 原 ID | 原等级 | 解决方式 | 轮次 |
|---|---|---|---|
| CC-01 | P0 | 已修复：补 OnAbilityByTypeResult 三路径 + RejectAsyncResult 参数校验单测 | R2 |
| CC-02 | P1 | 已修复：5 处 `#ifdef SUPPORT_SCREEN` 补 `#else` 回退 | R2 |
| CC-03 | P1 | 已修复：OnError 补 reject Held Promise（NAPI+ANI） | R2 |
| CC-04 | P1 | 用户整体裁决移除，不计扣分 | R2 |
| CC-05 | P1 | 已修复：补 RejectAsyncResult Promise 路径单测 | R2 |
| CC-06 | P2 | 已修复：5 处 onRelease lambda 加 errorFired CAS 兜底 resolve(0) | R3 |
| CC-07 | P2 | 已修复：5 处 phase 过滤放宽（非 FOREGROUND 错误也处理） | R3 |
| CC-08 | P2 | 已修复：SetCompletionCallback 返回 bool + 3 处调用方检查 | R3 |
| CC-09 | P2 | 已修复：4 处 .ets taskpool.execute 链 .then 错误处理 | R3 |
| CC-10 | P2 | 已修复：targetType 读取后加控制字符白名单校验 | R3 |
| CC-11 | P2 | 用户裁决移除，无需修改 | R3 |
| CC-12 | P2 | 已修复：UIServiceExtensionContext 添加 callback/Promise 双模式 | R3 |
| CC-13 | P2 | 已修复：RejectAsyncResult_002 用 AssertCompletionCallback 校验参数 | R2 |
| CC-15 | P2 | 用户裁决移除，无需修改 | R3 |
| CC-20 | P3 | 用户裁决移除，无需修改 | R3 |
| CC-21 | P3 | 已修复：ability_business_error.cpp 合并冲突标记已清理 | R2 |
| CC-22 | P3 | 用户裁决移除，无需修改 | R3 |

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议跟进（P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| CC-14 | P3 | ClearAsyncResult 死代码（全库无调用点）+ 零测试 | 在 OnError/OnRelease 中调用清理 pending 状态，或删除 | 跟进 |
| CC-17 | P3 | SetJsCallbackObject 未检查 napi_create_reference 返回值（pre-existing，G04/G14 同类 15+ 处） | 封装统一 wrapper，全库整改 | 跟进 |
| CC-18 | P3 | HasParameter vs !targetType.empty() 语义变更 | 确认为有意修复则无需改动 | 跟进 |
| CC-19 | P3 | HiSysEvent 失败路径写入调用方可控 moduleName（已充分校验） | 可选加长度截断 | 跟进 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | P3×1 | 无 P0/P1/P2 缺陷；UAF/竞态经 shared_from_this + atomic CAS + NapiAsyncTask 串行化保护；CC-03/06/07 修复后异步回调挂死风险已消除；残余 P3：pre-existing napi_create_reference 未检查（CC-17） |
| logic-scanner | P3×1 | CC-02/03/06/07/09 修复后异步模型健壮性显著提升；残余 P3：HasParameter 语义变更（CC-18） |
| input-scanner | P3×1 | 服务端 targetType→bms→want 链路经 pickerMap 白名单 + extensionInfos.empty() + CC-10 控制字符白名单三重校验；残余 P3：HiSysEvent moduleName 已充分校验（CC-19） |
| api-scanner | P3×1 | CC-01/08/12/13 修复后测试覆盖与双模式支持完善；CC-11/15/20/22 用户裁决移除；残余 P3：ClearAsyncResult 死代码（CC-14） |

---

## 6. 关键发现详情

### [CC-14] ClearAsyncResult 死代码 + 零测试 (P3, scanner=security+logic+api)

- **位置**：`frameworks/native/ability/native/js_ui_extension_callback.cpp:140-146`；`interfaces/kits/native/ability/native/js_ui_extension_callback.h:35`
- **触发路径**：`ClearAsyncResult()` 定义为将 `deferred_` 置空并 `FreeNativeReference(completionCallback_)`，但全库 grep 无任何调用点，也无单测。
- **影响**：维护性问题；CC-06 修复后 onRelease 兜底 resolve 部分缓解了 pending 状态未清理的问题。
- **建议**：在 OnError/OnRelease 中调用 ClearAsyncResult 清理 pending 状态，或删除此方法；补幂等清零用例。

### [CC-17] SetJsCallbackObject 未检查 napi_create_reference 返回值 (P3, scanner=security)

- **位置**：`frameworks/native/ability/native/js_ui_extension_callback.cpp:110`
- **触发路径**：`napi_create_reference(env_, jsCallbackObject, 1, &ref)` 未检查返回值，若失败 `jsCallbackObject_` 为空指针。本 commit 同文件新增的 `SetCompletionCallback`（CC-08 修复后）已正确检查返回值，形成同文件内不一致。
- **影响**：pre-existing 问题，触发需 napi_create_reference 失败（OOM 极端条件）。
- **建议**：封装统一 wrapper，全库 15+ 处同类写法一并整改。

### [CC-18] HasParameter vs !targetType.empty() 语义变更 (P3, scanner=logic)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:4924`
- **触发路径**：条件从 `!targetType.empty()` 改为 `HasParameter(UIEXTENSION_TARGET_TYPE_KEY)`。参数存在但值为空字符串时，旧代码跳过此分支，新代码进入分支。
- **影响**：行为变更，可能是有意修复（空值应报错）。
- **建议**：确认为有意行为变更则无需改动。

### [CC-19] HiSysEvent 失败路径写入 moduleName (P3, scanner=input)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:5034-5038`
- **触发路径**：新增失败路径 `BuildEventInfo` 提取 `want.GetModuleNameRef()` 写入 HiSysEvent。
- **影响**：MODULE_NAME 为结构化 STRING 字段，非 format string，已充分校验。
- **建议**：可选加长度截断。
