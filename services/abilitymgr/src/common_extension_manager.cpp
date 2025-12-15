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

CommonExtensionManager::CommonExtensionManager(int userId) : AbilityConnectManager(userId)
{}

CommonExtensionManager::~CommonExtensionManager()
{}

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

void AbilityConnectManager::SetServiceAfterNewCreate(const AbilityRequest &abilityRequest,
    ExtensionRecordBase &targetService)
{
    if (abilityRequest.abilityInfo.name == AbilityConfig::LAUNCHER_ABILITY_NAME) {
        targetService.SetLauncherRoot();
        if (abilityRequest.restart) {
            targetService.SetRestartTime(abilityRequest.restartTime);
            targetService.SetRestartCount(abilityRequest.restartCount);
        }
    } else if (IsAbilityNeedKeepAlive(ExtensionRecordBase::TransferToExtensionRecordBase(
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

void AbilityConnectManager::RemoveServiceFromMapSafe(const std::string &serviceKey)
{
    std::lock_guard lock(serviceMapMutex_);
    serviceMap_.erase(serviceKey);
    TAG_LOGD(AAFwkTag::EXT, "ServiceMap remove, size:%{public}zu", serviceMap_.size());
}

int32_t AbilityConnectManager::SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard guard(serialMutex_);
    TAG_LOGD(AAFwkTag::EXT, "call");

    // 1. check whether callback was connected.
    ConnectListType connectRecordList;
    GetConnectRecordListFromMap(connect, connectRecordList);
    if (connectRecordList.empty()) {
        TAG_LOGE(AAFwkTag::EXT, "empty");
        return CONNECTION_NOT_EXIST;
    }

    // 2. schedule suspend to target service
    int result = ERR_OK;
    for (auto &connectRecord : connectRecordList) {
        if (connectRecord) {
            if (connectRecord->GetCallerTokenId() != IPCSkeleton::GetCallingTokenID() &&
                static_cast<uint32_t>(IPCSkeleton::GetSelfTokenID() != IPCSkeleton::GetCallingTokenID())) {
                TAG_LOGW(AAFwkTag::EXT, "inconsistent caller");
                continue;
            }

            result = connectRecord->SuspendExtensionAbility();
            if (result != ERR_OK) {
                TAG_LOGE(AAFwkTag::EXT, "fail , ret = %{public}d", result);
                break;
            }
        }
    }
    return result;
}

int AbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken, sptr<SessionInfo> sessionInfo,
    sptr<UIExtensionAbilityConnectInfo> connectInfo)
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
    std::shared_ptr<ExtensionRecordBase> targetService;
    bool isLoadedAbility = false;
    ret = GetOrCreateTargetServiceRecord(abilityRequest, connectInfo, targetService, isLoadedAbility);
    if (ret != ERR_OK) {
        return ret;
    }

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
        auto updateRecordCallback = [mgr = shared_from_this()](
            const std::shared_ptr<ExtensionRecordBase>& targetService) {
            if (mgr != nullptr) {
                mgr->UpdateUIExtensionInfo(targetService, AAFwk::DEFAULT_INVAL_VALUE);
            }
        };
        LoadAbility(targetService, updateRecordCallback);
    } else if (targetService->IsAbilityState(AbilityState::ACTIVE)) {
        targetService->SetWant(abilityRequest.want);
        HandleActiveAbility(targetService, connectRecord);
    } else {
        TAG_LOGI(AAFwkTag::EXT, "targetService activing");
        targetService->SaveConnectWant(abilityRequest.want);
    }
    return ret;
}
}  // namespace AAFwk
}  // namespace OHOS