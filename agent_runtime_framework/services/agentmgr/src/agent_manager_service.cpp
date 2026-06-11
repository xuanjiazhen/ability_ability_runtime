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
        lowCodeAgentConnections_.clear();
        lowCodeOwnersByRemote_.clear();
    }
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

AgentManagerService::AgentCallerIdentity AgentManagerService::GetCurrentCallerIdentity() const
{
    AgentCallerIdentity callerIdentity(IPCSkeleton::GetCallingUid());
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
        IPCSkeleton::GetCallingPid(), processInfo));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "get process for caller identity failed: %{public}d", ret);
        return callerIdentity;
    }
    callerIdentity.instanceKey = processInfo.instanceKey;
    return callerIdentity;
}

AgentManagerService::AgentCallerIdentity AgentManagerService::GetEffectiveCallerIdentity(
    const TrackedConnectionRecord &record) const
{
    if (record.callerIdentity.uid != 0 || !record.callerIdentity.instanceKey.empty()) {
        return record.callerIdentity;
    }
    return AgentCallerIdentity(record.callerUid);
}

std::string AgentManagerService::BuildCallerIdentityStorageKey(const AgentCallerIdentity &callerIdentity) const
{
    return std::to_string(callerIdentity.uid) + "#" + callerIdentity.instanceKey;
}

std::string AgentManagerService::BuildLowCodeOwnerStorageKey(int32_t hostUid, const std::string &agentId) const
{
    return std::to_string(hostUid) + "#" + agentId;
}

std::string AgentManagerService::BuildLowCodeTargetStorageKey(const AAFwk::Want &want, int32_t hostUid) const
{
    const auto &element = want.GetElement();
    return std::to_string(hostUid) + "#" + element.GetBundleName() + "#" + element.GetModuleName() + "#" +
        element.GetAbilityName();
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
    // Step 1: validate caller state before classifying the agent connect request.
    AgentCallerIdentity callerIdentity;
    auto ret = ValidateConnectAgentRequest(connection, callerIdentity);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: resolve the agentId and backing card metadata from the request.
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    int32_t callerUid = callerIdentity.uid;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    callerIdentity.uid = callerUid;

    // Step 3: low-code agents are tracked only by logical agentId; AMS owns lifecycle dispatch.
    if (card.type == AgentCardType::LOW_CODE) {
        int32_t hostUid = 0;
        ret = ResolveLowCodeHostInfo(connectWant, callerUid / BASE_USER_RANGE, hostUid);
        if (ret != ERR_OK) {
            return ret;
        }
        return ConnectLowCodeAgentExtensionAbility(connectWant, agentId, connection, callerIdentity, hostUid);
    }

    // Step 4: prepare the final Want for standard agent-extension connect.
    ret = PrepareStandardAgentConnectWant(connectWant, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        connectWant.GetBundle().c_str(), agentId.c_str());

    // Step 5: standard agents delegate lifecycle decisions to AMS and the target process.
    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, callerIdentity, true, serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        return ret;
    }
    return ERR_OK;
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
    auto callerIdentity = GetCurrentCallerIdentity();
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, callerIdentity, false, serviceConnection);
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
    callerIdentity.uid = IPCSkeleton::GetCallingUid();
    callerIdentity.instanceKey = processInfo.instanceKey;
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

int32_t AgentManagerService::ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
    std::string &agentId, AgentCard &card, int32_t &callingUid) const
{
    // Copy the incoming Want so later steps can enrich it safely.
    connectWant = want;
    agentId = connectWant.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    int32_t cardRet = AgentCardMgr::GetInstance().GetAgentCardByAgentId(connectWant.GetBundle(), agentId, card);
    if (cardRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no such card");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }

    if (!IsMatchedAgentCardTarget(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "want target does not match AgentCard");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    if (card.type == AgentCardType::LOW_CODE && !IsLowCodeTargetMatched(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code target mismatch");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    NormalizeConnectAgentTarget(connectWant, card);

    // Capture the caller UID once the target metadata is known-good.
    callingUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card,
    int32_t callingUid) const
{
    // Resolve extension metadata unless the card is allowed to free-install on demand.
    bool isAtomicServiceAgent = card.type == AgentCardType::ATOMIC_SERVICE;
    int32_t userId = callingUid / BASE_USER_RANGE;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        if (!isAtomicServiceAgent || AgentCardUtils::BundleExists(connectWant.GetBundle(), userId)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
            return AAFwk::RESOLVE_ABILITY_ERR;
        }
    }
    if (queryResult && !extensionInfos.empty() && extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }

    // Atomic-service agents connect with install-on-demand metadata attached.
    if (isAtomicServiceAgent) {
        connectWant.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        connectWant.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    }
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
        ret = TryRegisterConnectionLocked(connection, callerIdentity, nullptr, countTowardsCallerLimit);
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

int32_t AgentManagerService::RegisterLowCodeConnectionAndGetServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
    const std::string &ownerKey, const std::string &targetKey,
    sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }

    serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection, ownerKey);
    if (serviceConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create low-code service connection failed");
        return ERR_INVALID_VALUE;
    }

    LowCodeConnectionRecord record;
    record.callerIdentity = callerIdentity;
    record.callerPid = IPCSkeleton::GetCallingPid();
    record.targetKey = targetKey;
    record.serviceConnection = serviceConnection;
    record.callerRemote = callerRemote;
    auto handler = [service = wptr<AgentManagerService>(AgentManagerService::GetInstance()),
                       callerRemote](const wptr<IRemoteObject> &remote) {
        (void)remote;
        auto serviceSptr = service.promote();
        if (serviceSptr != nullptr) {
            serviceSptr->HandleCallerConnectionDied(callerRemote);
        }
    };
    record.deathRecipient = sptr<AAFwk::AbilityConnectCallbackRecipient>::MakeSptr(std::move(handler));
    if (record.deathRecipient != nullptr) {
        callerRemote->AddDeathRecipient(record.deathRecipient);
    }

    std::lock_guard<std::mutex> lock(connectionLock_);
    if (lowCodeAgentConnections_.find(ownerKey) != lowCodeAgentConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent already active");
        if (record.deathRecipient != nullptr) {
            callerRemote->RemoveDeathRecipient(record.deathRecipient);
        }
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }
    lowCodeAgentConnections_.emplace(ownerKey, record);
    lowCodeOwnersByRemote_[callerRemote].insert(ownerKey);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "registered low-code owner, active:%{public}zu pid:%{public}d",
        lowCodeAgentConnections_.size(), record.callerPid);
    return ERR_OK;
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
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto ret = PrepareAgentDisconnectLocked(connection, plan);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    if (plan.records.empty()) {
        return ERR_OK;
    }

    for (const auto &record : plan.records) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(
            record.serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
            std::lock_guard<std::mutex> lock(connectionLock_);
            RollbackAgentDisconnectLocked(plan);
            return ret;
        }
    }

    return ERR_OK;
}

int32_t AgentManagerService::ResolveLowCodeHostInfo(const AAFwk::Want &want, int32_t userId, int32_t &hostUid) const
{
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    hostUid = extensionInfos[0].applicationInfo.uid;
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
    // Low-code sessions carry per-caller ownership semantics and must not be resolved via identity alone.
    auto matched = end;
    for (auto iter = trackedConnections_.begin(); iter != end; ++iter) {
        if (!(GetEffectiveCallerIdentity(iter->second) == callerIdentity)) {
            continue;
        }
        if (iter->second.isLowCode) {
            continue;
        }
        if (matched != end) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Multiple tracked connections exist for caller");
            return end;
        }
        matched = iter;
    }

    if (matched != end) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Resolved tracked connection by caller identity fallback");
    }
    return matched;
}

int32_t AgentManagerService::PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    AgentDisconnectPlan &plan)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    auto lowCodeRet = PrepareLowCodeProxyDisconnectLocked(callerRemote, plan);
    if (lowCodeRet != AAFwk::CONNECTION_NOT_EXIST) {
        return lowCodeRet;
    }

    auto it = FindTrackedConnectionLocked(connection, GetCurrentCallerIdentity());
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
        return ERR_INVALID_VALUE;
    }

    if (it->second.isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
        return ERR_OK;
    }
    return PrepareStandardDisconnectLocked(it, plan);
}

int32_t AgentManagerService::PrepareLowCodeProxyDisconnectLocked(
    const sptr<IRemoteObject> &callerRemote, AgentDisconnectPlan &plan)
{
    std::set<std::string> ownerKeys;
    auto ret = CollectLowCodeProxyOwnerKeysLocked(callerRemote, ownerKeys);
    if (ret != ERR_OK) {
        return ret;
    }

    for (const auto &ownerKey : ownerKeys) {
        auto recordIter = lowCodeAgentConnections_.find(ownerKey);
        if (recordIter == lowCodeAgentConnections_.end() || recordIter->second.isDisconnecting) {
            continue;
        }
        recordIter->second.isDisconnecting = true;
        plan.records.push_back(AgentDisconnectRecord {
            recordIter->second.serviceConnection, recordIter->second.callerRemote, ownerKey });
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "prepare low-code proxy disconnect, owners:%{public}zu",
        plan.records.size());
    return ERR_OK;
}

int32_t AgentManagerService::CollectLowCodeProxyOwnerKeysLocked(
    const sptr<IRemoteObject> &callerRemote, std::set<std::string> &ownerKeys)
{
    auto ownerIter = lowCodeOwnersByRemote_.find(callerRemote);
    if (ownerIter == lowCodeOwnersByRemote_.end() || ownerIter->second.empty()) {
        return CollectLowCodeProxyOwnerKeysByCallerLocked(ownerKeys);
    }

    for (const auto &ownerKey : ownerIter->second) {
        auto recordIter = lowCodeAgentConnections_.find(ownerKey);
        if (recordIter == lowCodeAgentConnections_.end() || recordIter->second.isDisconnecting) {
            continue;
        }
        CollectLowCodeOwnerKeysByGroupLocked(recordIter->second, ownerKeys);
        TAG_LOGI(AAFwkTag::SER_ROUTER, "low-code proxy group seed found, owners:%{public}zu", ownerKeys.size());
        return ownerKeys.empty() ? AAFwk::CONNECTION_NOT_EXIST : ERR_OK;
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::CollectLowCodeProxyOwnerKeysByCallerLocked(std::set<std::string> &ownerKeys)
{
    auto callerIdentity = GetCurrentCallerIdentity();
    auto callerPid = IPCSkeleton::GetCallingPid();
    std::set<std::string> targetKeys;
    for (const auto &[ownerKey, record] : lowCodeAgentConnections_) {
        if (record.isDisconnecting || record.callerPid != callerPid) {
            continue;
        }
        if (!IsLowCodeFallbackCallerMatch(callerIdentity, record.callerIdentity)) {
            continue;
        }
        targetKeys.insert(record.targetKey);
    }
    if (targetKeys.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code proxy owner not found by caller fallback");
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    if (targetKeys.size() > 1) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ambiguous low-code proxy owner fallback, targets:%{public}zu",
            targetKeys.size());
        return ERR_INVALID_VALUE;
    }

    LowCodeConnectionRecord seedRecord;
    seedRecord.callerIdentity = callerIdentity;
    seedRecord.callerPid = callerPid;
    seedRecord.targetKey = *targetKeys.begin();
    CollectLowCodeOwnerKeysByGroupLocked(seedRecord, ownerKeys);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "low-code proxy fallback owners:%{public}zu", ownerKeys.size());
    return ownerKeys.empty() ? AAFwk::CONNECTION_NOT_EXIST : ERR_OK;
}

void AgentManagerService::CollectLowCodeOwnerKeysByGroupLocked(const LowCodeConnectionRecord &seedRecord,
    std::set<std::string> &ownerKeys) const
{
    for (const auto &[ownerKey, record] : lowCodeAgentConnections_) {
        if (record.isDisconnecting) {
            continue;
        }
        if (!IsLowCodeFallbackCallerMatch(seedRecord.callerIdentity, record.callerIdentity)) {
            continue;
        }
        if (record.callerPid != seedRecord.callerPid || record.targetKey != seedRecord.targetKey) {
            continue;
        }
        ownerKeys.insert(ownerKey);
    }
}

bool AgentManagerService::IsLowCodeFallbackCallerMatch(const AgentCallerIdentity &currentIdentity,
    const AgentCallerIdentity &recordIdentity) const
{
    if (currentIdentity.uid != recordIdentity.uid) {
        return false;
    }
    if (currentIdentity.instanceKey.empty()) {
        return true;
    }
    return currentIdentity.instanceKey == recordIdentity.instanceKey;
}

int32_t AgentManagerService::PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan)
{
    it->second.isDisconnecting = true;
    if (!ReleaseCallerConnectionCountLocked(it->first)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
        return ERR_INVALID_VALUE;
    }
    plan.records.push_back(AgentDisconnectRecord { it->second.serviceConnection, it->first, "" });
    return ERR_OK;
}

void AgentManagerService::RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    for (const auto &record : plan.records) {
        if (!record.lowCodeOwnerKey.empty()) {
            auto lowCodeIter = lowCodeAgentConnections_.find(record.lowCodeOwnerKey);
            if (lowCodeIter != lowCodeAgentConnections_.end()) {
                lowCodeIter->second.isDisconnecting = false;
            }
            continue;
        }
        auto it = trackedConnections_.find(record.callerRemote);
        if (it != trackedConnections_.end() && it->second.isDisconnecting) {
            it->second.isDisconnecting = false;
            if (it->second.countTowardsCallerLimit) {
                callerConnectionCounts_[BuildCallerIdentityStorageKey(GetEffectiveCallerIdentity(it->second))]++;
            }
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
                callerConnectionCounts_[BuildCallerIdentityStorageKey(GetEffectiveCallerIdentity(it->second))]++;
            }
        }
        return ret;
    }
    return ERR_OK;
}
bool AgentManagerService::HasReachedCallerConnectionLimitLocked(const AgentCallerIdentity &callerIdentity) const
{
    auto countIt = callerConnectionCounts_.find(BuildCallerIdentityStorageKey(callerIdentity));
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    return countIt->second >= MAX_CONNECTIONS_PER_CALLER;
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    const AgentCallerIdentity &callerIdentity, const sptr<AAFwk::IAbilityConnection> &serviceConnection,
    bool countTowardsCallerLimit, bool isLowCode, const std::string &lowCodeOwnerKey)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    auto existing = trackedConnections_.find(callerRemote);
    if (existing != trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked");
        return ERR_INVALID_VALUE;
    }

    size_t currentCount = 0;
    auto callerKey = BuildCallerIdentityStorageKey(callerIdentity);
    auto countIt = callerConnectionCounts_.find(callerKey);
    if (countIt != callerConnectionCounts_.end()) {
        currentCount = countIt->second;
    }
    if (countTowardsCallerLimit && HasReachedCallerConnectionLimitLocked(callerIdentity)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Maximum agent connections reached for caller identity");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }

    sptr<AAFwk::IAbilityConnection> actualServiceConnection = serviceConnection;
    if (actualServiceConnection == nullptr) {
        actualServiceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
        if (actualServiceConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
            return ERR_INVALID_VALUE;
        }
    }

    TrackedConnectionRecord record;
    record.callerIdentity = callerIdentity;
    record.callerUid = callerIdentity.uid;
    record.serviceConnection = actualServiceConnection;
    record.callerRemote = callerRemote;
    record.isLowCode = isLowCode;
    record.lowCodeOwnerKey = lowCodeOwnerKey;
    record.countTowardsCallerLimit = countTowardsCallerLimit;
    if (record.callerRemote != nullptr) {
        auto handler = [service = wptr<AgentManagerService>(AgentManagerService::GetInstance()),
                           callerRemote = record.callerRemote](
            const wptr<IRemoteObject> &remote) {
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

    trackedConnections_.emplace(callerRemote, record);
    if (countTowardsCallerLimit) {
        callerConnectionCounts_[callerKey] = currentCount + 1;
    }
    return ERR_OK;
}

void AgentManagerService::ReleaseCallerConnectionCountLocked(const AgentCallerIdentity &callerIdentity)
{
    auto countIt = callerConnectionCounts_.find(BuildCallerIdentityStorageKey(callerIdentity));
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
    auto callerIdentity = GetEffectiveCallerIdentity(it->second);
    auto countIt = callerConnectionCounts_.find(BuildCallerIdentityStorageKey(callerIdentity));
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    ReleaseCallerConnectionCountLocked(callerIdentity);
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

    auto callerIdentity = GetEffectiveCallerIdentity(it->second);
    auto countTowardsCallerLimit = it->second.countTowardsCallerLimit;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    if (!it->second.lowCodeOwnerKey.empty()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "release low-code owner by connection");
        lowCodeAgentConnections_.erase(it->second.lowCodeOwnerKey);
    }
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }
    if (!countTowardsCallerLimit) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(BuildCallerIdentityStorageKey(callerIdentity));
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::ReleaseLowCodeConnectionByOwnerKeyLocked(const std::string &ownerKey)
{
    auto it = lowCodeAgentConnections_.find(ownerKey);
    if (it == lowCodeAgentConnections_.end()) {
        return;
    }

    auto callerRemote = it->second.callerRemote;
    if (callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    auto ownerIter = lowCodeOwnersByRemote_.find(callerRemote);
    if (ownerIter != lowCodeOwnersByRemote_.end()) {
        ownerIter->second.erase(ownerKey);
        if (ownerIter->second.empty()) {
            lowCodeOwnersByRemote_.erase(ownerIter);
        }
    }
    lowCodeAgentConnections_.erase(it);
}

void AgentManagerService::ReleaseLowCodeConnectionsByRemoteLocked(
    const sptr<IRemoteObject> &callerRemote, std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections)
{
    auto ownerIter = lowCodeOwnersByRemote_.find(callerRemote);
    if (ownerIter == lowCodeOwnersByRemote_.end()) {
        return;
    }

    auto ownerKeys = ownerIter->second;
    for (const auto &ownerKey : ownerKeys) {
        auto lowCodeIter = lowCodeAgentConnections_.find(ownerKey);
        if (lowCodeIter != lowCodeAgentConnections_.end()) {
            serviceConnections.push_back(lowCodeIter->second.serviceConnection);
        }
        ReleaseLowCodeConnectionByOwnerKeyLocked(ownerKey);
    }
}

void AgentManagerService::ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    auto callerIdentity = GetEffectiveCallerIdentity(it->second);
    auto countTowardsCallerLimit = it->second.countTowardsCallerLimit;
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    bool isDisconnecting = it->second.isDisconnecting;
    if (!it->second.lowCodeOwnerKey.empty()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "release low-code owner by remote");
        lowCodeAgentConnections_.erase(it->second.lowCodeOwnerKey);
    }
    trackedConnections_.erase(it);

    if (isDisconnecting) {
        return;
    }
    if (!countTowardsCallerLimit) {
        return;
    }

    auto countIt = callerConnectionCounts_.find(BuildCallerIdentityStorageKey(callerIdentity));
    if (countIt == callerConnectionCounts_.end()) {
        return;
    }
    if (countIt->second <= 1) {
        callerConnectionCounts_.erase(countIt);
        return;
    }
    countIt->second--;
}

void AgentManagerService::CleanupDeadStandardConnectionLocked(TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    serviceConnection = it->second.serviceConnection;
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

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> serviceConnections;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        if (remote == nullptr) {
            return;
        }
        auto it = trackedConnections_.find(remote);
        if (it != trackedConnections_.end()) {
            sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
            CleanupDeadStandardConnectionLocked(it, remote, serviceConnection);
            serviceConnections.push_back(serviceConnection);
        }
        ReleaseLowCodeConnectionsByRemoteLocked(remote, serviceConnections);
    }

    for (const auto &serviceConnection : serviceConnections) {
        DisconnectDeadServiceConnection(serviceConnection);
    }
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
    HandleConnectionDone(connection, "", resultCode, isDisconnect);
}

void AgentManagerService::HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection,
    const std::string &lowCodeOwnerKey, int32_t resultCode, bool isDisconnect)
{
    if (!lowCodeOwnerKey.empty()) {
        if (isDisconnect || resultCode != ERR_OK) {
            std::lock_guard<std::mutex> lock(connectionLock_);
            ReleaseLowCodeConnectionByOwnerKeyLocked(lowCodeOwnerKey);
        }
        return;
    }
    if (isDisconnect || resultCode != ERR_OK) {
        ReleaseTrackedConnection(connection);
    }
}

int32_t AgentManagerService::PrepareLowCodeCompleteLocked(const std::string &ownerKey, AgentDisconnectPlan &plan)
{
    auto ownerIter = lowCodeAgentConnections_.find(ownerKey);
    if (ownerIter == lowCodeAgentConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    if (ownerIter->second.isDisconnecting) {
        return ERR_OK;
    }

    ownerIter->second.isDisconnecting = true;
    plan.records.push_back(AgentDisconnectRecord {
        ownerIter->second.serviceConnection, ownerIter->second.callerRemote, ownerKey });
    TAG_LOGI(AAFwkTag::SER_ROUTER, "prepare low-code complete, active:%{public}zu",
        lowCodeAgentConnections_.size());
    return ERR_OK;
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
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

    AgentDisconnectPlan plan;
    auto ownerKey = BuildLowCodeOwnerStorageKey(IPCSkeleton::GetCallingUid(), agentId);
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto ret = PrepareLowCodeCompleteLocked(ownerKey, plan);
        if (ret != ERR_OK) {
            return ret;
        }
    }
    if (plan.records.empty()) {
        return ERR_OK;
    }

    for (const auto &record : plan.records) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(
            record.serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
            std::lock_guard<std::mutex> lock(connectionLock_);
            RollbackAgentDisconnectLocked(plan);
            return ret;
        }
    }
    return ERR_OK;
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection,
    const AgentCallerIdentity &callerIdentity, int32_t hostUid)
{
    AAFwk::Want connectWant = want;
    connectWant.SetParam(AGENT_LOW_CODE_BACKING_KEY, true);

    sptr<AAFwk::IAbilityConnection> serviceConnection;
    auto ownerKey = BuildLowCodeOwnerStorageKey(hostUid, agentId);
    auto targetKey = BuildLowCodeTargetStorageKey(connectWant, hostUid);
    auto ret = RegisterLowCodeConnectionAndGetServiceConnection(
        connection, callerIdentity, ownerKey, targetKey, serviceConnection);
    if (ret != ERR_OK) {
        return ret;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionLock_);
        ReleaseLowCodeConnectionByOwnerKeyLocked(ownerKey);
        return ret;
    }
    return ERR_OK;
}
}  // namespace AgentRuntime
}  // namespace OHOS
