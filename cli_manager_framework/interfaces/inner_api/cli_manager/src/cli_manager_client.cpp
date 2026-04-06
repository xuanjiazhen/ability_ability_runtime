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

#include "cli_manager_client.h"

#include "cli_manager_proxy.h"
#include "cli_manager_scheduler_stub.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t CLI_MANAGER_SA_ID = 8899; // TODO: Use proper SA ID
constexpr int32_t ERR_OK = 0;

class CliManagerSchedulerRecipient final : public CliManagerSchedulerStub {
public:
    explicit CliManagerSchedulerRecipient(const std::function<int32_t(
        const std::string &, const std::string &, const ToolEvent &)> &handler)
        : handler_(handler)
    {}

    ~CliManagerSchedulerRecipient() override = default;

    int32_t ScheduleSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
        const ToolEvent &event) override
    {
        if (!handler_) {
            return ERROR_CODE;
        }
        return handler_(sessionId, subscriptionId, event);
    }

private:
    std::function<int32_t(const std::string &, const std::string &, const ToolEvent &)> handler_;
};
} // namespace

CliManagerClient& CliManagerClient::GetInstance()
{
    static CliManagerClient instance;
    return instance;
}

void CliManagerClient::CliMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (proxy_) {
        proxy_(remote);
    }
}

sptr<ICliManager> CliManagerClient::GetCliManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return cliManager_;
}

bool CliManagerClient::LoadCliManagerService()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: start");
    if (cliManager_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: cliManager_ already exists");
        return true;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get SystemAbilityManager failed");
        return false;
    }

    auto remoteObject = samgr->GetSystemAbility(CLI_MANAGER_SA_ID);
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get CliManagerService failed - SA not found");
        return false;
    }

    SetCliManager(remoteObject);
    return true;
}

void CliManagerClient::SetCliManager(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(mutex_);

    cliManager_ = iface_cast<ICliManager>(remoteObject);
    if (!cliManager_ || !cliManager_->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create CliManagerProxy failed");
        cliManager_ = nullptr;
        return;
    }

    schedulerRegistered_ = false;
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(
        new CliMgrDeathRecipient([this](const wptr<IRemoteObject> &remote) {
            ClearProxy();
        }));

    if (deathRecipient_) {
        remoteObject->AddDeathRecipient(deathRecipient_);
    }
}

void CliManagerClient::ClearProxy()
{
    ServiceDeathHandler serviceDeathHandler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        cliManager_ = nullptr;
        schedulerRegistered_ = false;
        serviceDeathHandler = serviceDeathHandler_;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService died, clear proxy");
    if (serviceDeathHandler) {
        serviceDeathHandler();
    }
}

sptr<ICliManager> CliManagerClient::GetCliManager()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cliManager_ != nullptr && cliManager_->AsObject() != nullptr) {
            return cliManager_;
        }
    }

    if (!LoadCliManagerService()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Load CliManagerService failed");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (cliManager_ == nullptr || cliManager_->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliManager proxy");
        cliManager_ = nullptr;
        return nullptr;
    }
    return cliManager_;
}

int32_t CliManagerClient::EnsureSchedulerRegistered(const sptr<ICliManager> &proxy)
{
    sptr<ICliManagerScheduler> schedulerStub;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (schedulerRegistered_) {
            return ERR_OK;
        }
        schedulerStub = schedulerStub_;
    }

    if (proxy == nullptr || schedulerStub == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "EnsureSchedulerRegistered prerequisites are missing");
        return ERROR_CODE;
    }

    int32_t ret = proxy->RegisterScheduler(schedulerStub);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterScheduler failed: %{public}d", ret);
        return ret;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    schedulerRegistered_ = true;
    return ERR_OK;
}

int32_t CliManagerClient::DispatchSessionEvent(const std::string &sessionId, const std::string &subscriptionId,
    const ToolEvent &event)
{
    SessionEventHandler handler;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handler = sessionEventHandler_;
    }

    if (!handler) {
        return ERROR_CODE;
    }

    handler(sessionId, subscriptionId, event);
    return ERR_OK;
}

int32_t CliManagerClient::QueryTools(std::vector<ToolInfo> &tools)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->QueryTools(tools);
}

int32_t CliManagerClient::ExecTool(const std::string &name,
    const std::vector<std::pair<std::string, std::string>> &args,
    const ExecOptions &options,
    const std::string &challenge,
    SessionInfo &session)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->ExecTool(name, args, options, challenge, session);
}

int32_t CliManagerClient::CancelSession(const std::string &sessionId)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->CancelSession(sessionId);
}

int32_t CliManagerClient::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }

    int32_t ret = EnsureSchedulerRegistered(proxy);
    if (ret != ERR_OK) {
        return ret;
    }

    return proxy->SubscribeSession(sessionId, subscriptionId);
}

int32_t CliManagerClient::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }

    return proxy->UnsubscribeSession(sessionId, subscriptionId);
}

int32_t CliManagerClient::QuerySessionStatus(const std::string &sessionId, SessionStatus &status)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->QuerySessionStatus(sessionId, status);
}

void CliManagerClient::SetSessionEventHandler(SessionEventHandler handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessionEventHandler_ = std::move(handler);
    if (schedulerStub_ == nullptr) {
        schedulerStub_ = sptr<ICliManagerScheduler>(new (std::nothrow) CliManagerSchedulerRecipient(
            [this](const std::string &sessionId, const std::string &subscriptionId, const ToolEvent &event) {
                return DispatchSessionEvent(sessionId, subscriptionId, event);
            }));
    }
    schedulerRegistered_ = false;
}

void CliManagerClient::SetServiceDeathHandler(ServiceDeathHandler handler)
{
    std::lock_guard<std::mutex> lock(mutex_);
    serviceDeathHandler_ = std::move(handler);
}

} // namespace AbilityRuntime
} // namespace OHOS
