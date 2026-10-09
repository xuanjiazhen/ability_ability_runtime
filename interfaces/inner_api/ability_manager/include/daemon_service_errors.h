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

namespace OHOS {
namespace AAFwk {
constexpr int32_t ERR_DAEMON_ALREADY_REGISTERED = 2930001;
constexpr int32_t ERR_DAEMON_APPROVAL_REJECTED = 2930002;
constexpr int32_t ERR_DAEMON_NOT_REGISTERED = 2930003;
constexpr int32_t ERR_DAEMON_DISABLED = 2930004;
constexpr int32_t ERR_DAEMON_NOT_RUNNING = 2930005;
constexpr int32_t ERR_DAEMON_DEVICE_NOT_SUPPORTED = 2930006;
constexpr int32_t ERR_DAEMON_IPC_DEAD = 2930007;
constexpr int32_t ERR_DAEMON_STORAGE_FAILED = 2930009;

enum class DaemonStatus : int32_t {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2,
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_ERRORS_H
