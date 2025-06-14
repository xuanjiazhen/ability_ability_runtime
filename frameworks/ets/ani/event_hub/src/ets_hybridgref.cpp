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

#include "ets_hybridgref.h"
 
#include "hilog_tag_wrapper.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include <ani.h>
 
namespace OHOS {
namespace AbilityRuntime {
 
static __hybridgref* g_ref = nullptr;
static napi_value NativeGetRef(napi_env env, [[maybe_unused]] napi_callback_info info)
{
    napi_value result {};
    if (!hybridgref_get_napi_value(env, g_ref, &result)) {
        napi_get_undefined(env, &result);
    }
    return result;
}
 
static void NativeSaveHybridGref(ani_env *env, ani_object ref)
{
    if (!hybridgref_create_from_ani(env, static_cast<ani_ref>(ref), &g_ref)) {
        return;
    }
}
 
bool HybridGrefPrimitiveEtsToTs::RegisterNativeGetRef(napi_env env)
{
    napi_value global;
    if (napi_get_global(env, &global) != napi_ok) {
        return false;
    }
    napi_value fn;
    if (napi_create_function(env, "nativeGetRef", NAPI_AUTO_LENGTH, NativeGetRef, nullptr, &fn) != napi_ok) {
        return false;
    }
 
    if (napi_set_named_property(env, global, "nativeGetRef", fn) != napi_ok) {
        return false;
    }
 
    return true;
}
 
bool HybridGrefPrimitiveEtsToTs::RegisterNativeSaveRef(ani_env *env)
{
    ani_module etsMod {};
    auto status = env->FindModule("Lets_functions;", &etsMod);
    if (status != ANI_OK) {
        return false;
    }
    std::array methods = {
        ani_native_function {"saveHybridGref", nullptr, reinterpret_cast<void *>(NativeSaveHybridGref)},
    };
    status = env->Module_BindNativeFunctions(etsMod, methods.data(), methods.size());
    return status == ANI_OK;
}
 
} // namespace AbilityRuntime
} // namespace OHOS