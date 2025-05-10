/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "start_ability_with_wait_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool StartAbilityWithWaitData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt64(abilityManagerServiceTime)) {
        return false;
    }
    if (!parcel.WriteInt64(completeFirstFrameDrawingTime)) {
        return false;
    }
    if (!parcel.WriteBool(coldStart)) {
        return false;
    }
    if (!parcel.WriteBool(isCompleteFirstFrameFlag)) {
        return false;
    }
    return true;
}

bool StartAbilityWithWaitData::ReadFromParcel(Parcel &parcel)
{
    abilityManagerServiceTime = parcel.ReadInt64();
    completeFirstFrameDrawingTime = parcel.ReadInt64();
    coldStart = parcel.ReadBool();
    isCompleteFirstFrameFlag = parcel.ReadBool();
    return true;
}

StartAbilityWithWaitData *StartAbilityWithWaitData::Unmarshalling(Parcel &parcel)
{
    StartAbilityWithWaitData *startAbilityWithWaitData = new (std::nothrow) StartAbilityWithWaitData();
    if (startAbilityWithWaitData && !startAbilityWithWaitData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "ReadFromParcel failed");
        delete startAbilityWithWaitData;
        startAbilityWithWaitData = nullptr;
    }
    return startAbilityWithWaitData;
}
}  // namespace AppExecFwk
}  // namespace OHOS