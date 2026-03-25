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
    if (cliManager_) {
        return true;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!samgr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get SystemAbilityManager failed");
        return false;
    }

    auto remoteObject = samgr->GetSystemAbility(CLI_MANAGER_SA_ID);
    if (!remoteObject) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get CliManagerService failed");
        return false;
    }

    SetCliManager(remoteObject);
    return true;
}

void CliManagerClient::SetCliManager(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(mutex_);

    cliManager_ = iface_cast<ICliManager>(remoteObject);
    if (!cliManager_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create CliManagerProxy failed");
        return;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(
        new CliMgrDeathRecipient([this](const wptr<IRemoteObject> &remote) {
            ClearProxy();
        }));

    if (deathRecipient_) {
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
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cliManager_) {
        if (!LoadCliManagerService()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Load CliManagerService failed");
            return nullptr;
        }
    }
    return cliManager_;
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
                                    const std::map<std::string, std::string> &args,
                                    const ExecOptions &options,
                                    ExecResult &result)
{
    auto proxy = GetCliManager();
    if (!proxy) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Proxy is null");
        return ERROR_CODE;
    }
    return proxy->ExecTool(name, args, options, result);
}

} // namespace AbilityRuntime
} // namespace OHOS
