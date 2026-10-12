# Refute 审查记录

## 审查概要
| 指标 | 数值 |
|------|------|
| 审查范围 | security: 3 条, logic: 3 条 |
| P0/P1 全审 | 1 条 (SEC-01) |
| P2 抽样审 | 3 条 (LOG-01, SEC-02, — LOG-03) |
| 维持 | 4 条 |
| 降级 | 1 条 (LOG-01 P1→P2; SEC-03 P2→P3) |
| 推翻 | 0 条 |
| 合并 | 0 组 |
| **进入最终报告** | **6 条** (P0: 0, P1: 1, P2: 2, P3: 3) |

## 逐条审查

### ✅ 维持 — SEC-01 (P1)

**发现摘要**：`PrelaunchAbilityLocked` 新增 `ResolveAbility` 调用（connect != nullptr 时），在 `AbilityRecord` 的 `callContainer_` 中创建 `REQUESTING` 状态的 `CallRecord`。当 SCB 未在 10s 内响应时，`PostCallTimeoutTask` → `NotifyStartupExceptionBySCB` 从 `tmpAbilityMap_` 和 `callRequestCache_` 中删除记录，但不调用 `NotifyCallersOnTerminate`/`NotifyAllCallDisconnect`。即使调用 `NotifyAllCallDisconnect`，该函数（`call_container.cpp:239`）也跳过非 `REQUESTED` 状态的记录，`REQUESTING` 状态的 connect 回调的 `OnAbilityConnectDone` 和 `OnAbilityDisconnectDone` 均不会被调用。

**第一层质疑（触发路径）**：真的能被触发吗？
- `BY_CALL_TIMEOUT` = 10s（`ui_ability_lifecycle_manager.cpp:90`），是真实超时场景（SCB 繁忙/慢/崩溃）
- `PostCallTimeoutTask` 在 `PrelaunchAbilityLocked:1830` 无条件调度
- `ResolveAbility` 在 `1812-1814` 无条件调用（当 connect != nullptr）
- 结论：**确认可达**。维持。

**第二层质疑（影响夸大）**：后果真的这么严重吗？
- 调用方 RSS 传入 `IAbilityConnection` 回调，期望 `OnAbilityConnectDone` 或 `OnAbilityDisconnectDone`
- 超时后两者均不调用，RSS 回调状态永久悬挂
- 这不是内存泄漏（`sptr` 引用计数正确释放），而是回调合约违约
- 结论：**影响为回调合约违约，影响调用方正确性**。维持 P1。

**第三层质疑**：无跨 scanner 重复。维持。

---

### ⬇️ 降级 — LOG-01: P1 → P2

**发现摘要**：`PrelaunchAbilityLocked:1797-1799`，当目标 ability 已存在于 `sessionAbilityMap_` 时，直接返回 `ERR_INVALID_VALUE`，不调用 `ResolveAbility` 处理 connect 回调。与 `CallAbilityLocked:1913`（正常 call 路径，始终调用 `ResolveAbility`）不一致。

**第一层质疑**：真的能被触发吗？
- RSS 对已存在的 ability 调用 `StartAbilityForPrelaunch` 时触发
- 返回 `ERR_INVALID_VALUE`，调用方知道 prelaunch 失败
- 结论：**可达**，但调用方获得错误码。

**第二层质疑**：影响真的严重吗？
- 调用方获得 `ERR_INVALID_VALUE`，可以据此知道 connect 未注册
- `AbilityRequest` 在栈上销毁，`connect` 的 `sptr` 引用计数递减，无内存泄漏
- 与 SEC-01 不同：此处调用方获得明确错误码，可自行处理
- 结论：**降级为 P2**。调用方获得错误码，可自行清理，影响低于超时路径。

---

### ✅ 维持 — SEC-02 (P2)

**发现摘要**：Stub 端 `ability_manager_stub.cpp:2817-2820`，当 `hasConnect=true` 但 `ReadRemoteObject()` 返回 null 时，`connect` 静默置 null，无 `TAG_LOGW` 警告日志。Service 继续执行不带 connect，调用方收到 `ERR_OK` 但回调永不调用。

**第一层质疑**：真的能被触发吗？
- remote object 在 proxy 写入后、stub 读取前死亡 → 边缘场景
- 但 `iface_cast` 失败（remote 不实现 IAbilityConnection）也会走到此路径
- 结论：**边缘可达**。维持 P2。

**第二层质疑**：影响真的严重吗？
- 调用方收到 `ERR_OK`，认为 prelaunch 成功，期望回调被调用
- 回调永不调用（与 SEC-01 同类问题，但触发条件更边缘）
- 缺少警告日志增加排查难度
- 结论：**维持 P2**。

---

### ⬇️ 降级 — SEC-03: P2 → P3

**发现摘要**：prelaunch 的 `CallRecord` 持有 `IAbilityConnection` proxy 的 `sptr`，未注册 `DeathRecipient`。调用方进程死亡时 `CallRecord` 变为 stale。

**第一层质疑**：真的能被触发吗？
- 仅 RSS 可调用 `StartAbilityForPrelaunch`（`ability_manager_service.cpp:10947` UID 校验）
- RSS 是系统服务，极少死亡
- 结论：**边缘可达**。

**第二层质疑**：影响真的严重吗？
- RSS 死亡后 AMS 侧的 `CallRecord` 在 ability 终止时随 `callContainer_` 一起销毁
- 无持久影响
- 结论：**降级为 P3**。系统服务死亡是边缘场景，无持久影响。

---

### ✅ 维持 — LOG-02 (P3)

**发现摘要**：测试 `StartAbilityForPrelaunch_WithConnect_001` 创建了 `TrackableConnection`（含 `GetConnectDoneCount()`/`GetDisconnectDoneCount()` 计数器），但测试仅断言 `result != ERR_OK`，未验证回调计数。

**质疑**：测试覆盖缺口确实存在。维持 P3。

---

### ✅ 维持 — LOG-03 (P3)

**发现摘要**：IPC 序列化格式变更（新增 `hasConnect` bool + 可选 remote object），向后兼容依赖 `ReadBool()` 在无数据时返回 false 的默认行为。

**质疑**：`MessageParcel::ReadBool()` 在无更多数据时返回 false 是 OpenHarmony 框架行为，向后兼容事实成立。但此行为未在文档中显式声明，依赖隐式行为。维持 P3。
