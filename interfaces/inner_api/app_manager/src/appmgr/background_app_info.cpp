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
    int32_t missionIdData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, missionIdData);
    missionId = static_cast<int32_t>(missionIdData);
    bandleName = Str16ToStr8(parcel.ReadString16());
    abilityName  = Str16ToStr8(parcel.ReadString16());
    moduleName  = Str16ToStr8(parcel.ReadString16());
    int32_t appIndexData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, appIndexData);
    appIndex = static_cast<int32_t>(appIndexData);
    int32_t windowTypeData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, windowTypeData);
    windowType = static_cast<int32_t>(windowTypeData);
    int32_t windowStateData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, windowStateData);
    windowState = static_cast<int32_t>(windowStateData);
    return true;
}

BackgroundAppInfo *BackgroundAppInfo::Unmarshalling(Parcel &parcel)
{
    BackgroundAppInfo *info = new (std::nothrow) BackgroundAppInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool BackgroundAppInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(missionId));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bandleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(abilityName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(moduleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(appIndex));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(windowType));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(windowState));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS