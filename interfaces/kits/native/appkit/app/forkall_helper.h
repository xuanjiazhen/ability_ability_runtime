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

#ifndef OHOS_ABILITY_RUNTIME_FORKALL_HELPER_H
#define OHOS_ABILITY_RUNTIME_FORKALL_HELPER_H

#include "app_launch_data.h"

#ifdef ABILITY_COMMAND_FOR_TEST
#define FORKALL_HELPER_EXPORT __attribute__((visibility("default")))
#else
#define FORKALL_HELPER_EXPORT
#endif

namespace OHOS {
namespace AppExecFwk {
class FORKALL_HELPER_EXPORT ForkAllHelper {
public:
    static void HandleDebugAppLaunchDelay(const AppLaunchData &appLaunchData, bool isDeveloperMode);
};
} // namespace AppExecFwk
} // namespace OHOS

#undef FORKALL_HELPER_EXPORT

#endif // OHOS_ABILITY_RUNTIME_FORKALL_HELPER_H
