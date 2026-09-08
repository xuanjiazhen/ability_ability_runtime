# OpenHarmony 应用进程拉起 API 全景分析

> —— "应用定义任务 ID（taskId）关联该任务拉起进程的 PID 列表" 特性设计支撑文档

## 0. 文档信息

| 项 | 说明 |
|---|---|
| 代码基线 | OpenHarmony master（2026-09 工作区快照），ability_runtime 主分支 |
| 分析范围 | 所有"可能导致应用进程被创建"的应用侧 API（public / system）、系统服务路径、非 appspawn 路径，及其到进程诞生的完整代码链 |
| 代码根目录 | `foundation/ability/ability_runtime`（下称 `<ART>`，文中相对路径均基于此）；全源码树位于工作区 `code/` 根 |
| d.ts 权威目录 | `interface/sdk-js/api/`（public/system 分类以 `@systemapi` 标记为准） |
| 证据形式 | 所有结论均给出 `文件:行号`（以当前快照为准，行号会随演进漂移，函数名为稳定锚点） |

---

## 1. 结论速览

1. **应用进程的诞生收敛在极少数"咽喉点"**：AMS（SA 180）侧最终只有 3 个 `LoadAbility` 出口；AppMS（SA 501）侧只有一个 `AppMgrServiceInner::StartProcess`（主进程）加 3 个旁路（子进程 / 渲染进程 / 镜像·测试·perf）。全部经 **spawn 服务家族（appspawn / cjappspawn / hybridspawn / nwebspawn / nativespawn）** 的 unix socket 完成 fork。
2. **"调用拉起 API" ≠ "产生新进程"**：进程复用判断在 AppMS（`CheckAppRunningRecordIsExist`，按 processName + instanceKey + specifiedProcessFlag + customProcessFlag 匹配）。做 taskId→PID 关联时必须区分"新拉起"与"复用已有进程"。
3. **AMS 侧判断的是组件实例，不是进程**：AMS 的 `isReady_` / `isLoadedAbility` 判断只决定"要不要发 LoadAbility 给 AppMS"，真正的进程级判断在 AppMS。
4. **存在 4 条不经 AMS LoadAbility、直接由 AppMS 拉起进程的旁路**：用户测试进程（`StartUserTest`）、预加载（`PreloadApplication`/`PreloadApplicationByPhase`/`PreloadExtension`）、常驻进程（`StartupResidentProcess`）、渲染进程（`StartRenderProcess`→nwebspawn）。
5. **存在 3 类完全绕开 AppMS 的进程创建**：应用内 SELF_FORK 子进程（本地 `fork()`）、CLI 工具沙箱进程（climgr 直接 `fork+execvp`）、理论上应用 native 层的裸 fork（非 API 口径）。做 PID 全量关联时这些是观测盲区。
6. **外部子系统是拉起 Extension 进程的主力**：Form、WorkScheduler、DataShare、输入法、无障碍、通知点击、打印、账号认证等 30+ 子系统全部经 `ConnectAbility/ConnectAbilityCommon` 系入口打 AMS。三方 API 中 `createDataShareHelper`、`startWork`、`publish`（点击）等都会间接拉起他人应用进程。
7. **SCB（SceneBoard）是默认产品形态**：UIAbility 启动会经 SCB 异步回调（`PendingSessionActivation` → `StartUIAbilityBySCB`），**原始调用方 IPC 上下文在中途被替换为 SCB**——taskId 透传必须放业务数据（Want/sessionInfo），不能依赖 IPC calling identity。

---

## 2. 分析方法论与完整性保证

单一方向的列举容易遗漏。本文采用**三向交叉验证**：

### 2.1 正向枚举法（接口 → 实现）

从 IPC 接口定义出发，穷举所有"启动/连接/执行"语义方法：

| 接口 | 定义位置 | 规模 | 结论 |
|---|---|---|---|
| `IAbilityManager`（SA 180） | `<ART>/interfaces/inner_api/ability_manager/include/ability_manager_interface.h`（手写 IPC，2916 行） | 60+ 启动类方法 | 全部归类（附录 A） |
| `IAppMgr` / `IAmsMgr`（SA 501） | `<ART>/interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h`、`ams_mgr_interface.h` | 15+ 进程创建相关方法 | 全部归类 |
| `IAgentManager`（SA 185） | `<ART>/agent_runtime_framework/interfaces/inner_api/IAgentManager.idl` | connect 系 3 个 | 全部归类 |
| `IServiceRouterMgr`（SA 404） | `<ART>/service_router_framework/interfaces/inner_api/IServiceRouterMgr.idl` | Start/ConnectUIExtensionAbility（SA 内部用） | 已归类 |

### 2.2 反向汇聚法（spawn 出口 → 上游）

从"进程真正诞生的地方"逆推（详见附录 B）：

- `AppSpawnClient::StartProcess`（`services/appmgr/src/app_spawn_client.cpp:661`）在 appmgr 内共 **7 个调用点**（主进程×3 分流 / 子进程 / 渲染 / perf / 镜像 worker）；
- `AppSpawnClient::StartImageProcess`（`app_spawn_client.cpp:715`，镜像模板进程）；
- climgr 的 `fork()+execvp()`（CLI 沙箱，独立于 spawn 家族）；
- 应用进程内的本地 `fork()`（SELF_FORK 子进程模式）。

每个出口向上追到 IPC 入口、再追到 JS/C API 面，与 2.1 的结果对账。

### 2.3 横向扫描法（全源码树）

在 `<ART>` 之外对全源码树 grep：`AbilityManagerClient` / `ExtensionManagerClient` / `IAbilityManager` / `GetSystemAbility(ABILITY_MGR_SERVICE_ID)` / 手写 parcel 直发，共命中 **50+ 子系统**，其中 30+ 有实际拉起行为（第 5 章、附录 C）。

### 2.4 边界口径

- **计入**：appspawn 家族孵化的应用进程（含主进程、多实例进程、Extension 进程、隔离进程、子进程、渲染进程）、climgr 沙箱进程。
- **不计入**：系统自身常驻进程（init 直接拉起）、HDI 服务进程、内核线程。
- **单机口径**：分布式远端拉起发生在对端设备（本机仅转发），本机 PID 列表不含远端进程。

---

## 3. 进程拉起的底层统一机制

### 3.1 AMS 侧：三个 LoadAbility 汇聚点

```
所有 AMS IPC 入口
   │（拦截器链 services/abilitymgr/src/interceptor/、权限校验 PermissionVerification）
   ▼
┌─────────────────────────────────────────────────────────────────────┐
│ 汇聚点① AbilityRecord::LoadAbility          ability_record.cpp:310 │ ← UIAbility / DataAbility / ByCall / 预启动
│ 汇聚点② AbilityConnectManager::LoadAbility  ability_connect_manager.cpp:1310 │ ← ServiceExtension / 普通 Extension / Connect
│            └ UIExtensionAbilityManager 覆写  ui_extension_ability_manager.cpp:737 │ ← UIExtension / AgentUIExtension / AutoFill
│ 汇聚点③ (直发) AppScheduler::StartSpecifiedAbility/StartUserTest 等                       │ ← AcceptWant 两段式 / 测试
└─────────────────────────────────────────────────────────────────────┘
   ▼（三个汇聚点最终都调）
AppScheduler::LoadAbility                 app_scheduler.cpp:70
   → appMgrClient_->LoadAbility(...)      app_scheduler.cpp:82-83   ← AMS 进程的出口（IPC → SA 501）
```

拉起判断（组件实例级，不是进程级）：

| 场景 | 判断位置 | 语义 |
|---|---|---|
| SCB 模式 UIAbility | `AbilityRecord::isReady_`（ability_record.cpp:501） | 目标 ability 线程是否已 attach |
| 非 SCB UIAbility | mission_ability_record.cpp:221-259 | 同上 |
| Service/Extension | `GetOrCreateServiceRecord`（ability_connect_manager.cpp:3388，`isLoadedAbility`@3452） | serviceMap_ 无记录 |
| UIExtension | `GetOrCreateExtensionRecord`（ui_extension_ability_manager.cpp:955） | 记录管理器无记录 |
| ByCall | `ResolveAbility` 返回 OK_HAS_REMOTE_OBJ / persistentId 已存在 | scheduler 已存在 |
| DataAbility(FA) | data_ability_manager.cpp:82-85 | 该 authority 未加载 |

### 3.2 AppMS 侧：LoadAbility → StartProcess → spawn socket

```
AppMgrClient::LoadAbility                        interfaces/inner_api/app_manager/src/appmgr/app_mgr_client.cpp:162
  → [IPC] AmsMgrProxy::LoadAbility               ams_mgr_proxy.cpp:74（LOAD_ABILITY=0）
  → AmsMgrStub::HandleLoadAbility                ams_mgr_stub.cpp:250
  → AmsMgrScheduler::LoadAbility                 services/appmgr/src/ams_mgr_scheduler.cpp:73（FFRT 任务 "LoadAbilityTask"）
  → AppMgrServiceInner::LoadAbility              services/appmgr/src/app_mgr_service_inner.cpp:2075
      ├─ 复用判断 CheckAppRunningRecordIsExist    app_running_manager.cpp:123-192   ★（见 3.4）
      │    命中 → HandleExistingAppRecordAfterFound（inner:2325）→ 向已有进程调度，不 spawn
      ├─ 未命中 → CreateAppRunningRecord          inner:2290（app_running_manager.cpp:66，SetInstanceKey:100）
      │           → LoadAbilityNoAppRecord        inner:2672
      │                → StartProcess             inner:6059                        ★主进程唯一汇聚点
      │                     ├─ CJ 应用  → GetCJSpawnClient()->StartProcess    inner:6142（cjappspawn）
      │                     ├─ ArkTS1.2/Hybrid → GetHybridSpawnClient()      inner:6148（hybridspawn）
      │                     └─ 普通    → GetSpawnClient()->StartProcess      inner:6152（appspawn）
      └─ 成功后 SetPid/SetStartMsg/SetSpawned、OnProcessCreated 等    inner:6167-6188
  → AppSpawnClient::StartProcess                  services/appmgr/src/app_spawn_client.cpp:661
      → AppSpawnReqMsgCreate / AppspawnCreateDefaultMsg(:517) / AppSpawnClientSendMsg(:693)  ← 同步读回 pid
```

### 3.3 spawn 服务家族（5 个 socket 客户端）

`RemoteClientManager`（`services/appmgr/src/remote_client_manager.cpp:25-30`）持有 5 个孵化客户端：

| 客户端 | 服务名 | 用途 | 触发链 |
|---|---|---|---|
| `appSpawnClient_` | appspawn | 普通应用主进程 / JS·ARK 子进程 | StartProcess:6152、StartChildProcessImpl:10972 |
| `nwebSpawnClient_` | NWebSpawn | ArkWeb 渲染/GPU 进程 | StartRenderProcessImpl:8689 |
| `cjAppSpawnClient_` | cjappspawn | 仓颉（CJ）应用 | StartProcess:6142 |
| `nativeSpawnClient_` | nativespawn | NATIVE_ARGS 子进程 | StartChildProcessImpl（类型判断 :10925-10927） |
| `hybridSpawnClient_` | hybridspawn | ArkTS 1.2 / 混合语言应用 | StartProcess:6148 |

消息类型（`startMsg.code`）：`MSG_APP_SPAWN`(0，普通)、`MSG_SPAWN_NATIVE_PROCESS`(1，perf/debug)、`MSG_GET_RENDER_TERMINATION_STATUS`(2)、`MSG_SPAWN_WORKER_PROCESS`（镜像 fork worker，inner:1536）、`MSG_SPAWN_IMAGE_PROCESS`（镜像模板，StartImageProcess）。

### 3.4 进程复用判断（taskId→PID 特性的关键前提）

`AppRunningManager::CheckAppRunningRecordIsExist`（`services/appmgr/src/app_running_manager.cpp:123-192`）命中条件（全部同时满足）：

1. 进程名相同（`MakeProcessName` 产物：Stage PAGE 为 `bundleName[:process]`；隔离进程 `bundle:module`；克隆/DLP `:appIndex`；specifiedProcessFlag `:flag`；ServiceExt 多进程白名单可加后缀）；
2. `instanceKey` 相同（多实例核心；默认 `APP_INSTANCE_KEY_0`，来源 `Want::APP_INSTANCE_KEY`，透传链 multi_instance_utils.cpp:32 → ability_record.cpp:215/346 → loadParam.instanceKey → AppRunningRecord::SetInstanceKey）；
3. specifiedProcessFlag / customProcessFlag 匹配；
4. 不处于排除态（Terminating / Killing / RestartAppFlag / UserRequestCleaning / 缓存阻塞 / KillPrecedeStart）；
5. bundleName+uid 匹配（联合应用走 signCode 维度，**不比较 instanceKey**）。

**推论**：同一应用的多次 startAbility 通常只有一个主进程；`instanceKey`（多实例/应用克隆）、隔离进程、ServiceExt 多进程、NewProcessMode（`bundle:module:ability:requestId` 每次必新）才会产生新 PID。

### 3.5 绕过 AMS LoadAbility、直达 AppMS 的拉起旁路

| 旁路 | IPC 入口 | 链 | spawn |
|---|---|---|---|
| 用户测试进程 | `IAbilityManager::StartUserTest`（ams.cpp:11715）→ `AppScheduler::StartUserTest`（app_scheduler.cpp:524） | → `IAppMgr::StartUserTestProcess`（app_mgr_service.cpp:871）→ inner:7234 → `StartEmptyProcess`(:7378) | appspawn（测试空进程） |
| 分阶段预加载 | `IAbilityManager::PreloadApplication`（ams.cpp:18902 → preload_manager_service.cpp:49） | phase≤ABILITY_STAGE_CREATED → `PreloadApplicationByPhase`；窗口阶段走完整 StartAbility 链 | appspawn（preload 进程） |
| AppMS 直连预加载 | `IAppMgr::PreloadApplication`（app_mgr_interface.h:85）/ `PreloadExtension`(:1243) | inner:646 / inner:1722 | appspawn |
| 常驻进程 | `IAppMgr::StartupResidentProcess`(:404)（上游 BMS 更新 → AMS ResidentProcessManager） | inner:6920 → `StartEmptyResidentProcess`(:6988) | appspawn |
| 渲染进程 | `IAppMgr::StartRenderProcess`(:531)（上游 ArkWeb 适配层） | inner:8645 → `StartRenderProcessImpl`(:8689) | **nwebspawn** |
| 隔离/指定进程 | `IAmsMgr::StartSpecifiedProcess`(:314) | inner:537-628 → StartProcess:620 | appspawn |
| perf/debug 进程 | `IAppMgr::StartNativeProcessForDebugger`(:729)（hdc debug）；StartPerfProcess（shell） | inner:9958 → `StartPerfProcessByStartMsg`(:5560) | appspawn（MSG_SPAWN_NATIVE_PROCESS） |
| 镜像 | `IAppMgr::MakeImage`(:100) | `PreAddImageInfo` → `StartImageProcess` | appspawn（镜像模板） |

### 3.6 非 AppMS 的进程创建（观测盲区）

1. **应用内 SELF_FORK 子进程**：`childProcessManager.startChildProcess(srcEntry, StartMode.SELF_FORK)` → 应用进程内直接 `fork()`（`frameworks/native/ability/native/child_process_manager/child_process_manager.cpp:97`）。AppMS 不可见（子进程事后通过 `GetChildProcessInfoForSelf`+`AttachChildProcess` 注册，AppMS 才有记录）。
2. **CLI 工具沙箱进程**：`cliManager.execTool/execCmd`（systemapi）→ climgr `ProcessManager::CreateChildProcess/CreateShellProcess`（`cli_tool_framework/services/climgr/src/process_manager.cpp:139/203`）→ **直接 `fork()+execvp("/system/bin/claw_sandbox")`**，不走 spawn 家族。
3. **应用 native 层裸 fork/exec**：非 API 口径（沙箱与 seccomp 策略管控），不计入。

---

## 4. ability_runtime 内的进程拉起 API 与代码链

### 4.0 代码链模板（后文引用）

**模板 A（UIAbility 前台链，SCB 默认产品形态）**
```
JS/ArkTS API → NAPI/ANI 绑定 → Context 实现（AbilityContextImpl 等）
  → AbilityManagerClient::StartAbility 系（IPC）
  → AMS::StartAbility*（ams.cpp:731/815/2025/2285）→ StartAbilityInner(ams.cpp:1360) / StartAbilityForOptionInner(ams.cpp:2400)
  → 拦截器链 + 权限 → uiAbilityManager->NotifySCBToStartUIAbility（ualm.cpp:761）
  → SCB 回调 AMS::StartUIAbilityBySCB（ams.cpp:3335）→ StartUIAbilityBySCBDefaultCommon（ams.cpp:3573）
  → UIAbilityLifecycleManager::StartUIAbility（ualm.cpp:205）→ ProcessColdStartBranch（ualm.cpp:114）
  → AbilityRecord::ProcessForegroundAbility（ar.cpp:480）→ [!isReady_] LoadAbility（ar.cpp:505）★汇聚点①
  → AppScheduler::LoadAbility（asch.cpp:70）→ AppMS 主干（3.2）→ appspawn
```
非 SCB 形态：`MissionListManager::StartAbility`（mlm.cpp:167）→ … → mission_ability_record.cpp:259 `LoadAbility()`，殊途同归。

**模板 B（Extension/Connect 链）**
```
API → Context/ConnectionManager → AbilityManagerClient::ConnectAbility / ConnectAbilityWithExtensionType
  / StartExtensionAbility（IPC）
  → AMS::ConnectAbilityCommon（ams.cpp:6063）或 StartExtensionAbilityInner（ams.cpp:4656）
  → (UIExtension ? UIExtensionAbilityManager : CommonExtensionManager)::GetOrCreate*Record [!isLoadedAbility]
  → AbilityConnectManager::LoadAbility（acm.cpp:1310）★汇聚点② → AppScheduler → AppMS → appspawn
```

**模板 C（子进程链）**
```
API → ChildProcessManager（应用进程内）
  → [APP_SPAWN_FORK] AppMgrClient/GetSystemAbility(501) → IAppMgr::StartChildProcess（IPC）
  → AppMgrService::StartChildProcess（app_mgr_service.cpp:1814）→ Inner::StartChildProcess（inner:10799）
  → StartChildProcessImpl（inner:10912）→ spawnClient->StartProcess（inner:10972）★
  → [SELF_FORK] 本地 fork()（不经 AppMS）
```

### 4.1 Public API（三方应用可用）

#### 4.1.1 UIAbilityContext（`application/UIAbilityContext.d.ts`；实现 `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:2475+` 绑定，`ability_context_impl.cpp`）

| API | @since | 启动组件 | 代码链要点 |
|---|---|---|---|
| `startAbility(want)` ×3 重载 | 9 | UIAbility | 模板 A（js_ability_context.cpp:285→688→738 → ability_context_impl.cpp:201 → AMC::StartAbility:205） |
| `startAbilityForResult` ×3 | 9 | UIAbility | 模板 A（ability_context_impl.cpp:273/298） |
| `startAbilityByCall` | 9 | UIAbility（后台拉起+Caller） | → LocalCallContainer::StartAbilityByCallInner（local_call_container.cpp:25）→ AMC::StartAbilityByCall:71 → AMS::StartAbilityByCallWithErrMsg（ams.cpp:10390）→（SCB）CallUIAbilityBySCB（ams.cpp:14134）→ ualm.cpp:1947 `LoadAbility()`；仅自应用 |
| `connectServiceExtensionAbility` | 9 | ServiceExtensionAbility | → ConnectionManager::ConnectExtensionAbilityWithExtensionType（connection_manager.cpp:49→162）→ AMC::ConnectAbilityWithExtensionType → AMS::ConnectAbilityCommon → 模板 B |
| `openLink` | 12 | UIAbility（AppLinking 隐式） | ability_context_impl.cpp:1408 → AMC::OpenLink → ams.cpp:17516 → OpenLinkInner → 转普通 StartAbility（原子服务分支走 FreeInstall 间接拉起） |
| `openAtomicService` | 12 | 原子服务 UIAbility | ability_context_impl.cpp:1369 → AMC::OpenAtomicService → ams.cpp:3306 → StartUIAbilityForOptionWrap → 模板 A（FreeInstall 安装完成后回捞） |
| `startAbilityByType` | 11 | 系统 picker 类 UIExtensionAbility | 特殊：ability_context_impl.cpp:1208 → `uiContent->CreateModalUIExtension`（ArkUI 模态通道）→ SCB → AMS::StartUIExtensionAbility → 模板 B。**不走 service router** |
| `requestDialogService` | 9 | dialog 型 ServiceExtensionAbility | ability_context_impl.cpp:900 → AMC::RequestDialogService:945 → ams.cpp:3161 → RequestDialogServiceInner(:3200) → acm.cpp:141 链（模板 B） |
| `startUIServiceExtensionAbility` / `connectUIServiceExtensionAbility` | 14 | UIServiceExtensionAbility | StartExtensionAbilityCommon:1572 → ability_context_impl.cpp:1629 → AMC::StartExtensionAbility(UI_SERVICE)；connect 走 ConnectionManager |
| `startAppServiceExtensionAbility` / `connectAppServiceExtensionAbility` / `stopAppServiceExtensionAbility` | 20 | AppServiceExtensionAbility（三方可用的后台服务扩展） | 同上（APP_SERVICE 类型，AMS 侧免系统应用校验 ams.cpp:4665） |
| `startSelfUIAbilityInCurrentProcess` | 22 | 自身 UIAbility（**本进程内，不新拉进程**） | AMC::StartSelfUIAbility 系（ability_manager_client.cpp:2482+），loadParam.selfPid 复用自身进程 |
| `startSelfUIAbilityInChildProcess` | 26 | 自身 UIAbility（本应用子进程） | 模板 A 变体，进程选项指向自身子进程 |
| `startSelf` | 26 | 自身（激活/重拉） | AMC → AMS::StartSelf（ams.cpp:5467） |

> EmbeddableUIAbilityContext 直接继承 UIAbilityContext，全表适用。
> **注意**：public ≠ 无管控。后台拉起需 `START_ABILITIES_FROM_BACKGROUND`（系统权限）、跨应用非 exported 需 `START_INVISIBLE_ABILITY`、跨用户需权限——AMS 拦截器链统一管控。

#### 4.1.2 其他 Stage Context（宿主本身 public 的）

| API | @since | 宿主 | 链 |
|---|---|---|---|
| `UIExtensionContext.startAbility/startAbilityForResult/openLink/openAtomicService/connectServiceExtensionAbility` | 10/12 | UIExtensionAbility（三方可开发） | `frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp:52/460/406` → AMC 同名方法；**这是三方应用间接启动 ServiceExtension 的正规入口** |
| `AppServiceExtensionContext.startAbility/connectServiceExtensionAbility` | 20 | AppServiceExtensionAbility | appkit app_service_extension_context.cpp:54/26 → AMC/ConnectionManager |
| `ApplicationContext.restartApp` | 12 | 任意应用（重启自身） | js_application_context_utils.cpp:2286→576 → AMS::RestartApp（ams.cpp:16318，仅前台自应用）→ StartAbilityInner |
| `InputMethodExtensionContext.startAbility` | 12 | 输入法应用 | inputmethod 仓实现 → AMC::StartAbility |
| `DistributedExtensionContext.connectServiceExtensionAbility` | 26 | 分布式扩展 | 软总线场景 |
| `WebNativeMessagingExtensionContext.startAbility/startAbilityForResult` | 21/26 | WebNativeMessaging 扩展 | web 仓实现 |
| `InsightIntentContext.startAbility` | 11 | 任意（**仅本应用**意图执行） | js_insight_intent_context.cpp:198→99 → insight_intent_context.cpp:24 → AMC::StartAbilityByInsightIntent:28 → ams.cpp:947 |
| `LiveFormExtensionContext.startAbilityByLiveForm` | 20 | LiveForm 扩展 | form_fwk 仓 live_form_extension_context.cpp:100 → FormMgr → AMS |
| `EnterpriseAdminExtensionContext.startAbilityByAdmin` | 23（权限 ENTERPRISE_START_ABILITIES） | 企业管理扩展 | customization 仓 |
| `UIExtensionContentSession.startAbilityByType` | 11 | UIExtensionAbility | js_ui_extension_content_session.cpp:1053 → CreateModalUIExtension（同 startAbilityByType） |
| FA `featureAbility.startAbility/startAbilityForResult/connectAbility/acquireDataAbilityHelper` | 6/7/7/7 | FA 应用 | `frameworks/js/napi/featureAbility/feature_ability.cpp:133+` → js_napi_common.cpp:1333/96 → AbilityContext::StartAbility（ability_context.cpp:50→58 AMC::StartAbility）/ ConnectAbility(:158→:176 AMC::ConnectAbility)；acquireDataAbilityHelper → DataAbility 通道（AMS::AcquireDataAbility → data_ability_record.cpp:57 → AbilityRecord::LoadAbility ★） |
| FA `particleAbility.startAbility/connectAbility` | 7/7 | FA 粒子线程模型 | particle_ability.cpp:219/207 → 与 featureAbility 完全同构（仅调用方类型校验不同） |

#### 4.1.3 子进程 API（public，仅自身应用）

| API | @since | 实现入口 | 模式 |
|---|---|---|---|
| `childProcessManager.startChildProcess(srcEntry, startMode, callback)` | 11 | `frameworks/js/napi/js_child_process_manager/js_child_process_manager.cpp:366→95` | SELF_FORK（本地 fork）/ APP_SPAWN_FORK（模板 C） |
| `childProcessManager.startArkChildProcess` | 12 | 同上（JS/ARK 类型） | 模板 C |
| `childProcessManager.startNativeChildProcess` | 13 | 同上（NATIVE 类型） | 模板 C |
| （ANI）`@ohos.app.ability.childProcessManager` 静态版 | 23 | frameworks/ets/ani/child_process_manager/src/ets_child_process_manager.cpp:64+ | 同 NAPI |

#### 4.1.4 Native C API（`interfaces/kits/c/ability/ability_runtime/`，public）

| C API | @since | 实现 | 链 |
|---|---|---|---|
| `OH_Ability_CreateNativeChildProcess(libName, cb)` | 12 | frameworks/native/child_process/src/native_child_process.cpp:103 | ChildProcessManager::CreateNativeChildProcessByAppSpawnFork(:121) → 模板 C（`IAppMgr::CreateNativeChildProcess`） |
| `OH_Ability_CreateNativeChildProcessWithConfigs` | 20 | native_child_process.cpp:131 | 同上（隔离模式/自定义进程名） |
| `OH_Ability_StartNativeChildProcess(entry, args, options, pid)` | 13 | native_child_process.cpp:192 | StartChildProcessWithArgs → `IAppMgr::StartChildProcess`（NATIVE_ARGS → nativespawn） |
| `OH_Ability_StartNativeChildProcessWithConfigs` | 20 | native_child_process.cpp:226 | 同上 |
| `OH_AbilityRuntime_StartSelfUIAbility(WithStartOptions/WithPidResult)` | 15/17/21 | frameworks/c/ability_runtime/src/application_context.cpp:402/419/484 | AMC::StartSelfUIAbility 系 |

#### 4.1.5 其他 public 模块

| API | @since | 链 |
|---|---|---|
| `autoFillManager.requestAutoFill / requestAutoSave` | 11/26（26 起 public，11 时部分类型 systemapi） | js_auto_fill_manager.cpp:188→231 → AutoFillManager::RequestAutoFill（interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:59→188）→ `CreateModalUIExtension`（:224/:230）→ SCB → AMS::StartUIExtensionAbility（ams.cpp:4989，**SetAutoFillElementName :4942 把 Want 改写为系统密码管家**）→ 模板 B |
| `abilityManager.restartSelfAtomicService` | 20 | js_ability_manager.cpp:1261→928 → AMC::RestartSelfAtomicService |
| `wantAgent.getWantAgent + trigger`（间接） | 8+ | wantAgent 构造 OPERATION_START_ABILITY → 触发（如通知点击）→ AMS::SendWantSender（ams.cpp:7226）→ PendingWantManager::SendWantSender（pending_want_manager.cpp:301→445）→ StartAbilityWithSpecifyTokenIdInner（isPendingWantCaller=true）→ 模板 A |
| `abilityDelegator.startAbility` | 9（测试框架场景） | js_ability_delegator.cpp:855 → AbilityDelegator::StartAbility（ability_delegator.cpp:312→324 AMC::StartAbility） |

### 4.2 System API（系统应用专用）

#### 4.2.1 UIAbilityContext 上的 systemapi 方法

| API | @since | 权限 | 链 |
|---|---|---|---|
| `startAbilityWithAccount` / `startAbilityForResultWithAccount` | 9 | INTERACT_ACROSS_LOCAL_ACCOUNTS | ability_context_impl.cpp:223/285 → AMC::StartAbility(带 accountId) |
| `startAbilityByCallWithAccount` | 10 | +ABILITY_BACKGROUND_COMMUNICATION | LocalCallContainer |
| `startAbilityAsCaller` | 10 | — | AMC::StartAbilityAsCaller → ams.cpp:2325 → ForOptionInner(isStartAsCaller) |
| `startRecentAbility` | 9 | — | 复用 OnStartAbility（isStartRecent）→ AMC::StartAbility |
| `startServiceExtensionAbility` / `stopServiceExtensionAbility` | 9 | — | StartExtensionAbilityCommon:1572 → ability_context_impl.cpp:1629 → AMC::StartExtensionAbility → AMS::StartExtensionAbility（ams.cpp:4338，CHECK_CALLER_IS_SYSTEM_APP:4665）→ 模板 B |
| `requestModalUIExtension(+WithAccount)` | 11/26 | — | ModalUIExtension 通道（ams.cpp:4475，焦点应用内同进程弹窗或转 SCB StartUIExtensionAbility） |

#### 4.2.2 其他 systemapi

| API | @since | 权限 | 链 / 说明 |
|---|---|---|---|
| `ServiceExtensionContext.*`（startAbility/openLink/WithAccount/startServiceExtensionAbility/connectServiceExtensionAbility/startAbilityByCall/startRecentAbility/requestModalUIExtension/openAtomicService） | 9-18 | 跨账号需权限 | 整宿主 systemapi；service_extension_context.cpp:134/175/123 → AMC 同名（模板 A/B） |
| `UIServiceExtensionContext.*` | 14 | — | 整宿主 systemapi；js_ui_service_extension_context.cpp:498 |
| `UIExtensionContentSession.startAbility/startAbilityForResult` | 10 | — | CHECK_IS_SYSTEM_APP（js_ui_extension_content_session.cpp:243）→ AMC::StartAbilityByUIContentSession（ams.cpp:1016/1055，焦点 token 校验）→ 模板 A |
| `ApplicationContext.preloadUIExtensionAbility` | 12 | PRELOAD_UI_EXTENSION_ABILITY | js_application_context_utils.cpp:2273→922 → AMC::PreloadUIExtensionAbility → ams.cpp:4365 → PreloadUIExtensionAbilityLocked（ueam.cpp:174）→ 模板 B（isPreloadUIExtension） |
| `abilityManager.preloadUIExtensionAbility` | 23 | 同上 | js_ability_manager.cpp:1279→1137 → 同链 |
| `missionManager.moveMissionToFront / moveMissionsToForeground` | 9/10 | MANAGE_MISSIONS | **快照 mission 冷启动可拉起**：mission_manager.cpp:618→AMC::MoveMissionToFront → ams.cpp:7731 → mlm.cpp:282→mar.cpp:207→259 LoadAbility（!isReady_）；SCB 形态走 PendingSessionActivation 回调 |
| `insightIntentDriver.execute` | 11 | —（systemapi 本身限系统侧） | ets_insight_intent_driver.cpp:129 → AMC::ExecuteIntent（ability_manager_client.cpp:2155）→ AMS::ExecuteIntent（ams.cpp:14982）→ ExecuteIntentCommon(:14901) 按 executeMode 分流：UI_ABILITY_FOREGROUND→StartAbilityWithInsightIntent(:15221)；UI_ABILITY_BACKGROUND→StartAbilityByCallWithInsightIntent(:15255，未启动才 StartAbilityByCall:15293)；SERVICE_EXTENSION→StartExtensionAbilityWithInsightIntent(:15248)。**可跨应用**（param.bundleName 指定第三方） |
| `agentManager.connectAgentExtensionAbility` | 24 | CONNECT_AGENT | 见 4.2.3 |
| `agentManager.connectServiceExtensionAbility` | 26 | — | 见 4.2.3 |
| `formHost.*`（全模块） | 9 | — | 见 5.1 |
| `launcherBundleManager.startShortcut(WithReason)` | — | START_SHORTCUT | 桌面快捷方式 → AMS::StartShortcut（ams.cpp:16945，系统应用+权限 → StartUIAbilityForOptionWrap）→ 模板 A |
| `StaticSubscriberExtensionContext.startAbility` | 10 | START_ABILITIES_FROM_BACKGROUND | 静态公共事件后台拉起（CES 回调，见 5.2） |
| `FenceExtensionContext.startAbility` | 14 | — | 围栏子系统 |
| `WorkSchedulerExtensionContext.startServiceExtensionAbility` | 13 | — | 宿主 systemapi；触发方是 workScheduler.startWork（public，见 5.2） |
| `AccessibilityExtensionContext.startAbility` | 12 | — | 无障碍仓实现 |
| `SelectionExtensionContext.startAbility` | 20 | — | inputmethod（selection）仓 |
| `cliManager.execTool` | 26 | EXEC_CLI_TOOL | 见 4.2.4 |
| `businessAbilityRouter.queryBusinessAbilityInfo` | 10 | GET_BUNDLE_INFO_PRIVILEGED | **纯查询**（SRMS SA 404），不直接拉起；调用方拿结果自行 startAbility |
| `distributedMissionManager.*`（全模块） | 9+ | — | continueMission/跨端操作 → DMS（远端/对端拉起，本机 sink 侧落点见 5.2） |
| `@ohos.file.backup` | — | systemapi | backup_sa → ConnectAbility → BackupExtensionAbility（见 5.2） |

#### 4.2.3 Agent 框架（`agent_runtime_framework/`，SA 185）

`@ohos.app.agent.agentManager` 整命名空间 systemapi（since 24/26）。

```
agentManager.connectAgentExtensionAbility(want, agentId, callback)   [systemapi 24, CONNECT_AGENT]
  → ANI: ets_agent_manager.cpp:903（:939 GetAgentCardTypeForConnect 预检；:981 调用）
     /  NAPI: js_agent_manager.cpp:723（绑定 :1335）
  → AgentConnectionManager::ConnectAgentExtensionAbility（agent_connection_manager.cpp:142→519）
  → [IPC] AgentManagerService::ConnectAgentExtensionAbility（agent_manager_service.cpp:345）
      :350 权限+前台校验；:365 AgentCard 解析；:376 LOW_CODE / :385 标准
  → 标准：ConnectStandardAgentExtensionAbility(:410) → RequestStandardAgentConnect(:462)
       → AAFwk::AbilityManagerClient::ConnectAbilityWithExtensionType(AGENT)（:465-467）★
  → LOW_CODE：宿主会话复用（BuildAgentHostKey），仅宿主未连接时 CompleteAgentHostConnect(:932)
       → ConnectAbilityWithExtensionType(AGENT)（:956-958）
  → AMS::ConnectAbilityCommon（ams.cpp:6063；:6091-6097 还回调 AgentMgr 二次确认，双向防绕过）
  → 模板 B → AgentExtensionAbility 进程
```

`agentManager.connectServiceExtensionAbility(context, want, callback)`（systemapi 26）：AgentMgrService:470 → :497-498 `ConnectAbilityWithExtensionType(SERVICE)` → 模板 B。
`AgentUIExtensionAbility`（public 类，AGENT_UI 扩展）：无独立 JS 启动 API，由宿主 ArkUI `UIExtensionComponent` → SCB → AMS::StartUIExtensionAbility → 模板 B（限 5 个）。

#### 4.2.4 CLI 工具框架（`cli_tool_framework/`）

`cliManager.execTool`（systemapi 26）两分支：
- **skill 分支**（拉起应用）：cli_tool_manager_service.cpp:955 `TryDispatchSkillSession` → :1685 `AMC::ExecuteInAppSkillWithTokenId` → AMS::ExecuteInAppSkillWithTokenId（ams.cpp:15422）→ SkillExecuteManager 生成 Want → `StartExtensionAbilityWithSkill`(:15473) / `StartAbilityByCallWithSkill`(:15475) → 模板 A/B。
- **普通分支**（不拉起应用进程）：`ProcessManager::CreateChildProcess`（process_manager.cpp:139）→ fork+execvp claw_sandbox（**独立进程创建路径**，见 3.6）。

### 4.3 逐条链的要点补充

- **StartAbilityForResult** 无独立 AMS 入口：SDK 用 `StartAbility` + `PARAM_RESV_FOR_RESULT` 实现。
- **StartAbilityByCall** 默认拉起到后台（`CallToState::BACKGROUND`，ualm.cpp:1861），带 `PARAM_RESV_CALL_TO_FOREGROUND` 才前台；仅自应用（`JudgeSelfCalled` ams.cpp:10425）。
- **隐式启动**（action/uri 无 bundleName）：`ImplicitStartProcessor`（implicit_start_processor.cpp:777）先解析再回注 StartAbilityInner。
- **免安装/原子服务**：FreeInstall 安装完成后经 `StartUIAbilityByPreInstall`（ams.cpp:17173）或 SCB 缓存回捞进入模板 A——**间接拉起**。
- **AcceptWant 两段式**（SPECIFIED 启动模式）：AMS→AppMS 询问应用"新实例还是复用"（`StartSpecifiedAbility` asch.cpp:452）→ 回调 `OnAcceptWantResponse`（ams.cpp:10892）→ 复用/`StartAbilityLocked`→LoadAbility。
- **StartUIAbilityBySCB / CallUIAbilityBySCB / StartSpecifiedAbilityBySCB**：仅 SCB 可调（ams.cpp:3373/14168），是桌面图标点击、通知点击、SCB 预启动等"用户操作"路径的 AMS 落点。
- **`StartAbilityForPrelaunch`**：仅 RSS_UID（ams.cpp:10583），后台预启动 → CallUIAbilityBySCB → LoadAbility。
- **`StartAbilityWithWait` / `StartUIAbilityWithCallback`**：shell / SA 专用变体，终态同模板 A。

---

## 5. 外部子系统经 AMS/AppMS 拉起应用进程的 API

### 5.1 系统应用 API 触发（宿主应用可感知）

| 子系统 | 应用侧 API（分类） | 链（SA → AMS 方法 → 拉起对象） |
|---|---|---|
| 卡片 FMS（SA 403） | `@ohos.app.form.formHost`（**整模块 systemapi**；ArkUI `FormComponent` 为 public、`postCardAction` public） | FormAmsHelper（form_ams_helper.cpp:45）裸代理 SA 180：卡片数据获取 → `ConnectAbility`（:87）→ FormExtensionAbility 进程；卡片 router 点击 → `StartAbilityWithSpecifyTokenId`（form_event_adapter.cpp:184）；卡片 call → `StartAbilityByCall`（:267）；requestPublish → `StartAbility`（form_publish_adapter.cpp:621）；渲染服务 → `ConnectAbility`（form_render_mgr_inner.cpp:600，FRS 进程） |
| 桌面快捷方式 | `launcherBundleManager.startShortcut`（systemapi + START_SHORTCUT） | → AMS::StartShortcut（ams.cpp:16945）→ 模板 A |
| 输入法 | `@ohos.inputMethod` / `inputMethodSetting`（public；触发：文本框获焦/切默认输入法） | IMF PerUserSession（peruser_session.cpp:1970）→ `ConnectExtensionAbility` → InputMethodExtensionAbility 进程；输入法选择器 → `StartAbility`（input_method_system_ability.cpp:2303） |
| 无障碍 | `@ohos.accessibility`（public；enableAbility） | AAMS AccessibleAbilityConnection:453 → `ConnectAbility` → AccessibilityExtensionAbility 进程 |
| 打印 | `@ohos.print`（public） | PrintServiceHelper（print_service_helper.cpp:92/120，手写 IPC 适配器）→ `ConnectAbilityWithExtensionType(PRINT)` → PrintExtensionAbility 进程；预览走 startUIExtensionAbility |
| 备份恢复 | `@ohos.file.backup`（systemapi） | SvcBackupConnection（svc_backup_connection.cpp:132）→ `ConnectAbility` → BackupExtensionAbility 进程 |
| DLP | `@ohos.dlpPermission`（openDLPFile 等 systemapi） | dlp_ability_adapter.cpp:56 → `ConnectServiceExtensionAbility` → DLP 凭据服务扩展 |
| EDM | `@ohos.enterprise.adminManager`（9-11 systemapi，12+ 部分 public） | enterprise_conn_manager.cpp:205 等 → `ConnectEnterpriseAdminExtensionAbility` / `StartAbility` → 管理扩展 |

### 5.2 系统服务（SA）触发（应用 API 间接 / 用户·硬件事件驱动）

| 子系统 | 触发源（应用侧 API 分类） | SA → AMS 方法（调用点） → 拉起对象 |
|---|---|---|
| 公共事件 CES（SA 3299） | `commonEventManager.publish`（public） | 静态订阅者：StaticSubscriberManager（static_subscriber_manager.cpp:194/245）→ `ConnectAbility`（ability_manager_helper.cpp:69）→ StaticSubscriberExtensionAbility（扩展类型本身 systemapi） |
| 通知 ANS（SA 3203） | `notificationManager.publish`（public，带 WantAgent）→ **用户点击** | 通知面板（SystemUI/SCB 应用侧）触发 WantAgent → AMS::SendWantSender → 模板 A；ANS 侧 RemovalWantAgent（advanced_notification_utils.cpp:1229）；`requestEnableNotification`（public）→ `ConnectServiceExtensionAbility`（notification_dialog.cpp:92）→ systemdialog |
| 提醒 reminder | `reminderAgentManager.publishReminder`（public）→ 到期/点击 | ReminderDataManager::ClickReminder → `StartAbilityOnlyUIAbility`（reminder_data_manager.cpp:1889/:1850）→ 目标 UIAbility |
| 延迟任务（SA 1904） | `workScheduler.startWork`（**public**） | WorkPolicyManager::RealStartWork（work_policy_manager.cpp:522+）→ WorkConnManager（work_conn_manager.cpp:103）→ `ConnectAbility` → WorkSchedulerExtensionAbility 进程；2in1 后台加载 → `StartAbilityByCall`（background_loader_mgr.cpp:256） |
| 数据共享 DataShare | `dataShare.createDataShareHelper`（**9-19 systemapi / 20 起 public**） | DataShareConnection（datashare_connection.cpp:254）→ AmsMgrProxy::Connect（ams_mgr_proxy.cpp:77）→ `ConnectAbilityCommon(DATASHARE)` → DataShareExtensionAbility 提供方进程（断连自动重连） |
| 分布式 DMS（SA 1401） | `distributedMissionManager`（systemapi）；`startAbility` 带远端 deviceId（public） | 对端请求落本机：DistributedSchedService::StartAbility → `AMC::StartAbility`（distributed_sched_service.cpp:4154）→ 模板 A；ContinueMission sink → DSchedContinue::StartAbility（dsched_continue.cpp:1163）；ByCallFromRemote → distributed_sched_adapter.cpp:263 |
| 窗口/ArkUI WMS-SCB（SA 4606） | ArkUI `UIExtensionComponent`（internal 组件）；`@ohos.window.requestModalUIExtension`（public） | ExtensionSessionManager:142 → `StartUIExtensionAbility` → UIExtensionAbility 进程；SCB 自身：SceneSessionManager:4143 → `StartUIAbilityBySCB`；实体菜单 ui_content_impl.cpp:2189+ → `StartAbility` |
| 文件选择器 | `@ohos.file.picker`（public） | picker_n_exporter.cpp:429 → `CreateModalUIExtension` → ModalSystemUiExtension（modal_system_ui_extension.cpp:63）→ `ConnectAbility`（systemdialog）→ 对话框进程（选择后经 startAbility 打开真正 provider 应用） |
| 权限对话框 | `abilityAccessCtrl.requestPermissionsFromUser`（public） | napi_request_permission.cpp:487/537 → AMS::`RequestDialogService` → 调用方 bundle 内 dialog 型扩展 |
| 账号 | `@ohos.account.appAccount` auth（public） | app_account_authenticator_session.cpp:154 → AbilityManagerAdapter（手写 parcel 直发 SA 180，ability_manager_adapter.cpp:325-360）→ `ConnectAbility` → 应用认证 ServiceExtension 进程 |
| NFC | `@ohos.nfc.tag/cardEmulation`（public；硬件触发） | tag_ability_dispatcher.cpp:168+ `StartAbility`（tag 声明应用）；ndef_har_dispatch.cpp:104/140 `StartAbility/StartExtensionAbility`；HCE host_card_emulation_manager.cpp:826 `StartAbilityByCall` |
| 壁纸 | `@ohos.wallpaper`（public） | wallpaper_service.cpp:1292 → `ConnectExtensionAbility` → WallpaperExtAbility |
| 电话 | `@ohos.telephony.call`（public；网络/来电触发） | call_connect_ability.cpp:58 `ConnectAbility`（来电 UI）；STK stk_controller.cpp:549 / 小区广播 cb_start_ability.cpp:45 `StartExtensionAbility` |
| 媒体投播 | `@ohos.multimedia.avsession`（public） | avsession_service.cpp:2973 `StartAbilityByCall`（拉起播控应用） |
| 录屏/相机/录音 | `@ohos.media.screenCapture` 等（public） | screen_capture_server.cpp:2094+ `StartAbility`/`ConnectServiceExtensionAbility`（授权对话框/指示器） |
| 位置 | `@ohos.geoLocationManager`（public） | geo_convert_service.cpp:159 / network_ability.cpp:169 等 `ConnectAbility`（地图/网络定位提供扩展） |
| USB/电源 | `@ohos.usbManager.requestRight`（public）/ 系统事件 | usb_right_manager.cpp:201 / power_ability.cpp:32 `ConnectAbility`/`StartAbility`（系统对话框） |
| MMI 按键 | 硬件按键/触控笔（无应用 API） | ability_launcher.cpp:64/75/115 `StartExtensionAbility/StartAbility`（快捷键配置应用） |
| 智能语音（华为变体） | 语音唤醒 | intell_voice_util.cpp:227 `StartAbility`（语音助手） |
| WebNativeMessaging | `@ohos.web.webview`（public；网页 JS sendNativeMessage） | web_native_messaging_manager.cpp:458/498 `StartAbility`/`StartAbilityForResultAsCaller` → 宿主应用 |
| 广告 | `@ohos.advertising`（public） | ad_load_service.cpp:162/243 `ConnectAbility` → AdsServiceExtensionAbility |
| VPN | `@ohos.net.vpnExtension.startVpnExtensionAbility`（public 11） | netmanager → 扩展拉起 |
| 设备管理 | `@ohos.distributedDeviceManager`（public 部分） | dm_ability_manager.cpp:73 `StartAbility`（PIN/发现 UI） |
| 云同步 | datamgr_service（SA） | connect.cpp:80（rust）`ConnectServiceExtensionAbility` → cloudsync 扩展 |
| 选择框架 | ArkUI 文本选择 | ability_manager_plugin_impl.cpp:76 `ConnectAbility` → SelectionExtensionAbility |
| 安全 | userIAM/userIAM widget（public）；HUKS ukey | widget_context.cpp:510 / hks_ability_manager_service_connection.cpp:75（手写 IPC）`ConnectServiceExtensionAbility/ConnectAbility` |
| BMS 免安装 | `@ohos.bundle` 安装系（安装完成后） | bundle_connect_ability_mgr.cpp:499 `ConnectAbility`（服务中心）；AMS 侧 StartUIAbilityByPreInstall 回捞 |
| 桌面/SCB | 用户点击图标/通知/任务卡片 | SCB → `StartUIAbilityBySCB`（ams.cpp:3335）→ 模板 A |
| RSS 预启动 | 系统内部（仅 RSS_UID） | `StartAbilityForPrelaunch`（ams.cpp:10578）→ CallUIAbilityBySCB → LoadAbility |

> 完整调用点索引见附录 C。共性结论：**Extension 进程拉起几乎全部走 `ConnectAbility/ConnectAbilityCommon/ConnectServiceExtensionAbility`（connect 语义）；UIAbility 拉起集中在 `StartAbility/StartAbilityOnlyUIAbility/StartAbilityWithSpecifyTokenId/StartUIAbilityBySCB`；"静默拉起+调用"集中在 `StartAbilityByCall`。**

### 5.3 手写 IPC 直发 SA 180 的特例（不走标准客户端）

| 子系统 | 实现 | 说明 |
|---|---|---|
| account | `base/account/os_account/.../ability_manager_adapter.cpp:325-360` | 手写 parcel 直发 CONNECT_ABILITY |
| print | `base/print/print_fwk/.../frameworks/ability_manager_adapter/src/ability_manager_adapter.cpp:90-142` | 同上 |
| huks | `base/security/huks/.../hks_ability_manager_service_connection.cpp:75` | AMSConnectAbility 手写 parcel |
| form_fwk / CES / work_scheduler / data_share | `iface_cast<IAbilityManager>` 裸代理 | 不经 AbilityManagerClient 封装 |

---

## 6. 非 appspawn 路径汇总

| 路径 | 进程类型 | 创建者 | 触发 API（分类） | 链 |
|---|---|---|---|---|
| **nwebspawn** | ArkWeb 渲染/GPU 进程 | AppMS（SA 501，唯一 socket 客户端） | ArkUI `Web` 组件 / `@ohos.web.webview`（public） | ArkWeb → `AppMgrClient::StartRenderProcess`（aafwk_app_mgr_client_adapter_impl.cpp:38/49）→ `IAppMgr::StartRenderProcess`（app_mgr_service.cpp:1295）→ `StartRenderProcessImpl`（inner:8645→8689）→ nwebspawn socket |
| **climgr fork** | CLI 工具沙箱进程（claw_sandbox） | climgr SA | `cliManager.execTool/execCmd`（systemapi 26） | process_manager.cpp:139/203 `fork()+execvp` |
| **应用内 SELF_FORK** | 自身 JS/Ark 子进程 | 应用进程自身 | `childProcessManager.startChildProcess(SELF_FORK)`（public 11） | child_process_manager.cpp:97 本地 `fork()`；子进程事后 AttachChildProcess 注册 |
| 系统进程/HDI | 非应用进程 | init / HDI 框架 | —（排除口径） | — |

appspawn socket 的其他直连者（BMS 卸载调试 HAP、webview SA weblib 控制、begetctl 调试）只发**控制/查询消息**，不孵化进程。

---

## 7. 完整性推理：是否梳理全面？

### 7.1 正向核对（接口穷举）

- `IAbilityManager` 60+ 启动类方法全部归类（附录 A）。其中确认**纯转发/不拉起**的：`StartRemoteAbility`（内部路由）、`StartRemoteAbilityByCall`、`ConnectRemoteAbility`、`ContinueMission`×2、`StartContinuation`、`ContinueAbility`、`NotifyCompleteContinuation`、`StartSelf`、`AcquireShareData`、`MoveMissionsToForeground/Background`（纯 WMS）、`RequestModalUIExtension`（同进程弹窗或转 SCB）、自启动变体（强制复用自身进程）。
- `IAppMgr/IAmsMgr` 15+ 创建进程方法全部归类（3.5 节表 + 附录 B）。
- `IAgentManager`、`IServiceRouterMgr` IDL 全部归类。
- d.ts 全目录关键词扫描（start*/open*/connect*/execute*/request* + @systemapi 交叉），未发现漏网的启动类导出（`startAbilityByState`/`startAppService`（独立方法）/`startParticleAbility`/`startAbilityForAccount` 等传闻名称在当前 d.ts **不存在**，实际入口已在正文纠正）。

### 7.2 反向核对（出口穷举）

- `AppSpawnClient::StartProcess` 7 个调用点 + `StartImageProcess` 全部覆盖（附录 B）；每条向上回溯到 IPC 入口，与 7.1 对账**无孤例**。
- `StartChildProcessImpl`（appspawn/nativespawn）、`StartRenderProcessImpl`（nwebspawn）、climgr fork、SELF_FORK 均覆盖。
- spawn socket 客户端（`RemoteClientManager` 5 个）与外部直连者（BMS/webview SA/begetctl，仅控制消息）已穷举。

### 7.3 横向核对（全树扫描）

- 全源码树 grep `AbilityManagerClient`/`ExtensionManagerClient`/`IAbilityManager`/SA 180 直发 → 50+ 子系统；有实际拉起行为的 30+ 全部列入第 5 章/附录 C。
- 明确核查过**不拉起**的：锁屏（screenlock 仅查询）、CastEngine（仅查询）、graphic RS（仅杀进程）、resourceschedule bg_task/ressched（仅查询/注册）、dsoftbus LNN（仅查询，BR 保活除外）、BMS helper（仅查询）。

### 7.4 已识别的盲区与风险（诚实声明）

| # | 盲区 | 影响 | 缓解 |
|---|---|---|---|
| 1 | **闭源/私有服务**（Push Kit 等推送、厂商私有 SA）不在源码树 | 可能有树外的拉起方 | 推送等最终仍须经 AMS/AppMS 入口（本机拉起必经汇聚点），挂点若选在 AppMS 侧则天然覆盖；仅"哪些 API 触发"不可见 |
| 2 | **远端拉起**发生在对端设备 | 本机 PID 列表不含远端进程 | 单机口径声明；如需跨端需 DMS 配合 |
| 3 | **SELF_FORK / claw_sandbox / 应用裸 fork** 不经 AppMS | 挂点选 AppMS 时观测不到 | 子进程事后 Attach 有记录可补；claw_sandbox 可在 climgr 加挂点；应用裸 fork 本质不可观测（非 API） |
| 4 | **用户操作型触发**（桌面图标、通知点击、NFC、按键）无应用 API 发起时刻 | "API→链" 的发起方是 SCB/系统 SA | 归因时应记录"触发来源"维度，不能只记 callingUid |
| 5 | **SCB 回调换上下文** | StartUIAbilityBySCB 链上原始调用方信息丢失 | taskId 必须放 Want/sessionInfo 业务数据（见 8.2） |
| 6 | **镜像 fork worker / 测试空进程 / 常驻进程**等非常规拉起 | 容易被遗漏 | 已在 3.5 列出，挂点在 StartProcess 处天然覆盖 |
| 7 | **d.ts 快照时效**（dynamic/static 双轨、华为变体 API） | 个别 API 的 public/system 分类随版本演进（如 dataShare 9-19 systemapi→20 public；autoFill 26 转 public） | 分类以当前快照为准，落地前需对目标版本复核 |
| 8 | 行号会随代码演进漂移 | 文档时效 | 以函数名为锚点复核 |

### 7.5 结论

在"**本机、经系统孵化器创建的应用进程**"口径下，开源代码范围内的拉起路径可以声明**枚举完备**：
- 每条 API 链的终点都收敛到 3.2 节的主干（AppScheduler::LoadAbility → AppMgrServiceInner::LoadAbility → StartProcess → spawn）或 3.5 节的旁路；
- 每个进程诞生点（附录 B）向上都有明确 API/触发源映射。
残余不确定性集中在 7.4 的 8 项盲区，其中对特性影响最大的是 **SELF_FORK 不可观测**与**用户操作型触发的归因**。

---

## 8. 对 Task-ID → PID 列表特性的设计建议

### 8.1 挂点选择

| 方案 | 挂点 | 覆盖度 | 代价 |
|---|---|---|---|
| A（推荐） | `AppMgrServiceInner::StartProcess`(:6059) + `StartChildProcessImpl`(:10972) + `StartRenderProcessImpl`(:8689) | 覆盖全部 spawn 家族（含旁路、常驻、测试、镜像、perf） | AppMS 内部改造，需在 StartMsg/记录中带 taskId |
| B | `AppMgrServiceInner::LoadAbility`(:2075) 入口 | 额外覆盖"复用已有进程"场景（无新 PID 但有关联语义） | 同上；建议 A+B 都挂：LoadAbility 记录关联、StartProcess 记录诞生 |
| C | appspawn 消息层 | 理论最全 | 跨仓库（appspawn 仓），且丢失业务上下文 |
| D | AMS 汇聚点（ar.cpp:310 / acm.cpp:1310） | 只见组件加载，进程诞生/复用判断在下游 | 不推荐单独使用 |

**补充挂点**（按特性口径决定是否纳入）：climgr `ProcessManager::CreateChildProcess`（claw 进程）；`AttachChildProcess`（SELF_FORK 事后补录）。

### 8.2 taskId 透传路径设计（核心难点）

现成先例：**多实例 instanceKey 的全链透传**（Want 私有参数 → AbilityRequest → `loadParam.instanceKey` → `AppRunningRecord::SetInstanceKey`）。taskId 建议完全复刻该链：

1. **应用侧**：启动类 API 的 Want 中携带私有参数（如 `ohos.param.agent.taskId`）；AMS 在 AbilityRequest/LoadParam 构造时透传（multi_instance_utils.cpp 同层）。
2. **SCB 链路**：`NotifySCBToStartUIAbility` → sessionInfo 携带 taskId（SCB 回调 `StartUIAbilityBySCB` 时 IPC 上下文已变为 SCB，**必须**依赖 sessionInfo/Want 携带）。
3. **AppMS 侧**：`LoadParam`（utils/server/startup/include/param.h:26）增加字段 → `AppSpawnStartMsg`（可选，若需 appspawn 感知）→ `AppRunningRecord` 存储 → `OnProcessCreated`/`StartProcess` 成功回调处写入 taskId→PID 映射。
4. **外部 SA 触发链**（Form/WorkScheduler/DataShare/通知点击等代应用拉起）：第一期建议**不透传** taskId，改用"目标应用 + callingUid 归因"降级方案；后续按需逐子系统在 Want 中补透传。
5. **安全**：taskId 绑定 `callingUid + taskId` 双键校验（仅任务发起应用可挂载/查询自己的 taskId），防止伪造他人任务；AgentManagerService 侧 StartTask/FinishTask/AbortTask 已有系统应用 + 权限校验骨架，可作为任务注册面。

### 8.3 归因数据模型建议（每次拉起记录）

```
{ taskId, pid, hostPid(子进程), bundleName, uid, processName, instanceKey,
  spawnType(main/child/render/image/test/resident/perf/claw),
  triggerType(api/sa/user_event/preload), callingUid, callingPid, timestamp }
```

- **复用场景**：进程未新建时也记录一条 `spawnType=reuse`（关联已有 PID）。
- **生命周期**：StartTask → 持续收集 → FinishTask 封板 / AbortTask 可选联动清理（kill 或仅解绑）。
- **DFX**：按 `hisysevent.yaml` 合约新增事件（需走事件申报流程，不可破坏既有事件）。

### 8.4 必须在设计中显式回答的问题

1. 任务发起应用与被拉起应用不同（跨应用 startAbility）时，PID 列表归属哪个 taskId？——建议按"发起方 taskId"归因（记录 callingUid）。
2. 进程被多个任务复用时，一个 PID 可属于多个 taskId 列表（多对多关系，不能简单去重）。
3. 进程死亡后 PID 会被系统复用，映射需记录死亡时间或以 AppRunningRecord 生命周期为准。
4. 子进程/渲染进程是否计入任务 PID 列表（建议计入，spawnType 区分）。
5. LOW_CODE Agent 共享宿主：多个 agentId 复用同一宿主进程，任务归因需按宿主会话记录（见 4.2.3）。

---

## 附录 A：AMS 拉起相关 IPC 入口索引（IAbilityManager）

**UIAbility 主家族**：`StartAbility`×4（ams.cpp:731/815/2025/2285）、`StartAbilityWithSpecifyTokenId`(:883，FOUNDATION_UID)、`StartAbilityByInsightIntent`(:947)、`StartAbilityByOEExt`(:984)、`StartAbilityByUIContentSession`×2(:1016/1055)、`StartAbilityOnlyUIAbility`(接口默认)、`StartAbilityAsCaller`×2(:2325/2343)、`StartAbilityForResultAsCaller`×2(:2375)、`StartAbilityForAppCloneSelector`(:1804)、`StartAbilityWithInsightIntent`(:15221)、`StartAbilityWithWait`(:18305，shell)、`StartUIAbilityWithCallback`(:18329，SA)、`StartSelfUIAbility` 系(:17982+，2in1 自应用)、`StartSelf`(:5467，不拉起)、`StartShortcut`(:16945，system+权限)、`OpenLink`(:17516)、`OpenAtomicService`(:3306)、`RestartApp`(:16318)、`StartSandboxCloneAbility`(:19336，CLI token)、`LaunchGameCustomized`(:14523，Game SA)、`MoveMissionToFront`×2(:7731/7757，system，冷启动拉起)、`DoAbilityForeground`(:11953，delegator)、`StartUIAbilitiesInSplitWindowMode`(:2905)/`StartUIAbilities`(:2937)、`PreStartMission`(:17027)、`StartAbilityForPrelaunch`(:10578，RSS_UID)。

**Extension/UIExtension**：`StartExtensionAbility`(:4338，系统应用校验:4665；含旧 StartServiceExtensionAbility 语义)、`StartUIExtensionAbility`(:4989，SCB)、`PreloadUIExtensionAbility`(:4365)、`RequestDialogService`(:3161)。

**Connect**：`ConnectAbility`(:6057)、`ConnectAbilityCommon`(:6063，SA 可带 indirectCallerInfo)、`ConnectUIExtensionAbility`(:6260，权限)。

**ByCall**：`StartAbilityByCall`(:10381)、`StartAbilityByCallWithErrMsg`(:10390)。

**Intent/Skill**：`ExecuteIntent`(:14982)、`ExecuteIntentByFunctionCall`(:14860)、`ExecuteIntentForDistributed`(:14786，DMS_UID)、`QueryEntityInfo`(:15136)、`ExecuteInAppSkill`(:15364)、`ExecuteInAppSkillWithTokenId`(:15422)。

**SCB 专用**：`StartUIAbilityBySCB`(:3335)、`CallUIAbilityBySCB`(:14134)、`StartSpecifiedAbilityBySCB`(:14164)。

**WantSender**：`SendWantSender`(:7226)、`SendLocalWantSender`(:7251，权限)。

**DataAbility(FA)**：`AcquireDataAbility`(:7880)。

**免安装回捞**：`StartUIAbilityByPreInstall`(:17173)、`FreeInstallAbilityFromRemote`(:12325，DMS_UID)。

**AppMS 直连旁路**：`StartUserTest`(:11715)、`PreloadApplication`(:18902)、`StartUser`(:10712，间接)。

**纯转发/不拉起**：`StartRemoteAbility`(ams.cpp:5803)、`StartRemoteAbilityByCall`(:10346)、`ConnectRemoteAbility`(:6636)、`ContinueMission`(:6706/6722)、`StartContinuation`(:6768)、`ContinueAbility`(:6739)、`NotifyCompleteContinuation`(:6807)、`AcquireShareData`(:14057)、`RequestModalUIExtension(WithAccount)`(:4475/4481)、`MoveMissionsToForeground/Background`(:7784/7805)。

## 附录 B：进程诞生点索引（spawn 出口）

| 诞生点 | 位置 | 消息/服务 | 上游主入口 |
|---|---|---|---|
| 主进程（普通/CJ/Hybrid） | inner:6152/6142/6148 | MSG_APP_SPAWN → appspawn/cjappspawn/hybridspawn | LoadAbility 全家族 |
| 子进程（JS/ARK/NATIVE_ARGS） | inner:10972（类型分流 :10925-10927） | +APP_FLAGS_CHILDPROCESS → appspawn/nativespawn | `IAppMgr::StartChildProcess`(:893)/`CreateNativeChildProcess`(:1079/:1093) |
| 渲染进程 | inner:8689 | renderParam → nwebspawn | `IAppMgr::StartRenderProcess`(:531) |
| 镜像模板 / worker | StartImageProcess（app_spawn_client.cpp:715）/ inner:1541 | MSG_SPAWN_IMAGE_PROCESS / MSG_SPAWN_WORKER_PROCESS | `MakeImage`(:100)/TryToUseImageInfo(:1491) |
| perf/debug 进程 | inner:5588 | MSG_SPAWN_NATIVE_PROCESS | StartNativeProcessForDebugger(:729)/StartPerfProcess |
| 空测试进程 | inner:7378（StartEmptyProcess） | MSG_APP_SPAWN | `StartUserTestProcess`(:472) |
| 常驻空进程 | inner:6988（StartEmptyResidentProcess） | MSG_APP_SPAWN | `StartupResidentProcess`(:404) |
| CLI 沙箱进程 | climgr process_manager.cpp:139/203 | fork+execvp claw_sandbox | cliManager.execTool/execCmd |
| 应用内 SELF_FORK | child_process_manager.cpp:97 | 本地 fork() | startChildProcess(SELF_FORK) |

## 附录 C：外部子系统 AMS 调用点索引（节选，均已在正文第 5 章给出链）

form_fwk（form_ams_helper.cpp:45/87/253/276；form_event_adapter.cpp:184/267/310/323；form_publish_adapter.cpp:621；form_render_mgr_inner.cpp:600）｜work_scheduler（work_conn_manager.cpp:103；background_loader_mgr.cpp:256）｜data_share（ams_mgr_proxy.cpp:77）｜CES（ability_manager_helper.cpp:69；static_subscriber_manager.cpp:194/245）｜ANS（advanced_notification_utils.cpp:1229；notification_dialog.cpp:92；reminder_data_manager.cpp:1850/1889）｜IMF（peruser_session.cpp:1970/2885；input_method_system_ability.cpp:2303）｜AAMS（accessible_ability_connection.cpp:453；extension_ability_manager.cpp:102）｜dmsfwk（distributed_sched_service.cpp:4154；dsched_continue.cpp:1163；distributed_sched_adapter.cpp:95/263）｜WMS/SCB（extension_session_manager.cpp:142/192/225；scene_session_manager.cpp:4143；modal_system_ui_extension.cpp:63）｜account（app_account_authenticator_session.cpp:154；ability_manager_adapter.cpp:325；service_extension_connect.cpp:458）｜print（print_service_helper.cpp:92/120）｜NFC（tag_ability_dispatcher.cpp:168/191/226；ndef_har_dispatch.cpp:104/140/211/289；host_card_emulation_manager.cpp:826/865）｜wallpaper（wallpaper_service.cpp:1292）｜USB/power（usb_right_manager.cpp:201；power_ability.cpp:32/47）｜telephony（call_connect_ability.cpp:58；stk_controller.cpp:549；cb_start_ability.cpp:45/50）｜avsession（avsession_service.cpp:2973/3016）｜screen_capture（screen_capture_server.cpp:2094/2170/3702/3727）｜camera/audio/medialibrary（camera_dialog_manager.cpp:70；capturer_in_server.cpp；medialibrary_subscriber.cpp:1414；media_facard_operations.cpp:127/158）｜geo（geo_convert_service.cpp:159；poi_info_manager.cpp:154；network_ability.cpp:169；agnss_ni_manager.cpp:317）｜MMI（ability_launcher.cpp:64/75/115；stylus_key_handler.cpp:131；authorization_dialog.cpp:71）｜intelligent_voice（intell_voice_util.cpp:227）｜WebNativeMessaging（web_native_messaging_manager.cpp:458/498）｜advertising（ad_load_service.cpp:162/243；connect_ads_stub.cpp:367）｜object_editor（object_editor_connection.cpp）｜device_manager（dm_ability_manager.cpp:73）｜dsoftbus（br_proxy_adapter.cpp:65）｜datamgr_service（connect.cpp:80）｜EDM（enterprise_conn_manager.cpp:205 等）｜selectionfwk（ability_manager_plugin_impl.cpp:76）｜backup（svc_backup_connection.cpp:132）｜access_token（napi_request_permission.cpp:487/537）｜useriam（widget_context.cpp:510）｜huks（hks_ability_manager_service_connection.cpp:75）｜DLP（dlp_ability_adapter.cpp:56）｜BMS（bundle_connect_ability_mgr.cpp:499）｜file_api（open.cpp:239 OpenFile，非拉起）｜fusion/mechbody/updateservice 等（华为变体，同类对话框/扩展连接模式）

---

*本文档由代码静态分析产出（只读研究），所有链路均经源文件逐行核实；行号对应工作区快照，复核时请以函数名为锚点。*
