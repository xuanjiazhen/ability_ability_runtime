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
#include <algorithm>

#include "taihe/runtime.hpp"
#include "ani_error_utils.h"
#include "ability_manager_errors.h"

namespace ani_errorutils {
using namespace OHOS::AAFwk;

constexpr int32_t ERR_INVALID_VALUE = -1;

int32_t ErrorCodeReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return NO_ERROR;
        case CHECK_PERMISSION_FAILED:
            return PERMISSION_DENIED;
        case DMS_PERMISSION_DENIED:
            return PERMISSION_DENIED;
        case ERR_INVALID_VALUE:
            return PARAMETER_CHECK_FAILED;
        case INVALID_PARAMETERS_ERR:
            return PARAMETER_CHECK_FAILED;
        case REGISTER_REMOTE_MISSION_LISTENER_FAIL:
            return PARAMETER_CHECK_FAILED;
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return NO_MISSION_INFO_FOR_MISSION_ID;
        case CONTINUE_REMOTE_UNINSTALLED_UNSUPPORT_FREEINSTALL:
            return REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE;
        case CONTINUE_REMOTE_UNINSTALLED_SUPPORT_FREEINSTALL:
            return CONTINUE_WITHOUT_FREEINSTALL_FLAG;
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET;
        case CONTINUE_ALREADY_IN_PROGRESS:
            return CONTINUE_ALREADY_IN_PROGRESS;
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return MISSION_FOR_CONTINUING_IS_NOT_ALIVE;
        case ERR_NOT_SYSTEM_APP:
            return NOT_SYSTEM_APP;
        default:
            return SYSTEM_WORK_ABNORMALLY;
    };
}

std::string ErrorMessageReturn(int32_t code)
{
    switch (code) {
        case NO_ERROR:
            return std::string();
        case PERMISSION_DENIED:
            return std::string("permission denied");
        case PARAMETER_CHECK_FAILED:
            return std::string("parameter check failed.");
        case SYSTEM_WORK_ABNORMALLY:
            return std::string("the system ability work abnormally.");
        case NO_MISSION_INFO_FOR_MISSION_ID:
            return std::string("failed to get the missionInfo of the specified missionId.");
        case REMOTE_UNINSTALLED_AND_UNSUPPORT_FREEINSTALL_FOR_CONTINUE:
            return std::string("the application is not installed on the "
                "remote end and installation-free is not supported.");
        case CONTINUE_WITHOUT_FREEINSTALL_FLAG:
            return std::string("The application is not installed on the remote end and "
                "installation-free is supported. Try again with the freeInstall flag.");
        case OPERATION_DEVICE_NOT_INITIATOR_OR_TARGET:
            return std::string("The operation device must be the device where the "
                "application to be continued is currently located or the target device.");
        case ERR_CONTINUE_ALREADY_IN_PROGRESS:
        case CONTINUE_ALREADY_IN_PROGRESS:
            return std::string("the local continuation task is already in progress.");
        case MISSION_FOR_CONTINUING_IS_NOT_ALIVE:
            return std::string("the mission for continuing is not alive, "
                "try again after restart this mission.");
        case ERR_GET_MISSION_INFO_OF_BUNDLE_NAME:
            return std::string("Failed to get the missionInfo of the specified bundle name.");
        case ERR_BIND_REMOTE_HOTSPOT_ENABLE_STATE:
            return std::string("bind error due to the remote device hotspot enable, try again after disable "
                "the remote device hotspot.");
        case ERR_BIND_REMOTE_IN_BUSY_LINK:
            return std::string("the remote device has been linked with other devices, try again when "
                "the remote device is idle.");
        case NOT_SYSTEM_APP:
            return std::string("The app is not system-app.");
        default:
            return std::string("the system ability work abnormally.");
    };
}

void ThrowError(const char* message)
{
    if (message == nullptr) {
        return;
    }
    std::string errMsg(message);
    taihe::set_error(errMsg);
}

void ThrowError(int32_t code, const char* message)
{
    if (message == nullptr) {
        return;
    }
    std::string errMsg(message);
    taihe::set_business_error(code, errMsg);
}
} // namespace OHOS::DistributedKVStore