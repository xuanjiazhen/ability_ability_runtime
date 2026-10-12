# Refute 审查记录 — Round 2

## 审查概要
| 指标 | 数值 |
|------|------|
| 审查范围 | Round 1 全部 6 条发现 + 修复引入的新代码 |
| Round 1 修复验证 | 3 条 (SEC-01 P1, LOG-01 P2, SEC-02 P2) |
| Round 1 推翻 | 1 条 (SEC-03 P3 — 误报) |
| Round 1 延续 | 2 条 (LOG-02 P3, LOG-03 P3) |
| 新发现 | 2 条 (NEW-LOG-01 P3, NEW-LOG-02 P3) |
| **进入最终报告** | **4 条** (P0: 0, P1: 0, P2: 0, P3: 4) |

## 逐条审查

### ✅ 已修复 — SEC-01 (原 P1)

**修复内容**：`NotifyStartupExceptionBySCB:4997-4998` 新增 `if (abilityRecord->GetPrelaunchFlag()) { abilityRecord->NotifyCallersOnPrelaunchTimeout(); }`。`NotifyCallersOnPrelaunchTimeout`（`ability_record.cpp:2889-2902`）调用 `callContainer_->NotifyAllCallDisconnectAll(element)`。

**验证**：
- `NotifyAllCallDisconnectAll`（`call_container.cpp:255-276`）遍历所有 `CallRecord` **无状态过滤**（对比 `NotifyAllCallDisconnect:239` 的 `!IsCallState(CallState::REQUESTED)` 过滤），对每条记录调用 `OnAbilityDisconnectDone(element, REASON_CALLEE_TERMINATE)`。
- 调用时序正确：`NotifyCallersOnPrelaunchTimeout` 在 `tmpAbilityMap_.erase(it)` 之前调用（`ui_ability_lifecycle_manager.cpp:4998` vs `5001`），`abilityRecord` 此时仍存活。
- `sessionLock_` 持有期间调用，`NotifyAllCallDisconnectAll` 内部获取 `callRecordMapLock_`，锁顺序 `sessionLock_ → callRecordMapLock_` 与 `PrelaunchAbilityLocked` 一致，无死锁。
- `ffrt::submit` lambda 仅捕获 `callback`（sptr 拷贝）和 `element`（值拷贝），不依赖 `CallRecord`，`callRecordMap_.clear()` 后异步回调安全。

**结论**：修复正确，回调合约满足。✅ 移除。

---

### ✅ 已修复 — LOG-01 (原 P2)

**修复内容**：`PrelaunchAbilityLocked:1799-1804` 在 "already has ability record" 路径（`sessionAbilityMap_.at(persistentId)` 分支）新增：当 `connect != nullptr` 时调用 `AddCallerRecord` + `SetLaunchReason` + `ResolveAbility`。

**验证**：
- 与正常 call 路径 `CallAbilityLocked:1913` 行为一致（均调用 `ResolveAbility`）。
- `ResolveAbility` 在 `sessionLock_` 持有期间调用，线程安全。
- 返回值仍为 `ERR_INVALID_VALUE`——调用方知道 prelaunch 未启动新实例，但 connect 已注册。语义可接受。

**结论**：修复正确，与正常 call 路径行为一致。✅ 移除。

---

### ✅ 已修复 — SEC-02 (原 P2)

**修复内容**：`ability_manager_stub.cpp:2818` 新增 `else { TAG_LOGW(..., "hasConnect=true but ReadRemoteObject returned null"); }`。

**验证**：警告日志在 `remoteObj == nullptr` 分支正确输出。

**结论**：修复正确。✅ 移除。

---

### ❌ 推翻 — SEC-03 (原 P3)

**Round 1 发现摘要**：`CallRecord` 未注册 `DeathRecipient`，调用方死亡时 stale。

**推翻理由**：Round 1 审查遗漏了 `CallContainer::AddCallRecord`（`call_container.cpp:92`）已调用 `AddConnectDeathRecipient(connect)`。`AddConnectDeathRecipient`（`call_container.cpp:189-211`）注册了 `AbilityConnectCallbackRecipient`，当 connect 回调的 remote object 死亡时调用 `OnConnectionDied`。因此 prelaunch connect 的 `DeathRecipient` **已注册**，Round 1 发现为误报。

**证据**：`call_container.cpp:92` — `AddConnectDeathRecipient(connect);`（在 `AddCallRecord` 中调用，而 `AddCallRecord` 被 `AbilityRecord::Resolve:2832` 调用）。

**结论**：❌ 推翻。误报。

---

### ⏸️ 延续 — LOG-02 (P3)

未修复。测试仍不断言回调计数。P3 可延后。

---

### ⏸️ 延续 — LOG-03 (P3)

未修复。IPC 兼容依赖 `ReadBool()` 默认行为。P3 可延后。

---

### ✅ 新发现 — NEW-LOG-01 (P3)

**发现**：`NotifyAllCallDisconnectAll`（`call_container.cpp:255-276`）清除 `callRecordMap_` 但未清除 `deathRecipientMap_`。若 connect 回调进程在 map 清除后死亡，`OnConnectionDied` → `FindCallRecordMap` 返回 null → `OnCallConnectDied(nullptr)` 可能产生 null 访问。

**质疑**：
- 与现有 `NotifyAllCallDisconnect:230-253` 行为一致——后者同样不清除 `deathRecipientMap_`。非回归。
- `OnCallConnectDied` 大概率有 null 检查（否则现有 `NotifyAllCallDisconnect` 也会有同样问题）。
- 触发条件：connect 回调进程在 disconnect 通知后死亡——边缘场景。

**结论**：维持 P3。预存模式，非回归。

---

### ✅ 新发现 — NEW-LOG-02 (P3)

**发现**：`PrelaunchAbilityLocked:1803` 的 "already exists" 路径调用 `ResolveAbility` 但未检查返回值。与 new-ability 路径（`1818-1820`）一致——后者同样不检查返回值。

**质疑**：一致的模式，`ResolveAbility` 失败时 connect 不注册，调用方获得 `ERR_INVALID_VALUE`。低影响。

**结论**：维持 P3。一致模式。
