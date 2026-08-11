/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ACE_NAPI_HOOK_MODULE_REGISTRY_BRIDGE_H
#define ACE_NAPI_HOOK_MODULE_REGISTRY_BRIDGE_H

#include "../cj_macro.h"
#include "napi_ohos_compat.h"

struct NativeModule {
    const char* name;
    const char* moduleName;
    const char* fileName;
    const char* systemFilePath;
    napi_value (*registerCallback)(napi_env, napi_value);
    void (*getABCCode)(const char** buf, int* bufLen);
    void (*getJSCode)(const char** buf, int* bufLen);
    int32_t version;
    uint32_t flags;
    uint32_t refCount;
    NativeModule* next;
    const char* jsCode;
    const uint8_t* jsABCCode;
    int32_t jsCodeLen;
    bool moduleLoaded;
    bool isAppModule;
};

namespace ace_napi_hook {

bool FindPlatformNativeModule(const char* name, NativeModule* result);

} // namespace ace_napi_hook

extern "C" CJ_EXPORT int ace_napi_hook_module_find(const char* name, NativeModule* result);

#endif
