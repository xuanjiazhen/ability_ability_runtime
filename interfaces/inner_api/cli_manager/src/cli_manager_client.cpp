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

#include "cli_manager_client.h"
#include "cli_manager_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t CLI_MANAGER_SA_ID = 9999; // TODO: Use proper SA ID
}

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

    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: getting SystemAbilityManager");
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get SystemAbilityManager failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: getting CliManagerService SA %{public}d", CLI_MANAGER_SA_ID);
    auto remoteObject = samgr->GetSystemAbility(CLI_MANAGER_SA_ID);
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get CliManagerService failed - SA not found");
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: got remote object, calling SetCliManager");
    SetCliManager(remoteObject);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadCliManagerService: completed successfully");
    return true;
}

void CliManagerClient::SetCliManager(const sptr<IRemoteObject> &remoteObject)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetCliManager: start");
    std::lock_guard<std::mutex> lock(mutex_);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetCliManager: calling iface_cast");
    cliManager_ = iface_cast<ICliManager>(remoteObject);
    if (!cliManager_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create CliManagerProxy failed - iface_cast returned null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetCliManager: iface_cast succeeded, checking AsObject");
    if (!cliManager_->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create CliManagerProxy failed - AsObject is null");
        cliManager_ = nullptr;
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetCliManager: creating death recipient");
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(
        new CliMgrDeathRecipient([this](const wptr<IRemoteObject> &remote) {
            ClearProxy();
        }));

    if (deathRecipient_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "SetCliManager: adding death recipient");
        remoteObject->AddDeathRecipient(deathRecipient_);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Connect to CliManagerService success");
}

void CliManagerClient::ClearProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    cliManager_ = nullptr;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService died, clear proxy");
}

sptr<ICliManager> CliManagerClient::GetCliManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetCliManager: start");

    // First check: quick read with lock
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cliManager_ != nullptr && cliManager_->AsObject() != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "GetCliManager: returning cached proxy");
            return cliManager_;
        }
    }

    // Load service without holding lock (SetCliManager will acquire it)
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetCliManager: calling LoadCliManagerService");
    if (!LoadCliManagerService()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Load CliManagerService failed");
        return nullptr;
    }

    // Second check: verify result with lock
    std::lock_guard<std::mutex> lock(mutex_);
    if (cliManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliManager proxy - cliManager_ is null");
        return nullptr;
    }

    if (cliManager_->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliManager proxy - AsObject is null");
        cliManager_ = nullptr;
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetCliManager: returning new proxy");
    return cliManager_;
}

int32_t CliManagerClient::QueryTools(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerClient QueryTools called");
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "About to call proxy->QueryTools");
    return proxy->QueryTools(tools);
}

int32_t CliManagerClient::ExecTool(const std::string &name,
                                    const std::map<std::string, std::string> &args,
                                    const ExecOptions &options,
                                    ExecResult &result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerClient ExecTool called");
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->ExecTool(name, args, options, result);
}

} // namespace AbilityRuntime
} // namespace OHOS
