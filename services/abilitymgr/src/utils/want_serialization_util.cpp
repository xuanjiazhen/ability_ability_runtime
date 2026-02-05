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

#include "utils/want_serialization_util.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

bool WantSerializationUtil::IsDMSMode(const Want& want)
{
    // Check if deviceId is not empty (DMS mode)
    return !want.GetElement().GetDeviceID().empty();
}

void WantSerializationUtil::SetIsDMSFlag(Want& want)
{
    bool isDMS = IsDMSMode(want);
    // Set the IsDMS flag as an integer parameter (1 for true, 0 for false)
    want.SetParam(WANT_PARAM_IS_DMS_KEY, isDMS ? 1 : 0);
}

bool WantSerializationUtil::GetIsDMSFlag(const Want& want)
{
    // Get the IsDMS flag, defaulting to 0 (false) if not set
    int32_t flag = want.GetIntParam(WANT_PARAM_IS_DMS_KEY, 0);
    return flag != 0;
}

bool WantSerializationUtil::WriteStringByMode(Parcel& parcel, const std::string& str, bool isDMS)
{
    if (isDMS) {
        // DMS mode: use String16 encoding
        return parcel.WriteString16(Str8ToStr16(str));
    } else {
        // Non-DMS mode: use String encoding
        return parcel.WriteString(str);
    }
}

bool WantSerializationUtil::ReadStringByMode(Parcel& parcel, std::string& str, bool isDMS)
{
    if (isDMS) {
        // DMS mode: use String16 encoding
        str = Str16ToStr8(parcel.ReadString16());
        return true;
    } else {
        // Non-DMS mode: use String encoding
        return parcel.ReadString(str);
    }
}

} // namespace AAFwk
} // namespace OHOS
