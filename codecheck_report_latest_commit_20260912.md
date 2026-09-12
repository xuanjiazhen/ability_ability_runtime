# 代码检视报告 — 最新提交 c6ed2d2539（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "services/abilitymgr + interfaces/inner_api/ability_manager (commit c6ed2d2539, 给SA提供指定tokenId执行意图的inner接口)"
  round: 1
  commit_id: "c6ed2d253961520eba1b502f0057181e73f00845"
  change_id: "e1779e29085edf0f9f6901c2badb335d565d4d3e"
  report_id: "e1779e29085edf0f9f6901c2badb335d565d4d3e-R1"
  date: "2026-09-12"
  gate_decision: "approve"
  risk_level: "low"
  score: 90
  dimensions_required: "security-scanner, logic-scanner, input-scanner"
  dimensions_executed: "security-scanner, logic-scanner, input-scanner"
  findings_total: 5
  findings_by_severity: "P0:0, P1:0, P2:0, P3:5"
  gate_blockers: 0
  must_fix: 0
  followups: 5
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **90/100** |
| 阻塞项 | 无 P0/P1（3 项原始 P2/P3 已 refute：userId 解析至前台用户、现有调用方传空 Want、hostClient 约束为公用特性） |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 5 项 |

**一句话结论**：新增的 SA 指定 tokenId 执行前台意图 inner 接口复用既有 `CheckAndUpdateParam` / `GenerateWant` / `StartAbilityWithInsightIntent` 链路，FOUNDATION_UID 守卫与 downstream 权限校验模式与既有 `StartAbilityWithSpecifyTokenId` 一致，无 P0/P1 高危项。3 项原始发现已对抗性验证 refute（userId 解析至前台用户、现有调用方传空 Want、hostClient 约束为公用特性），剩余 5 项 P3 为安全登记、错误区分度、Marshalling 返回值、文件格式与 UT 覆盖建议。

---

## 2. 扣分原因

> 本报告为 conditional 决策，省略扣分原因表（仅 block 时呈现）。

---

## 3. 必须立即处理（P0/P1）

**无。**　详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

> 3 项原始发现已 Refute 关闭：LOG-001（userId 经 `GetCallerUserId` 解析至前台用户，符合设计）、LOG-002（现有调用方传空 Want，覆盖/合并等价）、IPC-001（hostClient 约束为公用特性）。详见 `refute_log.md`。

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| SEC-001 | P3 | 新提权面：foundation SA 可借任意 HAP token 执行前台意图 | 登记为安全敏感 inner 接口，纳入安全评审与调用方清单管理 | 下轮 |
| SEC-002 | P3 | `specifyTokenId=0` 时 `GetHapTokenInfo` 失败仅返回 INNER_ERR，错误无区分度 | 增加显式参数校验，返回明确错误码并记录调用方 | 下轮 |
| IPC-002 | P3 | `Marshalling` 忽略写失败恒返回 true，Proxy 的 `WriteParcelable` 防护对该类型失效 | 与既有 `InsightIntentExecuteParam::Marshalling` 模式一致，建议后续统一改为检查返回值 | 后续 |
| STYLE-001 | P3 | 新文件 `insight_intent_execute_lite_param.cpp` 末尾缺少换行 | 补充文件末尾换行 | 随手 |
| TEST-001 | P3 | 新服务方法仅有拒绝路径测试，正路径（foundation 调用 + 合法 token）无 UT 覆盖 | 增加对 `PrepareAndGenerateForegroundIntent` / `BuildExecuteParamFromWant` 子流程正路径测试 | 下轮 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | PASS（2 项 P3） | FOUNDATION_UID 守卫与既有模式一致，downstream 权限校验完整；新增 impersonation 面需安全登记 |
| logic-scanner | PASS（0 项） | LOG-001 (userId 解析) 与 LOG-002 (GenerateWant 语义) 已 refute，无残留 |
| input-scanner | PASS（3 项 P3） | IPC 入参 Want/LiteParam 均有 null 防护；Marshalling 返回值未检查、token=0 错误区分度为次要项 |

---

## 6. 关键发现详情

### 已 Refute 的原始发现

| ID | 原始结论 | 最终判定 | 证据 |
|---|---|---|---|
| LOG-001 | P2：userId 未从指定 token 推导 | ❌ 推翻 | `UserController::GetCallerUserId`（`services/common/src/user_controller/user_controller.cpp:124-133`）对 foundation 调用方（callerUser 0/1）映射到 `GetForegroundUserId()`，即当前前台用户；`GetValidUserId(-1)` 走该路径，与"当前用户"设计一致 |
| LOG-002 | P3：`GenerateWant` 覆盖改合并 | ❌ 推翻 | 既有调用方 `ExecuteIntentCommon` 传入全新空 `Want`（`ability_manager_service.cpp:15132`），覆盖与合并在该场景行为等价 |
| IPC-001 | P3：`insightIntentHostClient` 隐藏约束 | ⬇️ 关闭 | 为意图执行公用特性，符合设计 |

### [SEC-001] 新提权面：foundation SA 可借任意 HAP token 执行前台意图 (P3, scanner=security-scanner)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:15018-15068`
- **触发路径**：仅 foundation UID 可到达（15022-15025 守卫）；foundation SA 传入任意 `specifyTokenId`，若该 token 为系统 app 且持有 `EXECUTE_INSIGHT_INTENT_PERMISSION`，则意图按该 token 身份执行。
- **影响**：与既有 `StartAbilityWithSpecifyTokenId`（893 行同款守卫）一致，属设计意图；但本接口的 token 完全由调用方指定，恶意或缺陷的 foundation SA 可将启动动作归属到其他 app token 名下（影响防滥用/限流归因）。非新漏洞，需安全登记。
- **证据**：`permission_verification.cpp:59-75`（`VerifyCallingPermission` 用指定 token 校验）；`insight_intent_execute_manager.cpp:726-760`（`CheckCallerPermission`）。
- **建议**：将该 inner 接口列入安全敏感清单，维护允许调用方白名单与 token 来源约束。

### [SEC-002] `specifyTokenId=0` 无显式防护，错误无区分度 (P3, scanner=input-scanner)

- **位置**：`services/abilitymgr/src/ability_manager_service.cpp:15034-15041`
- **触发路径**：foundation SA 传入 `specifyTokenId=0` → `GetHapTokenInfo(0)` 失败 → 返回 `INNER_ERR`。
- **影响**：合法 token 缺失与查询失败无法区分，不利于定位。
- **建议**：stub 或服务入口对 `specifyTokenId==0` 提前校验，返回明确错误码。

### [IPC-002] `Marshalling` 忽略写失败恒返回 true (P3, scanner=input-scanner)

- **位置**：`services/abilitymgr/src/insight_intent/insight_intent_execute_lite_param.cpp:49-56`
- **触发路径**：Proxy `data.WriteParcelable(&param)` 依赖 `Marshalling` 返回值，写失败（如 OOM）时该类型仍返回 true，WriteParcelable 防护失效。
- **影响**：与既有 `InsightIntentExecuteParam::Marshalling`（execute_param.cpp:181-206）模式一致，未引入新偏差，属全库一致性问题。
- **建议**：后续统一补写返回值检查。

### [STYLE-001] 新文件末尾缺少换行 (P3, scanner=logic-scanner)

- **位置**：`services/abilitymgr/src/insight_intent/insight_intent_execute_lite_param.cpp`（文件末尾 `\ No newline at end of file`）
- **建议**：补充末尾换行。

### [TEST-001] 新服务方法无正路径 UT 覆盖 (P3, scanner=logic-scanner)

- **位置**：`test/unittest/ability_manager_service_mock_test/ability_manager_service_mock_test.cpp:673-689, 770-785`
- **触发路径**：测试仅覆盖非 foundation UID 拒绝路径与 LiteParam 编解码；正路径（foundation 调用 + 合法 token + CheckAndUpdateParam→GenerateWant→StartAbility）因 `FOUNDATION_UID` 守卫在 UT 进程内不可达。
- **影响**：新接口核心链路缺少自动化验证；建议对 `PrepareAndGenerateForegroundIntent` / `BuildExecuteParamFromWant` 子流程补充正路径断言。
