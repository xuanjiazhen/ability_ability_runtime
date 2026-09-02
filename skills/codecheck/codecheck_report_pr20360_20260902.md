# 代码检视报告 — PR #20360 OnAbilityRequestDone 白名单修复（Round 1 / 最新提交）

> 统一报告由 codecheck 工作台生成，用于门禁管控。

---

## 报告元数据

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "PR20360-OnAbilityRequestDone-whitelist"
  round: 1
  commit_id: "9beb49544d8d92f3bd80963b53ff611dc1c566f9"
  change_id: "N/A"
  report_id: "9beb4954-R1"
  date: "2026-09-02"
  gate_decision: "approve"
  risk_level: "low"
  score: 96
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 2
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 2}
  gate_blockers: []
  must_fix: []
  followups: ["LOG-01", "INP-01"]
```
<!-- codecheck-report-metadata:end -->

---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **approve** |
| 风险等级 | 🟢 low |
| 评分 | **96/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 0 项 |
| 建议跟进（P2/P3） | 2 项 |

**一句话结论**：将 `OnAbilityRequestDone` 的黑名单守卫（仅拦 FOREGROUNDING）改为白名单（仅放行 BACKGROUND/INITIAL），在异步回调时点重新校验生命周期前置条件，修复高频振荡场景下 state 卡死 FOREGROUNDING 的问题；三维度扫描无 P0/P1，2 项 P3 建议后续补齐。

---

## 3. 必须立即处理（P0/P1）

**无。**

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P3 | 测试仅覆盖 skip 路径（FOREGROUNDING/FOREGROUND），未覆盖 allow 路径（BACKGROUND/INITIAL） | 后续补充 mock scheduler 使 allow 路径可测，或通过集成测试覆盖 | 下一补档 |
| INP-01 | P3 | 测试文件中 `UIEXTENSION_LAUNCH_TIMESTAMP_HIGH` 常量与 `ability_record.cpp:106` 重复定义 | 导出或 include 共享定义，避免维护时遗漏同步修改 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | ✅ 通过 | 白名单消除 state=FOREGROUND 时重复调 ForegroundUIExtensionAbility 导致卡死 FOREGROUNDING 的 P0 级缺陷；消除 state=BACKGROUNDING 时覆写状态导致后台中断的风险；serialMutex_ 保护 check-then-act；超时安全不变量（state=FOREGROUND ⟹ RemoveForegroundTimeoutTask 已执行）保障无悬挂超时 |
| logic-scanner | ✅ 通过 | 白名单完整覆盖 AbilityState 全枚举：INITIAL/BACKGROUND 放行，其余全部拦截，与 DoForegroundUIExtension 前置条件一致；pendingState 兜底链路完整；RemoveUIExtensionLaunchTimestamp 放置于 ForegroundUIExtensionAbility 之后（IPC 已序列化），skip 路径残留由下次 SetParam 覆写 |
| input-scanner | ✅ 通过 | token 经 GetExtensionByTokenFromServiceMap + CHECK_POINTER 校验；state 经 ConvertToAppAbilityState + early return 校验；sessionInfo null 安全（三元运算符兜底 -1）；无新 IPC 接口暴露 |

---

## 6. 关键发现详情

### [LOG-01] 测试未覆盖 allow 路径（P3, scanner=logic-scanner）

- **位置**：`test/unittest/ui_extension_ability_manager_third_test/ui_extension_ability_manager_third_test.cpp:120-175`
- **触发路径**：OnAbilityRequestDone_001 测 FOREGROUNDING skip，_002 测 FOREGROUND skip，均未测 BACKGROUND/INITIAL allow 路径
- **影响**：allow 路径（ForegroundUIExtensionAbility + RemoveUIExtensionLaunchTimestamp）未被单测覆盖，仅通过集成测试间接验证
- **证据**：两个测试用例均设置非 BACKGROUND/INITIAL 状态，走 skip 路径
- **建议**：后续引入 mock ability scheduler，使 BACKGROUND 状态下可验证 ForegroundUIExtensionAbility 被调用且 timestamp 被清除

### [INP-01] 常量重复定义（P3, scanner=input-scanner）

- **位置**：`test/unittest/ui_extension_ability_manager_third_test/ui_extension_ability_manager_third_test.cpp:40`
- **触发路径**：测试文件 anonymous namespace 中定义 `UIEXTENSION_LAUNCH_TIMESTAMP_HIGH = "ohos.ability.params.uiExtensionLaunchTimestampHigh"`，与 `ability_record.cpp:106` 的 `constexpr` 定义重复
- **影响**：若 ability_record.cpp 中的常量值变更，测试文件不会同步，导致测试误判
- **证据**：两处定义相同字符串值，无 include 共享
- **建议**：将 `ability_record.cpp:106-107` 的常量声明提升到头文件，测试文件 include 引用

---

## 附：修复方案技术摘要

### 根因

`DoForegroundUIExtension` → `MoveToForeground` → `OnAbilityRequestDone`（异步 IPC 回调）之间存在时序鸿沟。黑名单守卫仅拦截 FOREGROUNDING，漏放 FOREGROUND 状态——高频振荡中两次 MoveToForeground 的回调交错到达，先到的完成前台（state→FOREGROUND），后到的未被拦截，调 ForegroundUIExtensionAbility 把 state 打回 FOREGROUNDING 并发重复 FOREGROUND_NEW，应用侧 lifecycleState_ 已是 5 → "lifecycle state equal" 丢弃 → 无 AbilityTransitionDone 回调 → 卡死。

### 修复

黑名单改白名单：仅放行 `BACKGROUND`/`INITIAL`，在异步回调时点重新校验 DoForegroundUIExtension 的全部生命周期前置条件（!INACTIVATING, !FOREGROUNDING, !BACKGROUNDING, !FOREGROUND）。

### 安全性论证

| 不变量 | 保障机制 |
|--------|---------|
| state=FOREGROUND ⟹ 超时已取消 | CompleteForeground ← DispatchForeground ← 先调 RemoveForegroundTimeoutTask |
| PostTimeout 仅在 state=BACKGROUND 时调用 | DoForegroundUIExtension state=FOREGROUND 时走 CommandAbilityWindow |
| 拦截不丢前台 | state=FOREGROUND/FOREGROUNDING/BACKGROUNDING ⟹ 前台已被先到达的放行回调执行 |
| 正常 前-后-前 不触发拦截 | 每次回调到达时 state=BACKGROUND → 放行 |
| 超时残留 | UIExtension 前台超时 needKillProcess=true + CRITICAL_TIMEOUT → DFX 杀进程 |
