# 三方 Agent 函数调用接口设计（invokeFunction for Third-party Agents）

> 状态：设计草案（未实现）
> 范围：仅函数调用（对齐 CLI `FunctionManager.invokeFunction` 能力）
> 目标读者：意图框架 / 公共 SDK / 安全评审

## 1. 背景与目标

当前系统内存在两条「按名调用应用函数」的链路，但二者都不面向三方 Agent：

| 链路 | 入口 | 权限 | 面向对象 | 缺口 |
|---|---|---|---|---|
| 意图框架系统调用 | `@ohos.app.ability.insightIntentDriver#execute` / `AbilityManagerService::ExecuteIntent` | `ohos.permission.EXECUTE_INSIGHT_INTENT`（系统权限） | 系统应用 / 意图框架自身 | 三方不可用 |
| CLI 工具函数调用 | `FunctionManager.invokeFunction`（`cli_tool_framework`） | 开发者模式参数门控 + 系统侧 `CliToolMGR` | 系统 CLI / 开发者工具 | 非公共 SDK，且依赖 CliTool 会话上下文 |

本设计新增第三条链路：**面向三方 Agent 的公共 ArkTS SDK**，通过
「新增 normal 权限（调用方可授予）+ 目标应用显式声明可被调用（被调方主动开放）」双重约束，
让普通三方应用（Agent）能够调用目标应用通过 `@InsightIntentFunctionMethod` 暴露的无界面函数，并复用意图框架既有的
匹配 / 下发 / 回传能力。

### 设计原则
1. **最小能力面**：首期只开放「函数调用」，不含页面/入口/link/实体查询/钩子。
2. **双端显式声明**：调用授权（normal 权限）+ 被调开放（profile 声明）缺一不可，默认不开放。
3. **复用既有执行引擎**：函数匹配、Want 组装、后台启动、`insightIntentId` 记录与结果回传全部复用，只新增「鉴权分支」与「入口封装」。
4. **公共 API 兼容**：只做加法（新增模块/字段/方法），不修改既有 API 签名、错误码值、profile 已发布字段语义。

## 2. 现有关键抓手（事实）

- 简化 IPC 已存在：`IntentClient::ExecuteIntentByFunctionCall`（`interfaces/inner_api/insight_intent/intent_client/src/intent_client.cpp:126`）→ opcode
  `EXECUTE_INTENT_BY_FUNCTION_CALL` → `AbilityManagerService::ExecuteIntentByFunctionCall`
  （`services/abilitymgr/src/ability_manager_service.cpp:15497`）→ `ExecuteIntentCommon`（:15638）。
- 鉴权闸门：`InsightIntentExecuteManager::CheckCallerPermission`
  （`services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:782`），分支：shell / 系统应用自身意图 / 非系统拒绝 /
  `EXECUTE_INSIGHT_INTENT` 校验。**没有 normal 权限分支。**
- 函数声明载体：profile 的 `extractInsightIntents[i]` 解析见
  `extract_insight_intent_profile.cpp`（字段常量 ~:33-76，intent 级解析 ~:300-330），
  结构体 `ExtractInsightIntentInfo` / `InsightIntentFunctionInfo` 见 `include/insight_intent/extract_insight_intent_profile.h`。
  现有 `permissions` 字段（`"permissions"`）当前**仅查询展示**（`insight_intent_utils.cpp` / `insight_intent_info_for_query.cpp`），不参与鉴权。
- 回调宿主：`InsightIntentHostClient`（`interfaces/kits/native/ability/native/insight_intent_host_client.h` +
  `frameworks/native/ability/native/insight_intent_host_client.cpp`），以自增 `key` 关联回调，
  `OnExecuteDone(key, resultCode, result)` → `InsightIntentExecuteCallbackInterface::ProcessInsightIntentExecute`。
- 超时/竞态先例：`InvokeFunctionExecutor::SetupTimeout`（30s，`invoke_function_executor.cpp:34`）用共享 `completed_` 原子标志保证「恰好一次」回传。
- 分布式频控先例：`AbilityManagerService::IsFloodAttackByCallerUid`（`ability_manager_service.cpp:15910`）。

## 3. 接口设计

### 3.1 ArkTS 公共 SDK（新增模块）

模块名建议 `@ohos.app.ability.insightIntentFunction`（NAPI 注册名 `app.ability.insightIntentFunction`），
与系统侧 `insightIntent` / `insightIntentDriver` 隔离，语义清晰且无需污染既有系统 API 面。

```ts
declare namespace insightIntentFunction {
  /** 调用选项，首期最小化 */
  interface InvokeFunctionOptions {
    /**
     * 链路追踪 id（可选）。仅用于日志/规因关联，不参与鉴权。
     * 由框架校验长度与字符集，非法则丢弃（对齐 invokeFunction 的 ADR-8 权威注入）。
     */
    toolCallId?: string;
  }

  /** 调用结果 */
  interface InvokeFunctionResult {
    /** 是否业务成功（executeResult.code == 0） */
    success: boolean;
    /** 业务/协议错误码（透传目标函数返回的 code） */
    code: number;
    /** 错误信息（可空） */
    message: string;
    /** 目标函数返回的数据（WantParams → Record<string, Object>，可空） */
    data?: Record<string, Object>;
  }

  /**
   * 调用目标应用通过 @InsightIntentFunctionMethod 暴露的函数。
   * 调用方需持有 ohos.permission.INVOKE_INSIGHT_INTENT_FUNCTION（normal）；
   * 目标应用需在意图 profile 中对该 intentName 声明 agentCallable: true。
   */
  function invokeFunction(
    bundleName: string,
    intentName: string,
    args?: Record<string, Object>,
    options?: InvokeFunctionOptions
  ): Promise<InvokeFunctionResult>;
}

export default insightIntentFunction;
```

要点说明：
- **调用方身份从 IPC 调用进程推导**（`IPCSkeleton::GetCallingUid/TokenID`），无需像 CLI 那样传 `context`。
- **`bundleName + intentName`** 语义对齐 `ExecuteIntentByFunctionCall(bundleName, intentName, wantParam)`；
  `intentName` 即目标应用 profile 里声明的意图名，落地为 `functionName`（装饰方法名）由服务端匹配得到。
- **不暴露** `dmSessionId` / `taskId`（首期三方不需要，且 `ohos.task.id` 是系统侧任务归属保留键，避免伪造面）。

### 3.2 返回结构映射

`InvokeFunctionResult` 由 `AppExecFwk::InsightIntentExecuteResult`（`code`/`result`/`innerErr`）在 NAPI 层转换，
对齐 CLI `InvokeFunctionCallbackClient::ProcessInsightIntentExecute`（`invoke_function_callback_client.cpp:28`）的字段映射：
`success = (result.code == 0)`、`code = result.code`、`message = result.`（错误文案）、`data = result.BuildFunctionResult()`。

### 3.3 错误码

沿用 `AbilityManagerErrorCode` 既有码值，NAPI 层通过 `GetJsErrorCodeByNativeError` 转换为 JS 异常；
不新增错误码值（避免触碰公共合约）。关键路径：
- `ERR_NOT_SYSTEM_APP` / `ERR_PERMISSION_DENIED` → 无 normal 权限或目标未声明 `agentCallable`
- `INTENT_NOT_EXIST` → 目标未匹配到该 `intentName` / 未声明
- `FUNCTION_EXECUTE_TIMEOUT`（新增内部常量，映射既有超时码）→ 超时
- `ERR_INVALID_VALUE` → 参数非法

## 4. 权限与安全模型

### 4.1 双端声明（双重约束）

**调用方（Agent）侧**
- 在 `module.json5` `requestPermissions` 声明新 normal 权限：
  `ohos.permission.INVOKE_INSIGHT_INTENT_FUNCTION`（`grant_type: user_grant`，用户可授予）。
- 权限本体定义在 **`security/permission` 子系统**（`permission_list.json`），本仓只以字符串常量消费，
  与 `EXECUTE_INSIGHT_INTENT` 现状一致，属跨仓依赖，需同步提审。

**目标应用（被调方）侧**
- 在函数意图 profile 的 `extractInsightIntents[i]` 中新增**可加性字段** `"agentCallable": true`（默认 false）。
- `false` 或缺省：三方 Agent 不可调用（仅系统路径可用），保持向后兼容。

### 4.2 服务端鉴权逻辑（新增分支，不动既有分支）

在 `InsightIntentExecuteManager::CheckCallerPermission` 现有分支之后追加（或独立新方法 `CheckAgentCallPermission`）：

```
1. IsShellCall()                                  → 放行（调试/排查，同现状）
2. 系统 API + 调用方 bundle == 目标 bundle        → 放行（同现状）
3. 系统 API + EXECUTE_INSIGHT_INTENT              → 放行（同现状，系统路径不变）
4. NEW: 调用方持有 INVOKE_INSIGHT_INTENT_FUNCTION
       且 目标意图 infos.agentCallable == true
       且 执行模式为函数类（decoratorType == @InsightIntentFunctionMethod）
                                                → 放行（normal 分支）
5. 其余                                          → ERR_NOT_SYSTEM_APP / ERR_PERMISSION_DENIED
```

关键约束（防越权）：
- `agentCallable` 的校验**只对函数类装饰器生效**，页面/入口/link/form 不开放给 normal 分支。
- 校验 `agentCallable` 的参数来源必须是**服务端 DbCache 匹配到的 `ExtractInsightIntentInfo`**，不得把调用方传入的 wantParam 当授权依据。
- `interactionInfo` 等敏感回传字段：沿用 `ExecuteIntentDone` 的「非系统应用裁剪」逻辑
  （`insight_intent_execute_manager.cpp:359`），三方 Agent 天然拿不到 `interactionInfo`。

### 4.3 反滥用

- **频控**：对三方 Agent 调用新增独立频控（复用 `IsFloodAttackByCallerUid` 思路，按 `callerUid + targetIntentName` 计数），
  阈值可配置，防止恶意扫目标函数。
- **超时**：复用 30s 超时 + `completed_` 原子「恰好一次」回传模式（`InvokeFunctionExecutor` 先例），
  同时服务端 `records_` 的死亡监听（`InsightIntentExecuteRecipient`）覆盖调用方进程崩溃清理。
- **保留键防伪**：NAPI 层剥离 `ohos.insightIntent.toolCallId` / `ohos.task.id` 等同名入参，避免调用方伪造保留键。
- **参数校验**：`args` 转 `WantParams` 走 `InsightIntentParamParser`（`PrepareFunctionCallParam` 路径）的 schema 校验，
  与函数 profile 的 `parameters` schema 对齐，非法参数在服务端拒绝。

## 5. 端到端时序

### 5.1 调用（发起）

```
三方 Agent 应用（持有 normal 权限）
  └─ insightIntentFunction.invokeFunction(bundleName, intentName, args, options)
       │  NAPI: frameworks/js/napi/insight_intent_function/
       ▼
① 参数校验 + 保留键剥离 + 转 WantParams + Promise/TSFN 建立
       ▼
② 新增 IPC 客户端（复用 IntentClient 模式，或新 InsightIntentFunctionClient）
     → opcode EXECUTE_AGENT_INTENT_FUNCTION（新增，隔离于系统路径）
       │  callerToken = InsightIntentHostClient（key 关联回调）
       ▼
③ AMS::ExecuteAgentIntentFunction（新增）
     → 鉴权（normal 权限 + agentCallable，见 §4.2）
     → PrepareFunctionCallParam（匹配 profile，复用）
     → CheckAndUpdateParam（分配 insightIntentId + 死亡监听）
     → GenerateWant（函数装饰器 UpdateFuncDecoratorParams）
     → StartAbilityByCallWithInsightIntent（UI_ABILITY_BACKGROUND 后台直调）
       ▼
④ 目标 UIAbility 执行 @InsightIntentFunctionMethod 函数
```

### 5.2 回传（结果）

```
④ 目标函数返回 → UIAbilityImpl::ExecuteInsightIntentDone(intentId, result)
       ▼
⑤ AMS::ExecuteInsightIntentDone → ExecuteIntentDone
     → 非系统裁剪 interactionInfo → OnExecuteDone(record->key, resultCode, filteredResult)
       ▼
⑥ InsightIntentHostClient::OnExecuteDone(key,...) → ProcessInsightIntentExecute
       ▼
⑦ NAPI 回调客户端：映射 InvokeFunctionResult → (TSFN) resolve/reject Promise
```

### 5.3 隐私同意（同步弹窗，PENDING_CONSENT）

三方 Agent 调用属于「代理发起」，执行前必须取得用户同意。同意弹窗由隐私中心领域实现，
AMS 侧只定义结果接缝（`RequestAgentPrivacyDialog` 返回 true/false 的适配层，mock 可替）。
接口**全程同步**：`ExecuteIntentByAgent` 一次 IPC 内走完鉴权、预登记、弹窗、执行，
弹窗展示时间由这次同步调用覆盖：

```
ExecuteIntentByAgent（IPC，同步）
  → ① 鉴权 VerifyAgentCallPermission（骨架桩，后续接 normal 权限 + agentCallable）
  → ② 预登记 RegisterExecuteRecord：记录 state = PENDING_CONSENT；
       此刻起隐私中心可通过 IPC 6191 IsInsightIntentCall(intentId, callerTokenId) 回查为 true
  → ③ RequestAgentPrivacyDialog（mock：恒返回 true；真实实现为隐私中心弹窗，阻塞至用户操作）
       ├─ granted → ④ CAS PENDING_CONSENT → EXECUTING
       │             → ExecuteIntentByFunctionCallWithId(specifiedIntentId)
       │             → 意图函数结果走既有 OnExecuteDone(key) 回传（§5.2 ⑤⑥⑦）
       │             （CAS 失败 = 弹窗期间调用方已死，清记录并直接返回失败）
       └─ denied / 记录不可回查 → RemoveExecuteIntent 清记录 → IPC 直接返回失败码
```

设计要点：
- **同意与执行绑定同一条记录**：intentId 在预登记时分配，弹窗回查与执行复用同一 id，
  「用户同意的那次」和「实际执行的那次」不可能错位；
- **PENDING_CONSENT 语义收紧**：`IsInsightIntentCall(intentId, callerTokenId)` 只对 PENDING_CONSENT /
  EXECUTING 返回 true，EXECUTE_DONE / REMOTE_DIED 旧记录不再能验证任何弹窗；
- **状态 CAS 守卫执行**：同意后必须 CAS 成功才执行，弹窗期间调用方死亡
  （死亡监听置 REMOTE_DIED）的记录不会被执行；
- **复用分支防劫持**：`CheckAndUpdateParam` 的 specifiedIntentId 复用分支要求
  同目标 bundle + EXECUTING 状态，调用方无法把执行挂到他人/未同意的记录上。

## 6. 分层落地清单（实现阶段用，本次不含代码）

| 层 | 改动 | 位置（新增/参考） |
|---|---|---|
| 公共 ArkTS API | `.d.ts` 模块 + 类型定义 | `interface_sdk-js`（跨仓）|
| NAPI 绑定 | 新模块 `app.ability.insightIntentFunction`，`invokeFunction` 绑定 + 回调客户端 | `frameworks/js/napi/insight_intent_function/`（新增，参考 `js_insight_intent_driver.cpp`）|
| ANI/ArkTS 直调 | 若需 ArkTS 侧直调可后续补 ANI | `frameworks/ets/ani/insight_intent_function/`（可选，首期可缓）|
| 内部 IPC 客户端 | 新 opcode `EXECUTE_AGENT_INTENT_FUNCTION` + client | `interfaces/inner_api/insight_intent/intent_client/`（参考现有 `intent_client.cpp`）|
| IPC 接口码 | 新增枚举 | `interfaces/inner_api/ability_manager/include/ability_manager_ipc_interface_code.h`（现有 `EXECUTE_INTENT_BY_FUNCTION_CALL` 侧）|
| AMS 服务端 | `ExecuteAgentIntentFunction` + 鉴权分支 + 频控 | `services/abilitymgr/src/ability_manager_service.cpp` |
| 鉴权 | `CheckAgentCallPermission`（normal 权限 + agentCallable） | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp` |
| profile 解析 | 新增 `agentCallable` 可加性字段解析 + 入库 | `services/abilitymgr/src/insight_intent/extract_insight_intent_profile.cpp`（+ 结构体）|
| 权限定义 | 新 normal 权限 | `security/permission` 子系统（跨仓）|
| BUILD.gn | 新增模块编译目标 + 源文件列表 | 各新增目录 `BUILD.gn` |

## 7. 兼容性评估

- **公共 API**：只在新增模块做加法；不动 `insightIntentDriver#execute` 等既有签名/语义。
- **IPC**：新增 opcode，不改既有 `EXECUTE_INSIGHT_INTENT` / `EXECUTE_INTENT_BY_FUNCTION_CALL` 序列化格式，无跨版本兼容风险。
- **profile/RDB**：`agentCallable` 为可加性字段，旧版本解析器忽略、新版本缺省按 `false`，不破坏已有 RDB 数据结构；
  **不**改变既有 `permissions` 字段语义。
- **权限/DFX**：不改 `EXECUTE_INSIGHT_INTENT` 既有判定；新增调用落 `EXECUTE_INSIGHT_INTENT_ERROR` 或新增独立事件（若新增需同步 `hisysevent.yaml` 评审）。
- **FA/Stage**：函数调用为 Stage 模型意图装饰器能力，不涉及 FA 模型。

## 8. 风险与开放问题（待评审）

1. **normal 权限的授权粒度**：normal(user_grant) 权限是「一刀切」授权，任意持有者可调用任意 `agentCallable` 函数；
   若需更细（按目标函数 / 按调用方白名单），需在 profile 增加 `agentCallerAllowlist`（签名/bundle 白名单），首期建议 hold。
2. **权限名与定义归属**：`INVOKE_INSIGHT_INTENT_FUNCTION` 需权限子系统评审与命名规范确认。
3. **是否复用 `ExecuteIntentByFunctionCall` 还是新开 opcode**：本设计倾向**新开 opcode** 以隔离「系统路径/Agent 路径」鉴权，
   避免在共享方法上叠加分支造成回归；代价是多一个接口码。
4. **result 回传 sensitivity**：三方拿不到 `interactionInfo`，但业务 `result` 可能含敏感数据，是否需要「目标函数返回分级」评审。
5. **函数执行副作用**：后台直调函数存在资源/耗时风险，需对齐现有 ttl/超时与 tmetric 打点。

## 9. 里程碑建议

- M1：profile `agentCallable` 解析 + 结构体扩展 + 单测（纯服务端，无公共 API 面变化）。
- M2：新增 opcode + `ExecuteAgentIntentFunction` + 鉴权分支 + 频控 + 服务端单测。
- M3：NAPI 模块 + `.d.ts`（跨仓）+ 模块单测 + 端到端联调。
- M4：安全评审（权限定义、反滥用、敏感字段）+ DFX 事件 + 文档合入。