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
#include "agent_receiver_proxy.h"
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

// Some connect paths normalize moduleName from bundle metadata while others still carry an empty moduleName.
bool IsModuleNameCompatible(const std::string &storedModuleName, const std::string &incomingModuleName)
{
    return storedModuleName.empty() || incomingModuleName.empty() || storedModuleName == incomingModuleName;
}

// Low-code host sessions are shared by one caller and one target AgentExt. Caller pid and host proxy binder are
// logical-connection details and must not split the target AgentExt instance.
bool IsSameLowCodeHostTarget(const AgentHostKey &left, const AgentHostKey &right)
{
    return left.userId == right.userId && left.callerUid == right.callerUid && left.bundleName == right.bundleName &&
        left.abilityName == right.abilityName && IsModuleNameCompatible(left.moduleName, right.moduleName);
}

sptr<IRemoteObject> GetLowCodeHostProxyIdentity(const AgentHostKey &hostKey)
{
    return hostKey.hostProxyRemote;
}

// Exact standard-session equality is used for records already created from the same normalized connect request, so
// moduleName must match exactly.
template<typename StandardKey>
bool IsSameStandardSession(const StandardKey &left, const StandardKey &right)
{
    return left.callerUid == right.callerUid && left.agentId == right.agentId &&
        left.bundleName == right.bundleName && left.moduleName == right.moduleName &&
        left.abilityName == right.abilityName;
}

// Standard-session lookup tolerates a missing moduleName because callers may provide either a fully normalized want or
// a target that still relies on bundle metadata to fill the module.
template<typename StandardKey>
bool IsCompatibleStandardSessionTarget(const StandardKey &storedKey, const StandardKey &incomingKey)
{
    return storedKey.callerUid == incomingKey.callerUid && storedKey.agentId == incomingKey.agentId &&
        storedKey.bundleName == incomingKey.bundleName && storedKey.abilityName == incomingKey.abilityName &&
        IsModuleNameCompatible(storedKey.moduleName, incomingKey.moduleName);
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
    int32_t callerUid = 0;
    auto ret = ValidateConnectAgentRequest(connection, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: resolve the agentId and backing card metadata from the request.
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 3: low-code agents use dedicated shared-host session management.
    if (card.type == AgentCardType::LOW_CODE) {
        int32_t hostUid = 0;
        ret = ResolveLowCodeHostInfo(connectWant, callerUid / BASE_USER_RANGE, hostUid);
        if (ret != ERR_OK) {
            return ret;
        }
        return ConnectLowCodeAgentExtensionAbility(connectWant, agentId, connection, callerUid, hostUid);
    }

    return ConnectStandardAgentExtensionAbility(connectWant, agentId, card, connection, callerUid);
}

int32_t AgentManagerService::ConnectStandardAgentExtensionAbility(const AAFwk::Want &connectWant,
    const std::string &agentId, const AgentCard &card, const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid)
{
    AAFwk::Want standardWant = connectWant;
    // Prepare the final Want for standard agent-extension connect.
    auto ret = PrepareStandardAgentConnectWant(standardWant, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        standardWant.GetBundle().c_str(), agentId.c_str());

    StandardAgentKey key = BuildStandardAgentKey(callerUid, agentId, standardWant);
    std::shared_ptr<StandardAgentSession> existingSession;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        existingSession = FindStandardSessionLocked(key);
    }
    bool handled = false;
    ret = ConnectExistingStandardSession(standardWant, existingSession, connection, handled);
    if (ret != ERR_OK || handled) {
        return ret;
    }
    return CreateStandardAgentSession(standardWant, agentId, key, connection);
}

int32_t AgentManagerService::ConnectExistingStandardSession(const AAFwk::Want &connectWant,
    const std::shared_ptr<StandardAgentSession> &session,
    const sptr<AAFwk::IAbilityConnection> &connection, bool &handled)
{
    handled = false;
    if (session == nullptr) {
        return ERR_OK;
    }
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    if (serviceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    serviceConnection->SetStandardSessionMode();
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        bool isNewCaller = false;
        auto ret = RegisterStandardSessionCallerLocked(session, connection, serviceConnection, false, true,
            isNewCaller);
        if (ret != ERR_OK) {
            return ret;
        }
        if (!isNewCaller) {
            TAG_LOGI(AAFwkTag::SER_ROUTER, "standard session caller already registered");
            handled = true;
            return ERR_OK;
        }
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(connectionLock_);
        RollbackStandardSessionCallerLocked(session, connection);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    handled = true;
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
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, IPCSkeleton::GetCallingUid(), false,
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
    int32_t &callerUid)
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

    callerUid = IPCSkeleton::GetCallingUid();

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
    return ERR_OK;
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
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid, bool countTowardsCallerLimit,
    sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    // Register the caller callback and allocate the service-side wrapper connection.
    int32_t ret = ERR_OK;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        ret = TryRegisterConnectionLocked(connection, callerUid, nullptr, nullptr, countTowardsCallerLimit);
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
    if (plan.serviceConnection != nullptr) {
        plan.serviceConnections.emplace_back(plan.serviceConnection);
    }
    if (plan.serviceConnections.empty() && plan.hostConnection == nullptr) {
        return ERR_OK;
    }
    return ExecuteAgentDisconnectPlan(plan);
}

int32_t AgentManagerService::ExecuteAgentDisconnectPlan(const AgentDisconnectPlan &plan)
{
    if (plan.hostConnection != nullptr) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(plan.hostConnection));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
            std::scoped_lock lock(connectionLock_, agentHostMutex_);
            RollbackAgentDisconnectLocked(plan);
            return ret;
        }
        return ERR_OK;
    }
    for (const auto &serviceConnection : plan.serviceConnections) {
        if (serviceConnection == nullptr) {
            continue;
        }
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
            std::scoped_lock lock(connectionLock_, agentHostMutex_);
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
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid)
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
    // When that happens, fall back only to the single tracked standard connection owned by this caller.
    // Low-code sessions carry per-caller ownership semantics and must not be resolved via callerUid alone.
    auto matched = end;
    for (auto iter = trackedConnections_.begin(); iter != end; ++iter) {
        if (iter->second.callerUid != callerUid) {
            continue;
        }
        if (iter->second.isLowCode) {
            continue;
        }
        if (matched != end) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Multiple tracked connections exist for callerUid: %{public}d", callerUid);
            return end;
        }
        matched = iter;
    }

    if (matched != end) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Resolved tracked connection by callerUid fallback: %{public}d", callerUid);
    }
    return matched;
}

int32_t AgentManagerService::PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    AgentDisconnectPlan &plan)
{
    auto it = FindTrackedConnectionLocked(connection, IPCSkeleton::GetCallingUid());
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
    if (sessionIter == agentHostSessions_.end() || *sessionIter == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
        return ERR_INVALID_VALUE;
    }

    auto session = *sessionIter;
    if (session->isDisconnecting) {
        return ERR_OK;
    }
    it->second.isDisconnecting = true;
    plan.hostKey = it->second.hostKey;
    plan.hasHostKey = true;
    std::set<sptr<IRemoteObject>> removedHostProxies;
    if (!RemoveLowCodeAgentsForCallerLocked(session, plan.callerRemote, plan, removedHostProxies)) {
        it->second.isDisconnecting = false;
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code caller has no active agents");
        return ERR_INVALID_VALUE;
    }
    auto ret = PrepareLowCodeCallerQuotaForDisconnectLocked(it, session, plan);
    if (ret != ERR_OK) {
        RollbackLowCodeAgentDisconnectLocked(plan);
        return ret;
    }
    SelectLowCodeDisconnectConnectionLocked(session, removedHostProxies, plan);
    return ERR_OK;
}

bool AgentManagerService::RemoveLowCodeAgentsForCallerLocked(const std::shared_ptr<AgentHostSession> &session,
    const sptr<IRemoteObject> &callerRemote, AgentDisconnectPlan &plan,
    std::set<sptr<IRemoteObject>> &removedHostProxies)
{
    for (auto agentIter = session->agents.begin(); agentIter != session->agents.end();) {
        if (agentIter->second.callerRemote != callerRemote) {
            ++agentIter;
            continue;
        }
        removedHostProxies.insert(agentIter->second.hostProxyRemote);
        plan.lowCodeRemovedHostProxies.insert(agentIter->second.hostProxyRemote);
        plan.lowCodeRemovedAgents[agentIter->first] = agentIter->second;
        EraseAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentIter->first });
        agentIter = session->agents.erase(agentIter);
    }
    return !removedHostProxies.empty();
}

int32_t AgentManagerService::PrepareLowCodeCallerQuotaForDisconnectLocked(TrackedConnectionIter it,
    const std::shared_ptr<AgentHostSession> &session, AgentDisconnectPlan &plan)
{
    auto callerConnectionIt = session->callerConnections.find(plan.callerRemote);
    if (callerConnectionIt != session->callerConnections.end()) {
        plan.lowCodeCallerConnection = callerConnectionIt->second;
        session->callerConnections.erase(callerConnectionIt);
    }
    for (const auto &hostProxyRemote : plan.lowCodeRemovedHostProxies) {
        session->disconnectingCallbacks[hostProxyRemote] = plan.lowCodeCallerConnection;
    }
    if (TransferLowCodeCallerLimitAcrossSessionsLocked(it->second.hostKey, it->second.callerUid,
        plan.callerRemote, plan.lowCodeQuotaTransferTo)) {
        plan.lowCodeQuotaTransferFrom = plan.callerRemote;
        return ERR_OK;
    }
    if (!ReleaseCallerConnectionCountLocked(plan.callerRemote)) {
        session->isDisconnecting = false;
        it->second.isDisconnecting = false;
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Release caller connection count failed");
        return ERR_INVALID_VALUE;
    }
    plan.lowCodeQuotaReleased = it->second.countTowardsCallerLimit;
    return ERR_OK;
}

void AgentManagerService::SelectLowCodeDisconnectConnectionLocked(const std::shared_ptr<AgentHostSession> &session,
    const std::set<sptr<IRemoteObject>> &removedHostProxies, AgentDisconnectPlan &plan)
{
    if (session->agents.empty()) {
        session->isDisconnecting = true;
    }
    auto proxyConnectionIt = session->hostProxyConnections.find(*removedHostProxies.begin());
    plan.hostConnection = proxyConnectionIt != session->hostProxyConnections.end() ?
        proxyConnectionIt->second : session->hostConnection;
}

int32_t AgentManagerService::PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan)
{
    const auto &stdKey = it->second.standardKey;
    if (stdKey.callerUid != 0 && !stdKey.agentId.empty()) {
        auto sessionIt = std::find_if(standardSessions_.begin(), standardSessions_.end(),
            [&stdKey](const auto &item) {
                return item != nullptr && IsSameStandardSession(item->key, stdKey);
            });
        if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
            return PrepareMatchedStandardDisconnectLocked(it, *sessionIt, plan);
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

int32_t AgentManagerService::PrepareMatchedStandardDisconnectLocked(TrackedConnectionIter it,
    const std::shared_ptr<StandardAgentSession> &session, AgentDisconnectPlan &plan)
{
    if (session->isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "standard session already disconnecting");
        return ERR_OK;
    }
    session->isDisconnecting = true;
    it->second.isDisconnecting = true;
    CollectStandardDisconnectConnectionsLocked(session, plan);
    ArmStandardDisconnectLocked(session, plan);
    plan.isStandard = true;
    plan.standardCallerUid = session->key.callerUid;
    plan.standardKey = session->key;
    if (!TransferStandardCallerLimitLocked(session->key, plan.standardQuotaTransferFrom,
        plan.standardQuotaTransferTo)) {
        ReleaseCallerConnectionCountByUidLocked(plan.standardCallerUid);
        plan.standardQuotaReleased = true;
    }
    return ERR_OK;
}

void AgentManagerService::CollectStandardDisconnectConnectionsLocked(
    const std::shared_ptr<StandardAgentSession> &session, AgentDisconnectPlan &plan)
{
    for (const auto &callerRemote : session->callerRemotes) {
        auto trackedIt = trackedConnections_.find(callerRemote);
        if (trackedIt == trackedConnections_.end() || trackedIt->second.serviceConnection == nullptr) {
            continue;
        }
        bool exists = std::any_of(plan.serviceConnections.begin(), plan.serviceConnections.end(),
            [&trackedIt](const auto &serviceConnection) {
                return serviceConnection != nullptr &&
                    serviceConnection->AsObject() == trackedIt->second.serviceConnection->AsObject();
            });
        if (!exists) {
            plan.serviceConnections.emplace_back(trackedIt->second.serviceConnection);
        }
    }
    if (plan.serviceConnections.empty()) {
        plan.serviceConnections.emplace_back(session->serviceConnection);
    }
}

void AgentManagerService::ArmStandardDisconnectLocked(const std::shared_ptr<StandardAgentSession> &session,
    const AgentDisconnectPlan &plan)
{
    session->pendingDisconnectServiceRemotes.clear();
    for (const auto &serviceConnection : plan.serviceConnections) {
        if (serviceConnection != nullptr) {
            session->pendingDisconnectServiceRemotes.insert(serviceConnection->AsObject());
        }
    }
}

void AgentManagerService::RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    if (plan.hasHostKey) {
        RollbackLowCodeAgentDisconnectLocked(plan);
    }
    if (plan.isStandard) {
        RollbackStandardAgentDisconnectLocked(plan);
    }
    RollbackTrackedAgentDisconnectLocked(plan);
}

void AgentManagerService::RollbackLowCodeAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    auto sessionIter = FindAgentHostSessionLocked(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && *sessionIter != nullptr) {
        auto session = *sessionIter;
        session->isDisconnecting = false;
        if (plan.callerRemote != nullptr && plan.lowCodeCallerConnection != nullptr) {
            session->callerConnections[plan.callerRemote] = plan.lowCodeCallerConnection;
        }
        for (const auto &hostProxyRemote : plan.lowCodeRemovedHostProxies) {
            session->disconnectingCallbacks.erase(hostProxyRemote);
        }
        for (const auto &[agentId, agentRecord] : plan.lowCodeRemovedAgents) {
            session->agents[agentId] = agentRecord;
            SetAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentId }, session);
        }
    }
    if (plan.lowCodeQuotaReleased) {
        auto it = trackedConnections_.find(plan.callerRemote);
        if (it != trackedConnections_.end()) {
            callerConnectionCounts_[it->second.callerUid]++;
        }
    }
    RollbackTransferredCallerLimitLocked(plan.lowCodeQuotaTransferFrom, plan.lowCodeQuotaTransferTo);
}

void AgentManagerService::RollbackStandardAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    auto sessionIt = std::find_if(standardSessions_.begin(), standardSessions_.end(),
        [&plan](const auto &item) {
            return item != nullptr && IsSameStandardSession(item->key, plan.standardKey);
        });
    if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
        (*sessionIt)->isDisconnecting = false;
        (*sessionIt)->pendingDisconnectServiceRemotes.clear();
    }
    if (plan.standardCallerUid != 0 && plan.standardQuotaReleased) {
        callerConnectionCounts_[plan.standardCallerUid]++;
    }
    RollbackTransferredCallerLimitLocked(plan.standardQuotaTransferFrom, plan.standardQuotaTransferTo);
}

void AgentManagerService::RollbackTransferredCallerLimitLocked(const sptr<IRemoteObject> &transferFrom,
    const sptr<IRemoteObject> &transferTo)
{
    if (transferFrom == nullptr || transferTo == nullptr) {
        return;
    }
    auto fromIt = trackedConnections_.find(transferFrom);
    if (fromIt != trackedConnections_.end()) {
        fromIt->second.countTowardsCallerLimit = true;
    }
    auto toIt = trackedConnections_.find(transferTo);
    if (toIt != trackedConnections_.end()) {
        toIt->second.countTowardsCallerLimit = false;
    }
}

void AgentManagerService::RollbackTrackedAgentDisconnectLocked(const AgentDisconnectPlan &plan)
{
    auto it = trackedConnections_.find(plan.callerRemote);
    if (it != trackedConnections_.end() && it->second.isDisconnecting) {
        it->second.isDisconnecting = false;
        if (!plan.isStandard && !plan.hasHostKey) {
            callerConnectionCounts_[it->second.callerUid]++;
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
                callerConnectionCounts_[it->second.callerUid]++;
            }
        }
        return ret;
    }
    return ERR_OK;
}

bool AgentManagerService::HasReachedCallerConnectionLimitLocked(int32_t callerUid) const
{
    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    return countIt->second >= MAX_CONNECTIONS_PER_CALLER;
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid, const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey,
    bool countTowardsCallerLimit)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    auto ret = CheckExistingTrackedConnectionLocked(callerRemote, hostKey);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }

    size_t currentCount = 0;
    auto countIt = callerConnectionCounts_.find(callerUid);
    if (countIt != callerConnectionCounts_.end()) {
        currentCount = countIt->second;
    }
    if (countTowardsCallerLimit && HasReachedCallerConnectionLimitLocked(callerUid)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Maximum agent connections reached for callerUid: %{public}d", callerUid);
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }

    auto actualServiceConnection = EnsureServiceConnection(connection, serviceConnection);
    if (actualServiceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }

    TrackedConnectionRecord record;
    record.callerUid = callerUid;
    record.serviceConnection = actualServiceConnection;
    record.callerRemote = callerRemote;
    record.isLowCode = hostKey != nullptr;
    if (hostKey != nullptr) {
        record.hostKey = *hostKey;
    }
    record.countTowardsCallerLimit = countTowardsCallerLimit;
    AddDeathRecipientLocked(record);

    trackedConnections_.emplace(callerRemote, record);
    if (countTowardsCallerLimit) {
        callerConnectionCounts_[callerUid] = currentCount + 1;
    }
    return ERR_OK;
}

int32_t AgentManagerService::CheckExistingTrackedConnectionLocked(const sptr<IRemoteObject> &callerRemote,
    const AgentHostKey *hostKey) const
{
    auto existing = trackedConnections_.find(callerRemote);
    if (existing == trackedConnections_.end()) {
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    if (hostKey != nullptr && existing->second.isLowCode &&
        IsLowCodeHostKeyMatched(existing->second.hostKey, *hostKey)) {
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked");
    return AAFwk::INVALID_PARAMETERS_ERR;
}

sptr<AAFwk::IAbilityConnection> AgentManagerService::EnsureServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection) const
{
    if (serviceConnection != nullptr) {
        return serviceConnection;
    }
    auto actualServiceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    if (actualServiceConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
    }
    return actualServiceConnection;
}

void AgentManagerService::AddDeathRecipientLocked(TrackedConnectionRecord &record)
{
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
}

void AgentManagerService::ReleaseCallerConnectionCountByUidLocked(int32_t callerUid)
{
    auto countIt = callerConnectionCounts_.find(callerUid);
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
    auto countIt = callerConnectionCounts_.find(it->second.callerUid);
    if (countIt == callerConnectionCounts_.end()) {
        return false;
    }
    ReleaseCallerConnectionCountByUidLocked(it->second.callerUid);
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

    auto callerUid = it->second.callerUid;
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

    auto countIt = callerConnectionCounts_.find(callerUid);
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

    auto callerUid = it->second.callerUid;
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

    auto countIt = callerConnectionCounts_.find(callerUid);
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
            candidateIter->second.callerUid != currentIter->second.callerUid ||
            candidateIter->second.countTowardsCallerLimit) {
            continue;
        }
        candidateIter->second.countTowardsCallerLimit = true;
        currentIter->second.countTowardsCallerLimit = false;
        return;
    }
}

bool AgentManagerService::IsLowCodeHostQuotaKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const
{
    return IsSameLowCodeHostTarget(left, right);
}

bool AgentManagerService::IsLowCodeHostKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const
{
    return IsSameLowCodeHostTarget(left, right);
}

AgentManagerService::AgentHostSessionIter AgentManagerService::FindAgentHostSessionLocked(const AgentHostKey &key)
{
    agentHostSessions_.erase(std::remove(agentHostSessions_.begin(), agentHostSessions_.end(), nullptr),
        agentHostSessions_.end());
    for (auto iter = agentHostSessions_.begin(); iter != agentHostSessions_.end(); ++iter) {
        if (*iter != nullptr && IsSameLowCodeHostTarget((*iter)->key, key)) {
            return iter;
        }
    }
    return agentHostSessions_.end();
}

bool AgentManagerService::IsAgentOwnerKeyEqual(const AgentOwnerKey &left, const AgentOwnerKey &right) const
{
    return left.hostUid == right.hostUid && left.agentId == right.agentId;
}

AgentManagerService::AgentOwnerIter AgentManagerService::FindAgentOwnerLocked(const AgentOwnerKey &key)
{
    return std::find_if(agentOwners_.begin(), agentOwners_.end(),
        [&key](const auto &owner) {
            return owner.key.hostUid == key.hostUid && owner.key.agentId == key.agentId;
        });
}

void AgentManagerService::SetAgentOwnerLocked(const AgentOwnerKey &key,
    const std::shared_ptr<AgentHostSession> &session)
{
    auto ownerIter = FindAgentOwnerLocked(key);
    if (ownerIter != agentOwners_.end()) {
        ownerIter->session = session;
        return;
    }
    agentOwners_.emplace_back(AgentOwnerRecord { key, session });
}

void AgentManagerService::EraseAgentOwnerLocked(const AgentOwnerKey &key)
{
    auto ownerIter = FindAgentOwnerLocked(key);
    if (ownerIter != agentOwners_.end()) {
        agentOwners_.erase(ownerIter);
    }
}

bool AgentManagerService::HasLowCodeHostQuotaLocked(const AgentHostKey &hostKey, int32_t callerUid) const
{
    for (const auto &sessionEntry : agentHostSessions_) {
        const auto &session = sessionEntry;
        if (session == nullptr || session->isDisconnecting || !IsLowCodeHostQuotaKeyMatched(session->key, hostKey)) {
            continue;
        }
        for (const auto &connectionEntry : session->callerConnections) {
            auto trackedIt = trackedConnections_.find(connectionEntry.first);
            if (trackedIt != trackedConnections_.end() && trackedIt->second.callerUid == callerUid) {
                return true;
            }
        }
    }
    return false;
}

bool AgentManagerService::TransferLowCodeCallerLimitAcrossSessionsLocked(const AgentHostKey &hostKey,
    int32_t callerUid, const sptr<IRemoteObject> &transferFrom, sptr<IRemoteObject> &transferTo)
{
    auto currentIter = trackedConnections_.find(transferFrom);
    if (currentIter == trackedConnections_.end() || !currentIter->second.countTowardsCallerLimit) {
        return false;
    }
    for (const auto &sessionEntry : agentHostSessions_) {
        const auto &session = sessionEntry;
        if (session == nullptr || session->isDisconnecting || !IsLowCodeHostQuotaKeyMatched(session->key, hostKey)) {
            continue;
        }
        for (const auto &connectionEntry : session->callerConnections) {
            const auto &candidateRemote = connectionEntry.first;
            if (candidateRemote == nullptr || candidateRemote == transferFrom) {
                continue;
            }
            auto trackedIt = trackedConnections_.find(candidateRemote);
            if (trackedIt == trackedConnections_.end() || trackedIt->second.callerUid != callerUid ||
                trackedIt->second.countTowardsCallerLimit) {
                continue;
            }
            trackedIt->second.countTowardsCallerLimit = true;
            currentIter->second.countTowardsCallerLimit = false;
            transferTo = candidateRemote;
            return true;
        }
    }
    return false;
}

void AgentManagerService::CleanupDeadStandardConnectionLocked(TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections)
{
    auto deadServiceConnection = it->second.serviceConnection;
    if (deadServiceConnection != nullptr) {
        serviceConnections.emplace_back(deadServiceConnection);
    }
    auto &stdKey = it->second.standardKey;
    if (stdKey.callerUid != 0 && !stdKey.agentId.empty()) {
        auto sessionIt = std::find_if(standardSessions_.begin(), standardSessions_.end(),
            [&stdKey](const auto &item) {
                return item != nullptr && IsSameStandardSession(item->key, stdKey);
            });
        if (sessionIt != standardSessions_.end() && *sessionIt != nullptr) {
            RemoveDeadStandardCallerFromSessionLocked(*sessionIt, remote, deadServiceConnection);
            if ((*sessionIt)->callerRemotes.empty()) {
                standardSessions_.erase(sessionIt);
            }
        }
    }
    TransferStandardQuotaAfterCallerDiedLocked(stdKey, remote);
    ReleaseTrackedConnectionByRemoteLocked(remote);
}

void AgentManagerService::RemoveDeadStandardCallerFromSessionLocked(
    const std::shared_ptr<StandardAgentSession> &session, const sptr<IRemoteObject> &remote,
    const sptr<AAFwk::IAbilityConnection> &deadServiceConnection)
{
    auto isDeadCallback = [&remote](const auto &callback) {
        return callback != nullptr && callback->AsObject() == remote;
    };
    session->pendingCallbacks.erase(std::remove_if(session->pendingCallbacks.begin(),
        session->pendingCallbacks.end(), isDeadCallback), session->pendingCallbacks.end());
    session->connectedCallbacks.erase(std::remove_if(session->connectedCallbacks.begin(),
        session->connectedCallbacks.end(), isDeadCallback), session->connectedCallbacks.end());
    session->callerRemotes.erase(std::remove(session->callerRemotes.begin(), session->callerRemotes.end(),
        remote), session->callerRemotes.end());
    for (auto iter = session->pendingCallbackByServiceRemote.begin();
         iter != session->pendingCallbackByServiceRemote.end();) {
        if (isDeadCallback(iter->second)) {
            iter = session->pendingCallbackByServiceRemote.erase(iter);
            continue;
        }
        ++iter;
    }
    for (auto iter = session->standardServiceConnections.begin(); iter != session->standardServiceConnections.end();) {
        if (iter->second != nullptr && deadServiceConnection != nullptr &&
            iter->second->AsObject() == deadServiceConnection->AsObject()) {
            iter = session->standardServiceConnections.erase(iter);
            continue;
        }
        ++iter;
    }
    if (session->serviceConnection != nullptr && deadServiceConnection != nullptr &&
        session->serviceConnection->AsObject() == deadServiceConnection->AsObject()) {
        session->serviceConnection = session->standardServiceConnections.empty() ?
            nullptr : session->standardServiceConnections.begin()->second;
    }
}

void AgentManagerService::TransferStandardQuotaAfterCallerDiedLocked(const StandardAgentKey &stdKey,
    const sptr<IRemoteObject> &remote)
{
    auto trackedIt = trackedConnections_.find(remote);
    if (trackedIt == trackedConnections_.end() || !trackedIt->second.countTowardsCallerLimit) {
        return;
    }
    for (const auto &session : standardSessions_) {
        if (session == nullptr || !IsStandardAgentQuotaKeyMatched(session->key, stdKey)) {
            continue;
        }
        for (const auto &callerRemote : session->callerRemotes) {
            auto remainingTrackedIt = trackedConnections_.find(callerRemote);
            if (remainingTrackedIt == trackedConnections_.end() ||
                remainingTrackedIt->second.callerUid != trackedIt->second.callerUid ||
                remainingTrackedIt->second.countTowardsCallerLimit) {
                continue;
            }
            remainingTrackedIt->second.countTowardsCallerLimit = true;
            trackedIt->second.countTowardsCallerLimit = false;
            return;
        }
    }
}

void AgentManagerService::CleanupDeadLowCodeConnectionLocked(TrackedConnectionIter it,
    const sptr<IRemoteObject> &remote, std::vector<sptr<AgentHostConnection>> &hostConnections)
{
    auto sessionIter = FindAgentHostSessionLocked(it->second.hostKey);
    if (sessionIter != agentHostSessions_.end() && *sessionIter != nullptr) {
        auto session = *sessionIter;
        session->callerConnections.erase(remote);
        std::set<sptr<IRemoteObject>> removedHostProxies;
        for (auto agentIter = session->agents.begin(); agentIter != session->agents.end();) {
            if (agentIter->second.callerRemote == remote) {
                removedHostProxies.insert(agentIter->second.hostProxyRemote);
                EraseAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentIter->first });
                agentIter = session->agents.erase(agentIter);
                continue;
            }
            ++agentIter;
        }
        for (const auto &hostProxyRemote : removedHostProxies) {
            session->disconnectingCallbacks[hostProxyRemote] = nullptr;
            auto proxyConnectionIt = session->hostProxyConnections.find(hostProxyRemote);
            if (proxyConnectionIt != session->hostProxyConnections.end()) {
                hostConnections.emplace_back(proxyConnectionIt->second);
            } else if (session->hostConnection != nullptr) {
                hostConnections.emplace_back(session->hostConnection);
            }
        }
        if (!session->isDisconnecting && session->agents.empty()) {
            session->isDisconnecting = true;
        } else {
            TransferLowCodeCallerLimitLocked(session, remote);
        }
        auto trackedIt = trackedConnections_.find(remote);
        if (trackedIt != trackedConnections_.end() && trackedIt->second.countTowardsCallerLimit) {
            sptr<IRemoteObject> transferTo = nullptr;
            TransferLowCodeCallerLimitAcrossSessionsLocked(session->key, trackedIt->second.callerUid, remote,
                transferTo);
        }
    }
    ReleaseTrackedConnectionByRemoteLocked(remote);
}

void AgentManagerService::DisconnectDeadServiceConnections(
    const std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections)
{
    for (const auto &serviceConnection : serviceConnections) {
        if (serviceConnection == nullptr) {
            continue;
        }
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
}

void AgentManagerService::DisconnectDeadHostConnections(const std::vector<sptr<AgentHostConnection>> &hostConnections)
{
    for (const auto &hostConnection : hostConnections) {
        if (hostConnection == nullptr) {
            continue;
        }
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
        if (ret == ERR_OK) {
            continue;
        }
        RestoreFailedDeadHostDisconnectLocked(hostConnection);
        TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
    }
}

AgentManagerService::AgentHostSessionIter AgentManagerService::FindAgentHostSessionByConnectionLocked(
    const sptr<AgentHostConnection> &hostConnection)
{
    return std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
        [hostConnection](const auto &item) {
            return item != nullptr && (item->hostConnection == hostConnection ||
                std::any_of(item->hostProxyConnections.begin(), item->hostProxyConnections.end(),
                    [hostConnection](const auto &entry) {
                        return entry.second == hostConnection;
                    }));
        });
}

void AgentManagerService::RemoveFailedHostProxyDisconnectLocked(
    const std::shared_ptr<AgentHostSession> &session, const sptr<AgentHostConnection> &hostConnection)
{
    for (auto iter = session->hostProxyConnections.begin(); iter != session->hostProxyConnections.end();) {
        if (iter->second != hostConnection) {
            ++iter;
            continue;
        }
        session->disconnectingCallbacks.erase(iter->first);
        iter = session->hostProxyConnections.erase(iter);
    }
}

void AgentManagerService::RestoreFailedDeadHostDisconnectLocked(const sptr<AgentHostConnection> &hostConnection)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto sessionIter = FindAgentHostSessionByConnectionLocked(hostConnection);
    if (sessionIter == agentHostSessions_.end() || *sessionIter == nullptr) {
        return;
    }
    auto session = *sessionIter;
    session->isDisconnecting = false;
    RemoveFailedHostProxyDisconnectLocked(session, hostConnection);
}

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> serviceConnections;
    std::vector<sptr<AgentHostConnection>> hostConnections;
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
            CleanupDeadLowCodeConnectionLocked(it, remote, hostConnections);
        } else {
            CleanupDeadStandardConnectionLocked(it, remote, serviceConnections);
        }
    }

    DisconnectDeadServiceConnections(serviceConnections);
    DisconnectDeadHostConnections(hostConnections);
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
    sptr<AgentHostConnection> hostConnection = nullptr;
    LowCodeCompleteRollbackInfo rollbackInfo;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto ret = RemoveCompletedLowCodeAgentLocked(agentId, hostConnection, rollbackInfo);
        if (ret != ERR_OK || hostConnection == nullptr) {
            return ret;
        }
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
    if (ret == ERR_OK) {
        return ERR_OK;
    }
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        RollbackLowCodeAgentCompleteLocked(agentId, rollbackInfo);
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
    return ret;
}

int32_t AgentManagerService::RemoveCompletedLowCodeAgentLocked(const std::string &agentId,
    sptr<AgentHostConnection> &hostConnection, LowCodeCompleteRollbackInfo &rollbackInfo)
{
    AgentOwnerKey ownerKey { IPCSkeleton::GetCallingUid(), agentId };
    auto ownerIter = FindAgentOwnerLocked(ownerKey);
    if (ownerIter == agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    auto session = ownerIter->session;
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

    auto callerRemote = agentIter->second.callerRemote;
    auto hostProxyRemote = agentIter->second.hostProxyRemote;
    rollbackInfo.agentRecord = agentIter->second;
    bool callerStillOwnsAgent = LowCodeCallerStillOwnsAgentLocked(session, agentId, callerRemote);
    session->agents.erase(agentId);
    agentOwners_.erase(ownerIter);
    if (callerStillOwnsAgent) {
        return ERR_OK;
    }
    if (callerRemote != nullptr) {
        RemoveCompletedLowCodeCallerLocked(session, callerRemote, hostProxyRemote, rollbackInfo);
        if (!session->agents.empty()) {
            return ERR_OK;
        }
    }
    if (session->isDisconnecting) {
        return ERR_OK;
    }
    session->isDisconnecting = true;
    auto proxyConnectionIt = session->hostProxyConnections.find(hostProxyRemote);
    hostConnection = proxyConnectionIt != session->hostProxyConnections.end() ?
        proxyConnectionIt->second : session->hostConnection;
    rollbackInfo.hostKey = session->key;
    rollbackInfo.ownerKey = AgentOwnerKey { session->hostUid, agentId };
    rollbackInfo.callerRemote = callerRemote;
    rollbackInfo.hostProxyRemote = hostProxyRemote;
    rollbackInfo.needRollback = true;
    return ERR_OK;
}

bool AgentManagerService::LowCodeCallerStillOwnsAgentLocked(const std::shared_ptr<AgentHostSession> &session,
    const std::string &agentId, const sptr<IRemoteObject> &callerRemote) const
{
    return std::any_of(session->agents.begin(), session->agents.end(),
        [&agentId, &callerRemote](const auto &entry) {
            return entry.first != agentId && entry.second.callerRemote == callerRemote;
        });
}

void AgentManagerService::RemoveCompletedLowCodeCallerLocked(const std::shared_ptr<AgentHostSession> &session,
    const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote,
    LowCodeCompleteRollbackInfo &rollbackInfo)
{
    auto callerConnectionIt = session->callerConnections.find(callerRemote);
    if (callerConnectionIt != session->callerConnections.end()) {
        rollbackInfo.callerConnection = callerConnectionIt->second;
        session->callerConnections.erase(callerConnectionIt);
    }
    TransferLowCodeCallerLimitLocked(session, callerRemote);
    auto trackedIt = trackedConnections_.find(callerRemote);
    if (trackedIt != trackedConnections_.end() && trackedIt->second.countTowardsCallerLimit) {
        TransferLowCodeCallerLimitAcrossSessionsLocked(session->key, trackedIt->second.callerUid,
            callerRemote, rollbackInfo.quotaTransferTo);
        rollbackInfo.quotaTransferFrom = callerRemote;
    }
    if (session->agents.empty()) {
        session->disconnectingCallbacks[hostProxyRemote] = rollbackInfo.callerConnection;
    } else {
        ReleaseTrackedConnectionByRemoteLocked(callerRemote);
    }
}

void AgentManagerService::RollbackLowCodeAgentCompleteLocked(const std::string &agentId,
    const LowCodeCompleteRollbackInfo &rollbackInfo)
{
    auto sessionIter = FindAgentHostSessionLocked(rollbackInfo.hostKey);
    if (rollbackInfo.needRollback && sessionIter != agentHostSessions_.end() && *sessionIter != nullptr) {
        auto session = *sessionIter;
        session->isDisconnecting = false;
        session->disconnectingCallbacks.erase(rollbackInfo.hostProxyRemote);
        session->agents[agentId] = rollbackInfo.agentRecord;
        if (rollbackInfo.callerRemote != nullptr && rollbackInfo.callerConnection != nullptr) {
            session->callerConnections[rollbackInfo.callerRemote] = rollbackInfo.callerConnection;
        }
        SetAgentOwnerLocked(rollbackInfo.ownerKey, session);
    }
    RollbackTransferredCallerLimitLocked(rollbackInfo.quotaTransferFrom, rollbackInfo.quotaTransferTo);
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    int32_t hostUid)
{
    // Step 1: create or reuse the shared low-code host session bookkeeping.
    AgentConnectPlan plan;
    auto ret = PrepareLowCodeConnectPlan(BuildAgentHostKey(want, callingUid), hostUid, agentId, connection,
        callingUid, plan);
    if (ret != ERR_OK) {
        return ret;
    }

    // Step 2: if the host is already connected, notify the caller immediately.
    if (plan.notifyExistingConnection) {
        NotifyExistingLowCodeConnection(plan, agentId, connection);
        return ERR_OK;
    }

    // Step 3: otherwise connect the shared host session once through AMS.
    if (!plan.needRealConnect) {
        return ERR_OK;
    }
    return CompleteAgentHostConnect(want, agentId, plan);
}

int32_t AgentManagerService::PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    AgentConnectPlan &plan)
{
    plan.hostKey = hostKey;
    plan.hostUid = hostUid;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    AgentOwnerKey ownerKey { hostUid, agentId };
    if (FindAgentOwnerLocked(ownerKey) != agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent already active");
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }

    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    plan.callerRemote = callerRemote;
    auto hostProxyRemote = GetLowCodeHostProxyIdentity(hostKey);
    if (hostProxyRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code host proxy is null");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<AgentHostSession> session;
    auto ret = GetOrCreateLowCodeHostSessionLocked(hostKey, hostUid, hostProxyRemote, session, plan);
    if (ret != ERR_OK) {
        return ret;
    }
    return RegisterLowCodePlanLocked(hostKey, agentId, connection, callingUid, callerRemote, hostProxyRemote,
        session, plan);
}

int32_t AgentManagerService::GetOrCreateLowCodeHostSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
    const sptr<IRemoteObject> &hostProxyRemote, std::shared_ptr<AgentHostSession> &session, AgentConnectPlan &plan)
{
    auto sessionIter = FindAgentHostSessionLocked(hostKey);
    if (sessionIter != agentHostSessions_.end()) {
        session = *sessionIter;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (session == nullptr) {
        auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
        if (hostConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
            return ERR_INVALID_VALUE;
        }
        session = std::make_shared<AgentHostSession>();
        session->key = hostKey;
        session->hostUid = hostUid;
        session->hostConnection = hostConnection;
        session->hostProxyConnections[hostProxyRemote] = hostConnection;
        agentHostSessions_.emplace_back(session);
        plan.needRealConnect = true;
        return ERR_OK;
    }
    if (session->isDisconnecting) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code host session is disconnecting");
        return ERR_INVALID_VALUE;
    }
    if (session->agents.size() >= MAX_AGENTS_PER_HOST_SESSION) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent host session limit reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    if (session->hostProxyConnections.find(hostProxyRemote) == session->hostProxyConnections.end()) {
        auto hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
        if (hostConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
            return ERR_INVALID_VALUE;
        }
        session->hostProxyConnections[hostProxyRemote] = hostConnection;
        plan.needRealConnect = true;
    }
    return ERR_OK;
}

int32_t AgentManagerService::RegisterLowCodePlanLocked(const AgentHostKey &hostKey, const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote,
    const std::shared_ptr<AgentHostSession> &session, AgentConnectPlan &plan)
{
    bool callerRemoteAlreadyTracked = trackedConnections_.find(callerRemote) != trackedConnections_.end();
    auto ret = TryRegisterConnectionLocked(connection, callingUid, session->hostConnection, &hostKey,
        !HasLowCodeHostQuotaLocked(hostKey, callingUid));
    if (ret != ERR_OK) {
        if (session->agents.empty() && session->callerConnections.empty()) {
            auto pendingSessionIter = FindAgentHostSessionLocked(hostKey);
            if (pendingSessionIter != agentHostSessions_.end()) {
                agentHostSessions_.erase(pendingSessionIter);
            }
        } else if (plan.needRealConnect) {
            session->hostProxyConnections.erase(hostProxyRemote);
        }
        return ret;
    }
    plan.registeredTrackedConnection = !callerRemoteAlreadyTracked;

    session->callerConnections[callerRemote] = connection;
    session->agents[agentId] = LowCodeAgentRecord { callerRemote, !session->isConnected || plan.needRealConnect,
        hostProxyRemote };
    SetAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentId }, session);
    plan.hostConnection = session->hostProxyConnections[hostProxyRemote];
    plan.notifyExistingConnection = session->isConnected && !plan.needRealConnect && session->remoteObject != nullptr;
    if (plan.notifyExistingConnection) {
        plan.cachedElement = session->element;
        plan.cachedRemoteObject = session->remoteObject;
        plan.cachedResultCode = session->resultCode;
    }
    return ERR_OK;
}

void AgentManagerService::NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    AgentHostSession callbackSession;
    callbackSession.remoteObject = plan.cachedRemoteObject;
    NotifyAgentInvokedLocked(callbackSession, agentId);
    connection->OnAbilityConnectDone(plan.cachedElement, plan.cachedRemoteObject, plan.cachedResultCode);
}

void AgentManagerService::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    EraseAgentOwnerLocked(AgentOwnerKey { plan.hostUid, agentId });
    auto sessionIter = FindAgentHostSessionLocked(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && *sessionIter != nullptr) {
        auto session = *sessionIter;
        auto hostProxyRemote = plan.hostKey.hostProxyRemote;
        auto agentIter = session->agents.find(agentId);
        if (agentIter != session->agents.end()) {
            hostProxyRemote = agentIter->second.hostProxyRemote;
            session->agents.erase(agentIter);
        }
        if (plan.callerRemote != nullptr) {
            session->callerConnections.erase(plan.callerRemote);
        }
        session->hostProxyConnections.erase(hostProxyRemote);
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

AgentHostKey AgentManagerService::BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const
{
    AgentHostKey key;
    key.userId = callingUid / BASE_USER_RANGE;
    key.callerUid = callingUid;
    key.callerPid = IPCSkeleton::GetCallingPid();
    key.bundleName = want.GetElement().GetBundleName();
    key.moduleName = want.GetElement().GetModuleName();
    key.abilityName = want.GetElement().GetAbilityName();
    key.hostProxyRemote = want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
    return key;
}

bool AgentManagerService::NotifyAgentInvokedLocked(const AgentHostSession &session, const std::string &agentId)
{
    if (session.remoteObject == nullptr) {
        return false;
    }
    auto receiver = iface_cast<IAgentReceiver>(session.remoteObject);
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent receiver null");
        return false;
    }
    receiver->AgentInvoked(agentId);
    return true;
}

void AgentManagerService::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        EraseAgentOwnerLocked(AgentOwnerKey { session.hostUid, agentEntry.first });
    }
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> pendingAgentIds;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = FindAgentHostSessionLocked(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        auto session = *sessionIter;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        bool wasConnected = session->isConnected;
        if (resultCode == ERR_OK && remoteObject != nullptr) {
            session->element = element;
            session->remoteObject = remoteObject;
            session->resultCode = resultCode;
            session->isConnected = true;
        } else if (!wasConnected) {
            session->element = element;
            session->remoteObject = remoteObject;
            session->resultCode = resultCode;
        }
        auto hostProxyRemote = GetLowCodeHostProxyIdentity(key);
        CollectPendingLowCodeConnectCallbacksLocked(session, hostProxyRemote, callbacks, pendingAgentIds);
        if (resultCode != ERR_OK || remoteObject == nullptr) {
            CleanupFailedLowCodeHostConnectLocked(session, key, hostProxyRemote, callbacks, pendingAgentIds);
        }
    }
    AgentHostSession callbackSession;
    callbackSession.remoteObject = remoteObject;
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        for (const auto &agentId : pendingAgentIds) {
            NotifyAgentInvokedLocked(callbackSession, agentId);
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
            HandleConnectionDone(callback, resultCode, false);
        }
    }
}

void AgentManagerService::CollectPendingLowCodeConnectCallbacksLocked(
    const std::shared_ptr<AgentHostSession> &session, const sptr<IRemoteObject> &hostProxyRemote,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks, std::vector<std::string> &pendingAgentIds)
{
    std::set<sptr<IRemoteObject>> callbackRemotes;
    for (auto &[pendingAgentId, agentRecord] : session->agents) {
        if (!agentRecord.isPending || agentRecord.hostProxyRemote != hostProxyRemote) {
            continue;
        }
        pendingAgentIds.emplace_back(pendingAgentId);
        callbackRemotes.insert(agentRecord.callerRemote);
        agentRecord.isPending = false;
    }
    for (const auto &callerRemote : callbackRemotes) {
        auto callbackIt = session->callerConnections.find(callerRemote);
        if (callbackIt != session->callerConnections.end()) {
            callbacks.emplace_back(callbackIt->second);
        }
    }
}

void AgentManagerService::CleanupFailedLowCodeHostConnectLocked(const std::shared_ptr<AgentHostSession> &session,
    const AgentHostKey &key, const sptr<IRemoteObject> &hostProxyRemote,
    const std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
    const std::vector<std::string> &pendingAgentIds)
{
    for (const auto &agentId : pendingAgentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter != session->agents.end()) {
            EraseAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentId });
            session->agents.erase(agentIter);
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            session->callerConnections.erase(callback->AsObject());
        }
    }
    session->hostProxyConnections.erase(hostProxyRemote);
    if (session->agents.empty()) {
        ClearAgentHostSessionLocked(key);
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
        if (*sessionIter == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        auto session = *sessionIter;
        auto hostProxyRemote = GetLowCodeHostProxyIdentity(key);
        auto disconnectingIt = session->disconnectingCallbacks.find(hostProxyRemote);
        if (!session->isDisconnecting && disconnectingIt != session->disconnectingCallbacks.end()) {
            callbacks.emplace_back(disconnectingIt->second);
            session->disconnectingCallbacks.erase(disconnectingIt);
            session->hostProxyConnections.erase(hostProxyRemote);
        } else if (!session->isDisconnecting && disconnectingIt == session->disconnectingCallbacks.end()) {
            return;
        } else {
            for (const auto &callbackEntry : session->callerConnections) {
                callbacks.emplace_back(callbackEntry.second);
            }
            for (const auto &callbackEntry : session->disconnectingCallbacks) {
                callbacks.emplace_back(callbackEntry.second);
            }
            ClearAgentHostSessionLocked(key);
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
    if (sessionIter == agentHostSessions_.end()) {
        return;
    }
    if (*sessionIter != nullptr) {
        EraseAgentOwnersLocked(**sessionIter);
    }
    agentHostSessions_.erase(sessionIter);
}

AgentManagerService::StandardAgentKey AgentManagerService::BuildStandardAgentKey(
    int32_t callerUid, const std::string &agentId, const AAFwk::Want &want) const
{
    StandardAgentKey key;
    key.callerUid = callerUid;
    key.agentId = agentId;
    const auto &element = want.GetElement();
    key.bundleName = element.GetBundleName();
    key.moduleName = element.GetModuleName();
    key.abilityName = element.GetAbilityName();
    key.hostProxyRemote = want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
    return key;
}

bool AgentManagerService::IsStandardAgentKeyEqual(const StandardAgentKey &left, const StandardAgentKey &right) const
{
    return IsSameStandardSession(left, right);
}

bool AgentManagerService::IsStandardAgentKeyMatched(
    const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const
{
    return IsCompatibleStandardSessionTarget(storedKey, incomingKey);
}

bool AgentManagerService::IsStandardAgentQuotaKeyMatched(
    const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const
{
    return storedKey.callerUid == incomingKey.callerUid && storedKey.agentId == incomingKey.agentId &&
        storedKey.bundleName == incomingKey.bundleName && storedKey.abilityName == incomingKey.abilityName &&
        (storedKey.moduleName.empty() || incomingKey.moduleName.empty() ||
            storedKey.moduleName == incomingKey.moduleName);
}

std::shared_ptr<AgentManagerService::StandardAgentSession> AgentManagerService::FindStandardSessionLocked(
    const StandardAgentKey &key) const
{
    for (const auto &entry : standardSessions_) {
        if (entry != nullptr && IsStandardAgentKeyMatched(entry->key, key)) {
            return entry;
        }
    }
    return nullptr;
}

AgentManagerService::StandardSessionIter AgentManagerService::FindStandardSessionByServiceRemoteLocked(
    const sptr<IRemoteObject> &serviceRemote)
{
    return std::find_if(standardSessions_.begin(), standardSessions_.end(),
        [&serviceRemote](const auto &item) {
            if (item == nullptr || serviceRemote == nullptr) {
                return false;
            }
            if (item->serviceConnection != nullptr && item->serviceConnection->AsObject() == serviceRemote) {
                return true;
            }
            if (item->standardServiceConnections.find(serviceRemote) != item->standardServiceConnections.end()) {
                return true;
            }
            return item->pendingCallbackByServiceRemote.find(serviceRemote) !=
                item->pendingCallbackByServiceRemote.end();
        });
}

int32_t AgentManagerService::RegisterStandardSessionCallerLocked(
    const std::shared_ptr<StandardAgentSession> &session,
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool countTowardsCallerLimit, bool forcePending,
    bool &isNewCaller)
{
    isNewCaller = false;
    if (session == nullptr || connection == nullptr || session->serviceConnection == nullptr ||
        serviceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> callerRemote = nullptr;
    auto ret = EnsureStandardTrackedCallerLocked(session, connection, serviceConnection, countTowardsCallerLimit,
        callerRemote);
    if (ret != ERR_OK) {
        return ret;
    }
    if (std::find(session->callerRemotes.begin(), session->callerRemotes.end(), callerRemote) ==
        session->callerRemotes.end()) {
        session->callerRemotes.push_back(callerRemote);
        isNewCaller = true;
    }
    if (!isNewCaller) {
        return ERR_OK;
    }
    AddStandardSessionCallbackLocked(session, connection, serviceConnection, forcePending);
    return ERR_OK;
}

int32_t AgentManagerService::EnsureStandardTrackedCallerLocked(
    const std::shared_ptr<StandardAgentSession> &session, const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool countTowardsCallerLimit,
    sptr<IRemoteObject> &callerRemote)
{
    callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto trackedIt = trackedConnections_.find(callerRemote);
    if (trackedIt == trackedConnections_.end()) {
        auto ret = TryRegisterConnectionLocked(connection, session->key.callerUid,
            serviceConnection, nullptr, countTowardsCallerLimit);
        if (ret != ERR_OK) {
            return ret;
        }
        trackedIt = trackedConnections_.find(callerRemote);
    } else if (trackedIt->second.standardKey.callerUid != 0 &&
        !IsStandardAgentKeyEqual(trackedIt->second.standardKey, session->key)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already belongs to another standard session");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (trackedIt != trackedConnections_.end()) {
        trackedIt->second.standardKey = session->key;
    }
    return ERR_OK;
}

void AgentManagerService::AddStandardSessionCallbackLocked(const std::shared_ptr<StandardAgentSession> &session,
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool forcePending)
{
    auto &callbacks = (forcePending || session->state != StandardAgentState::CONNECTED) ?
        session->pendingCallbacks : session->connectedCallbacks;
    if (std::find(callbacks.begin(), callbacks.end(), connection) == callbacks.end()) {
        callbacks.push_back(connection);
    }
    if (forcePending || session->state != StandardAgentState::CONNECTED) {
        session->pendingCallbackByServiceRemote[serviceConnection->AsObject()] = connection;
    }
    session->standardServiceConnections[serviceConnection->AsObject()] = serviceConnection;
}

void AgentManagerService::RollbackStandardSessionCallerLocked(
    const std::shared_ptr<StandardAgentSession> &session, const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (session == nullptr || connection == nullptr) {
        return;
    }
    auto callerRemote = GetConnectionIdentityRemote(connection);
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    auto trackedIt = trackedConnections_.find(callerRemote);
    if (trackedIt != trackedConnections_.end()) {
        serviceConnection = trackedIt->second.serviceConnection;
    }
    session->pendingCallbacks.erase(std::remove(session->pendingCallbacks.begin(), session->pendingCallbacks.end(),
        connection), session->pendingCallbacks.end());
    session->connectedCallbacks.erase(std::remove(session->connectedCallbacks.begin(),
        session->connectedCallbacks.end(), connection), session->connectedCallbacks.end());
    for (auto iter = session->pendingCallbackByServiceRemote.begin();
         iter != session->pendingCallbackByServiceRemote.end();) {
        if (iter->second == connection) {
            iter = session->pendingCallbackByServiceRemote.erase(iter);
            continue;
        }
        ++iter;
    }
    for (auto iter = session->standardServiceConnections.begin(); iter != session->standardServiceConnections.end();) {
        if (iter->second != nullptr && iter->second == serviceConnection) {
            iter = session->standardServiceConnections.erase(iter);
            continue;
        }
        ++iter;
    }
    session->callerRemotes.erase(std::remove(session->callerRemotes.begin(), session->callerRemotes.end(),
        callerRemote), session->callerRemotes.end());
    ReleaseTrackedConnectionByRemoteLocked(callerRemote);
}

int32_t AgentManagerService::CreateStandardAgentSession(const AAFwk::Want &connectWant,
    const std::string &agentId, const StandardAgentKey &key,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    if (serviceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }

    serviceConnection->SetStandardSessionMode();

    auto session = std::make_shared<StandardAgentSession>();
    session->key = key;
    session->state = StandardAgentState::CONNECTING;
    session->serviceConnection = serviceConnection;

    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        bool isNewCaller = false;
        auto ret = RegisterStandardSessionCallerLocked(session, connection, serviceConnection,
            !HasStandardAgentQuotaLocked(key), true, isNewCaller);
        if (ret != ERR_OK) {
            return ret;
        }
        standardSessions_.emplace_back(session);
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = FindStandardSessionByServiceRemoteLocked(serviceConnection->AsObject());
        if (sessionIt != standardSessions_.end()) {
            standardSessions_.erase(sessionIt);
        }
        return ret;
    }
    return ERR_OK;
}

bool AgentManagerService::HasStandardAgentQuotaLocked(const StandardAgentKey &key) const
{
    for (const auto &entry : standardSessions_) {
        if (entry != nullptr && !entry->isDisconnecting && IsStandardAgentQuotaKeyMatched(entry->key, key)) {
            return true;
        }
    }
    return false;
}

bool AgentManagerService::TransferStandardCallerLimitLocked(const StandardAgentKey &disconnectingKey,
    sptr<IRemoteObject> &transferFrom, sptr<IRemoteObject> &transferTo)
{
    sptr<IRemoteObject> currentQuotaRemote = nullptr;
    for (const auto &session : standardSessions_) {
        if (session == nullptr || !IsStandardAgentKeyEqual(session->key, disconnectingKey)) {
            continue;
        }
        for (const auto &callerRemote : session->callerRemotes) {
            auto trackedIt = trackedConnections_.find(callerRemote);
            if (trackedIt != trackedConnections_.end() && trackedIt->second.countTowardsCallerLimit) {
                currentQuotaRemote = callerRemote;
                break;
            }
        }
        break;
    }
    if (currentQuotaRemote == nullptr) {
        return false;
    }

    for (const auto &session : standardSessions_) {
        if (session == nullptr || session->isDisconnecting ||
            !IsStandardAgentQuotaKeyMatched(session->key, disconnectingKey) ||
            IsStandardAgentKeyEqual(session->key, disconnectingKey)) {
            continue;
        }
        for (const auto &callerRemote : session->callerRemotes) {
            auto trackedIt = trackedConnections_.find(callerRemote);
            if (trackedIt == trackedConnections_.end() || trackedIt->second.countTowardsCallerLimit) {
                continue;
            }
            trackedIt->second.countTowardsCallerLimit = true;
            trackedConnections_[currentQuotaRemote].countTowardsCallerLimit = false;
            transferFrom = currentQuotaRemote;
            transferTo = callerRemote;
            return true;
        }
    }
    return false;
}

void AgentManagerService::HandleStandardAgentConnectDone(
    const sptr<IRemoteObject> &serviceRemote,
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<sptr<AAFwk::IAbilityConnection>> failedCallbacks;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = FindStandardSessionByServiceRemoteLocked(serviceRemote);
        if (sessionIt == standardSessions_.end() || *sessionIt == nullptr) {
            return;
        }
        auto &session = *sessionIt;
        session->cachedElement = element;
        session->cachedRemoteObject = remoteObject;
        session->cachedResultCode = resultCode;

        if (!HandlePendingStandardConnectDoneLocked(serviceRemote, remoteObject, resultCode, sessionIt,
            callbacks, failedCallbacks)) {
            HandleLegacyStandardConnectDoneLocked(remoteObject, resultCode, sessionIt, callbacks, failedCallbacks);
        }
    }

    for (const auto &cb : callbacks) {
        if (cb != nullptr) {
            cb->OnAbilityConnectDone(element, remoteObject, resultCode);
        }
    }
    for (const auto &cb : failedCallbacks) {
        if (resultCode != ERR_OK) {
            ReleaseTrackedConnection(cb);
        }
    }
}

bool AgentManagerService::HandlePendingStandardConnectDoneLocked(const sptr<IRemoteObject> &serviceRemote,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode, const StandardSessionIter &sessionIt,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
    std::vector<sptr<AAFwk::IAbilityConnection>> &failedCallbacks)
{
    auto &session = *sessionIt;
    auto pendingIt = session->pendingCallbackByServiceRemote.find(serviceRemote);
    if (pendingIt == session->pendingCallbackByServiceRemote.end()) {
        return false;
    }
    auto callback = pendingIt->second;
    session->pendingCallbackByServiceRemote.erase(pendingIt);
    session->pendingCallbacks.erase(std::remove(session->pendingCallbacks.begin(),
        session->pendingCallbacks.end(), callback), session->pendingCallbacks.end());
    callbacks.emplace_back(callback);
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        session->state = StandardAgentState::CONNECTED;
        if (std::find(session->connectedCallbacks.begin(), session->connectedCallbacks.end(), callback) ==
            session->connectedCallbacks.end()) {
            session->connectedCallbacks.emplace_back(callback);
        }
        return true;
    }
    auto callerRemote = GetConnectionIdentityRemote(callback);
    session->standardServiceConnections.erase(serviceRemote);
    if (session->serviceConnection != nullptr && session->serviceConnection->AsObject() == serviceRemote) {
        session->serviceConnection = session->standardServiceConnections.empty() ?
            nullptr : session->standardServiceConnections.begin()->second;
    }
    session->callerRemotes.erase(std::remove(session->callerRemotes.begin(), session->callerRemotes.end(),
        callerRemote), session->callerRemotes.end());
    TransferStandardQuotaAfterCallerDiedLocked(session->key, callerRemote);
    failedCallbacks.emplace_back(callback);
    if (session->callerRemotes.empty() && session->pendingCallbacks.empty() && session->connectedCallbacks.empty()) {
        standardSessions_.erase(sessionIt);
    }
    return true;
}

void AgentManagerService::HandleLegacyStandardConnectDoneLocked(const sptr<IRemoteObject> &remoteObject,
    int32_t resultCode, const StandardSessionIter &sessionIt,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
    std::vector<sptr<AAFwk::IAbilityConnection>> &failedCallbacks)
{
    auto &session = *sessionIt;
    callbacks = session->pendingCallbacks;
    session->pendingCallbacks.clear();
    session->pendingCallbackByServiceRemote.clear();
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        session->state = StandardAgentState::CONNECTED;
        session->connectedCallbacks.insert(session->connectedCallbacks.end(), callbacks.begin(), callbacks.end());
        return;
    }
    failedCallbacks = callbacks;
    standardSessions_.erase(sessionIt);
}

void AgentManagerService::HandleStandardAgentDisconnectDone(
    const sptr<IRemoteObject> &serviceRemote,
    const AppExecFwk::ElementName &element, int32_t resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = std::find_if(standardSessions_.begin(), standardSessions_.end(),
            [&serviceRemote](const auto &item) {
                return item != nullptr && item->isDisconnecting &&
                    item->pendingDisconnectServiceRemotes.find(serviceRemote) !=
                    item->pendingDisconnectServiceRemotes.end();
            });
        if (sessionIt == standardSessions_.end() || *sessionIt == nullptr) {
            return;
        }
        auto &session = *sessionIt;
        session->pendingDisconnectServiceRemotes.erase(serviceRemote);
        if (!session->pendingDisconnectServiceRemotes.empty()) {
            return;
        }
        // Collect all callers: pending + connected
        callbacks = session->pendingCallbacks;
        session->pendingCallbacks.clear();
        callbacks.insert(callbacks.end(),
            session->connectedCallbacks.begin(), session->connectedCallbacks.end());
        session->connectedCallbacks.clear();
        // Erase session bookkeeping
        standardSessions_.erase(sessionIt);
    }

    for (const auto &cb : callbacks) {
        if (cb != nullptr) {
            cb->OnAbilityDisconnectDone(element, resultCode);
            HandleConnectionDone(cb, resultCode, true);
        }
    }
}
}  // namespace AgentRuntime
}  // namespace OHOS
