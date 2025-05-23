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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_WINDOW_OPTIONS_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_WINDOW_OPTIONS_UTILS_H

#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class WindowOptionsUtils final {
public:
    static void SetWindowPositionAndSize(Want& want,
        const sptr<IRemoteObject>& callerToken, const StartOptions& startOptions) {}
    static std::pair<bool, AppExecFwk::SupportWindowMode> WindowModeMap(int32_t windowMode)
    {
        return std::make_pair(false, AppExecFwk::SupportWindowMode::FULLSCREEN);
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
