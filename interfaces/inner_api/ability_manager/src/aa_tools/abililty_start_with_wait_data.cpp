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

#include "ability_start_with_wait_observer_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool AbilityStartWithWaitObserverData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(coldStart_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write coldStart error");
        return false;
    }
    if (!parcel.WriteUint32(reason_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write reason error");
        return false;
    }
    if (!parcel.WriteInt64(startTime_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write startTime error");
        return false;
    }
    if (!parcel.WriteInt64(foregroundTime_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write foregroundTime error");
        return false;
    }
    if (!parcel.WriteString(bundleName_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write bundleName error");
        return false;
    }
    if (!parcel.WriteString(abilityName_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Write abilityName error");
        return false;
    }
    return true;
}

bool AbilityStartWithWaitObserverData::ReadFromParcel(Parcel &parcel)
{
    coldStart_ = parcel.ReadBool();
    reason_ = parcel.ReadUint32();
    startTime_ = parcel.ReadInt64();
    foregroundTime_ = parcel.ReadInt64();
    bundleName_ = parcel.ReadString();
    abilityName_ = parcel.ReadString();
    return true;
}

AbilityStartWithWaitObserverData *AbilityStartWithWaitObserverData::Unmarshalling(Parcel &parcel)
{
    AbilityStartWithWaitObserverData *abilityStartWithWaitData = new (std::nothrow) AbilityStartWithWaitObserverData();
    if (abilityStartWithWaitData && !abilityStartWithWaitData->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ReadFromParcel failed");
        delete abilityStartWithWaitData;
        abilityStartWithWaitData = nullptr;
    }
    return abilityStartWithWaitData;
}
} // namespace AAFwk
} // namespace OHOS