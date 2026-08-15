# Want 系统参数 SET 点全量清单（补全版）

范围：ability_runtime `services/`+`interfaces/`（非测试/mock），当前 master 快照。
列：设置处(file:line / 所在函数) | 设置内容(key = value) | 设置前重置(同 key RemoveParam/Remove) | IPC 派生(值/函数体引用 `IPCSkeleton::GetCalling*`) | 附录状态

## 一、结论摘要

- 原附录 `set_sites_附录.md` 125 行：**全部可复现**（按 (文件, key) 匹配当前代码，行号因快照不同有漂移）。
- 当前 master 穷举结果：**228 处** SET（`SetParam` 491 次调用 → 去重后含 258 个 key，其中系统参数 SET 228 处 / 81 个归一化 key，原始 key 表达式 89 个）。
- **新增 103 处**（原附录未覆盖），其中：
  - 整类缺失（key 完全未出现在附录）共 **49 个 key**；
  - 已覆盖 key 内漏点共 17 处。
- 修正项：原附录 #8/#9 函数名误标为 `PermissionVerification::GetInstance`，实为 `StartAbilityUtils::RemoveAtomicServiceShareRouterIfNeeded`（已随 核对结果.md 修订）。

## 二、结构性遗漏（整类缺失 key）

| key（归一化） | 新增处数 | 典型设置处 |
|---|---|---|
| `ABILITY_OWNER_USERID` | 1 | `services/abilitymgr/src/ability_record.cpp:337` |
| `APP_INDEX` | 1 | `services/appmgr/src/app_mgr_service_inner.cpp:7121` |
| `APP_INSTANCE_KEY` | 5 | `services/abilitymgr/src/ability_manager_service.cpp:16352` |
| `CLI_CALLER_BUNDLE_NAME` | 2 | `services/abilitymgr/src/ability_manager_service.cpp:1607` |
| `CLI_CALLER_TOKEN_ID` | 2 | `services/abilitymgr/src/ability_manager_service.cpp:1609` |
| `CollaboratorRemoveCallerName` | 1 | `services/abilitymgr/include/collaborator/collaborator_util.h:56` |
| `DEBUG_APP` | 6 | `services/abilitymgr/src/ability_manager_service.cpp:8970` |
| `DLP_INDEX` | 3 | `services/abilitymgr/src/ability_manager_service.cpp:8962` |
| `DLP_PARAMS_SANDBOX` | 1 | `services/abilitymgr/src/ability_manager_client.cpp:1805` |
| `DLP_PARAMS_SECURITY_FLAG` | 3 | `services/abilitymgr/src/ability_manager_service.cpp:8963` |
| `DMS_CALLING_UID` | 2 | `services/abilitymgr/src/ability_manager_service.cpp:6721` |
| `FROM_REMOTE_KEY` | 2 | `services/abilitymgr/src/ability_manager_stub.cpp:3276` |
| `GAME_PRELAUNCH` | 2 | `services/abilitymgr/src/ability_manager_service.cpp:1776` |
| `HIDDEN_START_AUTOSTARTUP` | 1 | `services/abilitymgr/src/ability_auto_startup_service.cpp:667` |
| `INSIGHT_INTENT_PARAM_USER_ID` | 1 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:671` |
| `IS_CALLING_FROM_DMS` | 1 | `services/abilitymgr/src/utils/dms_util.cpp:38` |
| `IS_CALL_BY_SCB` | 10 | `services/abilitymgr/src/ability_manager_service.cpp:1759` |
| `IS_EMBEDDABLE_SERVICE` | 1 | `services/abilitymgr/src/disposed_observer.cpp:175` |
| `IS_HOOK` | 1 | `services/abilitymgr/src/ability_record.cpp:338` |
| `IS_PRELOAD_UIEXTENSION_ABILITY` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:4437` |
| `IS_SHELL_CALL` | 2 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:754` |
| `IS_WEB_SANDBOX_CLONE` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:734` |
| `KEY_IS_APP_RUNNING` | 1 | `services/abilitymgr/src/free_install_manager.cpp:779` |
| `KEY_SKIP_ABILITY_STAGE_LIFECYCLE` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:9016` |
| `KEY_SPECIFIED_FLAG` | 2 | `services/abilitymgr/src/ability_manager_service.cpp:18959` |
| `MULTI_THREAD` | 1 | `services/abilitymgr/src/ability_record.cpp:2130` |
| `NEED_STARTINGWINDOW` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:8940` |
| `PARAM_ASSERT_FAULT_SESSION_ID` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:16106` |
| `PARAM_FREEINSTALL_UID` | 1 | `services/abilitymgr/src/free_install_manager.cpp:141` |
| `PARAM_SPECIFIED_PROCESS_FLAG` | 2 | `services/abilitymgr/src/ability_connect_manager.cpp:1409` |
| `REUSING_WINDOW` | 1 | `services/appmgr/src/app_mgr_service_inner.cpp:12641` |
| `SANDBOX_CLONE_INDEX` | 2 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:736` |
| `SPECIFIED_PROCESS_CALLER_PROCESS` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3043` |
| `SPECIFY_TOKEN_ID` | 4 | `services/abilitymgr/src/ability_manager_service.cpp:1752` |
| `TARGET_UID_KEY` | 1 | `services/appmgr/src/app_mgr_service_inner.cpp:7140` |
| `TOKEN_ID` | 1 | `services/appmgr/src/app_mgr_service_inner.cpp:7136` |
| `UID` | 1 | `services/appmgr/src/app_mgr_service_inner.cpp:7138` |
| `UI_EXTENSION_TARGET_USER_ID` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:4598` |
| `USER_ID` | 2 | `services/appmgr/src/app_mgr_service_inner.cpp:7120` |
| `WANT_PARAMS_APP_RESTART_FLAG` | 1 | `services/abilitymgr/src/ability_connect_manager.cpp:2590` |
| `appId` | 1 | `services/abilitymgr/src/utils/ability_event_util.cpp:104` |
| `appIndex` | 1 | `services/abilitymgr/src/utils/ability_event_util.cpp:101` |
| `bundleName` | 3 | `services/abilitymgr/src/kiosk_manager.cpp:295` |
| `callerBundleName` | 1 | `services/abilitymgr/src/utils/ability_event_util.cpp:106` |
| `ohos.ability.params.isSkipErmsFromSCB` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1080` |
| `ohos.app.logout_recovery` | 1 | `services/abilitymgr/src/ability_manager_service.cpp:8946` |
| `ohos.app.recovery` | 1 | `services/abilitymgr/src/ability_connect_manager.cpp:2577` |
| `string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)` | 1 | `services/abilitymgr/src/ability_record.cpp:1483` |
| `string(PARAM_SEND_RESULT_CALLER_TOKENID)` | 1 | `services/abilitymgr/src/ability_record.cpp:1484` |

已覆盖 key 内的漏点（附录中该 key 已有行，但计数不足）合计 17 处：

| key（归一化） | 新增处数 | 典型设置处 |
|---|---|---|
| `PARAM_RESV_APP_REQUEST_ID` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:419` |
| `PARAM_RESV_CALLER_ABILITY_NAME` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5005` |
| `PARAM_RESV_CALLER_APP_CLONE_INDEX` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5007` |
| `PARAM_RESV_CALLER_APP_IDENTIFIER` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5012` |
| `PARAM_RESV_CALLER_BUNDLE_NAME` | 2 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:356` |
| `PARAM_RESV_CALLER_NATIVE_NAME` | 2 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5009` |
| `PARAM_RESV_CALLER_PID` | 1 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:352` |
| `PARAM_RESV_CALLER_TOKEN` | 1 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:344` |
| `PARAM_RESV_CALLER_UID` | 1 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:348` |
| `PARAM_RESV_DISPLAY_ID` | 3 | `services/abilitymgr/src/ability_manager_service.cpp:2697` |
| `PARAM_RESV_REQUEST_PROC_CODE` | 1 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:127` |
| `PARAM_RESV_REQUEST_TOKEN_CODE` | 1 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:129` |
| `PARAM_RESV_SCB_REQUEST_ID` | 1 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:423` |

## 三、全量 SET 清单（228 处，按文件/行号排序）

| # | 设置处(file:line / 函数) | 设置内容(key = value) | 设置前重置 | IPC 派生 | 附录状态 |
|---|---|---|---|---|---|
| 1 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:127` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_REQUEST_PROC_CODE` = want->GetIntParam(Want::PARAM_RESV_REQUEST_PROC_CODE, 0) | ❌否 | — | **新增** |
| 2 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:129` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_REQUEST_TOKEN_CODE` = want->GetIntParam(Want::PARAM_RESV_REQUEST_TOKEN_CODE, 0) | ❌否 | — | **新增** |
| 3 | `interfaces/inner_api/app_manager/src/appmgr/ability_info_callback_proxy.cpp:131` `AbilityInfoCallbackProxy::SetExtraParam` | `Want::PARAM_RESV_ABILITY_INFO_CALLBACK` = tempCallBack | ❌否 | — | 附录#1 |
| 4 | `services/abilitymgr/include/collaborator/collaborator_util.h:56` `HandleCallerIfNeed` | `"CollaboratorRemoveCallerName"` = callerBundleName | ❌否 | — | **新增** |
| 5 | `services/abilitymgr/include/utils/ability_util.h:261` `WantSetParameterWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 | — | 附录#2 |
| 6 | `services/abilitymgr/include/utils/ability_util.h:267` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 | — | 附录#3 |
| 7 | `services/abilitymgr/include/utils/ability_util.h:274` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 | ✅ | 附录#4 |
| 8 | `services/abilitymgr/include/utils/ability_util.h:277` `ProcessWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 | ✅ | 附录#5 |
| 9 | `services/abilitymgr/src/ability_auto_startup_service.cpp:667` `AbilityAutoStartupService::HiddenStartAutoStartupApp` | `HIDDEN_START_AUTOSTARTUP` = true | ❌否 | — | **新增** |
| 10 | `services/abilitymgr/src/ability_connect_manager.cpp:887` `AbilityConnectManager::ProcessPreload` | `"uid"` = uid | ❌否 | — | 附录#6 |
| 11 | `services/abilitymgr/src/ability_connect_manager.cpp:1409` `AbilityConnectManager::OnStartSpecifiedProcessResponse` | `PARAM_SPECIFIED_PROCESS_FLAG` = flag | ❌否 | — | **新增** |
| 12 | `services/abilitymgr/src/ability_connect_manager.cpp:2577` `AbilityConnectManager::RestartAbility` | `"ohos.app.recovery"` = true | ❌否 | — | **新增** |
| 13 | `services/abilitymgr/src/ability_connect_manager.cpp:2580` `AbilityConnectManager::RestartAbility` | `Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(displayId) | ❌否 | — | 附录#7 |
| 14 | `services/abilitymgr/src/ability_connect_manager.cpp:2590` `AbilityConnectManager::RestartAbility` | `WANT_PARAMS_APP_RESTART_FLAG` = true | ❌否 | — | **新增** |
| 15 | `services/abilitymgr/src/ability_manager_client.cpp:1805` `AbilityManagerClient::HandleDlpApp` | `DLP_PARAMS_SANDBOX` = sandboxFlag | ❌否 | — | **新增** |
| 16 | `services/abilitymgr/src/ability_manager_service.cpp:1607` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::CLI_CALLER_BUNDLE_NAME` = param.sandboxCloneParams->callerBundleName | ❌否 | — | **新增** |
| 17 | `services/abilitymgr/src/ability_manager_service.cpp:1609` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::CLI_CALLER_TOKEN_ID` = std::to_string(param.sandboxCloneParams->callerTokenId) | ❌否 | — | **新增** |
| 18 | `services/abilitymgr/src/ability_manager_service.cpp:1752` `AbilityManagerService::StartAbilityInner` | `SPECIFY_TOKEN_ID` = static_cast<int32_t>(param.specifyTokenId) | ✅是 | ✅ | **新增** |
| 19 | `services/abilitymgr/src/ability_manager_service.cpp:1759` `AbilityManagerService::StartAbilityInner` | `ServerConstant::IS_CALL_BY_SCB` = param.isBySCB | ❌否 | ✅ | **新增** |
| 20 | `services/abilitymgr/src/ability_manager_service.cpp:1771` `AbilityManagerService::StartAbilityInner` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = std::string("") | ❌否 | — | 附录#8 |
| 21 | `services/abilitymgr/src/ability_manager_service.cpp:1772` `AbilityManagerService::StartAbilityInner` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = std::string("") | ❌否 | — | 附录#9 |
| 22 | `services/abilitymgr/src/ability_manager_service.cpp:1776` `AbilityManagerService::StartAbilityInner` | `AbilityRuntime::GlobalConstant::GAME_PRELAUNCH` = param.isGamePrelaunch | ❌否 | — | **新增** |
| 23 | `services/abilitymgr/src/ability_manager_service.cpp:1962` `AbilityManagerService::PreprocessRequestParams` | `SPECIFY_TOKEN_ID` = static_cast<int32_t>(param.specifyTokenId) | ✅是 | — | **新增** |
| 24 | `services/abilitymgr/src/ability_manager_service.cpp:1976` `AbilityManagerService::ExecuteAbilityStart` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | — | **新增** |
| 25 | `services/abilitymgr/src/ability_manager_service.cpp:1977` `AbilityManagerService::ExecuteAbilityStart` | `AbilityRuntime::GlobalConstant::GAME_PRELAUNCH` = isGamePrelaunch | ❌否 | — | **新增** |
| 26 | `services/abilitymgr/src/ability_manager_service.cpp:2285` `AbilityManagerService::StartAbilityDetails` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | ✅ | **新增** |
| 27 | `services/abilitymgr/src/ability_manager_service.cpp:2552` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_WINDOW_FOCUSED` = windowFocused | ❌否 | — | 附录#10 |
| 28 | `services/abilitymgr/src/ability_manager_service.cpp:2557` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` = DisplayUtil::GetDefaultDisplayId() | ❌否 | — | 附录#11 |
| 29 | `services/abilitymgr/src/ability_manager_service.cpp:2560` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` = startOptions.GetDisplayID() | ❌否 | — | 附录#12 |
| 30 | `services/abilitymgr/src/ability_manager_service.cpp:2694` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` = DisplayUtil::GetDefaultDisplayId() | ❌否 | — | 附录#16 |
| 31 | `services/abilitymgr/src/ability_manager_service.cpp:2697` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` = startOptions.GetDisplayID() | ❌否 | — | **新增** |
| 32 | `services/abilitymgr/src/ability_manager_service.cpp:2712` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_WINDOW_FOCUSED` = windowfocused | ✅是 | — | 附录#13 |
| 33 | `services/abilitymgr/src/ability_manager_service.cpp:2751` `AbilityManagerService::StartAbilityForOptionInner` | `SPECIFY_TOKEN_ID` = static_cast<int32_t>(specifyTokenId) | ✅是 | — | **新增** |
| 34 | `services/abilitymgr/src/ability_manager_service.cpp:2759` `AbilityManagerService::StartAbilityForOptionInner` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | ✅ | **新增** |
| 35 | `services/abilitymgr/src/ability_manager_service.cpp:2784` `AbilityManagerService::StartAbilityForOptionInner` | `Want::PARAM_RESV_DISPLAY_ID` = DisplayUtil::GetDefaultDisplayId() | ❌否 | ✅ | **新增** |
| 36 | `services/abilitymgr/src/ability_manager_service.cpp:2934` `AbilityManagerService::StartUIAbilitiesInSplitWindowMode` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | — | **新增** |
| 37 | `services/abilitymgr/src/ability_manager_service.cpp:2980` `AbilityManagerService::StartUIAbilities` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | — | **新增** |
| 38 | `services/abilitymgr/src/ability_manager_service.cpp:4419` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | ✅ | 附录#14 |
| 39 | `services/abilitymgr/src/ability_manager_service.cpp:4421` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` = callerAppIndex | ❌否 | ✅ | 附录#15 |
| 40 | `services/abilitymgr/src/ability_manager_service.cpp:4437` `AbilityManagerService::PreloadUIExtensionAbilityInner` | `IS_PRELOAD_UIEXTENSION_ABILITY` = true | ❌否 | ✅ | **新增** |
| 41 | `services/abilitymgr/src/ability_manager_service.cpp:4598` `AbilityManagerService::RequestModalUIExtensionWithAccountInner` | `UI_EXTENSION_TARGET_USER_ID` = accountId | ❌否 | — | **新增** |
| 42 | `services/abilitymgr/src/ability_manager_service.cpp:6721` `AbilityManagerService::ContinueMission` | `DMS_CALLING_UID` = AAFwk::Integer::Box(callingUid) | ❌否 | ✅ | **新增** |
| 43 | `services/abilitymgr/src/ability_manager_service.cpp:6738` `AbilityManagerService::ContinueMission` | `DMS_CALLING_UID` = AAFwk::Integer::Box(callingUid) | ❌否 | ✅ | **新增** |
| 44 | `services/abilitymgr/src/ability_manager_service.cpp:8940` `AbilityManagerService::StartHighestPriorityAbility` | `NEED_STARTINGWINDOW` = false | ❌否 | — | **新增** |
| 45 | `services/abilitymgr/src/ability_manager_service.cpp:8941` `AbilityManagerService::StartHighestPriorityAbility` | `Want::PARAM_RESV_DISPLAY_ID` = static_cast<int32_t>(displayId) | ❌否 | — | **新增** |
| 46 | `services/abilitymgr/src/ability_manager_service.cpp:8946` `AbilityManagerService::StartHighestPriorityAbility` | `"ohos.app.logout_recovery"` = true | ❌否 | — | **新增** |
| 47 | `services/abilitymgr/src/ability_manager_service.cpp:8962` `AbilityManagerService::GenerateAbilityRequest` | `AbilityRuntime::ServerConstant::DLP_INDEX` = abilityRecord->GetAppIndex() | ❌否 | — | **新增** |
| 48 | `services/abilitymgr/src/ability_manager_service.cpp:8963` `AbilityManagerService::GenerateAbilityRequest` | `DLP_PARAMS_SECURITY_FLAG` = abilityRecord->GetSecurityFlag() | ❌否 | — | **新增** |
| 49 | `services/abilitymgr/src/ability_manager_service.cpp:8970` `AbilityManagerService::GenerateAbilityRequest` | `AbilityConfig::DEBUG_APP` = abilityRecord->IsDebugApp() | ❌否 | — | **新增** |
| 50 | `services/abilitymgr/src/ability_manager_service.cpp:8993` `AbilityManagerService::GenerateAbilityRequest` | `Want::PARAM_RESV_CALLER_TOKEN` = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()) | ❌否 | ✅ | 附录#17 |
| 51 | `services/abilitymgr/src/ability_manager_service.cpp:8994` `AbilityManagerService::GenerateAbilityRequest` | `Want::PARAM_RESV_CALLER_UID` = IPCSkeleton::GetCallingUid() | ❌否 | ✅ | 附录#18 |
| 52 | `services/abilitymgr/src/ability_manager_service.cpp:9016` `AbilityManagerService::GenerateAbilityRequest` | `KEY_SKIP_ABILITY_STAGE_LIFECYCLE` = abilityInfo->skipAbilityStageLifecycle | ❌否 | ✅ | **新增** |
| 53 | `services/abilitymgr/src/ability_manager_service.cpp:9095` `AbilityManagerService::GenerateExtensionAbilityRequest` | `Want::PARAM_APP_CLONE_INDEX_KEY` = abilityRecord->GetAppIndex() | ❌否 | — | 附录#19 |
| 54 | `services/abilitymgr/src/ability_manager_service.cpp:9097` `AbilityManagerService::GenerateExtensionAbilityRequest` | `Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | — | 附录#20 |
| 55 | `services/abilitymgr/src/ability_manager_service.cpp:9103` `AbilityManagerService::GenerateExtensionAbilityRequest` | `AbilityRuntime::ServerConstant::DLP_INDEX` = abilityRecord->GetAppIndex() | ❌否 | — | **新增** |
| 56 | `services/abilitymgr/src/ability_manager_service.cpp:9104` `AbilityManagerService::GenerateExtensionAbilityRequest` | `DLP_PARAMS_SECURITY_FLAG` = abilityRecord->GetSecurityFlag() | ❌否 | — | **新增** |
| 57 | `services/abilitymgr/src/ability_manager_service.cpp:10020` `AbilityManagerService::StartAutoStartupApps` | `Want::PARAM_APP_CLONE_INDEX_KEY` = info.appCloneIndex | ❌否 | — | 附录#21 |
| 58 | `services/abilitymgr/src/ability_manager_service.cpp:10552` `AbilityManagerService::StartAbilityByCallWithErrMsg` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | ✅ | **新增** |
| 59 | `services/abilitymgr/src/ability_manager_service.cpp:10603` `AbilityManagerService::StartAbilityForPrelaunch` | `Want::PARAM_APP_CLONE_INDEX_KEY` = 0 | ❌否 | ✅ | 附录#22 |
| 60 | `services/abilitymgr/src/ability_manager_service.cpp:10656` `AbilityManagerService::StartAbilityForPrelaunch` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | ✅ | **新增** |
| 61 | `services/abilitymgr/src/ability_manager_service.cpp:11397` `AbilityManagerService::ScheduleRecoverAbility` | `AAFwk::Want::PARAM_ABILITY_RECOVERY_RESTART` = true | ❌否 | — | 附录#23 |
| 62 | `services/abilitymgr/src/ability_manager_service.cpp:16106` `AbilityManagerService::RequestAssertFaultDialog` | `Want::PARAM_ASSERT_FAULT_SESSION_ID` = std::to_string(assertFaultSessionId) | ❌否 | ✅ | **新增** |
| 63 | `services/abilitymgr/src/ability_manager_service.cpp:16351` `AbilityManagerService::RestartApp` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = processInfo.appCloneIndex | ❌否 | ✅ | 附录#24 |
| 64 | `services/abilitymgr/src/ability_manager_service.cpp:16352` `AbilityManagerService::RestartApp` | `AAFwk::Want::APP_INSTANCE_KEY` = processInfo.instanceKey | ❌否 | ✅ | **新增** |
| 65 | `services/abilitymgr/src/ability_manager_service.cpp:16519` `AbilityManagerService::IsEmbeddedOpenAllowed` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerAbility->GetBundleName() | ❌否 | ✅ | 附录#25 |
| 66 | `services/abilitymgr/src/ability_manager_service.cpp:16566` `AbilityManagerService::QueryAtomicServiceStartupRule` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerAbility->GetBundleName() | ❌否 | ✅ | 附录#26 |
| 67 | `services/abilitymgr/src/ability_manager_service.cpp:17102` `AbilityManagerService::PreStartInner` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | — | **新增** |
| 68 | `services/abilitymgr/src/ability_manager_service.cpp:17311` `AbilityManagerService::StartUIAbilityByPreInstallInner` | `SPECIFY_TOKEN_ID` = static_cast<int32_t>(specifyTokenId) | ✅是 | — | **新增** |
| 69 | `services/abilitymgr/src/ability_manager_service.cpp:17432` `AbilityManagerService::IntentOpenLinkInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` = 0 | ❌否 | — | 附录#27 |
| 70 | `services/abilitymgr/src/ability_manager_service.cpp:17496` `AbilityManagerService::OpenLinkInner` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerBundleName | ❌否 | ✅ | 附录#28 |
| 71 | `services/abilitymgr/src/ability_manager_service.cpp:17901` `AbilityManagerService::StartSelfUIAbilityInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` = processInfo.appCloneIndex | ✅是 | ✅ | 附录#29 |
| 72 | `services/abilitymgr/src/ability_manager_service.cpp:17908` `AbilityManagerService::StartSelfUIAbilityInner` | `Want::APP_INSTANCE_KEY` = processInfo.instanceKey | ❌否 | ✅ | **新增** |
| 73 | `services/abilitymgr/src/ability_manager_service.cpp:18638` `AbilityManagerService::RestartSelfAtomicService` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 | ✅ | 附录#30 |
| 74 | `services/abilitymgr/src/ability_manager_service.cpp:18959` `AbilityManagerService::StartSelfUIAbilityInCurrentProcess` | `KEY_SPECIFIED_FLAG` = specifiedFlag | ❌否 | ✅ | **新增** |
| 75 | `services/abilitymgr/src/ability_manager_service.cpp:18989` `AbilityManagerService::StartSelfUIAbilityInChildProcess` | `KEY_SPECIFIED_FLAG` = specifiedFlag | ❌否 | ✅ | **新增** |
| 76 | `services/abilitymgr/src/ability_manager_service.cpp:19261` `AbilityManagerService::HandleAppDiedForRecovery` | `Want::PARAM_ABILITY_RECOVERY_RESTART` = true | ❌否 | — | 附录#31 |
| 77 | `services/abilitymgr/src/ability_manager_stub.cpp:3276` `AbilityManagerStub::FreeInstallAbilityFromRemoteInner` | `FROM_REMOTE_KEY` = true | ❌否 | — | **新增** |
| 78 | `services/abilitymgr/src/ability_record.cpp:337` `AbilityRecord::LoadAbility` | `ABILITY_OWNER_USERID` = ownerMissionUserId_ | ❌否 | — | **新增** |
| 79 | `services/abilitymgr/src/ability_record.cpp:338` `AbilityRecord::LoadAbility` | `IS_HOOK` = isHook_ | ❌否 | — | **新增** |
| 80 | `services/abilitymgr/src/ability_record.cpp:1483` `AbilityRecord::SaveResult` | `std::string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)` = abilityInfo_.applicationInfo.name | ❌否 | — | **新增** |
| 81 | `services/abilitymgr/src/ability_record.cpp:1484` `AbilityRecord::SaveResult` | `std::string(PARAM_SEND_RESULT_CALLER_TOKENID)` = static_cast<int32_t>( abilityInfo_.applicationInfo.accessTokenId) | ❌否 | — | **新增** |
| 82 | `services/abilitymgr/src/ability_record.cpp:2115` `AbilityRecord::SetWant` | `AbilityConfig::DEBUG_APP` = launchDebugInfo_.debugApp | ❌否 | — | **新增** |
| 83 | `services/abilitymgr/src/ability_record.cpp:2130` `AbilityRecord::SetWant` | `MULTI_THREAD` = true | ❌否 | — | **新增** |
| 84 | `services/abilitymgr/src/ability_record.cpp:2233` `AbilityRecord::SetWindowMode` | `Want::PARAM_RESV_WINDOW_MODE` = windowMode | ❌否 | — | 附录#32 |
| 85 | `services/abilitymgr/src/ability_record.cpp:2247` `AbilityRecord::UpdateRecoveryInfo` | `Want::PARAM_ABILITY_RECOVERY_RESTART` = true | ❌否 | — | 附录#33 |
| 86 | `services/abilitymgr/src/ability_record.cpp:2280` `AbilityRecord::EvaluateRecoveryLaunchReason` | `Want::PARAM_ABILITY_RECOVERY_RESTART` = false | ❌否 | — | 附录#34 |
| 87 | `services/abilitymgr/src/ability_record.cpp:2533` `AbilityRecord::SetWantAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | — | 附录#35 |
| 88 | `services/abilitymgr/src/ability_record.cpp:3241` `AbilityRecord::SetDebugAppByWaitingDebugFlag` | `AbilityConfig::DEBUG_APP` = true | ❌否 | — | **新增** |
| 89 | `services/abilitymgr/src/ability_record.cpp:3277` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` = -1 | ❌否 | — | 附录#36 |
| 90 | `services/abilitymgr/src/ability_record.cpp:3278` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_UID` = -1 | ❌否 | — | 附录#37 |
| 91 | `services/abilitymgr/src/ability_record.cpp:3279` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_PID` = -1 | ❌否 | — | 附录#38 |
| 92 | `services/abilitymgr/src/ability_record.cpp:3281` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = want.GetStringParam(DMS_CALLER_BUNDLE_NAME) | ❌否 | — | 附录#39 |
| 93 | `services/abilitymgr/src/ability_record.cpp:3283` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = want.GetStringParam(DMS_CALLER_ABILITY_NAME) | ❌否 | — | 附录#40 |
| 94 | `services/abilitymgr/src/ability_record.cpp:3285` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` = want.GetStringParam(DMS_CALLER_NATIVE_NAME) | ❌否 | — | 附录#41 |
| 95 | `services/abilitymgr/src/ability_record.cpp:3287` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_APP_ID` = want.GetStringParam(DMS_CALLER_APP_ID) | ❌否 | — | 附录#42 |
| 96 | `services/abilitymgr/src/ability_record.cpp:3289` `AbilityRecord::UpdateDmsCallerInfo` | `Want::PARAM_RESV_CALLER_APP_IDENTIFIER` = want.GetStringParam(DMS_CALLER_APP_IDENTIFIER) | ❌否 | — | 附录#43 |
| 97 | `services/abilitymgr/src/ability_record.cpp:3300` `AbilityRecord::SetDebugUIExtension` | `AbilityConfig::DEBUG_APP` = true | ❌否 | — | **新增** |
| 98 | `services/abilitymgr/src/ability_record.cpp:3308` `AbilityRecord::SetDebugApp` | `AbilityConfig::DEBUG_APP` = isDebugApp | ❌否 | — | **新增** |
| 99 | `services/abilitymgr/src/atomic_service_status_callback_stub.cpp:48` `AtomicServiceStatusCallbackStub::OnRemoteInstallFinishedInner` | `FROM_REMOTE_KEY` = true | ❌否 | — | **新增** |
| 100 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:308` `DialogSessionManager::SendDialogResult` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | — | 附录#44 |
| 101 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:315` `DialogSessionManager::SendDialogResult` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = 0 | ❌否 | — | 附录#45 |
| 102 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:404` `DialogSessionManager::CreateJumpModalDialog` | `"userId"` = AAFwk::Integer::Box(userId) | ❌否 | — | 附录#46 |
| 103 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:426` `DialogSessionManager::CreateImplicitSelectorModalDialog` | `"userId"` = userId | ❌否 | — | 附录#47 |
| 104 | `services/abilitymgr/src/dialog_session/dialog_session_manager.cpp:471` `DialogSessionManager::CreateCloneSelectorModalDialog` | `"userId"` = AAFwk::Integer::Box(userId) | ❌否 | — | 附录#48 |
| 105 | `services/abilitymgr/src/disposed_observer.cpp:175` `DisposedObserver::ExecuteUIExtension` | `IS_EMBEDDABLE_SERVICE` = true | ❌否 | — | **新增** |
| 106 | `services/abilitymgr/src/free_install_manager.cpp:141` `FreeInstallManager::StartFreeInstall` | `PARAM_FREEINSTALL_UID` = IPCSkeleton::GetCallingUid() | ❌否 | ✅ | **新增** |
| 107 | `services/abilitymgr/src/free_install_manager.cpp:779` `FreeInstallManager::SetAppRunningState` | `KEY_IS_APP_RUNNING` = isAppRunning | ❌否 | — | **新增** |
| 108 | `services/abilitymgr/src/implicit_start_processor.cpp:167` `ImplicitStartProcessor::ImplicitStartAbility` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | ✅ | 附录#49 |
| 109 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:660` `InsightIntentExecuteManager::GenerateWant` | `Want::PARAM_RESV_START_TIME` = startTime | ❌否 | — | 附录#50 |
| 110 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:671` `InsightIntentExecuteManager::GenerateWant` | `INSIGHT_INTENT_PARAM_USER_ID` = param->userId_ | ❌否 | — | **新增** |
| 111 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp:673` `InsightIntentExecuteManager::GenerateWant` | `Want::PARAM_RESV_DISPLAY_ID` = param->displayId_ | ❌否 | — | 附录#51 |
| 112 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:344` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_TOKEN` = AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_TOK... | ✅是 | — | **新增** |
| 113 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:348` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_UID` = AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_UID... | ✅是 | — | **新增** |
| 114 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:352` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_PID` = AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_PID... | ✅是 | — | **新增** |
| 115 | `services/abilitymgr/src/insight_intent/insight_intent_execute_param.cpp:356` `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo` | `AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME` = AAFwk::String::Box(wantParams.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_B... | ✅是 | — | **新增** |
| 116 | `services/abilitymgr/src/keep_alive/keep_alive_process_manager.cpp:155` `KeepAliveProcessManager::StartKeepAliveMainAbility` | `Want::PARAM_APP_CLONE_INDEX_KEY` = info.appCloneIndex | ❌否 | — | 附录#52 |
| 117 | `services/abilitymgr/src/keep_alive/keep_alive_process_manager.cpp:520` `KeepAliveProcessManager::StartKeepAliveAppServiceExtensionInner` | `Want::PARAM_APP_CLONE_INDEX_KEY` = info.appCloneIndex | ❌否 | — | 附录#53 |
| 118 | `services/abilitymgr/src/kiosk_manager.cpp:295` `KioskManager::NotifyKioskModeChanged` | `"bundleName"` = bundleName | ❌否 | — | **新增** |
| 119 | `services/abilitymgr/src/kiosk_manager.cpp:296` `KioskManager::NotifyKioskModeChanged` | `"uid"` = kioskBundleUid | ❌否 | — | 附录#54 |
| 120 | `services/abilitymgr/src/kiosk_manager.cpp:297` `KioskManager::NotifyKioskModeChanged` | `"userId"` = kioskBundleUid / BASE_USER_RANGE | ❌否 | — | 附录#55 |
| 121 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3079` `MissionListManager::ProcessPreload` | `"uid"` = uid | ❌否 | — | 附录#56 |
| 122 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3685` `MissionListManager::NotifyStartSpecifiedAbility` | `Want::PARAM_RESV_REQUEST_PROC_CODE` = procCode | ❌否 | — | 附录#57 |
| 123 | `services/abilitymgr/src/mission/mission_list_manager.cpp:3689` `MissionListManager::NotifyStartSpecifiedAbility` | `Want::PARAM_RESV_REQUEST_TOKEN_CODE` = tokenCode | ❌否 | — | 附录#58 |
| 124 | `services/abilitymgr/src/pending_want_record.cpp:241` `PendingWantRecord::BuildSendWant` | `Want::PARAM_APP_CLONE_INDEX_KEY` = Integer::Box(appIndex) | ❌否 | — | 附录#59 |
| 125 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:229` `UIAbilityLifecycleManager::StartUIAbility` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` = sessionInfo->requestId | ❌否 | — | 附录#60 |
| 126 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:232` `UIAbilityLifecycleManager::StartUIAbility` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` = sessionInfo->scbRequestId | ❌否 | — | 附录#61 |
| 127 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:412` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` = sessionInfo.requestId | ❌否 | — | 附录#62 |
| 128 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:415` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` = sessionInfo.scbRequestId | ❌否 | — | 附录#63 |
| 129 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:419` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_APP_REQUEST_ID` = sessionInfo.requestId | ❌否 | — | **新增** |
| 130 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:423` `UIAbilityLifecycleManager::HandleAbilityRecordReused` | `AAFwk::Want::PARAM_RESV_SCB_REQUEST_ID` = sessionInfo.scbRequestId | ❌否 | — | **新增** |
| 131 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:453` `UIAbilityLifecycleManager::GenerateAbilityRecord` | `Want::APP_INSTANCE_KEY` = sessionInfo->instanceKey | ❌否 | — | **新增** |
| 132 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:731` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::ServerConstant::DLP_INDEX` = abilityRequest.abilityInfo.applicationInfo.appIndex | ❌否 | ✅ | **新增** |
| 133 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:734` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::IS_WEB_SANDBOX_CLONE` = abilityRequest.isWebSandBoxClone | ❌否 | ✅ | **新增** |
| 134 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:736` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::SANDBOX_CLONE_INDEX` = abilityRequest.abilityInfo.applicationInfo.appIndex | ❌否 | ✅ | **新增** |
| 135 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:742` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::CLI_CALLER_BUNDLE_NAME` = callerBundleName | ❌否 | ✅ | **新增** |
| 136 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:743` `UIAbilityLifecycleManager::SetSandboxCloneParamsForSession` | `AbilityRuntime::GlobalConstant::CLI_CALLER_TOKEN_ID` = callerTokenId | ❌否 | ✅ | **新增** |
| 137 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:754` `UIAbilityLifecycleManager::NotifySCBToStartUIAbility` | `IS_SHELL_CALL` = AAFwk::PermissionVerification::GetInstance()->IsShellCall() | ❌否 | — | **新增** |
| 138 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:927` `UIAbilityLifecycleManager::HandleUIAbilityRequestList` | `IS_SHELL_CALL` = AAFwk::PermissionVerification::GetInstance()->IsShellCall() | ❌否 | — | **新增** |
| 139 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:1080` `UIAbilityLifecycleManager::NotifySCBToRecoveryAfterInterception` | `"ohos.ability.params.isSkipErmsFromSCB"` = true | ❌否 | — | **新增** |
| 140 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:2424` `UIAbilityLifecycleManager::BackToCallerAbilityWithResultLocked` | `ServerConstant::IS_CALL_BY_SCB` = false | ❌否 | — | **新增** |
| 141 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3040` `UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse` | `PARAM_SPECIFIED_PROCESS_FLAG` = flag | ❌否 | — | **新增** |
| 142 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3043` `UIAbilityLifecycleManager::OnStartSpecifiedProcessResponse` | `SPECIFIED_PROCESS_CALLER_PROCESS` = callerProcessName | ✅是 | — | **新增** |
| 143 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3135` `UIAbilityLifecycleManager::StartSpecifiedProcessRequest` | `Want::APP_INSTANCE_KEY` = instanceKey | ❌否 | — | **新增** |
| 144 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3921` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` = startOptions->GetDisplayID() | ❌否 | — | 附录#64 |
| 145 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3922` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_WINDOW_MODE` = startOptions->GetWindowMode() | ❌否 | — | 附录#65 |
| 146 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3924` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` = DisplayUtil::GetDefaultDisplayId() | ❌否 | — | 附录#66 |
| 147 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:3927` `UIAbilityLifecycleManager::MoveMissionToFront` | `Want::PARAM_RESV_DISPLAY_ID` = -1 | ❌否 | — | 附录#67 |
| 148 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5000` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` = realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_TOKEN, 0) | ❌否 | — | 附录#68 |
| 149 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5001` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_UID` = realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_UID, 0) | ❌否 | — | 附录#69 |
| 150 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5002` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_PID` = realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_PID, 0) | ❌否 | — | 附录#70 |
| 151 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5003` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) | ❌否 | — | **新增** |
| 152 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5005` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_ABILITY_NAME) | ❌否 | — | **新增** |
| 153 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5007` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` = realCallerWant.GetIntParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, 0) | ❌否 | — | **新增** |
| 154 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5009` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` = realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME) | ❌否 | — | **新增** |
| 155 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5011` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_ID` = realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_ID) | ❌否 | — | 附录#71 |
| 156 | `services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp:5012` `UIAbilityLifecycleManager::UpdateTokenIdAndWantWithRealCallerInfo` | `Want::PARAM_RESV_CALLER_APP_IDENTIFIER` = realCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_APP_IDENTIFIER) | ❌否 | — | **新增** |
| 157 | `services/abilitymgr/src/utils/ability_event_util.cpp:99` `AbilityEventUtil::HandleBundleFirstLaunch` | `"bundleName"` = appInfo.bundleName | ❌否 | — | **新增** |
| 158 | `services/abilitymgr/src/utils/ability_event_util.cpp:100` `AbilityEventUtil::HandleBundleFirstLaunch` | `"userId"` = userId | ❌否 | — | 附录#72 |
| 159 | `services/abilitymgr/src/utils/ability_event_util.cpp:101` `AbilityEventUtil::HandleBundleFirstLaunch` | `"appIndex"` = appInfo.appIndex | ❌否 | — | **新增** |
| 160 | `services/abilitymgr/src/utils/ability_event_util.cpp:102` `AbilityEventUtil::HandleBundleFirstLaunch` | `"uid"` = appInfo.uid | ❌否 | — | 附录#73 |
| 161 | `services/abilitymgr/src/utils/ability_event_util.cpp:104` `AbilityEventUtil::HandleBundleFirstLaunch` | `"appId"` = appId | ❌否 | — | **新增** |
| 162 | `services/abilitymgr/src/utils/ability_event_util.cpp:106` `AbilityEventUtil::HandleBundleFirstLaunch` | `"callerBundleName"` = callerBundleName | ❌否 | — | **新增** |
| 163 | `services/abilitymgr/src/utils/ability_permission_util.cpp:214` `AbilityPermissionUtil::UpdateInstanceKey` | `Want::APP_INSTANCE_KEY` = instanceKey | ❌否 | — | **新增** |
| 164 | `services/abilitymgr/src/utils/clone_for_account_util.cpp:42` `CloneForAccountUtil::ProcessAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` = abilityInfo.appIndex | ✅是 | — | 附录#74 |
| 165 | `services/abilitymgr/src/utils/clone_for_account_util.cpp:53` `CloneForAccountUtil::ProcessAppIndex` | `Want::PARAM_APP_CLONE_INDEX_KEY` = extensionInfo.appIndex | ✅是 | — | 附录#75 |
| 166 | `services/abilitymgr/src/utils/dms_util.cpp:38` `DmsUtil::UpdateFlagForCollaboration` | `IS_CALLING_FROM_DMS` = true | ❌否 | ✅ | **新增** |
| 167 | `services/abilitymgr/src/utils/oe_extension_utils.cpp:86` `OEExtensionUtils::ValidateCaller` | `Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | — | 附录#76 |
| 168 | `services/abilitymgr/src/utils/start_ability_utils.cpp:161` `StartAbilityInfoWrap::StartAbilityInfoWrap` | `Want::PARAM_RESV_CALLER_TOKEN` = static_cast<int32_t>(IPCSkeleton::GetCallingTokenID()) | ✅是 | ✅ | 附录#77 |
| 169 | `services/abilitymgr/src/utils/start_ability_utils.cpp:162` `StartAbilityInfoWrap::StartAbilityInfoWrap` | `Want::PARAM_RESV_CALLER_UID` = IPCSkeleton::GetCallingUid() | ✅是 | ✅ | 附录#78 |
| 170 | `services/abilitymgr/src/utils/start_ability_utils.cpp:451` `StartAbilityUtils::SetTargetCloneIndexInSameBundle` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | ✅ | 附录#79 |
| 171 | `services/abilitymgr/src/utils/start_ability_utils.cpp:453` `StartAbilityUtils::SetTargetCloneIndexInSameBundle` | `AbilityRuntime::GlobalConstant::SANDBOX_CLONE_INDEX` = appIndex | ❌否 | ✅ | **新增** |
| 172 | `services/abilitymgr/src/utils/start_ability_utils.cpp:473` `StartAbilityUtils::ResolveTargetAppCloneIndex` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = appIndex | ❌否 | — | 附录#80 |
| 173 | `services/abilitymgr/src/utils/start_ability_utils.cpp:482` `StartAbilityUtils::StartUIAbilitiesProcessAppIndex` | `AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY` = 0 | ❌否 | — | 附录#81 |
| 174 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:88` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_TOKEN` = tokenId | ✅是 | ✅ | 附录#82 |
| 175 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:90` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_UID` = callerUid | ✅是 | ✅ | 附录#83 |
| 176 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:92` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_PID` = callerPid | ✅是 | ✅ | 附录#84 |
| 177 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:111` `UpdateCallerInfoUtil::UpdateCallerInfo` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` = nativeName | ❌否 | ✅ | 附录#85 |
| 178 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:145` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_TOKEN` = static_cast<int32_t>(callerTokenId) | ✅是 | — | 附录#86 |
| 179 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:152` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_UID` = uid | ✅是 | — | 附录#87 |
| 180 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:161` `UpdateCallerInfoUtil::UpdateCallerInfoByHapTokenId` | `Want::PARAM_RESV_CALLER_PID` = 0 | ✅是 | — | 附录#88 |
| 181 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:231` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_TOKEN` = tokenId | ❌否 | — | 附录#89 |
| 182 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:232` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_UID` = processInfo.uid_ | ❌否 | — | 附录#90 |
| 183 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:233` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_PID` = processInfo.pid_ | ❌否 | — | 附录#91 |
| 184 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:236` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerBundleName | ❌否 | — | 附录#92 |
| 185 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:238` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = callerAbilityName | ❌否 | — | 附录#93 |
| 186 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:250` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_TOKEN` = sourceInfo->callerTokenId | ❌否 | — | 附录#94 |
| 187 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:251` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_UID` = sourceInfo->callerUid | ❌否 | — | 附录#95 |
| 188 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:252` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_PID` = sourceInfo->callerPid | ❌否 | — | 附录#96 |
| 189 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:253` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerBundleName | ❌否 | — | 附录#97 |
| 190 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:254` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = sourceInfo->callerAbilityName | ❌否 | — | 附录#98 |
| 191 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:255` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` = sourceInfo->callerAppCloneIndex | ❌否 | — | 附录#99 |
| 192 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:257` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromCallerRecord` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` = sourceInfo->callerNativeName | ❌否 | — | 附录#100 |
| 193 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:286` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_TOKEN` = tokenId | ❌否 | ✅ | 附录#101 |
| 194 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:287` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_UID` = uid | ❌否 | ✅ | 附录#102 |
| 195 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:288` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_PID` = pid | ❌否 | ✅ | 附录#103 |
| 196 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:289` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerBundleName | ❌否 | ✅ | 附录#104 |
| 197 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:290` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = callerAbilityName | ❌否 | ✅ | 附录#105 |
| 198 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:291` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` = callerAppCloneIndex | ❌否 | ✅ | 附录#106 |
| 199 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:295` `UpdateCallerInfoUtil::UpdateAsCallerInfoFromDialog` | `Want::PARAM_RESV_CALLER_NATIVE_NAME` = dialogCallerWant.GetStringParam(Want::PARAM_RESV_CALLER_NATIVE_NAME) | ❌否 | ✅ | **新增** |
| 200 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:315` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_TOKEN` = tokenId | ✅是 | — | 附录#107 |
| 201 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:317` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_UID` = callerUid | ✅是 | — | 附录#108 |
| 202 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:319` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_PID` = callerPid | ✅是 | — | 附录#109 |
| 203 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:326` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = callerBundleName | ✅是 | — | 附录#110 |
| 204 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:329` `UpdateCallerInfoUtil::UpdateCallerInfoFromToken` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = callerAbilityName | ✅是 | — | 附录#111 |
| 205 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:401` `UpdateCallerInfoUtil::UpdateCallerBundleName` | `Want::PARAM_RESV_CALLER_BUNDLE_NAME` = bundleName | ✅是 | ✅ | 附录#112 |
| 206 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:407` `UpdateCallerInfoUtil::UpdateCallerAbilityName` | `Want::PARAM_RESV_CALLER_ABILITY_NAME` = abilityName | ✅是 | — | 附录#113 |
| 207 | `services/abilitymgr/src/utils/update_caller_info_util.cpp:413` `UpdateCallerInfoUtil::UpdateCallerAppCloneIndex` | `Want::PARAM_RESV_CALLER_APP_CLONE_INDEX` = appIndex | ✅是 | — | 附录#114 |
| 208 | `services/abilitymgr/src/utils/uri_utils.cpp:511` `UriUtils::PublishFileOpenEvent` | `"bundleName"` = grantInfo.targetBundleName | ❌否 | — | **新增** |
| 209 | `services/abilitymgr/src/utils/uri_utils.cpp:518` `UriUtils::PublishFileOpenEvent` | `"callerUid"` = grantInfo.callerUid | ❌否 | — | 附录#115 |
| 210 | `services/abilitymgr/src/utils/window_options_utils.cpp:30` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_LEFT` = startOptions.GetWindowLeft() | ❌否 | — | 附录#116 |
| 211 | `services/abilitymgr/src/utils/window_options_utils.cpp:33` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_TOP` = startOptions.GetWindowTop() | ❌否 | — | 附录#117 |
| 212 | `services/abilitymgr/src/utils/window_options_utils.cpp:36` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_WIDTH` = startOptions.GetWindowWidth() | ❌否 | — | 附录#118 |
| 213 | `services/abilitymgr/src/utils/window_options_utils.cpp:39` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WINDOW_HEIGHT` = startOptions.GetWindowHeight() | ❌否 | — | 附录#119 |
| 214 | `services/abilitymgr/src/utils/window_options_utils.cpp:42` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MIN_WINDOW_WIDTH` = startOptions.GetMinWindowWidth() | ❌否 | — | 附录#120 |
| 215 | `services/abilitymgr/src/utils/window_options_utils.cpp:45` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MIN_WINDOW_HEIGHT` = startOptions.GetMinWindowHeight() | ❌否 | — | 附录#121 |
| 216 | `services/abilitymgr/src/utils/window_options_utils.cpp:48` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MAX_WINDOW_WIDTH` = startOptions.GetMaxWindowWidth() | ❌否 | — | 附录#122 |
| 217 | `services/abilitymgr/src/utils/window_options_utils.cpp:51` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_MAX_WINDOW_HEIGHT` = startOptions.GetMaxWindowHeight() | ❌否 | — | 附录#123 |
| 218 | `services/abilitymgr/src/utils/window_options_utils.cpp:57` `WindowOptionsUtils::SetWindowPositionAndSize` | `Want::PARAM_RESV_WITH_ANIMATION` = withAnimation | ❌否 | — | 附录#124 |
| 219 | `services/appmgr/src/app_mgr_service_inner.cpp:1786` `AppMgrServiceInner::SetPreloadDebugApp` | `DEBUG_APP` = true | ❌否 | — | **新增** |
| 220 | `services/appmgr/src/app_mgr_service_inner.cpp:5316` `AppMgrServiceInner::StartAbility` | `DLP_PARAMS_SECURITY_FLAG` = appRecord->GetSecurityFlag() | ❌否 | — | **新增** |
| 221 | `services/appmgr/src/app_mgr_service_inner.cpp:7120` `AppMgrServiceInner::NotifyAppStatus` | `Constants::USER_ID` = 0 | ❌否 | — | **新增** |
| 222 | `services/appmgr/src/app_mgr_service_inner.cpp:7121` `AppMgrServiceInner::NotifyAppStatus` | `Constants::APP_INDEX` = appIndex | ❌否 | — | **新增** |
| 223 | `services/appmgr/src/app_mgr_service_inner.cpp:7136` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `TOKEN_ID` = tokenId | ❌否 | — | **新增** |
| 224 | `services/appmgr/src/app_mgr_service_inner.cpp:7137` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Constants::USER_ID` = userId | ❌否 | — | **新增** |
| 225 | `services/appmgr/src/app_mgr_service_inner.cpp:7138` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Constants::UID` = callerUid | ❌否 | — | **新增** |
| 226 | `services/appmgr/src/app_mgr_service_inner.cpp:7139` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `Want::PARAM_RESV_CALLER_UID` = callerUid | ❌否 | — | 附录#125 |
| 227 | `services/appmgr/src/app_mgr_service_inner.cpp:7140` `AppMgrServiceInner::NotifyAppStatusByCallerUid` | `TARGET_UID_KEY` = targetUid | ❌否 | — | **新增** |
| 228 | `services/appmgr/src/app_mgr_service_inner.cpp:12641` `AppMgrServiceInner::NotifyAppFaultBySA` | `REUSING_WINDOW` = true | ❌否 | — | **新增** |

## 四、按 key 统计（228 处）

| key（归一化） | 总数 | 新增 |
|---|---|---|
| `ABILITY_OWNER_USERID` | 1 | 1 |
| `APP_INDEX` | 1 | 1 |
| `APP_INSTANCE_KEY` | 5 | 5 |
| `CLI_CALLER_BUNDLE_NAME` | 2 | 2 |
| `CLI_CALLER_TOKEN_ID` | 2 | 2 |
| `CollaboratorRemoveCallerName` | 1 | 1 |
| `DEBUG_APP` | 6 | 6 |
| `DLP_INDEX` | 3 | 3 |
| `DLP_PARAMS_SANDBOX` | 1 | 1 |
| `DLP_PARAMS_SECURITY_FLAG` | 3 | 3 |
| `DMS_CALLING_UID` | 2 | 2 |
| `FROM_REMOTE_KEY` | 2 | 2 |
| `GAME_PRELAUNCH` | 2 | 2 |
| `HIDDEN_START_AUTOSTARTUP` | 1 | 1 |
| `INSIGHT_INTENT_PARAM_USER_ID` | 1 | 1 |
| `IS_CALLING_FROM_DMS` | 1 | 1 |
| `IS_CALL_BY_SCB` | 10 | 10 |
| `IS_EMBEDDABLE_SERVICE` | 1 | 1 |
| `IS_HOOK` | 1 | 1 |
| `IS_PRELOAD_UIEXTENSION_ABILITY` | 1 | 1 |
| `IS_SHELL_CALL` | 2 | 2 |
| `IS_WEB_SANDBOX_CLONE` | 1 | 1 |
| `KEY_IS_APP_RUNNING` | 1 | 1 |
| `KEY_SKIP_ABILITY_STAGE_LIFECYCLE` | 1 | 1 |
| `KEY_SPECIFIED_FLAG` | 2 | 2 |
| `MULTI_THREAD` | 1 | 1 |
| `NEED_STARTINGWINDOW` | 1 | 1 |
| `PARAM_ABILITY_RECOVERY_RESTART` | 4 | 0 |
| `PARAM_APP_CLONE_INDEX_KEY` | 22 | 0 |
| `PARAM_ASSERT_FAULT_SESSION_ID` | 1 | 1 |
| `PARAM_FREEINSTALL_UID` | 1 | 1 |
| `PARAM_RESV_ABILITY_INFO_CALLBACK` | 1 | 0 |
| `PARAM_RESV_APP_REQUEST_ID` | 3 | 1 |
| `PARAM_RESV_CALLER_ABILITY_NAME` | 8 | 1 |
| `PARAM_RESV_CALLER_APP_CLONE_INDEX` | 4 | 1 |
| `PARAM_RESV_CALLER_APP_ID` | 2 | 0 |
| `PARAM_RESV_CALLER_APP_IDENTIFIER` | 2 | 1 |
| `PARAM_RESV_CALLER_BUNDLE_NAME` | 12 | 2 |
| `PARAM_RESV_CALLER_NATIVE_NAME` | 5 | 2 |
| `PARAM_RESV_CALLER_PID` | 9 | 1 |
| `PARAM_RESV_CALLER_TOKEN` | 11 | 1 |
| `PARAM_RESV_CALLER_UID` | 12 | 1 |
| `PARAM_RESV_DISPLAY_ID` | 11 | 3 |
| `PARAM_RESV_MAX_WINDOW_HEIGHT` | 1 | 0 |
| `PARAM_RESV_MAX_WINDOW_WIDTH` | 1 | 0 |
| `PARAM_RESV_MIN_WINDOW_HEIGHT` | 1 | 0 |
| `PARAM_RESV_MIN_WINDOW_WIDTH` | 1 | 0 |
| `PARAM_RESV_REQUEST_PROC_CODE` | 2 | 1 |
| `PARAM_RESV_REQUEST_TOKEN_CODE` | 2 | 1 |
| `PARAM_RESV_SCB_REQUEST_ID` | 3 | 1 |
| `PARAM_RESV_START_TIME` | 2 | 0 |
| `PARAM_RESV_WINDOW_FOCUSED` | 2 | 0 |
| `PARAM_RESV_WINDOW_HEIGHT` | 1 | 0 |
| `PARAM_RESV_WINDOW_LEFT` | 1 | 0 |
| `PARAM_RESV_WINDOW_MODE` | 6 | 0 |
| `PARAM_RESV_WINDOW_TOP` | 1 | 0 |
| `PARAM_RESV_WINDOW_WIDTH` | 1 | 0 |
| `PARAM_RESV_WITH_ANIMATION` | 1 | 0 |
| `PARAM_SPECIFIED_PROCESS_FLAG` | 2 | 2 |
| `REUSING_WINDOW` | 1 | 1 |
| `SANDBOX_CLONE_INDEX` | 2 | 2 |
| `SPECIFIED_PROCESS_CALLER_PROCESS` | 1 | 1 |
| `SPECIFY_TOKEN_ID` | 4 | 4 |
| `TARGET_UID_KEY` | 1 | 1 |
| `TOKEN_ID` | 1 | 1 |
| `UID` | 1 | 1 |
| `UI_EXTENSION_TARGET_USER_ID` | 1 | 1 |
| `USER_ID` | 2 | 2 |
| `WANT_PARAMS_APP_RESTART_FLAG` | 1 | 1 |
| `appId` | 1 | 1 |
| `appIndex` | 1 | 1 |
| `bundleName` | 3 | 3 |
| `callerBundleName` | 1 | 1 |
| `callerUid` | 1 | 0 |
| `ohos.ability.params.isSkipErmsFromSCB` | 1 | 1 |
| `ohos.app.logout_recovery` | 1 | 1 |
| `ohos.app.recovery` | 1 | 1 |
| `string(PARAM_SEND_RESULT_CALLER_BUNDLENAME)` | 1 | 1 |
| `string(PARAM_SEND_RESULT_CALLER_TOKENID)` | 1 | 1 |
| `uid` | 4 | 0 |
| `userId` | 5 | 0 |