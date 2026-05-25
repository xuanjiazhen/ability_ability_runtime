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

#ifndef OHOS_CLI_TOOL_DSTB_CALLER_INFO_H
#define OHOS_CLI_TOOL_DSTB_CALLER_INFO_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace CliTool {

// 远端调用者信息
struct DstbCallerInfo : public Parcelable {
    std::string sourceDeviceId;  // 来源设备网络ID
    int32_t uid = -1;            // 调用方UID
    int32_t pid = -1;            // 调用方PID
    std::string bundleName;      // 调用方包名
    uint32_t accessToken = 0;    // 调用方访问令牌

    DstbCallerInfo() = default;
    DstbCallerInfo(const std::string& deviceId, int32_t u, int32_t p, const std::string& bundle, uint32_t token)
        : sourceDeviceId(deviceId), uid(u), pid(p), bundleName(bundle), accessToken(token) {}

    bool Marshalling(Parcel& parcel) const override
    {
        return parcel.WriteString(sourceDeviceId) &&
               parcel.WriteInt32(uid) &&
               parcel.WriteInt32(pid) &&
               parcel.WriteString(bundleName) &&
               parcel.WriteUint32(accessToken);
    }

    static DstbCallerInfo* Unmarshalling(Parcel& parcel)
    {
        DstbCallerInfo* info = new DstbCallerInfo();
        if (!parcel.ReadString(info->sourceDeviceId) ||
            !parcel.ReadInt32(info->uid) ||
            !parcel.ReadInt32(info->pid) ||
            !parcel.ReadString(info->bundleName) ||
            !parcel.ReadUint32(info->accessToken)) {
            delete info;
            return nullptr;
        }
        return info;
    }
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_CALLER_INFO_H
