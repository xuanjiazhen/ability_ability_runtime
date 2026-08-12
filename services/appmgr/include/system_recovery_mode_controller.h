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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_COMPAT_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_COMPAT_H

#include "../../system_recovery/include/system_recovery_mode_framework.h"

namespace OHOS {
namespace AppExecFwk {

// 兼容旧名称，新的实现统一使用 SystemRecoveryModeFramework。
using SystemRecoveryModeController = SystemRecoveryModeFramework;

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_COMPAT_H
