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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_ERRORS_H
#define OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_ERRORS_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AAFwk {
// Internal daemon error codes. They are consumed by the service layer and must be
// translated into the public launch service error codes before leaving the bindings.
constexpr int32_t ERR_DAEMON_ALREADY_REGISTERED = 2930001;
constexpr int32_t ERR_DAEMON_APPROVAL_REJECTED = 2930002;
constexpr int32_t ERR_DAEMON_NOT_REGISTERED = 2930003;
constexpr int32_t ERR_DAEMON_DISABLED = 2930004;
constexpr int32_t ERR_DAEMON_NOT_RUNNING = 2930005;
constexpr int32_t ERR_DAEMON_DEVICE_NOT_SUPPORTED = 2930006;
constexpr int32_t ERR_DAEMON_IPC_DEAD = 2930007;
constexpr int32_t ERR_DAEMON_STORAGE_FAILED = 2930009;

// Public launch service error codes exposed through launchServiceManager APIs.
constexpr int32_t ERR_DAEMON_EXT_NOT_FOUND = 16000211;
constexpr int32_t ERR_DAEMON_EXT_ALREADY_REGISTERED = 16000212;
constexpr int32_t ERR_DAEMON_EXT_DISABLED = 16000213;
constexpr int32_t ERR_DAEMON_EXT_NOT_RUNNING = 16000214;
constexpr int32_t ERR_DAEMON_EXT_APPROVAL_REJECTED = 16000215;

enum class DaemonStatus : int32_t {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2,
};

struct DaemonInformation {
    std::string bundleName;
    std::string daemonName;
    DaemonStatus status = DaemonStatus::NOT_REGISTERED;
    int32_t pid = 0;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_ERRORS_H
