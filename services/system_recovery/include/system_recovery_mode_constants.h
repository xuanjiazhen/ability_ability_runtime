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

// 系统恢复模式只由启动参数决定，具体运行期策略由本目录中的框架统一管理。
inline constexpr char SYSTEM_RECOVERY_MODE_PARAMETER[] = "persist.start.mode";
inline constexpr char SYSTEM_RECOVERY_MODE_VALUE[] = "system_recovery";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_CONFIG[] =
    "etc/ability_runtime/system_recovery_app_whitelist.json";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_DEFAULT_CONFIG[] =
    "/system/etc/ability_runtime/system_recovery_app_whitelist.json";
inline constexpr char SYSTEM_RECOVERY_APP_WHITELIST_KEY[] = "bundleNames";

// 恢复模式水印由系统服务设置，普通应用不能通过窗口接口清除。
inline constexpr char SYSTEM_RECOVERY_WATERMARK_IMAGE[] =
    "/system/etc/ability_runtime/system_recovery_watermark.svg";
inline constexpr uint32_t SYSTEM_RECOVERY_WATERMARK_PRIORITY = 0;

// Backup SA 在系统服务配置中的名称，用于通过 BMS 的 UID 转换接口识别调用者。
inline constexpr char SYSTEM_RECOVERY_BACKUP_SA_NAME[] = "backup_sa";

// AMS 认证后的单次 BackupExtension 进程启动上下文，仅在 foundation 内部的 Want 中传递。
// 该标记不是白名单，也不授予普通 Ability 启动权限。
inline constexpr char SYSTEM_RECOVERY_BACKUP_EXTENSION_START_CONTEXT[] =
    "ohos.system.recovery.params.backup_extension_start";

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONSTANTS_H
