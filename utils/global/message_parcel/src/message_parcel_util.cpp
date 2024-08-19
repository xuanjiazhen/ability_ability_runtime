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

#include "message_parcel_util.h"

#include <memory>

namespace OHOS::AbilityRuntime {
namespace MessageParcelUtil {
MessageParcel MakeRawDataParcel()
{
    MessageParcel parcel;
    (void)parcel.SetMaxCapacity(RAW_DATA_EXPECTED_CAPACITY);
    return parcel;
}

bool WriteRawData(MessageParcel &dataParcel, MessageParcel &rawDataParcel)
{
    size_t size = rawDataParcel.GetDataSize();
    if (!dataParcel.WriteUint32(size)) {
        return false;
    }
    if (!dataParcel.WriteRawData(reinterpret_cast<uint8_t *>(rawDataParcel.GetData()), size)) {
        return false;
    }
    return true;
}

bool ReadRawData(MessageParcel &dataParcel, MessageParcel &rawDataParcel)
{
    size_t size = dataParcel.ReadUint32();
    if (size <= 0) {
        return false;
    }
    const void *rawData = dataParcel.ReadRawData(size);
    if (rawData == nullptr) {
        return false;
    }

    void *buffer = malloc(size);
    if (buffer == nullptr) {
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != 0) {
        free(buffer);
        return false;
    }

    if (!rawDataParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        return false;
    }
    return true;
}
}  // namespace MessageParcelUtil
}  // namespace OHOS::AbilityRuntime