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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_PROCESS_INFO_H
#define OHOS_ABILITY_RUNTIME_DAEMON_PROCESS_INFO_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "daemon_service_info.h"

namespace OHOS {
namespace AppExecFwk {
constexpr uint32_t DAEMON_PROCESS_FLAG_START = 1U << 0;
constexpr uint32_t DAEMON_PROCESS_FLAG_KEEP_ALIVE = 1U << 1;
constexpr uint32_t DAEMON_PROCESS_FLAG_RUN_AT_LOAD = 1U << 2;

struct DaemonProcessInfo {
    std::string bundleName;
    std::string moduleName;
    std::string serviceName;
    int32_t uid = -1;
    int32_t userId = -1;
    int32_t appIndex = 0;
    int32_t pid = 0;
    int64_t startTime = 0;
    int32_t spawnResult = 0;
    uint32_t flags = 0;

    bool IsRunning() const
    {
        return pid > 0;
    }
};

inline std::string MakeDaemonProcessKey(const AAFwk::LaunchServiceDescriptor &descriptor)
{
    return descriptor.bundleName + "#" + descriptor.moduleName + "#" + descriptor.serviceName + "#" +
        std::to_string(descriptor.userId) + "#" + std::to_string(descriptor.appIndex);
}

using ProcessInfoMap = std::unordered_map<std::string, DaemonProcessInfo>;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DAEMON_PROCESS_INFO_H
