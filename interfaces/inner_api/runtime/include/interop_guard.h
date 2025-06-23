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

#ifndef OHOS_ABILITY_RUNTIME_INTEROP_GUARD_H
#define OHOS_ABILITY_RUNTIME_INTEROP_GUARD_H

#include <node_api.h>

#include "ani.h"

typedef struct __hybridgref *hybridgref;

namespace OHOS {
namespace AbilityRuntime {
class InteropGuard {
public:
    InteropGuard(ani_env *env, ani_ref ref);
    InteropGuard(napi_env env, napi_value value);
    ~InteropGuard();

    ani_object GetAniValue(ani_env *env);
    napi_value GetNapiValue(napi_env env);

private:
    ani_env *GetAniEnv();

    hybridgref ref_ = nullptr;
    ani_vm *vm_ = nullptr;
    napi_env env_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INTEROP_GUARD_H
