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
#include "insight_intent_host_client.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "system_ability_definition.h"

#include "ability_manager_errors.h"
#include "ability_manager_ipc_interface_code.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t ABILITY_SERVICE_NOT_CONNECTED = -1;
constexpr const char *PERMISSION_DESC = "ohos.aafwk.AbilityManager";
constexpr const char *TAG = "IntentClient";

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
        HILOG_ERROR("get samgr failed");
        return nullptr;
    }
    remoteObj_ = samgr->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remoteObj_ == nullptr) {
        HILOG_ERROR("get ability manager failed");
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
    HILOG_INFO("ability manager remote died");
    IntentClient::GetInstance().ResetRemote(remote);
}

int32_t IntentClient::SendExecuteRequest(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, const std::string &intentName, const WantParams &wantParam)
{
    auto remote = GetAbilityManagerRemote();
    if (remote == nullptr) {
        HILOG_ERROR("service not connected");
        return ABILITY_SERVICE_NOT_CONNECTED;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(ToUtf16(PERMISSION_DESC))) {
        HILOG_ERROR("write interface token failed");
        return INNER_ERR;
    }
    if (!data.WriteUint64(key)) {
        HILOG_ERROR("write key failed");
        return INNER_ERR;
    }
    if (!data.WriteRemoteObject(callerToken)) {
        HILOG_ERROR("write callerToken failed");
        return INNER_ERR;
    }
    if (!data.WriteString(bundleName)) {
        HILOG_ERROR("write bundleName failed");
        return INNER_ERR;
    }
    if (!data.WriteString(intentName)) {
        HILOG_ERROR("write intentName failed");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&wantParam)) {
        HILOG_ERROR("write wantParam failed");
        return INNER_ERR;
    }
    int32_t err = remote->SendRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_INTENT_BY_FUNCTION_CALL),
        data, reply, option);
    if (err != 0) {
        HILOG_ERROR("send request failed: %{public}d", err);
        return err;
    }
    return reply.ReadInt32();
}

int32_t IntentClient::ExecuteIntentByFunctionCall(const std::string &bundleName,
    const std::string &intentName, const WantParams &wantParam,
    const std::shared_ptr<AbilityRuntime::InsightIntentExecuteCallbackInterface> &callback)
{
    if (callback == nullptr) {
        HILOG_ERROR("null callback");
        return ERR_INVALID_VALUE;
    }

    auto hostClient = AbilityRuntime::InsightIntentHostClient::GetInstance();
    uint64_t key = hostClient->AddInsightIntentExecute(callback);

    int32_t err = SendExecuteRequest(key, hostClient, bundleName, intentName, wantParam);
    if (err != ERR_OK) {
        hostClient->RemoveInsightIntentExecute(key);
    }
    return err;
}

} // namespace AAFwk
} // namespace OHOS
