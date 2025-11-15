/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ui_extension_connect_manager.h"

#include "ability_manager_service.h"
#include "ability_permission_util.h"
#include "app_exit_reason_data_manager.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "int_wrapper.h"
#include "request_id_util.h"
#include "session/host/include/zidl/session_interface.h"
#include "string_wrapper.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
const std::string UIEXTENSION_ROOT_HOST_PID = "ability.want.params.uiExtensionRootHostPid";
const std::string UIEXTENSION_HOST_PID = "ability.want.params.uiExtensionHostPid";
const std::string UIEXTENSION_HOST_UID = "ability.want.params.uiExtensionHostUid";
const std::string UIEXTENSION_HOST_BUNDLENAME = "ability.want.params.uiExtensionHostBundleName";
const std::string UIEXTENSION_BIND_ABILITY_ID = "ability.want.params.uiExtensionBindAbilityId";
const std::string UIEXTENSION_NOTIFY_BIND = "ohos.uiextension.params.notifyProcessBind";
const std::string IS_PRELOAD_UIEXTENSION_ABILITY = "ability.want.params.is_preload_uiextension_ability";
}

UIExtensionAbilityConnectManager::UIExtensionAbilityConnectManager(int userId) : AbilityConnectManagerBase(userId)
{}

UIExtensionAbilityConnectManager::~UIExtensionAbilityConnectManager()
{}

int32_t UIExtensionAbilityConnectManager::ConnectAbilityLocked(const AbilityRequest &abilityRequest,
    const sptr<IAbilityConnection> &connect, const sptr<IRemoteObject> &callerToken,
    sptr<SessionInfo> sessionInfo, sptr<UIExtensionAbilityConnectInfo> connectInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    
    if (!UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        return AbilityConnectManagerBase::ConnectAbilityLocked(abilityRequest, connect, callerToken, sessionInfo);
    }

    auto connectObject = connect->AsObject();
    std::lock_guard guard(serialMutex_);

    // 1. get target service ability record, and check whether it has been loaded.
    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    
    std::shared_ptr<AbilityRecord> targetService;
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
    if (isLoadedAbility && isCallbackConnected && connectedRecord != nullptr) {
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
            const std::shared_ptr<AbilityRecord>& targetService) {
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

int UIExtensionAbilityConnectManager::PreloadUIExtensionAbilityInner(const AbilityRequest &abilityRequest,
    std::string &hostBundleName, int32_t hostPid)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can't preload non-uiextension type");
        return ERR_WRONG_INTERFACE_CALL;
    }
    
    int32_t ret = AbilityPermissionUtil::GetInstance().CheckMultiInstanceKeyForExtension(abilityRequest);
    if (ret != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    
    std::shared_ptr<ExtensionRecord> extensionRecord = nullptr;
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    int32_t extensionRecordId = INVALID_EXTENSION_RECORD_ID;
    ret = uiExtensionAbilityRecordMgr_->CreateExtensionRecord(abilityRequest, hostBundleName,
        extensionRecord, extensionRecordId, hostPid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateExtensionRecord ERR");
        return ret;
    }
    
    CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
    std::shared_ptr<AbilityRecord> targetService = extensionRecord->abilityRecord_;
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    
    std::string extensionRecordKey = element.GetURI() + std::to_string(targetService->GetUIExtensionAbilityId());
    targetService->SetURI(extensionRecordKey);
    AddToServiceMap(extensionRecordKey, targetService);

    auto updateRecordCallback = [hostPid, mgr = shared_from_this()](
        const std::shared_ptr<AbilityRecord>& targetService) {
        if (mgr != nullptr) {
            mgr->UpdateUIExtensionInfo(targetService, hostPid);
        }
    };
    
    UpdateUIExtensionBindInfo(
        targetService, hostBundleName, abilityRequest.want.GetIntParam(UIEXTENSION_NOTIFY_BIND, -1));
    LoadAbility(targetService, updateRecordCallback);
    return ERR_OK;
}

int UIExtensionAbilityConnectManager::PreloadUIExtensionAbilityLocked(const AbilityRequest &abilityRequest,
    std::string &hostBundleName, int32_t hostPid)
{
    std::lock_guard guard(serialMutex_);
    return PreloadUIExtensionAbilityInner(abilityRequest, hostBundleName, hostPid);
}

int UIExtensionAbilityConnectManager::QueryPreLoadUIExtensionRecordInner(const AppExecFwk::ElementName &element,
    const std::string &moduleName, const std::string &hostBundleName, int32_t &recordNum)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->QueryPreLoadUIExtensionRecord(
        element, moduleName, hostBundleName, recordNum);
}

int UIExtensionAbilityConnectManager::UnloadUIExtensionAbility(const std::shared_ptr<AAFwk::AbilityRecord> &abilityRecord,
    std::string &bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    CHECK_POINTER_AND_RETURN(abilityRecord, ERR_INVALID_VALUE);
    
    //Get preLoadUIExtensionInfo
    auto preLoadUIExtensionInfo = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
        abilityRecord->GetWant().GetElement().GetBundleName(),
        abilityRecord->GetWant().GetElement().GetModuleName(), bundleName);
    
    //delete preLoadUIExtensionMap
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    uiExtensionAbilityRecordMgr_->RemovePreloadUIExtensionRecordById(preLoadUIExtensionInfo, extensionRecordId);
    
    //terminate preload uiextension
    auto token = abilityRecord->GetToken();
    auto result = TerminateAbilityInner(token);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "terminate error");
        return result;
    }
    return ERR_OK;
}

void UIExtensionAbilityConnectManager::ClearPreloadUIExtensionRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(abilityRecord);
    auto extensionRecordId = abilityRecord->GetUIExtensionAbilityId();
    std::string hostBundleName;
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    auto ret = uiExtensionAbilityRecordMgr_->GetHostBundleNameForExtensionId(extensionRecordId, hostBundleName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetHostBundleNameForExtensionId fail");
        return;
    }
    
    auto extensionRecordMapKey = std::make_tuple(abilityRecord->GetWant().GetElement().GetAbilityName(),
        abilityRecord->GetWant().GetElement().GetBundleName(),
        abilityRecord->GetWant().GetElement().GetModuleName(), hostBundleName);
    uiExtensionAbilityRecordMgr_->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
}

std::shared_ptr<AbilityRecord> UIExtensionAbilityConnectManager::GetUIExtensionBySessionInfo(
    const sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER_AND_RETURN(sessionInfo, nullptr);
    auto sessionToken = iface_cast<Rosen::ISession>(sessionInfo->sessionToken);
    CHECK_POINTER_AND_RETURN(sessionToken, nullptr);

    std::lock_guard guard(uiExtensionMapMutex_);
    auto it = uiExtensionMap_.find(sessionToken->AsObject());
    if (it != uiExtensionMap_.end()) {
        auto abilityRecord = it->second.first.lock();
        if (abilityRecord == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "abilityRecord null");
            RemoveUIExtWindowDeathRecipient(sessionToken->AsObject());
            uiExtensionMap_.erase(it);
            return nullptr;
        }
        auto savedSessionInfo = it->second.second;
        if (!savedSessionInfo || savedSessionInfo->sessionToken != sessionInfo->sessionToken
            || savedSessionInfo->callerToken != sessionInfo->callerToken) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "inconsistent sessionInfo");
            return nullptr;
        }
        return abilityRecord;
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "UIExtension not found");
    }
    return nullptr;
}

std::shared_ptr<AbilityRecord> UIExtensionAbilityConnectManager::GetUIExtensionBySessionFromServiceMap(
    const sptr<SessionInfo> &sessionInfo)
{
    int32_t persistentId = sessionInfo->persistentId;
    auto IsMatch = [persistentId](auto service) {
        if (!service.second) {
            return false;
        }
        auto sessionInfoPtr = service.second->GetSessionInfo();
        if (!sessionInfoPtr) {
            return false;
        }
        int32_t srcPersistentId = sessionInfoPtr->persistentId;
        return srcPersistentId == persistentId;
    };
    
    std::lock_guard lock(serviceMapMutex_);
    auto serviceRecord = std::find_if(serviceMap_.begin(), serviceMap_.end(), IsMatch);
    if (serviceRecord != serviceMap_.end()) {
        TAG_LOGW(AAFwkTag::UI_EXT, "abilityRecord still exists");
        return serviceRecord->second;
    }
    return nullptr;
}

int32_t UIExtensionAbilityConnectManager::GetActiveUIExtensionList(const int32_t pid,
    std::vector<std::string> &extensionList)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetActiveUIExtensionList(pid, extensionList);
}

int32_t UIExtensionAbilityConnectManager::GetActiveUIExtensionList(const std::string &bundleName,
    std::vector<std::string> &extensionList)
{
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetActiveUIExtensionList(bundleName, extensionList);
}

bool UIExtensionAbilityConnectManager::IsUIExtensionFocused(uint32_t uiExtensionTokenId,
    const sptr<IRemoteObject>& focusToken)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, false);
    std::lock_guard guard(uiExtensionMapMutex_);
    
    for (auto& item: uiExtensionMap_) {
        auto uiExtension = item.second.first.lock();
        auto sessionInfo = item.second.second;
        if (uiExtension && uiExtension->GetApplicationInfo().accessTokenId == uiExtensionTokenId) {
            if (sessionInfo && uiExtensionAbilityRecordMgr_->IsFocused(
                uiExtension->GetUIExtensionAbilityId(), sessionInfo->callerToken, focusToken)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Focused");
                return true;
            }
            if (sessionInfo && sessionInfo->callerToken == focusToken) {
                return true;
            }
        }
    }
    return false;
}

sptr<IRemoteObject> UIExtensionAbilityConnectManager::GetUIExtensionSourceToken(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called");
    std::lock_guard guard(uiExtensionMapMutex_);
    
    for (auto &item : uiExtensionMap_) {
        auto sessionInfo = item.second.second;
        auto uiExtension = item.second.first.lock();
        if (sessionInfo != nullptr && uiExtension != nullptr && uiExtension->GetToken() != nullptr &&
            uiExtension->GetToken()->AsObject() == token) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The source token found.");
            return sessionInfo->callerToken;
        }
    }
    return nullptr;
}

int32_t UIExtensionAbilityConnectManager::GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
    UIExtensionSessionInfo &uiExtensionSessionInfo)
{
    CHECK_POINTER_AND_RETURN(token, ERR_NULL_OBJECT);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    return uiExtensionAbilityRecordMgr_->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
}

void UIExtensionAbilityConnectManager::GetUIExtensionCallerTokenList(const std::shared_ptr<AbilityRecord> &abilityRecord,
    std::list<sptr<IRemoteObject>> &callerList)
{
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    uiExtensionAbilityRecordMgr_->GetCallerTokenList(abilityRecord, callerList);
}

std::shared_ptr<AAFwk::AbilityRecord> UIExtensionAbilityConnectManager::GetUIExtensionRootHostInfo(
    const sptr<IRemoteObject> token)
{
    CHECK_POINTER_AND_RETURN(token, nullptr);
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, nullptr);
    return uiExtensionAbilityRecordMgr_->GetUIExtensionRootHostInfo(token);
}

bool UIExtensionAbilityConnectManager::HasRequestIdInLoadAbilityQueue(int32_t requestId)
{
    std::lock_guard<std::mutex> guard(loadAbilityQueueLock_);
    for (const auto &map : loadAbilityQueue_) {
        if (map.find(requestId) != map.end()) {
            return true;
        }
    }
    return false;
}

void UIExtensionAbilityConnectManager::OnStartSpecifiedProcessResponse(const std::string &flag, int32_t requestId)
{
    std::lock_guard<std::mutex> guard(loadAbilityQueueLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnStartSpecifiedProcessResponse, requestId: %{public}d, flag: %{public}s",
        requestId, flag.c_str());
        
    if (!loadAbilityQueue_.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnStartSpecifiedProcessResponse LoadAbility");
        auto &front = loadAbilityQueue_.front();
        front[requestId].want->SetParam(PARAM_SPECIFIED_PROCESS_FLAG, flag);
        DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(*front[requestId].loadParam,
            *(front[requestId].abilityInfo), *(front[requestId].appInfo), *(front[requestId].want));
        loadAbilityQueue_.pop_front();
    }
}

void UIExtensionAbilityConnectManager::OnStartSpecifiedProcessTimeoutResponse(int32_t requestId)
{
    std::lock_guard<std::mutex> guard(loadAbilityQueueLock_);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnStartSpecifiedProcessTimeoutResponse requestId: %{public}d", requestId);

    if (!loadAbilityQueue_.empty()) {
        auto &front = loadAbilityQueue_.front();
        DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(*(front[requestId].loadParam),
            *(front[requestId].abilityInfo), *(front[requestId].appInfo), *(front[requestId].want));
        loadAbilityQueue_.pop_front();
    }
}

void UIExtensionAbilityConnectManager::StartSpecifiedProcess(
    const LoadAbilityContext &context, const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    std::lock_guard<std::mutex> guard(loadAbilityQueueLock_);
    auto requestId = RequestIdUtil::GetRequestId();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartSpecifiedProcess, requestId: %{public}d,", requestId);
    
    std::map<int32_t, LoadAbilityContext> mapContext_ = { { requestId, context } };
    loadAbilityQueue_.push_back(mapContext_);
    if (loadAbilityQueue_.size() > 1) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "loadAbilityQueue_ size > 1, requestId: %{public}d", requestId);
        return;
    }
    
    DelayedSingleton<AppScheduler>::GetInstance()->StartSpecifiedProcess(
        *context.want, *context.abilityInfo, requestId, context.loadParam->customProcessFlag);
}

void UIExtensionAbilityConnectManager::LoadAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    std::function<void(const std::shared_ptr<AbilityRecord>&)> updateRecordCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    abilityRecord->SetStartTime();

    if (!abilityRecord->CanRestartRootLauncher()) {
        TAG_LOGE(AAFwkTag::EXT, "CanRestartRootLauncher fail");
        RemoveServiceAbility(abilityRecord);
        return;
    }
    
    if (!abilityRecord->IsDebugApp()) {
        TAG_LOGD(AAFwkTag::EXT, "IsDebug is false, here is not debug app");
        PostTimeOutTask(abilityRecord, AbilityManagerService::LOAD_TIMEOUT_MSG);
    }
    
    sptr<Token> token = abilityRecord->GetToken();
    sptr<Token> perToken = nullptr;
    if (abilityRecord->IsCreateByConnect()) {
        auto connectingRecord = abilityRecord->GetConnectingRecord();
        CHECK_POINTER(connectingRecord);
        perToken = iface_cast<Token>(connectingRecord->GetToken());
    } else {
        auto callerList = abilityRecord->GetCallerRecordList();
        if (!callerList.empty() && callerList.back()) {
            auto caller = callerList.back()->GetCaller();
            if (caller) {
                perToken = caller->GetToken();
            }
        }
    }
    
    if (updateRecordCallback != nullptr) {
        updateRecordCallback(abilityRecord);
    }
    
    AbilityRuntime::LoadParam loadParam;
    loadParam.abilityRecordId = abilityRecord->GetRecordId();
    loadParam.isShellCall = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    loadParam.token = token;
    loadParam.preToken = perToken;
    loadParam.instanceKey = abilityRecord->GetInstanceKey();
    loadParam.isCallerSetProcess = abilityRecord->IsCallerSetProcess();
    loadParam.customProcessFlag = abilityRecord->GetCustomProcessFlag();
    loadParam.extensionProcessMode = abilityRecord->GetExtensionProcessMode();
    SetExtensionLoadParam(loadParam, abilityRecord);
    
    AbilityRuntime::FreezeUtil::GetInstance().AddLifecycleEvent(loadParam.token, 
        "UIExtensionAbilityConnectManager::LoadAbility");
    HandleLoadAbilityOrStartSpecifiedProcess(loadParam, abilityRecord);
}

void UIExtensionAbilityConnectManager::HandleLoadAbilityOrStartSpecifiedProcess(
    const AbilityRuntime::LoadParam &loadParam, const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    if (abilityRecord->GetAbilityInfo().isolationProcess &&
        AAFwk::UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGD(AAFwkTag::EXT, "Is UIExtension and isolationProcess, StartSpecifiedProcess");
        LoadAbilityContext context{ std::make_shared<AbilityRuntime::LoadParam>(loadParam),
            std::make_shared<AppExecFwk::AbilityInfo>(abilityRecord->GetAbilityInfo()),
            std::make_shared<AppExecFwk::ApplicationInfo>(abilityRecord->GetApplicationInfo()),
            std::make_shared<Want>(abilityRecord->GetWant()) };
        StartSpecifiedProcess(context, abilityRecord);
    } else {
        TAG_LOGD(AAFwkTag::EXT, "LoadAbility");
        DelayedSingleton<AppScheduler>::GetInstance()->LoadAbility(
            loadParam, abilityRecord->GetAbilityInfo(), abilityRecord->GetApplicationInfo(), abilityRecord->GetWant());
    }
}

void UIExtensionAbilityConnectManager::ConnectUIServiceExtAbility(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int connectRecordId, const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    PostTimeOutTask(abilityRecord, connectRecordId, AbilityManagerService::CONNECT_TIMEOUT_MSG);
    abilityRecord->ConnectAbilityWithWant(want);
}

void UIExtensionAbilityConnectManager::HandleUIExtensionDied(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    CHECK_POINTER(abilityRecord);
    std::lock_guard guard(uiExtensionMapMutex_);
    
    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end();) {
        std::shared_ptr<AbilityRecord> uiExtAbility = it->second.first.lock();
        if (uiExtAbility == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "uiExtAbility null");
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }

        if (abilityRecord == uiExtAbility) {
            sptr<Rosen::ISession> sessionProxy = iface_cast<Rosen::ISession>(it->first);
            if (sessionProxy) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "start NotifyExtensionDied");
                sessionProxy->NotifyExtensionDied();
            }
            TAG_LOGW(AAFwkTag::UI_EXT, "uiExtAbility died");
            RemoveUIExtWindowDeathRecipient(it->first);
            it = uiExtensionMap_.erase(it);
            continue;
        }
        ++it;
    }
}

void UIExtensionAbilityConnectManager::DoForegroundUIExtension(std::shared_ptr<AbilityRecord> abilityRecord,
    const AbilityRequest &abilityRequest)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(abilityRequest.sessionInfo);
    
    auto abilitystateStr = abilityRecord->ConvertAbilityState(abilityRecord->GetAbilityState());
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "foreground ability: %{public}s/%{public}s, persistentId: %{public}d, abilityState: %{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        abilityRequest.sessionInfo->persistentId, abilitystateStr.c_str());
        
    if (abilityRecord->IsReady() && !abilityRecord->IsAbilityState(AbilityState::INACTIVATING) &&
        !abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING) &&
        !abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING) &&
        abilityRecord->IsAbilityWindowReady()) {
        if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
            abilityRecord->SetWant(abilityRequest.want);
            CommandAbilityWindow(abilityRecord, abilityRequest.sessionInfo, WIN_CMD_FOREGROUND);
            return;
        } else {
            abilityRecord->SetWant(abilityRequest.want);
            abilityRecord->PostUIExtensionAbilityTimeoutTask(AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
            DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(abilityRecord->GetToken());
            if (!abilityRecord->IsConnectionReported() && ForegroundAppConnectionManager::IsForegroundAppConnection(
                abilityRecord->GetAbilityInfo(), abilityRecord->GetCallerRecord())) {
                abilityRecord->ReportAbilityConnectionRelations();
                abilityRecord->SetConnectionReported(true);
            }
            return;
        }
    }
    EnqueueStartServiceReq(abilityRequest, abilityRecord->GetURI());
}

void UIExtensionAbilityConnectManager::DoBackgroundAbilityWindow(const std::shared_ptr<AbilityRecord> &abilityRecord,
    const sptr<SessionInfo> &sessionInfo)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(sessionInfo);
    
    auto abilitystateStr = abilityRecord->ConvertAbilityState(abilityRecord->GetAbilityState());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s/%{public}s, persistentId:%{public}d, abilityState:%{public}s",
        abilityRecord->GetElementName().GetBundleName().c_str(),
        abilityRecord->GetElementName().GetAbilityName().c_str(),
        sessionInfo->persistentId, abilitystateStr.c_str());
        
    if (abilityRecord->IsAbilityState(AbilityState::FOREGROUND)) {
        MoveToBackground(abilityRecord);
    } else if (abilityRecord->IsAbilityState(AbilityState::INITIAL) ||
        abilityRecord->IsAbilityState(AbilityState::FOREGROUNDING)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "exist initial or foregrounding task");
        abilityRecord->DoBackgroundAbilityWindowDelayed(true);
    } else if (!abilityRecord->IsAbilityState(AbilityState::BACKGROUNDING)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid ability state");
    }
}

void UIExtensionAbilityConnectManager::AddUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    std::lock_guard lock(uiExtRecipientMapMutex_);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "recipient added before");
        return;
    } else {
        std::weak_ptr<UIExtensionAbilityConnectManager> thisWeakPtr(
            std::static_pointer_cast<UIExtensionAbilityConnectManager>(shared_from_this()));
        sptr<IRemoteObject::DeathRecipient> deathRecipient =
            new AbilityConnectCallbackRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto abilityConnectManager = thisWeakPtr.lock();
                if (abilityConnectManager) {
                    abilityConnectManager->OnUIExtWindowDied(remote);
                }
            });
        if (!session->AddDeathRecipient(deathRecipient)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient fail");
        }
        uiExtRecipientMap_.emplace(session, deathRecipient);
    }
}

void UIExtensionAbilityConnectManager::RemoveUIExtWindowDeathRecipient(const sptr<IRemoteObject> &session)
{
    CHECK_POINTER(session);
    std::lock_guard lock(uiExtRecipientMapMutex_);
    auto it = uiExtRecipientMap_.find(session);
    if (it != uiExtRecipientMap_.end() && it->first != nullptr) {
        it->first->RemoveDeathRecipient(it->second);
        uiExtRecipientMap_.erase(it);
        return;
    }
}

void UIExtensionAbilityConnectManager::OnUIExtWindowDied(const wptr<IRemoteObject> &remote)
{
    auto object = remote.promote();
    CHECK_POINTER(object);
    if (taskHandler_) {
        auto task = [object, connectManagerWeak = std::static_pointer_cast<UIExtensionAbilityConnectManager>(
            shared_from_this())]() {
            auto connectManager = connectManagerWeak.lock();
            CHECK_POINTER(connectManager);
            connectManager->HandleUIExtWindowDiedTask(object);
        };
        taskHandler_->SubmitTask(task);
    }
}

void UIExtensionAbilityConnectManager::HandleUIExtWindowDiedTask(const sptr<IRemoteObject> &remote)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call.");
    CHECK_POINTER(remote);
    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<SessionInfo> sessionInfo;
    {
        std::lock_guard guard(uiExtensionMapMutex_);
        auto it = uiExtensionMap_.find(remote);
        if (it != uiExtensionMap_.end()) {
            abilityRecord = it->second.first.lock();
            sessionInfo = it->second.second;
            TAG_LOGW(AAFwkTag::UI_EXT, "uiExtAbility caller died");
            uiExtensionMap_.erase(it);
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "not find");
            return;
        }
    }

    if (abilityRecord) {
        TerminateAbilityWindowLocked(abilityRecord, sessionInfo);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "abilityRecord null");
    }
    RemoveUIExtWindowDeathRecipient(remote);
}

bool UIExtensionAbilityConnectManager::IsUIExtensionAbility(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    return UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType);
}

bool UIExtensionAbilityConnectManager::CheckUIExtensionAbilityLoaded(const AbilityRequest &abilityRequest)
{
    // Implementation for checking if UI extension ability is loaded
    return true;
}

bool UIExtensionAbilityConnectManager::CheckUIExtensionAbilitySessionExist(
    const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN(abilityRecord, false);
    std::lock_guard guard(uiExtensionMapMutex_);
    
    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end(); ++it) {
        std::shared_ptr<AbilityRecord> uiExtAbility = it->second.first.lock();
        if (abilityRecord == uiExtAbility) {
            return true;
        }
    }
    return false;
}

void UIExtensionAbilityConnectManager::RemoveUIExtensionAbilityRecord(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        ClearPreloadUIExtensionRecord(abilityRecord);
    }
    uiExtensionAbilityRecordMgr_->RemoveExtensionRecord(abilityRecord->GetUIExtensionAbilityId());
}

void UIExtensionAbilityConnectManager::AddUIExtensionAbilityRecordToTerminatedList(
    const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER(abilityRecord);
    CHECK_POINTER(uiExtensionAbilityRecordMgr_);
    uiExtensionAbilityRecordMgr_->AddExtensionRecordToTerminatedList(abilityRecord->GetUIExtensionAbilityId());
}

int32_t UIExtensionAbilityConnectManager::GetOrCreateExtensionRecord(const AbilityRequest &abilityRequest,
    bool isCreatedByConnect, const std::string &hostBundleName, std::shared_ptr<AbilityRecord> &extensionRecord,
    bool &isLoaded)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId, abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName);
        
    CHECK_POINTER_AND_RETURN(uiExtensionAbilityRecordMgr_, ERR_NULL_OBJECT);
    if (uiExtensionAbilityRecordMgr_->IsBelongToManager(abilityRequest.abilityInfo)) {
        int32_t ret = uiExtensionAbilityRecordMgr_->GetOrCreateExtensionRecord(
            abilityRequest, hostBundleName, extensionRecord, isLoaded);
        if (ret != ERR_OK) {
            return ret;
        }
        CHECK_POINTER_AND_RETURN(extensionRecord, ERR_NULL_OBJECT);
        
        extensionRecord->SetCreateByConnectMode(isCreatedByConnect);
        std::string extensionRecordKey = element.GetURI() + std::to_string(extensionRecord->GetUIExtensionAbilityId());
        extensionRecord->SetURI(extensionRecordKey);
        
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Service map add, hostBundleName:%{public}s, key: %{public}s",
            hostBundleName.c_str(), extensionRecordKey.c_str());
        AddToServiceMap(extensionRecordKey, extensionRecord);
        
        if (IsAbilityNeedKeepAlive(extensionRecord)) {
            extensionRecord->SetRestartTime(abilityRequest.restartTime);
            extensionRecord->SetRestartCount(abilityRequest.restartCount);
        }
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

int32_t UIExtensionAbilityConnectManager::GetOrCreateTargetServiceRecord(
    const AbilityRequest &abilityRequest, const sptr<UIExtensionAbilityConnectInfo> &connectInfo,
    std::shared_ptr<AbilityRecord> &targetService, bool &isLoadedAbility)
{
    if (UIExtensionUtils::IsUIExtension(abilityRequest.abilityInfo.extensionAbilityType) && connectInfo != nullptr) {
        int32_t ret = GetOrCreateExtensionRecord(
            abilityRequest, true, connectInfo->hostBundleName, targetService, isLoadedAbility);
        if (ret != ERR_OK || targetService == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GetOrCreateExtensionRecord fail");
            return ERR_NULL_OBJECT;
        }
        connectInfo->uiExtensionAbilityId = targetService->GetUIExtensionAbilityId();
        TAG_LOGD(AAFwkTag::ABILITYMGR, "UIExtensionAbility id %{public}d.", connectInfo->uiExtensionAbilityId);
    } else {
        GetOrCreateServiceRecord(abilityRequest, true, targetService, isLoadedAbility);
    }
    CHECK_POINTER_AND_RETURN(targetService, ERR_INVALID_VALUE);
    return ERR_OK;
}

void UIExtensionAbilityConnectManager::UpdateUIExtensionInfo(const std::shared_ptr<AbilityRecord> &abilityRecord,
    int32_t hostPid)
{
    if (abilityRecord == nullptr ||
        !UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        return;
    }

    WantParams wantParams;
    auto uiExtensionAbilityId = abilityRecord->GetUIExtensionAbilityId();
    wantParams.SetParam(UIEXTENSION_ABILITY_ID, AAFwk::Integer::Box(uiExtensionAbilityId));
    
    auto rootHostRecord = GetUIExtensionRootHostInfo(abilityRecord->GetToken());
    if (rootHostRecord != nullptr) {
        auto rootHostPid = rootHostRecord->GetPid();
        wantParams.SetParam(UIEXTENSION_ROOT_HOST_PID, AAFwk::Integer::Box(rootHostPid));
    }
    
    if (abilityRecord->GetWant().GetBoolParam(IS_PRELOAD_UIEXTENSION_ABILITY, false)) {
        // Applicable only to preloadUIExtension scenarios
        auto rootHostPid = (hostPid == AAFwk::DEFAULT_INVAL_VALUE) ? IPCSkeleton::GetCallingPid() : hostPid;
        wantParams.SetParam(UIEXTENSION_ROOT_HOST_PID, AAFwk::Integer::Box(rootHostPid));
    }
    
    abilityRecord->UpdateUIExtensionInfo(wantParams);
}

void UIExtensionAbilityConnectManager::UpdateUIExtensionBindInfo(
    const std::shared_ptr<AbilityRecord> &abilityRecord, std::string callerBundleName, int32_t notifyProcessBind)
{
    if (abilityRecord == nullptr ||
        !UIExtensionUtils::IsUIExtension(abilityRecord->GetAbilityInfo().extensionAbilityType)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "record null or abilityType not match");
        return;
    }

    if (callerBundleName == AbilityConfig::SCENEBOARD_BUNDLE_NAME) {
        TAG_LOGE(AAFwkTag::UI_EXT, "scb not allow bind process");
        return;
    }

    auto sessionInfo = abilityRecord->GetSessionInfo();
    if (sessionInfo == nullptr) {
        if (AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
            TAG_LOGW(AAFwkTag::UI_EXT, "sa preload not allow bind process");
            return;
        }
    } else {
        if (sessionInfo->uiExtensionUsage == AAFwk::UIExtensionUsage::MODAL) {
            TAG_LOGE(AAFwkTag::UI_EXT, "modal not allow bind process");
            return;
        }
    }

    WantParams wantParams;
    auto uiExtensionBindAbilityId = abilityRecord->GetUIExtensionAbilityId();
    wantParams.SetParam(UIEXTENSION_BIND_ABILITY_ID, AAFwk::Integer::Box(uiExtensionBindAbilityId));
    wantParams.SetParam(UIEXTENSION_NOTIFY_BIND, AAFwk::Integer::Box(notifyProcessBind));
    wantParams.SetParam(UIEXTENSION_HOST_PID, AAFwk::Integer::Box(IPCSkeleton::GetCallingPid()));
    wantParams.SetParam(UIEXTENSION_HOST_UID, AAFwk::Integer::Box(IPCSkeleton::GetCallingUid()));
    wantParams.SetParam(UIEXTENSION_HOST_BUNDLENAME, String ::Box(callerBundleName));
    abilityRecord->UpdateUIExtensionBindInfo(wantParams);
}

bool UIExtensionAbilityConnectManager::IsCallerValid(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    CHECK_POINTER_AND_RETURN_LOG(abilityRecord, false, "Invalid caller for UIExtension");
    auto sessionInfo = abilityRecord->GetSessionInfo();
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo, false, "Invalid caller for UIExtension");
    CHECK_POINTER_AND_RETURN_LOG(sessionInfo->sessionToken, false, "Invalid caller for UIExtension");
    
    std::lock_guard lock(uiExtRecipientMapMutex_);
    if (uiExtRecipientMap_.find(sessionInfo->sessionToken) == uiExtRecipientMap_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "invalid caller for UIExtension");
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "The caller survival.");
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS