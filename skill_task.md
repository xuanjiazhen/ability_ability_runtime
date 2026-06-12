# Skill 执行流程分析 — 从 ExecTool 到目标 App 回调

## 入口：CliToolManagerService::ExecTool

**文件**: `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:606`

```
ExecTool(param, eventId, scheduler)
│
├─ 1. ValidateExecToolPermissions()          权限校验 (EXEC_CLI_TOOL)
├─ 2. EventDispatcher::SetScheduler()        注册回调调度器
├─ 3. TryDispatchSkillSession()              判断是否走 skill 分支 ★
│     ├─ ToolUtil::IsSkillTool(toolName)     判断工具名是否为 skill 类型
│     ├─ ValidateSkillTypeFromParam()        解析 args 中的 skillName 等参数
│     │   ├─ args.GetStringParam("bundleName")
│     │   ├─ args.GetStringParam("moduleName")
│     │   ├─ args.GetStringParam("skillName")   ← 必填
│     │   └─ ValidateSkillType()
│     │       └─ AbilityManagerClient::QuerySkillType()  → IPC → AMS
│     │           返回 skillType (0=普通, -1=独立)
│     │
│     ├─ skillType == -1 → 返回, 走 CLI 子进程路径
│     └─ skillType != -1 → SetupAndStartSkillSession()  ★ 进入 skill 流程
│
└─ 否则走 CLI 子进程路径 (SetupAndStartSession)
```

---

## Phase 1: CliToolManagerService — Skill Session 建立

**文件**: `cli_tool_manager_service.cpp:1083`

```
SetupAndStartSkillSession(param, eventId, toolInfo)
│
├─ 1. 从 param.args 提取参数:
│     bundleName, moduleName, skillName, scriptPath, functionName
│
├─ 2. ToolUtil::ExpandArgsJsonString(args)     展开嵌套 JSON
│   └─ ToolUtil::FilterSkillArgs(args)         过滤出 skill 专用参数 → skillArgs
│
├─ 3. CreateSessionRecord(param, eventId)
│     → 生成 sessionId, 设置 callerPid/Uid, state=RUNNING
│
├─ 4. record->sessionType = SessionType::SKILL   标记为 skill 类型会话
│   └─ AddSessionRecord(record)                  存入 sessionRecords_ map
│
├─ 5. 创建 SkillCallbackAdapter (回调适配器):
│     → 持有 sessionId, callerPid, callerUid, eventId
│     → 继承 SkillExecuteCallbackStub, 实现 OnExecuteDone
│
├─ 6. 构造 SkillExecuteRequest:
│     { callerTokenId, bundleName, moduleName, skillName,
│       scriptPath, functionName, skillArgs }
│
└─ 7. AbilityManagerClient::ExecuteInAppSkillWithTokenId(skillRequest, adaptor)
      → IPC 调用 → AMS 服务端                                   ← Phase 2
      → 失败则 RemoveSessionRecord 并返回错误码
```

### SkillCallbackAdapter::OnExecuteDone (回调入口)

**文件**: `cli_tool_manager_service.cpp:1026`

```
OnExecuteDone(requestCode, resultCode, result)
│
├─ 获取 service 和 record (通过 sessionId)
├─ 提取 result.result->ToString() → outputText
├─ record->SetSkillResult(resultCode, outputText)
├─ record->SetState(COMPLETED / FAILED)
├─ ToolUtil::BuildSkillSessionInfo() → CliSessionInfo
└─ HandleSkillSessionComplete(sessionId, callerPid, callerUid, eventId, ...)
     ├─ record->BeginCleanup()
     ├─ DispatchExecToolReplyEvent()   通知调用方结果
     ├─ DispatchExitEvent()            通知会话结束
     ├─ ClearSessionSubscribers()      清理订阅
     └─ RemoveSessionRecord()          移除会话记录
```

---

## Phase 2: AbilityManagerService — 调度中心

**文件**: `services/abilitymgr/src/ability_manager_service.cpp:14852`

```
ExecuteInAppSkillWithTokenId(request, callback)
│
├─ 1. 从 request.callerTokenId 解析:
│     → AccessTokenKit::GetHapTokenInfo → userId, callerBundleName
│
├─ 2. SkillExecuteManager::QuerySkillInfo(bundle, module, skill, userId)
│     → BMS 查询 SkillInfo { abilityName, srcEntries, hapPath, permissions, type }
│
├─ 3. SkillExecuteManager::CheckSkillPermission(skillInfo, callerTokenId)
│     → 校验调用方是否有权限执行此 skill
│
├─ 4. SkillExecuteManager::CreateExecuteRecord(nullptr, targetBundle, callerBundle, tokenId, callback)
│     → 生成唯一 requestCode (单调递增 int64)
│     → 存储 SkillExecuteRecord { requestCode, callback, state=EXECUTING }
│
├─ 5. SkillExecuteManager::GenerateSkillWant(skillInfo, want, userId, requestCode, targetType, ...)
│     → want.SetElementName(bundle, abilityName)
│     → SkillExecuteParam::WriteToWant(want, ..., requestCode, srcEntries, hapPath)
│     → 确定 targetType = SERVICE 或 UNSPECIFIED(UIAbility)
│
├─ 6. want.SetParam(SKILL_EXECUTE_PARAM_CALLER_TOKEN_ID, callerTokenId)
│
└─ 7. 按 targetType 分派:
     ├─ SERVICE    → StartExtensionAbilityWithSkill(want, userId)    ← Phase 3B
     └─ 其他       → StartAbilityByCallWithSkill(want, nullptr, userId) ← Phase 3A
```

---

## Phase 3A: UIAbility 分派路径

**文件**: `ability_manager_service.cpp:14908`

```
StartAbilityByCallWithSkill(want, callerToken, userId)
│
├─ 创建 AbilityBackgroundConnection
├─ GenerateAbilityRequest(want, ...) → abilityRequest
│
└─ IsAbilityStarted(abilityRequest, targetRecord)?
   │
   ├─ true (热启动):
   │   targetRecord->GetScheduler()->ExecuteSkill(want)
   │   → AbilitySchedulerProxy::ExecuteSkill (TF_ASYNC)
   │   → UIAbilityThread::ExecuteSkill → PostTask → 主线程
   │     → HandleExecuteSkill(want, onlyExecuteSkill=true)         ← Phase 4
   │
   └─ false (冷启动):
       StartAbilityByCall(want, connect, callerToken, userId)
       → 启动目标 UIAbility 进程
       → UIAbilityImpl::Start(want) → onStart
       → AMS 发送 BACKGROUND_NEW 状态
       → UIAbilityImpl::AbilityTransaction(want, BACKGROUND_NEW)
         ├─ OnWillBackground()
         ├─ IsSkillExecute(want) → true
         ├─ HandleExecuteSkill(want, onlyExecuteSkill=false)       ← Phase 4
         └─ AbilityTransactionCallback(BACKGROUND_NEW)
```

## Phase 3B: ServiceExtension 分派路径

**文件**: `ability_manager_service.cpp:14949`

```
StartExtensionAbilityWithSkill(want, userId)
│
└─ StartExtensionAbilityInner(want, nullptr, userId, SERVICE, true)
   → AbilityConnectManager::StartAbilityLocked
     │
     ├─ 冷启动: LoadAbility → 启动进程 → OnCreate → CommandAbility
     ├─ 热启动(ACTIVE): SetWant(skillWant) → CommandAbility
     └─ 热启动(非ACTIVE): 入队等待 → ACTIVE 后调度
     │
     CommandAbility → ScheduleCommandAbility(want) [TF_ASYNC]
       → ExtensionAbilityThread::ScheduleCommandAbility
         ├─ ScheduleCommandAbilityInner → PostTask → OnCommand(want)
         └─ IsSkillExecute(want) → true
            → ScheduleSkillExecuteInner → PostTask
              → HandleExecuteSkill(want)                               ← Phase 4
```

---

## Phase 4: 目标 App — 执行用户函数

**文件**: `frameworks/js/napi/inner/` 相关文件

```
HandleExecuteSkill(want)
│
├─ SkillExecuteParam::GenerateFromWant(want, param)
│
├─ 遍历 srcEntries 加载模块:
│   moduleRef = jsRuntime_.LoadModule(moduleName, srcPath)
│   method = GetPropertyValueByPropertyName(env, jsObj, funcName)
│   └─ 找到 method → break
│
├─ method == null:
│   ReportSkillDone(requestCode, -1)  → Phase 6 → 调用方收到 code=-1
│
├─ 构造 JS 参数:
│   info = { requestCode: BigInt, context: AbilityContext }
│   args = [info, skillArgs.key1, skillArgs.key2, ...]
│
├─ napi_call_function(env, jsObj, method, argc, args, &result)
│   → 开发者的 ArkTS 函数开始执行
│
└─ napi_call_function 失败:
    ReportSkillDone(requestCode, -1)  → Phase 6
```

---

## Phase 5: 目标 App — 上报结果

```
开发者代码:
  cliToolManager.completeArkTSScriptInApp(context, requestCode, executeResult)

OnCompleteArkTSScriptInApp(env, argc, argv)
│
├─ VerifyAbilityContext(argv[0])
├─ 提取 requestCode (int64)
├─ UnwrapSkillExecuteResult(argv[2])
│   → SkillExecuteResult { code, result(WantParams), uris, flags }
│
└─ AbilityManagerClient::ExecuteSkillDone(requestCode, resultCode, skillResult)
   → IPC → AMS                                                     ← Phase 6
```

---

## Phase 6: AMS — 回传结果

**文件**: `ability_manager_service.cpp:14956`

```
ExecuteSkillDone(token, requestCode, resultCode, result)
│
├─ Token::GetAbilityRecordByToken(token) → 校验 self-called
├─ SkillExecuteManager::ExecuteSkillDone(requestCode, resultCode, result, bundleName)
│   ├─ 查找 records_[requestCode] → SkillExecuteRecord
│   ├─ record->state = EXECUTE_DONE
│   ├─ GrantUriPermission(result.uris, callerBundleName)  [如有 uris]
│   ├─ record->callback->OnExecuteDone(requestCode, resultCode, result)
│   │   → SkillCallbackAdapter::OnExecuteDone                     ← 回到 Phase 1 的回调
│   └─ RemoveRecord(requestCode)
```

---

## 完整调用链路总结

```
调用方 (小艺/claw)                   CliToolMgr                     AMS                          目标 App
──────────────                ────────────                  ──────────                   ──────────
ExecTool(param)
  │
  ├─ TryDispatchSkillSession()
  │   ├─ QuerySkillType()  ──────IPC─────▶  QuerySkillType
  │   │                                       └─ BMS 查询
  │   │◀─────── skillType ───────
  │   │
  │   └─ SetupAndStartSkillSession()
  │       ├─ 创建 SessionRecord (SKILL)
  │       ├─ 创建 SkillCallbackAdapter
  │       └─ ExecuteInAppSkillWithTokenId()
  │            └──────────IPC──────────▶  ExecuteInAppSkillWithTokenId
  │                                        ├─ QuerySkillInfo (BMS)
  │                                        ├─ CheckSkillPermission
  │                                        ├─ CreateExecuteRecord + callback
  │                                        ├─ GenerateSkillWant
  │                                        └─ StartAbility/Extension
  │                                             └────────────────▶  HandleExecuteSkill
  │                                                                  ├─ LoadModule
  │                                                                  └─ napi_call_function
  │                                                                       → 开发者函数执行
  │
  │                                                              completeArkTSScriptInApp()
  │                                                                   └────IPC────▶ ExecuteSkillDone
  │                                                                                  ├─ 查找 record
  │                                                                                  ├─ 授权 URI
  │                                                                                  └─ callback->OnExecuteDone
  │                                                                                       └──IPC──▶ SkillCallbackAdapter
  │                                                                                                 ::OnExecuteDone
  │◀────────── DispatchExecToolReplyEvent ───────────────────────────────────────────────────────────
  │  (CliSessionInfo: 结果数据)
  │
  └─ DispatchExitEvent → 会话结束
```

---

## 关键数据结构

### SkillExecuteRequest (CliTool → AMS)

| 字段 | 类型 | 说明 |
|------|------|------|
| callerTokenId | uint32_t | 调用方 Token ID |
| bundleName | string | 目标 App 包名 |
| moduleName | string | 目标模块名 |
| skillName | string | Skill 名称 |
| scriptPath | string | ArkTS 文件路径 |
| functionName | string | 目标函数名 |
| skillArgs | shared_ptr\<WantParams\> | 函数参数 |

### SkillExecuteResult (目标 App → 调用方)

| 字段 | 类型 | 说明 |
|------|------|------|
| code | int32_t | 0=成功, -1=失败 |
| result | shared_ptr\<WantParams\> | 结构化返回数据 |
| uris | vector\<string\> | URI 列表 (AMS 自动授权) |
| flags | uint32_t | URI 权限标志位 |

### SessionRecord (CliToolMgr 内部)

| 字段 | 说明 |
|------|------|
| sessionType | SessionType::SKILL 标识 |
| sessionId | 唯一会话 ID |
| callerPid/Uid | 调用方进程信息 |
| eventId | 事件 ID |
| state | RUNNING → COMPLETED/FAILED |

---

## 文件清单

| 阶段 | 文件 | 关键方法 |
|------|------|----------|
| 入口 | `cli_tool_manager_service.cpp` | ExecTool, TryDispatchSkillSession, SetupAndStartSkillSession |
| 回调 | `cli_tool_manager_service.cpp` | SkillCallbackAdapter::OnExecuteDone, HandleSkillSessionComplete |
| IPC Client | `ability_manager_client.cpp` | ExecuteInAppSkillWithTokenId, QuerySkillType |
| IPC Proxy | `ability_manager_proxy.cpp` | 序列化 skillRequest + callback |
| IPC Stub | `ability_manager_stub.cpp` | ExecuteInAppSkillWithTokenIdInner 反序列化 |
| AMS 调度 | `ability_manager_service.cpp` | ExecuteInAppSkillWithTokenId, StartAbilityByCallWithSkill, StartExtensionAbilityWithSkill |
| Skill 管理 | `skill_execute_manager.cpp` | QuerySkillInfo, CheckSkillPermission, CreateExecuteRecord, GenerateSkillWant, ExecuteSkillDone |
| Skill 记录 | `skill_execute_record.h` | SkillExecuteRecord |
| UIAbility 分派 | `ui_ability_thread.cpp` | ExecuteSkill → PostTask |
| UIAbility 执行 | `ui_ability_impl.cpp` | HandleExecuteSkill |
| Extension 分派 | `extension_ability_thread.cpp` | ScheduleCommandAbility + ScheduleSkillExecuteInner |
| Extension 执行 | `extension_impl.cpp` | HandleExecuteSkill |
| 结果上报 | `js_cli_manager.cpp` | OnCompleteArkTSScriptInApp |
| 结果反序列 | `napi_common_skill_execute.cpp` | UnwrapSkillExecuteResult, WrapSkillExecuteResult |

---

## 异常处理

| 场景 | 触发点 | 处理 |
|------|--------|------|
| skillName 为空 | ValidateSkillTypeFromParam | 返回 ERR_INVALID_VALUE |
| QuerySkillType 失败 | TryDispatchSkillSession | 返回错误码, 不创建 session |
| ExecuteInAppSkillWithTokenId 失败 | SetupAndStartSkillSession | RemoveSessionRecord, 返回错误码 |
| 目标函数未找到 | Phase 4 | ReportSkillDone(requestCode, -1) |
| napi_call_function 失败 | Phase 4 | ReportSkillDone(requestCode, -1) |
| 目标方不上报结果 | Phase 5 | 无超时, session 永远 pending |
| 目标进程崩溃 | Phase 4 | AppAppStateObserver → OnProcessDied 清理 |
| skill session 超时 | HandleSkillSessionTimeout | 设置 FAILED, 清理 session |

---

## 待修复：CheckSkillPermission 权限校验缺失

### 问题分析

`ExecuteInAppSkillWithTokenId` 是 **CliToolMgr SA 代理模式**：
- IPC 实际调用者 = CliToolMgr（系统 SA，TOKEN_NATIVE）
- 被代理的间接调用者 = 某个 App（由 `request.callerTokenId` 指定）

**当前 `CheckSkillPermission` 实现** (`skill_execute_manager.cpp:85`):

```cpp
int32_t SkillExecuteManager::CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo,
    uint32_t callerTokenId)
{
    auto permVerif = PermissionVerification::GetInstance();
    if (callerTokenId == 0) {
        // 直接调用路径（ExecuteInAppSkill）：检查 IPC 调用者是否为系统应用
        if (!permVerif->JudgeCallerIsAllowedToUseSystemAPI()) {
            return ERR_NOT_SYSTEM_APP;
        }
    }
    // callerTokenId != 0 时：什么都不检查 ⚠️ 直接通过
    return ERR_OK;
}
```

**缺失的校验**：

| # | 缺失点 | 风险 |
|---|--------|------|
| 1 | 未验证 IPC 直接调用者是否为 SA | 任意进程可伪造 IPC 包直接调用此接口 |
| 2 | `callerTokenId != 0` 时跳过了所有权限检查 | 间接调用者无需拥有 skill 配置的 permissions |

**关于 `callerTokenId == 0` 分支**：`ExecuteInAppSkill`（不带 TokenId）除测试外无任何生产代码调用，所有实际链路走 `ExecuteInAppSkillWithTokenId`。该分支已移除。

### 修复方案

在 `CheckSkillPermission` 中增加两层校验：

1. **直接调用者必须是 SA** — 使用 `PermissionVerification::IsSACall()` 校验 IPC 调用者
2. **间接调用者必须拥有 skill 的 permissions** — 使用 `AccessTokenKit::VerifyAccessToken(callerTokenId, permission)` 校验

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | 重写 `CheckSkillPermission`，移除 `callerTokenId == 0` 分支，增加两层校验 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | 移除 `callerTokenId` 默认参数 `= 0` |
| `services/abilitymgr/src/ability_manager_service.cpp` | `ExecuteInAppSkill` 调用处补上 `IPCSkeleton::GetCallingTokenID()` 参数 |

### 修改后的 `CheckSkillPermission` 逻辑

```
CheckSkillPermission(skillInfo, callerTokenId)
│
├─ 第一层：IsSACall()  IPC 调用者必须是 SA (TOKEN_NATIVE)
│   └─ 失败 → 返回 ERR_NOT_SYSTEM_APP
│
└─ 第二层：校验间接调用者（callerTokenId）是否拥有 skill 配置的权限
    └─ 遍历 skillInfo.permissions:
        └─ VerifyAccessToken(callerTokenId, permission)
            └─ 任一权限未授予 → 返回 ERR_PERMISSION_DENIED
```

### 关键 API

| API | 用途 | 所在文件 |
|-----|------|----------|
| `PermissionVerification::IsSACall()` | 检查 IPC 调用者是否为 TOKEN_NATIVE | `services/common/include/permission_verification.h` |
| `AccessTokenKit::VerifyAccessToken(tokenId, permission, false)` | 检查指定 token 是否拥有某权限 | `accesstoken_kit.h` |

### 实际实现

```cpp
int32_t SkillExecuteManager::CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo,
    uint32_t callerTokenId)
{
    auto permVerif = PermissionVerification::GetInstance();
    if (!permVerif->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not SA");
        return ERR_NOT_SYSTEM_APP;
    }

    for (const auto &permission : skillInfo.permissions) {
        if (permission.empty()) {
            continue;
        }
        auto ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
            callerTokenId, permission, false);
        if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "caller lacks permission:%{public}s for skill:%{public}s",
                permission.c_str(), skillInfo.skillName.c_str());
            return ERR_PERMISSION_DENIED;
        }
    }
    return ERR_OK;
}
```

---

## 待修复：ExecuteInAppSkillWithTokenId 缺少 callback 死亡监听

### 问题分析

`ExecuteInAppSkillWithTokenId` 路径中，`CreateExecuteRecord` 的第一个参数 `callerToken` 传的是 `nullptr`：

```cpp
// ability_manager_service.cpp:14885
CreateExecuteRecord(nullptr, request.bundleName, callerBundleName,
    request.callerTokenId, callback);
```

导致 `CreateExecuteRecord` 中 `if (callerToken != nullptr)` 永远不进入，没有注册任何死亡监听。

### Binder 对象关系

```
CliToolMgr 进程                          AMS 进程
┌──────────────────┐               ┌──────────────────┐
│ SkillCallbackAdapter               │  SkillExecuteRecord
│  (Binder Stub)   │ ←── Binder ──→ │  .callback       │
│                  │     BpBinder   │  (指向左边的代理) │
└──────────────────┘               └──────────────────┘
```

AMS 持有的是 CliToolMgr 中 `SkillCallbackAdapter` 的 BpBinder 代理。

### 触发条件

`OnRemoteDied` 唯一触发场景：**CliToolMgr 进程崩溃或被杀**。

| 场景 | 是否触发 | 原因 |
|------|----------|------|
| CliToolMgr 进程崩溃/被杀 | 触发 | Binder 驱动感知进程死亡 |
| 目标 App 进程崩溃 | 不触发 | callback 不在目标 App 进程 |
| 调用方 App 崩溃 | 不触发 | callback 不在调用方进程 |

### 不修复的风险

CliToolMgr 崩溃时，AMS record 停留在 `EXECUTING` 状态：
- 目标 App 执行完调 `ExecuteSkillDone`，`callback->OnExecuteDone` 对端已死，失败无处理
- record 无意义占用超时时间等待 `PostSkillExecuteTimeout` 兜底清理

### 修复方案

对 callback 的 `AsObject()` 注册 `DeathRecipient`，CliToolMgr 死亡后立即清理 record。

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/include/skill/skill_execute_record.h` | 新增 `callbackDeathRecipient` 字段 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | 新增 `OnCallbackDied` 方法声明 |
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | `CreateExecuteRecord` 中对 callback 加监听；新增 `OnCallbackDied`；`RemoveRecord` 中移除 |

### 1. skill_execute_record.h

新增字段：

```cpp
struct SkillExecuteRecord {
    // ... 现有字段 ...
    sptr<IRemoteObject::DeathRecipient> callbackDeathRecipient = nullptr;  // 新增
};
```

### 2. skill_execute_manager.h

新增方法声明：

```cpp
void OnCallbackDied(const std::string &requestCode);
```

### 3. skill_execute_manager.cpp — CreateExecuteRecord

在现有 `callerToken` 监听逻辑之后补充：

```cpp
// 现有：监听 callerToken
if (callerToken != nullptr) {
    auto deathRecipient = sptr<CallerDeathRecipient>::MakeSptr(
        [this](const std::string &reqCode) { OnCallerDied(reqCode); }, requestCode);
    callerToken->AddDeathRecipient(deathRecipient);
    record->deathRecipient = deathRecipient;
}

// 新增：callerToken 为空时，监听 callback 的死亡
if (callerToken == nullptr && callback != nullptr) {
    auto cbDeathRecipient = sptr<CallerDeathRecipient>::MakeSptr(
        [this](const std::string &reqCode) { OnCallbackDied(reqCode); }, requestCode);
    callback->AsObject()->AddDeathRecipient(cbDeathRecipient);
    record->callbackDeathRecipient = cbDeathRecipient;
}
```

复用现有 `CallerDeathRecipient` 类，回调目标换成 `OnCallbackDied`。

### 4. skill_execute_manager.cpp — OnCallbackDied

```cpp
void SkillExecuteManager::OnCallbackDied(const std::string &requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callback died, requestCode:%{public}s", requestCode.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it != records_.end()) {
        RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
        it->second->state = SkillExecuteState::REMOTE_DIED;
        records_.erase(it);
    }
}
```

### 5. skill_execute_manager.cpp — RemoveRecord

补充移除 callback 死亡监听：

```cpp
void SkillExecuteManager::RemoveRecord(const std::string &requestCode)
{
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        return;
    }
    auto record = it->second;
    if (record->callerToken != nullptr && record->deathRecipient != nullptr) {
        record->callerToken->RemoveDeathRecipient(record->deathRecipient);
    }
    // 新增：移除 callback 死亡监听
    if (record->callback != nullptr && record->callbackDeathRecipient != nullptr) {
        record->callback->AsObject()->RemoveDeathRecipient(record->callbackDeathRecipient);
    }
    records_.erase(it);
}
```

---

## 待清理：GenerateSkillWant 中多余的 abilityName 空判断

### 问题分析

`GenerateSkillWant` 中有一段 `abilityName` 为空时的兜底逻辑，调用 `ResolveDefaultAbilityName` 从 BMS 查询模块的 `mainAbility`。

实际上 **BMS 安装阶段已经做了兜底**（`module_profile.cpp:3027`）：

```cpp
skill.abilityName = skillProfile.abilityName.empty() ?
    innerModuleInfo.mainAbility : skillProfile.abilityName;
```

查询时（`bundle_data_mgr.cpp:14365`）直接用 `profile.abilityName`，此时已经保证非空。

AMS 侧的 `ResolveDefaultAbilityName` 额外做了一次 `GetBundleInfoV9` IPC 调用，完全多余。

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | `GenerateSkillWant` 中删除空判断分支，直接使用 `skillInfo.abilityName`；删除 `ResolveDefaultAbilityName` 方法实现 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | 删除 `ResolveDefaultAbilityName` 方法声明 |

### 修改前

```cpp
std::string abilityName = skillInfo.abilityName;
if (abilityName.empty()) {
    abilityName = ResolveDefaultAbilityName(skillInfo.bundleName, skillInfo.moduleName, userId);
    if (abilityName.empty()) {
        TAG_LOGE(...);
        return ERR_INVALID_VALUE;
    }
    TAG_LOGD(...);
}
targetType = ResolveTargetType(...);
```

### 修改后

```cpp
const std::string &abilityName = skillInfo.abilityName;
targetType = ResolveTargetType(...);
```

---

## 待修复：ResolveTargetType 应显式校验目标类型

### 问题分析

当前 `ResolveTargetType` 用排除法：查 `QueryExtensionAbilityInfos`，查不到就当 UIAbility。

```cpp
// skill_execute_manager.cpp:303
bool ret = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(...));
if (ret && !extensionInfos.empty()) {
    return extensionInfos[0].type;  // 是 Extension
}
return AppExecFwk::ExtensionAbilityType::UNSPECIFIED;  // 排除法：不是 Extension 就是 UIAbility
```

问题：如果 abilityName 不存在或配置了不支持的类型，也会走到 UIAbility 分支，报错不直观。

**实际仅支持两种目标**：UIAbility 和 ServiceExtension。应显式校验，不支持的类型直接报错。

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | 重写 `ResolveTargetType`，先查 Extension 再查 UIAbility，都不匹配则报错 |
| `services/abilitymgr/include/skill/skill_execute_manager.h` | 返回值从 `ExtensionAbilityType` 改为 `int32_t`，targetType 通过输出参数返回 |

### 修改前

```cpp
AppExecFwk::ExtensionAbilityType SkillExecuteManager::ResolveTargetType(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &abilityName, int32_t userId)
{
    // ... 查 Extension，查不到就返回 UNSPECIFIED
}
```

调用方：
```cpp
targetType = ResolveTargetType(bundleName, moduleName, abilityName, userId);
// 无错误检查，直接用 targetType 分流
```

### 修改后

**声明改为返回错误码**：

```cpp
int32_t ResolveTargetType(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName,
    int32_t userId, AppExecFwk::ExtensionAbilityType &targetType);
```

**实现逻辑**：

```cpp
int32_t SkillExecuteManager::ResolveTargetType(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName,
    int32_t userId, AppExecFwk::ExtensionAbilityType &targetType)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BundleMgrHelper");
        return ERR_INVALID_VALUE;
    }

    // 1. 先查是否为 ServiceExtension
    Want queryWant;
    queryWant.SetElementName("", bundleName, abilityName, moduleName);
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    auto flags = static_cast<int32_t>(
        AppExecFwk::GetExtensionAbilityInfoFlag::GET_EXTENSION_ABILITY_INFO_DEFAULT);
    bool ret = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(
        queryWant, flags, userId, extensionInfos));
    if (ret && !extensionInfos.empty()) {
        if (extensionInfos[0].type == AppExecFwk::ExtensionAbilityType::SERVICE) {
            targetType = AppExecFwk::ExtensionAbilityType::SERVICE;
            return ERR_OK;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "unsupported extension type:%{public}d for ability:%{public}s",
            static_cast<int>(extensionInfos[0].type), abilityName.c_str());
        return ERR_INVALID_VALUE;
    }

    // 2. 再查是否为 UIAbility
    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityFlags = static_cast<int32_t>(
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT);
    bool abilityRet = IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(
        queryWant, abilityFlags, userId, abilityInfo));
    if (abilityRet && abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
        return ERR_OK;
    }

    // PAGE 是 Stage 模型 UIAbility 的唯一类型，SERVICE/DATA/FORM 均为 FA 模型旧类型
    // skill 是 Stage 模型才有的配置，不会命中 FA 模型类型
    TAG_LOGE(AAFwkTag::ABILITYMGR,
        "ability:%{public}s is neither UIAbility nor ServiceExtension", abilityName.c_str());
    return ERR_INVALID_VALUE;
}
```

**调用方改动**（`GenerateSkillWant`）：

```cpp
auto ret = ResolveTargetType(skillInfo.bundleName, skillInfo.moduleName, abilityName, userId, targetType);
if (ret != ERR_OK) {
    TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve target type failed");
    return ret;
}
```

### 进一步优化：将 ResolveTargetType 从 GenerateSkillWant 中前移

`ResolveTargetType` 需要的输入（`bundleName`、`moduleName`、`abilityName`、`userId`）在 `QuerySkillInfo` 返回后就可用了，不需要等 `GenerateSkillWant`。

将类型判断前移到 `ExecuteInAppSkillWithTokenId` 中 `QuerySkillInfo` 之后，`GenerateSkillWant` 只负责构造 Want，不再承担类型判断职责。

**修改前**（`ability_manager_service.cpp`）：

```cpp
// 1. QuerySkillInfo
// 2. CheckSkillPermission
// 3. CreateExecuteRecord
// 4. GenerateSkillWant（内部调用 ResolveTargetType）
// 5. 按 targetType 分派
```

**修改后**：

```cpp
// 1. QuerySkillInfo
// 2. ResolveTargetType（前移到这里，紧跟 QuerySkillInfo 之后）
// 3. CheckSkillPermission
// 4. CreateExecuteRecord
// 5. GenerateSkillWant（不再调用 ResolveTargetType，仅构造 Want）
// 6. 按 targetType 分派
```

**`ExecuteInAppSkillWithTokenId` 改动**：

```cpp
// 1. Query skill configuration from bundle framework
AppExecFwk::SkillInfo skillInfo;
ret = SkillExecuteManager::GetInstance()->QuerySkillInfo(...);
if (ret != ERR_OK) { ... }

// 2. Resolve target type (前移)
AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
ret = SkillExecuteManager::GetInstance()->ResolveTargetType(
    skillInfo.bundleName, skillInfo.moduleName, skillInfo.abilityName, userId, targetType);
if (ret != ERR_OK) {
    TAG_LOGE(AAFwkTag::ABILITYMGR, "resolve target type failed");
    return ret;
}

// 3. Verify caller permissions
ret = SkillExecuteManager::GetInstance()->CheckSkillPermission(skillInfo, request.callerTokenId);
...

// 4. Create execute record
...

// 5. Generate Want（签名移除 targetType 参数）
Want want;
ret = SkillExecuteManager::GetInstance()->GenerateSkillWant(
    skillInfo, want, userId, requestCode,
    request.scriptPath, request.functionName, request.skillArgs);
...

// 6. Launch target based on type
...
```

**`GenerateSkillWant` 签名变更**：移除 `targetType` 输出参数，改为纯 Want 构造。

```cpp
// 修改前
int32_t GenerateSkillWant(const SkillInfo &skillInfo, Want &want,
    int32_t userId, const std::string &requestCode,
    ExtensionAbilityType &targetType,          // 移除
    const std::string &scriptPath, const std::string &functionName,
    const shared_ptr<WantParams> &skillArgs);

// 修改后
int32_t GenerateSkillWant(const SkillInfo &skillInfo, Want &want,
    int32_t userId, const std::string &requestCode,
    const std::string &scriptPath, const std::string &functionName,
    const shared_ptr<WantParams> &skillArgs);
```

**`GenerateSkillWant` 实现变更**：删除 `ResolveTargetType` 调用和 `abilityName` 空判断。

```cpp
int32_t SkillExecuteManager::GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
    int32_t userId, const std::string &requestCode,
    const std::string &scriptPath, const std::string &functionName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs)
{
    const std::string &abilityName = skillInfo.abilityName;
    want.SetElementName("", skillInfo.bundleName, abilityName, skillInfo.moduleName);
    AppExecFwk::SkillExecuteParam::WriteToWant(want, skillInfo.bundleName, skillInfo.moduleName,
        skillInfo.skillName, scriptPath, functionName, skillArgs, skillInfo.srcEntries, requestCode,
        skillInfo.hapPath);
    return ERR_OK;
}
```

**`ResolveTargetType` 改为 public 方法**：从 `GenerateSkillWant` 内部调用变为 `ExecuteInAppSkillWithTokenId` 直接调用，需要从 private 移到 public。

### 后续优化方向

当前 `ResolveTargetType` 仍需 AMS 额外做 1~2 次 IPC 查询 BMS。更优方案是在 BMS 侧 `SkillInfo` 中新增 `abilityType` 字段，安装时直接填充，查询时随 `SkillInfo` 返回，彻底消除 AMS 侧的类型查询。涉及 BMS 侧改动，待后续排期。

---

## 待修复：普通 StartAbility 路径未清理 Skill 参数，存在伪造攻击风险

### 问题分析

目标 App 侧（`ui_ability_impl.cpp:731`、`extension_ability_thread.cpp:563`）**仅通过 `IsSkillExecute(want)` 判断**是否进入 Skill 执行流程：

```cpp
// ui_ability_impl.cpp — AbilityTransaction 中
if (SkillExecuteParam::IsSkillExecute(want)) {
    HandleExecuteSkill(want);  // 加载模块 → napi_call_function
}

// extension_ability_thread.cpp — ScheduleCommandAbility 中
if (AppExecFwk::SkillExecuteParam::IsSkillExecute(want)) {
    ScheduleSkillExecuteInner(want);
}
```

`IsSkillExecute` 只检查 Want 中是否含有 skill 相关参数，**不验证来源**。

### 攻击路径

```
恶意 App → 普通 StartAbility(want 构造 skill 参数)
  → AMS 走通用 StartAbility 路径
     ├─ 不经过 CliToolMgr 的 EXEC_CLI_TOOL 权限校验
     ├─ 不经过 CheckSkillPermission
     └─ 不经过 CreateExecuteRecord
  → 目标 App onStart / onCommand
  → IsSkillExecute(want) == true
  → HandleExecuteSkill → 加载模块 → napi_call_function
  → 绕过所有 skill 权限校验
```

### 修复方案

在 AMS 的通用 StartAbility 入口处，检测并清理 Want 中的 Skill 参数，防止伪造的 Skill Want 泄漏到目标 App。

参考意图侧的实现：意图侧在通用路径上通过 `IsInsightIntentExecute` 检测并拦截。

### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/ability_manager_service.cpp` | 在 `StartAbility` 系列入口处，调用 `SkillExecuteParam::RemoveSkillParam` 清理 Want |

### 需要拦截的入口

通用 StartAbility 路径上，Want 经过 AMS 到达目标 App 的所有入口：

| 方法 | 位置 | 说明 |
|------|------|------|
| `StartAbility` | ability_manager_service.cpp | 通用启动入口 |
| `StartAbilityByCall` | ability_manager_service.cpp | Call 模式启动 |
| `StartExtensionAbility` | ability_manager_service.cpp | Extension 启动 |
| `StartAbilityByCallWithSkill` | ability_manager_service.cpp | Skill 专用入口，**不需要清理** |
| `StartExtensionAbilityWithSkill` | ability_manager_service.cpp | Skill 专用入口，**不需要清理** |

### 实现方式

在通用入口的最前面（权限校验之前或之后），加一行清理：

```cpp
// 例如在 StartAbility 入口中
int32_t AbilityManagerService::StartAbility(const Want &want, ...)
{
    // 清理伪造的 Skill 参数
    Want cleanWant(want);
    if (SkillExecuteParam::IsSkillExecute(cleanWant)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "stripping skill params from normal StartAbility");
        SkillExecuteParam::RemoveSkillParam(cleanWant);
    }
    // 后续使用 cleanWant 而非 want
    ...
}
```

### 注意事项

- `StartAbilityByCallWithSkill` 和 `StartExtensionAbilityWithSkill` 是 Skill 专用入口，**不能清理**，这些路径的 Want 是 AMS 自己构造的
- `RemoveSkillParam` 已有现成实现（`skill_execute_param.cpp:138`）
- 意图侧采用相同方案：在通用路径上检测并清理意图参数

---

## 待增强：DFX 关键字段与入口出口日志

### 目标

增强 Skill 执行流程的可观测性，便于问题定位和耗时分析。

### 一、SkillExecuteRecord 新增关键字段

当前 `SkillExecuteRecord` 缺少 Skill 身份信息和时间戳，导致死亡/超时日志无法关联到具体 Skill，也无法计算执行耗时。

#### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/include/skill/skill_execute_record.h` | 新增 5 个字段 |

#### 新增字段

| 字段 | 类型 | 用途 |
|------|------|------|
| `skillName` | `std::string` | Skill 名称，死亡/超时日志关联 |
| `moduleName` | `std::string` | 模块名称 |
| `abilityName` | `std::string` | Ability 名称 |
| `createTime` | `int64_t` | 记录创建时间（毫秒），计算耗时 |
| `completeTime` | `int64_t` | 记录完成时间（毫秒），计算耗时 |

#### 修改后

```cpp
struct SkillExecuteRecord {
    std::string requestCode;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    sptr<IRemoteObject::DeathRecipient> callbackDeathRecipient = nullptr;
    std::string targetBundleName;
    std::string callerBundleName;
    uint32_t callerTokenId = 0;
    uint64_t requestCodeSeq = 0;
    SkillExecuteState state = SkillExecuteState::UNKNOWN;
    sptr<ISkillExecuteCallback> callback = nullptr;
    // DFX 新增字段
    std::string skillName;
    std::string moduleName;
    std::string abilityName;
    int64_t createTime = 0;
    int64_t completeTime = 0;
};
```

#### 字段填充位置

在 `CreateExecuteRecord` 中从调用方传入的参数填充 `skillName`、`moduleName`、`abilityName`、`createTime`：

```cpp
record->skillName = skillName;
record->moduleName = moduleName;
record->abilityName = abilityName;
record->createTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
```

> `CreateExecuteRecord` 签名需扩展，新增 `skillName`、`moduleName`、`abilityName` 参数。

在 `ExecuteSkillDone` 中填充 `completeTime`：

```cpp
record->completeTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
```

### 二、入口/出口 INFO 日志补全

当前大部分关键方法只有 DEBUG 日志，生产环境不可见。需要补全 INFO 级别的入口/出口日志。

#### 修改文件

| 文件 | 修改内容 |
|------|----------|
| `services/abilitymgr/src/ability_manager_service.cpp` | `ExecuteInAppSkillWithTokenId` 入口/出口日志 |
| `services/abilitymgr/src/skill/skill_execute_manager.cpp` | 各方法入口/出口日志 |

#### 日志清单

##### 1. ExecuteInAppSkillWithTokenId（ability_manager_service.cpp）

```cpp
// 入口
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "ExecuteInAppSkill start, bundle:%{public}s skill:%{public}s callerTokenId:%{public}u",
    request.bundleName.c_str(), request.skillName.c_str(), request.callerTokenId);

// 出口（成功）
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "ExecuteInAppSkill end, request:%{public}s type:%{public}d ret:%{public}d",
    requestCode.c_str(), static_cast<int>(targetType), ret);

// 出口（失败，每个错误返回点）
TAG_LOGE(AAFwkTag::ABILITYMGR,
    "ExecuteInAppSkill failed, ret:%{public}d", ret);
```

##### 2. QuerySkillInfo（skill_execute_manager.cpp）

```cpp
// 成功出口
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "QuerySkillInfo done, ability:%{public}s perms:%{public}zu",
    skillInfo.abilityName.c_str(), skillInfo.permissions.size());
```

##### 3. ResolveTargetType（skill_execute_manager.cpp）

```cpp
// 成功出口
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "ResolveTargetType done, type:%{public}d", static_cast<int>(targetType));
```

##### 4. CheckSkillPermission（skill_execute_manager.cpp）

```cpp
// 成功出口
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "CheckSkillPermission passed, skill:%{public}s", skillInfo.skillName.c_str());
```

##### 5. CreateExecuteRecord（skill_execute_manager.cpp）

现有 DEBUG 日志升级为 INFO，补充 Skill 上下文：

```cpp
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "CreateExecuteRecord, req:%{public}s skill:%{public}s target:%{public}s",
    requestCode.c_str(), skillName.c_str(), targetBundleName.c_str());
```

##### 6. OnTimeout（skill_execute_manager.cpp）

补充 Skill 身份和已耗时：

```cpp
TAG_LOGW(AAFwkTag::ABILITYMGR,
    "skill execute timed out, req:%{public}s skill:%{public}s target:%{public}s elapsed:%{public}" PRId64 "ms",
    requestCode.c_str(), record->skillName.c_str(),
    record->targetBundleName.c_str(),
    record->createTime > 0 ?
        (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() - record->createTime) : 0);
```

##### 7. OnCallbackDied（skill_execute_manager.cpp）

```cpp
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "callback died, req:%{public}s skill:%{public}s target:%{public}s",
    requestCode.c_str(), record->skillName.c_str(), record->targetBundleName.c_str());
```

##### 8. OnCallerDied（skill_execute_manager.cpp）

```cpp
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "caller died, req:%{public}s skill:%{public}s target:%{public}s",
    requestCode.c_str(), record->skillName.c_str(), record->targetBundleName.c_str());
```

##### 9. ExecuteSkillDone（skill_execute_manager.cpp）

补充耗时信息：

```cpp
int64_t elapsed = record->createTime > 0 ?
    (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() - record->createTime) : 0;
TAG_LOGI(AAFwkTag::ABILITYMGR,
    "ExecuteSkillDone, req:%{public}s skill:%{public}s code:%{public}d elapsed:%{public}" PRId64 "ms",
    requestCode.c_str(), record->skillName.c_str(), resultCode, elapsed);
```

##### 10. SetupAndStartSkillSession（cli_tool_manager_service.cpp）

记录 sessionId 与 requestCode 的映射关系，便于跨进程追踪：

```cpp
TAG_LOGI(CliToolMgrTag,
    "SkillSession created, session:%{public}s → request:%{public}s skill:%{public}s",
    sessionId.c_str(), requestCode.c_str(), skillName.c_str());
```
