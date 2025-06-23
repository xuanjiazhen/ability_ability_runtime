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

#include "interop_guard.h"

#include "hilog_tag_wrapper.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"

namespace OHOS {
namespace AbilityRuntime {
InteropGuard::InteropGuard(ani_env *env, ani_ref ref)
{
    if (env == nullptr || ref == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid args");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetVM failed");
        return;
    }
    vm_ = aniVM;
    hybridgref_create_from_ani(env, ref, &ref_);
}


InteropGuard::InteropGuard(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid args");
        return;
    }
    env_ = env;
    hybridgref_create_from_napi(env, value, &ref_);
}

InteropGuard::~InteropGuard()
{
    if (ref_ == nullptr) {
        return;
    }
    if (vm_ != nullptr) {
        ani_env *env = GetAniEnv();
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
            return;
        }
        hybridgref_delete_from_ani(env, ref_);
    }
    if (env_ != nullptr) {
        hybridgref_delete_from_napi(env_, ref_);
    }
}

ani_env *InteropGuard::GetAniEnv()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null vm_");
        return nullptr;
    }
    ani_env* env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetEnv failed");
        return nullptr;
    }
    return env;
}

ani_object InteropGuard::GetAniValue(ani_env *env)
{
    ani_object result = nullptr;
    hybridgref_get_esvalue(env, ref_, &result);
    return result;
}

napi_value InteropGuard::GetNapiValue(napi_env env)
{
    napi_value result = nullptr;
    if (!hybridgref_get_napi_value(env, ref_, &result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get napi value");
        napi_get_undefined(env, &result);
    }
    return result;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
