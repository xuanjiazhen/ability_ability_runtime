/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "want_sender_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

constexpr int32_t READ_PARCEL_MAX_WANT_INFO_SIZE = 1000;

bool WantSenderInfo::ReadFromParcel(Parcel &parcel)
{
    type = parcel.ReadInt32();
    bundleName = Str16ToStr8(parcel.ReadString16());
    resultWho = Str16ToStr8(parcel.ReadString16());
    requestCode = parcel.ReadInt32();
    int32_t wantsInfoSize = parcel.ReadInt32();
    if (wantsInfoSize > READ_PARCEL_MAX_WANT_INFO_SIZE) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "ReadFromParcel wantsInfoSize Control");
        return false;
    }
    for (int32_t i = 0; i < wantsInfoSize; i++) {
        std::unique_ptr<WantsInfo> wantsInfo(parcel.ReadParcelable<WantsInfo>());
        if (!wantsInfo) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "null wantsInfo");
            return false;
        }
        allWants.emplace_back(*wantsInfo);
    }
    flags = parcel.ReadUint32();
    userId = parcel.ReadInt32();
    return true;
}

WantSenderInfo *WantSenderInfo::Unmarshalling(Parcel &parcel)
{
    WantSenderInfo *info = new (std::nothrow) WantSenderInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool WantSenderInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(type);
    parcel.WriteString16(Str8ToStr16(bundleName));
    parcel.WriteString16(Str8ToStr16(resultWho));
    parcel.WriteInt32(requestCode);
    size_t wantsInfoSize = allWants.size();
    if (!parcel.WriteInt32(wantsInfoSize)) {
        return false;
    }
    for (size_t i = 0; i < wantsInfoSize; i++) {
        if (!parcel.WriteParcelable(&allWants[i])) {
            return false;
        }
    }
    parcel.WriteUint32(flags);
    parcel.WriteInt32(userId);
    return true;
}

bool WantTriggerInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<WantSenderInfo> SenderInforRequest(parcel.ReadParcelable<WantSenderInfo>());
    if (SenderInforRequest == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null SenderInforRequest");
        return false;
    }
    wantSenderInfo = *SenderInforRequest;
    TAG_LOGI(AAFwkTag::WANTAGENT, "ljj:ReadFromParcel type=%{public}d, %{public}s", wantSenderInfo.type, wantSenderInfo.bundleName.c_str());

    if (parcel.ReadBool()) {
        callerToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
        if (callerToken == nullptr) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "callerToken read failed");
            return false;
        }
    }

    callerTokenID = parcel.ReadInt32();
    appUid = parcel.ReadInt32();
    callerUid = parcel.ReadInt32();

    return true;
}

WantTriggerInfo *WantTriggerInfo::Unmarshalling(Parcel &parcel)
{
    WantTriggerInfo *info = new (std::nothrow) WantTriggerInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool WantTriggerInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&wantSenderInfo)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write wantSenderInfo");
        return false;
    }

    if (callerToken) {
        if (!parcel.WriteBool(true)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "true write failed");
            return false;
        }
        if (!(static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerToken)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "callerToken write failed");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::WANTAGENT, "false write failed");
            return false;
        }
    }

    parcel.WriteInt32(callerTokenID);
    parcel.WriteInt32(appUid);
    parcel.WriteInt32(callerUid);

    return true;
}

}  // namespace AAFwk
}  // namespace OHOS
