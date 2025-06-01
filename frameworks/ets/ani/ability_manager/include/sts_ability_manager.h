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

#ifndef OHOS_ABILITY_RUNTIME_STS_ABILITY_MANAGER_H
#define OHOS_ABILITY_RUNTIME_STS_ABILITY_MANAGER_H

#include "sts_runtime.h"

#include "ability_running_info.h"

namespace OHOS {
namespace AbilityManagerSts {
void StsAbilityManagerRegistryInit(ani_env *env);

class StsAbilityManager final {
public:
    static StsAbilityManager &GetInstance()
    {
        static StsAbilityManager instance;
        return instance;
    }
    static void GetAbilityRunningInfos(ani_env *env, ani_object callback);
    static void NotifyDebugAssertResult(
        ani_env *env, ani_string sessionIdObj, ani_object userStatusObj, ani_object callback);

private:
    static bool WrapAbilityRunningInfo(ani_env *env, ani_object &infoObj, const AAFwk::AbilityRunningInfo &info);
    static bool WrapAbilityRunningInfoArray(
        ani_env *env, ani_object &arrayObj, const std::vector<AAFwk::AbilityRunningInfo> &infos);
    static bool WrapAbilityRunningInfoInner(
        ani_env *env, ani_object &infoObj, const AAFwk::AbilityRunningInfo &info, ani_class cls);
};
} // namespace AbilityManagerSts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_ABILITY_MANAGER_H
