# 代码检视报告 — StartAbilityForPrelaunch connect 回调参数（Round 2 / 修复后）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。所有 codecheck 报告（含 orchestrator 合并出的统一报告、单 scanner 直接产出的统一报告）必须遵循本模板：章节顺序、字段名、报告元数据块、评分与门禁规则均为**固定格式**，跨报告保持一致，便于门禁脚本解析与历史对比。
> 权威评分与门禁规则为**通用规则**，不在输出报告中呈现；生成时必须按 [`conventions.md`](skills/codecheck/conventions.md) §7（等级归一化）/ §8（必检维度）/ §9（评分与决策矩阵）计算，不得自创分值。

---

## 报告元数据

> **门禁脚本只读取本 YAML 块**。字段名与取值域为固定合约，禁止改名、增删或自定义取值。人工阅读部分从「1. 门禁结论」开始。

<!-- codecheck-report-metadata:start -->
```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "StartAbilityForPrelaunch_connect_param"
  round: 2
  commit_id: "812cbc9489cfec6dbd8c8b7c69cbab9903bd7732"
  change_id: "I291308a07e0d676fca178e4d3076872b3e8da511"
  report_id: "I291308a07e0d676fca178e4d3076872b3e8da511-R2"
  date: "2026-10-12"
  gate_decision: "approve"
  risk_level: "low"
  score: 92
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 4
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 4}
  gate_blockers: []
  must_fix: []
  followups: ["LOG-02", "LOG-03", "NEW-LOG-01", "NEW-LOG-02"]
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
| 建议跟进（P2/P3） | 4 项 |

**一句话结论**：Round 1 的 P1（超时路径回调泄漏）和 2×P2（ability 已存在时 connect 未处理、stub 缺警告日志）均已正确修复；Round 1 的 SEC-03（无 DeathRecipient）经核实为误报（`AddCallRecord` 已注册 DeathRecipient）。剩余 4 项 P3 均为预存模式或测试覆盖缺口，不阻塞上库。

---

## 2. 扣分原因（仅 gate_decision=block 时呈现；approve/conditional/insufficient 时本节省略）

*(本节省略)*

---

## 3. 必须立即处理（P0/P1）

**无。**

Round 1 的 SEC-01（P1）已修复：`NotifyStartupExceptionBySCB:4997-4998` 在删除 `tmpAbilityMap_` 条目前调用 `NotifyCallersOnPrelaunchTimeout()` → `NotifyAllCallDisconnectAll()`，通知所有状态（含 `REQUESTING`）的 `CallRecord` 回调 `OnAbilityDisconnectDone`。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-02 | P3 | 测试未验证回调调用，`TrackableConnection` 计数器未使用 | 在 `StartAbilityForPrelaunch_WithConnect_001` 等用例增加 `GetConnectDoneCount()`/`GetDisconnectDoneCount()` 断言 | 下一补档 |
| LOG-03 | P3 | IPC 向后兼容依赖 `ReadBool()` 默认 false 行为 | 在 `ability_manager_interface.h` 注释中标注向后兼容依赖 | 下一补档 |
| NEW-LOG-01 | P3 | `NotifyAllCallDisconnectAll` 清除 `callRecordMap_` 但未清除 `deathRecipientMap_`（与现有 `NotifyAllCallDisconnect` 一致，预存模式） | 后续在 `NotifyAllCallDisconnect`/`NotifyAllCallDisconnectAll` 中补充 `deathRecipientMap_` 清理 | 下一补档 |
| NEW-LOG-02 | P3 | "already exists" 路径 `ResolveAbility` 返回值未检查（与 new-ability 路径一致） | 后续统一增加 `ResolveAbility` 返回值检查和日志 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | P1×0, P2×0, P3×1 | SEC-01 已修复；SEC-02 已修复；SEC-03 推翻（误报）；NEW-LOG-01 为预存模式 |
| logic-scanner | P2×0, P3×2 | LOG-01 已修复；LOG-02 延续；LOG-03 延续；NEW-LOG-02 为一致模式 |
| input-scanner | N/A — 无持久化写入路径 | 变更涉及 IPC Parcel 读写但无 DB/文件/缓存持久化链路 |

---

## 6. 关键发现详情

> 本轮无 P0/P1 项。以下为 P3 项详情。

### [LOG-02] 测试未验证回调调用 (P3, scanner=logic-scanner)

- **位置**：`test/unittest/ability_manager_service_twelfth_test/ability_manager_service_twelfth_test.cpp:2792`
- **触发路径**：测试 `StartAbilityForPrelaunch_WithConnect_001` 创建 `TrackableConnection`（含 `GetConnectDoneCount()`/`GetDisconnectDoneCount()`），但仅断言 `result != ERR_OK`，不断言回调计数
- **影响**：回调生命周期未测试，SEC-01 类问题无法被测试发现
- **证据**：`ability_manager_service_twelfth_test.cpp:2807-2808` — `auto result = abilityMs_->StartAbilityForPrelaunch(want, 0, connect); EXPECT_NE(result, ERR_OK);`
- **建议**：在测试用例中增加 `GetConnectDoneCount()`/`GetDisconnectDoneCount()` 断言

### [LOG-03] IPC 向后兼容依赖 ReadBool 默认行为 (P3, scanner=logic-scanner)

- **位置**：`services/abilitymgr/src/ability_manager_stub.cpp:2815`
- **触发路径**：旧客户端（无 connect 参数）调用新 Stub → Stub 读取 Want + frameNum → `ReadBool()` 无更多数据返回 false → `hasConnect=false` → connect=null → 向后兼容
- **影响**：向后兼容依赖 `MessageParcel::ReadBool()` 的默认 false 行为，此行为未在接口文档中显式声明
- **建议**：在 `ability_manager_interface.h` 的 `StartAbilityForPrelaunch` 注释中标注 IPC 向后兼容依赖

### [NEW-LOG-01] NotifyAllCallDisconnectAll 未清除 deathRecipientMap_ (P3, scanner=security-scanner)

- **位置**：`services/abilitymgr/src/call_container.cpp:275`
- **触发路径**：`NotifyAllCallDisconnectAll` 清除 `callRecordMap_` 但未清除 `deathRecipientMap_` → 若 connect 回调进程在 map 清除后死亡 → `OnConnectionDied` → `FindCallRecordMap` 返回 null → `OnCallConnectDied(nullptr)`
- **影响**：边缘场景（connect 回调进程在 disconnect 通知后死亡），且与现有 `NotifyAllCallDisconnect:252` 行为一致——预存模式，非回归
- **证据**：`call_container.cpp:275` — `callRecordMap_.clear();`（无 `deathRecipientMap_` 清理）；对比 `~CallContainer():30-37` 析构函数清除两个 map
- **建议**：后续在 `NotifyAllCallDisconnect`/`NotifyAllCallDisconnectAll` 中补充 `deathRecipientMap_` 清理

### [NEW-LOG-02] "already exists" 路径 ResolveAbility 返回值未检查 (P3, scanner=logic-scanner)

- **位置**：`services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1803`
- **触发路径**：`PrelaunchAbilityLocked` 的 "already has ability record" 路径调用 `ResolveAbility(uiAbilityRecord, abilityRequest)` 但未检查返回值
- **影响**：`ResolveAbility` 失败时 connect 未注册，调用方获得 `ERR_INVALID_VALUE`（正确行为）。与 new-ability 路径（`1818-1820`）一致——后者同样不检查返回值
- **证据**：`ui_ability_lifecycle_manager.cpp:1803` — `ResolveAbility(uiAbilityRecord, abilityRequest);`（无返回值检查）；`1818-1820` — `ResolveAbility(uiAbilityRecord, abilityRequest);`（同样无检查）
- **建议**：后续统一增加 `ResolveAbility` 返回值检查和日志

---

## 附录：Round 1 → Round 2 修复对照

| Round 1 ID | 严重度 | Round 1 问题 | Round 2 状态 | 修复方式 |
|---|---|---|---|---|
| SEC-01 | P1 | 超时路径回调泄漏 | ✅ 已修复 | `NotifyStartupExceptionBySCB` 新增 `NotifyCallersOnPrelaunchTimeout()` → `NotifyAllCallDisconnectAll()`（无状态过滤） |
| LOG-01 | P2 | ability 已存在时 connect 未处理 | ✅ 已修复 | "already exists" 路径新增 `ResolveAbility` 调用 |
| SEC-02 | P2 | stub 缺 null remoteObj 警告日志 | ✅ 已修复 | 新增 `TAG_LOGW` |
| SEC-03 | P3 | 无 DeathRecipient | ❌ 推翻（误报） | `AddCallRecord:92` 已调用 `AddConnectDeathRecipient` |
| LOG-02 | P3 | 测试未验证回调 | ⏸️ 延续 | — |
| LOG-03 | P3 | IPC 兼容隐式行为 | ⏸️ 延续 | — |
