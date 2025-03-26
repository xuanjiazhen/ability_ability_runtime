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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_IPC_UTIL_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_IPC_UTIL_H

#include <sys/types.h>

#include "hilog_tag_wrapper.h"
#include "parcel.h"
#include "securec.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128M
constexpr int32_t MAX_PARCEL_IPC_DATA_SIZE = 200 * 1024; // 200K

inline size_t GetPadSize(size_t size)
{
    const size_t offset = 3;
    return (((size + offset) & (~offset)) - size);
}

bool GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "null data");
        return false;
    }
    if (size == 0 || size > MAX_IPC_RAW_DATA_SIZE) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "size invalid: %{public}zu", size);
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::URIPERMMGR, "malloc buffer failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != EOK) {
        free(buffer);
        TAG_LOGE(AAFwkTag::URIPERMMGR, "memcpy failed");
        return false;
    }
    return true;
}
}

class UPMSIpcUtils {
public:
    static bool CheckUseRawData(const std::vector<std::string> &uriVec)
    {
        size_t oriSize = sizeof(int32_t);
        for (auto &uri : uriVec) {
            // calculate ipc data size of string uri, reference to parcel.h
            size_t desire = uri.length() + sizeof(char) + sizeof(int32_t);
            size_t padSize = GetPadSize(desire);
            oriSize += (desire + padSize);
            if (oriSize > MAX_PARCEL_IPC_DATA_SIZE) {
                TAG_LOGI(AAFwkTag::URIPERMMGR, "use raw data %{public}d", static_cast<int32_t>(oriSize));
                return true;
            }
        }
        return false;
    }

    static bool WriteStringUriByRawData(MessageParcel &data, const std::vector<std::string> &uriVec)
    {
        MessageParcel tempParcel;
        tempParcel.SetMaxCapacity(MAX_IPC_RAW_DATA_SIZE);
        if (!tempParcel.WriteStringVector(uriVec)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write uris failed");
            return false;
        }
        size_t dataSize = tempParcel.GetDataSize();
        if (!data.WriteUint32(static_cast<uint32_t>(dataSize))) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write data size failed");
            return false;
        }
        if (!data.WriteRawData(reinterpret_cast<uint8_t *>(tempParcel.GetData()), dataSize)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write raw data failed");
            return false;
        }
        return true;
    }

    static bool ReadStringUrisByRawData(MessageParcel &data, std::vector<std::string> &uriVec)
    {
        size_t dataSize = static_cast<size_t>(data.ReadUint32());
        if (dataSize == 0) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "parcel no data");
            return false;
        }

        void *buffer = nullptr;
        if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "read raw data failed: %{public}zu", dataSize);
            return false;
        }
        
        MessageParcel tempParcel;
        if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "failed to parseFrom");
            return false;
        }
        tempParcel.ReadStringVector(&uriVec);
        return true;
    }

    static bool WriteBatchResultByRawData(MessageParcel &data, const std::vector<bool> &result)
    {
        MessageParcel tempParcel;
        tempParcel.SetMaxCapacity(MAX_IPC_RAW_DATA_SIZE);
        if (!tempParcel.WriteBoolVector(result)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write result failed");
            return false;
        }
        size_t dataSize = tempParcel.GetDataSize();
        if (!data.WriteUint32(static_cast<uint32_t>(dataSize))) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write data size failed");
            return false;
        }
        if (!data.WriteRawData(reinterpret_cast<uint8_t *>(tempParcel.GetData()), dataSize)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "Write raw data failed");
            return false;
        }
        return true;
    }

    static bool ReadBatchResultByRawData(MessageParcel &data, std::vector<bool> &result)
    {
        size_t dataSize = static_cast<size_t>(data.ReadUint32());
        if (dataSize == 0) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "parcel no data");
            return false;
        }

        void *buffer = nullptr;
        if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "read raw data failed: %{public}zu", dataSize);
            return false;
        }

        MessageParcel tempParcel;
        if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
            TAG_LOGE(AAFwkTag::URIPERMMGR, "failed to parseFrom");
            return false;
        }
        tempParcel.ReadBoolVector(&result);
        return true;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_URI_PERMISSION_IPC_UTIL_H