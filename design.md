# InsightIntent 执行取消（Cancel）能力设计

> 状态：设计定稿（2026-09-27 评审收敛）
> 已拍板决策：① 取消关联键 = `toolCallId`（§13.6）；② 回执语义 = **D1 fire-and-forget**（cancel() 受理即应答，结局由 execute 结果揭示，见 §4.2）；③ onCancel 返回 `void | Promise<void>`、可选声明无默认体。
> 范围仓库：foundation/ability/ability_runtime
> 关联背景：意图框架（InsightIntent）执行链路目前仅支持单向执行 + 结果回传，长耗时意图（ServiceExtension 后台任务、AI agent 工具调用）无法被调用方中途取消。

## 1. 背景与目标

现状执行链路：调用方通过 `insightIntentDriver.execute()` 发起意图执行 → AMS 分派 → 目标应用内执行器（`InsightIntentExecutor` / `InsightIntentEntryExecutor`）通过 `onExecute` 系列方法处理 → 结果经 AMS 回传调用方 Promise。**调用方没有任何手段中止一次在途执行。**

本设计为意图执行增加取消能力：

- 调用方新增 `insightIntentDriver.cancel(toolCallId)` 入口（`toolCallId` 已是 `ExecuteParam` 既有字段，作为取消关联键，**不改动 execute 签名与返回形态**）；
- AMS 按 toolCallId 检索在途执行记录，校验发起者身份后路由到目标应用；
- 目标应用内执行器新增可选的 `onCancel` 接口（与 `onExecute` 并列），开发者在 onCancel 中实现业务中止；
- `cancel()` **受理即应答**（D1）；取消的真实结局由 execute 的 Promise 揭示——真正取消时 execute 以 `16000139` 收尾，未取消时 execute 以真实业务结果自然收尾。

## 2. 需求范围（已确认决策）

| 决策项 | 结论 |
|---|---|
| 取消发起方 | 调用方进程（driver.execute 的调用者）。系统侧/CLI 按 toolCallId 的取消入口不在本期，但 AMS 取消通道设计上可复用 |
| 关联键 | `toolCallId`（`InsightIntentExecuteParam::toolCallId_`，want key `ohos.insightIntent.toolCallId`）；仅覆盖经 driver.execute 的执行路径。**不传 toolCallId = 声明放弃该次执行的可取消性** |
| 执行器覆盖 | 全部执行模式（UIAbility 前台/后台、UIExtension、ServiceExtension）+ `InsightIntentEntryExecutor`（@InsightIntentEntry） |
| 不覆盖 | FUNC / PAGE / LINK 装饰器（方法直调 / UI 跳转，无法中断）；CJ（Cangjie）侧列为后续项 |
| 分布式意图 | 本期不支持取消（`isDistributed == true` 直接拒绝） |
| onCancel 返回类型 | `void \| Promise<void>`（评审结论，返回值无业务语义） |
| 未实现 onCancel | **fire-and-forget（D1）**：cancel() 受理即 resolve（无 16000140 反馈）；执行自然完成，调用方从 execute 的真实结果得知未取消 |
| 回执语义 | **D1**（评审结论）：不建回执闭环（无 6182 / OnCancelDone / pendingCancelKey / 超时兜底）；如未来需要确认语义，按 §13.5 记录的 D2 设计增量回补 |

## 3. 端到端流程

```
调用方进程                          AMS                                目标应用进程
────────────                       ────────                           ────────────
driver.execute(param 含 toolCallId)
 ──EXECUTE_INTENT(同步IPC)──▶ records_[intentId] 登记
                              (新增字段: toolCallId / callerTokenId / targetToken)
                              冷启动: AbilityRecord::Init 时绑定 targetToken
                              已启动: AMS::OnExecuteIntent 时绑定 targetToken

driver.cancel(toolCallId)
 ──EXECUTE_INTENT_CANCEL=76──▶ CancelIntent（锁内查 records_）:
   (同步IPC, 受理即应答)        toolCallId 相等 && state==EXECUTING
                                && callerTokenId 匹配
                              ├─ 不满足 ──▶ 同步返回错误码
                              │            (2097329 / 2097330 / CHECK_PERMISSION_FAILED)
                              └─ 满足 ───▶ GetTargetToken(intentId)
                                           ▶ Token::GetAbilityRecordByToken
                                           ──OnExecuteIntentCancel(intentId) [TF_ASYNC]──▶
                                                                                           AbilityThread → PostTask
                                                                                           → ExecutorMgr::CancelInsightIntent
                                                                                             (锁内 erase 原子取出)
                                                                           ┌── 未实现 onCancel ──▶ 桥 false
cancel() 按同步结果 settle ◀── EXECUTE_INTENT_CANCEL reply ── ERR_OK        │                     （回插执行器，仅日志）
                                                                           └── 已实现 ──▶ onCancel() 被调
                                                                                           │ settle
                                                                    ExecuteInsightIntentDone(intentId, 取消码 2097405)
                              ◀──────────────────────── 现有完成通道 ──────────┘
 execute Promise reject(16000139) ◀── OnExecuteDone(key, 2097405)
```

说明：cancel() 与目标内部结局是**解耦**的——cancel() 的同步结果只反映"受理层"（在途/授权/目标就绪），目标应用内"是否真的取消了"由 execute 的最终结果揭示。

## 4. 核心语义设计

1. **取消码**（经现有 `ExecuteInsightIntentDone` 完成通道上报，不新开回传 IPC）：
   - native 层：`ERR_INSIGHT_INTENT_CANCELED = 2097405`；inner 层：`InsightIntentInnerErr::INSIGHT_INTENT_ERR_CANCELED`
   - JS 业务码：`AbilityErrorCode::ERROR_CODE_INTENT_CANCELED = 16000139`
2. **cancel() 语义（D1，受理即应答）**：
   - 同步错误（IPC reply 直接返回，cancel() 即刻 reject）：空/非法 toolCallId → 401；无匹配记录 → `INTENT_NOT_EXIST(2097329)`；已结束 → `INTENT_STATE_NOT_EXECUTING(2097330)`；非原调用方 → `CHECK_PERMISSION_FAILED`；目标未就绪（targetToken 未绑定/路由失败）→ `ERR_TARGET_NOT_STARTED(2097333)` / `ERR_INVALID_VALUE`；分布式 → `INNER_ERR`。
   - 受理成功 → cancel() **resolve**（语义 = "取消请求已送达目标"）。目标应用内的真实结局**不经 cancel() 反馈**：
     - 已实现 onCancel 且中止成功 → execute Promise 以 16000139 收尾；
     - 未实现 onCancel / 中止失败 → 执行自然完成，execute Promise 以**真实业务结果**收尾。
   - 重复 cancel 同一 toolCallId：无害幂等——受理层仍会通过（state 仍 EXECUTING），但目标侧执行器已在首次取消时移除，第二次路由落空（仅日志）。
3. **onCancel 契约**
   - 返回类型 `void | Promise<void>`（评审结论）：返回值无业务语义——只要 onCancel settle（resolve 或 reject），即视为取消完成，框架强制上报取消码；onCancel 内部出错**不恢复执行**。桥因此无需解析结果结构。
   - **基类不给默认实现**（关键）：`onCancel` 在基类仅作可选声明（`onCancel?: () => void | Promise<void>`），无函数体。若给空默认体，继承体系下"未实现"永远探测命中，"取消"会静默变成空操作却向调用方上报已取消（业务没停却丢弃其结果——撒谎语义）。
   - 未实现 ⇒ 桥探测不到（JS 用 `GetPropertyValueByPropertyName` 判函数类型；ETS 用属性访问探测，见 §6.5）⇒ 桥返回 false，**执行器回插、仅记日志**，执行自然完成。
4. **授权**：仅原调用方进程可取消。`AddRecord` 时用 `IPCSkeleton::GetCallingTokenID()` 捕获 callerTokenId（binder 线程上执行，含 DMS 转发路径），cancel 时比对 `toolCallId + callerTokenId + state` 三重匹配；不改 6 个 `CheckAndUpdateParam` 调用点签名。
5. **竞态（单点串行化）**：AMS 锁内的 `state == EXECUTING` 检查是"受理 vs 拒绝"的唯一判定点；应用侧 ExecutorMgr 锁内 erase 是"取消接管 vs 自然完成"的唯一判定点。两道闸门保证结果只送达一次：
   - cancel 先于完成：路由成功 → onCancel settle 后上报取消码 → state 置 EXECUTE_DONE → 此后自然完成的结果被丢弃；
   - 完成先于 cancel：state = EXECUTE_DONE → cancel 受理层返回 2097330；
   - 路由后、onCancel settle 前执行自然完成（窗口竞态）：先到者定局，后到者被 state 检查丢弃，调用方只收一次。
6. **窄窗口竞态（文档化即可）**：execute 的 IPC 尚未在 AMS 落记录时发起 cancel → 返回 2097329（INTENT_NOT_EXIST），调用方可重试；窗口为一次 IPC 往返。
7. **UIExtension 特别说明**（现状已核实）：`AMS::ExecuteIntentCommon` 对 `UI_EXTENSION_ABILITY` 直接返回 `ERR_INVALID_OPERATION`（当前不可达）⇒ AMS 路由表不会出现 UIExtension 目标；执行器层 UIExtension 支持仅作完备性保留。`js_ui_extension_base.cpp:546-549` 的完成上报有条件（`uris 非空 || interactionInfo != nullptr`）⇒ cancel 上报必须走独立回调、无条件上报。

## 5. 已核实的关键事实（实现前提，勿推翻）

1. `InsightIntentExecuteManager::ExecuteIntentDone`（services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:301）只置 `state = EXECUTE_DONE` 并清空 `callerToken`，**记录不删除**，由 256 条上限淘汰（:44/:278）⇒「已结束不可取消」判定条件是 `state != EXECUTING`。
2. `AbilityRecord::Init()`（services/abilitymgr/src/ability_record.cpp:194，`token_` 创建于 :223）被 UIAbility / Mission / BaseExtension 三条 CreateAbilityRecord 路径共同调用 ⇒ 在 Init 内做 intentId→targetToken 绑定，**一次覆盖三条冷启动路径**。
3. `InsightIntentExecuteConnection::OnAbilityConnectDone` 是空实现且全仓无业务调用点，**不能**用作 ServiceExtension 冷启动 hook。
4. 编号空闲已核实：IPC `EXECUTE_INTENT_CANCEL = 76`（74/75/77-81 均已占用，76 是该簇唯一空位）；`2097405`、JS 码 `16000139` 均空闲；调度器枚举尾部现为 `SCHEDULE_ABILITIES_REQUEST_DONE`，追加即可。
5. `js_insight_intent_entry.cpp:322` 以 `CallJsFunctionWithResultInner("onExecute", 0, nullptr)` 调 Entry 执行器，onCancel 采用相同调用形态。
6. 取消是**协作式**的：业务是否真正停止取决于开发者是否实现并响应 onCancel；框架不能强杀执行中的 JS Promise。这是"不提供默认 onCancel"的根本依据。
7. **执行器生命周期**：`InsightIntentExecutorMgr::insightIntentExecutors_` 值类型为 `std::shared_ptr<InsightIntentExecutor>` 且为执行器的**唯一长期持有者**（insight_intent_executor_mgr.h:46 已核实；`ExecuteInsightIntent` 的局部 shared_ptr 在函数返回后即消失）⇒ 执行器从 map erase 即析构 C++ 桥对象并释放 ETS 全局引用，ETS 侧 `nativeExecutor: long` 随之悬垂。因此 **cancel 未接管（未实现 onCancel）时必须把执行器回插 map**，否则 onExecute 自然完成时 JS 回程触发 use-after-free。
8. **与 Apple AppIntents 取消模型对照**：Apple 以独立协议 `CancellableIntent`（iOS 26.4+）声明取消能力，取消同为协作式，与本设计"可选声明 + 未实现 = 取消不支持"同构。详细对照与差异点解析见 §11。

## 6. 详细修改点

### 6.1 常量与错误码

| 文件 | 修改 |
|---|---|
| `interfaces/inner_api/ability_manager/include/ability_manager_errors.h`（:1023 后） | `ERR_INSIGHT_INTENT_CANCELED = 2097405`（"for insight intent execution canceled by caller"） |
| `interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_constant.h`（`InsightIntentInnerErr`） | 加 `INSIGHT_INTENT_ERR_CANCELED = AAFwk::ERR_INSIGHT_INTENT_CANCELED`（写法对齐现有条目） |
| `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（:327 后） | `ERROR_CODE_INTENT_CANCELED = 16000139` |
| `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.cpp` | 消息常量 `"The insight intent execution has been canceled."` + 外部码→消息表、native→JS 映射表各加一条（`ERR_INTENT_DEVICE_DISCONNECTED` 条目旁） |
| `frameworks/c/ability_runtime/src/ability_business_error_utils.cpp` | 同步 C API 映射表 |

复用既有错误码（无需新增）：`INTENT_NOT_EXIST=2097329`、`INTENT_STATE_NOT_EXECUTING=2097330`、`ERR_TARGET_NOT_STARTED=2097333`、`CHECK_PERMISSION_FAILED`、`ERR_INVALID_VALUE`、`INNER_ERR`。**不新增**"未实现 onCancel"错误码（D1 下该结局不经 cancel() 反馈）。

### 6.2 inner_api 接口层

| 文件 | 修改 |
|---|---|
| `ability_manager_ipc_interface_code.h` | `EXECUTE_INTENT_CANCEL = 76`，插在 `GET_FOREGROUND_UI_ABILITIES = 75` 与 `OPEN_ATOMIC_SERVICE = 77` 之间（勿动其他值） |
| `ability_manager_interface.h`（:2105 `ExecuteInsightIntentDone` 后） | `virtual int32_t CancelExecuteIntent(const std::string &toolCallId) { return 0; }`（带默认实现，模式对齐 :2051，不破坏其他实现者） |
| `ability_scheduler_interface.h`（:297 `OnExecuteIntent` 后） | `virtual void OnExecuteIntentCancel(uint64_t intentId) {}`（带空默认体，模式对齐 :325 `ExecuteSkill`；其余实现者只有测试 mock）；枚举尾部（:432 后）加 `SCHEDULE_ONEXECUTE_INTENT_CANCEL,` |
| `ability_manager_client.h/.cpp`（:2485 后） | `ErrCode CancelExecuteIntent(const std::string &toolCallId);`（实现对齐 :2154 `ExecuteIntent`：GetAbilityManager + CHECK_POINTER_RETURN_NOT_CONNECTED + 转发 proxy） |

### 6.3 AMS 服务端

**`services/abilitymgr/include/insight_intent/insight_intent_execute_manager.h`**

```cpp
// InsightIntentExecuteRecord（:39-49）新增字段
std::string toolCallId;                        // 调用方业务标识（可为空；取消关联键）
uint32_t callerTokenId = 0;                    // IPCSkeleton::GetCallingTokenID() 捕获
sptr<IRemoteObject> targetToken = nullptr;     // 目标 ability token

// InsightIntentExecuteManager 公有新增
int32_t CancelIntent(const std::string &toolCallId, uint32_t callerTokenId, uint64_t &intentId);
int32_t BindTargetToken(uint64_t intentId, const sptr<IRemoteObject> &targetToken);
int32_t GetTargetToken(uint64_t intentId, sptr<IRemoteObject> &targetToken) const;
void UnbindTargetToken(uint64_t intentId);

// 私有
// AddRecord（:136）尾参加 const std::string &toolCallId = "", uint32_t callerTokenId = 0
// 成员新增 std::map<uint64_t, sptr<IRemoteObject>> targetTokens_;（复用 mutex_）
```

**`services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp`**

- `AddRecord`（:256）：填充新字段；`callerTokenId` 用 `IPCSkeleton::GetCallingTokenID()` 函数内捕获；淘汰分支（:278-288）调 `UnbindTargetToken`。
- `CancelIntent`：`lock_guard<ffrt::mutex>` 下线性扫 `records_`（上限 256，不建反向 map），比对 `toolCallId` 非空相等 + `state == EXECUTING` + `callerTokenId` 匹配；命中返回 intentId。**锁内不得做 IPC**。不满足时错误码见 §4.2。
- `BindTargetToken / GetTargetToken / UnbindTargetToken`：mutex_ 下 map 读写（const 方法用 mutable mutex_，模式同 `GetBundleName` :398）；`ExecuteIntentDone` 与 `RemoveExecuteIntent`（:292）同步 `UnbindTargetToken`。
- `ExecuteIntentDone`（:301）**无需修改**：取消码与自然结果码走同一状态机（置 EXECUTE_DONE + 回调 caller），双报由 state 检查天然丢弃。

**`services/abilitymgr/src/ability_record.cpp`（冷启动目标 token 绑定，唯一漏斗）**

- `Init()`（:194）内 `token_` 创建（:223）后：want 含 `INSIGHT_INTENT_EXECUTE_PARAM_ID` 时解析 intentId 并 `BindTargetToken(intentId, token_->AsObject())`（幂等、失败不传播）。
- `RemoveInsightIntent()`：移除参数前解析 intentId 并 `UnbindTargetToken`（兜底清理）。

**`services/abilitymgr/src/ability_manager_service.cpp`（头文件 :2256 后声明）**

`CancelExecuteIntent(const std::string &toolCallId)` 实现放在 `ExecuteInsightIntentDone`（:15724）后：

1. toolCallId 空 → `ERR_INVALID_VALUE`；
2. `CancelIntent(toolCallId, IPCSkeleton::GetCallingTokenID(), intentId)`，失败原样返回；
3. `GetTargetToken` 失败 → `ERR_TARGET_NOT_STARTED`（目标未就绪，同步返回）；
4. `Token::GetAbilityRecordByToken` → 判 `GetAbilityState() != TERMINATED && GetScheduler() != nullptr`（模式对齐 :15615），否则 `ERR_INVALID_VALUE`；
5. `scheduler->OnExecuteIntentCancel(intentId)`（TF_ASYNC 立即返回）；
6. 返回 `ERR_OK`（受理即应答）。

**Proxy / Stub**

| 文件 | 修改 |
|---|---|
| `ability_manager_proxy.h/.cpp`（:5752 后） | `CancelExecuteIntent`：WriteInterfaceToken → WriteString(toolCallId) → `SendRequest(EXECUTE_INTENT_CANCEL)` → ReadInt32 |
| `ability_manager_stub.h/.cpp`（:4704 后） | `CancelExecuteIntentInner`：读 string、空则 ERR_INVALID_VALUE、调 service、WriteInt32(result)；分发表 :314 后追加分支 |
| `ability_scheduler_proxy.h/.cpp`（:1131 后） | `OnExecuteIntentCancel(uint64_t)`：`MessageOption(MessageOption::TF_ASYNC)` + WriteUint64(intentId) + `SendTransactCmd(SCHEDULE_ONEXECUTE_INTENT_CANCEL)`，失败仅日志 |
| `ability_scheduler_stub.h/.cpp`（:717 后） | `OnExecuteIntentCancelInner`：ReadUint64 → `OnExecuteIntentCancel(intentId)` → NO_ERROR；分发加在 `OnRemoteRequestInnerThird` 的 `SCHEDULE_ONEXECUTE_INTENT` case（:151）后 |

### 6.4 应用侧调度（AbilityThread 通用实现）

**`interfaces/kits/native/ability/native/ability_thread.h`（:347 后）+ `frameworks/native/ability/native/ability_thread.cpp`（:265 附近）**

```cpp
void OnExecuteIntentCancel(uint64_t intentId) override;          // IPC 入口
protected:
virtual void HandleCancelInsightIntent(uint64_t intentId);       // 通用实现，UIAbility/Extension 共用
```

- `OnExecuteIntentCancel`：校验 `abilityHandler_` → wptr + `PostTask(task, "AbilityThread:OnExecuteIntentCancel")`（必须切到 JS/ETS 亲和线程，对齐 :670 写法）。
- `HandleCancelInsightIntent`（基类实现，UIAbilityThread / ExtensionAbilityThread 默认继承）：
  1. 构造 `InsightIntentExecutorAsyncCallback`，Push lambda：`AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token_, intentId, result{innerErr = 取消码})` —— `token_` 在 AbilityThread（ability_thread.h:383）。**无条件上报**（规避 §4.7），刻意不复用 `UIAbilityImpl::ExecuteInsightIntentDone`（:261，含 isNeedDelayResult 短路与生命周期回跳）；
  2. `InsightIntentExecutorMgr::GetInstance()->CancelInsightIntent(intentId, std::move(callback))`：
     - 返回 true（桥接管）→ 什么都不做，等 onCancel settle 后由 lambda 经现有 done 通道上报取消码；
     - 返回 false（无执行器 / 未实现 onCancel）→ 仅 `TAG_LOGW`，**不向 AMS 上报任何内容**（D1：结局由 execute 结果自然揭示），执行器已回插、原执行继续。
- 链接说明：`ability_thread` 经 `uiabilitykit_native → insight_intent_executor` 间接可用；若链接告警在其 BUILD.gn deps 显式补 `insight_intent_executor`。
- Extension 侧如需通知扩展 UI，预留 `Extension::HandleCancelInsightIntent` 虚函数扩展点（基类空实现，本期不接）。

### 6.5 执行器层

**基类与管理器**

| 文件 | 修改 |
|---|---|
| `interfaces/kits/native/ability/native/insight_intent_executor/insight_intent_executor.h`（:69 后） | `virtual bool HandleCancelIntent(std::unique_ptr<InsightIntentExecutorAsyncCallback> callback) { return false; }` —— false = 不支持且**不接管所有权**（mgr 释放）；true = 桥接管，负责最终 `callback->Call(...)` |
| `.../insight_intent_executor_mgr.h` + `.cpp`（`ExecuteInsightIntent` :33-92 后） | `bool CancelInsightIntent(uint64_t intentId, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);`：null callback→false；锁内 `find + erase` 原子取出（模式同 `RemoveInsightIntentExecutor` :102-107，防重复取消、与完成路径互斥）；找不到→消费 callback（`TriggerCallbackInner(std::move(callback), INTENT_NOT_EXIST)`，AMS 侧因记录已完成而丢弃，无害）并返回 false；**`executor->HandleCancelIntent` 返回 false（未实现 onCancel）→ 必须锁内回插 `insightIntentExecutors_[intentId] = executor`（§5.7：map 是唯一持有者，不回插则执行器析构、后续自然完成回程 UAF；回插覆盖同 id 条目无害）**，日志 + 返回 false。`TriggerCallbackInner`（:109-119）无需改动 |

**JS 桥（ArkJS）**

- `frameworks/native/ability/native/insight_intent_executor/js_insight_intent_executor.h/.cpp`：`HandleCancelIntent` override：
  1. 状态检查：`STATE_PATTERN_NAIVE_ACCEPT(State::EXECUTING, false)`；
  2. 探测 onCancel：`GetPropertyValueByPropertyName(env, jsObj_->GetNapiValue(), "onCancel", napi_function) == nullptr` → return false（原型链 duck-typing，方法语法与属性赋值两种实现写法均命中）。**不要用 `napi_has_named_property`**（需排除非函数属性）；
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

### 6.6 调用方入口（driver.cancel，D1 受理即应答）

**NAPI（ArkJS）** —— `frameworks/js/napi/insight_intent/insight_intent_driver/js_insight_intent_driver.cpp`

- 新增 `Cancel`/`OnCancel`（:242 `OnExecute` 后）：
  1. `argc < 1` → TooFew；toolCallId 转换失败/空 → `ThrowInvalidParamError`（401）；
  2. callback/Promise 双版本逐行对齐 `OnExecute` :259-274；
  3. 同步调 `AbilityManagerClient::GetInstance()->CancelExecuteIntent(toolCallId)`；非 0 → `Reject(CreateJsError(env, GetJsErrorCodeByNativeError(err)))`（对齐 :287-290）；0 → resolve。**无 waiter、无挂起**（D1 不等目标结局）；
  4. **不需要** `InsightIntentHostClient` 相关扩展（不查 key、不注册回调）。
- `JsInsightIntentDriverInit`（:649 后）：`BindNativeFunction(env, exportObj, "cancel", ...)`。

**ETS 声明** —— `frameworks/ets/ets/@ohos.app.ability.insightIntentDriver.ets`

- :238-252 声明区加 `nativeCancelCheck` / `nativeCancelSync` + `cancel` 双版本导出（照抄 execute :254-278，Promise 版先 Check 再 taskpool.execute）。
- **配套接口注释（实现必做）**：`ExecuteParam.toolCallId` 字段补写契约说明——**"取消定位键：需要取消能力时必传且保证唯一；不传则该次执行不可取消"**（随 d.ts 进 SDK 文档，开发者在 IDE 发起时即可见）。

**ANI（ArkTS native）** —— `frameworks/ets/ani/insight_intent/insight_intent_driver/src/ets_insight_intent_driver.cpp`

- `OnCancelCheck`：空/非串 → `ThrowInvalidParamError`（模式对齐 :234-257）。
- `OnCancelSync`：解参 → `AbilityManagerClient::CancelExecuteIntent` → **同步返回**：err 非 0 时 `AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, err), nullptr)`，成功时 `CreateError(ERROR_OK)`。**无需** GlobalReference/ani_vm/回调客户端（区别于 OnExecute :129-186）。
- `EtsInsightIntentDriverInit` 的 kitFunctions（:566-589）追加 `nativeCancelCheck` / `nativeCancelSync` 注册。

### 6.7 JS 可见错误码总表（cancel()）

| 场景 | native 码 | 反馈时机 |
|---|---|---|
| toolCallId 空/非法 | `ERR_INVALID_VALUE` | 同步（401） |
| 无在途执行（含窄窗口竞态） | `INTENT_NOT_EXIST` 2097329 | 同步 |
| 已结束不可取消 | `INTENT_STATE_NOT_EXECUTING` 2097330 | 同步 |
| 非原调用方 | `CHECK_PERMISSION_FAILED` | 同步 |
| 目标未就绪 / 路由失败 | `ERR_TARGET_NOT_STARTED` 2097333 / `ERR_INVALID_VALUE` | 同步 |
| 分布式意图 | `INNER_ERR` | 同步 |
| 受理成功 | `ERR_OK` → cancel() resolve（语义 = 请求已送达） | 同步 |
| **真正取消完成**（execute Promise 侧） | `ERR_INSIGHT_INTENT_CANCELED` 2097405 | 异步，经完成通道 → execute reject **16000139** |
| **未实现 onCancel / 未中止** | 无反馈（D1） | execute 以真实结果 settle |

新增 `AbilityErrorCode` 后需同步 C API 映射表并跑 `ability_business_error_test`。

## 7. 测试方案

| 测试目标 | 用例 |
|---|---|
| `test/unittest/insight_intent/insight_intent_execute_manager_test/`（BUILD.gn 不加源文件） | `CancelIntent_0100~0500`：空参 / 未注册 / state=EXECUTE_DONE / tokenId 不匹配 / 命中返回 intentId；重复取消幂等（第二次仍受理但目标侧落空由集成验证）；Bind/Unbind 生命周期与 256 淘汰清理。`insight_intent_utils_mock.cpp` 视 `IPCSkeleton::GetCallingTokenID` 依赖补 mock |
| `test/unittest/insight_intent/insight_intent_executor_test/` | `CancelInsightIntent` 返回值分支（true 接管 / false：找不到、未实现 onCancel）；**NOT_SUPPORTED（未实现）回插用例：失败后 map 中执行器仍存活（shared_ptr 未析构）、后续可正常完成**；`TriggerCallbackInner` 携带取消码；`HandleCancelIntent` 默认 false；桥探测（有/无 onCancel、方法语法/属性赋值两种写法）。`mock/mock_js_runtime.cpp` 按需补 `napi_call_function`/`napi_get_named_property` 语义 |
| `test/unittest/insight_intent/ets_insight_intent_tool_call_id_test/`（mock_ani_env.h 模式） | `EtsInsightIntentCancelCheckTest` / `EtsInsightIntentCancelSyncTest`：空串、非字符串、err/OK 同步回调路径 |
| `test/unittest/ability_manager_stub_test` 等 | `EXECUTE_INTENT_CANCEL(76)` 分发与 proxy 序列化（参照现有 ExecuteIntentInner 用例写法） |
| `test/fuzztest/abilitymgrinsightintentexecutemanager_fuzzer/` | 接口列表追加 `CancelIntent` |

UT 用 oh-ut-generator / ohos-test-ut-generation 技能产出，遵循 HWTEST 规范与 75% 覆盖率要求。

## 8. 实施顺序与注意事项

顺序：① 常量/错误码 → ② inner_api → ③ 服务端（②③一体）→ ④ 应用侧调度 → ⑤ 执行器层 → ⑥ 入口 → ⑦ 测试。

注意事项：
- **IPC 版本互操作**：新 proxy 调旧 service 得 `ERR_CODE_NOT_EXIST`，可接受（同仓同发版）。
- **锁纪律**：AMS `CancelIntent` 锁内只查表；应用侧 ExecutorMgr「锁内 erase 取出」是防重复取消与"取消接管 vs 自然完成"竞态的唯一占位点；双报由 AMS `state` 检查丢弃，调用方只收一次。
- **NOT_SUPPORTED 必须回插执行器**（§5.7）：不回插则桥对象析构、`nativeExecutor` 悬垂、自然完成回程 UAF。
- **回调所有权三规则**：mgr→桥 unique_ptr 转移；桥返回 false 不接管（mgr 释放）；桥接管后以 delete 收尾。禁止桥内复用 `callback_`。
- **基类禁给默认 onCancel 实现**（§4.3）：这是"未实现可判定"的前提，评审/code review 硬性检查点。
- `AbilityRecord::Init` 绑定幂等、失败不传播。
- 新增 IPC 码位置纪律：`76` 插 kit 簇空位；scheduler 枚举尾部追加。

## 9. 范围外与后续项

| 项 | 理由 |
|---|---|
| CJ（Cangjie）执行器 onCancel | CJ 走独立 `CJInsightIntentExecutorMgr` 与独立 so，反向链接会成依赖环；FFI 签名形态差异大。服务端改动语言无关，CJ 应用收到 cancel 走"未实现"分支（无反馈），无行为劣化 |
| 分布式意图取消（isDistributed） | 需跨设备协议扩展，本期直接拒绝（INNER_ERR + 日志） |
| 系统侧/CLI 按 toolCallId 取消入口 | 通道可复用；其调用方与执行调用方不同进程，需另行设计授权策略，本期不接。首个驱动场景可参考 Apple：执行超时自动取消（`IntentCancellationReason.timeout` 语义），系统检测超时后走同一取消通道并区分取消原因 |
| cancel 确认语义（D2 增量回补） | 本期 D1 不含；若未来调用方需要 cancel() 的独立确认（如取消后立即更新 UI），按 §13.5 记录的 D2 设计（回执 IPC + OnCancelDone + 挂起收敛）增量回补 |
| onCancel 扩展取消原因参数 | Apple 的 onCancel 携带 `IntentCancellationReason`（userCancelled/timeout）；本设计 onCancel 无参（当前 reason 恒为调用方取消），接口预留扩展为 `onCancel?(reason)` 的空间，待系统侧取消入口落地时一并评估 |
| 意图执行进度上报 | Apple 的 `ProgressReportingIntent` 是超时取消的判定基础（不报进度 30s 即取消）；本设计无进度概念，若引入超时自动取消可一并评估 |
| 「未实现 onCancel」的静态声明式方案（profile 标记 supportCancel） | 声明与实现存在漂移风险，且需 RDB/解析层改动；运行时探测已覆盖 |
| LiteParam 直连执行路径的取消 | 未走 driver.execute 常规链路，无 toolCallId 登记；如需要另行设计 |

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
1. **取消成功**：execute（带 toolCallId）发起 ServiceExtension 长任务 → cancel(toolCallId)：cancel() 即刻 resolve（受理）；目标 onCancel 被回调；随后 execute Promise reject 16000139；
2. **未实现 onCancel**：目标执行器不实现 onCancel → cancel() 同样 resolve（D1 无差异反馈）；原执行自然完成、execute Promise 以**真实结果** settle（据此感知未取消）；
3. **重复/迟到取消**：取消完成后再次 cancel 同一 toolCallId → reject 2097330；
4. **授权**：另一进程 cancel 同 toolCallId → CHECK_PERMISSION_FAILED，原执行不受影响；
5. **竞态**：cancel 受理后、onCancel settle 前执行自然完成 → execute 结果正常送达且仅一次收尾。

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

**（7）锁屏可执行性（延伸核实）**：Siri 可在锁屏调用 AppIntent，由 `AppIntent.authenticationPolicy`（`IntentAuthenticationPolicy`）声明式控制——默认 `.alwaysAllowed`（锁屏可执行，后台唤醒应用进程跑 perform），`.requiresAuthentication` / `.requiresLocalDeviceAuthentication` 显式收紧。锁屏拦的是前台 UI 与数据解密，不是代码执行。对照：OpenHarmony AMS 为全局 `ScreenUnlockInterceptor` 拦截锁屏启动，无 per-intent 声明式策略。

官方示例（支付意图）的关键形态：`perform()` 中每步 `updateProgress(...)` 防超时，`onCancel` 按 reason 分流——`.timeout` → 回滚事务，`.userCancelled` → 取消事务；onCancel 内的清理动作官方建议："stop the current task, release resources, save intermediate state, write out diagnostic information"。

### 11.2 逐项对照

| 设计点 | Apple AppIntents | 本设计 | 结论 |
|---|---|---|---|
| 取消模型 | 协作式（task cancellation，系统不抢占） | 协作式（onCancel 回调，框架不强杀 JS Promise） | 一致；业界共识 |
| 能力声明 | `CancellableIntent` 协议 conform（编译期、类型级） | 基类可选声明 `onCancel?` + 运行时探测 | 同构；Apple 佐证"支持取消是声明式能力"。本设计运行时探测更严格，能发现"声明了但未实现" |
| 未声明能力的后果 | 无清理宽限，系统超时硬掐（进程挂起） | 无反馈（D1），原执行不受影响 | 同为"不支持"语义；Apple 更激进（资源管控视角），本设计更温和（调用方视角） |
| onCancel 参数 | `(IntentCancellationReason)` 带 reason | 无参 | 当前场景 reason 恒为"调用方取消"，无参成立；reason 已列为预留扩展（§9） |
| 取消发起方 | 系统表面（Siri/Shortcuts/Live Activities 停止按钮）+ 系统超时 | 调用方进程（driver.cancel + toolCallId） | 场景差异：iOS 无跨进程第三方调用方 |
| 取消结果回执 | 无（发起方是用户界面，任务结束即感知） | 无（D1：结局由 execute 结果揭示） | **本设计经评审选择与 Apple 同构的轻量形态**；如未来需要 cancel() 确认语义按 §9 回补 D2 |
| 超时自动取消 | 有：30s 未报进度 → 系统自动取消 | 无（本期） | Apple 已验证必要性，列为后续项（§9），届时走同一取消通道 |
| 进度上报 | `ProgressReportingIntent` 是超时取消的判定基础 | 无进度概念 | 若引入超时自动取消可一并评估（§9） |
| 锁屏执行策略 | 声明式 per-intent（`authenticationPolicy`，默认锁屏可执行） | 全局 `ScreenUnlockInterceptor` 拦截，无 per-intent 策略 | 模型差异：iOS 拦"看"（UI/数据），OH 拦"做"（启动动作） |

### 11.3 结论与借鉴点

1. **本设计的两个关键决策获得印证**：取消必须协作式（Apple 同样无法抢占）；"支持取消 = 显式声明的能力，未声明即不支持"（Apple 用协议 conform，本设计用可选声明 + 运行时探测，语义一致）。
2. **D1（fire-and-forget）与 Apple 模型同构**：Apple 的取消没有"取消结果 API"——发起方（系统/用户）天然感知结局；本设计 D1 下调用方从 execute 结果感知结局，反馈路径等价，且避免了为编程式调用方额外构建回执机制。
3. **后续演进方向**（已并入 §9）：超时自动取消（系统侧取消入口的首个驱动场景）、onCancel 预留 reason 参数、进度上报基础设施。

### 11.4 参考来源

- [CancellableIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/cancellableintent)（iOS 26.4+）
- [IntentCancellationReason — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/intentcancellationreason)
- [LongRunningIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/longrunningintent)（iOS 27+）
- [AppIntent — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/appintent)
- [IntentAuthenticationPolicy — Apple Developer Documentation](https://developer.apple.com/documentation/appintents/intentauthenticationpolicy)
- [Discover new capabilities in the App Intents framework — WWDC26 Session 345](https://developer.apple.com/videos/play/wwdc2026/345)
- [CancellationError — Swift Documentation](https://developer.apple.com/documentation/swift/cancellationerror)
- [Explore structured concurrency in Swift — WWDC21](https://developer.apple.com/videos/play/wwdc2021/10132/)

### 11.5 苹果与本设计的机制差异点

差异的总根源只有一条：**取消发起方不同**。苹果的取消发起方是系统自身（用户经监控 UI 点按、或超时自动触发），任务句柄天然在系统手里；本设计的取消发起方是**另一个进程中的第三方调用方**。以下所有差异都是这一条的推论。

**（1）苹果的 cancel 根本不是 API。**
系统对"自己正在运行的 Task"发出控制信号（`Task.cancel()`），整个过程没有任何函数参与——开发者唯一接触的是**接收端钩子**（`onCancel(reason)`）。本设计的 cancel 是**跨进程 IPC 请求**：需要定位键、需要授权。参数设计的纠结（key 怎么选、谁保证唯一）在苹果的模型里不存在，因为那里没有参数。

**（2）逐条差异**

| 维度 | 苹果 AppIntents | 本设计 | 差异根源 |
|---|---|---|---|
| 取消发起方 | 系统自身（UI 点按 / 30s 超时） | 调用方进程（编程式） | **总根源** |
| 取消句柄 | 系统 Task 内部句柄，不暴露 | toolCallId（跨 IPC 定位键） | 发起方不持有任务 |
| "取消哪个"的选择机制 | 监控 UI 与执行实例一一绑定，按哪张卡取消哪个 | toolCallId → intentId → 执行器实例，查表路由 | 发起方在系统外 |
| 授权 | 不需要（系统取消自己的任务） | callerTokenId 三重匹配 | 第三方发起必须验权 |
| 取消结果反馈 | 不需要（用户肉眼可见界面结果） | D1：无独立反馈，由 execute 结果揭示 | 评审取舍：反馈信息可免费获得，不建回执机制 |
| 竞态收敛 | `Task.cancel()` 单点原子 | AMS state 检查 + ExecutorMgr erase 两道闸门 | 状态在 AMS 表里而非运行时对象上 |
| 取消原因 | `IntentCancellationReason`（timeout / userCancelled），发起方有两种 | 无参（发起方单一，reason 恒定） | 发起方多样性（§9 已预留） |
| 执行模型 | Swift Task 协作取消 | JS/ETS Promise 协作取消 | **一致点**：双方都是协作式，框架不抢占 |
| 锁屏执行策略 | 声明式 per-intent（`authenticationPolicy`） | 全局 ScreenUnlockInterceptor | iOS 拦"看"，OH 拦"做" |

**（3）复杂度归因与 D1 决策**

- **内生成本**（跨进程第三方取消必然产生，已保留）：定位键、授权校验、onCancel 钩子、结果通道。
- **自愿成本**（已评审拆除）：T2 回执闭环——独立回执 IPC、`OnCancelDone` 回调、`pendingCancelKey` 收敛、超时兜底、hostClient waiter。该块购买的是"cancel() 对目标内部结局的独立反馈"，经评审判断其增量价值（16000140 / 完成才 resolve）低于机制成本，**已降级为 D1**；如未来需要，按 §9 的 D2 记录增量回补。

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

  // ★ onCancel —— 可选实现；不实现则该意图不可被取消（执行自然完成）
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
  toolCallId,                       // ★ 取消关联键（取消前提：不传则本次执行不可取消）
});

// 中途取消：resolve = 取消请求已送达目标
try {
  await insightIntentDriver.cancel(toolCallId);
  // 此后 executePromise 会以 16000139 reject（已实现 onCancel 且中止成功）
  // 或以真实结果 settle（目标未实现 onCancel，未取消——从结果可感知）
} catch (err) {
  const e = err as BusinessError;
  if (e.code === 2097330) {
    // 该执行已结束，无需取消
  } else if (e.code === 2097329) {
    // 无匹配的在途执行（或发起时未传 toolCallId）
  }
}
```

### 12.4 开发者视角的取消时序

```
execute(toolCallId) ──▶ 目标 onExecute 开始（长任务）
cancel(toolCallId)  ──▶ AMS 校验（在途/授权/目标就绪）──▶ 桥探测 onCancel
   │                                                    ├─ 未实现 ──▶ 桥 false，仅日志
   │ resolve（受理即应答）                                               执行继续，execute 以真实结果 settle
   │                                                    └─ 已实现 ──▶ onCancel() 被调（中止业务）
   │                                                                    │ settle
   │                                             ExecuteInsightIntentDone(intentId, 取消码)
   ▼                                                              ▼
（同步错误时即刻 reject：                              execute Promise reject 16000139
  2097329 / 2097330 / 授权失败等）                     （onExecute 后续结果被丢弃）
```

## 13. cancel 参数设计备选方案（决策记录）

> 状态：**已拍板（2026-09-27）——采纳方案一**：`cancel(toolCallId: string)`，与 execute 的 toolCallId 既有透传链路绑定。正文（§2/§3/§6/§12）与之一致。本节保留全部候选与对比作为决策记录，决策理由见 §13.6。

### 13.1 参考系：execute 参数的三类字段

```typescript
// 现状 ExecuteParam（driver .ets:25-39）
{ bundleName, moduleName, abilityName,      // ① 身份字段：定位"哪个意图"
  insightIntentName,
  insightIntentParam, executeMode,          //    （业务参数/模式，不参与定位）
  displayId?, uris?, flags?, userId?, deviceId?,  // ② 修饰字段：不参与定位
  toolCallId? }                             // ③ 关联字段：定位"哪一次执行"
```

cancel 参数设计的本质 = **选哪一类字段做定位**。

### 13.2 候选方案

**方案一：`cancel(toolCallId: string)` —— 关联字段型（已采纳）**

```typescript
driver.cancel('call-001');
```
定位：toolCallId → records_ 唯一命中。坑：调用方不传 toolCallId 则该次执行不可取消，且取消时报 2097329 无法引导归因（契约违例延迟爆炸）——以接口注释强化契约（§6.6）。

**方案二：`cancel(param: ExecuteParam)` —— 入参回放型**

```typescript
const param = { bundleName: 'com.x', moduleName: 'entry', insightIntentName: 'ChatSummary', ... };
driver.execute(param);
driver.cancel(param);     // 用发起时的参数原样取消
```
语义："用什么发起，就用什么取消"，零新类型、对称性最好。缺点：调用方须保管原始参数对象；传大对象只用几个定位字段；匹配只能取身份字段子集（insightIntentParam 为 Record 不可深比较，需文档化排除）。

**方案三：`cancel(param: CancelParam)` —— 身份子集型（方案二收敛版）**

```typescript
export interface CancelParam {
  bundleName: string;            // 必选：限定调用边界
  moduleName?: string;           // 可选：进一步收窄
  insightIntentName: string;     // 必选：定位意图
  toolCallId?: string;           // 可选：提供则精确到单次执行
}
```
语义：默认按"我发起的某意图"取消（发起零负担），带 toolCallId 则精确到次。缺点：必填身份字段拼错时静默 2097329；引入新类型与匹配矩阵。

**方案四：`cancel(id: number)` —— 框架键型**

```typescript
// 前提：execute 侧先回传 intentId（ExecuteIntent 同步 IPC reply 尾部追加 / ExecuteParam 可选 onIntentId 回调）
driver.cancel(executableId);
```
唯一性最强（intentId 由 AMS 自增、框架保证）。缺点：依赖 execute 侧改造；id 靠回调捕获，漏捕获即失去取消能力（坑换个位置而非消失）。

**方案五：`cancelAll(intentName?: string)` —— 批量补充型**

```typescript
driver.cancelAll();                  // 取消我发起的全部在途执行（页面销毁清场）
driver.cancelAll('ChatSummary');     // 按意图批量
```
非替代而是补充，可与任一方案共存，实现成本低。

### 13.3 对比矩阵

| | 发起时负担 | 并发歧义 | 主要坑 | API 对称性 | 实现增量 |
|---|---|---|---|---|---|
| 一 toolCallId | 造 key 并保证唯一 | 无 | 忘传 = 不可取消且难归因 | 一般（新增语义） | record 加 toolCallId |
| 二 回放 ExecuteParam | 保管原始参数对象 | 有（按身份） | 参数对象丢失则无措 | **最好**（零新类型） | 匹配逻辑 |
| 三 身份子集 | **零**（身份字段 execute 本就必填） | 有 → 需定义命中策略 | 必填身份字段拼错 | 好（同族类型） | record 加 intentName + 匹配 |
| 四 intentId | 捕获回调返回值 | 无 | 漏捕获则失去取消能力 | 一般 | IPC reply 扩展 |
| 五 cancelAll | 零 | —（定义为批量） | — | — | 最小 |

### 13.4 评估中被否决的备选（避免重新踩坑）

- **`cancel(promise)`**（以 execute 返回的 Promise 为句柄）：句柄由框架铸造、无需新增参数字段，曾作为推荐；但被"调用方不一定持有 promise"证伪——fire-and-forget、立即 await、跨模块移交等场景下句柄丢失，且**丢失人群（不关心结果的人）与需要取消的人群（想止损的人）高度重合**。
- **AbortController/Signal 模式**（Web 惯例）：signal 对象无法跨 IPC，仅是 driver 内部 key 的糖；且 opt-in 创建控制器后"忘传"坑依旧。

### 13.5 决策记录与遗留项

1. **已决**：参数方案 = **方案一** `cancel(toolCallId: string)`（理由见 §13.6）。
2. **已决**：回执语义 = **D1 fire-and-forget**（2026-09-27）。拆除 T2 回执闭环全部"自愿购买"部件（§11.5（3））；如未来需要 cancel() 确认语义，按 §9 的 D2 记录增量回补。
3. **随方案一自动消解**：同 toolCallId 多条在途的命中策略问题；toolCallId 唯一性为调用方契约，接口注释文档化即可。

### 13.6 方案一决策理由

1. **定理约束**：调用方发起的取消必然要求调用方持有某种句柄，而任何句柄都存在"未创建或丢失"的方式（§11.5 推论）——不存在结构性无坑的参数形态，选择依据只能是目标场景的句柄天然度；
2. **目标场景空集**：agent/CLI 工具调用运行时的任务表本来就持有 toolCallId（它是工具调用关联键），方案一的丢失人群在目标场景为空集；
3. **可序列化**：string 可入任务表、可记日志，promise/对象句柄不可；
4. **API 形态无争议**：`cancel(toolCallId: string)` 为常规形状，评审风险最低；
5. **零新增字段**：toolCallId 已是 `ExecuteParam` 字段且具备完整透传链路（NAPI/ANI 参数转换 → Parcelable 尾部可选字段 → Want key → 执行器 context），取消能力直接复用。
