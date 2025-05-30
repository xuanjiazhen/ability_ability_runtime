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

#include "sts_query_erms_observer.h"
#include "sts_runtime.h"

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
    static void NotifyDebugAssertResult(ani_env *env, ani_string sessionIdObj, ani_object userStatusObj,
        ani_object callback);
    void QueryAtomicServiceStartupRule(ani_env *env, ani_object contextObj, ani_string aniAppId,
        ani_object callbackObj);
private:
    void QueryAtomicServiceStartupRuleInner(ani_env *env, sptr<IRemoteObject> token, std::string appId,
        ani_object callbackObj);
    int32_t AddQueryERMSObserver(ani_env *env, sptr<IRemoteObject> token, const std::string &appId,
        const std::string &startTime, ani_object callbackObj);
    sptr<AbilityRuntime::StsQueryERMSObserver> queryERMSObserver_ = nullptr;
};
} // namespace AbilityManagerSts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_ABILITY_MANAGER_H
