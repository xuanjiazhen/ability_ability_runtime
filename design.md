# InsightIntent 执行取消（Cancel）能力设计

> 状态：设计评审稿（接口语义已评审：onCancel 返回 `void | Promise<void>`；未实现 onCancel 走「错误码 + 回执闭环」）
> 范围仓库：foundation/ability/ability_runtime
> 关联背景：意图框架（InsightIntent）执行链路目前仅支持单向执行 + 结果回传，长耗时意图（ServiceExtension 后台任务、AI agent 工具调用）无法被调用方中途取消。

## 1. 背景与目标

现状执行链路：调用方通过 `insightIntentDriver.execute()` 发起意图执行 → AMS 分派 → 目标应用内执行器（`InsightIntentExecutor` / `InsightIntentEntryExecutor`）通过 `onExecute` 系列方法处理 → 结果经 AMS 回传调用方 Promise。**调用方没有任何手段中止一次在途执行。**

本设计为意图执行增加取消能力：

- 调用方新增 `insightIntentDriver.cancel(toolCallId)` 入口（`toolCallId` 已是 `ExecuteParam` 既有字段，作为取消关联键，**不改动 execute 签名与返回形态**）；
- AMS 按 toolCallId 检索在途执行记录，校验发起者身份后路由到目标应用；
- 目标应用内执行器新增可选的 `onCancel` 接口（与 `onExecute` 并列），开发者在 onCancel 中实现业务中止；
- 取消结果**等目标回执后**闭环：真正取消 → execute Promise 以 `16000139` 收尾、cancel() resolve；目标未实现 onCancel → cancel() reject `16000140`，原执行不受影响。

## 2. 需求范围（已确认决策）

| 决策项 | 结论 |
|---|---|
| 取消发起方 | 调用方进程（driver.execute 的调用者）。系统侧/CLI 按 toolCallId 的取消入口不在本期，但 AMS 取消通道设计上可复用 |
| 关联键 | `toolCallId`（`InsightIntentExecuteParam::toolCallId_`，want key `ohos.insightIntent.toolCallId`）；仅覆盖经 driver.execute 登记 key 的执行路径（LiteParam 直连路径不覆盖） |
| 执行器覆盖 | 全部执行模式（UIAbility 前台/后台、UIExtension、ServiceExtension）+ `InsightIntentEntryExecutor`（@InsightIntentEntry） |
| 不覆盖 | FUNC / PAGE / LINK 装饰器（方法直调 / UI 跳转，无法中断）；CJ（Cangjie）侧列为后续项 |
| 分布式意图 | 本期不支持取消（`isDistributed == true` 直接拒绝） |
| onCancel 返回类型 | `void \| Promise<void>`（评审结论，返回值无业务语义） |
| 未实现 onCancel | **错误码 + 回执闭环**（评审结论）：cancel() reject `16000140`，记录保持 EXECUTING，原执行不受影响 |

## 3. 端到端流程

```
调用方进程                          AMS                                目标应用进程
────────────                       ────────                           ────────────
driver.execute(param 含 toolCallId)
 ──EXECUTE_INTENT(同步IPC)──▶ records_[intentId] 登记
                              (新增字段: toolCallId / callerTokenId /
                               targetToken / pendingCancelKey=0)
                              冷启动: AbilityRecord::Init 时绑定 targetToken
                              已启动: AMS::OnExecuteIntent 时绑定 targetToken
 客户端 hostClient 记 toolCallId→key 映射

driver.cancel(toolCallId)
 1. 客户端按 toolCallId 查 key，挂起 cancel defer
 ──EXECUTE_INTENT_CANCEL=76(key,toolCallId)──▶
                              CancelIntent: 校验 state==EXECUTING &&
                              callerTokenId 匹配 && record.key==key &&
                              toolCallId 相等 → 挂 pendingCancelKey
                              ▶ GetTargetToken(intentId)
                              ▶ Token::GetAbilityRecordByToken
                              ──OnExecuteIntentCancel(intentId) [TF_ASYNC]──▶
                                                                              AbilityThread → PostTask
                                                                              → ExecutorMgr::CancelInsightIntent
                                                                                (锁内 erase 原子取出)
                                                        ┌── 未实现 onCancel ──▶ 桥返回 false
                              ◀─NOTIFY_INTENT_CANCEL_RESULT=6182──┘        （新IPC，仅"存在但不支持"时）
                              │                                            └── 已实现 ──▶ 桥调 onCancel
                              │                                                          │ settle
                              │                                    ExecuteInsightIntentDone(取消码)
                              ◀────────────────────────────────────┘（现有完成通道）
 OnCancelDone(key, err) ◀──── iface_cast<IInsightIntentExecuteCallback>->OnCancelDone
 cancel() settle: 取消→resolve；不支持/异常→reject
 execute Promise 以 16000139 收尾（取消成功时）
```

## 4. 核心语义设计

1. **取消码**
   - native 层：`ERR_INSIGHT_INTENT_CANCELED = 2097405`（已取消）、`ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED = 2097406`（目标未实现 onCancel）
   - inner 层：`InsightIntentInnerErr::INSIGHT_INTENT_ERR_CANCELED / INSIGHT_INTENT_ERR_CANCEL_NOT_SUPPORTED`
   - JS 业务码：`ERROR_CODE_INTENT_CANCELED = 16000139`、`ERROR_CODE_INTENT_CANCEL_NOT_SUPPORTED = 16000140`
2. **cancel() 语义（回执闭环）**：resolve = 目标确认已取消；reject = 空参 401 / 无在途执行 2097329 / 已结束 2097330 / 非发起方 CHECK_PERMISSION_FAILED / 目标未就绪 2097333 / 未实现 onCancel 16000140 / 回执超时或异常 INNER_ERR。**记录挂 `pendingCancelKey` 后 cancel() 保持 pending 直到回执**；挂起失败（重复取消）返回 INNER_ERR。
3. **回执超时**：cancel() 挂起期间目标无回执（进程死亡/消息丢失）时超时兜底——复用 `SetRemoteIntentTimeout`（ability_manager_service.cpp:15472）的 ffrt 延时 + `ffrt::skip` 取消模式，建议 `CANCEL_ACK_TIMEOUT_SECONDS = 10s`，超时按 INNER_ERR 拒绝并清理 pendingCancelKey。
4. **竞态（三方收敛，`pendingCancelKey` 只消费一次）**：AMS 在锁内串行化三类事件——目标「取消完成」（ExecuteIntentDone 携带取消码）/ 目标「自然完成」（普通结果码）/ 「不支持回执」（新 IPC）/ 「超时」。先到者消费 pendingCancelKey 并 settle cancel()，其余忽略；记录终态只允许进入一次。
5. **onCancel 契约**
   - 返回类型 `void | Promise<void>`（评审结论）：返回值无业务语义——settle（resolve 或 reject）即视为取消完成，框架强制上报取消码；onCancel 内部出错不恢复执行。
   - **基类不给默认实现**（关键）：`onCancel` 在基类仅作可选声明（`onCancel?: () => void | Promise<void>`），无函数体。若给空默认体，继承体系下"未实现"永远探测命中，错误码语义退化为"静默取消"（向调用方撒谎：业务没停却上报已取消）。
   - 未实现 ⇒ 桥探测不到（JS 用 `GetPropertyValueByPropertyName` 判函数类型；ETS 靠 `Class_FindMethod` 失败）⇒ 应用侧上报"不支持"（新 IPC），记录保持 EXECUTING，原执行不受影响。
6. **授权**：仅原调用方进程可取消。`AddRecord` 时用 `IPCSkeleton::GetCallingTokenID()` 捕获（binder 线程上执行，含 DMS 转发路径），cancel 时比对 `callerTokenId` + `record.key` + `toolCallId` 三重匹配；不改 6 个 `CheckAndUpdateParam` 调用点签名。
7. **窄窗口竞态（文档化即可）**：execute 的 IPC 尚未在 AMS 落记录时发起 cancel → 返回 2097329（INTENT_NOT_EXIST），调用方可重试；窗口为一次 IPC 往返。
8. **UIExtension 特别说明**（现状已核实）：`AMS::ExecuteIntentCommon` 对 `UI_EXTENSION_ABILITY` 直接返回 `ERR_INVALID_OPERATION`（当前不可达）⇒ AMS 路由表不会出现 UIExtension 目标；执行器层 UIExtension 支持仅作完备性保留。`js_ui_extension_base.cpp:546-549` 的完成上报有条件（`uris 非空 || interactionInfo != nullptr`）⇒ cancel 上报必须走独立回调、无条件上报。

## 5. 已核实的关键事实（实现前提，勿推翻）

1. `InsightIntentExecuteManager::ExecuteIntentDone`（services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:301）只置 `state = EXECUTE_DONE` 并清空 `callerToken`，**记录不删除**，由 256 条上限淘汰（:44/:278）⇒「已结束不可取消」判定条件是 `state != EXECUTING`，且注意：给调用方回 `OnCancelDone` 必须在清空 `callerToken` 之前（或在记录仍可访问时）完成。
2. `AbilityRecord::Init()`（services/abilitymgr/src/ability_record.cpp:194，`token_` 创建于 :223）被 UIAbility / Mission / BaseExtension 三条 CreateAbilityRecord 路径共同调用 ⇒ 在 Init 内做 intentId→targetToken 绑定，**一次覆盖三条冷启动路径**。
3. `InsightIntentExecuteConnection::OnAbilityConnectDone` 是空实现且全仓无业务调用点，**不能**用作 ServiceExtension 冷启动 hook。
4. 编号空闲已核实：IPC `EXECUTE_INTENT_CANCEL = 76`（74/75/77-81 均已占用，76 是该簇唯一空位）；应用→AMS 新通知 IPC 尾部追加 `NOTIFY_INTENT_CANCEL_RESULT = 6182`（现尾部 `REGISTER_WANT_AGENT_HOLDER = 6181`，该枚举按顺序增长）；`2097405/2097406`、`16000139/16000140` 均空闲；`IInsightIntentExecuteCallback` 消息枚举现有仅 `ON_INSIGHT_INTENT_EXECUTE_DONE = 1` ⇒ 新增 `ON_INSIGHT_INTENT_CANCEL_DONE = 2`。
5. `js_insight_intent_entry.cpp:322` 以 `CallJsFunctionWithResultInner("onExecute", 0, nullptr)` 调 Entry 执行器，onCancel 采用相同调用形态。
6. 取消是**协作式**的：业务是否真正停止取决于开发者是否实现并响应 onCancel；框架不能强杀执行中的 JS Promise。这是"未实现必须报错而非默认成功"的根本依据。
7. **与 Apple AppIntents 取消模型对照**（官方文档核实）：Apple 以独立协议 `CancellableIntent`（iOS 26.4+）声明取消能力，取消同为协作式，与本设计"可选声明 + 未实现 = 取消不支持"同构。详细对照与借鉴点见 §11。

## 6. 详细修改点

### 6.1 常量与错误码

| 文件 | 修改 |
|---|---|
| `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`（:1023 后） | `ERR_INSIGHT_INTENT_CANCELED = 2097405`、`ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED = 2097406` |
| `interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_constant.h`（`InsightIntentInnerErr`） | 加 `INSIGHT_INTENT_ERR_CANCELED = AAFwk::ERR_INSIGHT_INTENT_CANCELED`、`INSIGHT_INTENT_ERR_CANCEL_NOT_SUPPORTED = AAFwk::ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED`（写法对齐现有条目） |
| `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（:327 后） | `ERROR_CODE_INTENT_CANCELED = 16000139`、`ERROR_CODE_INTENT_CANCEL_NOT_SUPPORTED = 16000140` |
| `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.cpp` | 两条消息常量 + 外部码→消息表、native→JS 映射表各加两条（`ERR_INTENT_DEVICE_DISCONNECTED` 条目旁） |
| `frameworks/c/ability_runtime/src/ability_business_error_utils.cpp` | 同步 C API 映射表 |

复用既有错误码（无需新增）：`INTENT_NOT_EXIST=2097329`、`INTENT_STATE_NOT_EXECUTING=2097330`、`ERR_TARGET_NOT_STARTED=2097333`、`CHECK_PERMISSION_FAILED`、`ERR_INVALID_VALUE`、`INNER_ERR`。

### 6.2 inner_api 接口层

| 文件 | 修改 |
|---|---|
| `ability_manager_ipc_interface_code.h` | ① `EXECUTE_INTENT_CANCEL = 76`（插在 `GET_FOREGROUND_UI_ABILITIES = 75` 与 `OPEN_ATOMIC_SERVICE = 77` 之间，勿动其他值）；② 尾部 `REGISTER_WANT_AGENT_HOLDER = 6181` 后加 `NOTIFY_INTENT_CANCEL_RESULT = 6182`（应用→AMS，上报"未实现取消"） |
| `ability_manager_interface.h`（:2105 后） | `virtual int32_t CancelExecuteIntent(uint64_t key, const std::string &toolCallId) { return 0; }`、`virtual int32_t NotifyIntentCancelResult(uint64_t intentId, int32_t resultCode) { return 0; }`（均带默认实现，模式对齐 :2051） |
| `insight_intent_execute_callback_interface.h` | `virtual void OnCancelDone(uint64_t key, int32_t resultCode) = 0;` + 枚举 `ON_INSIGHT_INTENT_CANCEL_DONE = 2`；同步更新其 Stub/Proxy（`insight_intent_execute_callback_{stub,proxy}.h/.cpp`，现仅 1 个消息码，追加分发与发送分支） |
| `ability_scheduler_interface.h`（:297 后） | `virtual void OnExecuteIntentCancel(uint64_t intentId) {}`（带空默认体，模式对齐 :325 `ExecuteSkill`）；枚举尾部（:432 后）加 `SCHEDULE_ONEXECUTE_INTENT_CANCEL,` |
| `ability_manager_client.h/.cpp`（:2485 后） | `ErrCode CancelExecuteIntent(uint64_t key, const std::string &toolCallId);`、`ErrCode NotifyIntentCancelResult(uint64_t intentId, int32_t resultCode);`（实现对齐 :2154 `ExecuteIntent`） |

### 6.3 AMS 服务端

**`services/abilitymgr/include/insight_intent/insight_intent_execute_manager.h`**

```cpp
// InsightIntentExecuteRecord（:39-49）新增字段
std::string toolCallId;                        // 调用方业务标识（可为空）
uint32_t callerTokenId = 0;                    // IPCSkeleton::GetCallingTokenID() 捕获
sptr<IRemoteObject> targetToken = nullptr;     // 目标 ability token
uint64_t pendingCancelKey = 0;                 // 在途取消请求的调用方 key（0=无）

// InsightIntentExecuteManager 公有新增
int32_t CancelIntent(uint64_t key, const std::string &toolCallId, uint32_t callerTokenId, uint64_t &intentId);
int32_t ConsumeCancelKey(uint64_t intentId, uint64_t &cancelKey);   // 回执时原子取出 pendingCancelKey
int32_t BindTargetToken(uint64_t intentId, const sptr<IRemoteObject> &targetToken);
int32_t GetTargetToken(uint64_t intentId, sptr<IRemoteObject> &targetToken) const;
void UnbindTargetToken(uint64_t intentId);

// 私有
// AddRecord（:136）尾参加 toolCallId / callerTokenId
// 成员新增 std::map<uint64_t, sptr<IRemoteObject>> targetTokens_;（复用 mutex_）
```

**`services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp`**

- `AddRecord`（:256）：填充新字段；`callerTokenId` 用 `IPCSkeleton::GetCallingTokenID()` 函数内捕获；淘汰分支（:278-288）调 `UnbindTargetToken`。
- `CancelIntent`：锁内线性扫 `records_`（上限 256，不建反向 map），比对 `toolCallId` 非空相等 + `state == EXECUTING` + `callerTokenId` 匹配 + `record.key == key`；`pendingCancelKey != 0` → INNER_ERR（重复取消幂等保护）；通过则置 `pendingCancelKey = key` 并返回 intentId。**锁内不得做 IPC**。
- `ConsumeCancelKey`：锁内取出并清零 pendingCancelKey，供四类回执路径调用，保证只消费一次。
- `ExecuteIntentDone`（:301）：改两处——① 结果码为取消码分支：先 `ConsumeCancelKey` 取到 key 则通过 `callerToken`（**尚未清空时**）回调 `OnCancelDone(cancelKey, ERR_OK)`，再走既有完成流程；② 普通结果码分支（自然完成）：若 `pendingCancelKey != 0`，`ConsumeCancelKey` 后回调 `OnCancelDone(cancelKey, INTENT_STATE_NOT_EXECUTING)`（执行已自然结束，取消未生效），记录照常进入 EXECUTE_DONE。
- `BindTargetToken / GetTargetToken / UnbindTargetToken`：mutex_ 下 map 读写；`ExecuteIntentDone` 与 `RemoveExecuteIntent`（:292）同步 `UnbindTargetToken`。

**`services/abilitymgr/src/ability_record.cpp`（冷启动目标 token 绑定，唯一漏斗）**

- `Init()`（:194）内 `token_` 创建（:223）后：want 含 `INSIGHT_INTENT_EXECUTE_PARAM_ID` 时解析 intentId 并 `BindTargetToken(intentId, token_->AsObject())`（幂等、失败不传播）。
- `RemoveInsightIntent()`：移除参数前解析 intentId 并 `UnbindTargetToken`（兜底清理）。

**`services/abilitymgr/src/ability_manager_service.cpp`（头文件 :2256 后声明两个方法）**

- `CancelExecuteIntent(uint64_t key, const std::string &toolCallId)`（放 `ExecuteInsightIntentDone` :15724 后）：
  1. toolCallId 空 → `ERR_INVALID_VALUE`；
  2. `CancelIntent(key, toolCallId, IPCSkeleton::GetCallingTokenID(), intentId)`；
  3. `GetTargetToken` 失败 → 记录已挂 pendingCancelKey，需消费后回执：`ConsumeCancelKey` + 经 callerToken `OnCancelDone(cancelKey, ERR_TARGET_NOT_STARTED)`（cancel() 即刻 reject），返回 ERR_OK（IPC 层面成功，失败语义走回执）；
  4. `Token::GetAbilityRecordByToken` → 判 `GetAbilityState() != TERMINATED && GetScheduler() != nullptr`（模式对齐 :15615），否则同上消费回执 `ERR_INVALID_VALUE`；
  5. `scheduler->OnExecuteIntentCancel(intentId)`（TF_ASYNC）+ 挂 `CANCEL_ACK_TIMEOUT_SECONDS` ffrt 超时任务（复用 :15472 模式，`ffrt::skip` 可取消）；
  6. 返回 `ERR_OK`。
- `NotifyIntentCancelResult(uint64_t intentId, int32_t resultCode)`：目标上报"存在执行器但未实现 onCancel"。锁内 `ConsumeCancelKey` 取 key（取不到说明已超时/已消费，忽略）→ 经 callerToken `OnCancelDone(key, resultCode)`；记录保持 EXECUTING 不动。超时任务到此被 `ffrt::skip` 语义天然作废（pendingCancelKey 已消费）。

**Proxy / Stub**

| 文件 | 修改 |
|---|---|
| `ability_manager_proxy.h/.cpp`（:5752 后） | `CancelExecuteIntent`（WriteUint64(key)+WriteString+SendRequest(76)+ReadInt32）、`NotifyIntentCancelResult`（WriteUint64+WriteInt32+SendRequest(6182)） |
| `ability_manager_stub.h/.cpp`（:4704 后） | 两个 Inner 实现 + 分发表 :314 后追加两分支 |
| `insight_intent_execute_callback_proxy.cpp` | `OnCancelDone`：写 key+resultCode，`ON_INSIGHT_INTENT_CANCEL_DONE` |
| `insight_intent_execute_callback_stub.cpp` | `OnCancelDone` 分发（新消息码 2） |
| `ability_scheduler_proxy.h/.cpp`（:1131 后） | `OnExecuteIntentCancel(uint64_t)`：TF_ASYNC + WriteUint64，失败仅日志 |
| `ability_scheduler_stub.h/.cpp`（:717 后） | `OnExecuteIntentCancelInner`；分发加在 `OnRemoteRequestInnerThird` 的 `SCHEDULE_ONEXECUTE_INTENT` case（:151）后 |

### 6.4 应用侧调度（AbilityThread 通用实现）

**`interfaces/kits/native/ability/native/ability_thread.h`（:347 后）+ `frameworks/native/ability/native/ability_thread.cpp`（:265 附近）**

```cpp
void OnExecuteIntentCancel(uint64_t intentId) override;          // IPC 入口
protected:
virtual void HandleCancelInsightIntent(uint64_t intentId);       // 通用实现，UIAbility/Extension 共用
```

- `OnExecuteIntentCancel`：校验 `abilityHandler_` → wptr + `PostTask(task, "AbilityThread:OnExecuteIntentCancel")`（必须切到 JS/ETS 亲和线程，对齐 :670 写法）。
- `HandleCancelInsightIntent`（基类实现，UIAbilityThread / ExtensionAbilityThread 默认继承）：
  1. 构造 `InsightIntentExecutorAsyncCallback`，Push lambda：`AbilityManagerClient::ExecuteInsightIntentDone(token_, intentId, result{innerErr = 取消码})` —— 无条件上报（规避 §4.8），不复用 `UIAbilityImpl::ExecuteInsightIntentDone`（:261，含 isNeedDelayResult 短路与生命周期回跳）；
  2. `InsightIntentExecutorMgr::CancelInsightIntent(intentId, std::move(callback))`，按返回值分流：
     - `ERR_OK`（桥接管，等 onCancel settle 后由 lambda 上报）→ 什么都不做；
     - `INTENT_NOT_EXIST`（执行器不在 = 已完成/未注册）→ **不上报**，自然完成流程会经 AMS 侧"普通结果码分支"settle cancel；
     - `ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED`（执行器在但未实现 onCancel）→ `AbilityManagerClient::NotifyIntentCancelResult(intentId, ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED)`（新 IPC 6182）。
- 链接说明：`ability_thread` 经 `uiabilitykit_native → insight_intent_executor` 间接可用；若链接告警在其 BUILD.gn deps 显式补 `insight_intent_executor`。
- Extension 侧如需通知扩展 UI，预留 `Extension::HandleCancelInsightIntent` 虚函数扩展点（基类空实现，本期不接）。

### 6.5 执行器层

**基类与管理器**

| 文件 | 修改 |
|---|---|
| `interfaces/kits/native/ability/native/insight_intent_executor/insight_intent_executor.h`（:69 后） | `virtual bool HandleCancelIntent(std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) { return false; }` —— false = 不支持且**不接管所有权**（mgr 释放）；true = 桥接管，负责最终 `callback->Call(...)` |
| `.../insight_intent_executor_mgr.h` + `.cpp`（`ExecuteInsightIntent` :33-92 后） | `int32_t CancelInsightIntent(uint64_t intentId, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);`：null callback→`ERR_INVALID_VALUE`；锁内 `find + erase` 原子取出（模式同 `RemoveInsightIntentExecutor` :102-107，防重复取消）；找不到→`TriggerCallbackInner(std::move(callback), INTENT_NOT_EXIST)` 且返回 `INTENT_NOT_EXIST`（调用方按 §6.4 分流：不上报）；`executor->HandleCancelIntent` false→日志 + 返回 `ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED`。`TriggerCallbackInner`（:109-119）无需改动，传取消码合法 |

**JS 桥（ArkJS）**

- `frameworks/native/ability/native/insight_intent_executor/js_insight_intent_executor.h/.cpp`：`HandleCancelIntent` override：
  1. 状态检查：`STATE_PATTERN_NAIVE_ACCEPT(State::EXECUTING, false)`；
  2. 探测 onCancel：`GetPropertyValueByPropertyName(env, jsObj_->GetNapiValue(), "onCancel", napi_function) == nullptr` → return false。**不要用 `napi_has_named_property`**（需排除非函数属性）；
  3. `CallJsFunctionWithResultInner("onCancel", 0, nullptr)`（:245）；返回 null → 不接管；
  4. Promise 分支复用 `HandlePromiseResult`（:430-464）的 then/catch 结构，settle 回调走新增 `static void ReplyCancelled(...)`：对齐 `ReplySucceeded`（:352-365），强制 `innerErr = INSIGHT_INTENT_ERR_CANCELED` 后 `callback->Call(*resultCpp); delete callback;`；resolve 与 reject 同路径（onCancel 失败不恢复执行）；非 Promise 同步返回同路径。**onCancel 返回 void，无需 `GetResultFromJs` 解析结果结构。**
- **所有权规则**：cancel 必须用传入的 callback，**禁止复用 `callback_`**（execute 路径已在 `HandlePromiseResult` :430 被 release 给 JS 闭包）。三约定：mgr→桥 unique_ptr 转移；桥返回 false 不接管（mgr 释放）；桥接管后以 delete 收尾。
- `frameworks/native/ability/native/insight_intent_executor/js_insight_intent_entry.cpp`（onExecute 调用点 :322 已核实）：同法处理 onCancel（Entry 无参数）。

**ETS 桥（ArkTS/ANI）**

- `frameworks/native/ability/native/insight_intent_executor/ets_insight_intent_executor.h/.cpp`：`HandleCancelIntent` override：
  1. **探测用属性访问语义，不用 `Class_FindMethod`**：`etsObj_->aniCls` 被 `Init()`（:101-106）固定为基类 class，且开发者实现 onCancel 有两种合法写法——方法语法 `onCancel() {}`（原型方法）与属性赋值 `onCancel = () => {}`（实例字段），`Class_FindMethod` 只覆盖前者。应复用 entry 桥已验证的 `HasDeclaredProperty` 模式（ets_insight_intent_entry.cpp:236-258）：`Object_GetPropertyByName_Ref(etsObj_->aniObj, "onCancel")` + `Reference_IsUndefined`——属性访问走对象实际类型链，两种形态通吃，基类可选声明未实现时为 undefined。建议将该函数从 entry 提炼到公共 utils 供两个桥复用；
  2. **调用双路径**：探测命中后优先 `env->Object_GetClass(etsObj_->aniObj, &cls)` + `Class_FindMethod(cls, "onCancel")` + `Object_CallMethod`（方法形态，保 `this`）；方法句柄找不到但探测已命中（属性赋值形态）时，对取出的函数对象走 ANI 函数调用（箭头函数字段为词法 `this`，无需绑定）；
  3. promise 判别不需要 isAsync 布尔字段协议（那是 execute 的同步/异步判别机制，:362-367），onCancel 返回 void，直接 `checkPromise`/`callPromise`（照抄 entry，ets_insight_intent_entry.cpp:308-338）；
  4. settle（resolve/reject 均可）后构造 `InsightIntentExecuteResult` 强制 `innerErr = INSIGHT_INTENT_ERR_CANCELED` 经 `ReplySucceeded`/`ReplyFailed` 上报（ets_insight_intent_utils.h:39/:42）——无需 `GetResultFromEts` 解析。
- `frameworks/native/ability/native/insight_intent_executor/ets_insight_intent_entry.cpp`：新增 `HandleCancelIntent` + 私有 `bool CancelInsightIntent(ani_env *env, bool &isAsync);`，用 `HasDeclaredProperty`（:236-258）探测；native 方法表（:127-130）无需新增，复用 `nativeOnExecuteResult/nativeOnExecuteError`。

**JS/ETS 声明（开发者可见接口，注意：无默认实现）**

onCancel 的三层位置一览：

| 层 | 文件 | 内容 |
|---|---|---|
| 声明（本次新增） | `frameworks/ets/ets/@ohos.app.ability.InsightIntentExecutor.ets`（:118 后） | `onCancel?: () => void \| Promise<void>;` 仅可选声明、**无函数体** |
| 声明（本次新增） | `frameworks/ets/ets/@ohos.app.ability.InsightIntentEntryExecutor.ets`（:49 后） | 同上 |
| 声明 | `frameworks/js/napi/insight_intent/insight_intent_executor/insight_intent_executor.js` 及 entry js | **不加**（JS duck-typing，基类不声明才能保持"未实现 = 探测不到"） |
| 实现 | 开发者应用子类（hap 内 srcEntry 指向文件） | 运行时才存在；方法语法或属性赋值均可（探测须双兼容，见 ETS 桥要点） |
| 探测/调用 | `ets_insight_intent_executor.cpp` / `js_insight_intent_executor.cpp` / 两个 entry 桥的 `HandleCancelIntent` | 按名字反射；ETS 侧属性访问探测 + 双路径调用 |

注意：`.ets` 基类是系统模块（桥内 `FindClass("@ohos.app.ability.InsightIntentExecutor…")` 编译后即此类），改动属系统能力变更，SDK d.ts 由它生成，需同步评审 API 声明。

| 文件 | 新增 |
|---|---|
| `frameworks/ets/ets/@ohos.app.ability.InsightIntentExecutor.ets`（:118 后） | `onCancel?: () => void \| Promise<void>;`（**仅可选声明，不给函数体**——给了默认体则"未实现"不可判定，见 §4.5） |
| `frameworks/ets/ets/@ohos.app.ability.InsightIntentEntryExecutor.ets`（:49 后） | `onCancel?: () => void \| Promise<void>;` |
| `frameworks/js/napi/insight_intent/insight_intent_executor/insight_intent_executor.js` | 不加基类方法（保持未实现 = 探测不到）；如需文档可加注释 |
| `frameworks/js/napi/insight_intent/insight_intent_entry_executor/insight_intent_entry_executor.js` | 同上 |

### 6.6 调用方入口（driver.cancel，回执闭环版）

**客户端映射扩展（NAPI/ANI 共用底层）**

- `frameworks/native/ability/native/insight_intent_host_client.h/.cpp`：`JsInsightIntentExecuteCallbackClient`（或其 C++ 基类）增加 `toolCallId` 成员（`OnExecute` 注册时从 param 取得）；`InsightIntentHostClient` 增加 `FindKeyByToolCallId(toolCallId, uint64_t &key)`（遍历现存映射，OnExecuteDone 后随映射删除）；新增 `AddCancelWaiter(key, defer)` / `OnCancelDone(key, resultCode)` 分发（内部：取 waiter → settle → 删除）。

**NAPI（ArkJS）** —— `frameworks/js/napi/insight_intent/insight_intent_driver/js_insight_intent_driver.cpp`

- 新增 `Cancel`/`OnCancel`（:242 `OnExecute` 后）：
  1. `argc < 1` → TooFew；toolCallId 转换失败/空 → `ThrowInvalidParamError`（401）；
  2. `InsightIntentHostClient::FindKeyByToolCallId` 找不到 → 直接 Reject `INTENT_NOT_EXIST`（2097329）；
  3. 创建 promise defer，`AddCancelWaiter(key, defer)`；
  4. 同步调 `AbilityManagerClient::CancelExecuteIntent(key, toolCallId)`；非 OK → 注销 waiter 并 Reject（映射码见 §6.7）；OK → 返回 pending promise，等 hostClient `OnCancelDone` 回调线程 settle（resolve 或按 resultCode reject）；
  5. `JsInsightIntentExecuteCallbackClient` 增加 `ProcessCancelDone(key, resultCode)`（对齐 :66-88 `ProcessInsightIntentExecute` 的错误码转换，经 `GetJsErrorCodeByNativeError` 映射，2097406 → 16000140）。
- `JsInsightIntentDriverInit`（:649 后）：`BindNativeFunction(env, exportObj, "cancel", ...)`。

**ETS 声明** —— `frameworks/ets/ets/@ohos.app.ability.insightIntentDriver.ets`（:238-252 声明区）

```ts
export native function nativeCancelCheck(toolCallId: string): void;
export native function nativeCancelSync(toolCallId: string, callback: AsyncCallbackWrapper<void>, isCallback: boolean): void;
export function cancel(toolCallId: string, callback: AsyncCallback<void>): void;
export function cancel(toolCallId: string): Promise<void>;
```

双版本写法照抄 `execute`（:254-278）：先 `nativeCancelCheck`，再 `taskpool.execute` 调 `nativeCancelSync`。

**ANI（ArkTS native）** —— `frameworks/ets/ani/insight_intent/insight_intent_driver/src/ets_insight_intent_driver.cpp`

- `OnCancelCheck`：空/非串 → `ThrowInvalidParamError`（模式对齐 :234-257）。
- `OnCancelSync`：**与 OnExecute（:129-186）同构的异步机制**（回执闭环后不再是同步返回）：GlobalReference 持 callback + `ani_vm` 取 env，注册 cancel waiter；`AbilityManagerClient::CancelExecuteIntent(key, toolCallId)` 非 OK 时即刻以 `CreateErrorByNativeErr` 回调并返回；OK 时等 hostClient `OnCancelDone` 回到 JS 线程后以 `CreateError(ERROR_OK)` / `CreateErrorByNativeErr(resultCode)` 完成回调。`EtsErrorUtil::CreateErrorByNativeErr`（ets_error_utils.cpp:181-191）已覆盖 2097406 → JS 码映射。
- `EtsInsightIntentDriverInit` 的 kitFunctions（:566-589）追加 `nativeCancelCheck` / `nativeCancelSync` 注册。

### 6.7 JS 可见错误码总表（cancel()）

| 场景 | native 码 | JS 码 |
|---|---|---|
| toolCallId 空/非法 | `ERR_INVALID_VALUE` | 401 |
| 无在途执行（含窄窗口竞态） | `INTENT_NOT_EXIST` 2097329 | 内部错误系 |
| 已结束不可取消 | `INTENT_STATE_NOT_EXECUTING` 2097330 | 内部错误系 |
| 非原调用方 | `CHECK_PERMISSION_FAILED` | 权限错误系 |
| 目标未就绪 / 路由失败 | `ERR_TARGET_NOT_STARTED` 2097333 / `ERR_INVALID_VALUE` | 内部错误系（经回执通道） |
| **目标未实现 onCancel** | `ERR_INSIGHT_INTENT_CANCEL_NOT_SUPPORTED` **2097406** | **16000140** |
| 回执超时 / 异常 | `INNER_ERR` | 内部错误系 |
| 取消成功（execute Promise 侧） | `ERR_INSIGHT_INTENT_CANCELED` 2097405 | **16000139** |

新增 `AbilityErrorCode` 后需同步 C API 映射表并跑 `ability_business_error_test`。

## 7. 测试方案

| 测试目标 | 用例 |
|---|---|
| `test/unittest/insight_intent/insight_intent_execute_manager_test/` | `CancelIntent_0100~0600`：空参 / 未注册 / state=EXECUTE_DONE / tokenId 不匹配 / key 不匹配 / 命中返回 intentId 且 pendingCancelKey 已挂；重复取消 → INNER_ERR；`ConsumeCancelKey` 只消费一次；`ExecuteIntentDone` 三分支（取消码+回执 / 自然码+pendingCancelKey 回执 NOT_EXECUTING / 无 pending）；Bind/Unbind 生命周期与 256 淘汰清理。`insight_intent_utils_mock.cpp` 视 `IPCSkeleton::GetCallingTokenID` 依赖补 mock |
| `test/unittest/insight_intent/insight_intent_executor_test/` | `CancelInsightIntent` 返回值三分支（OK 接管 / INTENT_NOT_EXIST / NOT_SUPPORTED）；`TriggerCallbackInner` 携带取消码；`HandleCancelIntent` 默认 false；桥探测（有/无 onCancel）→ 回报分流是否正确。`mock/mock_js_runtime.cpp` 按需补 `napi_call_function`/`napi_get_named_property` 语义 |
| `test/unittest/insight_intent/ets_insight_intent_tool_call_id_test/`（mock_ani_env.h 模式） | `EtsInsightIntentCancelCheckTest` / `EtsInsightIntentCancelSyncTest`：空串、非字符串、即刻失败回调、成功回执回调路径 |
| `test/unittest/ability_manager_stub_test` 等 | `EXECUTE_INTENT_CANCEL(76)`、`NOTIFY_INTENT_CANCEL_RESULT(6182)` 分发与 proxy 序列化；`insight_intent_execute_callback_stub` 的 `ON_INSIGHT_INTENT_CANCEL_DONE(2)` 分发 |
| `test/unittest/host_client 相关` | `FindKeyByToolCallId` 命中/未命中/OnExecuteDone 后删除；cancel waiter 注入与 settle |
| `test/fuzztest/abilitymgrinsightintentexecutemanager_fuzzer/` | 接口列表追加 `CancelIntent`、`ConsumeCancelKey` |

UT 用 oh-ut-generator / ohos-test-ut-generation 技能产出，遵循 HWTEST 规范与 75% 覆盖率要求。

## 8. 实施顺序与注意事项

顺序：① 常量/错误码 → ② inner_api（含回调接口 OnCancelDone）→ ③ 服务端（②③一体）→ ④ 应用侧调度 → ⑤ 执行器层 → ⑥ 入口 → ⑦ 测试。

注意事项：
- **IPC 版本互操作**：新 proxy 调旧 service 得 `ERR_CODE_NOT_EXIST`，可接受（同仓同发版）。
- **锁纪律**：AMS `CancelIntent` 锁内只查表置 pendingCancelKey；回执（ExecuteIntentDone / NotifyIntentCancelResult / 超时）统一经 `ConsumeCancelKey` 在锁内消费，**只消费一次**；四类回执事件先到先得。
- **回执时序**：`ExecuteIntentDone` 中 `OnCancelDone` 必须在清空 `callerToken` 之前发送。
- **回调所有权三规则**：mgr→桥 unique_ptr 转移；桥返回 false 不接管（mgr 释放）；桥接管后以 delete 收尾。禁止桥内复用 `callback_`。
- **基类禁给默认 onCancel 实现**（§4.5）：这是"未实现可判定"的前提，评审/code review 需作为硬性检查点。
- `AbilityRecord::Init` 绑定幂等、失败不传播。
- 新增 IPC 码位置纪律：`76` 插 kit 簇空位；`6182` 尾部顺序追加；scheduler 枚举尾部追加。

## 9. 范围外与后续项

| 项 | 理由 |
|---|---|
| CJ（Cangjie）执行器 onCancel | CJ 走独立 `CJInsightIntentExecutorMgr` 与独立 so，反向链接会成依赖环；FFI 签名形态差异大。服务端改动语言无关，CJ 应用收到 cancel 会走"未实现"分支返回 16000140，无行为劣化 |
| 分布式意图取消（isDistributed） | 需跨设备协议扩展，本期直接拒绝 |
| 系统侧/CLI 按 toolCallId 取消入口 | 通道可复用；其调用方与执行调用方不同进程，需另行设计授权与回执路由，本期不接。首个驱动场景可参考 Apple：执行超时自动取消（`IntentCancellationReason.timeout` 语义），系统检测超时后走同一取消通道并区分取消原因 |
| onCancel 扩展取消原因参数 | Apple 的 onCancel 携带 `IntentCancellationReason`（userCancelled/timeout）；本设计 onCancel 无参（当前 reason 恒为调用方取消），接口预留扩展为 `onCancel?(reason)` 的空间，待系统侧取消入口落地时一并评估 |
| 意图执行进度上报 | Apple 的 `ProgressReportingIntent` 是超时取消的判定基础（不报进度 30s 即取消）；本设计无进度概念，若引入超时自动取消可一并评估 |
| 「未实现 onCancel」的静态声明式方案（profile 标记 supportCancel） | 声明与实现存在漂移风险，且需 RDB/解析层改动；运行时探测已覆盖 |
| LiteParam 直连执行路径的取消 | 未走 key 登记，无回执载体；如需要另行设计 |

## 10. 验证

```bash
# 编译（仓库根）
./build.sh --product-name <product> --build-target abilityms
./build.sh --product-name <product> --build-target ability_runtime

# TDD 构建与运行
./build.sh --product-name <product> --build-target ability_runtime_test
# 上板后
run -t UT -ts insight_intent_execute_manager_test
run -t UT -ts insight_intent_executor_test
run -t UT -ts ets_insight_intent_tool_call_id_test
run -t UT -tp ability_runtime   # 全量回归
```

端到端手工链路（有设备时）：
1. **取消成功**：execute（带 toolCallId）发起 ServiceExtension 长任务 → cancel(toolCallId)：目标 onCancel 被回调 → cancel() resolve，execute Promise reject 16000139；
2. **未实现 onCancel**：目标执行器不实现 onCancel → cancel() reject **16000140**，原执行自然完成、execute Promise 以真实结果 settle；
3. **重复/迟到取消**：取消完成后再次 cancel 同一 toolCallId → reject（2097330 或 INNER_ERR）；
4. **授权**：另一进程 cancel 同 toolCallId → CHECK_PERMISSION_FAILED，原执行不受影响；
5. **竞态**：cancel 挂起期间执行自然完成 → cancel() 以"已结束"语义 settle，execute 结果正常送达，仅一次收尾。

## 11. 竞品对照分析：Apple AppIntents 取消机制

> 依据 Apple 官方文档逐页核实（CancellableIntent / IntentCancellationReason / LongRunningIntent / AppIntent 协议页），检索时间 2026-09。

### 11.1 Apple 的取消模型概览

**（1）取消能力是独立的 opt-in 协议，不在协议基座上。** `AppIntent` 协议本身（成员：`perform` / `systemContext` / `requestConfirmation` / `donate` 等）没有任何取消钩子。取消能力由独立协议 `CancellableIntent`（iOS 26.4+）提供：

```swift
protocol CancellableIntent : AppIntent
// "An interface to support the graceful cancellation of your app intent's task."
```

**（2）取消钩子：handler 包裹而非方法覆写。** 开发者在 `perform()` 内用 `withIntentCancellationHandler(operation:onCancel:isolation:)` 包裹任务体，`onCancel: (IntentCancellationReason) -> Void` 接收取消原因。不需要原因时可直接用 Swift 标准库 `withTaskCancellationHandler`。

**（3）系统侧取消触发仅两种**（官方列举）：
- 意图未上报进度且超过 **30 秒运行时限**——系统自动取消（`timeout`）；
- 用户在 **Siri / Live Activities / Shortcuts** 中监控进度时显式取消（`userCancelled`）。

`IntentCancellationReason` 仅两个值：`.timeout`（"exceeded the allowed time limit without reporting progress"）与 `.userCancelled`（"someone explicitly canceled the intent"）。

**（4）清理宽限是采纳协议的对价。** 官方原文："the system gives your code extra time to handle cancellation before the app intent's task ends"；同时 Important 提示清理必须快——非 macOS 平台取消后进程随时可能被挂起，**不采纳协议的意图连这个宽限都没有**（等价于系统硬超时）。

**（5）长任务配套：`LongRunningIntent`（iOS 27+）**，继承 `ProgressReportingIntent`：
- 后台任务默认仅 30 秒，经 `performBackgroundTask(options:operation:onCancel:)` 延长；
- **必须定期更新 `progress` 属性，否则系统取消后台运行时扩展**——进度上报既是保活也是超时取消的判定依据；
- 任务体内配 `try Task.checkCancellation()` 做协作式中止；macOS 无时限。

**（6）底层是 Swift 协作式取消**：`withTaskCancellationHandler` + `Task.isCancelled` / `Task.checkCancellation()` + `CancellationError`——系统只发出取消信号并给宽限，从不抢占正在执行的代码。

官方示例（支付意图）的关键形态：`perform()` 中每步 `updateProgress(...)` 防超时，`onCancel` 按 reason 分流——`.timeout` → 回滚事务，`.userCancelled` → 取消事务；onCancel 内的清理动作官方建议："stop the current task, release resources, save intermediate state, write out diagnostic information"。

### 11.2 逐项对照

| 设计点 | Apple AppIntents | 本设计 | 结论 |
|---|---|---|---|
| 取消模型 | 协作式（task cancellation，系统不抢占） | 协作式（onCancel 回调，框架不强杀 JS Promise） | 一致；业界共识 |
| 能力声明 | `CancellableIntent` 协议 conform（编译期、类型级） | 基类可选声明 `onCancel?` + 运行时探测 | 同构；Apple 佐证"支持取消是声明式能力"。本设计运行时探测更严格，能发现"声明了但未实现" |
| 未声明能力的后果 | 无清理宽限，系统超时硬掐（进程挂起） | cancel() reject 16000140，原执行不受影响 | 同为"不支持"语义；Apple 更激进（资源管控视角），本设计更温和（调用方视角） |
| onCancel 参数 | `(IntentCancellationReason)` 带 reason | 无参 | 当前场景 reason 恒为"调用方取消"，无参成立；reason 已列为预留扩展（§9） |
| 取消发起方 | 系统表面（Siri/Shortcuts/Live Activities 停止按钮）+ 系统超时 | 调用方进程（driver.cancel + toolCallId） | 场景差异：iOS 无跨进程第三方调用方，因此**没有** cancel 结果回执的 API 需求 |
| 取消结果回执 | 无（发起方是用户界面，任务结束即感知） | 错误码回执闭环（OnCancelDone / 6182 / 16000140） | 本设计特有需求，回执闭环不可省略 |
| 超时自动取消 | 有：30s 未报进度 → 系统自动取消 | 无（仅有 cancel 回执超时 10s，语义不同） | Apple 已验证必要性，列为后续项（§9），届时走同一取消通道 |
| 进度上报 | `ProgressReportingIntent` 是超时取消的判定基础 | 无进度概念 | 若引入超时自动取消可一并评估（§9） |

### 11.3 结论与借鉴点

1. **本设计的两个关键决策获得印证**：取消必须协作式（Apple 同样无法抢占）；"支持取消 = 显式声明的能力，未声明即不支持"（Apple 用协议 conform，本设计用可选声明 + 运行时探测，语义一致）。
2. **错误码回执闭环是本场景的必要补充而非过度设计**：Apple 不需要它是因为取消发起方是系统 UI 本身；本设计的取消发起方是跨进程调用方（agent/CLI 工具调用场景），必须知道取消是否真正生效（16000139 vs 16000140）。
3. **后续演进方向**（已并入 §9）：超时自动取消（系统侧取消入口的首个驱动场景，区分 timeout/userCancelled 两种原因）、onCancel 预留 reason 参数、进度上报基础设施。

### 11.4 参考来源

- [CancellableIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/cancellableintent)（iOS 26.4+）
- [IntentCancellationReason — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/intentcancellationreason)
- [LongRunningIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/longrunningintent)（iOS 27+）
- [AppIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/appintent)
- [Discover new capabilities in the App Intents framework — WWDC26 Session 345](https://developer.apple.com/videos/play/wwdc2026/345)
- [CancellationError — Swift Documentation](https://developer.apple.com/documentation/swift/cancellationerror)
- [Explore structured concurrency in Swift — WWDC21](https://developer.apple.com/videos/play/wwdc2021/10132/)

## 12. 附录：意图提供方定义示例（含取消，目标形态）

> 本节为本设计落地后的开发者使用形态，onCancel / driver.cancel 为本设计新增接口（标注 ★）；其余接口为现状已有能力。profile 格式取自仓内测试样例（`test/unittest/insight_intent_profile_test`、`test/unittest/extract_insight_intent_profile_test`）。

### 12.1 方式一：配置式 — profile 声明 + `InsightIntentExecutor` 子类

适合 ServiceExtension 后台长任务（AI 推理、文件处理等）。

**① 声明意图**（`entry/src/main/resources/base/profile/insight_intent.json`）：

```json
{
  "insightIntents": [
    {
      "intentName": "ChatSummary",
      "domain": "ai",
      "intentVersion": "1.0.0",
      "srcEntry": "ets/executables/ChatSummaryExecutor",
      "serviceExtension": {
        "ability": "ChatSummaryExtAbility"
      }
    }
  ]
}
```

`module.json5` 中正常声明 `ChatSummaryExtAbility`（ServiceExtension）。意图安装后由 BMS 事件（`InsightIntentEventMgr`）自动解析入库，无需额外注册。

**② 实现执行器**（`entry/src/main/ets/executables/ChatSummaryExecutor.ets`，即 profile 的 srcEntry）：

```typescript
import InsightIntentExecutor from '@ohos.app.ability.InsightIntentExecutor';
import insightIntent from '@ohos.app.ability.insightIntent';

export default class ChatSummaryExecutor extends InsightIntentExecutor {

  // 长任务入口：ServiceExtension 后台模式（want 携带 intent 参数时由框架反射调用 callOnExecuteInServiceExtensionAbility）
  async onExecuteInServiceExtensionAbility(name: string, param: Record<string, Object>):
    Promise<insightIntent.ExecuteResult> {
    // this.context 由框架注入：instanceId / toolCallId / startAbility 等
    console.info(`intent ${name}, toolCallId: ${this.context.toolCallId}`);

    const summary = await this.doSummarize(param.text);   // 长耗时任务
    return { code: 0, result: { summary } };
  }

  // ★ onCancel —— 可选实现；不实现则该意图不可被取消（调用方 cancel() 收到 16000140）
  onCancel(): void | Promise<void> {
    this.abortSummarize();     // 中止推理请求、释放资源；同步返回或返回清理 Promise
  }

  private async doSummarize(text: string): Promise<string> {
    this.task = startRemoteInference(text);   // 保存任务句柄供中止
    return this.task.result;
  }

  private abortSummarize(): void {
    this.task?.abort();
  }
}
```

关键约定：**onCancel 只做中止动作**（abort 请求、释放资源），不返回业务结果——onCancel settle（Promise 完成 or 同步返回）即触发框架向调用方上报"已取消"（16000139）；此后 `onExecute` 即使完成，结果也会被丢弃（执行器已从 ExecutorMgr 移除）。

### 12.2 方式二：装饰器式 — `@InsightIntentEntry` + `InsightIntentEntryExecutor` 子类

适合轻量入口意图（参数在装饰器中声明，编译期生成 profile，无需手写 JSON）：

```typescript
import { IntentResult } from '@ohos.app.ability.insightIntent';
import { InsightIntentEntryExecutor } from '@ohos.app.ability.InsightIntentEntryExecutor';
import { InsightIntentEntry } from '@kit.IntentsKit';   // 装饰器来源以 SDK 实际导出为准

@InsightIntentEntry({
  intentName: 'StartMusicPlayer',
  displayName: '音乐播放器',
  abilityName: 'MusicAbility',
  executeMode: ['foreground', 'background']
})
export default class StartMusicPlayer extends InsightIntentEntryExecutor<void> {

  async onExecute(): Promise<IntentResult<void>> {
    await startPlay();
    return { code: 0 };
  }

  // ★ onCancel —— 可选实现，语义同 12.1
  onCancel(): void | Promise<void> {
    stopPlay();
  }
}
```

### 12.3 调用方视角：取消的完整闭环

```typescript
import { insightIntentDriver } from '@kit.AbilityKit';
import { BusinessError } from '@ohos.base';

const toolCallId = 'call-20260927-001';

// 发起：execute 与 cancel 用 toolCallId 关联
const executePromise = insightIntentDriver.execute({
  bundleName: 'com.example.ai',
  moduleName: 'entry',
  abilityName: 'ChatSummaryExtAbility',
  insightIntentName: 'ChatSummary',
  insightIntentParam: { text: '总结这段会议纪要……' },
  executeMode: insightIntent.ExecuteMode.SERVICE_EXTENSION_ABILITY,
  toolCallId,                       // ★ 取消关联键
});

// 中途取消：
try {
  await insightIntentDriver.cancel(toolCallId);   // 目标 onCancel settle 后才 resolve
  console.info('canceled');                       // 同时 executePromise 以 16000139 reject
} catch (err) {
  const e = err as BusinessError;
  if (e.code === 16000140) {
    // 目标未实现 onCancel：取消不被支持，原执行自然完成
  } else if (e.code === 2097330) {
    // 该执行已结束，无需取消
  }
}
```

### 12.4 开发者视角的取消时序

```
execute(toolCallId) ──▶ 目标 onExecute 开始（长任务）
cancel(toolCallId)  ──▶ AMS 校验/挂起 ──▶ 桥探测 onCancel
                                            ├─ 未实现 ──▶ cancel() reject 16000140，执行继续
                                            └─ 已实现 ──▶ onCancel() 被调（中止业务）
                                                            │ settle
                                                            ▼
                                    cancel() resolve；execute 以 16000139 reject；
                                    onExecute 后续结果被丢弃
```
