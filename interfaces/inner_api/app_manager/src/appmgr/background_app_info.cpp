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

#include "background_app_info.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {

bool BackgroundAppInfo::ReadFromParcel(Parcel &parcel)
{
    id = parcel.ReadInt32();
    runningState = parcel.ReadInt32();
    lockedState = parcel.ReadBool();
    continuable = parcel.ReadBool();
    time = Str16ToStr8(parcel.ReadString16());
    label = Str16ToStr8(parcel.ReadString16());
    iconPath = Str16ToStr8(parcel.ReadString16());
    std::unique_ptr<AAFwk::Want> parcelWant(parcel.ReadParcelable<AAFwk::Want>());
    if (parcelWant == nullptr) {
        return false;
    }
    want = *parcelWant;
    want.CloseAllFd();
    abilityState = parcel.ReadInt32();
    unclearable = parcel.ReadBool();
    continueState = static_cast<ContinueState>(parcel.ReadInt32());
    return true;
}

BackgroundAppInfo *BackgroundAppInfo::Unmarshalling(Parcel &parcel)
{
    BackgroundAppInfo *info = new BackgroundAppInfo();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool BackgroundAppInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(id)) {
        return false;
    }

    if (!parcel.WriteInt32(runningState)) {
        return false;
    }

    if (!parcel.WriteBool(lockedState)) {
        return false;
    }

    if (!parcel.WriteBool(continuable)) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(time))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(label))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(iconPath))) {
        return false;
    }

    // if (!parcel.WriteParcelable(&want)) {
    //     return false;
    // }

    if (!parcel.WriteInt32(abilityState)) {
        return false;
    }

    if (!parcel.WriteBool(unclearable)) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(continueState))) {
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS