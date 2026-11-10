# Design：系统与三方统一的意图函数调用（agent_intent_invoke）

> - **状态**：设计定稿，作为重写实现的唯一依据
> - **关联 PR**：#20905
> - **关联 Issue**：#16626
> - **分支**：`agent_intent_invoke`（基于 master `edcd44bc02`）
> - **范围**：将「三方函数调用」（`invokePublicFunction`）与「系统函数调用」（`invokeFunction`）整合到同一条执行链。权限模型采用**身份选权限 + exported 只拦三方**，服务端（AMS）兜底校验，三方调用在弹窗前先做一次授权预检；同时把调用方 Ability 的 token 贯穿到 `StartAbilityInner` 完成启动归属。

---

## 1. 概述

### 1.1 背景

OpenHarmony 元能力框架中，`invokeFunction` 原本是**系统应用/SA 专属**的能力：调用方需要系统身份 + `ohos.permission.EXECUTE_INSIGHT_INTENT`（`system_basic` 级权限，三方应用无法持有）。本设计新增 **`invokePublicFunction`** 入口，让三方应用也能调用「对三方开放」的函数，同时不破坏系统路径的既有语义。

### 1.2 设计目标

1. **一条执行链**：系统与三方复用同一套 NAPI → executor → IPC → AMS 执行机制，避免重复实现。
2. **身份驱动分流（仅 UX + 本地查询选择）**：在 NAPI 侧本地识别调用方身份（系统应用 vs 三方），据此决定是否弹授权窗；executor 据此选择 climgr 查询接口。**身份不跨进程传输、不作服务端放行依据。**
3. **服务端独立授权（AMS 兜底）**：AMS 用 calling token 判身份、用目标函数 `exported` 判可见性，无论系统还是三方都在 AMS 侧校验对应权限，形成纵深防御。
4. **弹窗前预检（防弹窗轰炸）**：三方调用在挂载 consent 弹窗之前，先调用一次服务端授权校验，未通过则直接失败、不弹窗。
5. **调用方 token 贯穿（启动归属）**：把调用方 Ability 的 token（`context->GetToken()`）沿执行链传到 `StartAbilityInner`，使启动归属（caller record / tokenId / mission）正确；三方只能传自己的 token。
6. **自动化控制免弹窗（本次新增）**：三方调用在挂载 consent 弹窗前调用 `AccessTokenKit::CheckAutomationControlAccess`，若调用方 A 已对目标 B 设置过「可控制且授权」，则跳过弹窗直接执行；其余情况（含显式拒绝）仍照常弹窗，避免已授权场景的重复弹窗打扰。
7. **兼容性**：不修改 IPC 序列化格式（Stub/Proxy 的 parcel 布局），不修改已发布的公共 API 签名与错误码数值。

### 1.3 关键结论（设计评审共识）

- **执行链共用是合理的**（客户端 helper + AMS 执行机制），机制一致、避免重复。
- **「单 IPC 入口 + 客户端注入布尔标志切换身份门」存在信任反转，已废弃**：`isPublicInvoke` 是客户端可构造的布尔值，AMS 若盲信它跳过身份门，等于把安全结论建立在不可验证的客户端声明上。**该标志不再跨进程传输。**
- **权限模型（最终）**：以**调用方身份**选权限（系统 `EXECUTE_INSIGHT_INTENT`、三方 `ACCESS_PUBLIC_FUNCTION`），以**目标函数 `exported`** 仅拦三方（私有函数三方不可见）。两个维度正交，且都在服务端用可信数据（calling token + 目标 profile 声明）判定。
- **弹窗决策是客户端 UX，可本地按身份判断**；**但弹窗前必须先用服务端授权预检**，未授权/目标不可见的调用不弹窗，避免三方恶意应用无限调用导致弹窗轰炸。
- **两个 token 分角色**：回调 token（`InsightIntentHostClient`）用于结果回传；调用方 ability token（`context->GetToken()`）用于启动归属。二者不可混用，ability token 经 `wantParam` 携带、AMS 读后剥离并校验归属——**系统/SA 传了就用、不传置空；三方必传自己；校验仅在意图执行入口做**。

---

## 2. 术语表

| 术语 | 含义 |
|---|---|
| **insight intent** | OpenHarmony 系统级意图标准，此处指可被 `invokeFunction`/`invokePublicFunction` 调用的函数型意图。 |
| **function** | 注册在 climgr（CliToolManagerService，SA）上的可调用函数，含 `functionNamespace`（目标 bundle）+ `functionName`。 |
| **系统路径** | `invokeFunction`：调用方为系统应用/SA，校验 `EXECUTE_INSIGHT_INTENT`，不弹授权窗。 |
| **公共路径** | `invokePublicFunction`：调用方为三方应用，校验 `ACCESS_PUBLIC_FUNCTION` + 函数 `exported`，弹授权窗。 |
| **exported / visible** | 目标函数「对三方可见」的声明。climgr 层命名 `visible`，意图 profile 层命名 `exported`（`func.visible = info.exported`）。**由目标应用开发者在 profile 里声明，默认 `false`（私有），非调用方/运行时决定。** |
| **回调 token** | `InsightIntentHostClient`，`ExecuteIntentByFunctionCall` 的 `callerToken` 实参，用于结果回调 `OnExecuteDone`。 |
| **调用方 ability token（新增）** | `context->GetToken()` 得到的调用方 Ability 在 AMS 注册的 token（`sptr<IRemoteObject>`），用于 `StartAbilityInner` 的启动归属。**系统/SA 传了就用、不传置空；三方必须传自己的**。 |
| **身份预检** | 三方调用在弹窗前发起的一次服务端授权校验，用以阻止未授权调用触发弹窗（防轰炸）。 |
| **climgr** | `CliToolManagerService`，函数注册/查询/授权服务（SA）。 |
| **AMS** | `AbilityManagerService`，意图执行调度服务（SA 180）。 |

---

## 3. 需求

### 3.1 功能需求

- **FR1**：三方应用可通过 `invokePublicFunction` 调用「对三方开放（exported=true）」的函数。
- **FR2**：系统应用通过 `invokeFunction` 调用函数，保持既有系统语义（系统身份 + `EXECUTE_INSIGHT_INTENT`，不弹窗）。
- **FR3**：三方调用必须经过授权弹窗（privacy-center consent dialog），用户同意后才执行；拒绝则以 `ERR_PERMISSION_DENIED` 失败。
- **FR4**：`context`（`UIAbilityContext`）作为可选参数；系统调用方允许为空，三方调用方为空则直接报错。
- **FR5**：调用方身份在 NAPI 侧本地判断，据此自动分流弹窗/校验，不依赖调用方「自选入口」。
- **FR6（防轰炸）**：三方调用在弹窗前先做一次服务端授权预检，未通过（无权限 / 目标不可见）直接失败、不弹窗。
- **FR7（启动归属）**：三方调用方把自身 Ability 的 token 沿执行链传到 `StartAbilityInner`，使启动正确归属到调用方；系统调用方不传 token（保持 nullptr 语义）。
- **FR8（自动化控制免弹窗）**：三方调用在弹窗前调用 `CheckAutomationControlAccess(controllerTokenId, {targetTokenId})`，`NO_POPUP_NEEDED_GRANTED` 跳过弹窗直接执行；其余情况（`NO_POPUP_NEEDED_DENIED` / `POPUP_NEEDED` / `TARGET_NOT_EXIST` / 查询失败）一律照常弹窗。

### 3.2 非功能需求

- **NFR1（安全）**：授权校验必须在服务端完成。系统路径校验 `EXECUTE_INSIGHT_INTENT`，公共路径校验 `ACCESS_PUBLIC_FUNCTION` + `exported`，**均以服务端可信数据（calling token / 目标 profile）判定**，不信任客户端注入的身份/授权结论。
- **NFR2（兼容）**：不修改 IPC Stub/Proxy 序列化格式；不修改已发布 API 签名、错误码值、`hisysevent.yaml` 事件定义。
- **NFR3（防泄漏）**：三方调用「函数不存在」与「函数不可见（exported=false）」返回同一错误码 `ERR_FUNCTION_NOT_EXIST`，避免泄露函数存在性。
- **NFR4（收敛）**：不跨进程传输任何客户端声明的身份/公共路径标志，服务端身份全部从 IPC 上下文推导。
- **NFR5（token 防冒充）**：三方**必须**传递**自己**的调用方 ability token（不传或传非自己即拒绝）；系统应用/SA 传了则使用、不传则置空（是唯一允许 token 为空者）。

---

## 4. 总体架构

### 4.1 分层视图

```
┌─────────────────────────────────────────────────────────────────────┐
│  调用方进程（系统应用 或 三方应用）                                      │
│                                                                       │
│  NAPI 入口层（cli_tool_framework/frameworks/js/napi/function_manager） │
│    InvokeFunction / InvokePublicFunction                              │
│      └─ StartInvokeFunction：本地身份判断 + 三方先预检再弹窗            │
│          提取 callerAbilityToken = abilityContext->GetToken()          │
│                                                                       │
│  executor 层（invoke_function_executor.cpp）                           │
│    GetPublicFunctionInfo / GetFunctionInfo（climgr 授权，按身份选）    │
│    wantParam 写入 callerAbilityToken（不注入身份标志）                  │
│                                                                       │
│  IPC 客户端（intent_client.cpp）                                       │
│    ExecuteIntentByFunctionCall / CheckInvokeFunctionCaller            │
│      / CheckPublicFunctionCaller（三方预检）                           │
└──────────────────────────────────────────────────────────────────────┘
                           │ IPC（ability_manager）
┌──────────────────────────────────────────────────────────────────────┐
│  AMS（services/abilitymgr）                                            │
│    CheckInvokeFunctionCaller / CheckPublicFunctionCaller（预检）        │
│    ExecuteIntentByFunctionCall：                                        │
│      读+剥离 callerAbilityToken → ResolveCallerAbilityToken（校验归属）  │
│      └─ CheckAndUpdateParam → CheckCallerPermission                    │
│             （calling token 判身份 → 选权限；三方先查 exported 拦门）    │
│      └─ ExecuteIntentCommon → StartAbilityWithInsightIntent            │
│             （透传 callerAbilityToken → StartAbilityInner）             │
└──────────────────────────────────────────────────────────────────────┘
                           │
┌──────────────────────────────────────────────────────────────────────┐
│  目标应用（被调用函数所在 bundle）                                       │
│    InsightIntentContext::StartAbilityByInsightIntent（回调执行）        │
└──────────────────────────────────────────────────────────────────────┘
```

### 4.2 系统调用时序（invokeFunction，调用方为系统应用）

```
系统应用
  → JSFunctionManager::InvokeFunction
  → OnInvokeFunction（context = nullptr）
  → StartInvokeFunction
       isSystemApp = IsSystemAppByFullTokenID(GetSelfTokenID()) == true
       callerAbilityToken = nullptr（invokeFunction 系统入口无 context）
       CheckInvokeFunctionCaller()  // IPC→AMS：系统身份 + EXECUTE_INSIGHT_INTENT
       submit()                     // 不弹窗，直接执行
  → executor::DoExecute(isPublicInvoke=false)
       GetFunctionInfo(...)         // climgr：系统应用 + ACCESS_FUNCTION
  → IntentClient::ExecuteIntentByFunctionCall（不注入身份标志，也无 token）
  → AMS::ExecuteIntentByFunctionCall
       CheckCallerPermission：calling token 判系统 → 校验 EXECUTE_INSIGHT_INTENT
       ExecuteIntentCommon → StartAbilityWithInsightIntent(..., callerToken=nullptr)
```

### 4.3 三方调用时序（invokePublicFunction，调用方为三方）

```
三方应用
  → JSFunctionManager::InvokePublicFunction
  → OnInvokePublicFunction
       abilityContext = GetInvokeContext(...)   // 解析 options.context
  → StartInvokeFunction
       isSystemApp = false（非系统）
       abilityContext == nullptr ? → reject(ERR_INNER_PARAM_INVALID)   // context 空
       : CheckPublicFunctionCaller(ns, name)    // ① 预检：IPC→AMS 授权校验（防轰炸）
           失败 → reject(错误码)，不弹窗
           成功 → CheckAutomationControlAccess(controller, {target})   // ② 自动化控制免弹窗
                 GRANTED        → submit()（跳过弹窗）
                 DENIED / 其它   → 弹 consent 弹窗 → 用户同意 → submit()
  → executor::DoExecute(isPublicInvoke=true)
       GetPublicFunctionInfo(...)   // climgr：ACCESS_PUBLIC_FUNCTION + visible
       wantParam 写入 callerAbilityToken = abilityContext->GetToken()
  → IntentClient::ExecuteIntentByFunctionCall（不注入身份标志）
  → AMS::ExecuteIntentByFunctionCall
       读+剥离 callerAbilityToken → ResolveCallerAbilityToken（三方：须是自己的 token）
       CheckCallerPermission：calling token 判三方 → 查 exported 拦门 → 校验 ACCESS_PUBLIC_FUNCTION
       ExecuteIntentCommon → StartAbilityWithInsightIntent(..., callerToken=callerAbilityToken)
```

### 4.4 调用流程图（端到端泳道）

三方路径 `invokePublicFunction`（最复杂，含预检 / 弹窗 / token 贯穿）：

```
┌─────────────── 调用方进程（三方应用） ───────────────┐        ┌─────────────── 系统服务进程 ────────────────┐
│                                                      │        │                                            │
│  invokePublicFunction(namespace, name, args, options)│        │                                            │
│    │                                                 │        │                                            │
│    ▼                                                 │        │                                            │
│  OnInvokePublicFunction                              │        │                                            │
│    ├─ ParseInvokeFunctionParam(namespace, name, args)│        │                                            │
│    └─ GetInvokeContext → abilityContext              │        │                                            │
│    │                                                 │        │                                            │
│    ▼                                                 │        │                                            │
│  StartInvokeFunction                                 │        │                                            │
│    ├─ isSystemApp = IsSystemAppByFullTokenID(GetSelfTokenID()) = false      │                              │
│    ├─ callerAbilityToken = abilityContext->GetToken()                       │                              │
│    ├─ context 空? ── 是 → reject(ERR_INNER_PARAM_INVALID)                   │                              │
│    │            └── 否 ↓                                                    │                              │
│    │  ①预检: CheckPublicFunctionCaller(namespace, name) ══ IPC ════════════▶ AMS::CheckPublicFunctionCaller   │
│    │            ├─ 失败 → reject(不弹窗)                                     │   └─ exported==true?          │
│    │            └─ 通过 ↓                                                   │   └─ ACCESS_PUBLIC_FUNCTION?  │
│    │  ②免弹窗判断: CheckAutomationControlAccess(controller, {target})      │                              │
│    │            ├─ GRANTED → submit()（跳过弹窗）                          │                              │
│    │            └─ DENIED / POPUP_NEEDED / 其它 → ③弹窗 ↓                  │                              │
│    │  ③弹窗: consent 弹窗（privacycenter CollaborationAccessAbility）       │                              │
│    │            ├─ 拒绝 → reject(ERR_PERMISSION_DENIED)                    │                              │
│    │            └─ 同意 → submit() ↓                                       │                              │
│    ▼                                                                        │                              │
│  executor::DoExecute(isPublicInvoke=true, callerAbilityToken)               │                              │
│    ├─ GetPublicFunctionInfo(namespace, name)  ══ IPC ═════════════════════▶ climgr: ACCESS_PUBLIC_FUNCTION + visible
│    ├─ wantParam.SetParam(INSIGHT_INTENT_CALLER_TOKEN, callerAbilityToken)   │                              │
│    └─ ExecuteIntentByFunctionCall(execParam)                                │                              │
│    │                                                                        │                              │
│    ▼                                                                        │                              │
│  IntentClient::ExecuteIntentByFunctionCall ════ IPC ═════════════════════▶ Stub: ExecuteIntentByFunctionCallInner
│                                                      │        ┌──────────────────────────────────────┐     │
│                                                      │        │ AMS::ExecuteIntentByFunctionCall       │     │
│                                                      │        │  1. 剥 INSIGHT_INTENT_CALLER_TOKEN     │     │
│                                                      │        │  2. ResolveCallerAbilityToken  <─ 决策图B │
│                                                      │        │  3. CheckAndUpdateParam                │     │
│                                                      │        │       └─ CheckCallerPermission  <─ 决策图A │
│                                                      │        │  4. GetInsightIntentExecuteInfo        │     │
│                                                      │        │  5. ExecuteIntentCommon                │     │
│                                                      │        └──────────────────────────────────────┘     │
│                                                      │                    │                              │
│                                                      │                    ▼                              │
│                                                      │    StartAbilityWithInsightIntent(want, userId,    │
│                                                      │        reqCode, fullTokenId, callerAbilityToken)   │
│                                                      │                    │                              │
│                                                      │                    ▼                              │
│                                                      │    StartAbilityInner(param.callerToken = token)  │
│                                                      └────────────────────┼──────────────────────────────┘
│                                                                           │ 启动
│                                                                           ▼
│                                          ┌───────────── 目标应用（被调函数所在 bundle） ─────────────┐
│                                          │ InsightIntentContext::StartAbilityByInsightIntent 执行   │
│                                          │ ── 结果上行：ExecuteInsightIntentDone → AMS → hostClient │
│                                          │    → OnExecuteDone → tsfn → promise resolve/reject        │
│                                          └───────────────────────────────────────────────────────────┘
```

系统路径 `invokeFunction`：

```
系统应用进程                                      AMS / climgr（系统进程）
    │
 invokeFunction(namespace, name, args)
    ▼
 OnInvokeFunction  →  StartInvokeFunction(param, nullptr)
    │                     ├─ isSystemApp = true
    │                     ├─ callerAbilityToken = nullptr（系统入口无 context）
    │                     └─ CheckInvokeFunctionCaller() ══ IPC ══▶ 系统身份 + EXECUTE_INSIGHT_INTENT
    │                              └─ 成功 → submit()（不弹窗）
    ▼
 executor::DoExecute(isPublicInvoke=false, token=nullptr)
    ├─ GetFunctionInfo(namespace, name)  ══ IPC ══▶ climgr: ACCESS_FUNCTION
    └─ ExecuteIntentByFunctionCall()（不写 token） ══ IPC ══▶ AMS::ExecuteIntentByFunctionCall
                                                          ├─ 剥 token（无）
                                                          ├─ ResolveCallerAbilityToken: 系统 → outToken=inToken=nullptr
                                                          ├─ CheckCallerPermission: 系统 → EXECUTE_INSIGHT_INTENT
                                                          └─ ExecuteIntentCommon → StartAbilityWithInsightIntent(..., nullptr)
                                                                                └─ StartAbilityInner(callerToken=nullptr)
```

### 4.5 AMS 内关键决策点

**决策图 A — CheckCallerPermission（身份选权限 + exported 拦三方）**

```
CheckCallerPermission(specifiedFullTokenId, callerBundle, targetBundle, module, intentName, userId)
        │
        ▼
isSystemAppCall = JudgeCallerIsAllowedToUseSystemAPI()   // calling token
        │
   ┌────┴──────────┐
   │ true(系统/SA) │            │ false(三方) │
   └────┬──────────┘            └──────┬──────┘
        ▼                               ▼
 VerifyCallingPermission        exported = GetInsightIntentExported(targetBundle, module, intentName, userId)
   (EXECUTE_INSIGHT_INTENT)             │
        │                    ┌──────────┴──────────┐
   ┌────┴────┐           false                    true
   │ 有 → OK │           │                        │
   │ 无 → 拒 │           ▼                        ▼
   └─────────┘    ERR_FUNCTION_NOT_EXIST    VerifyCallingPermission(ACCESS_PUBLIC_FUNCTION)
                 （不可见，同码防泄露）             │
                                        ┌──────────┴──────────┐
                                        │ 有 → OK              │
                                        │ 无 → 拒              │
                                        └─────────────────────┘
```

**决策图 B — ResolveCallerAbilityToken（仅意图执行入口做）**

```
ResolveCallerAbilityToken(inToken, outToken)   // 仅在 ExecuteIntentByFunctionCall 调用
        │
        ▼
isSystemCaller = JudgeCallerIsAllowedToUseSystemAPI()
        │
   ┌────┴──────────┐
   │ true(系统/SA) │            │ false(三方) │
   └────┬──────────┘            └──────┬──────┘
        │                               │
        │                    ┌──────────┼──────────┐
        │               null │          │ 非null   │
        │                    ▼          ▼          │
        │              ERR_PERMISSION_  GetAbilityRecordByToken(inToken)
        │               DENIED(不传)         │
        │                         ┌──────────┴───────────┐
        │                   record==null / uid!=callingUid   record 有且 uid==callingUid
        │                         │                            │
        │                         ▼                            ▼
        │                   ERR_PERMISSION_DENIED         outToken = inToken
        ▼                          （传非自己）                （自己的 token）
outToken = inToken
（传了就用，不传保持 nullptr）
```

---

## 5. 关键设计决策（ADR）

### ADR-1：系统与三方共用执行链

**决策**：NAPI 入口、executor、IPC、AMS 执行机制共用一套；仅本地（进程内）通过 `isPublicInvoke` 选择 climgr 查询接口与弹窗分支。**注意：`isPublicInvoke` 仅是进程内局部变量，不等于跨进程传输。** 该名字来自 `isPublicInvoke = !isSystemApp`（本地推导），可改用更贴切的 `isThirdParty`，但重写时不改公共 API，变量命名内部自定。

**理由**：执行机制（构建 Want → 生成前台意图 → StartAbility）完全一致，拆分是纯重复。

### ADR-2：身份不跨进程传输，服务端从 calling token 自判

**决策**：**废弃**「以 `WantParams` 传输 `INSIGHT_INTENT_IS_PUBLIC_INVOKE`、AMS 读后剥离」的方案。AMS 不再读取任何客户端注入的身份/路径标志，服务端身份统一用 `IPCSkeleton::GetCallingFullTokenID()` / `JudgeCallerIsAllowedToUseSystemAPI()` 自判。

**理由**：调用方是否是系统应用，是 IPC 上下文内的权威事实（内核/IPC 层填充，进程无法伪造），无需调用方向服务端陈述一件服务端本就能自己确认的事。删除该传输字段消除信任反转（NFR4）。

### ADR-3：NAPI 侧本地身份判断驱动弹窗

**决策**：`StartInvokeFunction` 内用 `Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())` 本地判断调用方身份。

**理由**：NAPI 运行在调用方进程，`GetSelfTokenID()` 即调用方自身 token；full token 由访问令牌服务维护、进程无法伪造，该判断可信。**但仅用于弹窗/UX 决策与本地查询选择，绝不作服务端放行依据。**

### ADR-4：AMS 服务侧校验 `ACCESS_PUBLIC_FUNCTION`（新增权限）

**决策**：公共路径在 AMS `CheckCallerPermission` 侧新增校验 `ohos.permission.ACCESS_PUBLIC_FUNCTION`，不把授权只押在 climgr 一处。

**理由**：仅 climgr 校验时，调用方绕过 NAPI/executor 直接构造 `EXECUTE_INTENT_BY_FUNCTION_CALL` IPC，即可跳过授权直达执行。AMS 侧兜底校验形成纵深防御。需在 `PermissionConstants` 新增 `PERMISSION_ACCESS_PUBLIC_FUNCTION`（当前仅 climgr 局部定义 `cli_tool_manager_service.cpp:89`）。

### ADR-5：身份选权限，exported 只拦三方（最终权限模型）

**决策**：权限选择依据是**调用方身份**，`exported` 只作为**三方的可见性门**：

| 调用方身份 | 目标函数 exported | AMS 校验 |
|---|---|---|
| 系统应用/SA | 任意 | `EXECUTE_INSIGHT_INTENT` |
| 三方 | `true` | `ACCESS_PUBLIC_FUNCTION` |
| 三方 | `false` | 拒绝（`ERR_FUNCTION_NOT_EXIST`，防泄露） |

**理由**：系统应用可能调用 exported 函数，此时仍应走系统惯用权限 `EXECUTE_INSIGHT_INTENT`，而非 `ACCESS_PUBLIC_FUNCTION`。二者不能只用 `exported` 一个维度区分；**身份选权限、exported 只拦三方**，两个概念各司其职，也保持了系统路径的完全向后兼容。

### ADR-6：`CheckCallerPermission` 内部新增一次 exported 查询

**决策**：不再靠「重排 `GetInsightIntentExecuteInfo` 的调用顺序」把 exported 传入 `CheckCallerPermission`，而是让 `CheckCallerPermission` **自己新增一次查询**目标函数的 `exported`（只对三方分支查询；系统分支无需查）。

**理由**：`CheckCallerPermission` 当前经 `CheckAndUpdateParam` 调用，位于 `GetInsightIntentExecuteInfo` 之前，拿不到 exported。内部自查询使该校验自包含，不依赖调用方/上层把 exported 传进来，也便于复用为「弹窗前预检」。

### ADR-7：三方弹窗前先做身份预检（防弹窗轰炸）

**决策**：三方调用在挂载 consent 弹窗**之前**，先经 IPC 调用一次 AMS 授权校验（复用 `CheckCallerPermission` 的「三方 → exported + ACCESS_PUBLIC_FUNCTION」逻辑，命名为 `CheckPublicFunctionCaller(bundleName, intentName)`）。未通过直接失败、不弹窗。

**理由**：阻止无 `ACCESS_PUBLIC_FUNCTION` 权限的三方应用、或对私有函数（exported=false）的调用反复触发 consent 弹窗，构成弹窗轰炸 DoS。预检在服务端权威校验，客户端无法绕过。

### ADR-8：context 可选、三方必传

**决策**：`options.context` 为可选；系统调用方允许为空（不弹窗），三方调用方为空直接 `reject(ERR_INNER_PARAM_INVALID)`。

**理由**：`context` 是 consent 弹窗的宿主（提供 bundleName + UIContent），三方必须弹窗故必传；系统不弹窗故可空。注意：`context` 仅在 NAPI 层消费；但从中取出的 ability token 需随调用下行（见 ADR-9）。

### ADR-9：调用方 ability token 贯穿到 StartAbilityInner（启动归属）

**决策**：NAPI 提取 `context->GetToken()`（调用方 Ability 的 token），经 executor 写入 `wantParam`（键 `INSIGHT_INTENT_CALLER_TOKEN`，`WantParams` 的 `sptr<IRemoteObject>` 值），AMS 读后**立即剥离**并校验归属，再透传到 `ExecuteIntentCommon` → `StartAbilityWithInsightIntent` / `StartAbilityByCallWithInsightIntent` → `StartAbilityInner`，作为启动的 `callerToken`。

**携带方式**：走 `wantParam`，**不改 IPC Stub/Proxy 序列化格式**（NFR2）。`Want::SetParam(key, sptr<IRemoteObject>)` 是现成能力。

**理由**：当前 `ExecuteIntentByFunctionCall` 的 `callerToken` 是回调 token（`InsightIntentHostClient`），前台 `StartAbilityWithInsightIntent` 又漏传（默认 nullptr）、后台误传 hostClient，导致 `StartAbilityInner` 拿不到真实调用方 Ability record，启动归属失真。需要一条独立通道把 ability token 传到底。

### ADR-10：调用方 ability token 的归属校验（三方只能传自己的）

**决策**：AMS 在 `ExecuteIntentByFunctionCall`（仅意图执行入口，不在预检/其它路径）读到 `callerAbilityToken` 后执行 `ResolveCallerAbilityToken`：

| 输入 | 处理 |
|---|---|
| 系统/SA，token == nullptr | 通过，`outToken = nullptr`（置空） |
| 系统/SA，token 非空 | 通过，`outToken = inToken`（**直接用**，不校验 uid） |
| 三方，token == nullptr | **直接拒绝** `ERR_PERMISSION_DENIED`（三方必须传 token） |
| 三方，token 非自己 | **直接拒绝** `ERR_PERMISSION_DENIED` |
| 三方，token 是自己 | 通过，`outToken = inToken` |

**两条硬限制**：

1. **只有系统应用/SA 才能 `callerToken == nullptr`**：三方必须传 token，不传即拒绝。
2. **只有意图执行才做此校验**：`ResolveCallerAbilityToken` 只在 `ExecuteIntentByFunctionCall` 内调用，不在预检 `CheckPublicFunctionCaller`、通用 `ExecuteIntent`、普通 `StartAbility` 等其它路径执行。

「是否属于自己」= `Token::GetAbilityRecordByToken(inToken)` 非空且 `record->GetUid() == IPCSkeleton::GetCallingUid()`。

**理由**：系统应用/SA 也可能通过自身 UIAbility 的 context 调用，其传的 token 可信，直接用于启动归属（不校验）；而客户端往 wantParam 塞 token 整体可控，恶意三方可能塞别人的 token 冒充其他 Ability，或干脆不传绕过归属，故三方必须证明 token 是自己进程的，否则拒绝，杜绝「借他人 token 改启动归属 / 逃逸归属」。

### ADR-11：自动化控制免弹窗（三方已授权控制目标时跳过 consent 弹窗）

**决策**：三方调用通过 `CheckPublicFunctionCaller` 预检后、挂载 consent 弹窗前，调用 `AccessTokenKit::CheckAutomationControlAccess(controllerTokenId, {targetTokenId}, decisions)` 判断调用方（controller）是否已对目标 bundle（target）设置过自动化控制状态。依据返回的决策分派：

| `AutomationControlAccessDecision` | 行为 |
|---|---|
| `NO_POPUP_NEEDED_GRANTED`(3) | 跳过弹窗，直接 `submit()`（等价于用户已同意） |
| `NO_POPUP_NEEDED_DENIED`(4) | 照常弹 consent 弹窗（仅 `NO_POPUP_NEEDED_GRANTED` 免弹窗直接执行，显式拒绝仍交回用户确认） |
| `POPUP_NEEDED`(2) | 照常弹 consent 弹窗 |
| `TARGET_NOT_EXIST`(1) / 查询失败 / 目标 token 解析失败 | 回退：照常弹 consent 弹窗（保守，不静默放行） |

**tokenID 来源**：controller = `static_cast<AccessTokenID>(IPCSkeleton::GetSelfTokenID())`（调用方自身，本地可信）；target = `GetHapTokenID(controllerHapInfo.userID, functionNamespace, 0)`。controller 的 `userID` 由 `GetHapTokenInfo(controllerTokenId)` 取得（同用户目标，`instIndex=0`；解析失败或返回 `INVALID_TOKENID` 时回退弹窗）。

**理由**：`setAutomationControlStatus` 已允许 A 声明「可控制 B」，A 再次调用 B 的 `invokePublicFunction` 时不应再被 consent 弹窗反复打扰。此判断是**纯 UX 决策**（跳过本地弹窗），服务端权威授权（`CheckCallerPermission` 的 exported + `ACCESS_PUBLIC_FUNCTION`）在意图执行阶段不变，因此跳过弹窗**不会降低安全门槛**。回退场景选择「照常弹窗」而非「放行/拒绝」，避免在目标 token 无法解析或服务异常时静默改变既有行为。`NO_POPUP_NEEDED_DENIED` 同样回退弹窗：仅 `NO_POPUP_NEEDED_GRANTED`（用户此前已明确授权）可跳过重复弹窗，预拒绝不越过用户本人做最终拒绝决定。

---

## 6. 详细设计

> 代码位置为参考路径与行号（基于当前分支），重写时以符号/语义为准。

### 6.1 NAPI 入口层

**文件**：`cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp`、`include/js_function_manager.h`

#### 6.1.1 模块导出（两个 API）

- `InvokeFunction`（系统入口）→ `OnInvokeFunction`
- `InvokePublicFunction`（三方入口）→ `OnInvokePublicFunction`

#### 6.1.2 OnInvokeFunction

```cpp
napi_value JSFunctionManager::OnInvokeFunction(napi_env env, size_t argc, napi_value *argv)
{
    InvokeFunctionParam param;
    if (!ParseInvokeFunctionParam(env, argc, argv, param)) {
        return CreateJsUndefined(env);   // 已抛 JS 异常
    }
    // 系统入口不传 context（系统调用方永不挂 consent 弹窗，也无 ability token）
    return StartInvokeFunction(env, param, nullptr);
}
```

#### 6.1.3 OnInvokePublicFunction

```cpp
napi_value JSFunctionManager::OnInvokePublicFunction(napi_env env, size_t argc, napi_value *argv)
{
    InvokeFunctionParam param;
    if (!ParseInvokeFunctionParam(env, argc, argv, param)) {
        return CreateJsUndefined(env);
    }
    auto abilityContext = GetInvokeContext(env, argc, argv);   // 解析 options.context
    return StartInvokeFunction(env, param, abilityContext);
}
```

#### 6.1.4 GetInvokeContext（解析 options.context，第 4 个参数）

```cpp
std::shared_ptr<AbilityRuntime::AbilityContext> GetInvokeContext(napi_env env, size_t argc, napi_value *argv)
{
    if (argc < INDEX_FOUR || argv[INDEX_THREE] == nullptr) return nullptr;
    if (typeof(argv[INDEX_THREE]) != napi_object) return nullptr;
    napi_value contextValue;
    napi_get_named_property(env, argv[INDEX_THREE], "context", &contextValue);
    if (contextValue == undefined || contextValue == null) return nullptr;
    return GetAbilityContext(env, contextValue);   // 必须是 stage UIAbilityContext
}
```

**约束**：`context` 缺席 / null / 非对象 / 非 `UIAbilityContext` 一律返回 `nullptr`。

### 6.2 身份判断、授权预检与弹窗

**函数**：`StartInvokeFunction(napi_env, const InvokeFunctionParam&, const shared_ptr<AbilityContext>&)`

#### 6.2.1 隐私弹窗交互策略（总览）

隐私弹窗（privacy-center consent dialog，`CollaborationAccessAbility`）**只面向三方调用**；系统调用（`invokeFunction`）永不弹窗。三方调用与隐私弹窗的交互遵循「先预检 → 再免弹窗判断 → 最后弹窗」的三层顺序，任何一层命中即终止、不再进入下一层：

| 条件（按顺序判定） | 是否弹窗 | 结果 |
|---|---|---|
| 系统应用 / SA（`invokeFunction`） | 否 | `CheckInvokeFunctionCaller` 校验 `EXECUTE_INSIGHT_INTENT` → 直接执行 |
| 三方 + `context == nullptr` | 否 | 直接 `reject(ERR_INNER_PARAM_INVALID)` |
| 三方 + 预检失败（`CheckPublicFunctionCaller`：无 `ACCESS_PUBLIC_FUNCTION` / 目标 exported=false） | 否 | 直接 `reject(预检错误码)`（防弹窗轰炸） |
| 三方 + 自动化控制 `NO_POPUP_NEEDED_GRANTED` | 否 | 跳过弹窗直接执行（用户此前已明确授权控制目标） |
| 三方 + 自动化控制其它（`NO_POPUP_NEEDED_DENIED` / `POPUP_NEEDED` / `TARGET_NOT_EXIST` / 查询失败） | **是** | 照常弹 consent 弹窗 → 用户决定 |
| 弹窗：用户「同意」 | — | `submit` 执行 |
| 弹窗：用户「拒绝」/ `onError` / `onRelease` / 无图形支持 | — | `reject(ERR_PERMISSION_DENIED)` |

**策略要点**：

- **弹窗是纯客户端 UX**：能否弹窗、是否跳过只影响本地交互，**不降低服务端安全门槛**——服务端权威授权（calling token 判身份 + exported 挡门 + 身份对应权限 `EXECUTE_INSIGHT_INTENT`/`ACCESS_PUBLIC_FUNCTION`）在意图执行阶段始终不变。
- **防弹窗轰炸**：预检（ADR-7）把「无权限」与「目标私有」两类调用挡在弹窗之外，未授权应用无法反复触发弹窗。
- **显式授权才免弹窗**：自动化控制（ADR-11）仅 `NO_POPUP_NEEDED_GRANTED`（用户此前已明确授权）跳过弹窗；`NO_POPUP_NEEDED_DENIED` 的「预拒绝」**不越过用户本人做最终拒绝决定**，仍回退弹窗交由用户确认。
- **恰好一次结算**：`onResult` / `onError` / `onRelease` 任意回调触发，用 `std::atomic<bool> fired` 保证结果只结算一次（同意执行 / 拒绝失败）。

#### 6.2.2 完整决策逻辑（必须严格遵守）

```cpp
// 1. 本地身份判断（可信，仅驱动 UX）
bool isSystemApp = Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(
    IPCSkeleton::GetSelfTokenID());
bool isPublicInvoke = !isSystemApp;

// 2. 提取调用方 ability token（context 非空时取 GetToken()；系统入口无 context 则为 nullptr）
sptr<IRemoteObject> callerAbilityToken = (abilityContext != nullptr)
    ? abilityContext->GetToken() : nullptr;

// 3. 系统应用：不弹窗，服务端校验系统身份 + EXECUTE_INSIGHT_INTENT 后直接执行
if (isSystemApp) {
    int32_t validatePermission = AAFwk::IntentClient::GetInstance().CheckInvokeFunctionCaller();
    if (validatePermission != ERR_OK) {
        reject(CreateCliJsErrorByNativeErr(env, validatePermission));
        return promise;
    }
    submit();                       // 不弹窗，callerAbilityToken 为 nullptr
    return promise;
}

// 4. 三方 + context 空：直接报错
if (abilityContext == nullptr) {
    reject(CreateCliJsErrorByNativeErr(env, ERR_INNER_PARAM_INVALID));
    return promise;
}

// 5. 三方：先预检（防弹窗轰炸），通过才弹窗
int32_t precheck = AAFwk::IntentClient::GetInstance().CheckPublicFunctionCaller(
    param.functionNamespace, param.functionName);
if (precheck != ERR_OK) {
    reject(CreateCliJsErrorByNativeErr(env, precheck));
    return promise;
}

// 6. 三方 + 预检通过：先查自动化控制状态（controller=调用方、target=目标 bundle）
AccessTokenID controllerTokenId = static_cast<AccessTokenID>(IPCSkeleton::GetSelfTokenID());
HapTokenInfo controllerHapInfo;
if (AccessTokenKit::GetHapTokenInfo(controllerTokenId, controllerHapInfo) == RET_SUCCESS) {
    AccessTokenID targetTokenId = AccessTokenKit::GetHapTokenID(
        controllerHapInfo.userID, param.functionNamespace, 0);
    if (targetTokenId != INVALID_TOKENID) {
        std::vector<AccessTokenID> targetList{targetTokenId};
        std::vector<AutomationControlAccessDecision> decisions;
        if (AccessTokenKit::CheckAutomationControlAccess(controllerTokenId, targetList, decisions)
            == RET_SUCCESS && decisions.size() == 1) {
            if (decisions[0] == NO_POPUP_NEEDED_GRANTED) { submit(); return promise; }
            // NO_POPUP_NEEDED_DENIED / POPUP_NEEDED / TARGET_NOT_EXIST → 回退弹窗
        }
    }
}

// 7. 三方 + 预检通过 + 未命中自动化控制免弹窗：弹 consent 弹窗，授权通过才 submit
mountConsentDialog(abilityContext, param, /*granted*/ submit, /*denied*/
    reject(ERR_PERMISSION_DENIED));
```

**分支对应关系**：

| 条件 | 行为 |
|---|---|
| 系统应用 | 不弹窗 → `CheckInvokeFunctionCaller` → `submit`（token=nullptr） |
| 三方 + `context == nullptr` | 直接 `reject(ERR_INNER_PARAM_INVALID)` |
| 三方 + 预检失败 | 直接 `reject(预检错误码)`，**不弹窗** |
| 三方 + 自动化控制 `NO_POPUP_NEEDED_GRANTED` | 跳过弹窗 → 直接 `submit` |
| 三方 + 自动化控制其它（含 `NO_POPUP_NEEDED_DENIED`）/ 查询失败 | 弹授权窗 → 同意 `submit` / 拒绝 `reject(ERR_PERMISSION_DENIED)` |

> **预检的关键作用**：把「无 `ACCESS_PUBLIC_FUNCTION` 权限」与「目标 exported=false」两类调用挡在弹窗之外。声明了该权限的三方应用、调用 exported 函数才可能触发弹窗。
>
> **自动化控制免弹窗的作用**：在预检已通过的前提下，若调用方 A 已设置过「可控制且授权」目标 B，则仅 `NO_POPUP_NEEDED_GRANTED` 跳过 consent 弹窗直接执行；`NO_POPUP_NEEDED_DENIED` 及其它情况仍照常弹窗（ADR-11），避免已授权场景的重复打扰。该判断仅为本地弹窗 UX，服务端权威授权不变。

#### 6.2.3 submit（executor 启动桥）

`submit` 创建 `napi_threadsafe_function`（tsfn），把 executor 的纯 C++ 结果经 tsfn 桥回 JS 线程，并调用：

```cpp
InvokeFunctionExecutor::Create()->Execute(execParam, bridge, isPublicInvoke, callerAbilityToken);
```

`isPublicInvoke` 与 `callerAbilityToken` 均由身份/context 推导、进程内闭包捕获传递，`isPublicInvoke` **不写入任何 wantParam / IPC**；`callerAbilityToken` 由 executor 写入 wantParam（ADR-9）。

#### 6.2.4 consent 弹窗（三方路径）

通过 `CreateModalUIExtensionInternal`（与公共 `createModalUIExtension` 同一 Ace UIContent 挂载路径）挂载 privacy-center 弹窗：

```cpp
AAFwk::Want want;
want.SetElementName("com.huawei.hmos.security.privacycenter", "CollaborationAccessAbility");
want.SetParam("ability.want.params.uiExtensionType", std::string("sys/commonUI"));
want.SetParam(CONSENT_PARAM_CALLER_BUNDLE, abilityContext->GetBundleName());
want.SetParam(CONSENT_PARAM_TARGET_BUNDLE, param.functionNamespace);
want.SetParam(CONSENT_PARAM_TARGET_BUNDLE_LIST, {param.functionNamespace});
want.SetParam(CONSENT_PARAM_TARGET_INTENT, param.functionName);
// amsParam: { sourceBundleInfo.bundleName = caller, targetInfo.bundleName = target }
want.SetParam(CONSENT_PARAM_AMS, amsParam.dump());

Ace::ModalUIExtensionConfig config;
config.isProhibitBack = true;
CreateModalUIExtensionInternal(abilityContext, want, callbacks, config);
```

**回调语义**：`onResult` 读取 `CONSENT_PARAM_RESULT`（bool）→ 授权 `submit()`，拒绝/`onError`/`onRelease` → `reject(ERR_PERMISSION_DENIED)`；用 `std::atomic<bool> fired` 保证只 settle 一次。

**SUPPORT_GRAPHICS 未定义时**：直接 `settle(false)`（图形不支持视为拒绝）。

### 6.3 执行器（executor）层

**文件**：`cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp`、`include/invoke_function_executor.h`

**签名**：`void Execute(const InvokeFunctionParam&, InvokeResultCallback, bool isPublicInvoke = false, const sptr<IRemoteObject> &callerAbilityToken = nullptr)`

#### 6.3.1 DoExecute 核心逻辑

```cpp
void InvokeFunctionExecutor::DoExecute(const InvokeFunctionParam &param,
    bool isPublicInvoke, const sptr<IRemoteObject> &callerAbilityToken)
{
    FunctionInfo functionInfo;
    // 进程内按身份选择查询接口（climgr 服务端授权）
    ErrCode queryErr = isPublicInvoke
        ? CliToolMGRClient::GetInstance().GetPublicFunctionInfo(ns, name, functionInfo)
        : CliToolMGRClient::GetInstance().GetFunctionInfo(ns, name, functionInfo);
    if (queryErr != ERR_OK) { ReportError(queryErr); return; }

    if (functionInfo.functionType != FunctionType::INTENT_FUNCTION) { ... reject ... }

    // hook + 追踪 id 处理（toolCallId / dmSessionId / taskId）...

    AAFwk::ExecuteIntentParam execParam;
    execParam.bundleName = hookParam.functionNamespace;
    execParam.intentName = hookParam.functionName;
    execParam.wantParam   = hookParams;

    // 写入调用方 ability token（三方路径；系统为 nullptr 时不写）
    if (callerAbilityToken != nullptr) {
        execParam.wantParam.SetParam(AppExecFwk::INSIGHT_INTENT_CALLER_TOKEN, callerAbilityToken);
    }
    // 注意：不再注入 INSIGHT_INTENT_IS_PUBLIC_INVOKE

    execParam.callback = client;
    auto err = AAFwk::IntentClient::GetInstance().ExecuteIntentByFunctionCall(execParam);
    ...
}
```

#### 6.3.2 两条查询路径的授权差异（climgr 服务端）

| 查询 | 权限校验 | 可见性校验 | 归属 |
|---|---|---|---|
| `GetFunctionInfo`（系统） | 系统应用/SA + `ohos.permission.ACCESS_FUNCTION` | 无（系统可见全部） | 系统路径 |
| `GetPublicFunctionInfo`（公共） | `ohos.permission.ACCESS_PUBLIC_FUNCTION` | `function.visible == true`，否则返回 `ERR_FUNCTION_NOT_EXIST`（不泄露存在性） | 公共路径 |

### 6.4 IPC 传输层

**文件**：`interfaces/inner_api/insight_intent/intent_client/src/intent_client.cpp`

#### 6.4.1 ExecuteIntentByFunctionCall

```cpp
int32_t IntentClient::ExecuteIntentByFunctionCall(const ExecuteIntentParam &param)
{
    if (param.callback == nullptr) return ERR_INVALID_VALUE;
    auto hostClient = AbilityRuntime::InsightIntentHostClient::GetInstance();
    uint64_t key = hostClient->AddInsightIntentExecute(param.callback);
    int32_t err = SendExecuteRequest(
        EXECUTE_INTENT_BY_FUNCTION_CALL, key, hostClient,
        param.bundleName, param.intentName, param.wantParam);
    if (err != ERR_OK) hostClient->RemoveInsightIntentExecute(key);
    return err;
}
```

- `callerToken` = `hostClient`（`InsightIntentHostClient`，结果回调的 `IRemoteObject`）—— 回调 token，保持不变。
- IPC code：`AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL`。
- **不携带任何身份/路径标志**；调用方 ability token 随 `wantParam` 的 `INSIGHT_INTENT_CALLER_TOKEN` 键传输（`Want::SetParam(key, sptr<IRemoteObject>)`）。

#### 6.4.2 CheckInvokeFunctionCaller（系统身份预检）

发 `CHECK_INVOKE_FUNCTION_CALLER` 到 AMS，由 AMS 用 `GetCallingFullTokenID()` 判断系统身份 + `EXECUTE_INSIGHT_INTENT`。NAPI 侧用它做系统路径「提前失败」。

#### 6.4.3 CheckPublicFunctionCaller（三方身份预检，新增）

```cpp
int32_t IntentClient::CheckPublicFunctionCaller(const std::string &bundleName,
    const std::string &intentName)
{
    // 发 CHECK_PUBLIC_FUNCTION_CALLER 到 AMS，携带目标函数 identity
    // AMS：calling token 判三方 → 查 exported（拦私有）→ 校验 ACCESS_PUBLIC_FUNCTION
}
```

**用途**：三方在弹窗前调用（ADR-7）。服务端权威校验，未授权/目标不可见直接失败、不弹窗。

### 6.5 AMS 执行层

**文件**：`services/abilitymgr/src/ability_manager_service.cpp`、`src/insight_intent/insight_intent_execute_manager.cpp`

#### 6.5.1 ExecuteIntentByFunctionCall（AMS 入口）

```cpp
int32_t AbilityManagerService::ExecuteIntentByFunctionCall(uint64_t key,
    const sptr<IRemoteObject> &callerToken, const std::string &bundleName,
    const std::string &intentName, const WantParams &wantParam)
{
    // 1. 读 + 剥离调用方 ability token（防泄漏到目标 Want）
    WantParams cleanedWantParam = wantParam;
    sptr<IRemoteObject> rawCallerToken =
        cleanedWantParam.GetRemoteObject(INSIGHT_INTENT_CALLER_TOKEN);
    cleanedWantParam.Remove(INSIGHT_INTENT_CALLER_TOKEN);

    // 2. 归属校验（ADR-10，仅在意图执行入口做）：
    //    系统/SA 传了就用、不传置空；三方必须传自己的 token，否则拒绝
    sptr<IRemoteObject> callerAbilityToken;
    int32_t resolveRet = ResolveCallerAbilityToken(rawCallerToken, callerAbilityToken);
    if (resolveRet != ERR_OK) {
        return resolveRet;   // 三方 null / 非自己 token → ERR_PERMISSION_DENIED
    }

    // 3. 参数校验 + PrepareFunctionCallParam（解析出 InsightIntentExecuteParam）
    //    + HandleTaskIdAttribution（剥离任务归因 id）+ CheckCrossUserPermission
    //    不再读/剥离任何身份标志

    // 4. CheckAndUpdateParam（内部经 CheckCallerPermission 自判身份 + 自查 exported）
    ret = InsightIntentExecuteManager::GetInstance()->CheckAndUpdateParam(
        key, callerToken, param, {callerBundleName, ...});

    // 5. 提取函数执行信息（供 ExecuteIntentCommon 生成 Want 用）
    AbilityRuntime::ExtractInsightIntentInfo extractInfo;
    AbilityRuntime::InsightIntentInfo configInfo;
    GetInsightIntentExecuteInfo(*param, extractInfo, configInfo);

    // 6. ExecuteIntentCommonOptions 不再携带 isPublicInvoke，新增 callerAbilityToken
    ExecuteIntentCommonOptions options(ignoreAbilityName, parseResult.matchedInfo, key);
    options.extractInfo = std::move(extractInfo);
    options.configInfo = std::move(configInfo);
    options.callerAbilityToken = callerAbilityToken;

    return ExecuteIntentCommon(callerToken, param, callerBundleName, options);
}
```

#### 6.5.2 ResolveCallerAbilityToken（新增，ADR-10 落地）

```cpp
// 仅在 ExecuteIntentByFunctionCall（意图执行入口）调用；预检 / 通用 ExecuteIntent 不调用。
int32_t AbilityManagerService::ResolveCallerAbilityToken(
    const sptr<IRemoteObject> &inToken, sptr<IRemoteObject> &outToken)
{
    outToken = nullptr;
    bool isSystemCaller = PermissionVerification::GetInstance()
        ->JudgeCallerIsAllowedToUseSystemAPI();   // 系统应用/SA

    if (!isSystemCaller) {
        // 三方：必须传 token，且必须属于自己进程
        if (inToken == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "third-party caller token is null");
            return ERR_PERMISSION_DENIED;        // 三方不传 token → 直接拒绝
        }
        auto record = Token::GetAbilityRecordByToken(inToken);
        if (record == nullptr || record->GetUid() != IPCSkeleton::GetCallingUid()) {
            TAG_LOGE(AAFwkTag::INTENT, "third-party caller token mismatch");
            return ERR_PERMISSION_DENIED;        // 传非自己 token → 直接拒绝
        }
    }
    // 系统/SA：传了就用（inToken 非空时 outToken = inToken），不传则置空（outToken 保持 nullptr）
    outToken = inToken;
    return ERR_OK;
}
```

#### 6.5.3 CheckInvokeFunctionCaller（系统身份校验，AMS 侧）

```cpp
int32_t AbilityManagerService::CheckInvokeFunctionCaller()
{
    bool isSystemAppCall = PermissionVerification::GetInstance()
        ->JudgeCallerIsAllowedToUseSystemAPI();   // SA || shell || 系统应用
    if (!isSystemAppCall) return ERR_NOT_SYSTEM_APP;
    bool hasPermission = PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_EXECUTE_INSIGHT_INTENT);
    if (!hasPermission) return ERR_PERMISSION_DENIED;
    return ERR_OK;
}
```

#### 6.5.4 CheckPublicFunctionCaller（三方身份预检，AMS 侧，新增）

```cpp
int32_t AbilityManagerService::CheckPublicFunctionCaller(const std::string &bundleName,
    const std::string &intentName)
{
    // 复用 InsightIntentExecuteManager::CheckCallerPermission 的三方分支：
    //   calling token 判身份（应为三方）→ 查 exported 拦门 → 校验 ACCESS_PUBLIC_FUNCTION
    // 遵循 ADR-5：系统身份 → EXECUTE_INSIGHT_INTENT；三方 → exported + ACCESS_PUBLIC_FUNCTION
}
```

#### 6.5.5 CheckCallerPermission（执行阶段权威校验，最终形态）

**文件**：`services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp`

```cpp
int32_t InsightIntentExecuteManager::CheckCallerPermission(
    uint64_t specifiedFullTokenId,
    const std::string &callerBundleName,
    const std::string &targetBundleName,
    const std::string &targetModuleName,   // 新增：exported 查询所需
    const std::string &targetIntentName,   // 新增：exported 查询所需
    int32_t userId)                        // 新增：exported 查询所需（以实际 param 结构为准）
{
    // ① 身份：calling token 权威判定（系统应用/SA/shell）
    bool isSystemAppCall = PermissionVerification::GetInstance()
        ->JudgeCallerIsAllowedToUseSystemAPI();

    if (isSystemAppCall) {
        // ② 系统路径：无论 exported，校验 EXECUTE_INSIGHT_INTENT
        bool isCallingPerm = PermissionVerification::GetInstance()->VerifyCallingPermission(
            EXECUTE_INSIGHT_INTENT_PERMISSION, specifiedFullTokenId);
        if (!isCallingPerm) return ERR_PERMISSION_DENIED;
        return ERR_OK;
    }

    // ③ 三方：先查 exported（新增一次查询），私有函数直接拦（防泄露）
    bool exported = GetInsightIntentExported(targetBundleName, targetModuleName,
        targetIntentName, userId);
    if (!exported) return ERR_FUNCTION_NOT_EXIST;    // 与「不存在」同码，防泄露

    // ④ 三方：校验 ACCESS_PUBLIC_FUNCTION
    bool hasPublicPerm = PermissionVerification::GetInstance()->VerifyCallingPermission(
        PERMISSION_ACCESS_PUBLIC_FUNCTION, specifiedFullTokenId);
    if (!hasPublicPerm) return ERR_PERMISSION_DENIED;
    return ERR_OK;
}
```

> **`GetInsightIntentExported`（新增辅助查询）**：按 `{bundleName, moduleName, intentName, userId}` 查询目标意图 profile，返回 `ExtractInsightIntentInfo.exported`（或 `InsightIntentInfo.exported`）。绝不读 `genericInfo`（`ExtractInsightIntentGenericInfo` 无 exported 字段）。

**调用链**：`CheckAndUpdateParam` → `CheckCallerPermission`（自判 + 自查）→ `ValidateExecuteParam` → `AcquireExecuteRecord`。

#### 6.5.6 ExecuteIntentCommon → StartAbilityWithInsightIntent

`ExecuteIntentCommon` 负责构建目标 Want（`InsightIntentExecuteManager::GenerateWant`）并触发前台意图启动。**`options.callerAbilityToken` 透传为启动的 `callerToken`**：

```cpp
switch (param->executeMode_) {
    case AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND:
        ret = StartAbilityWithInsightIntent(want, param->userId_, DEFAULT_INVAL_VALUE,
            options.specifiedFullTokenId, options.callerAbilityToken);   // 补第 5 参
        break;
    case AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND:
        ret = StartAbilityByCallWithInsightIntent(want, options.callerAbilityToken,
            *param, param->userId_, options.specifiedFullTokenId);       // 替换 hostClient
        break;
    // ... SERVICE_EXTENSION / UI_EXTENSION 不变
}
```

最终经 `StartAbilityWithInsightIntent` / `StartAbilityByCallWithInsightIntent` → `StartAbilityInner`，`StartAbilityWrapParam.callerToken = callerAbilityToken`，`StartAbilityInner` 内 `Token::GetAbilityRecordByToken(param.callerToken)` 拿到真实调用方 record。

#### 6.5.7 exported 的来源（谁赋值）

`exported` 是**目标应用开发者在意图 profile JSON 里声明的布尔字段**（键名 `"exported"`），随目标 HAP 打包、BMS 安装落库、AMS 查询读出。完整链：

```
目标 HAP 的 insight intent profile JSON → "exported": bool
  insight_intent_profile.cpp:373-380   解析 → InsightIntentProfileInfo.exported
  insight_intent_profile.cpp:438-440   info.exported = insightIntent.exported（Form/UIExtension 不拷贝）
  extract_insight_intent_profile.cpp:784-786
        info.exported = insightIntent.exported → ExtractInsightIntentInfo.exported
        （DECORATOR_FORM / IsUIExtensionIntent 不拷贝）
  function_call_convert.cpp:181/:204   func.visible = info.exported
```

**关键事实**：

1. 默认 `false`（私有），开发者不写/写非布尔值即 `false` —— 安全默认。
2. Form 与 UIExtension 意图**恒为 `false`**（两处都不拷贝 exported），天然私有，三方不可按公共函数调用。
3. 属于服务端可信数据（目标应用声明 + BMS 落库），调用方进程无法伪造 —— 这是它能作为「三方可见性门」依据的根本原因，与「调用方身份」同层可信。

### 6.6 目标侧回调

**文件**：`frameworks/native/insight_intent/insight_intent_context/insight_intent_context.cpp`

目标应用通过 `InsightIntentContext::StartAbilityByInsightIntent` 收到执行回调（`AbilityManagerClient → AMS → StartAbility`）。本方案不跨进程传输任何身份标志，且 `INSIGHT_INTENT_CALLER_TOKEN` 已在 §6.5.1 剥离，目标应用收不到额外字段。

---

## 7. 参数传输与常量

### 7.1 wantParam 键（传输层）

> 本方案**删除**了 `INSIGHT_INTENT_IS_PUBLIC_INVOKE`（`"ohos.insightIntent.isPublicInvoke"`）；**新增** `INSIGHT_INTENT_CALLER_TOKEN` 携带调用方 ability token。

| 常量 | 值 | 类型 | 用途 |
|---|---|---|---|
| `INSIGHT_INTENT_CALLER_TOKEN` | `"ohos.insightIntent.callerToken"` | `sptr<IRemoteObject>` | 调用方 ability token，AMS 读后剥离并校验归属（新增） |
| `INSIGHT_INTENT_TOOL_CALL_ID` | `"ohos.insightIntent.toolCallId"` | string | 工具调用 id（trace） |
| `INSIGHT_INTENT_TASK_ID` | `"ohos.task.id"` | string | 任务归因 id（系统侧使用后剥离） |
| `INSIGHT_INTENT_EXECUTE_PARAM_NAME` | `"ohos.insightIntent.executeParam.name"` | string | 意图名 |
| `INSIGHT_INTENT_EXECUTE_PARAM_ID` | `"ohos.insightIntent.executeParam.id"` | string | 意图执行记录 id |

### 7.2 consent 弹窗参数键

| 常量 | 值 |
|---|---|
| `CONSENT_PARAM_CALLER_BUNDLE` | `"ohos.insightIntent.consent.callerBundle"` |
| `CONSENT_PARAM_TARGET_BUNDLE` | `"ohos.insightIntent.consent.targetBundle"` |
| `CONSENT_PARAM_TARGET_BUNDLE_LIST` | `"ohos.insightIntent.consent.targetBundleNameList"` |
| `CONSENT_PARAM_TARGET_INTENT` | `"ohos.insightIntent.consent.targetIntent"` |
| `CONSENT_PARAM_AMS` | `"amsParam"` |
| `CONSENT_PARAM_RESULT` | `"ohos.insightIntent.consent.result"` |
| 弹窗目标 | bundle `com.huawei.hmos.security.privacycenter` / ability `CollaborationAccessAbility` / type `sys/commonUI` |

---

## 8. 权限与安全模型

### 8.1 权限清单

| 权限 | 级别 | 使用点 |
|---|---|---|
| `ohos.permission.EXECUTE_INSIGHT_INTENT` | `system_basic`（三方不可持有） | 系统路径：AMS `CheckInvokeFunctionCaller` + `CheckCallerPermission` |
| `ohos.permission.ACCESS_FUNCTION` | 系统 | 系统路径 climgr `GetFunctionInfo` |
| `ohos.permission.ACCESS_PUBLIC_FUNCTION` | 级别见权限配置文件（此处不重复校验级别） | 公共路径：climgr `GetPublicFunctionInfo` + **AMS `CheckCallerPermission`（新增）** |

### 8.2 校验点总览（最终）

| 校验点 | 位置 | 系统路径 | 公共路径 |
|---|---|---|---|
| 身份判断（弹窗/查询选择） | NAPI/executor（本地 `IsSystemAppByFullTokenID`） | 系统 | 三方 |
| **弹窗前预检**（新增） | NAPI → AMS `CheckPublicFunctionCaller` / `CheckInvokeFunctionCaller` | 系统身份 + `EXECUTE_INSIGHT_INTENT` | exported + `ACCESS_PUBLIC_FUNCTION` |
| climgr 函数授权 | executor `GetFunctionInfo` / `GetPublicFunctionInfo` | `ACCESS_FUNCTION` | `ACCESS_PUBLIC_FUNCTION` + `visible` |
| 执行阶段权威门 | AMS `CheckCallerPermission` | 系统身份 + `EXECUTE_INSIGHT_INTENT` | 查 exported 拦门 + `ACCESS_PUBLIC_FUNCTION` |
| **调用方 token 归属**（新增，仅意图执行） | AMS `ResolveCallerAbilityToken`（仅 `ExecuteIntentByFunctionCall` 调用） | 传了就用 / 不传置空 | 必须非空且属自己进程，否则拒绝 |
| **自动化控制免弹窗（UX，非授权门）**（新增） | NAPI `CheckAutomationControlAccess` | 不适用（系统不弹窗） | GRANTED→直接执行 / 其它（含 DENIED）→弹窗 |

### 8.3 信任边界

- **可信**：`GetSelfTokenID()` 的 full token、服务端 `GetCallingFullTokenID()` 的 calling token（内核/访问令牌服务维护，进程无法伪造）；目标函数的 `exported`（目标应用 profile 声明 + BMS 落库）。
- **不可信**：客户端进程注入的任何身份/路径标志（**本方案已取消跨进程传输**）、`options.context` 是否存在（客户端可省略）、客户端传入的 `INSIGHT_INTENT_CALLER_TOKEN`（可能伪造）。
- **因此**：弹窗决策 + 本地查询选择用本地身份判断；**服务端放行一律用 calling token + exported + 对应权限**；**调用方 token 必须经 `ResolveCallerAbilityToken` 归属校验**（系统/SA 传了就用、不传置空；三方必须非空且属自己），三者都不信任客户端声明。该 token 校验仅在意图执行入口做。

### 8.4 防弹窗轰炸

- **预检**（ADR-7）：三方调用在弹窗前先经 AMS 授权校验，未授权/目标不可见直接失败不弹窗。
- **自动化控制免弹窗**（ADR-11）：已合法持有 `ACCESS_PUBLIC_FUNCTION` 的应用对 exported 函数，若已设置过「可控制目标」则仅 `NO_POPUP_NEEDED_GRANTED` 免弹窗直接执行；`NO_POPUP_NEEDED_DENIED` 及其它情况仍照常弹窗。
- **遗留风险**：既未设置自动化控制、又反复发起的三方调用**仍可**反复触发弹窗。彻底防轰炸需在弹窗挂载层加「同一 (caller, target) 去重 / 频控」，属后续增强，不在本次范围。

---

## 9. 错误码

| 场景 | 错误码 |
|---|---|
| 三方缺 context | `ERR_INNER_PARAM_INVALID` |
| 授权弹窗被拒 / 出错 / 释放 | `ERR_PERMISSION_DENIED` |
| 系统路径非系统应用 | `ERR_NOT_SYSTEM_APP` |
| 系统路径缺 `EXECUTE_INSIGHT_INTENT` | `ERR_PERMISSION_DENIED` |
| 三方缺 `ACCESS_PUBLIC_FUNCTION` | `ERR_PERMISSION_DENIED` |
| 函数不存在 / 不可见（exported=false） | `ERR_FUNCTION_NOT_EXIST`（两者同码，防泄露） |
| 三方未传 / 传入非自身调用方 token | `ERR_PERMISSION_DENIED`（ADR-10，直接拒绝） |
| 函数类型非 intent function | `ERR_INNER_PARAM_INVALID` |
| 执行失败（通用） | `ERR_FUNCTION_EXECUTE_FAILED` |

> 错误码数值均为既有公共合约，重写时不得改动数值。

---

## 10. 边界条件与异常处理

1. **三方 + context 空**：`reject(ERR_INNER_PARAM_INVALID)`，绝不静默跳过弹窗。
2. **三方 + 预检失败**：`reject(预检错误码)`，绝不弹窗（防轰炸）。
3. **弹窗回调多次触发**：`std::atomic<bool> fired` 保证 settle 恰好一次。
4. **图形不支持（`SUPPORT_GRAPHICS` 未定义）**：三方路径 `settle(false)` → 拒绝。
5. **跨用户**：AMS `CheckCrossUserPermission` 拦截；climgr 函数查询严格按 IPC caller 的 userId，不支持跨用户。
6. **函数不存在 vs 不可见**：climgr 公共查询与 AMS `CheckCallerPermission` 都返回 `ERR_FUNCTION_NOT_EXIST`，不泄露存在性。
7. **Form / UIExtension 意图**：exported 恒为 false，三方调用一律按「不可见」拒绝。
8. **调用方 token 归属（仅意图执行）**：系统/SA 传了 token 则直接用、不传则置空（唯一可 null 的角色）；三方不传 token 或传非自己 token → `ERR_PERMISSION_DENIED`；token 在 `GenerateWant` 前已剥离，不泄漏进目标 Want。
9. **系统路径无 context**：`callerAbilityToken = nullptr`，`StartAbilityInner` 走既有 nullptr 行为（用 calling tokenId 归属）。
10. **hook 修改的 trace id 非法**：`IsValidTraceId` 校验，非法则丢弃（toolCallId/dmSessionId）。
11. **executor 失败上报**：`ExecuteIntentByFunctionCall` 返回 `ERR_PERMISSION_DENIED`/`ERR_NOT_SYSTEM_APP` 时原样上报，其余映射为 `ERR_FUNCTION_EXECUTE_FAILED`。
12. **自动化控制免弹窗查询失败/目标 token 解析失败**：controller token 查询失败、目标 token 为 `INVALID_TOKENID`、`CheckAutomationControlAccess` 非 `RET_SUCCESS` 或 `decisions.size() != 1` 时，一律回退「照常弹窗」（不静默放行、不静默拒绝）。

---

## 11. 重写要点（从当前实现到目标实现的差异）

> 重写代码时的检查清单。

### 11.1 删除项

- 删除 `INSIGHT_INTENT_IS_PUBLIC_INVOKE`（`insight_intent_execute_param.h`）传输常量及其写入（executor）、读取/剥离（`ExecuteIntentByFunctionCall`）。
- 删除 `ExecuteParam.isPublicInvoke`、`ExecuteIntentCommonOptions.isPublicInvoke` 字段。
- 删除 `CheckCallerPermission` 的 `bool isPublicInvoke` 参数与短路分支。

### 11.2 新增项

- `PermissionConstants::PERMISSION_ACCESS_PUBLIC_FUNCTION`（`services/common/include/permission_constants.h`）。
- `CheckPublicFunctionCaller(bundleName, intentName)`：AMS 侧方法 + IntentClient 侧方法 + IPC code（`CHECK_PUBLIC_FUNCTION_CALLER`）+ Stub/Proxy 反/序列化。
- `GetInsightIntentExported(...)`：`CheckCallerPermission` 内部新增的 exported 查询辅助方法。
- `INSIGHT_INTENT_CALLER_TOKEN`（`insight_intent_execute_param.h`）：wantParam 键。
- `ResolveCallerAbilityToken(...)`：AMS 侧 token 归属校验（ADR-10）。
- `ExecuteIntentCommonOptions.callerAbilityToken`：透传字段。
- executor `Execute(...)` 新增 `callerAbilityToken` 入参。
- NAPI `StartInvokeFunction` 三方分支：预检通过后调用 `AccessTokenKit::CheckAutomationControlAccess`（ADR-11），按决策 `submit` 或回退弹窗。
- NAPI BUILD.gn `external_deps` 新增 `access_token:libaccesstoken_sdk`（`AccessTokenKit` + `automation_control_types.h`）；`js_function_manager.cpp` 新增 `#include "accesstoken_kit.h"`。

### 11.3 修改项

- `CheckCallerPermission`：改为「calling token 判身份 → 系统查 `EXECUTE_INSIGHT_INTENT` / 三方查 exported + `ACCESS_PUBLIC_FUNCTION`」。
- NAPI `StartInvokeFunction`：三方分支在弹窗前加 `CheckPublicFunctionCaller` 预检与 `CheckAutomationControlAccess` 免弹窗判断；提取 `callerAbilityToken`。
- executor `DoExecute`：删除 wantParam 注入 isPublicInvoke；写入 `INSIGHT_INTENT_CALLER_TOKEN`。
- `ExecuteIntentCommon`：前台补 `StartAbilityWithInsightIntent` 第 5 参、后台替换 `StartAbilityByCallWithInsightIntent` 的 `callerToken` 为 `options.callerAbilityToken`。

---

## 12. 测试要点

### 12.1 单元测试

- **NAPI 身份分流**：mock `IsSystemAppByFullTokenID` 返回 true/false，验证分支（系统不弹窗 / 三方缺 context 报错 / 三方预检失败不弹窗 / 三方预检通过弹窗）。
- **预检拦截**：三方无 `ACCESS_PUBLIC_FUNCTION` 或目标 exported=false → `CheckPublicFunctionCaller` 返回错误、无弹窗。
- **CheckCallerPermission**：系统→走 `EXECUTE_INSIGHT_INTENT`；三方 exported=true + 有权限→`OK`；三方 exported=false→`ERR_FUNCTION_NOT_EXIST`；三方无权限→`ERR_PERMISSION_DENIED`。
- **exported 查询**：`GetInsightIntentExported` 正确取 `ExtractInsightIntentInfo.exported`（Form/UIExtension 返回 false）。
- **token 归属**：`ResolveCallerAbilityToken`：系统/SA+nullptr→outToken=nullptr；系统/SA+token→outToken=inToken；三方+nullptr→`ERR_PERMISSION_DENIED`；三方+非自己 token→`ERR_PERMISSION_DENIED`；三方+自己 token→outToken=inToken。
- **自动化控制免弹窗**：mock `CheckAutomationControlAccess` 返回 `NO_POPUP_NEEDED_GRANTED`/`NO_POPUP_NEEDED_DENIED`/`POPUP_NEEDED`/`TARGET_NOT_EXIST`/非 `RET_SUCCESS`，验证对应「直接 submit / 弹窗 / 弹窗 / 弹窗」分支；目标 token 解析失败（`GetHapTokenInfo` 失败或 `GetHapTokenID` 返回 `INVALID_TOKENID`）→ 回退弹窗。

### 12.2 集成/模块测试

- 系统应用 `invokeFunction` 成功（不弹窗，token=nullptr）。
- 三方 `invokePublicFunction` + context → 预检通过 → 弹窗 → 同意执行 / 拒绝 `ERR_PERMISSION_DENIED`。
- 三方 `invokePublicFunction` 缺 context → `ERR_INNER_PARAM_INVALID`。
- 三方调用 `visible=false` 函数 → 预检 climgr/AMS 返回 `ERR_FUNCTION_NOT_EXIST`，不弹窗。
- 三方调用 exported 函数 → `StartAbilityInner` 收到调用方 token，caller record 归属正确。
- 三方已 `setAutomationControlStatus(ENABLED)` 目标 → 再次 `invokePublicFunction` 不弹窗、直接执行；`DISABLED` → 照常弹窗（由用户最终决定）。

### 12.3 安全测试

- 绕过 NAPI 直接构造 `EXECUTE_INTENT_BY_FUNCTION_CALL`（不携带任何标志）：三方身份调用 exported 函数需 `ACCESS_PUBLIC_FUNCTION`；调用私有函数被 `ERR_FUNCTION_NOT_EXIST` 拦；系统身份调用需 `EXECUTE_INSIGHT_INTENT`。
- 三方在 `INSIGHT_INTENT_CALLER_TOKEN` 塞其他应用的 token → `ResolveCallerAbilityToken` 拒绝 `ERR_PERMISSION_DENIED`。

---

## 附录 A：关键数据结构

### A.1 ExecuteParam（`insight_intent_execute_manager.h`）—— 删除 `isPublicInvoke`

```cpp
struct ExecuteParam {
    std::string callerBundleName;
    bool ignoreAbilityName = false;
    bool isDistributed = false;
    std::string srcDeviceId;
    uint64_t requestCode = 0;
    uint64_t specifiedFullTokenId = 0;
    int32_t callerAppIndex = 0;
    int32_t targetAppIndex = 0;
    // 删除：bool isPublicInvoke = false;
};
```

### A.2 ExecuteIntentCommonOptions（`extract_insight_intent_profile.h`）—— 删除 `isPublicInvoke`，新增 `callerAbilityToken`

```cpp
struct ExecuteIntentCommonOptions {
    bool ignoreAbilityName = false;
    ExtractInsightIntentGenericInfo infos;
    ExtractInsightIntentInfo extractInfo;    // 含 exported
    InsightIntentInfo configInfo;
    uint64_t key = 0;
    std::string srcDeviceId;
    uint64_t requestCode = 0;
    uint64_t specifiedFullTokenId = 0;
    // 删除：bool isPublicInvoke = false;
    sptr<IRemoteObject> callerAbilityToken = nullptr;   // 新增：启动归属
    ExecuteIntentCommonOptions(bool ignoreAbilityName,
        const ExtractInsightIntentGenericInfo &infos, uint64_t key)
        : ignoreAbilityName(ignoreAbilityName), infos(infos), key(key) {}
};
```

### A.3 ExtractInsightIntentInfo（全量信息，含 exported）

```cpp
struct ExtractInsightIntentInfo {
    std::vector<InsightIntentEntityInfo> entities;
    std::vector<std::string> keywords;
    std::vector<std::string> permissions;
    ExtractInsightIntentGenericInfo genericInfo;   // 注意：genericInfo 不含 exported
    std::string decoratorFile;
    std::string decoratorClass;
    std::string displayDescription;
    std::string domain;
    std::string intentVersion;
    std::string schema;
    std::string icon;
    std::string llmDescription;
    std::string result;
    std::string example;
    std::string arkTSMode;
    bool exported = false;                         // ← 三方可见性门（服务端可信）
    ExtractInsightIntentInfo() = default;
};
```

### A.4 FunctionInfo（climgr 函数信息）

`functionNamespace`（目标 bundle）、`functionName`、`functionType`、`visible`（= `info.exported`）。

---

## 附录 B：关键常量

```cpp
// 权限（permission_constants.h，AMS 侧需新增第三条）
constexpr const char* PERMISSION_EXECUTE_INSIGHT_INTENT = "ohos.permission.EXECUTE_INSIGHT_INTENT";
constexpr const char* PERMISSION_ACCESS_FUNCTION       = "ohos.permission.ACCESS_FUNCTION";
constexpr const char* PERMISSION_ACCESS_PUBLIC_FUNCTION = "ohos.permission.ACCESS_PUBLIC_FUNCTION";  // 新增

// 传输键（insight_intent_execute_param.h）
constexpr char INSIGHT_INTENT_CALLER_TOKEN[] = "ohos.insightIntent.callerToken";   // 新增

// climgr 侧（cli_tool_manager_service.cpp，已有）
constexpr const char* PERMISSION_ACCESS_FUNCTION       = "ohos.permission.ACCESS_FUNCTION";
constexpr const char* PERMISSION_ACCESS_PUBLIC_FUNCTION = "ohos.permission.ACCESS_PUBLIC_FUNCTION";
```

### 身份判断 / token 获取 API

```cpp
#include "ipc_skeleton.h"    // IPCSkeleton::GetSelfTokenID()（调用方）/ GetCallingFullTokenID()（服务端）
#include "tokenid_kit.h"     // Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID
#include "accesstoken_kit.h" // Security::AccessToken::AccessTokenKit（GetHapTokenInfo / GetHapTokenID / CheckAutomationControlAccess）

// 调用方 ability token（NAPI 侧）
sptr<IRemoteObject> callerAbilityToken = abilityContext->GetToken();   // Context::GetToken() → sptr<IRemoteObject>

// 自动化控制免弹窗（NAPI 侧，三方预检通过后）
AccessTokenID controllerTokenId = static_cast<AccessTokenID>(IPCSkeleton::GetSelfTokenID());
HapTokenInfo controllerHapInfo;
AccessTokenID targetTokenId = (GetHapTokenInfo(controllerTokenId, controllerHapInfo) == RET_SUCCESS)
    ? GetHapTokenID(controllerHapInfo.userID, functionNamespace, 0) : INVALID_TOKENID;
// CheckAutomationControlAccess(controllerTokenId, {targetTokenId}, decisions) → 决策见 ADR-11

// 归属校验（AMS 侧）
auto record = Token::GetAbilityRecordByToken(inToken);                 // 非空且 record->GetUid() == GetCallingUid()
```

> NAPI 模块需在 BUILD.gn `external_deps` 加 `access_token:libaccesstoken_sdk`（`AccessTokenKit` + `automation_control_types.h`）与 `access_token:libtokenid_sdk`（`TokenIdKit`；`ipc:ipc_single` 提供 IPCSkeleton）。