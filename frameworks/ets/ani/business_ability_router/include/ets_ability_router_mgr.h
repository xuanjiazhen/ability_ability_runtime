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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_ROUTER_MGR_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_ROUTER_MGR_H

#include <memory>
#include "ani.h"
#include "service_info.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsAbilityRouterMgr : public std::enable_shared_from_this<EtsAbilityRouterMgr> {
public:
    EtsAbilityRouterMgr() = default;
    ~EtsAbilityRouterMgr() = default;
    static EtsAbilityRouterMgr& GetInstance();
    static void QueryBusinessAbilityInfos(ani_env *env, ani_object filterObj, ani_object callbackObj);
private:
    void OnQueryBusinessAbilityInfos(ani_env *env, ani_object filterObj, ani_object callbackObj);
    bool UnwrapBusinessAbilityFilter(ani_env *env, ani_object param, BusinessAbilityFilter &filter);
    ani_object ConvertBusinessAbilityInfos(ani_env *env, const std::vector<BusinessAbilityInfo> &infos);
    ani_object WrapBusinessAbilityInfo(ani_env *env, const BusinessAbilityInfo &info);
    bool SetBusinessAbilityInfo(ani_env *env, ani_class cls, ani_object object, const BusinessAbilityInfo &info);
    ani_object ConvertAppInfo(ani_env *env, const AppInfo &appInfo);
    bool SetApplicationInfo(ani_env *env, ani_object object, const AppInfo &appInfo);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ABILITY_ROUTER_MGR_H