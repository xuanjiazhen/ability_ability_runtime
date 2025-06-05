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

#ifndef OHOS_ABILITY_RUNTIME_ETS_RUNTIME_UTILS_H
#define OHOS_ABILITY_RUNTIME_ETS_RUNTIME_UTILS_H

#include "ani.h"

namespace OHOS {
namespace AbilityRuntime {
inline ani_object CreateEtsUndefined(ani_env *env)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_ref result = nullptr;
    env->GetUndefined(&result);
    return static_cast<ani_object>(result);
}

inline ani_ref CreateEtsUndefinedRef(ani_env *env)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_ref result = nullptr;
    env->GetUndefined(&result);
    return result;
}

inline ani_object CreateEtsNull(ani_env *env)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_ref result = nullptr;
    env->GetNull(&result);
    return static_cast<ani_object>(result);
}

inline ani_ref CreateEtsNullRef(ani_env *env)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_ref result = nullptr;
    env->GetNull(&result);
    return result;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_RUNTIME_UTILS_H
