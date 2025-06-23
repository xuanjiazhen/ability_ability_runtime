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

#include "event_hub.h"
 
#include "ts_hybridgref.h"
#include "ets_hybridgref.h"
#include <napi/native_api.h>
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
 
namespace OHOS {
namespace AbilityRuntime {
 
void InitAni(ani_env *aniEnv) {
    if (!NativeGrefTsToEts::RegisterETSGetter(aniEnv)) {
        TAG_LOGI(AAFwkTag::APPKIT, "Failed to register ETS getter functions");
    }
    if (aniEnv) {
        if (!HybridGrefPrimitiveEtsToTs::RegisterNativeSaveRef(aniEnv)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to register ETS save functions");
        }
    }
}
 
napi_value InitEventHubNapi(napi_env env, napi_value exportObj)
{
    if (!NativeGrefTsToEts::RegisterNativeSaveRef(env)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to register NativeSaveRef functions");
    }
    if (!HybridGrefPrimitiveEtsToTs::RegisterNativeGetRef(env)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to register JS getter functions");
    }
 
    return CreateJsUndefined(env);
}
 
extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::APPKIT, "in EventHub ETS.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null vm or result");
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed, status: %{public}d", status);
        return ANI_NOT_FOUND;
    }
    InitAni(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::APPKIT, "EventHub.ANI_Constructor finished");
    return ANI_OK;
}
}
 
} // namespace AbilityRuntime
} // namespace OHOS