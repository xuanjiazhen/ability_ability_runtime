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

#include "foreground_app_connection_data.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool ForegroundAppConnectionData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(callerPid_)) {
        return false;
    }

    if (!parcel.WriteInt32(targetPid_)) {
        return false;
    }

    return true;
}

bool ForegroundAppConnectionData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(callerPid_)) {
        return false;
    }

    if (!parcel.ReadInt32(targetPid_)) {
        return false;
    }

    return true;
}

ForegroundAppConnectionData *ForegroundAppConnectionData::Unmarshalling(Parcel &parcel)
{
    ForegroundAppConnectionData *data = new ForegroundAppConnectionData();
    if (!data->ReadFromParcel(parcel)) {
        delete data;
        data = nullptr;
    }
    return data;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
