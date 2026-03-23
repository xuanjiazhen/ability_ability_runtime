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

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "agent_bundle_event_callback.h"
#include "agent_card_mgr.h"
#include "agent_config.h"
#include "agent_receiver_proxy.h"
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
bool IsWorkflowTargetMatched(const AAFwk::Want &want, const AgentCard &card)
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

AgentManagerService::WorkflowHostConnection::WorkflowHostConnection(const WorkflowHostKey &key) : key_(key)
{}

void AgentManagerService::WorkflowHostConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    auto service = AgentManagerService::GetInstance();
    if (service == nullptr) {
        return;
    }
    service->HandleWorkflowConnectDone(key_, element, remoteObject, resultCode);
}

void AgentManagerService::WorkflowHostConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    auto service = AgentManagerService::GetInstance();
    if (service == nullptr) {
        return;
    }
    service->HandleWorkflowDisconnectDone(key_, element, resultCode);
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
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().GetAllAgentCards(cards);
}

int32_t AgentManagerService::GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    auto ret = AgentCardMgr::GetInstance().GetAgentCardsByBundleName(bundleName, cards);
    if (ret == ERR_NAME_NOT_FOUND) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "no agent cards of bundle %{public}s", bundleName.c_str());
        int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;
        AppExecFwk::ApplicationInfo appInfo;
        auto queryRet = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->GetApplicationInfo(
                bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, appInfo));
        if (!queryRet) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "bundle unexist");
            return AAFwk::ERR_BUNDLE_NOT_EXIST;
        }
        return ERR_OK;
    }
    return ret;
}

int32_t AgentManagerService::GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId,
    AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_GET_AGENT_CARD)) {
        int32_t callerPid = IPCSkeleton::GetCallingPid();
        std::string callerBundleName;
        int32_t callerUid;
        int32_t ret = IN_PROCESS_CALL(
            DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetBundleNameByPid(
                callerPid, callerBundleName, callerUid));
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "getBundleNameByPid failed %{public}d", ret);
            return ERR_PERMISSION_DENIED;
        }
        if (bundleName != callerBundleName) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
            return ERR_PERMISSION_DENIED;
        }
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no agent card of agentId %{public}s", agentId.c_str());
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    return ret;
}

int32_t AgentManagerService::RegisterAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().RegisterAgentCard(card);
}

int32_t AgentManagerService::UpdateAgentCard(const AgentCard &card)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_MODIFY_AGENT_CARD)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    return AgentCardMgr::GetInstance().UpdateAgentCard(card);
}

int32_t AgentManagerService::DeleteAgentCard(const std::string &bundleName, const std::string &agentId)
{
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
    // Validate permission
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
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

    AAFwk::Want connectWant = want;
    std::string agentId = connectWant.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    AgentCard card;
    int32_t cardRet = AgentCardMgr::GetInstance().GetAgentCardByAgentId(connectWant.GetBundle(), agentId, card);
    if (cardRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "no such card");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    if (card.type == AgentCardType::WORKFLOW && !IsWorkflowTargetMatched(connectWant, card)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "workflow target mismatch");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    if (card.type == AgentCardType::WORKFLOW) {
        return ConnectWorkflowAgentExtensionAbility(connectWant, agentId, connection, callingUid);
    }
    bool isAtomicServiceAgent = card.type == AgentCardType::ATOMIC_SERVICE;
    int32_t userId = callingUid / BASE_USER_RANGE;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            connectWant, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if ((!queryResult || extensionInfos.empty()) && !isAtomicServiceAgent) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (queryResult && !extensionInfos.empty() && extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }
    if (isAtomicServiceAgent) {
        connectWant.AddFlags(AAFwk::Want::FLAG_INSTALL_ON_DEMAND);
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        connectWant.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "connecting %{public}s-%{public}s",
        connectWant.GetBundle().c_str(), agentId.c_str());

    // Validate connection object
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid connection object");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, connection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection)
{
    // Validate permission
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

    sptr<WorkflowHostConnection> hostConnection = nullptr;
    {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        for (auto &[key, session] : workflowHostSessions_) {
            if (session == nullptr) {
                continue;
            }
            auto callbackIter = std::find(session->callbacks.begin(), session->callbacks.end(), connection);
            if (callbackIter == session->callbacks.end()) {
                continue;
            }
            if (session->isDisconnecting) {
                return ERR_OK;
            }
            session->isDisconnecting = true;
            EraseWorkflowOwnersLocked(*session);
            session->activeAgentIds.clear();
            session->pendingAgentIds.clear();
            hostConnection = session->hostConnection;
            break;
        }
    }
    if (hostConnection != nullptr) {
        auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
        if (ret != ERR_OK) {
            std::lock_guard<std::mutex> lock(workflowMutex_);
            auto sessionIter = std::find_if(workflowHostSessions_.begin(), workflowHostSessions_.end(),
                [hostConnection](const auto &item) {
                    return item.second != nullptr && item.second->hostConnection == hostConnection;
                });
            if (sessionIter != workflowHostSessions_.end()) {
                sessionIter->second->isDisconnecting = false;
            }
            TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
            return ret;
        }
        return ERR_OK;
    }

    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connection));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }

    return ERR_OK;
}

int32_t AgentManagerService::NotifyWorkflowAgentComplete(const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    sptr<WorkflowHostConnection> hostConnection = nullptr;
    {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        WorkflowAgentOwnerKey ownerKey { IPCSkeleton::GetCallingUid(), agentId };
        auto ownerIter = workflowAgentOwners_.find(ownerKey);
        if (ownerIter == workflowAgentOwners_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "workflow agent not found");
            return AAFwk::ERR_INVALID_AGENT_CARD_ID;
        }
        auto session = ownerIter->second;
        if (session == nullptr) {
            workflowAgentOwners_.erase(ownerIter);
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        session->activeAgentIds.erase(agentId);
        session->pendingAgentIds.erase(agentId);
        workflowAgentOwners_.erase(ownerIter);
        if (!session->activeAgentIds.empty() || session->isDisconnecting) {
            return ERR_OK;
        }
        session->isDisconnecting = true;
        hostConnection = session->hostConnection;
    }
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection));
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        auto sessionIter = std::find_if(workflowHostSessions_.begin(), workflowHostSessions_.end(),
            [hostConnection](const auto &item) {
                return item.second != nullptr && item.second->hostConnection == hostConnection;
            });
        if (sessionIter != workflowHostSessions_.end()) {
            sessionIter->second->isDisconnecting = false;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ConnectWorkflowAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid)
{
    WorkflowHostKey hostKey = BuildWorkflowHostKey(want, callingUid);
    sptr<WorkflowHostConnection> hostConnection = nullptr;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject = nullptr;
    int32_t cachedResultCode = ERR_OK;
    bool notifyExistingConnection = false;
    bool needRealConnect = false;
    {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        WorkflowAgentOwnerKey ownerKey { callingUid, agentId };
        if (workflowAgentOwners_.find(ownerKey) != workflowAgentOwners_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "workflow agent already active");
            return ERR_WORKFLOW_AGENT_ALREADY_ACTIVE;
        }
        auto sessionIter = workflowHostSessions_.find(hostKey);
        if (sessionIter != workflowHostSessions_.end()) {
            auto session = sessionIter->second;
            if (session == nullptr) {
                workflowHostSessions_.erase(sessionIter);
            } else {
                session->callbacks.push_back(connection);
                session->activeAgentIds.insert(agentId);
                workflowAgentOwners_[ownerKey] = session;
                if (session->isConnected && session->remoteObject != nullptr) {
                    cachedElement = session->element;
                    cachedRemoteObject = session->remoteObject;
                    cachedResultCode = session->resultCode;
                    notifyExistingConnection = true;
                } else {
                    session->pendingAgentIds.insert(agentId);
                }
                hostConnection = session->hostConnection;
            }
        }
        if (!notifyExistingConnection && hostConnection == nullptr) {
            auto session = std::make_shared<WorkflowHostSession>();
            session->key = hostKey;
            session->hostConnection = sptr<WorkflowHostConnection>::MakeSptr(hostKey);
            session->callbacks.push_back(connection);
            session->activeAgentIds.insert(agentId);
            session->pendingAgentIds.insert(agentId);
            hostConnection = session->hostConnection;
            workflowHostSessions_[hostKey] = session;
            workflowAgentOwners_[ownerKey] = session;
            needRealConnect = true;
        }
    }
    if (notifyExistingConnection) {
        WorkflowHostSession callbackSession;
        callbackSession.remoteObject = cachedRemoteObject;
        NotifyWorkflowInvokedLocked(callbackSession, agentId);
        connection->OnAbilityConnectDone(cachedElement, cachedRemoteObject, cachedResultCode);
        return ERR_OK;
    }
    if (!needRealConnect) {
        return ERR_OK;
    }

    int32_t userId = callingUid / BASE_USER_RANGE;
    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    bool queryResult = IN_PROCESS_CALL(
        DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance()->QueryExtensionAbilityInfos(
            want, AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, extensionInfos));
    if (!queryResult || extensionInfos.empty()) {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        workflowAgentOwners_.erase(WorkflowAgentOwnerKey { callingUid, agentId });
        workflowHostSessions_.erase(hostKey);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "extension ability not exist");
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (extensionInfos[0].type != AppExecFwk::ExtensionAbilityType::AGENT) {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        workflowAgentOwners_.erase(WorkflowAgentOwnerKey { callingUid, agentId });
        workflowHostSessions_.erase(hostKey);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "incorrect extension");
        return AAFwk::ERR_WRONG_INTERFACE_CALL;
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        want, hostConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        workflowAgentOwners_.erase(WorkflowAgentOwnerKey { callingUid, agentId });
        workflowHostSessions_.erase(hostKey);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

AgentManagerService::WorkflowHostKey AgentManagerService::BuildWorkflowHostKey(const AAFwk::Want &want,
    int32_t callingUid) const
{
    WorkflowHostKey key;
    key.callingUid = callingUid;
    key.bundleName = want.GetElement().GetBundleName();
    key.moduleName = want.GetElement().GetModuleName();
    key.abilityName = want.GetElement().GetAbilityName();
    return key;
}

bool AgentManagerService::NotifyWorkflowInvokedLocked(const WorkflowHostSession &session, const std::string &agentId)
{
    if (session.remoteObject == nullptr) {
        return false;
    }
    auto receiver = iface_cast<IAgentReceiver>(session.remoteObject);
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "workflow receiver null");
        return false;
    }
    receiver->WorkflowInvoked(agentId);
    return true;
}

void AgentManagerService::EraseWorkflowOwnersLocked(const WorkflowHostSession &session)
{
    for (const auto &agentId : session.activeAgentIds) {
        workflowAgentOwners_.erase(WorkflowAgentOwnerKey { session.key.callingUid, agentId });
    }
}

void AgentManagerService::HandleWorkflowConnectDone(const WorkflowHostKey &key, const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> pendingAgentIds;
    {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        auto sessionIter = workflowHostSessions_.find(key);
        if (sessionIter == workflowHostSessions_.end()) {
            return;
        }
        auto session = sessionIter->second;
        if (session == nullptr) {
            workflowHostSessions_.erase(sessionIter);
            return;
        }
        session->element = element;
        session->remoteObject = remoteObject;
        session->resultCode = resultCode;
        session->isConnected = (resultCode == ERR_OK && remoteObject != nullptr);
        callbacks = session->callbacks;
        pendingAgentIds.assign(session->pendingAgentIds.begin(), session->pendingAgentIds.end());
        session->pendingAgentIds.clear();
        if (!session->isConnected) {
            ClearWorkflowSessionLocked(key);
        }
    }
    WorkflowHostSession callbackSession;
    callbackSession.remoteObject = remoteObject;
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        for (const auto &agentId : pendingAgentIds) {
            NotifyWorkflowInvokedLocked(callbackSession, agentId);
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
        }
    }
}

void AgentManagerService::HandleWorkflowDisconnectDone(const WorkflowHostKey &key,
    const AppExecFwk::ElementName &element, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::lock_guard<std::mutex> lock(workflowMutex_);
        auto sessionIter = workflowHostSessions_.find(key);
        if (sessionIter == workflowHostSessions_.end()) {
            return;
        }
        if (sessionIter->second == nullptr) {
            workflowHostSessions_.erase(sessionIter);
            return;
        }
        callbacks = sessionIter->second->callbacks;
        ClearWorkflowSessionLocked(key);
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityDisconnectDone(element, resultCode);
        }
    }
}

void AgentManagerService::ClearWorkflowSessionLocked(const WorkflowHostKey &key)
{
    auto sessionIter = workflowHostSessions_.find(key);
    if (sessionIter == workflowHostSessions_.end()) {
        return;
    }
    if (sessionIter->second != nullptr) {
        EraseWorkflowOwnersLocked(*sessionIter->second);
    }
    workflowHostSessions_.erase(sessionIter);
}
}  // namespace AgentRuntime
}  // namespace OHOS
