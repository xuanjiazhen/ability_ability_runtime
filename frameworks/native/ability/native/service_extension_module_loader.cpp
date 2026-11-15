/*
 * Author: Keyang Li <2024865311@qq.com>
 * Affiliation: Shandong University
 * Mentor: Hongjun Dai <dahogn@sdu.edu.cn>
 * Mentor: Hongbo Chen <hongbochen@sdu.edu.cn>
 */

/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>

#include "service_extension_module_loader.h"
#include "service_extension.h"

namespace OHOS::AbilityRuntime {
ServiceExtensionModuleLoader::ServiceExtensionModuleLoader() = default;
ServiceExtensionModuleLoader::~ServiceExtensionModuleLoader() = default;

Extension *ServiceExtensionModuleLoader::Create(const std::unique_ptr<Runtime>& runtime) const
{
    using Function = decltype(&ServiceExtension::Create);
    static Function entry = [] {
        void *handle = dlopen("libservice_extension.z.so", RTLD_NOW | RTLD_LOCAL);
        if (handle == nullptr) {
            std::fprintf(stderr, "Extension provider load failed: %s\n", dlerror());
            std::abort();
        }
        auto function = reinterpret_cast<Function>(dlsym(handle,
            "_ZN4OHOS14AbilityRuntime16ServiceExtension6CreateERKNSt3__h10unique_ptrINS0_7RuntimeENS2_1"
            "4default_deleteIS4_EEEE"));
        if (function == nullptr) {
            std::fprintf(stderr, "Extension factory lookup failed: %s\n", dlerror());
            std::abort();
        }
        return function;
    }();
    return entry(runtime);
}

std::map<std::string, std::string> ServiceExtensionModuleLoader::GetParams()
{
    std::map<std::string, std::string> params;
    // type means extension type in ExtensionAbilityType of extension_ability_info.h, 3 means service.
    params.insert(std::pair<std::string, std::string>("type", "3"));
    // extension name
    params.insert(std::pair<std::string, std::string>("name", "ServiceExtension"));
    return params;
}

extern "C" __attribute__((visibility("default"))) void* OHOS_EXTENSION_GetExtensionModule()
{
    return &ServiceExtensionModuleLoader::GetInstance();
}
} // namespace OHOS::AbilityRuntime
