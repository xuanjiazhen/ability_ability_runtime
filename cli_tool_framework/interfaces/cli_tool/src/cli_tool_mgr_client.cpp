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

#include "cli_tool_mgr_client.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "cli_event_reply_manager.h"
#include "cli_session_subscription_manager.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NO_INIT = -1;

std::string CliEventReplyTypeToString(const CliEventReplyType &type)
{
    switch (type) {
        case CliEventReplyType::EXEC_TOOL:
            return "-ExecTool-";
        case CliEventReplyType::SEND_MESSAGE:
            return "-SendMessage-";
        default:
            return "-Unknown-";
    }
}
} // namespace

CliToolMGRClient& CliToolMGRClient::GetInstance()
{
    static CliToolMGRClient instance;
    return instance;
}

sptr<ICliToolManager> CliToolMGRClient::GetCliToolManager()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!proxy_) {
        (void)Connect();
    }

    return proxy_;
}

ErrCode CliToolMGRClient::Connect()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return ERR_OK;
    }
    sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "get registry failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    sptr<IRemoteObject> remoteObj = systemManager->GetSystemAbility(CLI_TOOL_MGR_SERVICE_ID);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new CliSaDeathRecipient());
    if (deathRecipient_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create CliSaDeathRecipient failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    if ((remoteObj->IsProxyObject()) && (!remoteObj->AddDeathRecipient(deathRecipient_))) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "add deathRecipient failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    schedulerRegistered_ = false;
    proxy_ = iface_cast<ICliToolManager>(remoteObj);
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "iface_cast failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Connect cli manager service success.");
    return ERR_OK;
}

void CliToolMGRClient::SetServiceDeathHandler(const ServiceDeathHandler &handler)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    serviceDeathHandlers_.emplace_back(std::move(handler));
}

void CliToolMGRClient::CliSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliSaDeathRecipient handle remote died.");
    CliToolMGRClient::GetInstance().ResetProxy(remote);
}

ErrCode CliToolMGRClient::ExecTool(const ExecToolParam &param,
                                   const std::map<std::string, std::string> &args,
                                   const ExecToolReplyCallback &callback)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    ErrCode ret = EnsureSchedulerRegistered(proxy);
    if (ret != ERR_OK) {
        return ret;
    }

    std::string prefix = param.toolName + CliEventReplyTypeToString(CliEventReplyType::EXEC_TOOL);
    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(prefix,
        CliEventReplyType::EXEC_TOOL,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                CliSessionInfo sessionInfo;
                if (result.sessionInfo.has_value()) {
                    sessionInfo = result.sessionInfo.value();
                }
                cb(result.code, sessionInfo);
            }
        });
    ret = proxy->ExecTool(param, args, eventId);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }
    return ret;
}

void CliToolMGRClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::vector<ServiceDeathHandler> serviceDeathHandlers;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!proxy_) {
            return;
        }

        auto serviceRemote = proxy_->AsObject();
        if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
            TAG_LOGD(AAFwkTag::CLI_TOOL, "To remove death recipient.");
            serviceRemote->RemoveDeathRecipient(deathRecipient_);
            proxy_ = nullptr;
        }
        schedulerRegistered_ = false;
        serviceDeathHandlers = serviceDeathHandlers_;
    }

    for (auto &handler : serviceDeathHandlers) {
        if (handler) {
            handler();
        }
    }

    CliEventReplyManager::GetInstance().ClearAllEvent();
    CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
}

ErrCode CliToolMGRClient::EnsureSchedulerRegistered(const sptr<ICliToolManager> &proxy)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (schedulerRegistered_) {
        return ERR_OK;
    }
    if (schedulerStub_ == nullptr) {
        schedulerStub_ = new (std::nothrow) CliToolManagerSchedulerRecipient(
            [](const std::string &sessionId, const std::string &subscriptionId, const CliToolEvent &event) {
                return CliSessionSubscriptionManager::GetInstance().HandleSessionEvent(
                    sessionId, subscriptionId, event);
            },
            [](const std::string &eventId, const CliEventReplyResult &result) {
                return CliEventReplyManager::GetInstance().HandleEventReply(eventId, result);
            });
    }

    if (schedulerStub_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "EnsureSchedulerRegistered prerequisites are missing");
        return ERR_NO_INIT;
    }
    ErrCode ret = proxy->RegisterScheduler(schedulerStub_->AsObject());
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterScheduler failed: %{public}d", ret);
        return ret;
    }
    schedulerRegistered_ = true;
    return ret;
}

ErrCode CliToolMGRClient::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetAllToolSummaries(summaries);
}

ErrCode CliToolMGRClient::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetToolInfoByName(name, tool);
}

ErrCode CliToolMGRClient::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetAllToolInfos(tools);
}

ErrCode CliToolMGRClient::RegisterTool(const ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->RegisterTool(tool);
}

ErrCode CliToolMGRClient::SubscribeSession(const std::string &sessionId,
                                           const std::shared_ptr<SessionEventCallback> &callback,
                                           std::string &subscriptionId)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    ErrCode ret = EnsureSchedulerRegistered(proxy);
    if (ret != ERR_OK) {
        return ret;
    }

    subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription(sessionId,
        [callback](const std::string &sessionId,
            const std::string &subscriptionId, const CliToolEvent &event) {
            if (callback) {
                callback->OnToolEvent(sessionId, subscriptionId, event);
            }
        });
    ret = proxy->SubscribeSession(sessionId, subscriptionId);
    if (ret != ERR_OK) {
        CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
        subscriptionId = "";
    } else {
        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    }
    return ret;
}

ErrCode CliToolMGRClient::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);

    return proxy->UnsubscribeSession(sessionId, subscriptionId);
}

ErrCode CliToolMGRClient::ClearSession(const std::string &sessionId)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->ClearSession(sessionId);
}

ErrCode CliToolMGRClient::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->QuerySession(sessionId, session);
}

ErrCode CliToolMGRClient::SendMessage(const std::string &sessionId,
                                      const std::string &inputText,
                                      const EventReplyCallback &callback)
{
    auto proxy = GetCliToolManager();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return AAFwk::GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    ErrCode ret = EnsureSchedulerRegistered(proxy);
    if (ret != ERR_OK) {
        return ret;
    }

    std::string prefix = sessionId + CliEventReplyTypeToString(CliEventReplyType::SEND_MESSAGE);
    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(prefix,
        CliEventReplyType::SEND_MESSAGE,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                cb(result.code);
            }
        });
    ret = proxy->SendMessage(sessionId, inputText, eventId);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }
    return ret;
}
} // namespace CliTool
} // namespace OHOS
