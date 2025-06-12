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
#include "ets_hybridgref.h"
#include <napi/native_api.h>
#include "hilog_tag_wrapper.h"

using namespace AbilityRuntime;

static napi_value Init(napi_env env, napi_value exports) {

    if (!RegisterNativeSaveRef(env)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to register NativeSaveRef functions");
    }
    
    // 注册 ETS 到 TS 的功能
    ani_env* aniEnv = nullptr;
    if (NativeGrefTsToEts::GetAniEnv(&aniEnv)) {
        if (!RegisterETSGetter(aniEnv)) {
            TAG_LOGI(AAFwkTag::APPKIT\, ("Failed to register ETS getter functions");
        }
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get ANI environment");
    }
    
    // 注册 ETS 侧功能
    if (aniEnv) {
        if (!HybridGrefPrimitiveEtsToTs::RegisterNativeSaveRef(aniEnv)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to register ETS save functions");
        }
    }
    
    // 注册 JS 侧功能
    if (!HybridGrefPrimitiveEtsToTs::RegisterNativeGetRef(env)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to register JS getter functions");
    }
    
    return exports;
}

// 模块注册
EXTERN_C_START
static napi_module eventhub_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "eventhub",
    .nm_priv = nullptr,
    .reserved = {0},
};
EXTERN_C_END

extern "C" __attribute__((constructor)) void RegisterModule() {
    napi_module_register(&eventhub_module);
}