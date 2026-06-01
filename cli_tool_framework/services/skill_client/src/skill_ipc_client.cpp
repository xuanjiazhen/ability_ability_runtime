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

#include "skill_ipc_client.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "skill_callback_stub_lite.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char16_t *AMS_INTERFACE_DESCRIPTOR = u"ohos.aafwk.AbilityManager";

class AmsDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit AmsDeathRecipient(SkillIpcClient &client) : client_(client) {}
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        client_.ResetProxy(remote);
    }

private:
    SkillIpcClient &client_;
};
} // namespace

SkillIpcClient &SkillIpcClient::GetInstance()
{
    static SkillIpcClient instance;
    return instance;
}

bool SkillIpcClient::WriteInterfaceToken(MessageParcel &data)
{
    return data.WriteInterfaceToken(AMS_INTERFACE_DESCRIPTOR);
}

sptr<IRemoteObject> SkillIpcClient::GetAbilityManagerProxy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ != nullptr) {
        return proxy_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return nullptr;
    }
    proxy_ = samgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (proxy_ == nullptr) {
        return nullptr;
    }
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(
            new (std::nothrow) AmsDeathRecipient(*this));
    }
    if (deathRecipient_ != nullptr && proxy_->IsProxyObject()) {
        proxy_->AddDeathRecipient(deathRecipient_);
    }
    return proxy_;
}

void SkillIpcClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (proxy_ == nullptr || proxy_ != remote.promote()) {
        return;
    }
    proxy_->RemoveDeathRecipient(deathRecipient_);
    proxy_ = nullptr;
}

int32_t SkillIpcClient::QuerySkillType(const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName, int32_t &skillType)
{
    auto proxy = GetAbilityManagerProxy();
    if (proxy == nullptr) {
        return ERR_INVALID_STATE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_STATE;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName)) ||
        !data.WriteString16(Str8ToStr16(moduleName)) ||
        !data.WriteString16(Str8ToStr16(skillName))) {
        return ERR_INVALID_STATE;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = proxy->SendRequest(QUERY_SKILL_TYPE_CODE, data, reply, option);
    if (ret != NO_ERROR) {
        return ret;
    }
    int32_t result = reply.ReadInt32();
    if (result == ERR_OK) {
        skillType = reply.ReadInt32();
    }
    return result;
}

int32_t SkillIpcClient::ExecuteInAppSkillWithTokenId(
    uint32_t callerTokenId, const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName,
    const std::string &scriptPath, const std::string &functionName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs,
    const sptr<SkillCallbackLite> &callback)
{
    auto proxy = GetAbilityManagerProxy();
    if (proxy == nullptr) {
        return ERR_INVALID_STATE;
    }

    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_STATE;
    }
    if (!data.WriteUint32(callerTokenId)) {
        return ERR_INVALID_STATE;
    }
    if (!data.WriteString16(Str8ToStr16(bundleName)) ||
        !data.WriteString16(Str8ToStr16(moduleName)) ||
        !data.WriteString16(Str8ToStr16(skillName)) ||
        !data.WriteString16(Str8ToStr16(scriptPath)) ||
        !data.WriteString16(Str8ToStr16(functionName))) {
        return ERR_INVALID_STATE;
    }
    auto paramsToWrite = (skillArgs != nullptr)
        ? skillArgs : std::make_shared<AAFwk::WantParams>();
    if (!data.WriteParcelable(paramsToWrite.get())) {
        return ERR_INVALID_STATE;
    }
    bool hasCallback = callback != nullptr;
    if (!data.WriteBool(hasCallback) ||
        (hasCallback && !data.WriteRemoteObject(callback.GetRefPtr()))) {
        return ERR_INVALID_STATE;
    }

    MessageParcel reply;
    MessageOption option;
    auto ret = proxy->SendRequest(EXECUTE_IN_APP_SKILL_CODE, data, reply, option);
    if (ret != NO_ERROR) {
        return ret;
    }
    return reply.ReadInt32();
}

} // namespace CliTool
} // namespace OHOS
