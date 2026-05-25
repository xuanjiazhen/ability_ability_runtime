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

#ifndef OHOS_CLI_TOOL_DSTB_TYPES_H
#define OHOS_CLI_TOOL_DSTB_TYPES_H

// 引用单独的头文件，避免重复定义
#include "dstb_caller_info.h"
#include "dstb_device_info.h"

namespace OHOS {
namespace CliTool {

// 设备状态枚举
enum class DstbDeviceState {
    UNKNOWN = 0,
    ONLINE = 1,
    OFFLINE = 2,
};

// 设备类型枚举
enum class DstbDeviceType {
    UNKNOWN = 0,
    PHONE = 1,
    TABLET = 2,
    WATCH = 3,
    TV = 4,
    CAR = 5,
};

// 分布式错误码
namespace DstbErrorCode {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_DEVICE_OFFLINE = 1001;
constexpr int32_t ERR_DEVICE_NOT_FOUND = 1002;
constexpr int32_t ERR_REMOTE_CALL_FAILED = 1003;
constexpr int32_t ERR_REMOTE_EXEC_DENIED = 1004;
constexpr int32_t ERR_REMOTE_SESSION_NOT_FOUND = 1005;
constexpr int32_t ERR_REMOTE_TIMEOUT = 1006;
constexpr int32_t ERR_DISTRIBUTED_PERMISSION_DENIED = 1007;
constexpr int32_t ERR_DATA_TRANSPORT_FAILED = 1008;
constexpr int32_t ERR_DEVICE_SELECTOR_CANCELED = 1009;
constexpr int32_t ERR_GET_REMOTE_PROXY_FAILED = 1010;
} // namespace DstbErrorCode

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_TYPES_H
