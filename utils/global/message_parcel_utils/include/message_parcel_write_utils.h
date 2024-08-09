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

#ifndef OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_WRITE_UTILS_H
#define OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_WRITE_UTILS_H

#include <memory>
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"

namespace OHOS::AbilityRuntime {
namespace MessageParcelUtils {
constexpr int32_t MAX_PARCEL_CAPACITY = 100 * 1024 * 1024; // 100M

template<typename T>
int32_t WriteParcelInfo(const T &parcelInfo, MessageParcel &data)
{
    MessageParcel tmpParcel;
    (void)tmpParcel.SetMaxCapacity(MAX_PARCEL_CAPACITY);
    if (!tmpParcel.WriteParcelable(&parcelInfo)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "write parcelInfo failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    size_t dataSize = tmpParcel.GetDataSize();
    if (!data.WriteUint32(dataSize)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "write dataSize failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteRawData(reinterpret_cast<uint8_t *>(tmpParcel.GetData()), dataSize)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "write tmpParcel failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ERR_OK;
}
} // namespace MessageParcelUtils
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_WRITE_UTILS_H