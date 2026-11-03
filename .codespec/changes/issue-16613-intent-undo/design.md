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
              │  onUndo    │ 三回调契约             │
              │  onRedo    ──┘                      │
              └──────────┬───────────────▲──────────┘
                 成功入栈 │               │ undo()/redo() 回调
                         ▼               │
              ┌─────────────────────────────────────┐
              │  AppUndoStack（应用进程内，共享时间线） │
              │  undo 段 / redo 段，组带 kind+seq     │
              │  组 = { kind, sequence, intentName,  │
              │        srcEntry, params, result,     │
              │        innerGroups }                 │
              │  ── 模型层闭包组与意图组同栈 ──        │
              └─────────────────────────────────────┘
```

核心原则：

1. **撤销 = 一次"携带历史凭据的意图调用"**，完全复用现有执行分发/权限校验/结果回调链路，系统侧仅透传 action 字段，不新增 IPC 接口。
2. **撤销栈宿主在应用进程**（库层 `AppUndoStack`），`abilitymgr` 服务保持无业务状态（现有 `records_` 仅做执行期跟踪）；栈与模型层**共享同一条时间线**（见 §4），不另建意图专属栈。
3. **一次意图执行 = 一个撤销组**，无需 Run Loop 式事件边界机制。

## 2. 三回调契约

| 回调 | 签名 | 语义 | 约束 |
|---|---|---|---|
| `onExecute` | `(params) => Promise<IntentResult>` | 执行业务事务（现有） | 成功判定 `code == 0` |
| `onUndo` | `(params, executeResult) => Promise<IntentResult>` | 幂等回滚 | 必须能凭凭据重建现场 |
| `onRedo` | `(params, executeResult) => Promise<IntentResult>` | 重放（通常委托 onExecute） | 同上 |

- 凭据 = `(params, executeResult)`，由引擎在 onExecute 成功时保存并在 undo/redo 时回传；
- 未实现 `onUndo`（基类默认返回 `code: -1`）= 不可撤销，不入栈；
- `onUndo` 必须幂等（重复调用不产生二次破坏）。

**命名说明**：`onUndo` 表示"回滚**已完成**的事务"，与 iOS 26 `UndoableIntent` 的 `undo` 语义对齐；"**取消在途**执行"是另一件事（iOS 用 `CancellableIntent.onCancel(reason:)` 承担），若后续需要应在意图框架另立取消通道，**不复用** `onUndo`。

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
enum class InsightIntentAction : uint8_t { EXECUTE = 0, UNDO = 1, REDO = 2 };
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
EXECUTING → EXECUTE_DONE → UNDOING → UNDONE → REDOING →（回到 EXECUTE_DONE）
```

- 新增 `UndoIntent(intentId, credential)` / `RedoIntent(intentId, credential)`；
- 复用 `records_` 表校验"意图确实执行过且属于该调用方"（对齐现有 `INTENT_STATE_NOT_EXECUTING` 拒绝模式），防伪造凭据；
- `SendIntentReport` 增加 undo/redo 事件埋点。

### 3.5 挂接点④：执行器层（基类对称扩展）

`insight_intent_executor.h`（native 基类，与 `HandleExecuteIntent` 对称）：

```cpp
virtual bool HandleUndoIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool& isAsync) { return false; }
virtual bool HandleRedoIntent(...) { return false; }   // 默认未实现 = 不可撤销
```

ETS 基类 `@ohos.app.ability.InsightIntentEntryExecutor.ets`（对称新增，模式复用 `callPromise`）：

```typescript
onUndo(params: Record<string, Object>, r: insightIntent.ExecuteResult): Promise<insightIntent.IntentResult<T>> {
    let ret: insightIntent.IntentResult<T> = { code: -1 };   // 默认不支持撤销
    return Promise.resolve(ret);
}
onRedo(params, r): Promise<insightIntent.IntentResult<T>> { ... }
public native nativeOnUndoResult(data: Record<string, RecordData>): void;
public native nativeOnUndoError(err: Error): void;
```

native 分发 `frameworks/native/ability/native/insight_intent_executor/js_insight_intent_entry.cpp`：在现有取用 executor 方法处增加 `onUndo`/`onRedo` 方法探测，探测到才标记 undoable。

### 3.6 挂接点⑤：撤销栈宿主（应用进程内，零系统侵入）

```typescript
class UndoGroup {
  kind: 'model' | 'intent';                    // 组类型：决定回放方式
  sequence: number;                            // 全局单调递增，撤销按栈顶取，落盘后仍保序
  intentName: string;                          // 定位意图（kind=intent 时有效）
  srcEntry: string;                            // 定位执行器代码路径
  params: Record<string, Object>;              // 参数快照（可序列化）
  executeResult: insightIntent.ExecuteResult;  // 凭据（ToJsonString 承载）
  innerGroups: UndoGroup[];                    // 意图执行窗口内产生的模型层组，整批丢弃
  actionName: ResourceStr;                     // 撤销按钮文案
}
```

- `undo()`：取栈顶组 → `kind=intent` 时组装带 `InsightIntentAction::UNDO` + 凭据的 Want → 走普通意图执行通道（`IntentClient`，`interfaces/inner_api/insight_intent/intent_client/`）→ `onUndo` 被调用；`kind=model` 时直接回放闭包；
- `redo()`：镜像，`REDO` + 同一凭据；
- 分叉清空：任何新组入栈时清空 redo 段（对齐 NSUndoManager 语义）；
- 凭据落盘（M3）：整组 `ToJsonString` 持久化，支持跨会话撤销；
- Agent 联动：`toolCallId_`（execute_param.h 已有，"Shared by Driver and FunctionManager"）随撤销调用透传，发起意图的 Agent 可感知工具调用被用户撤销。

## 4. 撤销时间线：与模型层共享同一条栈

早期草案曾设计"意图层独立双栈 + 模型层嵌套"，重新评估后**放弃该方案**，理由来自 iOS App Intents 的实际取舍（见 §7）：撤销体验的底线是**顺序正确**——用户改字段 A → 跑意图 → 再改字段 B，连续撤销必须按 B、意图、A 的逆序回放。两条独立栈做不到交错回放。

因此采用**单一撤销时间线**，意图组是栈中一种带类型的组：

```
AppUndoStack（应用进程内唯一撤销时间线，LIFO）
├── Group{ kind: .model,  ops: [闭包 ...] }              ← UI 编辑
├── Group{ kind: .intent, intentName, srcEntry,
│          params, executeResult, actionName }           ← 意图事务（可序列化）
└── Group{ kind: .model,  ops: [...] }                   ← UI 编辑
```

规则：

1. **同一栈、同一 seq 序列**：每次入栈分配单调递增 `sequence`，撤销永远取栈顶组，天然按发生顺序逆序回放；
2. **意图组是原子单位**：撤销意图组 = 调一次 `onUndo`（携带凭据）；组内不再暴露模型层细粒度操作；
3. **意图执行期间产生的模型层组收编**：`onExecute` 执行窗口内注册的模型组不单独入栈，而是作为该意图组的 `innerGroups` 一并提交——撤销时丢弃整个子列表，不做逐条回放（避免"撤一半"的中间态）；
4. **应用内撤销按钮 = 同一个栈**：按钮与全局入口只是触发源不同，语义完全一致；若产品需要"只撤 UI、不撤意图"的局部撤销，用**作用域过滤**（`undo(scope:)` 跳过不匹配 kind 的组）而不是另建一条栈；
5. **凭据落盘时保序**：`sequence` 随组持久化，重启后按序恢复，跨会话撤销仍保持时间线正确。

> 与 Apple 方案的差异：Apple 用 `NSUndoManager` 的共享栈 + 闭包解决顺序问题；本提案保留共享栈思想，但组的 payload 换成可序列化凭据（因为意图可能跨进程/跨设备，闭包无法携带）。

模型层引擎（ArkTS 闭包式 UndoManager）设计见独立文档，本文档只定义它如何向共享栈提交组。

## 5. 边界与风险

| 风险 | 对策 |
|---|---|
| onUndo 时远端状态已被第三方改变 | 约定返回 `code != 0` 带原因；引擎保留组并置"冲突待处理"，不静默丢弃 |
| 凭据含敏感业务数据被持久化 | 落盘前脱敏 + 应用加密存储；系统服务不落盘凭据 |
| 不可回滚事务（已扣款/已发通知） | 规范要求：不可幂等回滚的事务应在 onExecute 前置确认，不依赖 onUndo 兜底 |
| 跨设备 executor 环境缺失 | srcEntry 定位失败 → 组降级为"仅查看"并提示 |
| 凭据体积过大 | 单组凭据 ≤ 64KB，超限改存应用内引用（存 ID 不存全量） |

**best-effort 条款**：撤销可用性不设硬保证（executor 被回收、跨设备环境缺失、凭据超限、栈深淘汰）。产品流程不得依赖"撤销一定成功"——不可逆操作仍须前置确认，撤销只做二次兜底。此条对齐 iOS `UndoableIntent.undoManager` 可为 `nil` 的降级哲学。

## 6. 里程碑

| 阶段 | 范围 |
|---|---|
| M1 | 库层 `UndoableIntentEntryExecutor` + `AppUndoStack`（共享时间线，内存态；零系统改动，独立交付） |
| M2 | 挂接点①~④系统侧落地；装饰器与配置文件双路径 |
| M3 | 凭据落盘（跨会话撤销）、冲突态处理 |
| M4 | 标准意图 Schema / 文档仓规范同步 |

## 7. 参考实现分析（iOS 两代方案）

本设计对照 iOS 的两层实现，完整分析见两份参考文档：

| 文档 | 覆盖范围 |
|---|---|
| [reference/ios-undomanager-insight.md](reference/ios-undomanager-insight.md) | **引擎层**：`NSUndoManager` 的组/双栈/状态机、三种注册 API、`forwardInvocation` 消息转发截获、命令模式与快照的权衡（4 张图） |
| [reference/ios-undo-system-analysis.md](reference/ios-undo-system-analysis.md) | **全景 + 意图层**：iOS 26 `UndoableIntent` 的精确规格、执行链路逐跳、开发者六触点、分组边界陷阱、与 Apple 方案的横向对照（4 张图） |

### 7.1 iOS 26 意图层的关键事实（决定了本文档的形态）

- `UndoableIntent` 只有**一个成员**：`@MainActor var undoManager: UndoManager?`；
- **没有 `onUndo()` / `onRedo()`，也没有 `undoable` 字段**——遵守协议即声明，撤销靠 `perform()` 内 `registerUndo` 向**共享** `NSUndoManager` 注册闭包；
- **意图自身永不调用 `undo()` / `redo()`**，触发只能来自 App 界面（三指左滑、菜单、摇一摇、⌘Z）；
- `undoManager` 可能为 `nil`（后台执行、宿主未拉起等），撤销能力是 **best-effort**；
- **Redo 零 API**，完全依赖撤销回放时的对称注册（隐式契约，不注册就静默失效）；
- `CancellableIntent.onCancel(reason:)` 是"**取消在途任务**"（用户点取消 / 超时 / 系统回收资源），与撤销无关。

### 7.2 由此产生的五点修正（均已落入本文档）

| # | 修正 | 落点 |
|---|---|---|
| 1 | 撤销回调由 `onCancel` 改名 **`onUndo`**，避免与 `CancellableIntent` 的"取消"语义撞名 | §2、§3.3、§3.5 |
| 2 | 放弃"意图层独立双栈"，改为**与模型层共享同一条撤销时间线**（顺序正确是撤销体验的底线） | §4 |
| 3 | 保留显式 `onRedo`，但明确理由是"跨进程黑盒意图无法做对称注册"，并要求规范标注"可安全委托 `onExecute`" | §2、§3.5 |
| 4 | 撤销组边界绑定**意图执行起止**而非事件边界（Apple 侧存在 `await` 把一次意图拆成多组的坑，本提案无此问题，作为设计优点记录） | §3.6、§4 |
| 5 | 增加 **best-effort 条款**：撤销不设硬保证，不可逆操作仍须前置确认 | §5 |

### 7.3 与 Apple 方案的横向对照

| 维度 | Apple `UndoableIntent`（iOS 26） | 本提案 |
|---|---|---|
| 能力声明 | 协议一致性（编译期） | `undoable` 字段 + 实现探测（运行期，适配 JSON 配置） |
| 撤销凭据 | 闭包捕获内存快照 | `(params, executeResult)` 可序列化凭据 |
| 撤销栈 | 共享 `NSUndoManager` 单时间线 | 同样共享时间线；组 payload 换为可序列化凭据 |
| Redo | 无 API，靠对称注册（隐式契约） | 显式 `onRedo`（跨进程无法对称注册），通常一行委托 `onExecute` |
| 持久化 | 无（内存栈，杀进程即丢） | `ToJsonString` 落盘 → 支持跨会话撤销 |
| 触发面 | 仅 App 界面手势/菜单 | 小艺 / 服务卡片 / 跨设备 / 应用内，凭据随意图流动 |
| 失败语义 | `undoManager == nil` 静默降级 | `code != 0` 携带原因 + 组置"冲突待处理"，不静默丢弃 |

核心差异一句话：**Apple 用"闭包 + 内存栈"覆盖"App 活着 + 用户在界面旁"的场景；本提案用"可序列化凭据 + 共享时间线"把覆盖域扩展到跨会话、跨设备、跨入口，同时用显式 `onRedo` 补上黑盒业务无法自动求逆的缺口。**
