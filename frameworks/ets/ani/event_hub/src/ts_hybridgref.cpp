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

#include "ts_hybridgref.h"
 
#include "hilog_tag_wrapper.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include <ani.h>
 
namespace OHOS {
namespace AbilityRuntime {
 
static __hybridgref* g_jsToEtsEventHubOnRef = nullptr;
static __hybridgref* g_jsToEtsEventHubOffRef = nullptr;
static __hybridgref* g_jsToEtsEventHubEmitRef = nullptr;
 
static napi_value NativeSaveEventHubOnRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    if (argc < 1) {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        return undefined;
    }
 
    if (g_jsToEtsEventHubOnRef != nullptr) {
        hybridgref_delete_from_napi(env, g_jsToEtsEventHubOnRef);
        g_jsToEtsEventHubOnRef = nullptr;
    }
 
    bool ok = hybridgref_create_from_napi(env, argv[0], &g_jsToEtsEventHubOnRef);
    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}
 
static napi_value NativeSaveEventHubOffRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    if (argc < 1) {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        return undefined;
    }
 
    if (g_jsToEtsEventHubOffRef != nullptr) {
        hybridgref_delete_from_napi(env, g_jsToEtsEventHubOffRef);
        g_jsToEtsEventHubOffRef = nullptr;
    }
 
    bool ok = hybridgref_create_from_napi(env, argv[0], &g_jsToEtsEventHubOffRef);
    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}
 
static napi_value NativeSaveEventHubEmitRef(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];  // NOLINT(modernize-avoid-c-arrays)
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
 
    if (argc < 1) {
        napi_value undefined;
        napi_get_undefined(env, &undefined);
        return undefined;
    }
 
    if (g_jsToEtsEventHubEmitRef != nullptr) {
        hybridgref_delete_from_napi(env, g_jsToEtsEventHubEmitRef);
        g_jsToEtsEventHubEmitRef = nullptr;
    }
 
    bool ok = hybridgref_create_from_napi(env, argv[0], &g_jsToEtsEventHubEmitRef);
    napi_value result;
    napi_get_boolean(env, ok, &result);
    return result;
}
 
static ani_object NativeGetEventHubOnRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsEventHubOnRef, &result);
    return result;
}
 
static ani_object NativeGetEventHubOffRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsEventHubOffRef, &result);
    return result;
}
 
static ani_object NativeGetEventHubEmitRef(ani_env *env)
{
    ani_object result {};
    hybridgref_get_esvalue(env, g_jsToEtsEventHubEmitRef, &result);
    return result;
}
 
bool NativeGrefTsToEts::RegisterNativeSaveRef(napi_env env)
{
    napi_value global;
    if (napi_get_global(env, &global) != napi_ok) {
        return false;
    }
 
    napi_value nativeSaveEventHubOnFn;
    if (napi_create_function(env, "nativeSaveEventHubOnRef", NAPI_AUTO_LENGTH, NativeSaveEventHubOnRef, nullptr, &nativeSaveEventHubOnFn) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, global, "nativeSaveEventHubOnRef", nativeSaveEventHubOnFn) != napi_ok) {
        return false;
    }
 
    napi_value nativeSaveEventHubOffFn;
    if (napi_create_function(env, "nativeSaveEventHubOffRef", NAPI_AUTO_LENGTH, NativeSaveEventHubOffRef, nullptr, &nativeSaveEventHubOffFn) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, global, "nativeSaveEventHubOffRef", nativeSaveEventHubOffFn) != napi_ok) {
        return false;
    }
 
    napi_value nativeSaveEventHubEmitFn;
    if (napi_create_function(env, "nativeSaveEventHubEmitRef", NAPI_AUTO_LENGTH, NativeSaveEventHubEmitRef, nullptr, &nativeSaveEventHubEmitFn) != napi_ok) {
        return false;
    }
    if (napi_set_named_property(env, global, "nativeSaveEventHubEmitRef", nativeSaveEventHubEmitFn) != napi_ok) {
        return false;
    }
 
    return true;
}
 
bool NativeGrefTsToEts::RegisterETSGetter(ani_env *env)
{
    ani_module mod {};
    if (env->FindModule("Lets_functions;", &mod) != ANI_OK) {
        return false;
    }
 
    std::array methods = {
        ani_native_function {"nativeGetEventHubOnRef", nullptr, reinterpret_cast<void *>(NativeGetEventHubOnRef)},
        ani_native_function {"nativeGetEventHubOffRef", nullptr, reinterpret_cast<void *>(NativeGetEventHubOffRef)},
        ani_native_function {"nativeGetEventHubEmitRef", nullptr, reinterpret_cast<void *>(NativeGetEventHubEmitRef)},
    };
    return env->Module_BindNativeFunctions(mod, methods.data(), methods.size()) == ANI_OK;
}
 
} // namespace AbilityRuntime
} // namespace OHOS