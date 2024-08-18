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
        TAG_LOGE(AAFwkTag::DEFAULT, "data is nullptr");
        return false;
    }
    if (size <= 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "size is invalid");
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "malloc failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != 0) {
        free(buffer);
        TAG_LOGE(AAFwkTag::DEFAULT, "memcpy_s failed");
        return false;
    }
    return true;
}

template<typename T>
int32_t ReadParcelInfo(MessageParcel &data, T &parcelInfo)
{
    size_t dataSize = data.ReadUint32();
    TAG_LOGD(AAFwkTag::DEFAULT, "dataSize=%{public}zu", dataSize);
    void *buffer = nullptr;
    if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
        TAG_LOGE(AAFwkTag::DEFAULT, "GetData failed dataSize : %{public}zu", dataSize);
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel tmpParcel;
    if (!tmpParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "ParseFrom failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    std::unique_ptr<T> info(tmpParcel.ReadParcelable<T>());
    if (info == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "ReadParcelable failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    parcelInfo = *info;
    return ERR_OK;
}
} // namespace MessageParcelUtils
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_MESSAGE_PARCEL_READ_UTILS_H