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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONSTANTS_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONSTANTS_H

#include <cstdint>

namespace OHOS {
namespace AAFwk {

inline constexpr char SYSTEM_RECOVERY_MODE_PARAMETER[] = "persist.start.mode";
inline constexpr char SYSTEM_RECOVERY_MODE_VALUE[] = "system_recovery";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_CONFIG[] =
    "etc/ability_runtime/system_recovery_app_whitelist.json";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_DEFAULT_CONFIG[] =
    "/system/etc/ability_runtime/system_recovery_app_whitelist.json";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_KEY[] = "bundleNames";

// 与 app_file_service 中 BConstants::BACKUP_UID 保持一致，表示 Backup SA 的 IPC UID。
inline constexpr int32_t SYSTEM_RECOVERY_BACKUP_UID = 1089;

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONSTANTS_H
