# 代码检视报告 — commit 671406f2 skillCallerTokenId fix（Round 2 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "commit 671406f2 — services/abilitymgr skillCallerTokenId fix"
  round: 2
  commit_id: "671406f2be50e6ef8e80676a404efa85f44e728d"
  change_id: "N/A"
  report_id: "671406f2-R2"
  date: "2026-08-08"
  gate_decision: "approve"
  risk_level: "low"
  score: 90
  dimensions_required: [security-scanner, logic-scanner, input-scanner]
  dimensions_executed: [security-scanner, logic-scanner, input-scanner]
  findings_total: 2
  findings_by_severity: {P0: 0, P1: 0, P2: 2, P3: 0}
  gate_blockers: []
  must_fix: []
  followups: [F1, F2]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **90/100** |
| 阻塞项 | 无 P0 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 2 项 |

**一句话结论**：本次"fix: skillCallerTokenId"的新增 `else { return ERR_INVALID_VALUE; }` 分支因 `skillCallerTokenId` 全链路为 `uint32_t` 而**不可达**——IPC 注入的"-1"实为 `0xFFFFFFFF`，读回后 `> 0`，进的是第一个分支而非 `else`，因此"拒绝非法注入"意图未达成；若意图是防御性早退，该守卫不会触发但无回归。同时存在 fresh-start 路径不传播 skill 调用者身份的两分支不一致。两项均为 P2，可上库，登记跟进。

---

## 2. 扣分原因（仅 gate_decision=block 时呈现；conditional 时本节省略）

> gate_decision = conditional，本节按模板省略。

---

## 3. 必须立即处理（P0/P1）

**无。**　详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| F1 | P2 | 新增 `else { return ERR_INVALID_VALUE; }` 是不可达死代码：`skillCallerTokenId` 全链路 `uint32_t`（签名 :h:1448/2242/2245、字段 :ability_request.h:95、:skill_execute_param.h:32、IPC `ReadUint32` :ability_manager_stub.cpp:5891），`< 0` 不可能；IPC 注入的"-1"=`0xFFFFFFFF` 读回 `4294967295 > 0` 仍进 `UpdateCallerInfoByHapTokenId`（:update_caller_info_util.cpp:126-139）→ `GetHapTokenInfo` 失败仅打日志，Want 无 caller 身份。"拒绝非法注入"意图未达成 | 若目的是拒绝非法 tokenId：改为 `if (skillCallerTokenId == Security::AccessToken::INVALID_TOKENID)` 显式比较，或在 `> 0` 分支内先 `GetHapTokenInfo` 校验成功再继续；若仅防御性写法，删除死 `else` 保持 `!= 0` | 下一补档 |
| F2 | P2 | `StartAbilityByCallWithSkill` 目标未启动时（:15544）调 `StartAbilityByCall(want, connect, nullptr, oriValidUserId)`，`skillCallerTokenId` 未传播、callerToken 为 nullptr；已启动分支（:15528-15541）用 skillCallerTokenId 更新 caller → 同一请求两分支 caller 身份不一致 | fresh-start 路径也基于 skillCallerTokenId 更新 caller 信息，或文档化两路径身份语义差异 | 下一补档 |

> **不计分观察项**（`P3`，未计入 `findings_total`/评分）：`ExecuteInAppSkillWithTokenId`（stub :1043-1044 无显式权限校验）的 `request.callerTokenId` 直接读自 wire（:5891），服务端 `CheckSkillPermission` 依赖 `IsSACall()` 挡第三方（:skill_execute_manager.cpp:98）。当前可达调用方限于 SA（可信），无实际风险面；建议保持 `IsSACall` 门禁，若未来开放第三方调用需改为按实际 IPC 调用者 token 核验。

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 本提交未引入权限绕过/内存问题；skill 入口受 `IsSACall` 门禁（P3 观察项不计分，见 §4 注） |
| logic-scanner | 🟡 2 项 P2 | F1 类型误判导致死分支（fix 无效，无回归）；F2 两分支 caller 身份不一致 |
| input-scanner | 🟡 1 项 P2（与 logic 合并） | `skillCallerTokenId` 来源全为 `uint32_t`（`ReadUint32`/`GetCallingTokenID`），负值分支不可达；与 F1 同一根因合并 |

---

## 6. 关键发现详情

### [F1] 新增拒绝分支为不可达死代码，`fix: skillCallerTokenId` 未达成"拒绝非法注入"意图 (P2, scanner=logic+input)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:4861`、`:15535`
- **触发路径**：任何进入 `StartExtensionAbilityInner`（`if (!isStartAsCaller)` 分支）或 `StartAbilityByCallWithSkill`（`IsAbilityStarted` 为真分支）的调用，`skillCallerTokenId` 由 `uint32_t` 承载：
  - 签名：`ability_manager_service.h:1448`、`:2242`、`:2245`；实现 `ability_manager_service.cpp:4678`、`:15499`、`:15551`
  - 字段：`ability_request.h:95`（`uint32_t skillCallerTokenId = 0`）、`skill_execute_param.h:32`（`uint32_t callerTokenId = 0`）
  - IPC：proxy `WriteUint32` / stub `data.ReadUint32()`（`ability_manager_stub.cpp:5891`）；来源 `IPCSkeleton::GetCallingTokenID()`（cpp:15424/15388/12174）
  - 结论：`uint32_t` 恒非负 → `> 0` ≡ `!= 0`，最终 `else`（需 `< 0`）**不可达**
- **触发路径反证（回应"IPC 注入负数→直接返回增加稳定性"）**：客户端想注入"-1"，线路上为 `0xFFFFFFFF`，`ReadUint32` 读回 `4294967295 > 0`，**进入第一个分支而非 `else`** → `UpdateCallerInfoByHapTokenId(0xFFFFFFFF)`（`update_caller_info_util.cpp:126-139`）→ `GetHapTokenInfo` 失败仅打日志 return，Want 未获得 caller 身份。即防御性早退分支永远不会触发，"拒绝非法注入"未达成
- **影响**：无行为回归（可达路径与原代码等价）；死分支可能误导复核人认为"已校验非法 tokenId"
- **证据**：类型链逐级核实 + `GetHapTokenInfo` 失败路径
- **建议**：若意图为拒绝非法 tokenId，用 `skillCallerTokenId == Security::AccessToken::INVALID_TOKENID` 显式比较或先 `GetHapTokenInfo` 校验；若仅防御写法，删死 `else`

### [F2] `StartAbilityByCallWithSkill` fresh-start 路径未传播 skillCallerTokenId，两分支 caller 身份不一致 (P2, scanner=logic)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:15544`
- **触发路径**：skill 请求目标 Ability 尚未启动 → 走 `else` 分支 `StartAbilityByCall(want, connect, callerToken/*nullptr，见 :15490*/, oriValidUserId)`
- **影响**：已启动分支用 skillCallerTokenId 覆盖 caller 身份，未启动分支回落为 IPC 实际调用者（SA），同一请求产生不同 caller 身份语义
- **证据**：`:15528-15541` 已启动分支 vs `:15544` fresh-start 分支
- **建议**：fresh-start 也基于 skillCallerTokenId 更新 caller 信息，或文档化两路径语义

### [F3] `ExecuteInAppSkillWithTokenId` 的 callerTokenId 为 IPC 直供外部输入（P3 观察项·不计分, scanner=security+input）

- **位置**：`services/abilitymgr/src/ability_manager_stub.cpp:5891`、`services/abilitymgr/src/ability_manager_service.cpp:15437-15496`
- **触发路径**：任意进程调用 `EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID`（stub :1043-1044）→ `request.callerTokenId` 直读 wire → `CheckSkillPermission` 需 `IsSACall()`（`skill_execute_manager.cpp:98`）放行
- **影响**：实际可达调用方限于 SA（可信），第三方 app 被挡；若未来开放第三方调用且未绑定实际 IPC caller 身份，存在身份伪造面。当前无实际风险面，按观察项处理，不计入 `findings_total`/评分
- **证据**：`CheckSkillPermission` 第 98 行 `IsSACall()` 门禁
- **建议**：观察项——保持 `IsSACall` 门禁；若开放需改为按实际 IPC 调用者核验

---

> 检视阶段未修改任何源码。修复方案请确认后另起任务。
