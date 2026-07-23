/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "extension_module_loader.h"

#include <dlfcn.h>

#include "hilog_tag_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace {
constexpr char EXTENSION_MODULE_ENTRY[] = "OHOS_EXTENSION_GetExtensionModule";

using DynamicEntry = void* (*)();

class DummyExtensionModuleLoader final : public ExtensionModuleLoader, public Singleton<DummyExtensionModuleLoader> {
    DECLARE_SINGLETON(DummyExtensionModuleLoader);

public:
    Extension* Create(const std::unique_ptr<Runtime>& runtime) const override
    {
        return nullptr;
    }

    std::map<std::string, std::string> GetParams() override
    {
        TAG_LOGW(AAFwkTag::EXT, "DummyExtensionModuleLoader::GetParams start");
        std::map<std::string, std::string> params;
        TAG_LOGW(AAFwkTag::EXT, "DummyExtensionModuleLoader::GetParams end");
        return params;
    }
};

DummyExtensionModuleLoader::DummyExtensionModuleLoader() = default;
DummyExtensionModuleLoader::~DummyExtensionModuleLoader() = default;

ExtensionModuleLoader& GetExtensionModuleLoader(const char* sharedLibrary)
{
    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader start, lib:%{public}s", sharedLibrary);
    if (sharedLibrary == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "null sharedLibrary");
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader end (null sharedLibrary)");
        return DummyExtensionModuleLoader::GetInstance();
    }

    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlopen begin, lib:%{public}s", sharedLibrary);
    void* handle = dlopen(sharedLibrary, RTLD_LAZY);
    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlopen end, lib:%{public}s", sharedLibrary);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::EXT, "open extension library %{public}s failed, reason: %{public}sn", sharedLibrary,
            dlerror());
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader end (dlopen fail)");
        return DummyExtensionModuleLoader::GetInstance();
    }

    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlsym begin, lib:%{public}s", sharedLibrary);
    auto entry = reinterpret_cast<DynamicEntry>(dlsym(handle, EXTENSION_MODULE_ENTRY));
    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlsym end, lib:%{public}s", sharedLibrary);
    if (entry == nullptr) {
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlclose begin (dlsym fail)");
        dlclose(handle);
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlclose end (dlsym fail)");
        TAG_LOGE(AAFwkTag::EXT, "get extension symbol %{public}s in %{public}s failed", EXTENSION_MODULE_ENTRY,
            sharedLibrary);
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader end (dlsym fail)");
        return DummyExtensionModuleLoader::GetInstance();
    }

    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader entry begin, lib:%{public}s", sharedLibrary);
    auto loader = reinterpret_cast<ExtensionModuleLoader*>(entry());
    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader entry end, lib:%{public}s", sharedLibrary);
    if (loader == nullptr) {
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlclose begin (entry fail)");
        dlclose(handle);
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader dlclose end (entry fail)");
        TAG_LOGE(AAFwkTag::EXT, "get extension module loader in %{public}s failed", sharedLibrary);
        TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader end (entry fail)");
        return DummyExtensionModuleLoader::GetInstance();
    }

    TAG_LOGW(AAFwkTag::EXT, "GetExtensionModuleLoader end, lib:%{public}s", sharedLibrary);
    return *loader;
}
} // namespace

ExtensionModuleLoader& ExtensionModuleLoader::GetLoader(const char* sharedLibrary)
{
    TAG_LOGW(AAFwkTag::EXT, "ExtensionModuleLoader::GetLoader start, lib:%{public}s", sharedLibrary);
    auto& loader = GetExtensionModuleLoader(sharedLibrary);
    TAG_LOGW(AAFwkTag::EXT, "ExtensionModuleLoader::GetLoader end, lib:%{public}s", sharedLibrary);
    return loader;
}

Extension *ExtensionModuleLoader::Create(const std::unique_ptr<Runtime>& runtime) const
{
    return nullptr;
}
}
