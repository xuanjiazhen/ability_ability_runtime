# executeSkill 执行规格

## 1. 接口定义

```typescript
executeSkill(skillParam: Record<string, Object>): Promise<ExecuteResult>
```

### 入参 skillParam

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| cmd | string | 是 | 固定值 `"ohos-arktsscript"` |
| bundleName | string | 是 | 目标 App 包名 |
| moduleName | string | 是 | 目标模块名 |
| skillName | string | 是 | Skill 名称 |
| arkTSPath | string | 否 | ArkTS 文件路径 |
| funcName | string | 是 | 目标函数名 |
| skillArgs | Object | 否 | 传递给目标函数的参数，键值对展开为函数参数 |

### 返回值 ExecuteResult

| 字段 | 类型 | 说明 |
|------|------|------|
| code | number | 执行结果码，0 表示成功，-1 表示失败 |
| result | Object | 目标函数返回的结构化数据 (WantParams) |
| uris | string[] | 目标函数返回的 URI 列表，AMS 自动授权给调用方 |
| flags | number | URI 权限标志位 |

## 2. 系统架构

```
┌──────────┐     IPC ①      ┌─────┐    IPC ②     ┌──────────────────┐
│ 调用方 App │ ──────────────▶│ AMS │ ────────────▶│ 目标方 App        │
│           │ ◀────────────── │     │ ◀──────────── │                  │
└──────────┘     IPC ③      └─────┘    IPC ④     └──────────────────┘

IPC ①: ExecuteInAppSkill(bundle, module, skill, path, func, args, callback)
IPC ②: ScheduleCommandAbility(want) 或 ExecuteSkill(want) [热启动]
IPC ③: SkillExecuteCallback::OnExecuteDone(requestCode, resultCode, result) [TF_ASYNC]
IPC ④: ExecuteSkillDone(requestCode, resultCode, result)
```

### 2.1 进程角色

| 角色 | 说明 |
|------|------|
| 调用方 App | 调用 `executeSkill` 的进程（如小艺 claw），持有 `ISkillExecuteCallback` Stub |
| AMS | `AbilityManagerService` 系统服务，中转调度和结果回传 |
| 目标方 App | 执行 Skill 函数的进程，可以是 UIAbility 或 ServiceExtension |

### 2.2 关键数据结构

```
SkillExecuteParam (Want 参数传递):
  bundleName, moduleName, skillName, arkTSPath, funcName,
  skillArgs (WantParams), srcEntries[], requestCode (int64), hapPath

SkillExecuteResult (结果回传):
  code (int32), result (WantParams), uris (string[]), flags (uint32)

SkillExecuteRecord (AMS 内部存储):
  requestCode, targetBundleName, callerBundleName, callerTokenId,
  state (EXECUTING / EXECUTE_DONE / REMOTE_DIED),
  callback (sptr<ISkillExecuteCallback>)
```

## 3. 执行流程

### 3.1 Phase 1: 调用方发起

```
JS: await executeSkill(skillParam)
│
├─ OnExecuteSkill(env, argc, argv)
│  ├─ 校验 cmd == "ohos-arktsscript"
│  ├─ 解析参数 → bundleName, moduleName, skillName, funcName, skillArgs
│  ├─ 创建 deferred Promise
│  │   asyncTask = CreateAsyncTaskWithLastParam(nullptr, nullptr, nullptr, &result)
│  │   callback = sptr<SkillExecuteCallbackImpl>(env, asyncTask)
│  ├─ DispatchExecuteSkill(..., callback)
│  │  ├─ QuerySkillType(bundle, module, skill) → IPC → AMS
│  │  └─ ExecuteInAppSkill(..., callback) → IPC → AMS [Phase 2]
│  ├─ IPC 失败 → asyncTask->Reject(env, error) → Promise reject
│  └─ IPC 成功 → 不做处理，Promise 保持 pending，等待 callback
│
└─ return result (pending Promise)
```

**涉及文件**:
- `cli_tool_framework/.../js_cli_manager.cpp` — OnExecuteSkill, DispatchExecuteSkill, SkillExecuteCallbackImpl

### 3.2 Phase 2: AMS 处理调度

```
AbilityManagerStub::ExecuteInAppSkillInner
├─ 反序列化: bundleName, moduleName, skillName, arkTSPath, funcName, skillArgs
├─ 反序列化: callback → iface_cast<ISkillExecuteCallback>(remoteObject)
└─ AbilityManagerService::ExecuteInAppSkill
   ├─ 1. QuerySkillInfo(bundle, module, skill, userId) → BMS
   │   → SkillInfo { abilityName, srcEntries, hapPath, permissions, type }
   ├─ 2. CheckSkillPermission(skillInfo) → 校验调用方权限
   ├─ 3. CreateExecuteRecord(nullptr, targetBundle, callerBundle, callerTokenId, callback)
   │   → 生成唯一 requestCode (int64, 单调递增)
   │   → 存储 SkillExecuteRecord { requestCode, callback, state=EXECUTING }
   ├─ 4. GenerateSkillWant(skillInfo, want, userId, requestCode, ...)
   │   → want.SetElementName(bundle, abilityName)
   │   → SkillExecuteParam::WriteToWant(want, ..., requestCode, srcEntries, hapPath)
   │   → targetType = SERVICE 或 UNSPECIFIED(UIAbility)
   ├─ 5. 按 targetType 分派:
   │   ├─ SERVICE → StartExtensionAbilityWithSkill [Phase 3B]
   │   └─ 其他   → StartAbilityByCallWithSkill [Phase 3A]
   └─ return ERR_OK → IPC reply
```

**涉及文件**:
- `services/abilitymgr/src/ability_manager_stub.cpp` — ExecuteInAppSkillInner
- `services/abilitymgr/src/ability_manager_service.cpp` — ExecuteInAppSkill
- `services/abilitymgr/src/skill/skill_execute_manager.cpp` — QuerySkillInfo, CheckSkillPermission, CreateExecuteRecord, GenerateSkillWant
- `services/abilitymgr/include/skill/skill_execute_record.h` — SkillExecuteRecord

### 3.3 Phase 3A: UIAbility 分派

```
StartAbilityByCallWithSkill(want, callerToken, userId)
├─ GenerateAbilityRequest → abilityRequest
└─ IsAbilityStarted(abilityRequest)?
   ├─ true (热启动):
   │   targetRecord->GetScheduler()->ExecuteSkill(want)
   │   → AbilitySchedulerProxy::ExecuteSkill (TF_ASYNC, Want)
   │   → UIAbilityThread::ExecuteSkill(want)
   │     → PostTask("UIAbilityThread:ExecuteSkill")
   │     → HandleExecuteSkill(want, onlyExecuteSkill=true) [Phase 4]
   │
   └─ false (冷启动):
       StartAbilityByCall(want, connect, callerToken, userId)
       → 启动目标 UIAbility 进程
       → UIAbilityImpl::Start(want) → onStart → lifecycleState_ = STARTED_NEW
       → AMS 发送 BACKGROUND_NEW 状态切换
       → UIAbilityImpl::AbilityTransaction(want, BACKGROUND_NEW)
         ├─ OnWillBackground()
         ├─ IsSkillExecute(want) → true
         ├─ HandleExecuteSkill(want, onlyExecuteSkill=false) [Phase 4]
         │   └─ 内部调用 Background() → lifecycleState_ = BACKGROUND_NEW
         └─ AbilityTransactionCallback(BACKGROUND_NEW) → 通知 AMS 状态切换完成
```

**涉及文件**:
- `services/abilitymgr/src/ability_manager_service.cpp:14391` — StartAbilityByCallWithSkill
- `services/abilitymgr/src/ability_scheduler_proxy.cpp:1151` — ExecuteSkill (热启动)
- `services/abilitymgr/src/ability_scheduler_stub.cpp:729` — ExecuteSkillInner
- `frameworks/.../ui_ability_thread.cpp:697` — ExecuteSkill → PostTask
- `frameworks/.../ui_ability_impl.cpp:264` — HandleExecuteSkill

### 3.4 Phase 3B: ServiceExtension 分派

```
StartExtensionAbilityWithSkill(want, userId)
→ StartExtensionAbilityInner(want, nullptr, userId, SERVICE, true)
  → CheckOptExtensionAbility (权限校验)
  → connectManager->StartAbility(abilityRequest)
    → StartAbilityLocked:
      → GetOrCreateServiceRecord → isLoadedAbility?

      ├─ false (冷启动):
      │   LoadAbility → 启动 Extension 进程 → OnCreate
      │   → CommandAbility ─┐
      │                      │
      ├─ true + ACTIVE (热启动):    │
      │   SetWant(skillWant) │
      │   CommandAbility ────┘
      │
      └─ true + 非 ACTIVE:
          EnqueueStartServiceReq → 等待 ACTIVE 后再调度

  CommandAbility → abilityRecord->CommandAbility()
    → lifecycleDeal_->CommandAbility(want, restart, startId)
      → ScheduleCommandAbility(want, restart, startId) [TF_ASYNC, Want]

  ExtensionAbilityThread::ScheduleCommandAbility(want, restart, startId)
    ├─ ScheduleCommandAbilityInner → PostTask → OnCommand(want)
    └─ IsSkillExecute(want) → true
       → ScheduleSkillExecuteInner → PostTask → HandleExecuteSkill(want) [Phase 4]
```

**涉及文件**:
- `services/abilitymgr/src/ability_manager_service.cpp:14432` — StartExtensionAbilityWithSkill
- `services/abilitymgr/src/ability_connect_manager.cpp:135` — StartAbilityLocked
- `services/abilitymgr/src/ability_connect_manager.cpp:1837` — CommandAbility
- `frameworks/.../extension_ability_thread.cpp:542` — ScheduleCommandAbility
- `frameworks/.../extension_impl.cpp:435` — HandleExecuteSkill

### 3.5 Phase 4: 目标 App 执行用户函数

```
HandleExecuteSkill(want)
→ JsUIAbility::ExecuteSkill / JsServiceExtension::ExecuteSkill
  ├─ SkillExecuteParam::GenerateFromWant(want, param)
  ├─ 遍历 srcEntries 加载模块:
  │   srcPath = moduleName + "/" + srcEntry → ".abc"
  │   moduleRef = jsRuntime_.LoadModule(moduleName, srcPath, "", true)
  │   method = GetPropertyValueByPropertyName(env, jsObj, funcName)
  │   └─ 找到 method → break
  ├─ method == null:
  │   ReportSkillDone(requestCode, -1) → [Phase 6] → 调用方收到 code=-1
  ├─ 构造 JS 参数:
  │   info = { requestCode: BigInt, context: AbilityContext/ServiceExtensionContext }
  │   args = [info, skillArgs.key1, skillArgs.key2, ...]
  ├─ napi_call_function(env, jsObj, method, argc, args, &result)
  │   → 目标 App 开发者的 ArkTS 函数开始执行
  └─ napi_call_function 失败:
      ReportSkillDone(requestCode, -1) → [Phase 6]
```

**涉及文件**:
- `frameworks/.../js_ui_ability.cpp:2516` — JsUIAbility::ExecuteSkill
- `frameworks/.../js_service_extension.cpp:582` — JsServiceExtension::ExecuteSkill

### 3.6 Phase 5: 目标 App 上报结果

```
目标 App 开发者代码:
  cliToolManager.completeArkTSScriptInApp(context, requestCode, executeResult)

OnCompleteArkTSScriptInApp(env, argc, argv)
├─ VerifyAbilityContext(argv[0])
├─ requestCode = argv[1] (int64)
├─ UnwrapSkillExecuteResult(argv[2])
│   → SkillExecuteResult { code, result(WantParams), uris, flags }
└─ NapiAsyncTask::Schedule → worker 线程:
   AbilityManagerClient::ExecuteSkillDone(requestCode, resultCode, skillResult)
   → IPC → AMS [Phase 6]
```

**涉及文件**:
- `cli_tool_framework/.../js_cli_manager.cpp` — OnCompleteArkTSScriptInApp
- `frameworks/js/napi/inner/napi_common/napi_common_skill_execute.cpp` — UnwrapSkillExecuteResult

### 3.7 Phase 6: AMS 回传结果

```
AbilityManagerStub::ExecuteSkillDoneInner
├─ 反序列化: requestCode(int64) + resultCode(int32) + SkillExecuteResult
└─ AbilityManagerService::ExecuteSkillDone
   → SkillExecuteManager::ExecuteSkillDone(requestCode, resultCode, result)
     ├─ 查找 records_[requestCode] → SkillExecuteRecord
     ├─ record->state = EXECUTE_DONE
     ├─ GrantUriPermission(result.uris, callerBundleName) [如有 uris]
     ├─ record->callback->OnExecuteDone(requestCode, resultCode, result)
     │   → SkillExecuteCallbackProxy::OnExecuteDone
     │     ├─ 序列化: interfaceToken + requestCode(int64) + resultCode(int32) + SkillExecuteResult
     │     └─ SendRequest(ON_SKILL_EXECUTE_DONE, TF_ASYNC) → IPC [Phase 7]
     └─ RemoveRecord(requestCode)
```

**涉及文件**:
- `services/abilitymgr/src/ability_manager_stub.cpp` — ExecuteSkillDoneInner
- `services/abilitymgr/src/ability_manager_service.cpp` — ExecuteSkillDone
- `services/abilitymgr/src/skill/skill_execute_manager.cpp` — ExecuteSkillDone
- `services/abilitymgr/src/skill/skill_execute_callback_proxy.cpp` — OnExecuteDone

### 3.8 Phase 7: 调用方收到结果

```
[binder 线程]
SkillExecuteCallbackStub::OnRemoteRequest(ON_SKILL_EXECUTE_DONE)
├─ 校验 interface token
└─ OnExecuteDoneInner(data, reply)
   ├─ 反序列化: requestCode(int64) + resultCode(int32) + SkillExecuteResult
   └─ SkillExecuteCallbackImpl::OnExecuteDone(requestCode, resultCode, result)
     ├─ resultCopy = result (拷贝)
     └─ JsCliEventHandlerManager::PostTask → [主线程事件队列]

[主线程]
├─ WrapSkillExecuteResult(env, resultCopy)
│   → JS 对象: { code: number, result: Object, uris: string[], flags: number }
└─ asyncTask_->ResolveWithNoError(env, jsResult)
  → resolve Phase 1 创建的 deferred Promise

[JS 层]
const result = await executeSkill(param);
// result = { code: 0, result: {...}, uris: [...], flags: 1 }
```

**涉及文件**:
- `services/abilitymgr/src/skill/skill_execute_callback_stub.cpp` — OnRemoteRequest, OnExecuteDoneInner
- `cli_tool_framework/.../js_cli_manager.cpp` — SkillExecuteCallbackImpl::OnExecuteDone
- `frameworks/js/napi/inner/napi_common/napi_common_skill_execute.cpp` — WrapSkillExecuteResult

## 4. 文件清单

### 4.1 新增文件

| 文件 | 说明 |
|------|------|
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_callback_interface.h` | ISkillExecuteCallback IPC 接口 |
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_callback_stub.h` | 回调 Stub 基类 |
| `interfaces/inner_api/ability_manager/include/skill/skill_execute_callback_proxy.h` | 回调 Proxy |
| `services/abilitymgr/src/skill/skill_execute_callback_stub.cpp` | Stub 实现（调用方进程接收回调） |
| `services/abilitymgr/src/skill/skill_execute_callback_proxy.cpp` | Proxy 实现（AMS 调用回调，TF_ASYNC） |

### 4.2 修改文件

| 层级 | 文件 | 修改内容 |
|------|------|----------|
| IPC 接口 | `ability_manager_interface.h` | ExecuteInAppSkill 签名增加 callback 参数 |
| IPC Proxy | `ability_manager_proxy.h/.cpp` | 序列化 callback RemoteObject |
| IPC Stub | `ability_manager_stub.cpp` | 反序列化 callback，iface_cast |
| Client | `ability_manager_client.h/.cpp` | 透传 callback |
| AMS | `ability_manager_service.h/.cpp` | 传递 callback 给 CreateExecuteRecord |
| AMS | `skill_execute_record.h` | 增加 callback 字段 |
| AMS | `skill_execute_manager.h/.cpp` | CreateExecuteRecord 接收 callback；ExecuteSkillDone 调用 callback |
| 调用方 | `js_cli_manager.cpp` | OnExecuteSkill 改 deferred Promise；新增 SkillExecuteCallbackImpl |
| NAPI | `napi_common_skill_execute.h/.cpp` | 新增 WrapSkillExecuteResult |
| 构建 | `abilitymgr.gni` | 添加 callback stub/proxy 源文件 |
| 构建 | `inner_api/ability_manager/BUILD.gn` | 添加 callback stub/proxy 源文件 |

## 5. 异常处理

| 异常场景 | 触发点 | 处理方式 | 调用方感知 |
|----------|--------|----------|------------|
| cmd 校验失败 | Phase 1 | Throw JS Error | JS 异常 |
| 参数解析失败 | Phase 1 | Throw JS Error | JS 异常 |
| QuerySkillType 失败 | Phase 1 | asyncTask->Reject | Promise reject(BusinessError) |
| QuerySkillInfo 失败 | Phase 2 | 返回错误码 | Promise reject(BusinessError) |
| 权限校验失败 | Phase 2 | 返回错误码 | Promise reject(BusinessError) |
| 目标 Ability 启动失败 | Phase 2 | 返回错误码 | Promise reject(BusinessError) |
| GenerateFromWant 失败 | Phase 4 | return true，不上报 | Promise 永远 pending |
| funcName 未找到 | Phase 4 | ReportSkillDone(requestCode, -1) | resolve({ code: -1 }) |
| napi_call_function 失败 | Phase 4 | ReportSkillDone(requestCode, -1) | resolve({ code: -1 }) |
| 目标方不调 completeArkTSScriptInApp | — | 无超时机制 | Promise 永远 pending |
| callback IPC 发送失败 | Phase 6 | TF_ASYNC 不重试 | Promise 永远 pending |
| 目标进程崩溃 | Phase 4 | DeathRecipient 感知（待实现） | Promise 永远 pending |

## 6. 线程模型

| 阶段 | 执行线程 | 说明 |
|------|----------|------|
| OnExecuteSkill | JS 主线程 | 解析参数、创建 Promise、同步 IPC |
| DispatchExecuteSkill | JS 主线程 | QuerySkillType + ExecuteInAppSkill 同步 IPC |
| OnCompleteArkTSScriptInApp | JS 主线程 → worker | Schedule 投递 IPC 到 worker |
| SkillExecuteCallbackImpl::OnExecuteDone | binder 线程 → JS 主线程 | PostTask 投递 napi 操作到主线程 |
| UIAbilityThread::ExecuteSkill | binder 线程 → 主线程 | PostTask 投递到 Ability Handler |
| ExtensionAbilityThread::ScheduleSkillExecuteInner | binder 线程 → 主线程 | PostTask 投递到 Ability Handler |

## 7. 编译依赖

```
climanager_napi (调用方 NAPI 模块)
  └─ ability_manager (inner API)
       ├─ skill_execute_callback_stub.cpp  ← 调用方进程链接
       ├─ skill_execute_callback_proxy.cpp
       ├─ skill_execute_param.cpp
       └─ skill_execute_result.cpp

abilityms (AMS 服务模块)
  └─ skill_execute_callback_proxy.cpp  ← AMS 进程链接
     skill_execute_callback_stub.cpp
     skill_execute_manager.cpp
     skill_execute_result.cpp
```
