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

#include "agent_manager_client.h"

#include "ability_manager_errors.h"
#include "agent_load_callback.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
AgentManagerClient &AgentManagerClient::GetInstance()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "GetInstance called");
    static AgentManagerClient instance;
    return instance;
}

int32_t AgentManagerClient::GetAllAgentCards(std::vector<AgentCard> &cards)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    AgentCardsRawData rawData;
    auto ret = agentMgr->GetAllAgentCards(rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get all failed: %{public}d", ret);
        return ret;
    }
    return AgentCardsRawData::ToAgentCardVec(rawData, cards);
}

int32_t AgentManagerClient::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    AgentCardsRawData rawData;
    auto ret = agentMgr->GetAgentCardsByBundleName(bundleName, rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get by bundle failed: %{public}d", ret);
        return ret;
    }
    return AgentCardsRawData::ToAgentCardVec(rawData, cards);
}

int32_t AgentManagerClient::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->GetAgentCardByAgentId(bundleName, agentId, card);
}

int32_t AgentManagerClient::GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->GetCallerAgentCardByAgentId(agentId, card);
}

int32_t AgentManagerClient::RegisterAgentCard(const AgentCard &card)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->RegisterAgentCard(card);
}

int32_t AgentManagerClient::UpdateAgentCard(const AgentCard &card)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->UpdateAgentCard(card);
}

int32_t AgentManagerClient::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->DeleteAgentCard(bundleName, agentId);
}

int32_t AgentManagerClient::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->ConnectAgentExtensionAbility(want, connection);
}

int32_t AgentManagerClient::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->DisconnectAgentExtensionAbility(connection);
}

int32_t AgentManagerClient::ConnectAgentExtensionAbilityForCli(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &callerIdentity)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->ConnectAgentExtensionAbilityForCli(want, connection, callerIdentity);
}

int32_t AgentManagerClient::DisconnectAgentExtensionAbilityForCli(
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &callerIdentity)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->DisconnectAgentExtensionAbilityForCli(connection, callerIdentity);
}

int32_t AgentManagerClient::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->ConnectServiceExtensionAbility(callerToken, want, connection);
}

int32_t AgentManagerClient::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->DisconnectServiceExtensionAbility(callerToken, connection);
}

int32_t AgentManagerClient::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->NotifyLowCodeAgentComplete(agentId);
}

int32_t AgentManagerClient::GetAgentCardTypeForConnect(AAFwk::Want &want, int32_t &cardType)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->GetAgentCardTypeForConnect(want, cardType);
}

int32_t AgentManagerClient::VerifyAgentConnectRequest(const AAFwk::Want &want,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->VerifyAgentConnectRequest(want, connection, callerIdentity);
}

int32_t AgentManagerClient::VerifyAgentDisconnectRequests(const std::vector<AAFwk::Want> &wants,
    const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity)
{
    auto agentMgr = GetAgentMgrProxy();
    if (agentMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentmgr");
        return ERR_NULL_AGENT_MGR_PROXY;
    }
    return agentMgr->VerifyAgentDisconnectRequests(wants, connection, callerIdentity);
}

sptr<IAgentManager> AgentManagerClient::GetAgentMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto agentMgr = GetAgentMgr();
    if (agentMgr != nullptr) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "agent manager has been started");
        return agentMgr;
    }

    // Serialize load + registration: without this, two concurrent first callers both pass the
    // empty-cache check, both load, and both register a death recipient on the same remote, so a
    // later service death fires the notification twice (the second one only produces noise in
    // ClearProxyIfMatch). Re-check the cache after acquiring the lock.
    std::lock_guard<std::mutex> registerLock(registerMutex_);
    agentMgr = GetAgentMgr();
    if (agentMgr != nullptr) {
        return agentMgr;
    }

    if (!LoadAgentMgrService()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Load agent manager service failed");
        return nullptr;
    }

    agentMgr = GetAgentMgr();
    if (agentMgr == nullptr || agentMgr->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get agent manager");
        return nullptr;
    }

    if (!RegisterDeathRecipient(agentMgr->AsObject())) {
        // The remote died between GetSystemAbility and registration. Drop the cached proxy so the
        // next call retries the load instead of serving a proxy whose death can never be observed.
        SetAgentMgr(nullptr);
        return nullptr;
    }

    return agentMgr;
}

bool AgentManagerClient::RegisterDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        return false;
    }
    sptr<IRemoteObject> registered = registeredRemote_.promote();
    auto cached = GetAgentMgr();
    if (cached != nullptr && cached->AsObject() == remoteObject && registered == remoteObject) {
        // This live remote is the one currently cached and already carries our recipient
        // (idempotent re-delivery, or a sequential install of the same remote through another
        // path): do not attach a second recipient.
        return true;
    }
    // The singleton is a function-local static whose lifetime covers every thread, so capturing
    // nothing and using GetInstance() inside the callback is safe. (enable_shared_from_this /
    // weak_from_this is unusable here: the instance is never owned by a shared_ptr, so the weak
    // would always be empty and the death notification a no-op.) ClearProxyIfMatch compares and
    // clears under mutex_ so check-and-clear is a single critical section.
    const auto &onClearProxyCallback = [](const wptr<IRemoteObject> &remote) {
        GetInstance().ClearProxyIfMatch(remote);
    };

    sptr<AgentManagerServiceDeathRecipient> recipient =
        new (std::nothrow) AgentManagerServiceDeathRecipient(onClearProxyCallback);
    if (recipient == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create death recipient");
        return false;
    }
    // AddDeathRecipient is a binder call issued while the caller holds registerMutex_; the death
    // callback only takes mutex_, so even an immediate notification cannot deadlock. A false
    // return usually means the remote is already dead: report it so callers do not cache the proxy.
    if (!remoteObject->AddDeathRecipient(recipient)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "AddDeathRecipient failed");
        return false;
    }
    registeredRemote_ = remoteObject;
    return true;
}

void AgentManagerClient::ClearProxyIfMatch(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // agentMgr_ is an iface_cast broker proxy, NOT the remote object itself: comparing
    // agentMgr_ == remote would compare two different heap objects and always be false. Compare
    // the proxy's underlying remote (AsObject()) with the died remote instead.
    if (agentMgr_ == nullptr) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr death notification after proxy already cleared, ignore");
        return;
    }
    if (agentMgr_->AsObject() != remote) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "stale agentmgr death notification, ignore");
        return;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr service died, clear cached proxy");
    agentMgr_ = nullptr;
}

void AgentManagerClient::AgentManagerServiceDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (proxy_ != nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agentmgr service died");
        proxy_(remote);
    }
}

bool AgentManagerClient::LoadAgentMgrService()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetSystemAbilityManager");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get SystemAbilityManager");
        return false;
    }

    auto remoteObject = systemAbilityMgr->GetSystemAbility(AGENT_MGR_SERVICE_ID);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetSystemAbility %{public}d failed", AGENT_MGR_SERVICE_ID);
        return false;
    }

    SetAgentMgr(remoteObject);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "GetSystemAbility success");
    return true;
}

void AgentManagerClient::SetAgentMgr(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(mutex_);
    agentMgr_ = iface_cast<IAgentManager>(remoteObject);
}

sptr<IAgentManager> AgentManagerClient::GetAgentMgr()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return agentMgr_;
}

void AgentManagerClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    // Take registerMutex_ so this path cannot interleave with GetAgentMgrProxy's load+register
    // sequence, and register the death recipient here as well: without it, proxies installed via
    // this callback would never self-heal after a service death (stale cache until process exit).
    std::lock_guard<std::mutex> registerLock(registerMutex_);
    // A late or replayed callback for an older service instance must not clobber a live proxy
    // installed by another path: overwriting it with a dead remote would leave the cache holding
    // a proxy whose death can never be observed, so the cache would never recover.
    auto cached = GetAgentMgr();
    if (cached != nullptr && cached->AsObject() != remoteObject) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Ignore stale load callback: cached proxy differs");
        return;
    }
    // Register before caching: if the remote is already dead, AddDeathRecipient fails and the
    // cache must not be populated with a proxy that can never self-heal.
    if (!RegisterDeathRecipient(remoteObject)) {
        return;
    }
    SetAgentMgr(remoteObject);
}

void AgentManagerClient::OnLoadSystemAbilityFail()
{
    // Same serialization as the success path so load callbacks of both kinds stay ordered
    // against GetAgentMgrProxy's load+register sequence.
    std::lock_guard<std::mutex> registerLock(registerMutex_);
    SetAgentMgr(nullptr);
}
}  // namespace AgentRuntime
}  // namespace OHOS
