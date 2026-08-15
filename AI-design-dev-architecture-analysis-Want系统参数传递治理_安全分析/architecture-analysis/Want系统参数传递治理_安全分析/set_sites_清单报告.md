# Want SET 点清单报告

范围：ability_runtime 服务侧 `services/`+`interfaces/`（一~五节，非测试/mock）+ app 侧框架 `frameworks/` Context（六~七节），当前 master 快照。
统计口径：`Want::SetParam`/`WantParams::SetParam` 写入 Want 的调用点（按调用语句计，同一语句多次调用各计一次）。
系统参数白名单：一~四节所列 81 个 key（`Want::PARAM_RESV_*` 预留 key + 调用者身份/用户/进程控制/窗口/安全调试等 9 类）；其余 key 见第五节。
列：设置处(file:line / 所在函数) | 设置内容(key = value) | 设置前重置(同 key 且同一函数体内先 RemoveParam/Remove) | IPC 派生(值或函数体引用 `IPCSkeleton::GetCalling*`)

## 一、统计摘要

| 指标 | 数值 |
|---|---|
| SET 点总数（服务侧，含白名单外） | 491 |
| ├ 系统参数白名单（四节） | 228 |
| ├ 白名单外（五节） | 263 |
| └ app 侧框架（六节） | 103 |
| 系统参数归一化 key 数 | 81 |
| 白名单外归一化 key 数 | 167 |
| 涉及文件数（服务侧） | 48 |
| 设置前重置(Y，白名单内) | 29 |
| IPC 派生(✅，白名单内) | 53 |
| 常量值(布尔/0 等，白名单内) | 35 |

> 注：一~四节为系统参数白名单清单；第五节补充白名单外 SET 点；六~七节为 app 侧框架补充。

## 二、按类别分布

| 类别 | key 数 | SET 处数 |
|---|---|---|
| 调用者身份 | 18 | 80 |
| 用户与克隆 | 9 | 39 |
| 调度与进程控制 | 18 | 32 |
| 显示与窗口 | 13 | 29 |
| 安全与调试 | 7 | 17 |
| 业务字符串 | 7 | 12 |
| 会话与回调 | 5 | 11 |
| 恢复与保活 | 3 | 6 |
| 时间戳 | 1 | 2 |

### 各类别 key 明细

- **调用者身份**（80 处）：`CLI_CALLER_BUNDLE_NAME`(2)、`CLI_CALLER_TOKEN_ID`(2)、`DMS_CALLING_UID`(2)、`PARAM_FREEINSTALL_UID`(1)、`PARAM_RESV_CALLER_ABILITY_NAME`(8)、`PARAM_RESV_CALLER_APP_CLONE_INDEX`(4)、`PARAM_RESV_CALLER_APP_ID`(2)、`PARAM_RESV_CALLER_APP_IDENTIFIER`(2)、`PARAM_RESV_CALLER_BUNDLE_NAME`(12)、`PARAM_RESV_CALLER_NATIVE_NAME`(5)、`PARAM_RESV_CALLER_PID`(9)、`PARAM_RESV_CALLER_TOKEN`(11)、`PARAM_RESV_CALLER_UID`(12)、`SPECIFY_TOKEN_ID`(4)、`TARGET_UID_KEY`(1)、`TOKEN_ID`(1)、`UID`(1)、`callerUid`(1)
- **用户与克隆**（39 处）：`ABILITY_OWNER_USERID`(1)、`APP_INDEX`(1)、`APP_INSTANCE_KEY`(5)、`INSIGHT_INTENT_PARAM_USER_ID`(1)、`PARAM_APP_CLONE_INDEX_KEY`(22)、`UI_EXTENSION_TARGET_USER_ID`(1)、`USER_ID`(2)、`appIndex`(1)、`userId`(5)
- **调度与进程控制**（32 处）：`FROM_REMOTE_KEY`(2)、`GAME_PRELAUNCH`(2)、`HIDDEN_START_AUTOSTARTUP`(1)、`IS_CALLING_FROM_DMS`(1)、`IS_CALL_BY_SCB`(10)、`IS_EMBEDDABLE_SERVICE`(1)、`IS_HOOK`(1)、`IS_PRELOAD_UIEXTENSION_ABILITY`(1)、`IS_SHELL_CALL`(2)、`KEY_IS_APP_RUNNING`(1)、`KEY_SKIP_ABILITY_STAGE_LIFECYCLE`(1)、`KEY_SPECIFIED_FLAG`(2)、`MULTI_THREAD`(1)、`NEED_STARTINGWINDOW`(1)、`PARAM_SPECIFIED_PROCESS_FLAG`(2)、`SPECIFIED_PROCESS_CALLER_PROCESS`(1)、`WANT_PARAMS_APP_RESTART_FLAG`(1)、`ohos.ability.params.isSkipErmsFromSCB`(1)
- **显示与窗口**（29 处）：`PARAM_RESV_DISPLAY_ID`(11)、`PARAM_RESV_MAX_WINDOW_HEIGHT`(1)、`PARAM_RESV_MAX_WINDOW_WIDTH`(1)、`PARAM_RESV_MIN_WINDOW_HEIGHT`(1)、`PARAM_RESV_MIN_WINDOW_WIDTH`(1)、`PARAM_RESV_WINDOW_FOCUSED`(2)、`PARAM_RESV_WINDOW_HEIGHT`(1)、`PARAM_RESV_WINDOW_LEFT`(1)、`PARAM_RESV_WINDOW_MODE`(6)、`PARAM_RESV_WINDOW_TOP`(1)、`PARAM_RESV_WINDOW_WIDTH`(1)、`PARAM_RESV_WITH_ANIMATION`(1)、`REUSING_WINDOW`(1)
- **安全与调试**（17 处）：`DEBUG_APP`(6)、`DLP_INDEX`(3)、`DLP_PARAMS_SANDBOX`(1)、`DLP_PARAMS_SECURITY_FLAG`(3)、`IS_WEB_SANDBOX_CLONE`(1)、`PARAM_ASSERT_FAULT_SESSION_ID`(1)、`SANDBOX_CLONE_INDEX`(2)
- **业务字符串**（12 处）：`CollaboratorRemoveCallerName`(1)、`appId`(1)、`bundleName`(3)、`callerBundleName`(1)、`string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)`(1)、`string(PARAM_SEND_RESULT_CALLER_TOKENID)`(1)、`uid`(4)
- **会话与回调**（11 处）：`PARAM_RESV_ABILITY_INFO_CALLBACK`(1)、`PARAM_RESV_APP_REQUEST_ID`(3)、`PARAM_RESV_REQUEST_PROC_CODE`(2)、`PARAM_RESV_REQUEST_TOKEN_CODE`(2)、`PARAM_RESV_SCB_REQUEST_ID`(3)
- **恢复与保活**（6 处）：`PARAM_ABILITY_RECOVERY_RESTART`(4)、`ohos.app.logout_recovery`(1)、`ohos.app.recovery`(1)
- **时间戳**（2 处）：`PARAM_RESV_START_TIME`(2)

## 三、全量 SET 清单（228 处，按文件/行号排序）

| # | 设置处(file:line / 函数) | key | value | 设置前重置 | IPC 派生 |
|---|---|---|---|---|---|
| 1 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:127` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_REQUEST_PROC_CODE` | want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0) | ❌否 | — |
| 2 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:129` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_REQUEST_TOKEN_CODE` | want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0) | ❌否 | — |
| 3 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:131` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_ABILITY_INFO_CALLBACK` | tempCallBack | ❌否 | — |
| 4 | `services/abilitymgr/include/collaborator/collaborator_util.h:56` `HandleCallerIfNeed` | `"CollaboratorRemoveCallerName"` | callerBundleName | ❌否 | — |
| 5 | `services/abilitymgr/include/utils/ability_util.h:261` `WantSetParameterWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` | windowMode | ❌否 | — |
| 6 | `services/abilitymgr/include/utils/ability_util.h:267` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` | windowMode | ❌否 | — |
| 7 | `services/abilitymgr/include/utils/ability_util.h:274` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` | windowMode | ❌否 | ✅ |
| 8 | `services/abilitymgr/include/utils/ability_util.h:277` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` | windowMode | ❌否 | ✅ |
| 9 | `services/abilitymgr/src/ability_auto_startup_service.cpp:667` `AbilityAutoStartupService::HiddenStartAutoStartupApp` | `HIDDEN_START_AUTOSTARTUP` | true | ❌否 | — |
| 10 | `services/abilitymgr/src/ability_connect_manager.cpp:887` `AbilityConnectManager::ProcessPreload` | `"uid"` | uid | ❌否 | — |
| 11 | `services/abilitymgr/src/ability_connect_manager.cpp:1409` `AbilityConnectManager::OnStartSpecifiedProcessResponse` | `PARAM_SPECIFIED_PROCESS_FLAG` | flag | ❌否 | — |
| 12 | `services/abilitymgr/src/ability_connect_manager.cpp:2577` `AbilityConnectManager::RestartAbility` | `"ohos.app.recovery"` | true | ❌否 | — |
| 13 | `services/abilitymgr/src/ability_connect_manager.cpp:2580` `AbilityConnectManager::RestartAbility` | `Want::PARAM_RESV_DISPLAY_ID` | static_cast<int32_t>(displayId) | ❌否 | — |
| 14 | `services/abilitymgr/src/ability_connect_manager.cpp:2590` `AbilityConnectManager::RestartAbility` | `WANT_PARAMS_APP_RESTART_FLAG` | true | ❌否 | — |
| 15 | `services/abilitymgr/src/ability_manager_client.cpp:1805` `AbilityManagerClient::HandleDlpApp` | `DLP_PARAMS_SANDBOX` | sandboxFlag | ❌否 | — |
| 16 | `services/abilitymgr/src/ability_manager_service.cpp:1607` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::CLI_CALLER_BUNDLE_NAME` | param.sandboxCloneParams->callerBundleName | ❌否 | — |
| 17 | `services/abilitymgr/src/ability_manager_service.cpp:1609` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::CLI_CALLER_TOKEN_ID` | std::to_string(param.sandboxCloneParams->callerTokenId) | ❌否 | — |
| 18 | `services/abilitymgr/src/ability_manager_service.cpp:1752` `AbilityManagerService::StartAbilityInner` | `SPECIFY_TOKEN_ID` | static_cast<int32_t>(param.specifyTokenId) | ✅是 | ✅ |
| 19 | `services/abilitymgr/src/ability_manager_service.cpp:1759` `AbilityManagerService::StartAbilityInner` | `ServerConstant::IS_CALL_BY_SCB` | param.isBySCB | ❌否 | ✅ |
| 20 | `services/abilitymgr/src/ability_manager_service.cpp:1771` `AbilityManagerService::StartAbilityInner` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | std::string("") | ❌否 | — |
| 21 | `services/abilitymgr/src/ability_manager_service.cpp:1772` `AbilityManagerService::StartAbilityInner` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | std::string("") | ❌否 | — |
| 22 | `services/abilitymgr/src/ability_manager_service.cpp:1776` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::GAME_PRELAUNCH` | param.isGamePrelaunch | ❌否 | — |
| 23 | `services/abilitymgr/src/ability_manager_service.cpp:1962` `AbilityManagerService::PreprocessRequestParams` | `SPECIFY_TOKEN_ID` | static_cast<int32_t>(param.specifyTokenId) | ✅是 | — |
| 24 | `services/abilitymgr/src/ability_manager_service.cpp:1976` `AbilityManagerService::ExecuteAbilityStart` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | — |
| 25 | `services/abilitymgr/src/ability_manager_service.cpp:1977` `AbilityManagerService::ExecuteAbilityStart` | `AbilityRuntime::GlobalConstant::GAME_PRELAUNCH` | isGamePrelaunch | ❌否 | — |
| 26 | `services/abilitymgr/src/ability_manager_service.cpp:2285` `AbilityManagerService::StartAbilityDetails` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | ✅ |
| 27 | `services/abilitymgr/src/ability_manager_service.cpp:2552` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_WINDOW_FOCUSED` | windowFocused | ❌否 | — |
| 28 | `services/abilitymgr/src/ability_manager_service.cpp:2557` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` | DisplayUtil::GetDefaultDisplayId() | ❌否 | — |
| 29 | `services/abilitymgr/src/ability_manager_service.cpp:2560` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` | startOptions.GetDisplayID() | ❌否 | — |
| 30 | `services/abilitymgr/src/ability_manager_service.cpp:2694` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` | DisplayUtil::GetDefaultDisplayId() | ❌否 | — |
| 31 | `services/abilitymgr/src/ability_manager_service.cpp:2697` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` | startOptions.GetDisplayID() | ❌否 | — |
| 32 | `services/abilitymgr/src/ability_manager_service.cpp:2712` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_WINDOW_FOCUSED` | windowfocused | ✅是 | — |
| 33 | `services/abilitymgr/src/ability_manager_service.cpp:2751` `AbilityManagerService::StartAbilityForOptionInner` | `SPECIFY_TOKEN_ID` | static_cast<int32_t>(specifyTokenId) | ✅是 | — |
| 34 | `services/abilitymgr/src/ability_manager_service.cpp:2759` `AbilityManagerService::StartAbilityForOptionInner` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | ✅ |
| 35 | `services/abilitymgr/src/ability_manager_service.cpp:2784` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` | DisplayUtil::GetDefaultDisplayId() | ❌否 | ✅ |
| 36 | `services/abilitymgr/src/ability_manager_service.cpp:2934` `AbilityManagerService::StartUIAbilitiesInSplitWindowMode` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | — |
| 37 | `services/abilitymgr/src/ability_manager_service.cpp:2980` `AbilityManagerService::StartUIAbilities` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | — |
| 38 | `services/abilitymgr/src/ability_manager_service.cpp:4419` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | ✅ |
| 39 | `services/abilitymgr/src/ability_manager_service.cpp:4421` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` | callerAppIndex | ❌否 | ✅ |
| 40 | `services/abilitymgr/src/ability_manager_service.cpp:4437` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `IS_PRELOAD_UIEXTENSION_ABILITY` | true | ❌否 | ✅ |
| 41 | `services/abilitymgr/src/ability_manager_service.cpp:4598` `AbilityManagerService::RequestModalUIExtensionWithAccountInner` | `UI_EXTENSION_TARGET_USER_ID` | accountId | ❌否 | — |
| 42 | `services/abilitymgr/src/ability_manager_service.cpp:6721` `AbilityManagerService::ContinueMission` | `DMS_CALLING_UID` | AAFwk::Integer::Box(callingUid) | ❌否 | ✅ |
| 43 | `services/abilitymgr/src/ability_manager_service.cpp:6738` `AbilityManagerService::ContinueMission` | `DMS_CALLING_UID` | AAFwk::Integer::Box(callingUid) | ❌否 | ✅ |
| 44 | `services/abilitymgr/src/ability_manager_service.cpp:8940` `AbilityManagerService::StartHighestPriorityAbility` | `NEED_STARTINGWINDOW` | false | ❌否 | — |
| 45 | `services/abilitymgr/src/ability_manager_service.cpp:8941` `AbilityManagerService::StartHighestPriorityAbility` | `Want::PARAM_RESV_DISPLAY_ID` | static_cast<int32_t>(displayId) | ❌否 | — |
| 46 | `services/abilitymgr/src/ability_manager_service.cpp:8946` `AbilityManagerService::StartHighestPriorityAbility` | `"ohos.app.logout_recovery"` | true | ❌否 | — |
| 47 | `services/abilitymgr/src/ability_manager_service.cpp:8962` `AbilityManagerService::GenerateAbilityRequest` | `AbilityRuntime::ServerConstant::DLP_INDEX` | abilityRecord->GetAppIndex() | ❌否 | — |
| 48 | `services/abilitymgr/src/ability_manager_service.cpp:8963` `AbilityManagerService::GenerateAbilityRequest` | `DLP_PARAMS_SECURITY_FLAG` | abilityRecord->GetSecurityFlag() | ❌否 | — |
| 49 | `services/abilitymgr/src/ability_manager_service.cpp:8970` `AbilityManagerService::GenerateAbilityRequest` | `AbilityConfig::DEBUG_APP` | abilityRecord->IsDebugApp() | ❌否 | — |
| 50 | `services/abilitymgr/src/ability_manager_service.cpp:8993` `AbilityManagerService::GenerateAbilityRequest` | `Want::PARAM_RESV_CALLER_TOKEN` | static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()) | ❌否 | ✅ |
| 51 | `services/abilitymgr/src/ability_manager_service.cpp:8994` `AbilityManagerService::GenerateAbilityRequest` | `Want::PARAM_RESV_CALLER_UID` | IPCSkeleton::GetCallingUid() | ❌否 | ✅ |
| 52 | `services/abilitymgr/src/ability_manager_service.cpp:9016` `AbilityManagerService::GenerateAbilityRequest` | `KEY_SKIP_ABILITY_STAGE_LIFECYCLE` | abilityInfo->skipAbilityStageLifecycle | ❌否 | ✅ |
| 53 | `services/abilitymgr/src/ability_manager_service.cpp:9095` `AbilityManagerService::GenerateExtensionAbilityRequest` | `Want::PARAM_APP_CLONE_INDEX_KEY` | abilityRecord->GetAppIndex() | ❌否 | — |
| 54 | `services/abilitymgr/src/ability_manager_service.cpp:9097` `AbilityManagerService::GenerateExtensionAbilityRequest` | `Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | — |
| 55 | `services/abilitymgr/src/ability_manager_service.cpp:9103` `AbilityManagerService::GenerateExtensionAbilityRequest` | `AbilityRuntime::ServerConstant::DLP_INDEX` | abilityRecord->GetAppIndex() | ❌否 | — |
| 56 | `services/abilitymgr/src/ability_manager_service.cpp:9104` `AbilityManagerService::GenerateExtensionAbilityRequest` | `DLP_PARAMS_SECURITY_FLAG` | abilityRecord->GetSecurityFlag() | ❌否 | — |
| 57 | `services/abilitymgr/src/ability_manager_service.cpp:10020` `AbilityManagerService::StartAutoStartupApps` | `Want::PARAM_APP_CLONE_INDEX_KEY` | info.appCloneIndex | ❌否 | — |
| 58 | `services/abilitymgr/src/ability_manager_service.cpp:10552` `AbilityManagerService::StartAbilityByCallWithErrMsg` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | ✅ |
| 59 | `services/abilitymgr/src/ability_manager_service.cpp:10603` `AbilityManagerService::StartAbilityForPrelaunch` | `Want::PARAM_APP_CLONE_INDEX_KEY` | 0 | ❌否 | ✅ |
| 60 | `services/abilitymgr/src/ability_manager_service.cpp:10656` `AbilityManagerService::StartAbilityForPrelaunch` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | ✅ |
| 61 | `services/abilitymgr/src/ability_manager_service.cpp:11397` `AbilityManagerService::ScheduleRecoverAbility` | `AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART` | true | ❌否 | — |
| 62 | `services/abilitymgr/src/ability_manager_service.cpp:16106` `AbilityManagerService::RequestAssertFaultDialog` | `Want::PARAM_ASSERT_FAULT_SESSION_ID` | std::to_string(assertFaultSessionId) | ❌否 | ✅ |
| 63 | `services/abilitymgr/src/ability_manager_service.cpp:16351` `AbilityManagerService::RestartApp` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | processInfo.appCloneIndex | ❌否 | ✅ |
| 64 | `services/abilitymgr/src/ability_manager_service.cpp:16352` `AbilityManagerService::RestartApp` | `AAFwk::Want::APP_INSTANCE_KEY` | processInfo.instanceKey | ❌否 | ✅ |
| 65 | `services/abilitymgr/src/ability_manager_service.cpp:16519` `AbilityManagerService::IsEmbeddedOpenAllowed` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerAbility->GetBundleName() | ❌否 | ✅ |
| 66 | `services/abilitymgr/src/ability_manager_service.cpp:16566` `AbilityManagerService::QueryAtomicServiceStartupRule` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerAbility->GetBundleName() | ❌否 | ✅ |
| 67 | `services/abilitymgr/src/ability_manager_service.cpp:17102` `AbilityManagerService::PreStartInner` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | — |
| 68 | `services/abilitymgr/src/ability_manager_service.cpp:17311` `AbilityManagerService::StartUIAbilityByPreInstallInner` | `SPECIFY_TOKEN_ID` | static_cast<int32_t>(specifyTokenId) | ✅是 | — |
| 69 | `services/abilitymgr/src/ability_manager_service.cpp:17432` `AbilityManagerService::IntentOpenLinkInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` | 0 | ❌否 | — |
| 70 | `services/abilitymgr/src/ability_manager_service.cpp:17496` `AbilityManagerService::OpenLinkInner` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerBundleName | ❌否 | ✅ |
| 71 | `services/abilitymgr/src/ability_manager_service.cpp:17901` `AbilityManagerService::StartSelfUIAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` | processInfo.appCloneIndex | ✅是 | ✅ |
| 72 | `services/abilitymgr/src/ability_manager_service.cpp:17908` `AbilityManagerService::StartSelfUIAbilityInner` | `Want::APP_INSTANCE_KEY` | processInfo.instanceKey | ❌否 | ✅ |
| 73 | `services/abilitymgr/src/ability_manager_service.cpp:18638` `AbilityManagerService::RestartSelfAtomicService` | `Want::PARAM_RESV_START_TIME` | startTime | ❌否 | ✅ |
| 74 | `services/abilitymgr/src/ability_manager_service.cpp:18959` `AbilityManagerService::StartSelfUIAbilityInCurrentProcess` | `KEY_SPECIFIED_FLAG` | specifiedFlag | ❌否 | ✅ |
| 75 | `services/abilitymgr/src/ability_manager_service.cpp:18989` `AbilityManagerService::StartSelfUIAbilityInChildProcess` | `KEY_SPECIFIED_FLAG` | specifiedFlag | ❌否 | ✅ |
| 76 | `services/abilitymgr/src/ability_manager_service.cpp:19261` `AbilityManagerService::HandleAppDiedForRecovery` | `Want::PARAM_ABILITY_RECOVERY_RESTART` | true | ❌否 | — |
| 77 | `services/abilitymgr/src/ability_manager_stub.cpp:3276` `AbilityManagerStub::FreeInstallAbilityFromRemoteInner` | `FROM_REMOTE_KEY` | true | ❌否 | — |
| 78 | `services/abilitymgr/src/ability_record.cpp:337` `AbilityRecord::LoadAbility` | `ABILITY_OWNER_USERID` | ownerMissionUserId_ | ❌否 | — |
| 79 | `services/abilitymgr/src/ability_record.cpp:338` `AbilityRecord::LoadAbility` | `IS_HOOK` | isHook_ | ❌否 | — |
| 80 | `services/abilitymgr/src/ability_record.cpp:1483` `AbilityRecord::SaveResult` | `std::string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)` | abilityInfo_.applicationInfo.name | ❌否 | — |
| 81 | `services/abilitymgr/src/ability_record.cpp:1484` `AbilityRecord::SaveResult` | `std::string(PARAM_SEND_RESULT_CALLER_TOKENID)` | static_cast<int32_t>( abilityInfo_.applicationInfo.accessTokenId) | ❌否 | — |
| 82 | `services/abilitymgr/src/ability_record.cpp:2115` `AbilityRecord::SetWant` | `AbilityConfig::DEBUG_APP` | launchDebugInfo_.debugApp | ❌否 | — |
| 83 | `services/abilitymgr/src/ability_record.cpp:2130` `AbilityRecord::SetWant` | `MULTI_THREAD` | true | ❌否 | — |
| 84 | `services/abilitymgr/src/ability_record.cpp:2233` `AbilityRecord::SetWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` | windowMode | ❌否 | — |
| 85 | `services/abilitymgr/src/ability_record.cpp:2247` `AbilityRecord::UpdateRecoveryInfo` | `Want::PARAM_ABILITY_RECOVERY_RESTART` | true | ❌否 | — |
| 86 | `services/abilitymgr/src/ability_record.cpp:2280` `AbilityRecord::EvaluateRecoveryLaunchReason` | `Want::PARAM_ABILITY_RECOVERY_RESTART` | false | ❌否 | — |
| 87 | `services/abilitymgr/src/ability_record.cpp:2533` `AbilityRecord::SetWantAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | — |
| 88 | `services/abilitymgr/src/ability_record.cpp:3241` `AbilityRecord::SetDebugAppByWaitingDebugFlag` | `AbilityConfig::DEBUG_APP` | true | ❌否 | — |
| 89 | `services/abilitymgr/src/ability_record.cpp:3277` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` | -1 | ❌否 | — |
| 90 | `services/abilitymgr/src/ability_record.cpp:3278` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_UID` | -1 | ❌否 | — |
| 91 | `services/abilitymgr/src/ability_record.cpp:3279` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_PID` | -1 | ❌否 | — |
| 92 | `services/abilitymgr/src/ability_record.cpp:3281` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | want.GetStringParam(DMS_CALLER_BUNDLE_NAME) | ❌否 | — |
| 93 | `services/abilitymgr/src/ability_record.cpp:3283` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | want.GetStringParam(DMS_CALLER_ABILITY_NAME) | ❌否 | — |
| 94 | `services/abilitymgr/src/ability_record.cpp:3285` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` | want.GetStringParam(DMS_CALLER_NATIVE_NAME) | ❌否 | — |
| 95 | `services/abilitymgr/src/ability_record.cpp:3287` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_APP_ID` | want.GetStringParam(DMS_CALLER_APP_ID) | ❌否 | — |
| 96 | `services/abilitymgr/src/ability_record.cpp:3289` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_APP_IDENTIFIER` | want.GetStringParam(DMS_CALLER_APP_IDENTIFIER) | ❌否 | — |
| 97 | `services/abilitymgr/src/ability_record.cpp:3300` `AbilityRecord::SetDebugUIExtension` | `AbilityConfig::DEBUG_APP` | true | ❌否 | — |
| 98 | `services/abilitymgr/src/ability_record.cpp:3308` `AbilityRecord::SetDebugApp` | `AbilityConfig::DEBUG_APP` | isDebugApp | ❌否 | — |
| 99 | `services/abilitymgr/src/atomic_service_status_callback_stub.cpp:48` `AtomicServiceStatusCallbackStub::OnRemoteInstallFinishedInner` | `FROM_REMOTE_KEY` | true | ❌否 | — |
| 100 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:308` `DialogSessionManager::SendDialogResult` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | — |
| 101 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:315` `DialogSessionManager::SendDialogResult` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | 0 | ❌否 | — |
| 102 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:404` `DialogSessionManager::CreateJumpModalDialog` | `"userId"` | AAFwk::Integer::Box(userId) | ❌否 | — |
| 103 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:426` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"userId"` | userId | ❌否 | — |
| 104 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:471` `DialogSessionManager::CreateCloneSelectorModalDialog` | `"userId"` | AAFwk::Integer::Box(userId) | ❌否 | — |
| 105 | `services/abilitymgr/src/disposed_observer.cpp:175` `DisposedObserver::ExecuteUIExtension` | `IS_EMBEDDABLE_SERVICE` | true | ❌否 | — |
| 106 | `services/abilitymgr/src/free_install_manager.cpp:141` `FreeInstallManager::StartFreeInstall` | `PARAM_FREEINSTALL_UID` | IPCSkeleton::GetCallingUid() | ❌否 | ✅ |
| 107 | `services/abilitymgr/src/free_install_manager.cpp:779` `FreeInstallManager::SetAppRunningState` | `KEY_IS_APP_RUNNING` | isAppRunning | ❌否 | — |
| 108 | `services/abilitymgr/src/implicit_start_processor.cpp:167` `ImplicitStartProcessor::ImplicitStartAbility` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | ✅ |
| 109 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:660` `InsightIntentExecuteManager::GenerateWant` | `Want::PARAM_RESV_START_TIME` | startTime | ❌否 | — |
| 110 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:671` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_PARAM_USER_ID` | param->userId_ | ❌否 | — |
| 111 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:673` `InsightIntentExecuteManager::GenerateWant` | `Want::PARAM_RESV_DISPLAY_ID` | param->displayId_ | ❌否 | — |
| 112 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:344` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_TOKEN` | AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_TOK... | ✅是 | — |
| 113 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:348` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_UID` | AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_UID... | ✅是 | — |
| 114 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:352` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_PID` | AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_PID... | ✅是 | — |
| 115 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:356` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME` | AAFwk::String::Box(wantParams.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_B... | ✅是 | — |
| 116 | `services/abilitymgr/src/keep_alive/keep_alive_process_manager.cpp:155` `KeepAliveProcessManager::StartKeepAliveMainAbility` | `Want::PARAM_APP_CLONE_INDEX_KEY` | info.appCloneIndex | ❌否 | — |
| 117 | `services/abilitymgr/src/keep_alive/keep_alive_process_manager.cpp:520` `KeepAliveProcessManager::StartKeepAliveAppServiceExtensionInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` | info.appCloneIndex | ❌否 | — |
| 118 | `services/abilitymgr/src/kiosk_manager.cpp:295` `KioskManager::NotifyKioskModeChanged` | `"bundleName"` | bundleName | ❌否 | — |
| 119 | `services/abilitymgr/src/kiosk_manager.cpp:296` `KioskManager::NotifyKioskModeChanged` | `"uid"` | kioskBundleUid | ❌否 | — |
| 120 | `services/abilitymgr/src/kiosk_manager.cpp:297` `KioskManager::NotifyKioskModeChanged` | `"userId"` | kioskBundleUid / BASE_USER_RANGE | ❌否 | — |
| 121 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3079` `MissionListManager::ProcessPreload` | `"uid"` | uid | ❌否 | — |
| 122 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3685` `MissionListManager::NotifyStartSpecifiedAbility` | `Want::PARAM_RESV_REQUEST_PROC_CODE` | procCode | ❌否 | — |
| 123 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3689` `MissionListManager::NotifyStartSpecifiedAbility` | `Want::PARAM_RESV_REQUEST_TOKEN_CODE` | tokenCode | ❌否 | — |
| 124 | `services/abilitymgr/src/pending_want_record.cpp:241` `PendingWantRecord::BuildSendWant` | `Want::PARAM_APP_CLONE_INDEX_KEY` | Integer::Box(appIndex) | ❌否 | — |
| 125 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:229` `UIAbilityLifecycleManager::StartUIAbility` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` | sessionInfo->requestId | ❌否 | — |
| 126 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:232` `UIAbilityLifecycleManager::StartUIAbility` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` | sessionInfo->scbRequestId | ❌否 | — |
| 127 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:412` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` | sessionInfo.requestId | ❌否 | — |
| 128 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:415` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` | sessionInfo.scbRequestId | ❌否 | — |
| 129 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:419` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` | sessionInfo.requestId | ❌否 | — |
| 130 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:423` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` | sessionInfo.scbRequestId | ❌否 | — |
| 131 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:453` `UIAbilityLifecycleManager::GenerateAbilityRecord` | `Want::APP_INSTANCE_KEY` | sessionInfo->instanceKey | ❌否 | — |
| 132 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:731` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::ServerConstant::DLP_INDEX` | abilityRequest.abilityInfo.applicationInfo.appIndex | ❌否 | ✅ |
| 133 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:734` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::IS_WEB_SANDBOX_CLONE` | abilityRequest.isWebSandBoxClone | ❌否 | ✅ |
| 134 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:736` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::SANDBOX_CLONE_INDEX` | abilityRequest.abilityInfo.applicationInfo.appIndex | ❌否 | ✅ |
| 135 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:742` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::CLI_CALLER_BUNDLE_NAME` | callerBundleName | ❌否 | ✅ |
| 136 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:743` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::CLI_CALLER_TOKEN_ID` | callerTokenId | ❌否 | ✅ |
| 137 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:754` `UIAbilityLifecycleManager::NotifySCBToStartUIAbility` | `IS_SHELL_CALL` | AAFwk::PermissionVerification::GetInstance()->IsShellCall() | ❌否 | — |
| 138 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:927` `UIAbilityLifecycleManager::HandleUIAbilityRequestList` | `IS_SHELL_CALL` | AAFwk::PermissionVerification::GetInstance()->IsShellCall() | ❌否 | — |
| 139 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1080` `UIAbilityLifecycleManager::NotifySCBToRecoveryAfterInterception` | `"ohos.ability.params.isSkipErmsFromSCB"` | true | ❌否 | — |
| 140 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:2424` `UIAbilityLifecycleManager::BackToCallerAbilityWithResultLocked` | `ServerConstant::IS_CALL_BY_SCB` | false | ❌否 | — |
| 141 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3040` `UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse` | `PARAM_SPECIFIED_PROCESS_FLAG` | flag | ❌否 | — |
| 142 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3043` `UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse` | `SPECIFIED_PROCESS_CALLER_PROCESS` | callerProcessName | ✅是 | — |
| 143 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3135` `UIAbilityLifecycleManager::StartSpecifiedProcessRequest` | `Want::APP_INSTANCE_KEY` | instanceKey | ❌否 | — |
| 144 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3921` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` | startOptions->GetDisplayID() | ❌否 | — |
| 145 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3922` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_WINDOW_MODE` | startOptions->GetWindowMode() | ❌否 | — |
| 146 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3924` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` | DisplayUtil::GetDefaultDisplayId() | ❌否 | — |
| 147 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3927` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` | -1 | ❌否 | — |
| 148 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5000` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` | realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0) | ❌否 | — |
| 149 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5001` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_UID` | realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0) | ❌否 | — |
| 150 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5002` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_PID` | realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0) | ❌否 | — |
| 151 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5003` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) | ❌否 | — |
| 152 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5005` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME) | ❌否 | — |
| 153 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5007` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` | realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 0) | ❌否 | — |
| 154 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5009` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` | realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME) | ❌否 | — |
| 155 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5011` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_ID` | realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_ID) | ❌否 | — |
| 156 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5012` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_IDENTIFIER` | realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER) | ❌否 | — |
| 157 | `services/abilitymgr/src/utils/ability_event_util.cpp:99` `AbilityEventUtil::HandleBundleFirstLaunch` | `"bundleName"` | appInfo.bundleName | ❌否 | — |
| 158 | `services/abilitymgr/src/utils/ability_event_util.cpp:100` `AbilityEventUtil::HandleBundleFirstLaunch` | `"userId"` | userId | ❌否 | — |
| 159 | `services/abilitymgr/src/utils/ability_event_util.cpp:101` `AbilityEventUtil::HandleBundleFirstLaunch` | `"appIndex"` | appInfo.appIndex | ❌否 | — |
| 160 | `services/abilitymgr/src/utils/ability_event_util.cpp:102` `AbilityEventUtil::HandleBundleFirstLaunch` | `"uid"` | appInfo.uid | ❌否 | — |
| 161 | `services/abilitymgr/src/utils/ability_event_util.cpp:104` `AbilityEventUtil::HandleBundleFirstLaunch` | `"appId"` | appId | ❌否 | — |
| 162 | `services/abilitymgr/src/utils/ability_event_util.cpp:106` `AbilityEventUtil::HandleBundleFirstLaunch` | `"callerBundleName"` | callerBundleName | ❌否 | — |
| 163 | `services/abilitymgr/src/utils/ability_permission_util.cpp:214` `AbilityPermissionUtil::UpdateInstanceKey` | `Want::APP_INSTANCE_KEY` | instanceKey | ❌否 | — |
| 164 | `services/abilitymgr/src/utils/clone_for_account_util.cpp:42` `CloneForAccountUtil::ProcessAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` | abilityInfo.appIndex | ✅是 | — |
| 165 | `services/abilitymgr/src/utils/clone_for_account_util.cpp:53` `CloneForAccountUtil::ProcessAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` | extensionInfo.appIndex | ✅是 | — |
| 166 | `services/abilitymgr/src/utils/dms_util.cpp:38` `DmsUtil::UpdateFlagForCollaboration` | `IS_CALLING_FROM_DMS` | true | ❌否 | ✅ |
| 167 | `services/abilitymgr/src/utils/oe_extension_utils.cpp:86` `OEExtensionUtils::ValidateCaller` | `Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | — |
| 168 | `services/abilitymgr/src/utils/start_ability_utils.cpp:161` `StartAbilityInfoWrap::StartAbilityInfoWrap` | `Want::PARAM_RESV_CALLER_TOKEN` | static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()) | ✅是 | ✅ |
| 169 | `services/abilitymgr/src/utils/start_ability_utils.cpp:162` `StartAbilityInfoWrap::StartAbilityInfoWrap` | `Want::PARAM_RESV_CALLER_UID` | IPCSkeleton::GetCallingUid() | ✅是 | ✅ |
| 170 | `services/abilitymgr/src/utils/start_ability_utils.cpp:451` `StartAbilityUtils::SetTargetCloneIndexInSameBundle` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | ✅ |
| 171 | `services/abilitymgr/src/utils/start_ability_utils.cpp:453` `StartAbilityUtils::SetTargetCloneIndexInSameBundle` | `AbilityRuntime::GlobalConstant::SANDBOX_CLONE_INDEX` | appIndex | ❌否 | ✅ |
| 172 | `services/abilitymgr/src/utils/start_ability_utils.cpp:473` `StartAbilityUtils::ResolveTargetAppCloneIndex` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | appIndex | ❌否 | — |
| 173 | `services/abilitymgr/src/utils/start_ability_utils.cpp:482` `StartAbilityUtils::StartUIAbilitiesProcessAppIndex` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` | 0 | ❌否 | — |
| 174 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:88` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` | tokenId | ✅是 | ✅ |
| 175 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:90` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_UID` | callerUid | ✅是 | ✅ |
| 176 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:92` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_PID` | callerPid | ✅是 | ✅ |
| 177 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:111` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` | nativeName | ❌否 | ✅ |
| 178 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:145` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_TOKEN` | static_cast<int32_t>(callerTokenId) | ✅是 | — |
| 179 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:152` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_UID` | uid | ✅是 | — |
| 180 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:161` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_PID` | 0 | ✅是 | — |
| 181 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:231` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_TOKEN` | tokenId | ❌否 | — |
| 182 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:232` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_UID` | processInfo.uid_ | ❌否 | — |
| 183 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:233` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_PID` | processInfo.pid_ | ❌否 | — |
| 184 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:236` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerBundleName | ❌否 | — |
| 185 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:238` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | callerAbilityName | ❌否 | — |
| 186 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:250` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_TOKEN` | sourceInfo->callerTokenId | ❌否 | — |
| 187 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:251` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_UID` | sourceInfo->callerUid | ❌否 | — |
| 188 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:252` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_PID` | sourceInfo->callerPid | ❌否 | — |
| 189 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:253` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerBundleName | ❌否 | — |
| 190 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:254` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | sourceInfo->callerAbilityName | ❌否 | — |
| 191 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:255` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` | sourceInfo->callerAppCloneIndex | ❌否 | — |
| 192 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:257` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` | sourceInfo->callerNativeName | ❌否 | — |
| 193 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:286` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_TOKEN` | tokenId | ❌否 | ✅ |
| 194 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:287` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_UID` | uid | ❌否 | ✅ |
| 195 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:288` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_PID` | pid | ❌否 | ✅ |
| 196 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:289` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerBundleName | ❌否 | ✅ |
| 197 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:290` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | callerAbilityName | ❌否 | ✅ |
| 198 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:291` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` | callerAppCloneIndex | ❌否 | ✅ |
| 199 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:295` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` | dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME) | ❌否 | ✅ |
| 200 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:315` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_TOKEN` | tokenId | ✅是 | — |
| 201 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:317` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_UID` | callerUid | ✅是 | — |
| 202 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:319` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_PID` | callerPid | ✅是 | — |
| 203 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:326` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | callerBundleName | ✅是 | — |
| 204 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:329` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | callerAbilityName | ✅是 | — |
| 205 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:401` `UpdateCallerInfoUtil::UpdateCallerBundleName` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` | bundleName | ✅是 | ✅ |
| 206 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:407` `UpdateCallerInfoUtil::UpdateCallerAbilityName` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` | abilityName | ✅是 | — |
| 207 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:413` `UpdateCallerInfoUtil::UpdateCallerAppCloneIndex` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` | appIndex | ✅是 | — |
| 208 | `services/abilitymgr/src/utils/uri_utils.cpp:511` `UriUtils::PublishFileOpenEvent` | `"bundleName"` | grantInfo.targetBundleName | ❌否 | — |
| 209 | `services/abilitymgr/src/utils/uri_utils.cpp:518` `UriUtils::PublishFileOpenEvent` | `"callerUid"` | grantInfo.callerUid | ❌否 | — |
| 210 | `services/abilitymgr/src/utils/window_options_utils.cpp:30` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_LEFT` | startOptions.GetWindowLeft() | ❌否 | — |
| 211 | `services/abilitymgr/src/utils/window_options_utils.cpp:33` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_TOP` | startOptions.GetWindowTop() | ❌否 | — |
| 212 | `services/abilitymgr/src/utils/window_options_utils.cpp:36` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_WIDTH` | startOptions.GetWindowWidth() | ❌否 | — |
| 213 | `services/abilitymgr/src/utils/window_options_utils.cpp:39` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_HEIGHT` | startOptions.GetWindowHeight() | ❌否 | — |
| 214 | `services/abilitymgr/src/utils/window_options_utils.cpp:42` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MIN_WINDOW_WIDTH` | startOptions.GetMinWindowWidth() | ❌否 | — |
| 215 | `services/abilitymgr/src/utils/window_options_utils.cpp:45` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MIN_WINDOW_HEIGHT` | startOptions.GetMinWindowHeight() | ❌否 | — |
| 216 | `services/abilitymgr/src/utils/window_options_utils.cpp:48` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MAX_WINDOW_WIDTH` | startOptions.GetMaxWindowWidth() | ❌否 | — |
| 217 | `services/abilitymgr/src/utils/window_options_utils.cpp:51` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MAX_WINDOW_HEIGHT` | startOptions.GetMaxWindowHeight() | ❌否 | — |
| 218 | `services/abilitymgr/src/utils/window_options_utils.cpp:57` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WITH_ANIMATION` | withAnimation | ❌否 | — |
| 219 | `services/appmgr/src/app_mgr_service_inner.cpp:1786` `AppMgrServiceInner::SetPreloadDebugApp` | `DEBUG_APP` | true | ❌否 | — |
| 220 | `services/appmgr/src/app_mgr_service_inner.cpp:5316` `AppMgrServiceInner::StartAbility` | `DLP_PARAMS_SECURITY_FLAG` | appRecord->GetSecurityFlag() | ❌否 | — |
| 221 | `services/appmgr/src/app_mgr_service_inner.cpp:7120` `AppMgrServiceInner::NotifyAppStatus` | `Constants::USER_ID` | 0 | ❌否 | — |
| 222 | `services/appmgr/src/app_mgr_service_inner.cpp:7121` `AppMgrServiceInner::NotifyAppStatus` | `Constants::APP_INDEX` | appIndex | ❌否 | — |
| 223 | `services/appmgr/src/app_mgr_service_inner.cpp:7136` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `TOKEN_ID` | tokenId | ❌否 | — |
| 224 | `services/appmgr/src/app_mgr_service_inner.cpp:7137` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Constants::USER_ID` | userId | ❌否 | — |
| 225 | `services/appmgr/src/app_mgr_service_inner.cpp:7138` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Constants::UID` | callerUid | ❌否 | — |
| 226 | `services/appmgr/src/app_mgr_service_inner.cpp:7139` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Want::PARAM_RESV_CALLER_UID` | callerUid | ❌否 | — |
| 227 | `services/appmgr/src/app_mgr_service_inner.cpp:7140` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `TARGET_UID_KEY` | targetUid | ❌否 | — |
| 228 | `services/appmgr/src/app_mgr_service_inner.cpp:12641` `AppMgrServiceInner::NotifyAppFaultBySA` | `REUSING_WINDOW` | true | ❌否 | — |

## 四、按 key 统计

| key（归一化） | 类别 | 处数 |
|---|---|---|
| `ABILITY_OWNER_USERID` | 用户与克隆 | 1 |
| `APP_INDEX` | 用户与克隆 | 1 |
| `APP_INSTANCE_KEY` | 用户与克隆 | 5 |
| `CLI_CALLER_BUNDLE_NAME` | 调用者身份 | 2 |
| `CLI_CALLER_TOKEN_ID` | 调用者身份 | 2 |
| `CollaboratorRemoveCallerName` | 业务字符串 | 1 |
| `DEBUG_APP` | 安全与调试 | 6 |
| `DLP_INDEX` | 安全与调试 | 3 |
| `DLP_PARAMS_SANDBOX` | 安全与调试 | 1 |
| `DLP_PARAMS_SECURITY_FLAG` | 安全与调试 | 3 |
| `DMS_CALLING_UID` | 调用者身份 | 2 |
| `FROM_REMOTE_KEY` | 调度与进程控制 | 2 |
| `GAME_PRELAUNCH` | 调度与进程控制 | 2 |
| `HIDDEN_START_AUTOSTARTUP` | 调度与进程控制 | 1 |
| `INSIGHT_INTENT_PARAM_USER_ID` | 用户与克隆 | 1 |
| `IS_CALLING_FROM_DMS` | 调度与进程控制 | 1 |
| `IS_CALL_BY_SCB` | 调度与进程控制 | 10 |
| `IS_EMBEDDABLE_SERVICE` | 调度与进程控制 | 1 |
| `IS_HOOK` | 调度与进程控制 | 1 |
| `IS_PRELOAD_UIEXTENSION_ABILITY` | 调度与进程控制 | 1 |
| `IS_SHELL_CALL` | 调度与进程控制 | 2 |
| `IS_WEB_SANDBOX_CLONE` | 安全与调试 | 1 |
| `KEY_IS_APP_RUNNING` | 调度与进程控制 | 1 |
| `KEY_SKIP_ABILITY_STAGE_LIFECYCLE` | 调度与进程控制 | 1 |
| `KEY_SPECIFIED_FLAG` | 调度与进程控制 | 2 |
| `MULTI_THREAD` | 调度与进程控制 | 1 |
| `NEED_STARTINGWINDOW` | 调度与进程控制 | 1 |
| `PARAM_ABILITY_RECOVERY_RESTART` | 恢复与保活 | 4 |
| `PARAM_APP_CLONE_INDEX_KEY` | 用户与克隆 | 22 |
| `PARAM_ASSERT_FAULT_SESSION_ID` | 安全与调试 | 1 |
| `PARAM_FREEINSTALL_UID` | 调用者身份 | 1 |
| `PARAM_RESV_ABILITY_INFO_CALLBACK` | 会话与回调 | 1 |
| `PARAM_RESV_APP_REQUEST_ID` | 会话与回调 | 3 |
| `PARAM_RESV_CALLER_ABILITY_NAME` | 调用者身份 | 8 |
| `PARAM_RESV_CALLER_APP_CLONE_INDEX` | 调用者身份 | 4 |
| `PARAM_RESV_CALLER_APP_ID` | 调用者身份 | 2 |
| `PARAM_RESV_CALLER_APP_IDENTIFIER` | 调用者身份 | 2 |
| `PARAM_RESV_CALLER_BUNDLE_NAME` | 调用者身份 | 12 |
| `PARAM_RESV_CALLER_NATIVE_NAME` | 调用者身份 | 5 |
| `PARAM_RESV_CALLER_PID` | 调用者身份 | 9 |
| `PARAM_RESV_CALLER_TOKEN` | 调用者身份 | 11 |
| `PARAM_RESV_CALLER_UID` | 调用者身份 | 12 |
| `PARAM_RESV_DISPLAY_ID` | 显示与窗口 | 11 |
| `PARAM_RESV_MAX_WINDOW_HEIGHT` | 显示与窗口 | 1 |
| `PARAM_RESV_MAX_WINDOW_WIDTH` | 显示与窗口 | 1 |
| `PARAM_RESV_MIN_WINDOW_HEIGHT` | 显示与窗口 | 1 |
| `PARAM_RESV_MIN_WINDOW_WIDTH` | 显示与窗口 | 1 |
| `PARAM_RESV_REQUEST_PROC_CODE` | 会话与回调 | 2 |
| `PARAM_RESV_REQUEST_TOKEN_CODE` | 会话与回调 | 2 |
| `PARAM_RESV_SCB_REQUEST_ID` | 会话与回调 | 3 |
| `PARAM_RESV_START_TIME` | 时间戳 | 2 |
| `PARAM_RESV_WINDOW_FOCUSED` | 显示与窗口 | 2 |
| `PARAM_RESV_WINDOW_HEIGHT` | 显示与窗口 | 1 |
| `PARAM_RESV_WINDOW_LEFT` | 显示与窗口 | 1 |
| `PARAM_RESV_WINDOW_MODE` | 显示与窗口 | 6 |
| `PARAM_RESV_WINDOW_TOP` | 显示与窗口 | 1 |
| `PARAM_RESV_WINDOW_WIDTH` | 显示与窗口 | 1 |
| `PARAM_RESV_WITH_ANIMATION` | 显示与窗口 | 1 |
| `PARAM_SPECIFIED_PROCESS_FLAG` | 调度与进程控制 | 2 |
| `REUSING_WINDOW` | 显示与窗口 | 1 |
| `SANDBOX_CLONE_INDEX` | 安全与调试 | 2 |
| `SPECIFIED_PROCESS_CALLER_PROCESS` | 调度与进程控制 | 1 |
| `SPECIFY_TOKEN_ID` | 调用者身份 | 4 |
| `TARGET_UID_KEY` | 调用者身份 | 1 |
| `TOKEN_ID` | 调用者身份 | 1 |
| `UID` | 调用者身份 | 1 |
| `UI_EXTENSION_TARGET_USER_ID` | 用户与克隆 | 1 |
| `USER_ID` | 用户与克隆 | 2 |
| `WANT_PARAMS_APP_RESTART_FLAG` | 调度与进程控制 | 1 |
| `appId` | 业务字符串 | 1 |
| `appIndex` | 用户与克隆 | 1 |
| `bundleName` | 业务字符串 | 3 |
| `callerBundleName` | 业务字符串 | 1 |
| `callerUid` | 调用者身份 | 1 |
| `ohos.ability.params.isSkipErmsFromSCB` | 调度与进程控制 | 1 |
| `ohos.app.logout_recovery` | 恢复与保活 | 1 |
| `ohos.app.recovery` | 恢复与保活 | 1 |
| `string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)` | 业务字符串 | 1 |
| `string(PARAM_SEND_RESULT_CALLER_TOKENID)` | 业务字符串 | 1 |
| `uid` | 业务字符串 | 4 |
| `userId` | 用户与克隆 | 5 |

## 五、系统参数白名单外 SET 点（263 处，服务侧）

范围：同一枚举（`services/`+`interfaces/` 非测试/mock），但 key 不在系统参数白名单（前四节 81 个 key）内。
口径：与正文一致，仅统计 `Want::SetParam`/`WantParams::SetParam` 调用点；`app 侧框架`见第六节。

| # | 设置处(file:line / 函数) | key | value | 设置前重置 | IPC 派生 |
|---|---|---|---|---|---|
| 1 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:138` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_AUTO_FILL_CMD_KEY` | static_cast<int32_t>(AutoFill::AutoFillCommand::RELOAD_IN_MODAL) | ❌否 | — |
| 2 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:139` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_CUSTOM_DATA_KEY` | wantParams.GetStringParam(WANT_PARAMS_CUSTOM_DATA_KEY) | ❌否 | — |
| 3 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:140` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_EXTENSION_TYPE_KEY` | std::string(WANT_PARAMS_SMART_EXTENSION_TYPE)) : | ❌否 | — |
| 4 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:141` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_EXTENSION_TYPE_KEY` | std::string(WANT_PARAMS_EXTENSION_TYPE) | ❌否 | — |
| 5 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:142` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_AUTO_FILL_TYPE_KEY` | static_cast<int32_t>(request_.autoFillType) | ❌否 | — |
| 6 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:143` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_VIEW_DATA_KEY` | request_.viewData.ToJsonString() | ❌否 | — |
| 7 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:144` `AutoFillExtensionCallback::ReloadInModal` | `WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY` | false | ❌否 | — |
| 8 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:333` `AutoFillExtensionCallback::UpdateCustomPopupUIExtension` | `WANT_PARAMS_AUTO_FILL_CMD_KEY` | AAFwk::Integer::Box(static_cast<int32_t>(AutoFill::AutoFillCommand::UPDATE)) | ❌否 | — |
| 9 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_extension_callback.cpp:335` `AutoFillExtensionCallback::UpdateCustomPopupUIExtension` | `WANT_PARAMS_VIEW_DATA_KEY` | AAFwk::String::Box(viewData.ToJsonString()) | ❌否 | — |
| 10 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:201` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_AUTO_FILL_CMD_KEY` | static_cast<int32_t>(request.autoFillCommand) | ❌否 | — |
| 11 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:202` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_AUTO_FILL_TYPE_KEY` | static_cast<int32_t>(request.autoFillType) | ❌否 | — |
| 12 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:203` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_VIEW_DATA_KEY` | request.viewData.ToJsonString() | ❌否 | — |
| 13 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:205` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_AUTO_FILL_TRIGGER_TYPE_KEY` | static_cast<int32_t>(request.autoFillTriggerType) | ❌否 | — |
| 14 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:207` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_EXTENSION_TYPE_KEY` | WANT_PARAMS_SMART_EXTENSION_TYPE) : | ❌否 | — |
| 15 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:208` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_EXTENSION_TYPE_KEY` | WANT_PARAMS_EXTENSION_TYPE | ❌否 | — |
| 16 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:211` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY` | true | ❌否 | — |
| 17 | `interfaces/inner_api/auto_fill_manager/src/auto_fill_manager.cpp:226` `AutoFillManager::CreateAutoFillExtension` | `WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY` | false | ❌否 | — |
| 18 | `services/abilitymgr/include/utils/ability_util.h:206` `HandleDlpApp` | `DLP_PARAMS_BUNDLE_NAME` | element.GetBundleName() | ❌否 | — |
| 19 | `services/abilitymgr/include/utils/ability_util.h:207` `HandleDlpApp` | `DLP_PARAMS_MODULE_NAME` | element.GetModuleName() | ❌否 | — |
| 20 | `services/abilitymgr/include/utils/ability_util.h:208` `HandleDlpApp` | `DLP_PARAMS_ABILITY_NAME` | element.GetAbilityName() | ❌否 | — |
| 21 | `services/abilitymgr/include/utils/ability_util.h:299` `StartAppgallery` | `MARKET_CROWD_TEST_BUNDLE_PARAM` | bundleName | ❌否 | ✅ |
| 22 | `services/abilitymgr/src/ability_manager_service.cpp:2575` `AbilityManagerService::StartAbilityForOptionInner` | `KEY_REQUEST_ID` | startOptions.requestId_ | ✅是 | — |
| 23 | `services/abilitymgr/src/ability_manager_service.cpp:2701` `AbilityManagerService::StartAbilityForOptionInner` | `KEY_REQUEST_ID` | startOptions.requestId_ | ✅是 | — |
| 24 | `services/abilitymgr/src/ability_manager_service.cpp:3314` `AbilityManagerService::OpenAtomicService` | `AAFwk::SCREEN_MODE_KEY` | AAFwk::ScreenMode::JUMP_SCREEN_MODE | ❌否 | — |
| 25 | `services/abilitymgr/src/ability_manager_service.cpp:3358` `AbilityManagerService::StartUIAbilityBySCB` | `AbilityRuntime::GlobalConstant::PAGE_CONFIG` | params.pageConfig | ❌否 | ✅ |
| 26 | `services/abilitymgr/src/ability_manager_service.cpp:4548` `AbilityManagerService::RequestModalUIExtensionInner` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 27 | `services/abilitymgr/src/ability_manager_service.cpp:4608` `AbilityManagerService::RequestModalUIExtensionWithAccountInner` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 28 | `services/abilitymgr/src/ability_manager_service.cpp:4895` `AbilityManagerService::SetPickerElementNameAndParams` | `UIEXTENSION_TYPE_KEY` | AAFwk::String::Box("sys/commonUI") | ❌否 | — |
| 29 | `services/abilitymgr/src/ability_manager_service.cpp:4937` `AbilityManagerService::SetPickerElementNameAndParams` | `UIEXTENSION_TYPE_KEY` | AAFwk::String::Box(pickerType) | ❌否 | — |
| 30 | `services/abilitymgr/src/ability_manager_service.cpp:5041` `AbilityManagerService::StartUIExtensionAbility` | `"send_to_erms_embedded"` | 1 | ❌否 | — |
| 31 | `services/abilitymgr/src/ability_manager_service.cpp:5838` `AbilityManagerService::StartRemoteAbility` | `DMS_MISSION_ID` | missionId | ❌否 | — |
| 32 | `services/abilitymgr/src/ability_manager_service.cpp:6636` `AbilityManagerService::GetAndSetRootHostToken` | `Want::UI_EXTENSION_ROOT_TOKEN` | rootCallerToken | ❌否 | — |
| 33 | `services/abilitymgr/src/ability_manager_service.cpp:10018` `AbilityManagerService::StartAutoStartupApps` | `Want::PARAM_APP_AUTO_STARTUP_LAUNCH_REASON` | true | ❌否 | — |
| 34 | `services/abilitymgr/src/ability_manager_service.cpp:10369` `AbilityManagerService::StartRemoteAbilityByCall` | `DMS_MISSION_ID` | missionId | ❌否 | — |
| 35 | `services/abilitymgr/src/ability_manager_service.cpp:12683` `AbilityManagerService::ShowPickerDialog` | `TOKEN_KEY` | callerToken | ❌否 | ✅ |
| 36 | `services/abilitymgr/src/ability_manager_service.cpp:13824` `AbilityManagerService::AddStartControlParam` | `DMS_API_VERSION` | apiVersion | ❌否 | — |
| 37 | `services/abilitymgr/src/ability_manager_service.cpp:13834` `AbilityManagerService::AddStartControlParam` | `DMS_IS_CALLER_BACKGROUND` | isCallerBackground | ❌否 | — |
| 38 | `services/abilitymgr/src/ability_manager_service.cpp:14765` `AbilityManagerService::StartAbilityWithServiceMatch` | `AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME` | param.insightIntentName_ | ❌否 | ✅ |
| 39 | `services/abilitymgr/src/ability_manager_service.cpp:16107` `AbilityManagerService::RequestAssertFaultDialog` | `ASSERT_FAULT_DETAIL` | wantParams.GetStringParam(ASSERT_FAULT_DETAIL) | ❌否 | — |
| 40 | `services/abilitymgr/src/ability_manager_service.cpp:16109` `AbilityManagerService::RequestAssertFaultDialog` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 41 | `services/abilitymgr/src/ability_manager_service.cpp:16511` `AbilityManagerService::IsEmbeddedOpenAllowed` | `"send_to_erms_embedded"` | 1 | ❌否 | ✅ |
| 42 | `services/abilitymgr/src/ability_manager_service.cpp:16555` `AbilityManagerService::QueryAtomicServiceStartupRule` | `"send_to_erms_embedded"` | 1 | ❌否 | ✅ |
| 43 | `services/abilitymgr/src/ability_manager_service.cpp:17105` `AbilityManagerService::PreStartInner` | `KEY_SESSION_ID` | sessionId | ❌否 | — |
| 44 | `services/abilitymgr/src/ability_manager_service.cpp:17394` `AbilityManagerService::CombinLinkInfo` | `mapInfo.paramMappingName` | value | ❌否 | — |
| 45 | `services/abilitymgr/src/ability_manager_service.cpp:17427` `AbilityManagerService::IntentOpenLinkInner` | `APP_LINKING_ONLY` | false | ❌否 | — |
| 46 | `services/abilitymgr/src/ability_manager_service.cpp:17429` `AbilityManagerService::IntentOpenLinkInner` | `AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME` | param->insightIntentName_ | ❌否 | — |
| 47 | `services/abilitymgr/src/ability_manager_service.cpp:17430` `AbilityManagerService::IntentOpenLinkInner` | `AppExecFwk::INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG` | 1 | ❌否 | — |
| 48 | `services/abilitymgr/src/ability_manager_service.cpp:17687` `AbilityManagerService::SetAbilityRequestSessionInfo` | `WANT_PARAMS_HOST_WINDOW_ID_KEY` | callerSessionInfo->persistentId | ✅是 | — |
| 49 | `services/abilitymgr/src/ability_manager_service.cpp:17696` `AbilityManagerService::SetAbilityRequestSessionInfo` | `WANT_PARAMS_HOST_WINDOW_ID_KEY` | mainWindowId | ✅是 | — |
| 50 | `services/abilitymgr/src/ability_manager_service.cpp:17698` `AbilityManagerService::SetAbilityRequestSessionInfo` | `WANT_PARAMS_HOST_WINDOW_ID_KEY` | 0 | ✅是 | — |
| 51 | `services/abilitymgr/src/ability_record.cpp:156` `want_` | `COMPONENT_STARTUP_NEW_RULES` | flag | ❌否 | — |
| 52 | `services/abilitymgr/src/ability_record.cpp:438` `AbilityRecord::ForegroundAbility` | `SPECIFIED_ABILITY_FLAG` | GetSpecifiedFlag() | ❌否 | — |
| 53 | `services/abilitymgr/src/ability_record.cpp:1520` `SystemAbilityCallerRecord::SetResultToSystemAbility` | `DMS_SRC_NETWORK_ID` | srcDeviceId | ❌否 | — |
| 54 | `services/abilitymgr/src/ability_record.cpp:1521` `SystemAbilityCallerRecord::SetResultToSystemAbility` | `DMS_MISSION_ID` | missionId | ❌否 | — |
| 55 | `services/abilitymgr/src/ability_record.cpp:2120` `AbilityRecord::SetWant` | `AbilityConfig::NATIVE_DEBUG` | launchDebugInfo_.nativeDebug | ❌否 | — |
| 56 | `services/abilitymgr/src/ability_record.cpp:2125` `AbilityRecord::SetWant` | `AbilityConfig::PERF_CMD` | launchDebugInfo_.perfCmd | ❌否 | — |
| 57 | `services/abilitymgr/src/ability_record.cpp:2133` `AbilityRecord::SetWant` | `ERROR_INFO_ENHANCE` | true | ❌否 | — |
| 58 | `services/abilitymgr/src/ability_record.cpp:2471` `AbilityRecord::SetMissionId` | `KEY_MISSION_ID` | missionId_ | ✅是 | — |
| 59 | `services/abilitymgr/src/ability_record.cpp:3220` `AbilityRecord::UpdateUIExtensionInfo` | `UIEXTENSION_ABILITY_ID` | wantParams.GetIntParam(UIEXTENSION_ABILITY_ID, -1) | ✅是 | — |
| 60 | `services/abilitymgr/src/ability_record.cpp:3225` `AbilityRecord::UpdateUIExtensionInfo` | `UIEXTENSION_ROOT_HOST_PID` | wantParams.GetIntParam(UIEXTENSION_ROOT_HOST_PID, -1) | ✅是 | — |
| 61 | `services/abilitymgr/src/ability_record.cpp:3330` `AbilityRecord::UpdateUIExtensionBindInfo` | `UIEXTENSION_BIND_ABILITY_ID` | wantParams.GetIntParam(UIEXTENSION_BIND_ABILITY_ID, -1) | ✅是 | — |
| 62 | `services/abilitymgr/src/ability_record.cpp:3335` `AbilityRecord::UpdateUIExtensionBindInfo` | `UIEXTENSION_NOTIFY_BIND` | wantParams.GetIntParam(UIEXTENSION_NOTIFY_BIND, -1) | ✅是 | — |
| 63 | `services/abilitymgr/src/ability_record.cpp:3340` `AbilityRecord::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_PID` | wantParams.GetIntParam(UIEXTENSION_HOST_PID, -1) | ✅是 | — |
| 64 | `services/abilitymgr/src/ability_record.cpp:3345` `AbilityRecord::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_UID` | wantParams.GetIntParam(UIEXTENSION_HOST_UID, -1) | ✅是 | — |
| 65 | `services/abilitymgr/src/ability_record.cpp:3350` `AbilityRecord::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_BUNDLENAME` | wantParams.GetStringParam(UIEXTENSION_HOST_BUNDLENAME) | ✅是 | — |
| 66 | `services/abilitymgr/src/ability_record.cpp:3403` `AbilityRecord::AddUIExtensionLaunchTimestamp` | `UIEXTENSION_LAUNCH_TIMESTAMP_HIGH` | high | ❌否 | — |
| 67 | `services/abilitymgr/src/ability_record.cpp:3404` `AbilityRecord::AddUIExtensionLaunchTimestamp` | `UIEXTENSION_LAUNCH_TIMESTAMP_LOW` | low | ❌否 | — |
| 68 | `services/abilitymgr/src/ability_start_with_wait_observer_manager/ability_start_with_wait_observer_manager.cpp:61` `AbilityStartWithWaitObserverManager::RegisterObserver` | `Want::START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY` | observerId_ | ❌否 | — |
| 69 | `services/abilitymgr/src/connection_record.cpp:376` `ConnectionRecord::ScheduleConnectAbilityDone` | `UISERVICEHOSTPROXY_KEY` | hostproxy | ❌否 | — |
| 70 | `services/abilitymgr/src/connection_record.cpp:379` `ConnectionRecord::ScheduleConnectAbilityDone` | `AgentRuntime::AGENTEXTENSIONHOSTPROXY_KEY` | connectorProxy | ❌否 | — |
| 71 | `services/abilitymgr/src/connection_record.cpp:382` `ConnectionRecord::ScheduleConnectAbilityDone` | `AgentRuntime::AGENTID_KEY` | agentId | ❌否 | — |
| 72 | `services/abilitymgr/src/connection_record.cpp:385` `ConnectionRecord::ScheduleConnectAbilityDone` | `AgentRuntime::AGENT_CARD_TYPE_KEY` | agentCardType | ❌否 | — |
| 73 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:304` `DialogSessionManager::SendDialogResult` | `"isSelector"` | dialogCallerInfo->type != SelectorType::WITHOUT_SELECTOR | ❌否 | — |
| 74 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:305` `DialogSessionManager::SendDialogResult` | `DIALOG_SESSION_ID` | dialogSessionId | ❌否 | — |
| 75 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:312` `DialogSessionManager::SendDialogResult` | `PICKER_ERMS_POLICY` | want.GetParams().GetIntParam(PICKER_ERMS_POLICY, 1) | ✅是 | — |
| 76 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:403` `DialogSessionManager::CreateJumpModalDialog` | `"deviceType"` | AAFwk::String::Box(OHOS::system::GetDeviceType()) | ❌否 | — |
| 77 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:405` `DialogSessionManager::CreateJumpModalDialog` | `ATOMIC_SERVICE_SHORT_LINK` | AAFwk::Integer::Box(abilityRequest.atomicServiceShortLink) | ❌否 | — |
| 78 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:425` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"deviceType"` | OHOS::system::GetDeviceType() | ❌否 | — |
| 79 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:427` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"action"` | abilityRequest.want.GetActionRef() | ❌否 | — |
| 80 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:428` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"wantType"` | abilityRequest.want.GetType() | ❌否 | — |
| 81 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:429` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"uri"` | abilityRequest.want.GetUriString() | ❌否 | — |
| 82 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:430` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"entities"` | abilityRequest.want.GetEntities() | ❌否 | — |
| 83 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:431` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"appselector.selectorType"` | static_cast<int>(SelectorType::IMPLICIT_START_SELECTOR) | ❌否 | — |
| 84 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:433` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"showCaller"` | showCaller | ❌否 | — |
| 85 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:434` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"ohos.ability.params.showDefaultPicker"` | abilityRequest.want.GetBoolParam("ohos.ability.params.showDefaultPicker", false) | ❌否 | — |
| 86 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:436` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `ATOMIC_SERVICE_SHORT_LINK` | abilityRequest.atomicServiceShortLink | ❌否 | — |
| 87 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:438` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `APP_LAUNCH_TRUSTLIST` | abilityRequest.want.GetStringArrayParam(APP_LAUNCH_TRUSTLIST) | ❌否 | — |
| 88 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:450` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"show_default_appselect"` | abilityRequest.want.GetIntParam("show_default_appselect", -1) | ❌否 | — |
| 89 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:470` `DialogSessionManager::CreateCloneSelectorModalDialog` | `"deviceType"` | AAFwk::String::Box(OHOS::system::GetDeviceType()) | ❌否 | — |
| 90 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:472` `DialogSessionManager::CreateCloneSelectorModalDialog` | `"appselector.selectorType"` | AAFwk::Integer::Box(static_cast<int>(SelectorType::APP_CLONE_SELECTOR)) | ❌否 | — |
| 91 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:475` `DialogSessionManager::CreateCloneSelectorModalDialog` | `"ecological.replaceWant"` | AAFwk::String::Box(replaceWant) | ❌否 | — |
| 92 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:477` `DialogSessionManager::CreateCloneSelectorModalDialog` | `ATOMIC_SERVICE_SHORT_LINK` | AAFwk::Integer::Box(abilityRequest.atomicServiceShortLink) | ❌否 | — |
| 93 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:493` `DialogSessionManager::CreateModalDialogCommon` | `DIALOG_SESSION_ID` | dialogSessionId | ❌否 | — |
| 94 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:497` `DialogSessionManager::CreateModalDialogCommon` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 95 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:498` `DialogSessionManager::CreateModalDialogCommon` | `SUPPORT_CLOSE_ON_BLUR` | true | ❌否 | — |
| 96 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:536` `DialogSessionManager::CreateModalDialogCommon` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 97 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:537` `DialogSessionManager::CreateModalDialogCommon` | `SUPPORT_CLOSE_ON_BLUR` | true | ❌否 | — |
| 98 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:562` `DialogSessionManager::HandleErmsResultBySCB` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 99 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:563` `DialogSessionManager::HandleErmsResultBySCB` | `SUPPORT_CLOSE_ON_BLUR` | true | ❌否 | — |
| 100 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:570` `DialogSessionManager::HandleErmsResultBySCB` | `DIALOG_SESSION_ID` | dialogSessionId | ❌否 | — |
| 101 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:631` `DialogSessionManager::UpdateExtensionWantWithDialogCallerInfo` | `AbilityConfig::PARAMS_STREAM` | uriVec | ❌否 | — |
| 102 | `services/abilitymgr/src/disposed_observer.cpp:104` `DisposedObserver::OnAbilityStateChanged` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 103 | `services/abilitymgr/src/disposed_observer.cpp:107` `DisposedObserver::OnAbilityStateChanged` | `INTERCEPT_MISSION_ID` | sessionInfo->persistentId | ❌否 | — |
| 104 | `services/abilitymgr/src/disposed_observer.cpp:109` `DisposedObserver::OnAbilityStateChanged` | `INTERCEPT_MISSION_ID` | abilityRecord->GetMissionId() | ❌否 | — |
| 105 | `services/abilitymgr/src/disposed_observer.cpp:187` `DisposedObserver::ExecuteUIExtension` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 106 | `services/abilitymgr/src/disposed_observer.cpp:199` `DisposedObserver::ExecuteUIExtension` | `INTERCEPT_MISSION_ID` | sessionInfo->persistentId | ❌否 | — |
| 107 | `services/abilitymgr/src/disposed_observer.cpp:201` `DisposedObserver::ExecuteUIExtension` | `INTERCEPT_MISSION_ID` | abilityRecord->GetMissionId() | ❌否 | — |
| 108 | `services/abilitymgr/src/ecological_rule/ability_ecological_rule_mgr_service.cpp:143` `AbilityEcologicalRuleMgrServiceClient::QueryStartExperience` | `ERMS_ORIGINAL_TARGET` | want.ToString() | ❌否 | — |
| 109 | `services/abilitymgr/src/free_install_manager.cpp:233` `FreeInstallManager::BuildFreeInstallInfo` | `KEY_REQUEST_ID` | param->startOptions->requestId_ | ❌否 | — |
| 110 | `services/abilitymgr/src/free_install_manager.cpp:257` `FreeInstallManager::StartRemoteFreeInstall` | `DMS_MISSION_ID` | missionId | ❌否 | — |
| 111 | `services/abilitymgr/src/free_install_manager.cpp:850` `FreeInstallManager::SetFreeInstallTaskSessionId` | `KEY_SESSION_ID` | sessionId | ❌否 | — |
| 112 | `services/abilitymgr/src/free_install_manager.cpp:898` `FreeInstallManager::NotifyInsightIntentFreeInstallResult` | `AppExecFwk::INSIGHT_INTENT_SRC_ENTRY` | srcEntry | ❌否 | — |
| 113 | `services/abilitymgr/src/implicit_start_processor.cpp:1045` `ImplicitStartProcessor::SetTargetLinkInfo` | `"send_to_erms_targetLinkFeature"` | iter.linkFeature | ✅是 | — |
| 114 | `services/abilitymgr/src/implicit_start_processor.cpp:1048` `ImplicitStartProcessor::SetTargetLinkInfo` | `"send_to_erms_targetLinkType"` | AbilityCallerInfo::LINK_TYPE_UNIVERSAL_LINK | ✅是 | — |
| 115 | `services/abilitymgr/src/implicit_start_processor.cpp:1051` `ImplicitStartProcessor::SetTargetLinkInfo` | `"send_to_erms_targetLinkType"` | AbilityCallerInfo::LINK_TYPE_WEB_LINK | ✅是 | — |
| 116 | `services/abilitymgr/src/implicit_start_processor.cpp:1053` `ImplicitStartProcessor::SetTargetLinkInfo` | `"send_to_erms_targetLinkType"` | AbilityCallerInfo::LINK_TYPE_DEEP_LINK | ✅是 | — |
| 117 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:183` `InsightIntentExecuteManager::CheckAndUpdateWant` | `AbilityConfig::PARAMS_STREAM` | uriVec | ❌否 | — |
| 118 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:212` `InsightIntentExecuteManager::CheckAndUpdateWant` | `INSIGHT_INTENT_SRC_ENTRY` | srcEntry | ❌否 | — |
| 119 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:213` `InsightIntentExecuteManager::CheckAndUpdateWant` | `INSIGHT_INTENT_ARKTS_MODE` | arkTSMode | ❌否 | — |
| 120 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:215` `InsightIntentExecuteManager::CheckAndUpdateWant` | `"ohos.insightIntent.decoratorClass"` | decoratorClass | ❌否 | — |
| 121 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:217` `InsightIntentExecuteManager::CheckAndUpdateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_ID` | std::to_string(intentId) | ❌否 | — |
| 122 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:218` `InsightIntentExecuteManager::CheckAndUpdateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_MODE` | executeMode | ❌否 | — |
| 123 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:237` `InsightIntentExecuteManager::UpdateEntryDecoratorParams` | `INSIGHT_INTENT_DECORATOR_TYPE` | static_cast<int>(type) | ❌否 | ✅ |
| 124 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:239` `InsightIntentExecuteManager::UpdateEntryDecoratorParams` | `INSIGHT_INTENT_SRC_ENTRANCE` | srcEntrance | ❌否 | ✅ |
| 125 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:428` `InsightIntentExecuteManager::AddWantUirsAndFlagsFromParam` | `AbilityConfig::PARAMS_STREAM` | uriVec | ❌否 | — |
| 126 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:455` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_SRC_ENTRANCE` | srcEntrance | ❌否 | — |
| 127 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:474` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_FUNC_PARAM_CLASSNAME` | className | ❌否 | — |
| 128 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:475` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_FUNC_PARAM_METHODNAME` | methodName | ❌否 | — |
| 129 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:476` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE` | methodReturnType | ❌否 | — |
| 130 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:477` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS` | methodParams | ❌否 | — |
| 131 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:478` `InsightIntentExecuteManager::UpdateFuncDecoratorParams` | `INSIGHT_INTENT_ARKTS_MODE` | info.arkTSMode | ❌否 | — |
| 132 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:521` `InsightIntentExecuteManager::UpdatePageDecoratorParams` | `INSIGHT_INTENT_SRC_ENTRANCE` | srcEntrance | ❌否 | ✅ |
| 133 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:541` `InsightIntentExecuteManager::UpdatePageDecoratorParams` | `INSIGHT_INTENT_PAGE_PARAM_PAGEPATH` | pagePath | ❌否 | — |
| 134 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:542` `InsightIntentExecuteManager::UpdatePageDecoratorParams` | `INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID` | navigationId | ❌否 | — |
| 135 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:543` `InsightIntentExecuteManager::UpdatePageDecoratorParams` | `INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME` | navDestinationName | ❌否 | — |
| 136 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:553` `InsightIntentExecuteManager::UpdateEntryDecoratorParams` | `INSIGHT_INTENT_SRC_ENTRANCE` | srcEntrance | ❌否 | — |
| 137 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:599` `InsightIntentExecuteManager::CheckAndUpdateDecoratorParams` | `INSIGHT_INTENT_DECORATOR_TYPE` | static_cast<int>(type) | ❌否 | ✅ |
| 138 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:636` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_PARAM` | pExecuteParams | ❌否 | — |
| 139 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:648` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_ARKTS_MODE` | arkTSMode | ❌否 | — |
| 140 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:651` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_DECORATOR_CLASS` | decoratorClass | ❌否 | — |
| 141 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:654` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_SRC_ENTRY` | srcEntry | ❌否 | — |
| 142 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:668` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_NAME` | param->insightIntentName_ | ❌否 | — |
| 143 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:669` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_MODE` | param->executeMode_ | ❌否 | — |
| 144 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:670` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_ID` | std::to_string(param->insightIntentId_) | ❌否 | — |
| 145 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:681` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_EXECUTE_PARAM_URI` | param->uris_ | ❌否 | — |
| 146 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1017` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_EXECUTE_PARAM_PARAM` | pExecuteParams | ❌否 | — |
| 147 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1021` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM` | pQueryEntityParams | ❌否 | — |
| 148 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1031` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_ARKTS_MODE` | intentInfo.arkTSMode | ❌否 | — |
| 149 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1032` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_SRC_ENTRANCE` | intentInfo.decoratorFile | ❌否 | — |
| 150 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1033` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_EXECUTE_PARAM_NAME` | param->intentName_ | ❌否 | — |
| 151 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1034` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_EXECUTE_PARAM_MODE` | static_cast<int32_t>(AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND) | ❌否 | — |
| 152 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1036` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_EXECUTE_PARAM_ID` | std::to_string(param->intentId_) | ❌否 | — |
| 153 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1037` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_DECORATOR_TYPE` | static_cast<int>(InsightIntentType::DECOR_QUERY_ENTITY) | ❌否 | — |
| 154 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1039` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME` | param->className_ | ❌否 | — |
| 155 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:1040` `InsightIntentExecuteManager::GenerateQueryEntityWant` | `INSIGHT_INTENT_QUERY_TYPE` | param->queryEntityParam_.queryType_ | ❌否 | — |
| 156 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:513` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_FLAGS` | OHOS::AAFwk::Integer::Box(flags) | ❌否 | — |
| 157 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:521` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_URIS` | uriArray | ❌否 | — |
| 158 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:525` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_RESULT` | OHOS::AAFwk::WantParamWrapper::Box(*result) | ❌否 | — |
| 159 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:530` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_INTERACTION_UI_TYPE` | OHOS::AAFwk::String::Box(interactionInfo->interactionUI->interactionUIType) | ❌否 | — |
| 160 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:536` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_BUNDLE_NAME` | OHOS::AAFwk::String::Box(modalUI->bundleName) | ❌否 | — |
| 161 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:537` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_ABILITY_NAME` | OHOS::AAFwk::String::Box(modalUI->abilityName) | ❌否 | — |
| 162 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:538` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_MODULE_NAME` | OHOS::AAFwk::String::Box(modalUI->moduleName) | ❌否 | — |
| 163 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:539` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_UI_EXTENSION_TYPE` | OHOS::AAFwk::String::Box(modalUI->uiExtensionType) | ❌否 | — |
| 164 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:540` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_URI` | OHOS::AAFwk::String::Box(modalUI->uri) | ❌否 | — |
| 165 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:542` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_PARAMETERS` | OHOS::AAFwk::WantParamWrapper::Box(*modalUI->parameters) | ❌否 | — |
| 166 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:546` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_INTERACTION_UI` | OHOS::AAFwk::WantParamWrapper::Box(*uiParams) | ❌否 | — |
| 167 | `services/abilitymgr/src/insight_intent/insight_intent_execute_result.cpp:547` `InsightIntentExecuteResult::BuildFunctionResult` | `KEY_INTERACTION_INFO` | OHOS::AAFwk::WantParamWrapper::Box(*infoParams) | ❌否 | — |
| 168 | `services/abilitymgr/src/interceptor/control_interceptor.cpp:48` `ControlInterceptor::DoProcess` | `INTERCEPT_PARAMETERS` | interceptParam | ❌否 | — |
| 169 | `services/abilitymgr/src/interceptor/control_interceptor.cpp:52` `ControlInterceptor::DoProcess` | `INTERCEPT_BUNDLE_NAME` | wantEle.GetBundleName() | ❌否 | — |
| 170 | `services/abilitymgr/src/interceptor/control_interceptor.cpp:53` `ControlInterceptor::DoProcess` | `INTERCEPT_ABILITY_NAME` | wantEle.GetAbilityName() | ❌否 | — |
| 171 | `services/abilitymgr/src/interceptor/control_interceptor.cpp:54` `ControlInterceptor::DoProcess` | `INTERCEPT_MODULE_NAME` | wantEle.GetModuleNameRef() | ❌否 | — |
| 172 | `services/abilitymgr/src/interceptor/crowd_test_interceptor.cpp:62` `CrowdTestInterceptor::DoProcess` | `UIEXTENSION_TYPE_KEY` | UIEXTENSION_SYS_COMMON_UI | ❌否 | — |
| 173 | `services/abilitymgr/src/interceptor/crowd_test_interceptor.cpp:64` `CrowdTestInterceptor::DoProcess` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 174 | `services/abilitymgr/src/interceptor/crowd_test_interceptor.cpp:65` `CrowdTestInterceptor::DoProcess` | `APP_BUNDLE_NAME` | param.want.GetBundleNameRef() | ❌否 | — |
| 175 | `services/abilitymgr/src/interceptor/disposed_rule_interceptor.cpp:299` `DisposedRuleInterceptor::CreateModalUIExtension` | `UIEXTENSION_MODAL_TYPE` | 1 | ❌否 | — |
| 176 | `services/abilitymgr/src/interceptor/disposed_rule_interceptor.cpp:313` `DisposedRuleInterceptor::SetInterceptInfo` | `INTERCEPT_BUNDLE_NAME` | want.GetBundleNameRef() | ❌否 | — |
| 177 | `services/abilitymgr/src/interceptor/disposed_rule_interceptor.cpp:314` `DisposedRuleInterceptor::SetInterceptInfo` | `INTERCEPT_ABILITY_NAME` | want.GetAbilityNameRef() | ❌否 | — |
| 178 | `services/abilitymgr/src/interceptor/disposed_rule_interceptor.cpp:315` `DisposedRuleInterceptor::SetInterceptInfo` | `INTERCEPT_MODULE_NAME` | want.GetModuleNameRef() | ❌否 | — |
| 179 | `services/abilitymgr/src/interceptor/ecological_rule_interceptor.cpp:77` `EcologicalRuleInterceptor::DoProcess` | `"queryWantFromErms"` | true | ❌否 | — |
| 180 | `services/abilitymgr/src/interceptor/ecological_rule_interceptor.cpp:193` `EcologicalRuleInterceptor::QueryAtomicServiceStartupRule` | `"queryWantFromErms"` | true | ❌否 | — |
| 181 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3675` `MissionListManager::NotifyStartSpecifiedAbility` | `"abilityType"` | type | ❌否 | — |
| 182 | `services/abilitymgr/src/pending_want_record.cpp:229` `PendingWantRecord::BuildSendWant` | `sendInfoWantParamKey` | mapIter->second | ❌否 | — |
| 183 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:242` `UIAbilityLifecycleManager::StartUIAbility` | `PARAM_DIALOG_SESSION_ID` | asCallerForAncoSessionId | ❌否 | — |
| 184 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:762` `UIAbilityLifecycleManager::NotifySCBToStartUIAbility` | `Want::PARAMS_REAL_CALLER_KEY` | callerKey | ❌否 | ✅ |
| 185 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:936` `UIAbilityLifecycleManager::HandleUIAbilityRequestList` | `Want::PARAMS_REAL_CALLER_KEY` | callerKey | ❌否 | ✅ |
| 186 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1944` `UIAbilityLifecycleManager::CallUIAbilityBySCB` | `AbilityRuntime::GlobalConstant::PAGE_CONFIG` | params.pageConfig | ❌否 | ✅ |
| 187 | `services/abilitymgr/src/skill/skill_execute_param.cpp:134` `SkillExecuteParam::GenerateFromWant` | `key` | it->second | ❌否 | — |
| 188 | `services/abilitymgr/src/skill/skill_execute_param.cpp:195` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_BUNDLE_NAME` | bundleName | ❌否 | — |
| 189 | `services/abilitymgr/src/skill/skill_execute_param.cpp:196` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_MODULE_NAME` | moduleName | ❌否 | — |
| 190 | `services/abilitymgr/src/skill/skill_execute_param.cpp:197` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_SKILL_NAME` | skillName | ❌否 | — |
| 191 | `services/abilitymgr/src/skill/skill_execute_param.cpp:199` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_SCRIPT_PATH` | scriptPath | ❌否 | — |
| 192 | `services/abilitymgr/src/skill/skill_execute_param.cpp:202` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_FUNCTION_NAME` | functionName | ❌否 | — |
| 193 | `services/abilitymgr/src/skill/skill_execute_param.cpp:211` `SkillExecuteParam::WriteToWant` | `wantKey` | value | ❌否 | — |
| 194 | `services/abilitymgr/src/skill/skill_execute_param.cpp:213` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_ARGS_KEYS` | AAFwk::String::Box(argsKeys) | ❌否 | — |
| 195 | `services/abilitymgr/src/skill/skill_execute_param.cpp:217` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT` | std::to_string(srcEntries.size()) | ❌否 | — |
| 196 | `services/abilitymgr/src/skill/skill_execute_param.cpp:220` `SkillExecuteParam::WriteToWant` | `key` | srcEntries[i] | ❌否 | — |
| 197 | `services/abilitymgr/src/skill/skill_execute_param.cpp:224` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_REQUEST_CODE` | requestCode | ❌否 | — |
| 198 | `services/abilitymgr/src/skill/skill_execute_param.cpp:227` `SkillExecuteParam::WriteToWant` | `SKILL_EXECUTE_PARAM_HAP_PATH` | hapPath | ❌否 | — |
| 199 | `services/abilitymgr/src/system_dialog_scheduler.cpp:129` `SystemDialogScheduler::GetTipsDialogWant` | `DIALOG_POSITION` | GetDialogPositionParams(position) | ❌否 | — |
| 200 | `services/abilitymgr/src/system_dialog_scheduler.cpp:130` `SystemDialogScheduler::GetTipsDialogWant` | `DIALOG_PARAMS` | params | ❌否 | — |
| 201 | `services/abilitymgr/src/system_dialog_scheduler.cpp:133` `SystemDialogScheduler::GetTipsDialogWant` | `CALLER_TOKEN` | callerToken | ❌否 | — |
| 202 | `services/abilitymgr/src/system_dialog_scheduler.cpp:289` `SystemDialogScheduler::GetSelectorDialogWant` | `DIALOG_POSITION` | GetDialogPositionParams(portraitPosition) | ❌否 | — |
| 203 | `services/abilitymgr/src/system_dialog_scheduler.cpp:290` `SystemDialogScheduler::GetSelectorDialogWant` | `VERTICAL_SCREEN_DIALOG_POSITION` | GetDialogPositionParams(landscapePosition) | ❌否 | — |
| 204 | `services/abilitymgr/src/system_dialog_scheduler.cpp:291` `SystemDialogScheduler::GetSelectorDialogWant` | `DIALOG_PARAMS` | params | ❌否 | — |
| 205 | `services/abilitymgr/src/system_dialog_scheduler.cpp:334` `SystemDialogScheduler::GetPcSelectorDialogWant` | `DIALOG_POSITION` | GetDialogPositionParams(position) | ❌否 | — |
| 206 | `services/abilitymgr/src/system_dialog_scheduler.cpp:335` `SystemDialogScheduler::GetPcSelectorDialogWant` | `DIALOG_PARAMS` | params | ❌否 | — |
| 207 | `services/abilitymgr/src/system_dialog_scheduler.cpp:393` `SystemDialogScheduler::GetSelectorDialogWantCommon` | `CALLER_TOKEN` | callerToken | ✅是 | — |
| 208 | `services/abilitymgr/src/system_dialog_scheduler.cpp:409` `SystemDialogScheduler::GetSelectorDialogWantCommon` | `UIEXTENSION_TYPE_KEY` | UIEXTENSION_SYS_COMMON_UI | ❌否 | — |
| 209 | `services/abilitymgr/src/system_dialog_scheduler.cpp:410` `SystemDialogScheduler::GetSelectorDialogWantCommon` | `"isCreateAppGallerySelector"` | true | ❌否 | — |
| 210 | `services/abilitymgr/src/system_dialog_scheduler.cpp:569` `SystemDialogScheduler::GetAssertFaultDialogWant` | `BUNDLE_NAME` | bundleName | ❌否 | ✅ |
| 211 | `services/abilitymgr/src/system_dialog_scheduler.cpp:570` `SystemDialogScheduler::GetAssertFaultDialogWant` | `UIEXTENSION_TYPE_KEY` | UIEXTENSION_SYS_COMMON_UI | ❌否 | ✅ |
| 212 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:986` `UIExtensionAbilityManager::UpdateUIExtensionInfo` | `UIEXTENSION_ABILITY_ID` | AAFwk::Integer::Box(uiExtensionAbilityId) | ❌否 | — |
| 213 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:991` `UIExtensionAbilityManager::UpdateUIExtensionInfo` | `UIEXTENSION_ROOT_HOST_PID` | AAFwk::Integer::Box(rootHostPid) | ❌否 | — |
| 214 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:996` `UIExtensionAbilityManager::UpdateUIExtensionInfo` | `UIEXTENSION_ROOT_HOST_PID` | AAFwk::Integer::Box(rootHostPid) | ❌否 | ✅ |
| 215 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:1031` `UIExtensionAbilityManager::UpdateUIExtensionBindInfo` | `UIEXTENSION_BIND_ABILITY_ID` | AAFwk::Integer::Box(uiExtensionBindAbilityId) | ❌否 | ✅ |
| 216 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:1032` `UIExtensionAbilityManager::UpdateUIExtensionBindInfo` | `UIEXTENSION_NOTIFY_BIND` | AAFwk::Integer::Box(notifyProcessBind) | ❌否 | ✅ |
| 217 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:1033` `UIExtensionAbilityManager::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_PID` | AAFwk::Integer::Box(IPCSkeleton::GetCallingPid()) | ❌否 | ✅ |
| 218 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:1034` `UIExtensionAbilityManager::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_UID` | AAFwk::Integer::Box(IPCSkeleton::GetCallingUid()) | ❌否 | ✅ |
| 219 | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp:1035` `UIExtensionAbilityManager::UpdateUIExtensionBindInfo` | `UIEXTENSION_HOST_BUNDLENAME` | String ::Box(callerBundleName) | ❌否 | ✅ |
| 220 | `services/abilitymgr/src/utils/agent_ability_util.cpp:75` `AgentAbilityUtil::SetAgentVerificationNonceParam` | `AgentRuntime::AGENT_VERIFICATION_NONCE_KEY` | Long::Box64(nonce) | ❌否 | — |
| 221 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:182` `UpdateCallerInfoUtil::UpdateSignatureInfo` | `callerAppId` | signatureInfo.appId | ✅是 | — |
| 222 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:184` `UpdateCallerInfoUtil::UpdateSignatureInfo` | `callerAppIdentifier` | signatureInfo.appIdentifier | ✅是 | — |
| 223 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:352` `UpdateCallerInfoUtil::UpdateBackToCallerFlag` | `CALLER_REQUEST_CODE` | std::to_string(fullRequestCode) | ✅是 | — |
| 224 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:381` `UpdateCallerInfoUtil::UpdateDmsCallerInfo` | `DMS_CALLER_NATIVE_NAME` | nativeName | ✅是 | ✅ |
| 225 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:384` `UpdateCallerInfoUtil::UpdateDmsCallerInfo` | `DMS_CALLER_BUNDLE_NAME` | bundleName | ✅是 | ✅ |
| 226 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:386` `UpdateCallerInfoUtil::UpdateDmsCallerInfo` | `DMS_CALLER_ABILITY_NAME` | std::string("") | ✅是 | ✅ |
| 227 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:391` `UpdateCallerInfoUtil::UpdateDmsCallerInfo` | `DMS_CALLER_BUNDLE_NAME` | callerBundleName | ✅是 | ✅ |
| 228 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:394` `UpdateCallerInfoUtil::UpdateDmsCallerInfo` | `DMS_CALLER_ABILITY_NAME` | callerAbilityName | ✅是 | ✅ |
| 229 | `services/abilitymgr/src/utils/uri_utils.cpp:131` `UriUtils::GetUriListFromWantDms` | `PARAMS_URI` | uriStrVec | ❌否 | — |
| 230 | `services/abilitymgr/src/utils/uri_utils.cpp:224` `UriUtils::GetPermissionedUriList` | `AbilityConfig::PARAMS_STREAM` | paramStreamUris | ✅是 | — |
| 231 | `services/abilitymgr/src/utils/uri_utils.cpp:246` `UriUtils::GetUriListFromWant` | `AbilityConfig::PARAMS_STREAM` | uriVec | ✅是 | — |
| 232 | `services/abilitymgr/src/utils/uri_utils.cpp:252` `UriUtils::GetUriListFromWant` | `AbilityConfig::PARAMS_STREAM` | uriVec | ✅是 | — |
| 233 | `services/abilitymgr/src/utils/uri_utils.cpp:510` `UriUtils::PublishFileOpenEvent` | `"uri"` | uriStr | ❌否 | — |
| 234 | `services/abilitymgr/src/utils/uri_utils.cpp:512` `UriUtils::PublishFileOpenEvent` | `"abilityName"` | grantInfo.targetAbilityName | ❌否 | — |
| 235 | `services/abilitymgr/src/utils/uri_utils.cpp:516` `UriUtils::PublishFileOpenEvent` | `"viewTime"` | currentTime | ❌否 | — |
| 236 | `services/abilitymgr/src/utils/uri_utils.cpp:517` `UriUtils::PublishFileOpenEvent` | `"targetUid"` | grantInfo.targetUid | ❌否 | — |
| 237 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:114` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `UIEXTENSION_TYPE_KEY` | UIEXTENSION_SYS_COMMON_UI | ❌否 | — |
| 238 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:115` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `APP_FREEZE_PID` | pid | ❌否 | — |
| 239 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:116` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `START_BUNDLE_NAME` | bundleName | ❌否 | — |
| 240 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:117` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `APP_FREEZE_EVENT_NAME` | faultData.errorObject.name | ❌否 | — |
| 241 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:118` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `APP_FREEZE_RUNNING_UNIQUE_ID` | faultData.appRunningUniqueId | ❌否 | — |
| 242 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:130` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `APP_FREEZE_TOKEN` | token | ❌否 | — |
| 243 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:157` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `FREEZE_WINDOW_POSX` | std::to_string(posX) | ❌否 | — |
| 244 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:158` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `FREEZE_WINDOW_POSY` | std::to_string(posY) | ❌否 | — |
| 245 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:159` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `FREEZE_WINDOW_WIDTH` | std::to_string(width) | ❌否 | — |
| 246 | `services/appmgr/src/modal_system_app_freeze_uiextension.cpp:160` `ModalSystemAppFreezeUIExtension::CreateSystemDialogWant` | `FREEZE_WINDOW_HEIGHT` | std::to_string(height) | ❌否 | — |
| 247 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:934` `DataObsMgrService::NotifyProcessObserver` | `"promptText"` | std::string(PROMPT_TEXT) | ❌否 | ✅ |
| 248 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:935` `DataObsMgrService::NotifyProcessObserver` | `"remoteDeviceName"` | std::string() | ❌否 | ✅ |
| 249 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:936` `DataObsMgrService::NotifyProcessObserver` | `"progressKey"` | key | ❌否 | ✅ |
| 250 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:937` `DataObsMgrService::NotifyProcessObserver` | `"isRemote"` | false | ❌否 | ✅ |
| 251 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:938` `DataObsMgrService::NotifyProcessObserver` | `"ipcCallback"` | observer | ❌否 | ✅ |
| 252 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:939` `DataObsMgrService::NotifyProcessObserver` | `"rectLeft"` | appInfo.left | ❌否 | ✅ |
| 253 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:940` `DataObsMgrService::NotifyProcessObserver` | `"rectTop"` | appInfo.top | ❌否 | ✅ |
| 254 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:941` `DataObsMgrService::NotifyProcessObserver` | `"rectWidth"` | static_cast<int32_t>(appInfo.width) | ❌否 | ✅ |
| 255 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:942` `DataObsMgrService::NotifyProcessObserver` | `"rectHeight"` | static_cast<int32_t>(appInfo.height) | ❌否 | ✅ |
| 256 | `services/dataobsmgr/src/dataobs_mgr_service.cpp:944` `DataObsMgrService::NotifyProcessObserver` | `"tokenKey"` | appInfo.abilityToken | ❌否 | ✅ |
| 257 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:630` `QuickFixManagerApplyTask::NotifyApplyStatus` | `APPLY_RESULT` | QuickFixErrorUtil::GetErrorCode(resultCode) | ❌否 | — |
| 258 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:631` `QuickFixManagerApplyTask::NotifyApplyStatus` | `APPLY_RESULT_INFO` | QuickFixErrorUtil::GetErrorMessage(resultCode) | ❌否 | — |
| 259 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:632` `QuickFixManagerApplyTask::NotifyApplyStatus` | `BUNDLE_VERSION` | bundleVersionCode_ | ❌否 | — |
| 260 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:633` `QuickFixManagerApplyTask::NotifyApplyStatus` | `PATCH_VERSION` | patchVersionCode_ | ❌否 | — |
| 261 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:642` `QuickFixManagerApplyTask::NotifyApplyStatus` | `REVOKE_RESULT` | QuickFixErrorUtil::GetErrorCode(resultCode) | ❌否 | — |
| 262 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:643` `QuickFixManagerApplyTask::NotifyApplyStatus` | `REVOKE_RESULT_INFO` | QuickFixErrorUtil::GetErrorMessage(resultCode) | ❌否 | — |
| 263 | `services/quickfixmgr/src/quick_fix_manager_apply_task.cpp:649` `QuickFixManagerApplyTask::NotifyApplyStatus` | `BUNDLE_NAME` | bundleName_ | ❌否 | — |

### 白名单外 key 统计

| key（归一化） | 处数 |
|---|---|
| `AGENTEXTENSIONHOSTPROXY_KEY` | 1 |
| `AGENTID_KEY` | 1 |
| `AGENT_CARD_TYPE_KEY` | 1 |
| `AGENT_VERIFICATION_NONCE_KEY` | 1 |
| `APPLY_RESULT` | 1 |
| `APPLY_RESULT_INFO` | 1 |
| `APP_BUNDLE_NAME` | 1 |
| `APP_FREEZE_EVENT_NAME` | 1 |
| `APP_FREEZE_PID` | 1 |
| `APP_FREEZE_RUNNING_UNIQUE_ID` | 1 |
| `APP_FREEZE_TOKEN` | 1 |
| `APP_LAUNCH_TRUSTLIST` | 1 |
| `APP_LINKING_ONLY` | 1 |
| `ASSERT_FAULT_DETAIL` | 1 |
| `ATOMIC_SERVICE_SHORT_LINK` | 3 |
| `BUNDLE_NAME` | 2 |
| `BUNDLE_VERSION` | 1 |
| `CALLER_REQUEST_CODE` | 1 |
| `CALLER_TOKEN` | 2 |
| `COMPONENT_STARTUP_NEW_RULES` | 1 |
| `DIALOG_PARAMS` | 3 |
| `DIALOG_POSITION` | 3 |
| `DIALOG_SESSION_ID` | 3 |
| `DLP_PARAMS_ABILITY_NAME` | 1 |
| `DLP_PARAMS_BUNDLE_NAME` | 1 |
| `DLP_PARAMS_MODULE_NAME` | 1 |
| `DMS_API_VERSION` | 1 |
| `DMS_CALLER_ABILITY_NAME` | 2 |
| `DMS_CALLER_BUNDLE_NAME` | 2 |
| `DMS_CALLER_NATIVE_NAME` | 1 |
| `DMS_IS_CALLER_BACKGROUND` | 1 |
| `DMS_MISSION_ID` | 4 |
| `DMS_SRC_NETWORK_ID` | 1 |
| `ERMS_ORIGINAL_TARGET` | 1 |
| `ERROR_INFO_ENHANCE` | 1 |
| `FREEZE_WINDOW_HEIGHT` | 1 |
| `FREEZE_WINDOW_POSX` | 1 |
| `FREEZE_WINDOW_POSY` | 1 |
| `FREEZE_WINDOW_WIDTH` | 1 |
| `INSIGHT_INTENT_ARKTS_MODE` | 4 |
| `INSIGHT_INTENT_DECORATOR_CLASS` | 1 |
| `INSIGHT_INTENT_DECORATOR_TYPE` | 3 |
| `INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG` | 1 |
| `INSIGHT_INTENT_EXECUTE_PARAM_ID` | 3 |
| `INSIGHT_INTENT_EXECUTE_PARAM_MODE` | 3 |
| `INSIGHT_INTENT_EXECUTE_PARAM_NAME` | 4 |
| `INSIGHT_INTENT_EXECUTE_PARAM_PARAM` | 2 |
| `INSIGHT_INTENT_EXECUTE_PARAM_URI` | 1 |
| `INSIGHT_INTENT_FUNC_PARAM_CLASSNAME` | 1 |
| `INSIGHT_INTENT_FUNC_PARAM_METHODNAME` | 1 |
| `INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS` | 1 |
| `INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE` | 1 |
| `INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME` | 1 |
| `INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID` | 1 |
| `INSIGHT_INTENT_PAGE_PARAM_PAGEPATH` | 1 |
| `INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME` | 1 |
| `INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM` | 1 |
| `INSIGHT_INTENT_QUERY_TYPE` | 1 |
| `INSIGHT_INTENT_SRC_ENTRANCE` | 5 |
| `INSIGHT_INTENT_SRC_ENTRY` | 3 |
| `INTERCEPT_ABILITY_NAME` | 2 |
| `INTERCEPT_BUNDLE_NAME` | 2 |
| `INTERCEPT_MISSION_ID` | 4 |
| `INTERCEPT_MODULE_NAME` | 2 |
| `INTERCEPT_PARAMETERS` | 1 |
| `KEY_ABILITY_NAME` | 1 |
| `KEY_BUNDLE_NAME` | 1 |
| `KEY_FLAGS` | 1 |
| `KEY_INTERACTION_INFO` | 1 |
| `KEY_INTERACTION_UI` | 1 |
| `KEY_INTERACTION_UI_TYPE` | 1 |
| `KEY_MISSION_ID` | 1 |
| `KEY_MODULE_NAME` | 1 |
| `KEY_PARAMETERS` | 1 |
| `KEY_REQUEST_ID` | 3 |
| `KEY_RESULT` | 1 |
| `KEY_SESSION_ID` | 2 |
| `KEY_UI_EXTENSION_TYPE` | 1 |
| `KEY_URI` | 1 |
| `KEY_URIS` | 1 |
| `MARKET_CROWD_TEST_BUNDLE_PARAM` | 1 |
| `NATIVE_DEBUG` | 1 |
| `PAGE_CONFIG` | 2 |
| `PARAMS_REAL_CALLER_KEY` | 2 |
| `PARAMS_STREAM` | 6 |
| `PARAMS_URI` | 1 |
| `PARAM_APP_AUTO_STARTUP_LAUNCH_REASON` | 1 |
| `PARAM_DIALOG_SESSION_ID` | 1 |
| `PATCH_VERSION` | 1 |
| `PERF_CMD` | 1 |
| `PICKER_ERMS_POLICY` | 1 |
| `REVOKE_RESULT` | 1 |
| `REVOKE_RESULT_INFO` | 1 |
| `SCREEN_MODE_KEY` | 1 |
| `SKILL_EXECUTE_PARAM_ARGS_KEYS` | 1 |
| `SKILL_EXECUTE_PARAM_BUNDLE_NAME` | 1 |
| `SKILL_EXECUTE_PARAM_FUNCTION_NAME` | 1 |
| `SKILL_EXECUTE_PARAM_HAP_PATH` | 1 |
| `SKILL_EXECUTE_PARAM_MODULE_NAME` | 1 |
| `SKILL_EXECUTE_PARAM_REQUEST_CODE` | 1 |
| `SKILL_EXECUTE_PARAM_SCRIPT_PATH` | 1 |
| `SKILL_EXECUTE_PARAM_SKILL_NAME` | 1 |
| `SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT` | 1 |
| `SPECIFIED_ABILITY_FLAG` | 1 |
| `START_ABILITY_WITH_WAIT_OBSERVER_ID_KEY` | 1 |
| `START_BUNDLE_NAME` | 1 |
| `SUPPORT_CLOSE_ON_BLUR` | 3 |
| `TOKEN_KEY` | 1 |
| `UIEXTENSION_ABILITY_ID` | 2 |
| `UIEXTENSION_BIND_ABILITY_ID` | 2 |
| `UIEXTENSION_HOST_BUNDLENAME` | 2 |
| `UIEXTENSION_HOST_PID` | 2 |
| `UIEXTENSION_HOST_UID` | 2 |
| `UIEXTENSION_LAUNCH_TIMESTAMP_HIGH` | 1 |
| `UIEXTENSION_LAUNCH_TIMESTAMP_LOW` | 1 |
| `UIEXTENSION_MODAL_TYPE` | 10 |
| `UIEXTENSION_NOTIFY_BIND` | 2 |
| `UIEXTENSION_ROOT_HOST_PID` | 3 |
| `UIEXTENSION_TYPE_KEY` | 6 |
| `UISERVICEHOSTPROXY_KEY` | 1 |
| `UI_EXTENSION_ROOT_TOKEN` | 1 |
| `VERTICAL_SCREEN_DIALOG_POSITION` | 1 |
| `WANT_PARAMS_AUTO_FILL_CMD_KEY` | 3 |
| `WANT_PARAMS_AUTO_FILL_POPUP_WINDOW_KEY` | 3 |
| `WANT_PARAMS_AUTO_FILL_TRIGGER_TYPE_KEY` | 1 |
| `WANT_PARAMS_AUTO_FILL_TYPE_KEY` | 2 |
| `WANT_PARAMS_CUSTOM_DATA_KEY` | 1 |
| `WANT_PARAMS_EXTENSION_TYPE_KEY` | 4 |
| `WANT_PARAMS_HOST_WINDOW_ID_KEY` | 3 |
| `WANT_PARAMS_VIEW_DATA_KEY` | 3 |
| `abilityName` | 1 |
| `abilityType` | 1 |
| `action` | 1 |
| `appselector.selectorType` | 2 |
| `callerAppId` | 1 |
| `callerAppIdentifier` | 1 |
| `deviceType` | 3 |
| `ecological.replaceWant` | 1 |
| `entities` | 1 |
| `ipcCallback` | 1 |
| `isCreateAppGallerySelector` | 1 |
| `isRemote` | 1 |
| `isSelector` | 1 |
| `key` | 2 |
| `mapInfo.paramMappingName` | 1 |
| `ohos.ability.params.showDefaultPicker` | 1 |
| `ohos.insightIntent.decoratorClass` | 1 |
| `progressKey` | 1 |
| `promptText` | 1 |
| `queryWantFromErms` | 2 |
| `rectHeight` | 1 |
| `rectLeft` | 1 |
| `rectTop` | 1 |
| `rectWidth` | 1 |
| `remoteDeviceName` | 1 |
| `sendInfoWantParamKey` | 1 |
| `send_to_erms_embedded` | 3 |
| `send_to_erms_targetLinkFeature` | 1 |
| `send_to_erms_targetLinkType` | 3 |
| `showCaller` | 1 |
| `show_default_appselect` | 1 |
| `targetUid` | 1 |
| `tokenKey` | 1 |
| `uri` | 2 |
| `viewTime` | 1 |
| `wantKey` | 1 |
| `wantType` | 1 |

## 六、app 侧框架 SET 点补充（103 处，app 进程内构建启动 Want）

范围：`frameworks/` 下 UIAbility/UIExtension/Service 等 Context 及 app 侧 Options 构建（js_ability_context、ets_ability_context、ui_extension 等，非测试）。
口径：与正文一致，仅统计**系统参数** key（`Want::PARAM_RESV_*`/`PARAM_*` 预留 key 及跨层控制 key `APP_LINKING_ONLY`、`PARAM_BACK_TO_OTHER_MISSION_STACK`、`UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN`）。
注：app 侧为客户端进程，`IPC 派生` 列通常不适用（`IPCSkeleton::GetCalling*` 在 app 进程内不可作调用方身份）；下列个别行因函数体出现该引用而标记 ✅，仅供定位。

### A. UIAbility Context（UIAbility 侧，app 进程）（32 处）

| # | 设置处(file:line / 函数) | 设置内容(key = value) | 设置前重置 |
|---|---|---|---|
| 1 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:941` `EtsAbilityContext::InheritWindowMode` | `AAFwk::Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 2 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:959` `EtsAbilityContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_RECENT` = true | ❌否 |
| 3 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:971` `EtsAbilityContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 4 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:1133` `EtsAbilityContext::StartAbilityForResultInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 5 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:1322` `EtsAbilityContext::HandleOpenLinkOptions` | `APP_LINKING_ONLY` = false | ❌否 |
| 6 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:1348` `EtsAbilityContext::OnOpenLink` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 7 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2134` `EtsAbilityContext::OnStartAbilityForResultWithAccountInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 8 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2250` `EtsAbilityContext::CreateOpenLinkTask` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 9 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2368` `EtsAbilityContext::OpenAtomicServiceInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 10 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:2404` `OnInstallFinished` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 11 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:3019` `EtsAbilityContext::OnStartAbilityWithAccount` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 12 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1738` `AbilityContextImpl::RestartAppWithWindow` | `Want::PARAM_APP_CLONE_INDEX_KEY` = abilityInfo_->appIndex | ❌否 |
| 13 | `frameworks/native/ability/native/ability_runtime/cj_ability_context.cpp:328` `CJAbilityContext::InheritWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 14 | `frameworks/native/ability/native/ability_runtime/cj_ability_context.cpp:445` `CJAbilityContext::CreateOpenLinkTask` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 15 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:818` `FFIAbilityContextOpenAtomicService` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 16 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:819` `FFIAbilityContextOpenAtomicService` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 17 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:835` `FFIAbilityContextOpenLink` | `APP_LINKING_ONLY` = false | ❌否 |
| 18 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:846` `FFIAbilityContextOpenLink` | `APP_LINKING_ONLY` = appLinkingOnly | ❌否 |
| 19 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:849` `FFIAbilityContextOpenLink` | `APP_LINKING_ONLY` = false | ❌否 |
| 20 | `frameworks/native/ability/native/ability_runtime/cj_ability_context_object.cpp:855` `FFIAbilityContextOpenLink` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 21 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:718` `JsAbilityContext::OnStartAbility` | `Want::PARAM_RESV_START_RECENT` = true | ❌否 |
| 22 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:724` `JsAbilityContext::OnStartAbility` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 23 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1015` `JsAbilityContext::CreateOpenLinkTask` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 24 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1079` `JsAbilityContext::OnOpenLink` | `AppExecFwk::APP_LINKING_ONLY` = false | ❌否 |
| 25 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1097` `JsAbilityContext::OnOpenLink` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 26 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1267` `JsAbilityContext::OnStartAbilityWithAccount` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 27 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1437` `JsAbilityContext::OnStartAbilityForResult` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 28 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1472` `JsAbilityContext::OnStartAbilityForResult` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 29 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:1522` `JsAbilityContext::OnStartAbilityForResultWithAccount` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 30 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:2391` `JsAbilityContext::InheritWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 31 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:3355` `JsAbilityContext::OpenAtomicServiceInner` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 32 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:3383` `JsAbilityContext::OpenAtomicServiceInner` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |

### B. UIExtension Context（UIExtension/Embedded 侧，app 进程）（47 处）

| # | 设置处(file:line / 函数) | 设置内容(key = value) | 设置前重置 |
|---|---|---|---|
| 33 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1066` `EtsUIExtensionContentSession::OnStartAbility` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 34 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1072` `EtsUIExtensionContentSession::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 35 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1137` `EtsUIExtensionContentSession::OnStartAbilityForResult` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 36 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1158` `EtsUIExtensionContentSession::OnStartAbilityForResult` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 37 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1212` `EtsUIExtensionContentSession::StartAbilityForResultDoTask` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 38 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_content_session.cpp:1366` `EtsUIExtensionContentSession::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |
| 39 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:449` `EtsUIExtensionContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 40 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:491` `EtsUIExtensionContext::OnStartAbilityForResult` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 41 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:533` `EtsUIExtensionContext::OnStartAbilityForResult` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 42 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:614` `EtsUIExtensionContext::OnConnectServiceExtensionAbility` | `UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN` = true | ✅是 |
| 43 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1028` `EtsUIExtensionContext::OnStartAbilityForResultAsCaller` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 44 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1063` `EtsUIExtensionContext::OnStartAbilityForResultAsCaller` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 45 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1131` `EtsUIExtensionContext::HandleOpenLinkOptions` | `APP_LINKING_ONLY` = false | ❌否 |
| 46 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1157` `EtsUIExtensionContext::OpenLinkInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 47 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1200` `EtsUIExtensionContext::CreateOpenLinkTask` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 48 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1264` `EtsUIExtensionContext::OpenAtomicServiceInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 49 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1304` `EtsUIExtensionContext::OpenAtomicServiceInner` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 50 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1663` `EtsUIExtensionContext::OnStartUIAbilitiesInSplitWindowMode` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 51 | `frameworks/ets/ani/ui_extension_ability/src/ets_ui_extension_context.cpp:1727` `EtsUIExtensionContext::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |
| 52 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_content_session.cpp:178` `CJUIExtensionContentSession::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |
| 53 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:88` `CJUIExtensionContextImpl::CheckStartAbilityInputParam` | `AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 54 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:107` `CJUIExtensionContextImpl::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |
| 55 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:171` `CJUIExtensionContextImpl::StartAbilityForRes` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 56 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:188` `CJUIExtensionContextImpl::StartAbilityForRes` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 57 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:299` `CJUIExtensionContextImpl::OpenAtomicService` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 58 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:308` `CJUIExtensionContextImpl::OpenAtomicService` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 59 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:343` `CJUIExtensionContextImpl::OpenLink` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 60 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:347` `CJUIExtensionContextImpl::OpenLink` | `AAFwk::Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 61 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:637` `UnwrapOpenLinkOptions` | `APP_LINKING_ONLY` = false | ❌否 |
| 62 | `frameworks/native/ability/native/ui_extension_base/cj_ui_extension_context.cpp:645` `UnwrapOpenLinkOptions` | `APP_LINKING_ONLY` = cOpenLinkOptions->appLinkingOnly | ❌否 |
| 63 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:256` `JsUIExtensionContentSession::OnStartAbility` | `Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 64 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:409` `JsUIExtensionContentSession::StartAbilityExecuteCallback` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 65 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:453` `JsUIExtensionContentSession::OnStartAbilityForResult` | `Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 66 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:473` `JsUIExtensionContentSession::OnStartAbilityForResult` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 67 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:523` `JsUIExtensionContentSession::StartAbilityForResultRuntimeTask` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 68 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp:1244` `JsUIExtensionContentSession::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |
| 69 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:338` `JsUIExtensionContext::CreateOpenLinkTask` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 70 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:423` `JsUIExtensionContext::OnOpenLink` | `AppExecFwk::APP_LINKING_ONLY` = false | ❌否 |
| 71 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:441` `JsUIExtensionContext::OnOpenLink` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 72 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:568` `JsUIExtensionContext::OnStartAbilityForResult` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 73 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:759` `JsUIExtensionContext::OnStartAbilityForResultAsCaller` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 74 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:866` `JsUIExtensionContext::CheckStartUIAbilitiesInSplitWindowModeInputParam` | `Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 75 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:942` `JsUIExtensionContext::OnConnectAbilityInner` | `UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN` = true | ✅是 |
| 76 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:1375` `JsUIExtensionContext::OpenAtomicServiceInner` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 77 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:1418` `JsUIExtensionContext::OpenAtomicServiceInner` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 78 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:1736` `JsUIExtensionContext::CheckStartAbilityInputParam` | `Want::PARAM_BACK_TO_OTHER_MISSION_STACK` = true | ❌否 |
| 79 | `frameworks/native/ability/native/ui_extension_base/js_ui_extension_context.cpp:1762` `JsUIExtensionContext::InitDisplayId` | `AAFwk::Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(window->GetDisplayId()) | ❌否 |

### C. 其他 Extension Context（Service/InsightIntent，app 进程）（15 处）

| # | 设置处(file:line / 函数) | 设置内容(key = value) | 设置前重置 |
|---|---|---|---|
| 80 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:805` `EtsServiceExtensionContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_RECENT` = true | ❌否 |
| 81 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:810` `EtsServiceExtensionContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 82 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:861` `EtsServiceExtensionContext::OnStartAbilityWithAccount` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 83 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:1591` `EtsServiceExtensionContext::OpenAtomicServiceInner` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 84 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:1765` `EtsServiceExtensionContext::OnOpenLink` | `APP_LINKING_ONLY` = false | ❌否 |
| 85 | `frameworks/ets/ani/service_extension_ability/src/ets_service_extension_context.cpp:1782` `EtsServiceExtensionContext::OnOpenLink` | `AAFwk::Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 86 | `frameworks/native/ability/native/js_service_extension_context.cpp:270` `OnStartAbility` | `Want::PARAM_RESV_START_RECENT` = true | ❌否 |
| 87 | `frameworks/native/ability/native/js_service_extension_context.cpp:276` `OnStartAbility` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 88 | `frameworks/native/ability/native/js_service_extension_context.cpp:369` `OnOpenLink` | `AppExecFwk::APP_LINKING_ONLY` = false | ❌否 |
| 89 | `frameworks/native/ability/native/js_service_extension_context.cpp:387` `OnOpenLink` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 90 | `frameworks/native/ability/native/js_service_extension_context.cpp:563` `OnOpenAtomicService` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 91 | `frameworks/native/ability/native/js_service_extension_context.cpp:859` `OnStartAbilityWithAccount` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 92 | `frameworks/native/insight_intent/insight_intent_context/cj_insight_intent_context.cpp:53` `CjInsightIntentContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 93 | `frameworks/native/insight_intent/insight_intent_context/ets_insight_intent_context.cpp:177` `EtsInsightIntentContext::StartAbilityInner` | `AAFwk::Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 94 | `frameworks/native/insight_intent/insight_intent_context/js_insight_intent_context.cpp:87` `JsInsightIntentContext::OnStartAbility` | `AAFwk::Want::PARAM_RESV_WINDOW_MODE` = context->GetCurrentWindowMode() | ❌否 |

### D. Options/公共构建与 FA（app 侧框架工具）（9 处）

| # | 设置处(file:line / 函数) | 设置内容(key = value) | 设置前重置 |
|---|---|---|---|
| 95 | `frameworks/ets/ani/ani_common/src/ani_common_start_options.cpp:374` `UnWrapOpenLinkOptions` | `std::string(APP_LINKING_ONLY)` = appLinkingOnly | ❌否 |
| 96 | `frameworks/ets/ani/ani_common/src/ani_common_start_options.cpp:377` `UnWrapOpenLinkOptions` | `std::string(APP_LINKING_ONLY)` = false | ❌否 |
| 97 | `frameworks/js/napi/feature_ability/js_feature_ability.cpp:165` `JsFeatureAbility::OnStartAbilityForResult` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |
| 98 | `frameworks/js/napi/inner/napi_ability_common/js_napi_common.cpp:1304` `JsNapiCommon::SetJsStartAbilityExecuteCallback` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 99 | `frameworks/js/napi/inner/napi_ability_common/js_napi_common.cpp:1354` `JsNapiCommon::JsStartAbility` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 |
| 100 | `frameworks/js/napi/inner/napi_common/open_link/napi_common_open_link_options.cpp:48` `UnwrapOpenLinkOptions` | `APP_LINKING_ONLY` = appLinkingOnly | ❌否 |
| 101 | `frameworks/js/napi/inner/napi_common/open_link/napi_common_open_link_options.cpp:51` `UnwrapOpenLinkOptions` | `APP_LINKING_ONLY` = false | ❌否 |
| 102 | `frameworks/native/ability/native/ability_process.cpp:78` `AbilityProcess::StartAbility` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 |
| 103 | `frameworks/native/ability/native/ability_process.cpp:86` `AbilityProcess::StartAbility` | `Want::PARAM_RESV_FOR_RESULT` = true | ❌否 |

## 七、app 侧 key 统计

| key（归一化） | 处数 | 备注 |
|---|---|---|
| `APP_LINKING_ONLY` | 15 | openLink App Linking 仅应用内打开 |
| `PARAM_APP_CLONE_INDEX_KEY` | 1 | 应用分身索引 |
| `PARAM_BACK_TO_OTHER_MISSION_STACK` | 10 | UIExtension 返回宿主任务栈 |
| `PARAM_RESV_DISPLAY_ID` | 6 | 目标显示 |
| `PARAM_RESV_FOR_RESULT` | 23 | startAbilityForResult 标记 |
| `PARAM_RESV_START_RECENT` | 4 | 加入最近任务 |
| `PARAM_RESV_START_TIME` | 34 | 启动时间戳，配合限频/冷启动统计 |
| `PARAM_RESV_WINDOW_MODE` | 8 | 窗口模式(启动选项) |
| `UI_EXTENSION_CONTEXT_TRANSFER_ROOT_HOST_TOKEN` | 2 | UIExtension 跨组件宿主 token 传递 |