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
#include <limits>
#include <random>
#include <utility>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_manager_caller_identity.h"
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
using namespace std::chrono;

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
constexpr size_t MAX_AGENT_CONNECTIONS_PER_CALLER = 5;
constexpr size_t MAX_LOW_CODE_AGENTS_PER_HOST = 100;
constexpr size_t MAX_AGENT_CONNECT_PREFLIGHTS = 1024;
constexpr auto AGENT_CONNECT_PREFLIGHT_TIMEOUT = minutes(1);
constexpr const char *AGENT_CONNECT_PREFLIGHT_CLEANUP_TASK = "AgentConnectPreflightCleanup";
constexpr const char *FOUNDATION_PROCESS_NAME = "foundation";

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

bool IsConnectPreflightTargetMatched(const AAFwk::Want &want, const AgentConnectPreflightRecord &record)
{
    return want.GetStringParam(AGENTID_KEY) == record.agentId &&
        want.GetElement() == record.connectWant.GetElement() &&
        want.GetBundle() == record.connectWant.GetBundle() &&
        want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0) ==
            record.connectWant.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
}

void NormalizeAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card)
{
    if (card.appInfo == nullptr) {
        return;
    }
    auto element = connectWant.GetElement();
    if (element.GetModuleName().empty() && !card.appInfo->moduleName.empty()) {
        element.SetModuleName(card.appInfo->moduleName);
        connectWant.SetElement(element);
    }
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
        connectPreflights_.clear();
        connectPreflightCleanupScheduled_ = false;
        callerQuotas_.clear();
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

    // Step 2: consume the optional preflight nonce, but do not trust it as current target metadata.
    AAFwk::Want preflightWant;
    std::string preflightAgentId;
    AgentCard preflightCard;
    bool hasPreflight = TryConsumeConnectPreflight(want, callerUid, preflightWant, preflightAgentId, preflightCard);

    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (hasPreflight && preflightCard.type != card.type) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight type is stale");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    connectWant.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(card.type));

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

int32_t AgentManagerService::GetAgentCardTypeForConnect(AAFwk::Want &want, int32_t &cardType)
{
    int32_t callerUid = 0;
    auto ret = ValidateConnectAgentCaller(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }

    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    ret = ResolveConnectAgentTarget(want, connectWant, agentId, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    connectWant.SetParam(AGENT_CARD_TYPE_KEY, static_cast<int32_t>(card.type));
    RegisterConnectPreflight(connectWant, agentId, card, callerUid);
    want = connectWant;
    cardType = static_cast<int32_t>(card.type);
    return ERR_OK;
}

int32_t AgentManagerService::ConnectStandardAgentExtensionAbility(AAFwk::Want &connectWant,
    const std::string &agentId, const AgentCard &card, const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid)
{
    auto ret = PrepareStandardAgentConnectWant(connectWant, card, callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        connectWant.GetBundle().c_str(), agentId.c_str());

    AgentQuotaKey quotaKey = BuildStandardQuotaKey(connectWant, agentId, callerUid);
    ret = AdmitStandardAgentQuota(callerUid, quotaKey);
    if (ret != ERR_OK) {
        return ret;
    }

    sptr<AAFwk::IAbilityConnection> serviceConnection;
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, callerUid, serviceConnection);
    if (ret != ERR_OK) {
        ReleaseCallerQuota(callerUid, quotaKey);
        return ret;
    }

    long verificationNonce = GenerateAgentVerificationNonce();
    connectWant.SetParam(AGENT_VERIFICATION_NONCE_KEY, verificationNonce);
    ret = RequestStandardAgentConnect(connectWant, connection, agentId, quotaKey, verificationNonce,
        serviceConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::AdmitStandardAgentQuota(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    return AdmitStandardAgentConnectionLocked(callerUid, quotaKey);
}

void AgentManagerService::ReleaseCallerQuota(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    ReleaseCallerQuotaLocked(callerUid, quotaKey);
}

int32_t AgentManagerService::RequestStandardAgentConnect(const AAFwk::Want &connectWant,
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId,
    const AgentQuotaKey &quotaKey, long verificationNonce,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    AgentManagerCallerIdentityScope identityScope;
    auto ret = SetTrackedConnectionIdentityAndQuota(connection, agentId, identityScope.GetOriginalIdentity(),
        quotaKey, verificationNonce);
    if (ret != ERR_OK) {
        return ret;
    }
    return AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
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
    ret = RegisterTrackedConnectionAndGetServiceConnection(connection, IPCSkeleton::GetCallingUid(),
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
    auto ret = ValidateConnectAgentPermission(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return ERR_INVALID_VALUE;
    }
    return ValidateConnectCallerForeground();
}

int32_t AgentManagerService::ValidateConnectAgentCaller(int32_t &callerUid) const
{
    auto ret = ValidateConnectAgentPermission(callerUid);
    if (ret != ERR_OK) {
        return ret;
    }
    return ValidateConnectCallerForeground();
}

long AgentManagerService::RegisterConnectPreflight(AAFwk::Want &connectWant, const std::string &agentId,
    const AgentCard &card, int32_t callerUid)
{
    long nonce = GenerateAgentVerificationNonce();
    auto now = AgentPreflightClock::now();
    std::lock_guard<std::mutex> lock(connectionLock_);
    PruneExpiredConnectPreflightsLocked(now);
    while (connectPreflights_.find(nonce) != connectPreflights_.end()) {
        nonce = GenerateAgentVerificationNonce();
    }
    connectWant.SetParam(AGENT_VERIFICATION_NONCE_KEY, nonce);
    AgentConnectPreflightRecord record;
    record.callerUid = callerUid;
    record.connectWant = connectWant;
    record.agentId = agentId;
    record.card = card;
    record.expiresAt = now + AGENT_CONNECT_PREFLIGHT_TIMEOUT;
    if (connectPreflights_.size() >= MAX_AGENT_CONNECT_PREFLIGHTS) {
        EraseOldestConnectPreflightLocked();
    }
    connectPreflights_[nonce] = record;
    ScheduleConnectPreflightCleanupLocked(record.expiresAt);
    return nonce;
}

bool AgentManagerService::TryConsumeConnectPreflight(const AAFwk::Want &want, int32_t callerUid,
    AAFwk::Want &connectWant, std::string &agentId, AgentCard &card)
{
    long nonce = want.GetLongParam(AGENT_VERIFICATION_NONCE_KEY, 0);
    if (nonce <= 0) {
        return false;
    }
    std::lock_guard<std::mutex> lock(connectionLock_);
    PruneExpiredConnectPreflightsLocked(AgentPreflightClock::now());
    auto iter = connectPreflights_.find(nonce);
    if (iter == connectPreflights_.end()) {
        return false;
    }
    if (iter->second.callerUid != callerUid) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight caller mismatch");
        connectPreflights_.erase(iter);
        return false;
    }
    if (!IsConnectPreflightTargetMatched(want, iter->second)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight target mismatch");
        connectPreflights_.erase(iter);
        return false;
    }
    connectWant = iter->second.connectWant;
    agentId = iter->second.agentId;
    card = iter->second.card;
    connectPreflights_.erase(iter);
    return true;
}

void AgentManagerService::ScheduleConnectPreflightCleanupLocked(AgentPreflightTimePoint expiresAt)
{
    if (taskHandler_ == nullptr) {
        return;
    }
    if (connectPreflightCleanupScheduled_ && connectPreflightCleanupAt_ <= expiresAt) {
        return;
    }
    connectPreflightCleanupScheduled_ = true;
    connectPreflightCleanupAt_ = expiresAt;
    auto task = [expiresAt]() {
        auto service = AgentManagerService::GetInstance();
        if (service != nullptr) {
            service->CleanupExpiredConnectPreflights(expiresAt);
        }
    };
    taskHandler_->SubmitTaskJust(task, AGENT_CONNECT_PREFLIGHT_CLEANUP_TASK,
        GetConnectPreflightCleanupDelayMillis(expiresAt));
}

void AgentManagerService::CleanupExpiredConnectPreflights(AgentPreflightTimePoint scheduledAt)
{
    std::lock_guard<std::mutex> lock(connectionLock_);
    if (!connectPreflightCleanupScheduled_ || connectPreflightCleanupAt_ != scheduledAt) {
        return;
    }
    connectPreflightCleanupScheduled_ = false;
    PruneExpiredConnectPreflightsLocked(AgentPreflightClock::now());
    if (!connectPreflights_.empty()) {
        ScheduleConnectPreflightCleanupLocked(GetEarliestConnectPreflightExpiryLocked());
    }
}

void AgentManagerService::PruneExpiredConnectPreflightsLocked(AgentPreflightTimePoint now)
{
    for (auto iter = connectPreflights_.begin(); iter != connectPreflights_.end();) {
        if (iter->second.expiresAt > now) {
            ++iter;
            continue;
        }
        iter = connectPreflights_.erase(iter);
    }
}

AgentPreflightTimePoint AgentManagerService::GetEarliestConnectPreflightExpiryLocked() const
{
    auto earliest = std::min_element(connectPreflights_.begin(), connectPreflights_.end(),
        [](const auto &left, const auto &right) {
            return left.second.expiresAt < right.second.expiresAt;
        });
    return earliest == connectPreflights_.end() ? AgentPreflightTimePoint::max() :
        earliest->second.expiresAt;
}

int64_t AgentManagerService::GetConnectPreflightCleanupDelayMillis(AgentPreflightTimePoint expiresAt) const
{
    auto now = AgentPreflightClock::now();
    if (expiresAt <= now) {
        return 0;
    }
    return duration_cast<milliseconds>(expiresAt - now).count();
}

void AgentManagerService::EraseOldestConnectPreflightLocked()
{
    auto oldestExpiry = GetEarliestConnectPreflightExpiryLocked();
    auto oldest = std::find_if(connectPreflights_.begin(), connectPreflights_.end(),
        [oldestExpiry](const auto &item) {
            return item.second.expiresAt == oldestExpiry;
        });
    if (oldest != connectPreflights_.end()) {
        connectPreflights_.erase(oldest);
    }
}

int32_t AgentManagerService::ValidateConnectAgentPermission(int32_t &callerUid) const
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
    callerUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::ValidateConnectCallerForeground() const
{
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
    NormalizeAgentConnectWant(connectWant, card);

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
        std::string startTime = std::to_string(duration_cast<milliseconds>(
            system_clock::now().time_since_epoch()).count());
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
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
    sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    // Register the caller callback and allocate the service-side wrapper connection.
    int32_t ret = ERR_OK;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        ret = TryRegisterConnectionLocked(connection, callerUid);
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

AgentQuotaKey AgentManagerService::BuildStandardQuotaKey(
    const AAFwk::Want &want, const std::string &agentId, int32_t callingUid) const
{
    AgentQuotaKey key;
    key.hostKey = BuildAgentHostKey(want, callingUid);
    key.agentId = agentId;
    key.isLowCode = false;
    return key;
}

AgentQuotaKey AgentManagerService::BuildLowCodeQuotaKey(const AgentHostKey &hostKey) const
{
    AgentQuotaKey key;
    key.hostKey = hostKey;
    key.isLowCode = true;
    return key;
}

int32_t AgentManagerService::AdmitStandardAgentConnectionLocked(int32_t callerUid,
    const AgentQuotaKey &quotaKey)
{
    auto &callerQuota = callerQuotas_[callerUid];
    if (callerQuota.find(quotaKey) == callerQuota.end() &&
        callerQuota.size() >= MAX_AGENT_CONNECTIONS_PER_CALLER) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT caller quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    callerQuota[quotaKey]++;
    return ERR_OK;
}

int32_t AgentManagerService::AdmitLowCodeAgentLocked(int32_t callerUid, const AgentQuotaKey &quotaKey,
    const AgentHostSession &session, const std::string &agentId)
{
    if (session.agents.find(agentId) != session.agents.end()) {
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }
    if (session.agents.size() >= MAX_LOW_CODE_AGENTS_PER_HOST) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code AGENT host quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    bool callerAlreadyOwnsHost = std::any_of(session.agents.begin(), session.agents.end(),
        [callerUid](const auto &entry) {
            return entry.second.callerUid == callerUid;
        });
    if (callerAlreadyOwnsHost) {
        return ERR_OK;
    }
    auto &callerQuota = callerQuotas_[callerUid];
    if (callerQuota.find(quotaKey) == callerQuota.end() &&
        callerQuota.size() >= MAX_AGENT_CONNECTIONS_PER_CALLER) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT caller quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    callerQuota[quotaKey] = 1;
    return ERR_OK;
}

void AgentManagerService::ReleaseCallerQuotaLocked(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    auto callerIter = callerQuotas_.find(callerUid);
    if (callerIter == callerQuotas_.end()) {
        return;
    }
    auto quotaIter = callerIter->second.find(quotaKey);
    if (quotaIter == callerIter->second.end()) {
        return;
    }
    if (quotaIter->second > 1) {
        quotaIter->second--;
        return;
    }
    callerIter->second.erase(quotaIter);
    if (callerIter->second.empty()) {
        callerQuotas_.erase(callerIter);
    }
}

void AgentManagerService::ReleaseTrackedConnectionQuotaLocked(TrackedConnectionRecord &record)
{
    if (!record.hasQuota || record.isLowCode) {
        return;
    }
    AgentQuotaKey quotaKey;
    quotaKey.hostKey = record.hostKey;
    quotaKey.agentId = record.agentId;
    quotaKey.isLowCode = false;
    ReleaseCallerQuotaLocked(record.callerUid, quotaKey);
    record.hasQuota = false;
}

void AgentManagerService::ReleaseLowCodeAgentQuotaLocked(AgentHostSession &session, const std::string &agentId,
    int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    bool callerStillOwnsHost = false;
    for (const auto &entry : session.agents) {
        if (entry.first == agentId) {
            continue;
        }
        if (entry.second.callerUid == callerUid) {
            callerStillOwnsHost = true;
            break;
        }
    }
    if (!callerStillOwnsHost) {
        ReleaseCallerQuotaLocked(callerUid, quotaKey);
    }
}

int32_t AgentManagerService::SetTrackedConnectionIdentityAndQuota(
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId,
    const std::string &originalIdentity, const AgentQuotaKey &quotaKey, long verificationNonce)
{
    std::lock_guard<std::mutex> lock(connectionLock_);
    auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
    if (it == trackedConnections_.end()) {
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    it->second.agentId = agentId;
    it->second.hostKey = quotaKey.hostKey;
    it->second.originalIdentity = originalIdentity;
    it->second.verificationNonce = verificationNonce;
    it->second.hasQuota = true;
    return ERR_OK;
}

long AgentManagerService::GenerateAgentVerificationNonce() const
{
    constexpr long minVerificationNonce = 1000000000L;
    thread_local std::mt19937_64 engine(std::random_device{}());
    std::uniform_int_distribution<long> dist(minVerificationNonce, std::numeric_limits<long>::max());
    return dist(engine);
}

bool AgentManagerService::IsAgentVerificationNonceMatched(const AAFwk::Want &want, long verificationNonce) const
{
    return verificationNonce > 0 &&
        want.GetLongParam(AGENT_VERIFICATION_NONCE_KEY, 0) == verificationNonce;
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto ret = ValidateDisconnectAgentRequest(connection);
    if (ret != ERR_OK) {
        return ret;
    }

    AgentDisconnectRequest request;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        ret = PrepareAgentDisconnectRequestLocked(connection, request);
    }
    if (ret != ERR_OK || request.alreadyDisconnecting) {
        return ret;
    }
    if (request.isLowCode) {
        return DisconnectLowCodeTrackedConnection(request.callerRemote, request.hostKey, request.lowCodeAgentIds);
    }
    return RequestStandardAgentDisconnect(request);
}

int32_t AgentManagerService::ValidateDisconnectAgentRequest(
    const sptr<AAFwk::IAbilityConnection> &connection) const
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
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

int32_t AgentManagerService::PrepareAgentDisconnectRequestLocked(
    const sptr<AAFwk::IAbilityConnection> &connection, AgentDisconnectRequest &request)
{
    auto it = FindTrackedConnectionLocked(connection, IPCSkeleton::GetCallingUid());
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
        return ERR_INVALID_VALUE;
    }
    request.callerRemote = it->first;
    if (it->second.isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    if (it->second.isLowCode) {
        return PrepareLowCodeDisconnectRequestLocked(it, request);
    }
    it->second.isDisconnecting = true;
    request.serviceConnection = it->second.serviceConnection;
    return ERR_OK;
}

int32_t AgentManagerService::PrepareLowCodeDisconnectRequestLocked(
    TrackedConnectionIter trackedIter, AgentDisconnectRequest &request)
{
    auto sessionIter = agentHostSessions_.find(trackedIter->second.hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
        return ERR_INVALID_VALUE;
    }
    request.hostKey = trackedIter->second.hostKey;
    request.lowCodeAgentIds = trackedIter->second.lowCodeAgentIds;
    request.isLowCode = true;
    return ERR_OK;
}

int32_t AgentManagerService::RequestStandardAgentDisconnect(const AgentDisconnectRequest &request)
{
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(request.serviceConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        RestoreStandardAgentDisconnectingState(request.callerRemote);
        return ret;
    }
    return ERR_OK;
}

void AgentManagerService::RestoreStandardAgentDisconnectingState(const sptr<IRemoteObject> &callerRemote)
{
    std::lock_guard<std::mutex> lock(connectionLock_);
    auto it = trackedConnections_.find(callerRemote);
    if (it != trackedConnections_.end() && it->second.isDisconnecting) {
        it->second.isDisconnecting = false;
    }
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
        serviceConnection = it->second.serviceConnection;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(serviceConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto it = trackedConnections_.find(connection->AsObject());
        if (it != trackedConnections_.end() && it->second.isDisconnecting) {
            it->second.isDisconnecting = false;
        }
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::VerifyAgentConnectRequest(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS_NAME)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-foundation AMS confirmation caller");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    auto remote = GetConnectionIdentityRemote(connection);
    if (remote == nullptr) {
        return ERR_INVALID_VALUE;
    }
    const std::string agentId = want.GetStringParam(AGENTID_KEY);
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto ret = VerifyStandardAgentConnectRequestLocked(remote, agentId, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    ret = VerifyLowCodeAgentConnectRequestLocked(remote, agentId, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT connect is not owned by AgentMgr");
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::VerifyStandardAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
    const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : trackedConnections_) {
        const auto &record = entry.second;
        if (record.isLowCode || record.serviceConnection == nullptr ||
            record.serviceConnection->AsObject() != remote) {
            continue;
        }
        if (!agentId.empty() && !record.agentId.empty() && record.agentId != agentId) {
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT connect nonce mismatch");
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        if (record.originalIdentity.empty()) {
            return ERR_INVALID_VALUE;
        }
        callerIdentity = record.originalIdentity;
        return ERR_OK;
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::VerifyLowCodeAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
    const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : agentHostSessions_) {
        const auto &session = entry.second;
        if (session == nullptr) {
            continue;
        }
        for (const auto &agentEntry : session->agents) {
            const auto &record = agentEntry.second;
            if (record.hostConnection == nullptr || record.hostConnection->AsObject() != remote) {
                continue;
            }
            if (!agentId.empty() && agentEntry.first != agentId) {
                continue;
            }
            if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT host connect nonce mismatch");
                return AAFwk::ERR_WRONG_INTERFACE_CALL;
            }
            if (record.originalIdentity.empty()) {
                return ERR_INVALID_VALUE;
            }
            callerIdentity = record.originalIdentity;
            return ERR_OK;
        }
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::VerifyAgentDisconnectRequest(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    if (!AAFwk::PermissionVerification::GetInstance()->CheckSpecificSystemAbilityAccessPermission(
        FOUNDATION_PROCESS_NAME)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "non-foundation AMS confirmation caller");
        return AAFwk::CHECK_PERMISSION_FAILED;
    }
    auto remote = GetConnectionIdentityRemote(connection);
    if (remote == nullptr) {
        return ERR_INVALID_VALUE;
    }
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto ret = VerifyStandardAgentDisconnectRequestLocked(remote, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    ret = VerifyLowCodeAgentDisconnectRequestLocked(remote, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT disconnect is not owned by AgentMgr");
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::VerifyStandardAgentDisconnectRequestLocked(const sptr<IRemoteObject> &remote,
    const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : trackedConnections_) {
        const auto &record = entry.second;
        if (record.isLowCode || record.serviceConnection == nullptr ||
            record.serviceConnection->AsObject() != remote) {
            continue;
        }
        if (record.originalIdentity.empty()) {
            return ERR_INVALID_VALUE;
        }
        if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT disconnect nonce mismatch");
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        callerIdentity = record.originalIdentity;
        return ERR_OK;
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::VerifyLowCodeAgentDisconnectRequestLocked(const sptr<IRemoteObject> &remote,
    const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : agentHostSessions_) {
        const auto &session = entry.second;
        if (session == nullptr) {
            continue;
        }
        for (const auto &agentEntry : session->agents) {
            const auto &record = agentEntry.second;
            if (record.hostConnection == nullptr || record.hostConnection->AsObject() != remote) {
                continue;
            }
            if (record.originalIdentity.empty()) {
                return ERR_INVALID_VALUE;
            }
            if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT host disconnect nonce mismatch");
                return AAFwk::ERR_WRONG_INTERFACE_CALL;
            }
            callerIdentity = record.originalIdentity;
            return ERR_OK;
        }
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentManagerService::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid, const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey)
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

    auto actualServiceConnection = CreateServiceConnection(connection, serviceConnection);
    if (actualServiceConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
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
    AddCallerDeathRecipient(record);

    trackedConnections_.emplace(callerRemote, record);
    return ERR_OK;
}

sptr<AAFwk::IAbilityConnection> AgentManagerService::CreateServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection) const
{
    if (serviceConnection != nullptr) {
        return serviceConnection;
    }
    return sptr<AgentServiceConnection>::MakeSptr(connection);
}

void AgentManagerService::AddCallerDeathRecipient(TrackedConnectionRecord &record)
{
    if (record.callerRemote == nullptr) {
        return;
    }
    auto handler = [callerRemote = record.callerRemote](const wptr<IRemoteObject> &remote) {
        (void)remote;
        auto service = AgentManagerService::GetInstance();
        if (service != nullptr) {
            service->HandleCallerConnectionDied(callerRemote);
        }
    };
    record.deathRecipient = sptr<AAFwk::AbilityConnectCallbackRecipient>::MakeSptr(std::move(handler));
    if (record.deathRecipient != nullptr) {
        record.callerRemote->AddDeathRecipient(record.deathRecipient);
    }
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

    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    ReleaseTrackedConnectionQuotaLocked(it->second);
    it->second.verificationNonce = 0;
    trackedConnections_.erase(it);
}

void AgentManagerService::ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return;
    }

    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    ReleaseTrackedConnectionQuotaLocked(it->second);
    it->second.verificationNonce = 0;
    trackedConnections_.erase(it);
}

void AgentManagerService::HandleCallerConnectionDied(const sptr<IRemoteObject> &remote)
{
    AgentCallerDeathRequest request;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        if (!PrepareCallerDeathRequestLocked(remote, request)) {
            return;
        }
    }

    DisconnectAfterCallerDeath(request, remote);
}

bool AgentManagerService::PrepareCallerDeathRequestLocked(
    const sptr<IRemoteObject> &remote, AgentCallerDeathRequest &request)
{
    if (remote == nullptr) {
        return false;
    }
    auto it = trackedConnections_.find(remote);
    if (it == trackedConnections_.end()) {
        return false;
    }

    if (!it->second.isLowCode) {
        request.serviceConnection = it->second.serviceConnection;
        it->second.isDisconnecting = true;
        return true;
    }
    request.hostKey = it->second.hostKey;
    PrepareLowCodeCallerDeathLocked(remote, it->second.hostKey, request);
    ReleaseTrackedConnectionByRemoteLocked(remote);
    return !request.lowCodeTargets.empty();
}

void AgentManagerService::PrepareLowCodeCallerDeathLocked(
    const sptr<IRemoteObject> &remote, const AgentHostKey &hostKey, AgentCallerDeathRequest &request)
{
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        return;
    }

    auto session = sessionIter->second;
    std::set<std::string> ownedAgentIds;
    auto trackedIter = trackedConnections_.find(remote);
    if (trackedIter != trackedConnections_.end()) {
        ownedAgentIds = trackedIter->second.lowCodeAgentIds;
    }
    session->callerConnections.erase(remote);
    if (ownedAgentIds.empty()) {
        for (const auto &agentEntry : session->agents) {
            if (agentEntry.second.callerRemote == remote) {
                ownedAgentIds.insert(agentEntry.first);
            }
        }
    }
    for (const auto &agentId : ownedAgentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "owned low-code agent not active: %{public}s", agentId.c_str());
            continue;
        }
        if (agentIter->second.callerRemote != remote) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code agent owner remote changed: %{public}s", agentId.c_str());
            continue;
        }
        if (agentIter->second.isDisconnecting) {
            continue;
        }
        if (agentIter->second.hostConnection == nullptr) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code agent host connection missing: %{public}s", agentId.c_str());
            continue;
        }
        agentIter->second.isDisconnecting = true;
        auto hostRemote = agentIter->second.hostConnection->AsObject();
        auto targetIter = std::find_if(request.lowCodeTargets.begin(), request.lowCodeTargets.end(),
            [hostRemote](const LowCodeDisconnectTarget &target) {
                return target.hostConnection != nullptr && target.hostConnection->AsObject() == hostRemote;
            });
        if (targetIter == request.lowCodeTargets.end()) {
            request.lowCodeTargets.emplace_back(
                LowCodeDisconnectTarget { { agentIter->first }, agentIter->second.hostConnection });
        } else {
            targetIter->agentIds.insert(agentIter->first);
        }
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "Low-code caller died targetAgentCount=%{public}zu, activeAgentCount=%{public}zu",
        request.lowCodeTargets.size(), session->agents.size());
}

void AgentManagerService::DisconnectAfterCallerDeath(
    const AgentCallerDeathRequest &request, const sptr<IRemoteObject> &remote)
{
    if (request.serviceConnection != nullptr) {
        auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(request.serviceConnection);
        if (ret != ERR_OK) {
            std::lock_guard<std::mutex> lock(connectionLock_);
            ReleaseTrackedConnectionByRemoteLocked(remote);
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
    }
    for (const auto &target : request.lowCodeTargets) {
        if (target.hostConnection == nullptr) {
            continue;
        }
        auto ret = RequestLowCodeHostDisconnect(request.hostKey, target.hostConnection, target.agentIds);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "DisconnectAbility after caller death failed: %{public}d", ret);
        }
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
    if (isDisconnect || resultCode != ERR_OK) {
        ReleaseTrackedConnection(connection);
    }
}

}  // namespace AgentRuntime
}  // namespace OHOS
