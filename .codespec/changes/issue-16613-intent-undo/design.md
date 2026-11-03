# Design：意图框架撤销/重做能力（IntentUndo）

> 对应 proposal：`.codespec/changes/issue-16613-intent-undo/proposal.md`
> 本文所有引用路径均为 `ability_ability_runtime` master 真实源码。

## 1. 总体架构

```
                ┌──────────────────────────────────────────┐
   触发入口      │  小艺对话 │ 服务卡片 │ 应用内 UI │ 跨设备    │
                └───────────────────┬──────────────────────┘
                                    ▼
                     系统意图分发（intentName + params）
                                    ▼
              ┌─────────────────────────────────────┐
              │   InsightIntentExecutor（业务黑盒）    │
              │  onExecute ──┐                      │
              │  onCancel    │ 三回调契约             │
              │  onRedo    ──┘                      │
              └──────────┬───────────────▲──────────┘
                 成功入栈 │               │ undo()/redo() 回调
                         ▼               │
              ┌─────────────────────────────────────┐
              │  IntentUndoManager（应用进程内，双栈）  │
              │  undoStack / redoStack               │
              │  组 = { intentName, srcEntry,        │
              │        params, executeResult }       │
              └─────────────────────────────────────┘
```

核心原则：

1. **撤销 = 一次"携带历史凭据的意图调用"**，完全复用现有执行分发/权限校验/结果回调链路，系统侧仅透传 action 字段，不新增 IPC 接口。
2. **撤销栈宿主在应用进程**（库层 `IntentUndoManager`），abilitymgr 服务保持无业务状态（现有 `records_` 仅做执行期跟踪）。
3. **一次意图执行 = 一个撤销组**，无需 Run Loop 式事件边界机制。

## 2. 三回调契约

| 回调 | 签名 | 语义 | 约束 |
|---|---|---|---|
| `onExecute` | `(params) => Promise<IntentResult>` | 执行业务事务（现有） | 成功判定 `code == 0` |
| `onCancel` | `(params, executeResult) => Promise<IntentResult>` | 幂等回滚 | 必须能凭凭据重建现场 |
| `onRedo` | `(params, executeResult) => Promise<IntentResult>` | 重放（通常委托 onExecute） | 同上 |

- 凭据 = `(params, executeResult)`，由引擎在 onExecute 成功时保存并在 cancel/redo 时回传；
- 未实现 `onCancel`（基类默认返回 `code: -1`）= 不可撤销，不入栈；
- `onCancel` 必须幂等（重复调用不产生二次破坏）。

## 3. 开源代码级实现细节（五个挂接点）

### 3.1 现状执行链路（源码核实）

```
[系统侧 abilitymgr]
InsightIntentExecuteManager（services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp）
  ├─ AddRecord / records_（intentId → 执行记录，状态机 EXECUTING→EXECUTE_DONE）
  ├─ GenerateWant（执行参数组装进 Want，常量见 execute_param.h）
  ├─ ExecuteIntentDone（结果过滤 + 分布式 DistributedClient::SendIntentResult + HiSysEvent）
  └─ OnRemoteDied / HandleDiedRecordsCleanup
        │ Want（ohos.insightIntent.executeParam.*）
        ▼
[应用侧框架层]
AbilityRuntime::InsightIntentExecutor（interfaces/kits/native/ability/native/insight_intent_executor/insight_intent_executor.h）
  ├─ Create(Runtime&, InsightIntentType)：JS/ETS × DECOR_ENTRY/FUNC/PAGE/QUERY_ENTITY 分发
  └─ HandleExecuteIntent(executeParam, pageLoader, callback, isAsync)
        ▼
frameworks/ets/ets/@ohos.app.ability.InsightIntentEntryExecutor.ets
  ├─ onExecute(): Promise<IntentResult<T>>          ← 开发者重写
  └─ callPromise → nativeOnExecuteResult / nativeOnExecuteError
```

关键有利事实：`InsightIntentExecuteResult`（interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_execute_result.h）已自带 `ToJsonString()/FromJsonString()`，撤销凭据序列化零新增工作。

### 3.2 挂接点①：配置解析层（undoable 声明）

| 文件 | 修改 |
|---|---|
| `insight_intent.json` 规范 | 新增 `"undoable": true` |
| `services/abilitymgr/include/insight_intent/insight_intent_profile.h` | `struct InsightIntentInfo` 增加 `bool undoable = false;` |
| `services/abilitymgr/src/insight_intent/extract_insight_intent_profile.cpp` | 解析字段（nlohmann::json，参照 `supportExecuteMode`） |
| `insight_intent_rdb_storage_mgr.cpp` / `insight_intent_db_cache.cpp` | RDB 加列 + 缓存同步 |

### 3.3 挂接点②：IPC 参数层（动作类型与凭据）

`interfaces/inner_api/ability_manager/include/insight_intent/insight_intent_execute_param.h`：

```cpp
enum class InsightIntentAction : uint8_t { EXECUTE = 0, CANCEL = 1, REDO = 2 };
constexpr char INSIGHT_INTENT_EXECUTE_ACTION[] = "ohos.insightIntent.executeParam.action";
constexpr char INSIGHT_INTENT_EXECUTE_CREDENTIAL[] = "ohos.insightIntent.executeParam.credential";
// InsightIntentExecuteParam 新增：
//   InsightIntentAction action_ = InsightIntentAction::EXECUTE;
//   std::shared_ptr<WantParams> undoCredential_;  // 承载原 params + 原 executeResult（JSON）
```

只增不改，既有序列化格式不变；回调链路复用 `insight_intent_execute_callback_proxy/stub`。

### 3.4 挂接点③：服务层（撤销生命周期）

`InsightIntentExecuteManager`（insight_intent_execute_manager.cpp）扩展状态机：

```
EXECUTING → EXECUTE_DONE → CANCELING → CANCELED → REDOING →（回到 EXECUTE_DONE）
```

- 新增 `CancelIntent(intentId, credential)` / `RedoIntent(intentId, credential)`；
- 复用 `records_` 表校验"意图确实执行过且属于该调用方"（对齐现有 `INTENT_STATE_NOT_EXECUTING` 拒绝模式），防伪造凭据；
- `SendIntentReport` 增加 cancel/redo 事件埋点。

### 3.5 挂接点④：执行器层（基类对称扩展）

`insight_intent_executor.h`（native 基类，与 `HandleExecuteIntent` 对称）：

```cpp
virtual bool HandleCancelIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool& isAsync) { return false; }
virtual bool HandleRedoIntent(...) { return false; }   // 默认未实现 = 不可撤销
```

ETS 基类 `@ohos.app.ability.InsightIntentEntryExecutor.ets`（对称新增，模式复用 `callPromise`）：

```typescript
onCancel(params: Record<string, Object>, r: insightIntent.ExecuteResult): Promise<insightIntent.IntentResult<T>> {
    let ret: insightIntent.IntentResult<T> = { code: -1 };   // 默认不支持撤销
    return Promise.resolve(ret);
}
onRedo(params, r): Promise<insightIntent.IntentResult<T>> { ... }
public native nativeOnCancelResult(data: Record<string, RecordData>): void;
public native nativeOnCancelError(err: Error): void;
```

native 分发 `frameworks/native/ability/native/insight_intent_executor/js_insight_intent_entry.cpp`：在现有取用 executor 方法处增加 `onCancel`/`onRedo` 方法探测，探测到才标记 undoable。

### 3.6 挂接点⑤：撤销栈宿主（应用进程内，零系统侵入）

```typescript
class IntentUndoGroup {
  intentName: string;                          // 定位意图
  srcEntry: string;                            // 定位执行器代码路径
  params: Record<string, Object>;              // 参数快照（可序列化）
  executeResult: insightIntent.ExecuteResult;  // 凭据（ToJsonString 承载）
  actionName: ResourceStr;                     // 撤销按钮文案
}
```

- `undo()`：取栈顶组 → 组装带 `InsightIntentAction::CANCEL` + 凭据的 Want → 走普通意图执行通道（`IntentClient`，`interfaces/inner_api/insight_intent/intent_client/`）→ `onCancel` 被调用；
- `redo()`：镜像，`REDO` + 同一凭据；
- 分叉清空：任何新意图成功执行后清空 redoStack；
- 凭据落盘（M3）：整组 `ToJsonString` 持久化，支持跨会话撤销；
- Agent 联动：`toolCallId_`（execute_param.h 已有，"Shared by Driver and FunctionManager"）随撤销调用透传，发起意图的 Agent 可感知工具调用被用户撤销。

## 4. 双层撤销（意图层 × 模型层）

```
IntentUndoGroup（业务事务，可序列化，跨入口）
└── 内嵌 0..n 个模型层 UndoGroup（闭包操作，UI 细粒度）
```

- 应用内撤销按钮 → 模型层栈（细粒度）；
- 全局/长按入口 → 意图层栈（整个业务事务，内嵌组整批丢弃）；
- 意图执行期间产生的模型层组自动挂入当前意图组（嵌套组语义）。

模型层引擎设计见独立文档（ArkTS 闭包式 UndoManager：双栈 + 组 + 对称 setter），本文档仅定义嵌套集成点。

## 5. 边界与风险

| 风险 | 对策 |
|---|---|
| onCancel 时远端状态已被第三方改变 | 约定返回 `code != 0` 带原因；引擎保留组并置"冲突待处理"，不静默丢弃 |
| 凭据含敏感业务数据被持久化 | 落盘前脱敏 + 应用加密存储；系统服务不落盘凭据 |
| 不可回滚事务（已扣款/已发通知） | 规范要求：不可幂等回滚的事务应在 onExecute 前置确认，不依赖 onCancel 兜底 |
| 跨设备 executor 环境缺失 | srcEntry 定位失败 → 组降级为"仅查看"并提示 |
| 凭据体积过大 | 单组凭据 ≤ 64KB，超限改存应用内引用（存 ID 不存全量） |

## 6. 里程碑

| 阶段 | 范围 |
|---|---|
| M1 | 库层 `UndoableIntentEntryExecutor` + `IntentUndoManager` 内存态双栈（零系统改动，独立交付） |
| M2 | 挂接点①~④系统侧落地；装饰器与配置文件双路径 |
| M3 | 凭据落盘（跨会话撤销）、冲突态处理 |
| M4 | 标准意图 Schema / 文档仓规范同步 |

## 7. 参考实现分析（iOS NSUndoManager）

本设计的每个关键决策都有一条 iOS `NSUndoManager` 的对照出处，完整分析（含架构分层、双栈流转、消息转发截获、机制映射 4 张图）见 [reference/ios-undomanager-insight.md](reference/ios-undomanager-insight.md)。直接影响本设计的 5 条结论：

1. iOS "执行即注册" → 本提案改为框架在 `onExecute` 成功时自动入栈，一致性从开发者责任降级为框架默认值；
2. iOS 靠 Run Loop hook 猜"事件边界" → 意图天然有起止，`一次意图执行 = 一个撤销组`，无需分组机制；
3. iOS Redo 免费来自对称注册 → 意图是黑盒业务无法自动求逆，显式提供 `onRedo`，但引擎回传原 `executeResult` 使其通常只需一行委托 `onExecute`；
4. iOS `setActionIsDiscardable:` → `undoable` 声明 + 未实现 `onCancel` 即不入栈（导航、纯查询自动排除）；
5. iOS 栈在内存且持对象引用，杀进程即丢 → 本提案组为纯数据（`ToJsonString` 可落盘），支持跨会话撤销，且凭据随意图流动可覆盖小艺/卡片/跨设备等应用外入口——iOS 方案做不到的结构性收益。
