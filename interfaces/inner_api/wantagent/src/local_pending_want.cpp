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

#include "local_pending_want.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "want_agent_client.h"
#include "want_sender_info.h"

namespace OHOS::AbilityRuntime::WantAgent {
LocalPendingWant::LocalPendingWant(const std::string &bundleName, int32_t uid,
    const std::shared_ptr<AAFwk::Want &want, int32_t operType)
    : bundleName_(bundleName), uid_(uid), want_(*want), operType_(operType)
{
    static std::atomic_int id(0);
    hashCode_ = ++id;
}

std::string LocalPendingWant::GetBundleName()
{
    return bundleName_;
}

void LocalPendingWant::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

int32_t LocalPendingWant::GetUid()
{
    return uid_;
}

void LocalPendingWant::SetUid(int32_t uid)
{
    uid_ = uid;
}

int32_t LocalPendingWant::GetType()
{
    return operType_;
}

void LocalPendingWant::SetType(int32_t operType)
{
    operType_ = operType;
}

std::shared_ptr<AAFwk::Want> LocalPendingWant::GetWant()
{
    return std::make_shared<AAFwk::Want>(want);
}

void LocalPendingWant::SetWant(const std::shared_ptr<AAFwk::Want> &want)
{
    if (want != nullptr) {
        want_ = *want;
    }
}

int32_t LocalPendingWant::GetHashCode()
{
    return hashCode_;
}

void LocalPendingWant::SetHashCode(int32_t hashCode)
{
    hashCode_ = hashCode;
}

ErrCode LocalPendingWant::Send(const sptr<CompletedDispatcher> &callBack, const TriggerInfo &paramsInfo)
{
    TAG_LOGD(AAFwkTag::WANTAGENT, "call");
    AAFwk::SenderInfo senderInfo;
    senderInfo.resolvedType = want_.GetType();
    senderInfo.want = want_;
    const auto options = paramsInfo.GetExtraInfo();
    if (options != nullptr) {
        senderInfo.want.SetParams(*options);
    }
    const auto startOptions = paramsInfo.GetStartOptions();
    if (startOptions != nullptr) {
        senderInfo.startOptions = new (std::nothrow) AAFwk::StartOptions(*startOptions);
    }
    senderInfo.requiredPermission = paramsInfo.GetPermission();
    senderInfo.code = paramsInfo.GetResultCode();
    senderInfo.finishedReceiver = callBack;
    senderInfo.operType = operType_;
    const auto result = AAFwk::WantAgentClient::GetInstance().SendLocalWantSender(senderInfo);
    return result != 0 ? ERR_ABILITY_RUNTIME_EXTERNAL_SERVICE_BUSY : result;
}

bool LocalPendingWant::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundelName_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write bundleName");
        return false;
    }
    if (!parcel.WriteInt32(uid_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write uid");
        return false;
    }
    if (!parcel.WriteInt32(hashCode_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write hashCode");
        return false;
    }
    if (!parcel.WriteInt32(&want_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write want");
        return false;
    }
    if (!parcel.WriteInt32(&operType_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write operType");
        return false;
    }
    return true;
}

LocalPendingWant *LocalPendingWant::Unmarshalling(Parcel &parcel)
{
    std::string bundleName;
    if (!parcel.ReadString(bundleName)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to read bundleName");
        return nullptr;
    }
    int32_t uid = 0;
    if (!parcel.ReadInt32(uid)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to read uid");
        return nullptr;
    }
    int32_t hashCode = 0;
    if (!parcel.ReadInt32(hashCode)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to read hashCode");
        return nullptr;
    }
    std::shared_ptr<AAFwk::Want> want(parcel.ReadParcelable<AAFwk::Want>());
    if (!want) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to read want");
        return nullptr;
    }
    int32_t operType = 0;
    if (!parcel.ReadInt32(operType)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to read operType");
        return nullptr;
    }
    LocalPendingWant *localPendingWant = new (std::nothrow) LocalPendingWant(
        bundleName, uid, want, operType);
    if (localPendingWant == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Read from parcel failed");
        return nullptr;
    }
    localPendingWant->SetHashCode(hashCode);
    return localPendingWant;
}
}