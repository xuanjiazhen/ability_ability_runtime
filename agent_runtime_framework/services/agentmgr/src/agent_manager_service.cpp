/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "agent_manager_service.h"

#include <algorithm>
#include <chrono>
#include <utility>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_card_mgr.h"
#include "agent_card_utils.h"
#include "agent_config.h"
#include "agent_extension_connection_constants.h"
#include "agent_service_connection.h"
#include "app_mgr_client.h"
#include "bundle_mgr_helper.h"
#include "extension_ability_info.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
bool IsLowCodeTargetMatched(const AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return false;
    }
    if (want.GetElement().GetBundleName() != card.appInfo->bundleName ||
        want.GetElement().GetAbilityName() != card.appInfo->abilityName) {
        return false;
    }
    const std::string moduleName = want.GetElement().GetModuleName();
    if (!moduleName.empty() && moduleName != card.appInfo->moduleName) {
        return false;
    }
    return true;
}
}

std::mutex g_mutex;
sptr<AgentManagerService> AgentManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(AgentManagerService::GetInstance());

constexpr int32_t BASE_USER_RANGE = 200000;

namespace {
bool IsMatchedAgentCardTarget(const AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return false;
    }

    const auto &element = want.GetElement();
    if (element.GetBundleName() != card.appInfo->bundleName ||
        element.GetAbilityName() != card.appInfo->abilityName) {
        return false;
    }

    return element.GetModuleName().empty() || card.appInfo->moduleName.empty() ||
        element.GetModuleName() == card.appInfo->moduleName;
}

void NormalizeConnectAgentTarget(AAFwk::Want &want, const AgentCard &card)
{
    if (card.appInfo == nullptr || card.appInfo->moduleName.empty()) {
        return;
    }
    const auto &element = want.GetElement();
    if (!element.GetModuleName().empty()) {
        return;
    }
    want.SetElementName(element.GetBundleName(), element.GetAbilityName());
    want.SetModuleName(card.appInfo->moduleName);
}

void NormalizeConnectAgentTarget(AgentResolvedTarget &target,
    const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
{
    if (!extensionInfo.moduleName.empty()) {
        target.connectWant.SetModuleName(extensionInfo.moduleName);
    }
    target.targetUid = extensionInfo.applicationInfo.uid;
    target.targetAppIndex = extensionInfo.applicationInfo.appIndex;
}

sptr<IRemoteObject> GetLowCodeHostProxyIdentity(const AgentHostKey &key, const sptr<IRemoteObject> &callerRemote)
{
    return key.hostProxyRemote != nullptr ? key.hostProxyRemote : callerRemote;
}

AgentCallerIdentity BuildCallerIdentity(const AppExecFwk::RunningProcessInfo &processInfo)
{
    AgentCallerIdentity identity;
    identity.uid = processInfo.uid_;
    identity.instanceKey = processInfo.instanceKey;
    return identity;
}

// Some tests and legacy helper paths still fill the old uid-only fields.
AgentCallerIdentity GetStandardCallerIdentity(const StandardAgentKey &key)
{
    return key.callerIdentity.IsValid() ? key.callerIdentity : AgentCallerIdentity(key.callerUid);
}

// Prefer the target identity; fall back to the legacy flat field only for compatibility.
std::string GetStandardAgentId(const StandardAgentKey &key)
{
    return !key.targetIdentity.agentId.empty() ? key.targetIdentity.agentId : key.agentId;
}
}

sptr<AgentManagerService> AgentManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) AgentManagerService();
    return instance_;
}

AgentManagerService::AgentManagerService() : SystemAbility(AGENT_MGR_SERVICE_ID, true)
{}

void AgentManagerService::Init()
{
    taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(AgentConfig::NAME_AGENT_MGR_SERVICE);
    eventHandler_ = std::make_shared<AgentEventHandler>(taskHandler_, weak_from_this());
}

AgentManagerService::~AgentManagerService()
{}

void AgentManagerService::OnStart() noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr start");
    Init();
    if (!Publish(AgentManagerService::GetInstance())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Publish failed");
        return;
    }
    bool addBundleMgr = AddSystemAbilityListener(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (!addBundleMgr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "addBundleMgr failed");
    }
}

void AgentManagerService::OnStop() noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr stop");
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        trackedConnections_.clear();
        callerConnectionCounts_.clear();
        standardSessions_.clear();
    }
    std::lock_guard<std::mutex> hostLock(agentHostMutex_);
    agentHostSessions_.clear();
    agentOwners_.clear();
}

void AgentManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "add sysAbilityId %{public}d", systemAbilityId);
    if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
        std::string identity = IPCSkeleton::ResetCallingIdentity();
        RegisterBundleEventCallback();
        IPCSkeleton::SetCallingIdentity(identity);
    }
}

void AgentManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "remove sysAbilityId %{public}d", systemAbilityId);
}

void AgentManagerService::RegisterBundleEventCallback()
{
    if (bundleEventCallback_ != nullptr) {
        return;
    }
    bundleEventCallback_ = sptr<AgentBundleEventCallback>::MakeSptr();
    bool ret = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->RegisterBundleEventCallback(
        bundleEventCallback_);
    if (!ret) {
        bundleEventCallback_ = nullptr;
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register bundle event error");
    }
}

AgentCallerIdentity AgentManagerService::GetCurrentCallerIdentity() const
{
    AgentCallerIdentity identity { IPCSkeleton::GetCallingUid(), "" };
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
        IPCSkeleton::GetCallingPid(), processInfo));
    if (ret == ERR_OK) {
        identity.instanceKey = processInfo.instanceKey;
    }
    return identity;
}

int32_t AgentManagerService::GetAllAgentCards(AgentCardsRawData &cards)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().GetAllAgentCards(cards);
}

int32_t AgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName, AgentCardsRawData &cards)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    std::vector<AgentCard> cardVec;
    auto ret = AgentCardMgr::GetInstance().GetAgentCardsByBundleName(bundleName, cardVec);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "no AgentCards of bundle %{public}s", bundleName.c_str());
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        AgentCardsRawData::FromAgentCardVec({}, cards);
        return ERR_OK;
    }
    if (ret == ERR_OK) {
        AgentCardsRawData::FromAgentCardVec(cardVec, cards);
    }
    return ret;
}

int32_t AgentManagerService::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    auto ret = AgentCardMgr::GetInstance().GetAgentCardByAgentId(bundleName, agentId, card);
    if (ret == ERR_NAME_NOT_FOUND) {
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no AgentCard of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card)
{
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    std::string callerBundleName;
    int32_t callerUid = 0;
    int32_t ret = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetBundleNameByPid(
            callerPid, callerBundleName, callerUid));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetBundleNameByPid failed %{public}d", ret);
        return ret;
    }
    ret = AgentCardMgr::GetInstance().GetAgentCardByAgentId(callerBundleName, agentId, card);
    if (ret == ERR_NAME_NOT_FOUND) {
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                callerBundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no AgentCard of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::RegisterAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().RegisterAgentCard(card);
}

int32_t AgentManagerService::UpdateAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().UpdateAgentCard(card);
}

int32_t AgentManagerService::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().DeleteAgentCard(bundleName, agentId);
}

int32_t AgentManagerService::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    AgentCallerIdentity callerIdentity;
    auto ret = ValidateConnectAgentRequest(connection, callerIdentity);
    if (ret != ERR_OK) {
        return ret;
    }

    AgentResolvedTarget target;
    ret = ResolveConnectAgentTarget(want, target);
    if (ret != ERR_OK) {
        return ret;
    }

    if (target.card.type == AgentCardType::LOW_CODE) {
        return ConnectLowCodeAgentExtensionAbility(target, connection, callerIdentity);
    }

    return ConnectStandardAgentExtensionAbility(target, connection, callerIdentity);
}

int32_t AgentManagerService::ConnectStandardAgentExtensionAbility(AgentResolvedTarget &target,
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity)
{
    if (target.card.type == AgentCardType::ATOMIC_SERVICE) {
        target.connectWant.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        target.connectWant.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        target.connectWant.GetBundle().c_str(), target.agentId.c_str());

    StandardAgentKey key = StandardAgentKeyBuilder()
        .SetCallerIdentity(callerIdentity)
        .SetAgentId(target.agentId)
        .SetCardType(target.card.type)
        .SetTargetUid(target.targetUid)
        .SetTargetInstanceKey(target.targetInstanceKey)
        .SetWant(target.connectWant)
        .Build();
    bool isExistingSession = false;
    auto ret = TryConnectExistingAgentStandardRecord(target.connectWant, key, connection, isExistingSession);
    if (ret != ERR_OK || isExistingSession) {
        return ret;
    }

    return CreateAgentStandardRecord(target.connectWant, target.agentId, key, connection);
}

int32_t AgentManagerService::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto ret = ValidateConnectServiceRequest(callerToken, connection);
    if (ret != ERR_OK) {
        return ret;
    }

    AAFwk::Want connectWant;
    ret = PrepareServiceConnectWant(want, connectWant);
    if (ret != ERR_OK) {
        return ret;
    }

    sptr<AAFwk::IAbilityConnection> serviceConnection;
    AgentCallerIdentity callerIdentity = GetCurrentCallerIdentity();
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, callerIdentity, false,
        serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(connectWant, serviceConnection,
        callerToken, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connect service extension ability failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
    AgentCallerIdentity &callerIdentity)
{
    // Reject unauthorized callers before doing any stateful work.
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    // Only foreground apps are allowed to initiate agent connects.
    auto callerPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
        callerPid, processInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get process failed: %{public}d", ret);
        return ret;
    }
    if (processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "not foreground app");
        return AAFwk::NOT_TOP_ABILITY;
    }
    callerIdentity = BuildCallerIdentity(processInfo);
    callerIdentity.uid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t &callerUid)
{
    AgentCallerIdentity callerIdentity;
    auto ret = ValidateConnectAgentRequest(connection, callerIdentity);
    callerUid = callerIdentity.uid;
    return ret;
}

int32_t AgentManagerService::ResolveConnectAgentTarget(const AAFwk::Want &want,
    AgentResolvedTarget &target) const
{
    // Copy the incoming Want so later steps can enrich it safely.
    target.connectWant = want;
    target.agentId = target.connectWant.GetStringParam(AGENTID_KEY);
    if (target.agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    target.callingUid = IPCSkeleton::GetCallingUid();
    target.userId = target.callingUid / BASE_USER_RANGE;
    int32_t resolveRet = ResolveAgentExtensionAbility(target);
    if (resolveRet != ERR_OK && resolveRet != AAFwk::RESOLVE_ABILITY_ERR) {
        return resolveRet;
    }

    int32_t cardRet = AgentCardMgr::GetInstance().GetAgentCardByAgentId(
        target.connectWant.GetBundle(), target.agentId, target.card);
    if (cardRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no such card");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }

    if (resolveRet != ERR_OK) {
        bool isInstallOnDemand = target.card.type == AgentCardType::ATOMIC_SERVICE &&
            !AgentCardUtils::BundleExists(target.connectWant.GetBundle(), target.userId);
        if (!isInstallOnDemand) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
            return AAFwk::RESOLVE_ABILITY_ERR;
        }
        NormalizeConnectAgentTarget(target.connectWant, target.card);
    }

    if (!IsMatchedAgentCardTarget(target.connectWant, target.card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "want target does not match AgentCard");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    if (target.card.type == AgentCardType::LOW_CODE &&
        !IsLowCodeTargetMatched(target.connectWant, target.card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code target mismatch");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ResolveAgentExtensionAbility(AgentResolvedTarget &target) const
{
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            target.connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION,
            target.userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    NormalizeConnectAgentTarget(target, extensionInfos[0]);
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectServiceRequest(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection) const
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }

    if (callerToken == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller token is null");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AgentManagerService::PrepareServiceConnectWant(const AAFwk::Want &want, AAFwk::Want &connectWant) const
{
    connectWant = want;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    auto userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "service extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::SERVICE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension type");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    return ERR_OK;
}

int32_t AgentManagerService::RegisterTrackedConnectionAndGetServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
    bool countTowardsCallerLimit, sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    // Register the caller callback and allocate the service-side wrapper connection.
    int32_t ret = ERR_OK;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        ret = TryRegisterConnectionLocked(connection, callerIdentity, nullptr, nullptr, countTowardsCallerLimit);
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register tracked connection failed: %{public}d", ret);
        return ret;
    }

    // Fetch the wrapper connection that was just installed into tracked state.
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
        if (it == trackedConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "tracked connection missing after register");
            return AAFwk::INVALID_PARAMETERS_ERR;
        }
        serviceConnection = it->second.serviceConnection;
    }
    return ERR_OK;
}

int32_t AgentManagerService::RegisterTrackedConnectionAndGetServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid, bool countTowardsCallerLimit,
    sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    return RegisterTrackedConnectionAndGetServiceConnection(connection, AgentCallerIdentity(callerUid),
        countTowardsCallerLimit, serviceConnection);
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }

    // Validate connection object
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    AgentDisconnectPlan plan;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto ret = PrepareAgentDisconnectLocked(connection, plan);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    if (plan.notifyCallerDisconnect && plan.callerConnection != nullptr) {
        plan.callerConnection->OnAbilityDisconnectDone(plan.cachedElement, plan.cachedResultCode);
        HandleConnectionDone(plan.callerConnection, plan.cachedResultCode, true);
        return ERR_OK;
    }
    if (plan.serviceConnection == nullptr && plan.hostConnection == nullptr) {
        return ERR_OK;
    }

    sptr<AAFwk::IAbilityConnection> disconnectConnection = plan.serviceConnection;
    if (plan.hostConnection != nullptr) {
        disconnectConnection = plan.hostConnection;
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(disconnectConnection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        RollbackAgentDisconnectLocked(plan);
        return ret;
    }

    return ERR_OK;
}

sptr<IRemoteObject> AgentManagerService::GetConnectionIdentityRemote(
    const sptr<AAFwk::IAbilityConnection> &connection) const
{
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->AsObject();
}

AgentManagerService::TrackedConnectionIter AgentManagerService::FindTrackedConnectionLocked(
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity)
{
    auto end = trackedConnections_.end();
    if (connection == nullptr) {
        return end;
    }
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        return end;
    }

    auto it = trackedConnections_.find(callerRemote);
    if (it != end) {
        return it;
    }

    // IPC may reconstruct a fresh proxy wrapper for the same remote callback object on disconnect.
    // When that happens, fall back only to the single tracked standard connection owned by this caller identity.
    // Low-code sessions carry per-caller ownership semantics and must not be resolved through fallback.
    auto matched = end;
    for (auto iter = trackedConnections_.begin(); iter != end; ++iter) {
        if (!(iter->second.callerIdentity == callerIdentity)) {
            continue;
        }
        if (iter->second.isLowCode) {
            continue;
        }
        if (matched != end) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Multiple tracked standard connections exist for caller identity");
            return end;
        }
        matched = iter;
    }

    if (matched != end) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Resolved tracked standard connection by caller identity fallback");
    }
    return matched;
}

AgentManagerService::TrackedConnectionIter AgentManagerService::FindTrackedConnectionLocked(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid)
{
    return FindTrackedConnectionLocked(connection, AgentCallerIdentity(callerUid));
}

int32_t AgentManagerService::PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    AgentDisconnectPlan &plan)
{
    AgentCallerIdentity callerIdentity = GetCurrentCallerIdentity();
    auto it = FindTrackedConnectionLocked(connection, callerIdentity);
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
        return ERR_INVALID_VALUE;
    }

    plan.callerRemote = it->first;
    if (it->second.isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
        return ERR_OK;
    }
    if (it->second.isLowCode) {
        return PrepareLowCodeDisconnectLocked(it, plan);
    }
    return PrepareStandardDisconnectLocked(it, plan);
}

int32_t AgentManagerService::PrepareLowCodeDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan)
{
    auto sessionIter = FindAgentHostSessionLocked(it->second.hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->session == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
        return ERR_INVALID_VALUE;
    }

    auto session = sessionIter->session;
    if (session->isDisconnecting) {
        return ERR_OK;
    }
    it->second.isDisconnecting = true;
    plan.hostKey = it->second.hostKey;
    plan.hasHostKey = true;
    auto callerConnectionIt = session->callerConnections.find(it->first);
    if (callerConnectionIt != session->callerConnections.end()) {
        plan.callerConnection = callerConnectionIt->second;
    }
    std::vector<std::string> removedAgentIds;
    sptr<IRemoteObject> hostProxyRemote = nullptr;
    session->RemoveAgentsByCaller(it->first, removedAgentIds, hostProxyRemote);
    for (const auto &agentId : removedAgentIds) {
        EraseAgentOwnerLocked(AgentOwnerKey { it->second.callerIdentity, agentId });
    }
    session->RemoveCaller(it->first, hostProxyRemote, false);
    if (session->agents.empty()) {
        session->MarkBackingDisconnecting();
        plan.hostConnection = session->GetHostConnection(hostProxyRemote);
    } else {
        session->MarkConnected();
        plan.cachedElement = session->element;
        plan.cachedResultCode = ERR_OK;
        plan.notifyCallerDisconnect = true;
    }
    if (!ReleaseCallerConnectionCountLocked(plan.callerRemote)) {
        session->MarkDisconnectFailed();
        it->second.isDisconnecting = false;
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AgentManagerService::PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan)
{
    const auto &stdKey = it->second.standardKey;
    if (GetStandardCallerIdentity(stdKey).IsValid() && !GetStandardAgentId(stdKey).empty()) {
        auto sessionIt = FindStandardSessionIterLocked(stdKey);
        if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
            return PrepareStandardSessionDisconnectLocked(it, *sessionIt, plan);
        }
    }

    it->second.isDisconnecting = true;
    if (!ReleaseCallerConnectionCountLocked(plan.callerRemote)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
        return ERR_INVALID_VALUE;
    }
    plan.serviceConnection = it->second.serviceConnection;
    return ERR_OK;
}

AgentManagerService::StandardSessionIter AgentManagerService::FindStandardSessionIterLocked(
    const StandardAgentKey &key)
{
    for (auto iter = standardSessions_.begin(); iter != standardSessions_.end(); ++iter) {
        if (*iter != nullptr && IsStandardAgentKeyEqual((*iter)->key, key)) {
            return iter;
        }
    }
    return standardSessions_.end();
}

int32_t AgentManagerService::PrepareStandardSessionDisconnectLocked(TrackedConnectionIter it,
    const std::shared_ptr<AgentStandardRecord> &session, AgentDisconnectPlan &plan)
{
    if (session->isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "standard session already disconnecting");
        return ERR_OK;
    }
    it->second.isDisconnecting = true;
    plan.serviceConnection = it->second.serviceConnection;
    plan.isStandard = true;
    plan.standardCallerUid = GetStandardCallerIdentity(session->key).uid;
    plan.standardCallerIdentity = GetStandardCallerIdentity(session->key);
    plan.standardKey = session->key;
    if (session->callerRemotes.size() > 1) {
        session->MarkDisconnectRequested();
        if (plan.serviceConnection != nullptr) {
            session->pendingDisconnectServiceRemotes.insert(plan.serviceConnection->AsObject());
        }
        return ERR_OK;
    }
    session->MarkBackingDisconnecting();
    ReleaseCallerConnectionCountByIdentityLocked(plan.standardCallerIdentity);
    return ERR_OK;
}

void AgentManagerService::RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    auto it = trackedConnections_.find(plan.callerRemote);
    if (plan.hasHostKey) {
        auto sessionIter = FindAgentHostSessionLocked(plan.hostKey);
        if (sessionIter != agentHostSessions_.end() && sessionIter->session != nullptr) {
            sessionIter->session->MarkDisconnectFailed();
        }
    }
    if (plan.isStandard) {
        auto sessionIt = FindStandardSessionIterLocked(plan.standardKey);
        if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
            (*sessionIt)->MarkDisconnectFailed();
        }
        if (plan.standardCallerIdentity.IsValid() && sessionIt != standardSessions_.end() &&
            *sessionIt != nullptr && (*sessionIt)->callerRemotes.size() <= 1) {
            callerConnectionCounts_[plan.standardCallerIdentity]++;
        }
    }
    if (it != trackedConnections_.end() && it->second.isDisconnecting) {
        it->second.isDisconnecting = false;
        if (!plan.isStandard) {
            callerConnectionCounts_[it->second.callerIdentity]++;
        }
    }
}

int32_t AgentManagerService::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid callerToken or connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    (void)callerToken;

    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(connection->AsObject());
        if (it == trackedConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
            return ERR_INVALID_VALUE;
        }
        if (it->second.isDisconnecting) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
            return ERR_OK;
        }
        it->second.isDisconnecting = true;
        if (it->second.countTowardsCallerLimit && !ReleaseCallerConnectionCountLocked(it->first)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
            return ERR_INVALID_VALUE;
        }
        serviceConnection = it->second.serviceConnection;
    }

    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(connection->AsObject());
        if (it != trackedConnections_.end() && it->second.isDisconnecting) {
            it->second.isDisconnecting = false;
            if (it->second.countTowardsCallerLimit) {
                callerConnectionCounts_[it->second.callerIdentity]++;
            }
        }
        return ret;
    }
    return ERR_OK;
}
bool AgentManagerService::HasReachedCallerConnectionLimitLocked(const AgentCallerIdentity &callerIdentity) const
{
    auto countIt = callerConnectionCounts_.find(callerIdentity);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    return countIt->second >= MAX_CONNECTIONS_PER_CALLER;
}

int32_t AgentManagerService::ValidateTrackedConnectionRegistrationLocked(const sptr<IRemoteObject> &callerRemote,
    const AgentCallerIdentity &callerIdentity, bool countTowardsCallerLimit, size_t &currentCount) const
{
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    if (trackedConnections_.find(callerRemote) != trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked");
        return ERR_INVALID_VALUE;
    }
    auto countIt = callerConnectionCounts_.find(callerIdentity);
    currentCount = countIt != callerConnectionCounts_.end() ? countIt->second : 0;
    if (countTowardsCallerLimit && HasReachedCallerConnectionLimitLocked(callerIdentity)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Maximum agent connections reached for caller identity");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    return ERR_OK;
}

sptr<AAFwk::IAbilityConnection> AgentManagerService::GetOrCreateServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection) const
{
    if (serviceConnection != nullptr) {
        return serviceConnection;
    }
    return sptr<AgentServiceConnection>::MakeSptr(connection);
}

TrackedConnectionRecord AgentManagerService::BuildTrackedConnectionRecord(
    const sptr<IRemoteObject> &callerRemote, const AgentCallerIdentity &callerIdentity,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey,
    bool countTowardsCallerLimit)
{
    TrackedConnectionRecord record;
    record.callerUid = callerIdentity.uid;
    record.callerIdentity = callerIdentity;
    record.serviceConnection = serviceConnection;
    record.callerRemote = callerRemote;
    record.isLowCode = hostKey != nullptr;
    record.countTowardsCallerLimit = countTowardsCallerLimit;
    if (hostKey != nullptr) {
        record.hostKey = *hostKey;
    }
    AddCallerDeathRecipient(record);
    return record;
}

void AgentManagerService::AddCallerDeathRecipient(TrackedConnectionRecord &record)
{
    if (record.callerRemote == nullptr) {
        return;
    }
    auto handler = [service = wptr<AgentManagerService>(AgentManagerService::GetInstance()),
                       callerRemote = record.callerRemote](const wptr<IRemoteObject> &remote) {
        (void)remote;
        auto serviceSptr = service.promote();
        if (serviceSptr != nullptr) {
            serviceSptr->HandleCallerConnectionDied(callerRemote);
        }
    };
    record.deathRecipient = sptr<AAFwk::AbilityConnectCallbackRecipient>::MakeSptr(std::move(handler));
    if (record.deathRecipient != nullptr) {
        record.callerRemote->AddDeathRecipient(record.deathRecipient);
    }
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    const AgentCallerIdentity &callerIdentity, const sptr<AAFwk::IAbilityConnection> &serviceConnection,
    const AgentHostKey *hostKey, bool countTowardsCallerLimit)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    size_t currentCount;
    auto ret = ValidateTrackedConnectionRegistrationLocked(callerRemote, callerIdentity, countTowardsCallerLimit,
        currentCount);
    if (ret != ERR_OK) {
        return ret;
    }

    auto actualServiceConnection = GetOrCreateServiceConnection(connection, serviceConnection);
    if (actualServiceConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
        return ERR_INVALID_VALUE;
    }

    auto record = BuildTrackedConnectionRecord(callerRemote, callerIdentity, actualServiceConnection, hostKey,
        countTowardsCallerLimit);
    trackedConnections_.emplace(callerRemote, record);
    if (countTowardsCallerLimit) {
        callerConnectionCounts_[callerIdentity] = currentCount + 1;
    }
    return ERR_OK;
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid, const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey,
    bool countTowardsCallerLimit)
{
    return TryRegisterConnectionLocked(connection, AgentCallerIdentity(callerUid), serviceConnection, hostKey,
        countTowardsCallerLimit);
}

void AgentManagerService::ReleaseCallerConnectionCountByIdentityLocked(const AgentCallerIdentity &callerIdentity)
{
    auto countIt = callerConnectionCounts_.find(callerIdentity);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

bool AgentManagerService::ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return false;
    }
    if (!it->second.countTowardsCallerLimit) {
        return true;
    }
    auto countIt = callerConnectionCounts_.find(it->second.callerIdentity);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    ReleaseCallerConnectionCountByIdentityLocked(it->second.callerIdentity);
    return true;
}

void AgentManagerService::ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (connection == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(connectionLock_);
    auto identityRemote = GetConnectionIdentityRemote(connection);
    auto it = trackedConnections_.find(identityRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    auto callerIdentity = it->second.callerIdentity;
    auto countTowardsCallerLimit = it->second.countTowardsCallerLimit;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }
    if (!countTowardsCallerLimit) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(callerIdentity);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    auto callerIdentity = it->second.callerIdentity;
    auto countTowardsCallerLimit = it->second.countTowardsCallerLimit;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }
    if (!countTowardsCallerLimit) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(callerIdentity);
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::TransferLowCodeCallerLimitLocked(const std::shared_ptr<AgentHostSession> &session,
    const sptr<IRemoteObject> &callerRemote)
{
    if (session == nullptr || callerRemote == nullptr) {
        return;
    }
    auto currentIter = trackedConnections_.find(callerRemote);
    if (currentIter == trackedConnections_.end() || !currentIter->second.countTowardsCallerLimit) {
        return;
    }
    for (const auto &connectionEntry : session->callerConnections) {
        const auto &candidateRemote = connectionEntry.first;
        if (candidateRemote == nullptr || candidateRemote == callerRemote) {
            continue;
        }
        auto candidateIter = trackedConnections_.find(candidateRemote);
        if (candidateIter == trackedConnections_.end() || !candidateIter->second.isLowCode ||
            !IsLowCodeHostKeyMatched(candidateIter->second.hostKey, currentIter->second.hostKey) ||
            !(candidateIter->second.callerIdentity == currentIter->second.callerIdentity) ||
            candidateIter->second.countTowardsCallerLimit) {
            continue;
        }
        candidateIter->second.countTowardsCallerLimit = true;
        currentIter->second.countTowardsCallerLimit = false;
        return;
    }
}

bool AgentManagerService::HasLowCodeCallerIdentityLocked(const std::shared_ptr<AgentHostSession> &session,
    const AgentCallerIdentity &callerIdentity) const
{
    if (session == nullptr) {
        return false;
    }
    for (const auto &trackedEntry : trackedConnections_) {
        const auto &record = trackedEntry.second;
        if (record.isLowCode && IsLowCodeHostKeyMatched(record.hostKey, session->key) &&
            record.callerIdentity == callerIdentity) {
            return true;
        }
    }
    for (const auto &agentEntry : session->agents) {
        auto trackedIt = trackedConnections_.find(agentEntry.second.callerRemote);
        if (trackedIt != trackedConnections_.end() && trackedIt->second.isLowCode &&
            trackedIt->second.callerIdentity == callerIdentity) {
            return true;
        }
    }
    return false;
}

bool AgentManagerService::IsLowCodeHostKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const
{
    AgentTargetInfo leftTarget { left.userId, left.bundleName, left.moduleName, left.abilityName };
    AgentTargetInfo rightTarget { right.userId, right.bundleName, right.moduleName, right.abilityName };
    return AgentBaseRecord::IsSameTarget(leftTarget, rightTarget) &&
        left.targetUid == right.targetUid && left.appIndex == right.appIndex &&
        left.targetInstanceKey == right.targetInstanceKey;
}

bool AgentManagerService::IsAgentOwnerKeyEqual(const AgentOwnerKey &left, const AgentOwnerKey &right) const
{
    return left.callerIdentity == right.callerIdentity && left.agentId == right.agentId;
}

AgentManagerService::AgentHostSessionIter AgentManagerService::FindAgentHostSessionLocked(const AgentHostKey &key)
{
    for (auto iter = agentHostSessions_.begin(); iter != agentHostSessions_.end(); ++iter) {
        if (iter->session != nullptr && iter->session->Matches(key)) {
            return iter;
        }
        if (iter->session == nullptr && AgentLowCodeRecord(iter->key).Matches(key)) {
            return iter;
        }
    }
    return agentHostSessions_.end();
}

AgentManagerService::AgentOwnerIter AgentManagerService::FindAgentOwnerLocked(const AgentOwnerKey &key)
{
    for (auto iter = agentOwners_.begin(); iter != agentOwners_.end(); ++iter) {
        if (IsAgentOwnerKeyEqual(iter->key, key)) {
            return iter;
        }
    }
    return agentOwners_.end();
}

void AgentManagerService::SetAgentOwnerLocked(const AgentOwnerKey &key,
    const std::shared_ptr<AgentHostSession> &session)
{
    auto ownerIter = FindAgentOwnerLocked(key);
    if (ownerIter != agentOwners_.end()) {
        ownerIter->session = session;
        return;
    }
    agentOwners_.push_back(AgentOwnerRecord { key, session });
}

void AgentManagerService::EraseAgentOwnerLocked(const AgentOwnerKey &key)
{
    auto ownerIter = FindAgentOwnerLocked(key);
    if (ownerIter != agentOwners_.end()) {
        agentOwners_.erase(ownerIter);
    }
}

void AgentManagerService::CleanupDeadStandardConnectionLocked(TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    serviceConnection = it->second.serviceConnection;
    auto &stdKey = it->second.standardKey;
    if (GetStandardCallerIdentity(stdKey).IsValid() && !GetStandardAgentId(stdKey).empty()) {
        auto sessionIt = FindStandardSessionIterLocked(stdKey);
        if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
            CleanupDeadStandardSessionLocked(sessionIt, it, remote, serviceConnection);
            return;
        }
    }
    ReleaseTrackedConnectionByRemoteLocked(remote);
}

void AgentManagerService::CleanupDeadStandardSessionLocked(StandardSessionIter sessionIt, TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    auto &session = *sessionIt;
    bool isLastOwner = session->callerRemotes.size() <= 1;
    auto deadServiceConnection = it->second.serviceConnection;
    // Caller death removes only that owner; the shared backing session ends only after the last owner.
    if (isLastOwner) {
        session->MarkBackingDisconnecting();
        serviceConnection = session->serviceConnection != nullptr ?
            session->serviceConnection : deadServiceConnection;
        RemoveStandardSessionCallerLocked(session, remote, deadServiceConnection);
        session->MarkDisconnected();
        standardSessions_.erase(sessionIt);
        return;
    }
    serviceConnection = deadServiceConnection;
    if (serviceConnection != nullptr) {
        session->pendingDisconnectServiceRemotes.insert(serviceConnection->AsObject());
    }
    RemoveStandardSessionCallerLocked(session, remote, deadServiceConnection);
    session->MarkConnected();
}

void AgentManagerService::CleanupDeadLowCodeConnectionLocked(TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, sptr<AgentHostConnection> &hostConnection)
{
    auto sessionIter = FindAgentHostSessionLocked(it->second.hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->session != nullptr) {
        auto session = sessionIter->session;
        session->callerConnections.erase(remote);
        for (auto agentIter = session->agents.begin(); agentIter != session->agents.end();) {
            if (agentIter->second.callerRemote == remote) {
                EraseAgentOwnerLocked(AgentOwnerKey { it->second.callerIdentity, agentIter->first });
                agentIter = session->agents.erase(agentIter);
                continue;
            }
            ++agentIter;
        }
        if (!session->isDisconnecting && session->agents.empty()) {
            session->MarkBackingDisconnecting();
            hostConnection = session->hostConnection;
        } else {
            TransferLowCodeCallerLimitLocked(session, remote);
        }
    }
    ReleaseTrackedConnectionByRemoteLocked(remote);
}

void AgentManagerService::DisconnectDeadServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    if (serviceConnection != nullptr) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
}

void AgentManagerService::DisconnectDeadHostConnection(const sptr<AgentHostConnection> &hostConnection)
{
    if (hostConnection == nullptr) {
        return;
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
    if (ret == ERR_OK) {
        return;
    }
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto sessionIter = std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
        [hostConnection](const auto &item) {
            return item.session != nullptr && item.session->hostConnection == hostConnection;
        });
    if (sessionIter != agentHostSessions_.end() && sessionIter->session != nullptr) {
        sessionIter->session->MarkDisconnectFailed();
    }
    TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
}

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<AgentHostConnection> hostConnection = nullptr;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        if (remote == nullptr) {
            return;
        }
        auto it = trackedConnections_.find(remote);
        if (it == trackedConnections_.end()) {
            return;
        }
        if (it->second.isLowCode) {
            CleanupDeadLowCodeConnectionLocked(it, remote, hostConnection);
        } else {
            CleanupDeadStandardConnectionLocked(it, remote, serviceConnection);
        }
    }

    DisconnectDeadServiceConnection(serviceConnection);
    DisconnectDeadHostConnection(hostConnection);
}

void AgentManagerService::HandleCallerConnectionDied(const wptr<IRemoteObject> &remote)
{
    auto remoteObject = remote.promote();
    if (remoteObject == nullptr) {
        return;
    }
    HandleCallerConnectionDied(remoteObject);
}

void AgentManagerService::HandleConnectionDone(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect)
{
    if (isDisconnect || resultCode != ERR_OK) {
        ReleaseTrackedConnection(connection);
    }
}

int32_t AgentManagerService::ValidateNotifyLowCodeAgentCompleteRequest(const std::string &agentId) const
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AgentManagerService::PrepareNotifyLowCodeAgentCompleteLocked(const std::string &agentId,
    sptr<AgentHostConnection> &hostConnection, sptr<AAFwk::IAbilityConnection> &callerConnection,
    AppExecFwk::ElementName &element)
{
    AgentOwnerIter ownerIter;
    std::shared_ptr<AgentHostSession> session;
    LowCodeAgentRecord agentRecord;
    auto ret = ResolveNotifyLowCodeAgentLocked(agentId, ownerIter, session, agentRecord);
    if (ret != ERR_OK) {
        return ret;
    }
    bool callerStillOwnsAgent = session->HasOtherAgentsForCaller(
        agentId, agentRecord.callerRemote, agentRecord.hostProxyRemote);
    bool isLastAgent = session->agents.size() <= 1;

    PrepareLowCodeCompleteCallerCallbackLocked(
        session, agentRecord, callerStillOwnsAgent, isLastAgent, callerConnection, element);
    session->RemoveAgent(agentId);
    agentOwners_.erase(ownerIter);

    ReleaseLowCodeCompleteCallerLocked(session, agentRecord, callerStillOwnsAgent, isLastAgent);
    return PrepareLowCodeCompleteBackingDisconnectLocked(session, agentRecord, callerStillOwnsAgent, hostConnection);
}

int32_t AgentManagerService::ResolveNotifyLowCodeAgentLocked(const std::string &agentId,
    AgentOwnerIter &ownerIter, std::shared_ptr<AgentHostSession> &session, LowCodeAgentRecord &agentRecord)
{
    AgentOwnerKey ownerKey { GetCurrentCallerIdentity(), agentId };
    ownerIter = FindAgentOwnerLocked(ownerKey);
    if (ownerIter == agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    session = ownerIter->session;
    if (session == nullptr) {
        agentOwners_.erase(ownerIter);
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    auto agentIter = session->agents.find(agentId);
    if (agentIter == session->agents.end()) {
        agentOwners_.erase(ownerIter);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent bookkeeping missing");
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    agentRecord = agentIter->second;
    return ERR_OK;
}

void AgentManagerService::PrepareLowCodeCompleteCallerCallbackLocked(
    const std::shared_ptr<AgentHostSession> &session, const LowCodeAgentRecord &agentRecord,
    bool callerStillOwnsAgent, bool isLastAgent, sptr<AAFwk::IAbilityConnection> &callerConnection,
    AppExecFwk::ElementName &element)
{
    if (callerStillOwnsAgent || isLastAgent) {
        return;
    }
    auto callerConnectionIt = session->callerConnections.find(agentRecord.callerRemote);
    if (callerConnectionIt != session->callerConnections.end()) {
        callerConnection = callerConnectionIt->second;
        element = session->element;
    }
}

void AgentManagerService::ReleaseLowCodeCompleteCallerLocked(const std::shared_ptr<AgentHostSession> &session,
    const LowCodeAgentRecord &agentRecord, bool callerStillOwnsAgent, bool isLastAgent)
{
    if (callerStillOwnsAgent || agentRecord.callerRemote == nullptr) {
        return;
    }
    session->RemoveCaller(agentRecord.callerRemote, agentRecord.hostProxyRemote, false);
    if (!isLastAgent) {
        session->disconnectingCallbacks.erase(agentRecord.hostProxyRemote);
    }
    TransferLowCodeCallerLimitLocked(session, agentRecord.callerRemote);
    ReleaseTrackedConnectionByRemoteLocked(agentRecord.callerRemote);
}

int32_t AgentManagerService::PrepareLowCodeCompleteBackingDisconnectLocked(
    const std::shared_ptr<AgentHostSession> &session, const LowCodeAgentRecord &agentRecord,
    bool callerStillOwnsAgent, sptr<AgentHostConnection> &hostConnection)
{
    if (callerStillOwnsAgent || session->isDisconnecting) {
        return ERR_OK;
    }
    if (!session->agents.empty()) {
        session->MarkConnected();
        return ERR_OK;
    }
    session->MarkBackingDisconnecting();
    hostConnection = session->GetHostConnection(agentRecord.hostProxyRemote);
    return ERR_OK;
}

void AgentManagerService::RollbackNotifyLowCodeAgentCompleteDisconnect(const sptr<AgentHostConnection> &hostConnection)
{
    std::lock_guard<std::mutex> lock(agentHostMutex_);
    auto sessionIter = std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
        [hostConnection](const auto &item) {
            return item.session != nullptr && item.session->hostConnection == hostConnection;
        });
    if (sessionIter != agentHostSessions_.end()) {
        sessionIter->session->MarkDisconnectFailed();
    }
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    auto ret = ValidateNotifyLowCodeAgentCompleteRequest(agentId);
    if (ret != ERR_OK) {
        return ret;
    }
    sptr<AgentHostConnection> hostConnection = nullptr;
    sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
    AppExecFwk::ElementName element;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        ret = PrepareNotifyLowCodeAgentCompleteLocked(agentId, hostConnection, callerConnection, element);
    }
    if (ret != ERR_OK) {
        return ret;
    }
    if (callerConnection != nullptr) {
        callerConnection->OnAbilityDisconnectDone(element, ERR_OK);
        return ERR_OK;
    }
    if (hostConnection == nullptr) {
        return ERR_OK;
    }
    ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
    if (ret != ERR_OK) {
        RollbackNotifyLowCodeAgentCompleteDisconnect(hostConnection);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AgentResolvedTarget &target,
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity)
{
    // Step 1: create or reuse the shared low-code host session bookkeeping.
    AgentConnectPlan plan;
    auto ret = PrepareLowCodeConnectPlan(
        BuildAgentHostKey(target), target.targetUid, target.agentId, connection, callerIdentity, plan);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: if the host is already connected, notify the caller immediately.
    if (plan.notifyExistingConnection) {
        NotifyExistingLowCodeConnection(plan, target.agentId, connection);
        return ERR_OK;
    }

    // Step 3: otherwise connect the shared host session once through AMS.
    if (!plan.needRealConnect) {
        return ERR_OK;
    }
    return CompleteAgentHostConnect(target.connectWant, target.agentId, plan);
}

int32_t AgentManagerService::PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection,
    const AgentCallerIdentity &callerIdentity, AgentConnectPlan &plan)
{
    plan.hostKey = hostKey;
    plan.hostUid = hostUid;
    plan.callerIdentity = callerIdentity;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);

    std::shared_ptr<AgentHostSession> session;
    sptr<IRemoteObject> hostProxyRemote = nullptr;
    auto ret = ValidateLowCodeConnectPlanLocked(agentId, connection, callerIdentity, hostKey, plan, hostProxyRemote);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = GetOrCreateLowCodeSessionLocked(hostKey, hostUid, hostProxyRemote, plan, session);
    if (ret != ERR_OK) {
        return ret;
    }
    if (FindAgentOwnerLocked(AgentOwnerKey { callerIdentity, agentId }) != agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent already active");
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }
    if (session->HasAgent(agentId)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent already active in host session");
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }
    ret = EnsureLowCodeHostProxyConnectionLocked(hostKey, hostProxyRemote, plan, session);
    if (ret != ERR_OK) {
        return ret;
    }

    bool countTowardsCallerLimit = !HasLowCodeCallerIdentityLocked(session, callerIdentity);
    ret = TryRegisterConnectionLocked(connection, callerIdentity, session->hostProxyConnections[hostProxyRemote],
        &hostKey, countTowardsCallerLimit);
    if (ret != ERR_OK) {
        RollbackLowCodeConnectRegistrationLocked(hostKey, hostProxyRemote, session, plan);
        return ret;
    }
    RegisterLowCodeAgentLocked(agentId, connection, callerIdentity, hostProxyRemote, session, plan);
    return ERR_OK;
}

int32_t AgentManagerService::ValidateLowCodeConnectPlanLocked(const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
    const AgentHostKey &hostKey, AgentConnectPlan &plan, sptr<IRemoteObject> &hostProxyRemote)
{
    plan.callerRemote = GetConnectionIdentityRemote(connection);
    if (plan.callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    hostProxyRemote = GetLowCodeHostProxyIdentity(hostKey, plan.callerRemote);
    if (hostProxyRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Host proxy remote object is null");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t AgentManagerService::GetOrCreateLowCodeSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
    const sptr<IRemoteObject> &hostProxyRemote, AgentConnectPlan &plan,
    std::shared_ptr<AgentHostSession> &session)
{
    auto sessionIter = FindAgentHostSessionLocked(hostKey);
    if (sessionIter != agentHostSessions_.end()) {
        session = sessionIter->session;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
        } else if (session->HasDeadCachedRemoteObject()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code cached remote is dead, reconnect host");
            ClearLowCodeHostSessionLocked(sessionIter);
            session = nullptr;
        }
    }
    if (session == nullptr) {
        session = std::make_shared<AgentHostSession>(hostKey);
        session->hostUid = hostUid;
        if (session->CreateHostConnection(hostKey, hostProxyRemote) != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
            return ERR_INVALID_VALUE;
        }
        agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
        plan.needRealConnect = true;
        return ERR_OK;
    }
    if (!session->IsReusable()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code host session is disconnecting");
        return ERR_INVALID_VALUE;
    }
    if (!session->HasCapacity(MAX_AGENTS_PER_HOST_SESSION)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent host session limit reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    return ERR_OK;
}

int32_t AgentManagerService::EnsureLowCodeHostProxyConnectionLocked(const AgentHostKey &hostKey,
    const sptr<IRemoteObject> &hostProxyRemote, AgentConnectPlan &plan,
    const std::shared_ptr<AgentHostSession> &session)
{
    if (session == nullptr || session->HasHostProxyConnection(hostProxyRemote)) {
        return ERR_OK;
    }
    if (session->hostConnection != nullptr) {
        session->AddHostConnection(hostProxyRemote, session->hostConnection);
        return ERR_OK;
    }
    if (session->CreateHostConnection(hostKey, hostProxyRemote) != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host proxy connection failed");
        return ERR_INVALID_VALUE;
    }
    plan.needRealConnect = true;
    return ERR_OK;
}

void AgentManagerService::RollbackLowCodeConnectRegistrationLocked(const AgentHostKey &hostKey,
    const sptr<IRemoteObject> &hostProxyRemote, const std::shared_ptr<AgentHostSession> &session,
    const AgentConnectPlan &plan)
{
    if (!plan.needRealConnect || session == nullptr) {
        return;
    }
    session->hostProxyConnections.erase(hostProxyRemote);
    if (!session->callerConnections.empty() || !session->agents.empty()) {
        return;
    }
    auto cleanupIter = FindAgentHostSessionLocked(hostKey);
    if (cleanupIter != agentHostSessions_.end()) {
        agentHostSessions_.erase(cleanupIter);
    }
}

void AgentManagerService::RegisterLowCodeAgentLocked(const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
    const sptr<IRemoteObject> &hostProxyRemote, const std::shared_ptr<AgentHostSession> &session,
    AgentConnectPlan &plan)
{
    plan.registeredTrackedConnection = true;
    bool isPending = !session->isConnected || session->GetCachedRemoteObject() == nullptr;
    session->AddAgent(agentId, plan.callerRemote, connection, hostProxyRemote, isPending);
    SetAgentOwnerLocked(AgentOwnerKey { callerIdentity, agentId }, session);
    plan.hostConnection = session->GetHostConnection(hostProxyRemote);
    plan.notifyExistingConnection = !isPending && session->GetCachedRemoteObject() != nullptr;
    if (plan.notifyExistingConnection) {
        plan.cachedElement = session->element;
        plan.cachedRemoteObject = session->GetCachedRemoteObject();
        plan.cachedResultCode = session->resultCode;
    }
}

void AgentManagerService::NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    connection->OnAbilityConnectDone(plan.cachedElement, plan.cachedRemoteObject, plan.cachedResultCode);
    bool invoked = AgentLowCodeRecord::NotifyAgentInvoked(plan.cachedRemoteObject, agentId);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "notify existing low-code invoked:%{public}d", invoked);
}

void AgentManagerService::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    EraseAgentOwnerLocked(AgentOwnerKey { plan.callerIdentity, agentId });
    auto sessionIter = FindAgentHostSessionLocked(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->session != nullptr) {
        auto session = sessionIter->session;
        session->RemoveAgent(agentId);
        if (plan.callerRemote != nullptr) {
            session->callerConnections.erase(plan.callerRemote);
        }
        if (session->callerConnections.empty() && session->agents.empty()) {
            agentHostSessions_.erase(sessionIter);
        } else {
            TransferLowCodeCallerLimitLocked(session, plan.callerRemote);
        }
    }
    if (plan.registeredTrackedConnection && plan.callerRemote != nullptr) {
        ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
    }
}

int32_t AgentManagerService::CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, plan.hostConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        CleanupLowCodeConnectPlan(plan, agentId);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

AgentHostKey AgentManagerService::BuildAgentHostKey(const AgentResolvedTarget &target) const
{
    AgentHostKey key;
    key.userId = target.userId;
    key.targetUid = target.targetUid;
    key.appIndex = target.targetAppIndex;
    key.bundleName = target.connectWant.GetElement().GetBundleName();
    key.moduleName = target.connectWant.GetElement().GetModuleName();
    key.abilityName = target.connectWant.GetElement().GetAbilityName();
    key.targetInstanceKey = target.targetInstanceKey;
    return key;
}

void AgentManagerService::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        auto trackedIt = trackedConnections_.find(agentEntry.second.callerRemote);
        if (trackedIt != trackedConnections_.end()) {
            EraseAgentOwnerLocked(AgentOwnerKey { trackedIt->second.callerIdentity, agentEntry.first });
        }
    }
}

void AgentManagerService::ClearLowCodeHostSessionLocked(AgentHostSessionIter sessionIter)
{
    if (sessionIter == agentHostSessions_.end()) {
        return;
    }
    auto session = sessionIter->session;
    if (session == nullptr) {
        agentHostSessions_.erase(sessionIter);
        return;
    }
    std::vector<sptr<IRemoteObject>> callerRemotes;
    for (const auto &callerEntry : session->callerConnections) {
        callerRemotes.emplace_back(callerEntry.first);
    }
    for (const auto &agentEntry : session->agents) {
        callerRemotes.emplace_back(agentEntry.second.callerRemote);
    }
    EraseAgentOwnersLocked(*session);
    for (const auto &callerRemote : callerRemotes) {
        ReleaseTrackedConnectionByRemoteLocked(callerRemote);
    }
    agentHostSessions_.erase(sessionIter);
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> pendingAgentIds;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        if (!PrepareAgentHostConnectDoneLocked(key, element, remoteObject, resultCode, callbacks, pendingAgentIds)) {
            return;
        }
    }
    NotifyLowCodeConnectDoneCallbacks(element, remoteObject, resultCode, callbacks, pendingAgentIds);
}

bool AgentManagerService::PrepareAgentHostConnectDoneLocked(const AgentHostKey &key,
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks, std::vector<std::string> &pendingAgentIds)
{
    auto sessionIter = FindAgentHostSessionLocked(key);
    if (sessionIter == agentHostSessions_.end()) {
        return false;
    }
    auto session = sessionIter->session;
    if (session == nullptr) {
        agentHostSessions_.erase(sessionIter);
        return false;
    }
    session->CacheConnectDone(element, remoteObject, resultCode);
    session->CollectPendingCallbacks(nullptr, callbacks, pendingAgentIds);
    if (resultCode != ERR_OK || remoteObject == nullptr) {
        for (const auto &agentId : pendingAgentIds) {
            auto agentIter = session->agents.find(agentId);
            if (agentIter == session->agents.end()) {
                continue;
            }
            auto trackedIt = trackedConnections_.find(agentIter->second.callerRemote);
            if (trackedIt != trackedConnections_.end()) {
                EraseAgentOwnerLocked(AgentOwnerKey { trackedIt->second.callerIdentity, agentId });
            }
        }
        session->CleanupFailedConnect(nullptr, pendingAgentIds);
        if (session->agents.empty()) {
            ClearAgentHostSessionLocked(key);
        } else {
            session->MarkConnected();
        }
    }
    return true;
}

void AgentManagerService::NotifyLowCodeConnectDoneCallbacks(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode,
    const std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
    const std::vector<std::string> &pendingAgentIds)
{
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
            HandleConnectionDone(callback, resultCode, false);
        }
    }
    if (resultCode != ERR_OK || remoteObject == nullptr) {
        return;
    }
    for (const auto &agentId : pendingAgentIds) {
        bool invoked = AgentLowCodeRecord::NotifyAgentInvoked(remoteObject, agentId);
        TAG_LOGI(AAFwkTag::SER_ROUTER, "notify low-code invoked:%{public}d", invoked);
    }
}

void AgentManagerService::HandleAgentHostDisconnectDone(const AgentHostKey &key,
    const AppExecFwk::ElementName &element, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = FindAgentHostSessionLocked(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        if (sessionIter->session == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        auto session = sessionIter->session;
        if (resultCode != ERR_OK) {
            session->MarkDisconnectFailed();
            return;
        }
        session->CollectDisconnectCallback(nullptr, callbacks);
        if (session->IsEmptyForRemoval()) {
            session->MarkDisconnected();
            ClearAgentHostSessionLocked(key);
        } else {
            session->MarkConnected();
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityDisconnectDone(element, resultCode);
            HandleConnectionDone(callback, resultCode, true);
        }
    }
}

void AgentManagerService::ClearAgentHostSessionLocked(const AgentHostKey &key)
{
    auto sessionIter = FindAgentHostSessionLocked(key);
    ClearLowCodeHostSessionLocked(sessionIter);
}
}  // namespace AgentRuntime
}  // namespace OHOS
