/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H

#include <memory>

#include "iremote_object.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
struct RunningProcessInfo;
}
namespace AAFwk {
struct AbilityRequest;
class Want;
class AbilityPermissionUtil {
public:
    static AbilityPermissionUtil &GetInstance();

    // check caller is delegator
    bool IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo, const AbilityRequest &abilityRequest) const;

    // check dominate screen
    bool IsDominateScreen(const Want &want, bool isPendingWantCaller);

    /**
     * Check if Caller is allowed to start PageAbility(FA) or Ability(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability.
     */
    int CheckCallAbilityPermission(const AbilityRequest &abilityRequest, uint32_t specifyTokenId = 0,
        bool isCallByShortcut = false);

    /**
     * Check if Caller is allowed to start Ability(Stage) by call.
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start Ability by call.
     */
    int CheckStartByCallPermission(const AbilityRequest &abilityRequest);

private:
    AbilityPermissionUtil() = default;
    ~AbilityPermissionUtil() = default;

    DISALLOW_COPY_AND_MOVE(AbilityPermissionUtil);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H