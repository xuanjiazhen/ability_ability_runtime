/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "intent_client.h"

#include "hilog/log.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "system_ability_definition.h"

#include "ability_manager_ipc_interface_code.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t ABILITY_SERVICE_NOT_CONNECTED = -1;
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001300, "IntentClient" };
constexpr const char *PERMISSION_DESC = "ohos.aafwk.AbilityManager";

std::u16string ToUtf16(const std::string &str)
{
    return std::u16string(str.begin(), str.end());
}

}

IntentClient &IntentClient::GetInstance()
{
    static IntentClient instance;
    return instance;
}

sptr<IRemoteObject> IntentClient::GetAbilityManagerRemote()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObj_ != nullptr) {
        return remoteObj_;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        HiLog::Error(LABEL, "get samgr failed");
        return nullptr;
    }
    remoteObj_ = samgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj_ == nullptr) {
        HiLog::Error(LABEL, "get ability manager failed");
        return nullptr;
    }
    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DeathRecipient());
    if (deathRecipient_ != nullptr && remoteObj_->IsProxyObject()) {
        remoteObj_->AddDeathRecipient(deathRecipient_);
    }
    return remoteObj_;
}

void IntentClient::ResetRemote(wptr<IRemoteObject> remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (remoteObj_ == remote.promote()) {
        remoteObj_ = nullptr;
    }
}

void IntentClient::DeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    HiLog::Info(LABEL, "ability manager remote died");
    IntentClient::GetInstance().ResetRemote(remote);
}

int32_t IntentClient::ExecuteIntentByFunctionCall(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, const std::string &intentName, const WantParams &wantParam)
{
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        HiLog::Error(LABEL, "service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ToUtf16(PERMISSION_DESC))) {
        HiLog::Error(LABEL, "write interface token failed");
        return -1;
    }
    if (!data.WriteUint64(key)) {
        HiLog::Error(LABEL, "write key failed");
        return -1;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        HiLog::Error(LABEL, "write callerToken failed");
        return -1;
    }
    if (!data.WriteString(bundleName)) {
        HiLog::Error(LABEL, "write bundleName failed");
        return -1;
    }
    if (!data.WriteString(intentName)) {
        HiLog::Error(LABEL, "write intentName failed");
        return -1;
    }
    if (!data.WriteParcelable(&wantParam)) {
        HiLog::Error(LABEL, "write wantParam failed");
        return -1;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL),
        data, reply, option);
    if (err != 0) {
        HiLog::Error(LABEL, "send request failed: %{public}d", err);
        return err;
    }
    return reply.ReadInt32();
}

} // namespace AAFwk
} // namespace OHOS
