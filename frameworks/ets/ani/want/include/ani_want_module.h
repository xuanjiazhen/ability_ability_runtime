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

#ifndef OHOS_ABILITY_RUNTIME_ANI_WANT_MODULE_H
#define OHOS_ABILITY_RUNTIME_ANI_WANT_MODULE_H

#include "ani.h"

namespace OHOS::AppExecFwk {
class EtsWantParams {
public:
    EtsWantParams() = default;
    ~EtsWantParams() = default;

    static ani_long NativeCreate(ani_env *env, ani_object);
    static void NativeDestroy(ani_env *env, ani_object, ani_long nativeWantParams);
    static ani_boolean NativeSetStringParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
        ani_string value);
    static ani_boolean NativeSetNumberParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
        ani_double value);
    static ani_boolean NativeSetIntParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
        ani_int value);
    static ani_boolean NativeSetWantParams(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
        ani_long value);
};
} // namespace OHOS::AppExecFwk
#endif // OHOS_ABILITY_RUNTIME_ANI_WANT_MODULE_H
