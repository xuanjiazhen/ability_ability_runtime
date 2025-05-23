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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MODAL_SYSTEM_DIALOG_UTIL_H
#define MOCK_OHOS_ABILITY_RUNTIME_MODAL_SYSTEM_DIALOG_UTIL_H

#include "app_mgr_interface.h"

namespace OHOS {
namespace AbilityRuntime {
class ModalSystemDialogUtil final {
public:
    static bool CheckDebugAppNotInDeveloperMode(const AppExecFwk::ApplicationInfo &applicationInfo)
    {
        return false;
    }
    static void ShowDeveloperModeDialog(
        const std::string &bundleName, const std::string &moduleName, uint32_t labelId, int32_t userId) {}
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif
