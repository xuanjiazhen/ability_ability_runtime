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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_SETTINGS_DATA_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_SETTINGS_DATA_H

#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * 系统恢复模式访问 SettingsData 的最小适配器。
 *
 * 恢复框架只读取飞行模式进入前的用户状态，用于退出恢复模式时恢复该状态。
 * 读取直接复用 SettingsData 的 DataShare IPC，不为此扩展 NetConnService 接口；
 * 飞行模式的实际开关仍由既有 NetConnClient::SetAirplaneMode() 负责。
 */
class SystemRecoverySettingsData final {
public:
    static bool GetAirplaneMode(bool &enabled);

private:
    SystemRecoverySettingsData() = delete;
    ~SystemRecoverySettingsData() = delete;
    DISALLOW_COPY_AND_MOVE(SystemRecoverySettingsData);
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_SETTINGS_DATA_H
