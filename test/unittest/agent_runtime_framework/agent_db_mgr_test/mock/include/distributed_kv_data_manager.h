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

#ifndef OHOS_AGENT_RUNTIME_PARCEL_MOCK_H
#define OHOS_AGENT_RUNTIME_PARCEL_MOCK_H

#include <string>
#include <vector>

namespace OHOS {
enum Status : int32_t {
    /**
     * Result(2228224)
     */
    SUCCESS = NO_ERROR,
    DATAOBS_SERVICE_NOT_CONNECTED = AAFWK_DATAOBS_SERVICE_ERR_OFFSET,
    GET_DATAOBS_SERVICE_FAILED,
    DATAOBS_PROXY_INNER_ERR,
    DATA_OBSERVER_IS_NULL,
    DATAOBS_SERVICE_HANDLER_IS_NULL,
    DATAOBS_SERVICE_INNER_IS_NULL,
    DATAOBS_SERVICE_TASK_LIMMIT,
    DATAOBS_SERVICE_OBS_LIMMIT,
    DATAOBS_SERVICE_POST_TASK_FAILED,
    NO_OBS_FOR_URI,
    OBS_EXIST,
    DATAOBS_HIDUMP_ERROR,
    IPC_PARCEL_ERROR,
    IPC_ERROR,
    INVALID_PARAM,
    GET_TOKENINFO_ERR,
    DATAOBS_INVALID_USERID,
    DATAOBS_NOT_SYSTEM_APP,
    BMS_NOT_READY,
    COMMON_ERROR,
    URI_NOT_EXIST,
    DATAOBS_PERMISSION_DENY,
    DATAOBS_INVALID_URI,
    DATAOBS_RDB_INVALID_URI,
    DATAOBS_PREFERENCE_INVALID_URI,
};
class Parcel {
public:
    bool WriteUint32(uint32_t value);
    bool WriteString(const std::string& value);
    bool WriteParcelable(const Parcelable* parcelable);
    uint32_t ReadUint32();
    const std::string ReadString();

    template <typename T>
    T* ReadParcelable() const
    {
        return nullptr;
    }

    // Additional methods needed for testing
    bool WriteBool(bool value);
    bool WriteStringVector(const std::vector<std::string>& value);
    bool ReadBool();
    bool ReadStringVector(std::vector<std::string>* value);
};
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_PARCEL_MOCK_H