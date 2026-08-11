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

#ifndef ACE_NAPI_HOOK_BACKEND_H
#define ACE_NAPI_HOOK_BACKEND_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "module_registry_bridge.h"

namespace ace_napi_hook {

using DispatchMethods = std::vector<std::pair<std::string, void*>>;

class NapiHookBackend {
public:
    virtual ~NapiHookBackend() = default;

    virtual const char* Name() const = 0;
    virtual bool Initialize(const DispatchMethods& methods) = 0;
    virtual bool RefreshLibraries(const char* primaryPath,
        const char* const* additionalPaths, size_t additionalCount) = 0;
    virtual int32_t RequireSystemModule(const char* moduleName,
        const char* const* additionalDepLibs, size_t additionalDepCount) = 0;
    virtual bool FindModule(const char* moduleName, NativeModule* result) = 0;
};

std::unique_ptr<NapiHookBackend> TryCreateNamespaceBackend(
    const DispatchMethods& methods);
std::unique_ptr<NapiHookBackend> CreateXHookBackend(
    const DispatchMethods& methods);

} // namespace ace_napi_hook

#endif // ACE_NAPI_HOOK_BACKEND_H

