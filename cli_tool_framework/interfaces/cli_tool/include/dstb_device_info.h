/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLI_TOOL_DSTB_DEVICE_INFO_H
#define OHOS_CLI_TOOL_DSTB_DEVICE_INFO_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace CliTool {

// 分布式设备信息
struct DstbDeviceInfo : public Parcelable {
    std::string deviceId;      // 设备网络ID（唯一标识）
    std::string deviceName;    // 设备显示名称
    int32_t deviceType = 0;    // 设备类型
    int32_t deviceState = 0;   // 设备状态

    DstbDeviceInfo() = default;
    DstbDeviceInfo(const std::string& id, const std::string& name, int32_t type, int32_t state)
        : deviceId(id), deviceName(name), deviceType(type), deviceState(state) {}

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteString(deviceId) &&
               parcel.WriteString(deviceName) &&
               parcel.WriteInt32(deviceType) &&
               parcel.WriteInt32(deviceState);
    }

    static DstbDeviceInfo* Unmarshalling(Parcel& parcel)
    {
        DstbDeviceInfo* info = new DstbDeviceInfo();
        if (!parcel.ReadString(info->deviceId) ||
            !parcel.ReadString(info->deviceName) ||
            !parcel.ReadInt32(info->deviceType) ||
            !parcel.ReadInt32(info->deviceState)) {
            delete info;
            return nullptr;
        }
        return info;
    }
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_DEVICE_INFO_H
