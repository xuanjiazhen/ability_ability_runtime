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

#ifndef OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_READ_UTILS_H
#define OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_READ_UTILS_H

#include <memory>
#include "appexecfwk_errors.h"
#include "hilog_tag_wrapper.h"
#include "message_parcel.h"
#include "securec.h"

namespace OHOS::AbilityRuntime {
namespace MessageParcelUtils {
bool GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        return false;
    }
    if (size <= 0) {
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != 0) {
        free(buffer);
        return false;
    }
    return true;
}

int32_t ReadRawData(MessageParcel &data, MessageParcel &rawData)
{
    size_t dataSize = data.ReadUint32();
    TAG_LOGD(AAFwkTag::DEFAULT, "dataSize=%{public}zu", dataSize);
    void *buffer = nullptr;
    if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    MessageParcel rawData;
    if (!rawData.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ERR_OK;
}

} // namespace MessageParcelUtils
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_READ_UTILS_H