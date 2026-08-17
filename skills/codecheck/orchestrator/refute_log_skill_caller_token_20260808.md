# Refute 审查记录 — commit 671406f2（skillCallerTokenId）— Round 2

## 审查概要
| 指标 | 数值 |
|------|------|
| 审查范围 | security: 1 条, logic: 2 条, input: 1 条（跨 scanner 去重后 3 条） |
| P0/P1 全审 | 1 条 |
| P2/P3 抽样审 | 2 条 |
| 维持 | 3 条 |
| 降级 | 1 条（F1: P1 → P2） |
| 推翻 | 0 条 |
| 合并 | 0 组 |
| **进入最终报告** | **2 条计分** (P0: 0, P1: 0, P2: 2) + **1 条不计分观察项** (F3, P3) |

## 逐条审查

### ⬇️ 降级 — F1（P1 → P2, logic+input）：新增 `else { return ERR_INVALID_VALUE; }` 为不可达死代码，fix 无效
**发现摘要**：`skillCallerTokenId` 全链路为 `uint32_t`，`< 0` 分支不可达；`> 0` 与旧 `!= 0` 对 uint32 等价，本次修改功能等价于原代码。
**审查结论**：降级 P1 → P2。
**降级理由（回应"IPC 注入负数→直接返回，增加稳定性"的反驳）**：
- 反驳方意图成立：对意外输入做防御性早退是合理的健壮性写法。
- 但该守卫在**当前类型下无法被触发**：`skillCallerTokenId` 端到端为 `uint32_t`（签名 `ability_manager_service.h:1448/2242/2245`，字段 `ability_request.h:95`、`skill_execute_param.h:32`，IPC `ReadUint32`/`WriteUint32` `ability_manager_stub.cpp:5891`），uint32 恒非负，`else` 分支（需 `< 0`）不可达。
- IPC 注入的"-1"实际为线路上 `0xFFFFFFFF`，`ReadUint32` 读回 `4294967295 > 0`，**进入第一个分支而非 `else`** → 流入 `UpdateCallerInfoByHapTokenId`（`update_caller_info_util.cpp:126-139`）→ `GetHapTokenInfo(0xFFFFFFFF)` 失败仅打日志 return，Want 无 caller 身份。即"拒绝非法注入"的意图未达成。
- 无行为回归：改动对可达路径功能等价，属"fix 无效 + 死分支可能误导复核"，非活缺陷/权限问题，故按 §7 归一化为 P2（逻辑缺陷/健壮性）。

### ✅ 维持 — F2（P2, logic）：fresh-start 路径未传播 skillCallerTokenId，两分支 caller 身份不一致
**发现摘要**：`StartAbilityByCallWithSkill` 中目标未启动时（`ability_manager_service.cpp:15544`）调用 `StartAbilityByCall(want, connect, callerToken /* nullptr，见 :15490 */, oriValidUserId)`，不带 skill 调用者 token，也不使用 `abilityRequest.want`。
**审查结论**：维持 P2。
**证据**：已启动分支（:15528-15541）对 `abilityRequest.want` 执行 `UpdateCallerInfoByHapTokenId/UpdateCallerInfo`；未启动分支（:15544）直接用原始 `want` 走 `StartAbilityByCall`，`skillCallerTokenId` 被丢弃。同一 skill 请求两分支 caller 身份语义不同。前置提交遗留，本提交未覆盖。
无变化。

### ✅ 维持 — F3（P3 观察项·不计分, security/input 上下文）：`ExecuteInAppSkillWithTokenId` 的 callerTokenId 为 IPC 直供外部输入
**发现摘要**：IPC 方法 `EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID` 的 `request.callerTokenId` 直接读自 wire（`ability_manager_stub.cpp:5891`），`CheckSkillPermission`（`skill_execute_manager.cpp:90-116`）要求 `IsSACall()` 才放行。
**审查结论**：维持 P3，作为**不计分观察项**处理（不纳入 `findings_total` 与评分）。
**证据**：stub 分发（`ability_manager_stub.cpp:1043-1044`）无显式权限校验；服务端 `CheckSkillPermission` 第 98 行 `if (!permVerif->IsSACall()) return ERR_NOT_SYSTEM_APP;` 将第三方 app 挡在外面，实际可达调用方限于 SA（可信），当前无实际风险面。
无变化。

## refute 耗时与统计
- 审查耗时：~15 秒
- 调用链追踪：3 次（类型链 uint32 全链路；IPC 暴露面；注入值 0xFFFFFFFF 的读取路径）
- 代码片段二次验证：4 次（update_caller_info_util.cpp、skill_execute_manager.cpp、ability_manager_stub.cpp、ability_manager_proxy.cpp）
