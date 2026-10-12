# 代码检视报告 — StartAbilityForPrelaunch connect 回调参数（Round 1 / 最新提交）

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
  round: 1
  commit_id: "be51824d69284aaf0bed1644425ee2feb518efde"
  change_id: "I291308a07e0d676fca178e4d3076872b3e8da511"
  report_id: "I291308a07e0d676fca178e4d3076872b3e8da511-R1"
  date: "2026-10-12"
  gate_decision: "conditional"
  risk_level: "medium"
  score: 72
  dimensions_required: ["security-scanner", "logic-scanner", "input-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner", "input-scanner"]
  findings_total: 6
  findings_by_severity: {P0: 0, P1: 1, P2: 2, P3: 3}
  gate_blockers: []
  must_fix: ["SEC-01"]
  followups: ["LOG-01", "SEC-02", "SEC-03", "LOG-02", "LOG-03"]
```
<!-- codecheck-report-metadata:end


---

## 1. 门禁结论

| 项目 | 结论 |
|---|---|
| 决策 | **conditional** |
| 风险等级 | 🟡 medium |
| 评分 | **72/100** |
| 阻塞项 | 无 |
| 必须修复（P0/P1） | 1 项 |
| 建议跟进（P2/P3） | 5 项 |

**一句话结论**：新增 connect 回调参数在 prelaunch 超时/失败路径存在回调合约违约（`OnAbilityConnectDone`/`OnAbilityDisconnectDone` 均不调用），需修复超时清理路径后再上库。

---

## 2. 扣分原因（仅 gate_decision=block 时呈现；approve/conditional/insufficient 时本节省略）

*(本节省略)*

---

## 3. 必须立即处理（P0/P1）

| ID | 优先级 | Scanner | 问题 | file:line | 触发路径 | 影响 |
|---|---|---|---|---|---|---|
| SEC-01 | P1 | security | Connect 回调在超时/清理路径泄漏：`OnAbilityConnectDone` 与 `OnAbilityDisconnectDone` 均不调用 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1812` | `StartAbilityForPrelaunch(connect!=nullptr)` → `PrelaunchAbilityLocked` → `ResolveAbility` 创建 `REQUESTING` 状态 CallRecord → SCB 10s 未响应 → `PostCallTimeoutTask` → `NotifyStartupExceptionBySCB` 删除记录但不通知回调 | 调用方 RSS 的 `IAbilityConnection` 回调永久悬挂，回调合约违约 |

> 详情见第 6 节。

---

## 4. 建议本轮或下一补档处理（P2/P3）

| ID | 优先级 | 问题 | 建议行动 | 排期 |
|---|---|---|---|---|
| LOG-01 | P2 | ability 已存在时 connect 未处理，与正常 call 路径不一致 | 在 `ERR_INVALID_VALUE` 路径增加 `ResolveAbility` 调用或在文档中明确 connect 未注册 | 本补档 |
| SEC-02 | P2 | Stub 端 remoteObj 为 null 时静默丢弃 connect，无警告日志 | 在 `ability_manager_stub.cpp:2818` 增加 `TAG_LOGW` 日志 | 本补档 |
| SEC-03 | P3 | CallRecord 未注册 DeathRecipient，调用方死亡时 stale | 为 prelaunch connect 注册 DeathRecipient（或注明仅限 RSS 调用，接受风险） | 下一补档 |
| LOG-02 | P3 | 测试未验证回调调用，TrackableConnection 计数器未使用 | 在 `StartAbilityForPrelaunch_WithConnect_001` 等用例增加 `GetConnectDoneCount()` 断言 | 下一补档 |
| LOG-03 | P3 | IPC 向后兼容依赖 `ReadBool()` 默认 false 行为 | 增加 IPC 版本号协商或在头文件注释中标注向后兼容依赖 | 下一补档 |

---

## 5. 分维度速览

| 维度 | 结果 | 关键说明 |
|---|---|---|
| security-scanner | 3 条发现 (P1×1, P2×1, P3×1) | 回调泄漏（超时路径）、silent drop（stub）、缺 DeathRecipient |
| logic-scanner | 3 条发现 (P2×1, P3×2) | ability 已存在时 connect 处理不一致、测试覆盖缺口、IPC 兼容隐式行为 |
| input-scanner | N/A — 无持久化写入路径 | 变更涉及 IPC Parcel 读写但无 DB/文件/缓存持久化链路，input 维度无新增发现 |

---

## 6. 关键发现详情

### [SEC-01] Connect 回调在超时/清理路径泄漏 (P1, scanner=security-scanner)

- **位置**：`services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1812`（新增代码）+ `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:4979`（现有超时清理）+ `services/abilitymgr/src/call_container.cpp:239`（现有 NotifyAllCallDisconnect 过滤逻辑）
- **触发路径**：
  1. RSS 调用 `StartAbilityForPrelaunch(want, frameNum, connect)`，connect != nullptr
  2. `PrelaunchAbilityLocked:1812-1814` 调用 `ResolveAbility(uiAbilityRecord, abilityRequest)`
  3. `ResolveAbility:2269` → `AbilityRecord::Resolve:2843` 创建 `CallRecord`，状态设为 `CallState::REQUESTING`
  4. `PostCallTimeoutTask:1830` 调度 10s 超时任务
  5. SCB 未在 10s 内响应 → `NotifyStartupExceptionBySCB:4979-4996` 从 `tmpAbilityMap_` 和 `callRequestCache_` 删除记录
  6. `NotifyStartupExceptionBySCB` **不调用** `NotifyCallersOnTerminate` 或 `NotifyAllCallDisconnect`
  7. 即使调用 `NotifyAllCallDisconnect`，`call_container.cpp:239` 跳过非 `REQUESTED` 状态记录（`REQUESTING` 被跳过）
  8. `AbilityRecord` shared_ptr 引用计数归零 → 析构 → `callContainer_` 析构 → `CallRecord` 析构
  9. `OnAbilityConnectDone` 和 `OnAbilityDisconnectDone` **均未被调用**
- **影响**：调用方 RSS 的 `IAbilityConnection` 回调永久悬挂。RSS 认为连接仍在建立中，无法得知连接已失败。回调合约违约影响调用方正确性。
- **证据**：
  - 新增代码：`ui_ability_lifecycle_manager.cpp:1812-1814` — `if (abilityRequest.connect != nullptr) { ResolveAbility(uiAbilityRecord, abilityRequest); }`
  - 超时清理：`ui_ability_lifecycle_manager.cpp:1995` — `pThis->NotifyStartupExceptionBySCB(requestId, reason);`
  - 清理逻辑：`ui_ability_lifecycle_manager.cpp:4992-4993` — `tmpAbilityMap_.erase(it); callRequestCache_.erase(abilityRecord);`（无回调通知）
  - 过滤逻辑：`call_container.cpp:239` — `if (callRecord == nullptr || !callRecord->IsCallState(CallState::REQUESTED)) { continue; }`（跳过 REQUESTING）
  - 超时值：`ui_ability_lifecycle_manager.cpp:90` — `constexpr int32_t BY_CALL_TIMEOUT = 10 * 1000 * 1000; // 10s`
  - 状态设置：`ability_record.cpp:2843` — `callRecord->SetCallState(CallState::REQUESTING);`
- **建议**：
  1. 在 `NotifyStartupExceptionBySCB` 中，删除 `tmpAbilityMap_` 条目前，调用 `abilityRecord->NotifyCallersOnTerminate()` 通知所有 connect 回调
  2. 或在 `NotifyAllCallDisconnect` 中移除 `CallState::REQUESTED` 过滤，对所有状态的 `CallRecord` 通知 `OnAbilityDisconnectDone`
  3. 推荐方案 1，因为超时清理是明确的失败路径，应通知所有等待中的 connect 回调

### [LOG-01] ability 已存在时 connect 未处理，与正常 call 路径不一致 (P2, scanner=logic-scanner)

- **位置**：`services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1797`
- **触发路径**：RSS 调用 `StartAbilityForPrelaunch(want, frameNum, connect)`，目标 ability 已存在于 `sessionAbilityMap_` → `persistentId != 0` → `sessionAbilityMap_.at(persistentId)` → 直接返回 `ERR_INVALID_VALUE`，不调用 `ResolveAbility`
- **影响**：connect 回调未注册。调用方获得 `ERR_INVALID_VALUE`，可据此知道 prelaunch 失败，但 connect 回调合约（connect done 或 disconnect done）未满足。与正常 call 路径 `CallAbilityLocked:1913` 不一致——后者始终调用 `ResolveAbility`。
- **证据**：
  - prelaunch 路径：`ui_ability_lifecycle_manager.cpp:1797-1799` — `uiAbilityRecord = sessionAbilityMap_.at(persistentId); return ERR_INVALID_VALUE;`（不处理 connect）
  - 正常 call 路径：`ui_ability_lifecycle_manager.cpp:1913` — `auto ret = ResolveAbility(uiAbilityRecord, abilityRequest);`（始终处理 connect）
- **建议**：在 `ERR_INVALID_VALUE` 路径增加 `ResolveAbility` 调用处理 connect（与 `CallAbilityLocked` 一致），或在头文件注释中标注 `ERR_INVALID_VALUE` 表示 connect 未注册。

### [SEC-02] Stub 端 remoteObj 为 null 时静默丢弃 connect (P2, scanner=security-scanner)

- **位置**：`services/abilitymgr/src/ability_manager_stub.cpp:2817`
- **触发路径**：Proxy 写入 `hasConnect=true` + `connect->AsObject()` → Stub 读取 `hasConnect=true` → `ReadRemoteObject()` 返回 null（remote object 已死或 iface_cast 失败）→ `connect` 保持 null → Service 无 connect 执行 → 调用方收到 `ERR_OK` 但回调永不调用
- **影响**：调用方认为 prelaunch 成功但回调不调用。缺少警告日志增加排查难度。
- **证据**：
  - stub 代码：`ability_manager_stub.cpp:2817-2820` — `auto remoteObj = data.ReadRemoteObject(); if (remoteObj != nullptr) { connect = iface_cast<IAbilityConnection>(remoteObj); }`（remoteObj 为 null 时无日志）
  - proxy 代码：`ability_manager_proxy.cpp:4159` — `bool hasConnect = (connect != nullptr);` + `data.WriteRemoteObject(connect->AsObject())`
- **建议**：在 `ability_manager_stub.cpp:2818` 的 `remoteObj == nullptr` 分支增加 `TAG_LOGW` 警告日志。

### [SEC-03] CallRecord 未注册 DeathRecipient (P3, scanner=security-scanner)

- **位置**：`services/abilitymgr/src/ability_record.cpp:2822-2832`
- **触发路径**：RSS 调用 `StartAbilityForPrelaunch` 传入 connect → `CallRecord::CreateCallRecord` 存储 `connCallback_` → RSS 进程死亡 → `CallRecord` 持有 stale proxy → 无自动清理
- **影响**：stale `CallRecord` 在 `callContainer_` 中。因仅 RSS 可调用（`ability_manager_service.cpp:10947` UID 校验），RSS 为系统服务极少死亡，实际风险低。
- **证据**：`ability_record.cpp:2824` — `CallRecord::CreateCallRecord(abilityRequest.callerUid, shared_from_this(), callback, abilityRequest.callerToken);`（无 DeathRecipient 注册）
- **建议**：为 prelaunch connect 注册 `DeathRecipient`（或注明仅限 RSS 调用，接受风险）。

### [LOG-02] 测试未验证回调调用 (P3, scanner=logic-scanner)

- **位置**：`test/unittest/ability_manager_service_twelfth_test/ability_manager_service_twelfth_test.cpp:2792`
- **触发路径**：测试 `StartAbilityForPrelaunch_WithConnect_001` 创建 `TrackableConnection`（含 `GetConnectDoneCount()`/`GetDisconnectDoneCount()`），但仅断言 `result != ERR_OK`，不断言回调计数
- **影响**：回调生命周期未测试，SEC-01 类问题无法被测试发现
- **证据**：`ability_manager_service_twelfth_test.cpp:2807-2808` — `auto result = abilityMs_->StartAbilityForPrelaunch(want, 0, connect); EXPECT_NE(result, ERR_OK);`（无 `EXPECT_EQ(connect->GetConnectDoneCount(), 0)` 等断言）
- **建议**：在测试用例中增加 `GetConnectDoneCount()` / `GetDisconnectDoneCount()` 断言。

### [LOG-03] IPC 向后兼容依赖 ReadBool 默认行为 (P3, scanner=logic-scanner)

- **位置**：`services/abilitymgr/src/ability_manager_stub.cpp:2815`
- **触发路径**：旧客户端（无 connect 参数）调用新 Stub → Stub 读取 Want + frameNum → `ReadBool()` 无更多数据返回 false → `hasConnect=false` → connect=null → 向后兼容
- **影响**：向后兼容依赖 `MessageParcel::ReadBool()` 的默认 false 行为，此行为未在接口文档中显式声明
- **证据**：旧客户端写入 Want + frameNum（2 个 parcelable）；新 Stub 读取 Want + frameNum + hasConnect（3 个 parcelable），第 3 个 `ReadBool()` 默认 false
- **建议**：在 `ability_manager_interface.h` 的 `StartAbilityForPrelaunch` 注释中标注 IPC 向后兼容依赖 `ReadBool` 默认行为。
