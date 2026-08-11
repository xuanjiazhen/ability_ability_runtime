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

#ifndef ACE_NAPI_HOOK_INIT_H
#define ACE_NAPI_HOOK_INIT_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../cj_macro.h"
#include "module_registry_bridge.h"

namespace ace_napi_hook {

std::vector<std::pair<std::string, void*>> BuildAllDispatchMethods();
bool InstallAllNapiHooks();
bool InitAceNapiHook();
bool RefreshAceNapiHook(const char* so_path);
bool RefreshAceNapiHooks(const char* primary_path, const char* const* additional_paths,
                         size_t additional_count);
int32_t RequireSystemModule(const char* module_name,
    const char* const* additional_dep_libs, size_t additional_dep_count);
bool FindNativeModule(const char* module_name, NativeModule* result);

}  // namespace ace_napi_hook

extern "C" CJ_EXPORT void ACE_NAPI_HOOK_InitOrAbort();
extern "C" CJ_EXPORT void ACE_NAPI_HOOK_RefreshOrAbort(const char* so_path);
extern "C" CJ_EXPORT void ACE_NAPI_HOOK_RefreshManyOrAbort(const char* primary_path,
                                                  const char* const* additional_paths,
                                                  size_t additional_count);
extern "C" CJ_EXPORT int32_t ACE_NAPI_HOOK_RequireSystemModule(const char* module_name,
    const char* const* additional_dep_libs, size_t additional_dep_count);

#endif
