/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common_extension_manager.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string FRS_BUNDLE_NAME = "com.ohos.formrenderservice";
}

CommonExtensionManager::CommonExtensionManager(int userId) : AbilityConnectManager(userId)
{}

CommonExtensionManager::~CommonExtensionManager()
{}

int AbilityConnectManager::StartAbilityLocked(const AbilityRequest &abilityRequest)
{
    if (AppUtils::GetInstance().IsForbidStart()) {
        TAG_LOGW(AAFwkTag::EXT, "forbid start: %{public}s", abilityRequest.want.GetElement().GetBundleName().c_str());
        return INNER_ERR;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EXT, "bundle/ability:%{public}s/%{public}s",
        abilityRequest.want.GetElement().GetBundleName().c_str(),
        abilityRequest.want.GetElement().GetAbilityName().c_str());

    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        //  Do not distinguishing specific error codes
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<BaseExtensionRecord> targetService;
    bool isLoadedAbility = false;
    std::string hostBundleName;
    GetOrCreateServiceRecord(abilityRequest, false, targetService, isLoadedAbility);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::EXT, "%{public}s/%{public}s",
        targetService->GetElementName().GetBundleName().c_str(),
        targetService->GetElementName().GetAbilityName().c_str());

    std::string value = abilityRequest.want.GetStringParam(Want::PARM_LAUNCH_REASON_MESSAGE);
    targetService->AddCallerRecord(abilityRequest.callerToken, abilityRequest.requestCode, abilityRequest.want);
    targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_START_EXTENSION);
    targetService->DoBackgroundAbilityWindowDelayed(false);
    targetService->SetSessionInfo(abilityRequest.sessionInfo);

    ret = ReportXiaoYiToRSSIfNeeded(abilityRequest.abilityInfo);
    if (ret != ERR_OK) {
        return ret;
    }
    ReportEventToRSS(abilityRequest.abilityInfo, targetService, abilityRequest.callerToken);
    if (!isLoadedAbility) {
        TAG_LOGD(AAFwkTag::EXT, "targetService has not been loaded");
        LoadAbility(targetService, nullptr);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        // It may have been started through connect
        targetService->SetWant(abilityRequest.want);
        CommandAbility(targetService);
    } else {
        TAG_LOGI(AAFwkTag::EXT, "TargetService not active, state: %{public}d", targetService->GetAbilityState());
        EnqueueStartServiceReq(abilityRequest);
        return ERR_OK;
    }
    return ERR_OK;
}

int AbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    auto connectObject = connect->AsObject();
#ifdef SUPPORT_UPMS
    // grant uri to service extension by connect, must call out of serialMutex_
    int32_t callerUser = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    if (userId_ == U0_USER_ID || callerUser == U0_USER_ID || callerUser == U1_USER_ID ||
        userId_ == AbilityRuntime::UserController::GetInstance().GetCallerUserId()) {
        UriUtils::GetInstance().GrantUriPermissionForServiceExtension(abilityRequest);
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "cross user, without grantUriPermission");
    }
#endif // SUPPORT_UPMS
    std::lock_guard guard(serialMutex_);

    // 1. get target service ability record, and check whether it has been loaded.
    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        //  Do not distinguishing specific error codes
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<BaseExtensionRecord> targetService;
    bool isLoadedAbility = false;
    GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);

    ReportEventToRSS(abilityRequest.abilityInfo, targetService, callerToken);
    // 2. get target connectRecordList, and check whether this callback has been connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    bool isCallbackConnected = !connectRecordList.empty();
    auto connectedRecord = GetAbilityConnectedRecordFromRecordList(targetService, connectRecordList);
    // 3. If this service ability and callback has been connected, There is no need to connect repeatedly
    if (isLoadedAbility && (isCallbackConnected) && (connectedRecord != nullptr)) {
        TAG_LOGI(AAFwkTag::EXT, "service/callback connected");
        connectedRecord->CompleteConnectAndOnlyCallConnectDone();
        return ERR_OK;
    }

    // 4. Other cases , need to connect the service ability
    auto connectRecord = ConnectionRecord::CreateConnectionRecord(
        callerToken, targetService, connect, shared_from_this());
    CHECK_POINTER_AND_RETURN(connectRecord, ERR_INVALID_VALUE);
    connectRecord->AttachCallerInfo();
    connectRecord->SetConnectState(ConnectionState::CONNECTING);
    if (targetService->GetAbilityInfo().extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        connectRecord->SetConnectWant(abilityRequest.want);
    }
    targetService->AddConnectRecordToList(connectRecord);
    targetService->SetSessionInfo(sessionInfo);
    connectRecordList.push_back(connectRecord);
    AddConnectObjectToMap(connectObject, connectRecordList, isCallbackConnected);
    HandleConnectionCountIncrement(connectRecord->GetCallerPid(), connectRecord->GetCallerName(),
        abilityRequest.abilityInfo.bundleName + "/" + abilityRequest.abilityInfo.name);
    targetService->SetLaunchReason(LaunchReason::LAUNCHREASON_CONNECT_EXTENSION);

    if (UIExtensionUtils::IsWindowExtension(targetService->GetAbilityInfo().extensionAbilityType)
        && abilityRequest.sessionInfo) {
        std::lock_guard guard(windowExtensionMapMutex_);
        windowExtensionMap_.emplace(connectObject,
            WindowExtMapValType(targetService->GetApplicationInfo().accessTokenId, abilityRequest.sessionInfo));
    }

    auto &abilityInfo = abilityRequest.abilityInfo;
    ret = ReportXiaoYiToRSSIfNeeded(abilityInfo);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!isLoadedAbility) {
        TAG_LOGI(AAFwkTag::EXT, "load");
        LoadAbility(targetService, nullptr);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        targetService->SetWant(abilityRequest.want);
        HandleActiveAbility(targetService, connectRecord);
    } else {
        TAG_LOGI(AAFwkTag::EXT, "targetService activing");
        targetService->SaveConnectWant(abilityRequest.want);
    }
    return ret;
}

void AbilityConnectManager::GetOrCreateServiceRecord(const AbilityRequest &abilityRequest,
    const bool isCreatedByConnect, std::shared_ptr<BaseExtensionRecord> &targetService, bool &isLoadedAbility)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // lifecycle is not complete when window extension is reused
    bool noReuse = UIExtensionUtils::IsWindowExtension(abilityRequest.abilityInfo.extensionAbilityType);
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, GenerateBundleName(abilityRequest),
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    std::string serviceKey = element.GetURI();
    if (FRS_BUNDLE_NAME == abilityRequest.abilityInfo.bundleName) {
        serviceKey = element.GetURI() + std::to_string(abilityRequest.want.GetIntParam(FRS_APP_INDEX, 0));
    }
    {
        std::lock_guard lock(serviceMapMutex_);
        auto serviceMapIter = serviceMap_.find(serviceKey);
        targetService = serviceMapIter == serviceMap_.end() ? nullptr : serviceMapIter->second;
    }
    if (targetService == nullptr &&
        IsCacheExtensionAbilityByInfo(abilityRequest.abilityInfo)) {
        targetService = AbilityCacheManager::GetInstance().Get(abilityRequest);
        if (targetService != nullptr) {
            AddToServiceMap(serviceKey, targetService);
        }
    }
    if (noReuse && targetService) {
        if (IsSpecialAbility(abilityRequest.abilityInfo)) {
            TAG_LOGI(AAFwkTag::EXT, "removing ability: %{public}s/%{public}s",
                element.GetBundleName().c_str(),
                element.GetAbilityName().c_str());
        }
        RemoveServiceFromMapSafe(serviceKey);
    }
    isLoadedAbility = true;
    if (noReuse || targetService == nullptr) {
        targetService = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
        CHECK_POINTER(targetService);
        targetService->SetOwnerMissionUserId(userId_);
        if (isCreatedByConnect) {
            targetService->SetCreateByConnectMode();
        }
        SetServiceAfterNewCreate(abilityRequest, *targetService);
        AddToServiceMap(serviceKey, targetService);
        isLoadedAbility = false;
    }
    TAG_LOGD(AAFwkTag::EXT, "service map add, serviceKey: %{public}s", serviceKey.c_str());
}


void AbilityConnectManager::SetServiceAfterNewCreate(const AbilityRequest &abilityRequest,
    BaseExtensionRecord &targetService)
{
    if (abilityRequest.abilityInfo.name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
        targetService.SetLauncherRoot();
        if (abilityRequest.restart) {
            targetService.SetRestartTime(abilityRequest.restartTime);
            targetService.SetRestartCount(abilityRequest.restartCount);
        }
    } else if (IsAbilityNeedKeepAlive(BaseExtensionRecord::TransferToExtensionRecordBase(
        targetService.shared_from_this())) && abilityRequest.restart) {
        targetService.SetRestartTime(abilityRequest.restartTime);
        targetService.SetRestartCount(abilityRequest.restartCount);
    }
    if (MultiInstanceUtils::IsMultiInstanceApp(abilityRequest.appInfo)) {
        targetService.SetInstanceKey(MultiInstanceUtils::GetValidExtensionInstanceKey(abilityRequest));
    }
    if (targetService.IsSceneBoard()) {
        TAG_LOGI(AAFwkTag::EXT, "create sceneboard");
        sceneBoardTokenId_ = abilityRequest.appInfo.accessTokenId;
    }
}

int AbilityConnectManager::AttachAbilityThreadInner(const sptr<IAbilityScheduler> &scheduler,
    const sptr<IRemoteObject> &token, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string element = abilityRecord->GetURI();
    TAG_LOGD(AAFwkTag::EXT, "ability:%{public}s", element.c_str());
    abilityRecord->RemoveLoadTimeoutTask();
    AbilityRuntime::FreezeUtil::GetInstance().DeleteLifecycleEvent(token);
    abilityRecord->SetScheduler(scheduler);
    TAG_LOGD(AAFwkTag::EXT, "Inactivate");
    abilityRecord->Inactivate();
    return ERR_OK;
}

void AbilityConnectManager::HandleLoadAbilityOrStartSpecifiedProcess(
    const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::EXT, "LoadAbility");
    DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
        loadParam, abilityRecord->GetAbilityInfo(), abilityRecord->GetApplicationInfo(), abilityRecord->GetWant());
}

int AbilityConnectManager::TerminateAbilityInner(const sptr<IRemoteObject> &token)
{
    auto abilityRecord = GetExtensionByTokenFromServiceMap(token);
    if (abilityRecord == nullptr) {
        abilityRecord = AbilityCacheManager::GetInstance().FindRecordByToken(token);
    }
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_CONNECT_MANAGER_NULL_ABILITY_RECORD);
    std::string element = abilityRecord->GetURI();
    TAG_LOGD(AAFwkTag::EXT, "terminate ability, ability is %{public}s", element.c_str());
    MoveToTerminatingMap(abilityRecord);
    return TerminateAbilityLocked(token);
}

void AbilityConnectManager::HandleStopTimeoutTask(const std::shared_ptr<BaseExtensionRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::EXT, "Complete stop ability timeout start.");
    std::lock_guard guard(serialMutex_);
    TerminateDone(abilityRecord);
}
}  // namespace AAFwk
}  // namespace OHOS