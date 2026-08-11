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

#include "hook_backend.h"

#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <unordered_set>

#include <hilog/log.h>

#include "xhook_compat.h"

namespace ace_napi_hook {

namespace {

constexpr uint32_t HOOK_LOG_DOMAIN = 0xD001150;
constexpr const char* HOOK_LOG_TAG = "AbilityAceNapiHook";
constexpr const char* CJ_ENVIRONMENT_LIBRARY = "libcj_environment.z.so";
constexpr const char* REGISTER_HOOK_SYMBOL = "cjenv_register_hook";
constexpr const char* VALIDATE_HOOKS_SYMBOL = "cjenv_validate_hooks";
constexpr const char* REQUIRE_SYSTEM_MODULE_SYMBOL = "cjenv_require_system_module_with_deps";
constexpr const char* GET_MODULE_INFO_SYMBOL = "cjenv_get_module_info";

using RegisterNamespaceHookFn = void (*)(const char*, void*);
using ValidateNamespaceHooksFn = bool (*)(const char* const*, size_t);
using RequireSystemModuleFn = bool (*)(const char*, const char* const*, size_t);

struct CjenvModuleInfo {
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
    bool moduleLoaded;
    bool isAppModule;
};

using GetModuleInfoFn = bool (*)(const char*, CjenvModuleInfo*);

const char* Basename(const char* path)
{
    if (path == nullptr) {
        return nullptr;
    }
    const char* slash = strrchr(path, '/');
    return slash == nullptr ? path : slash + 1;
}

template<typename Function>
Function ResolveRequiredSymbol(void* handle, const char* symbolName)
{
    dlerror();
    auto function = reinterpret_cast<Function>(dlsym(handle, symbolName));
    const char* error = dlerror();
    if (function == nullptr || error != nullptr) {
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "namespace backend unavailable: %{public}s not found in %{public}s",
            symbolName, CJ_ENVIRONMENT_LIBRARY);
        return nullptr;
    }
    return function;
}

class NamespaceBackend final : public NapiHookBackend {
public:
    NamespaceBackend(void* handle, RegisterNamespaceHookFn registerHook,
        ValidateNamespaceHooksFn validateHooks,
        RequireSystemModuleFn requireSystemModule, GetModuleInfoFn getModuleInfo)
        : handle_(handle), registerHook_(registerHook), validateHooks_(validateHooks),
          requireSystemModule_(requireSystemModule), getModuleInfo_(getModuleInfo)
    {
    }

    const char* Name() const override
    {
        return "namespace";
    }

    bool Initialize(const DispatchMethods& methods) override
    {
        std::vector<const char*> symbolNames;
        symbolNames.reserve(methods.size());
        for (const auto& method : methods) {
            registerHook_(method.first.c_str(), method.second);
            symbolNames.emplace_back(method.first.c_str());
        }
        if (!validateHooks_(symbolNames.data(), symbolNames.size())) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "namespace backend rejected: hook validation failed count=%{public}zu",
                symbolNames.size());
            return false;
        }
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "namespace backend enabled: validated %{public}zu dispatch symbols",
            symbolNames.size());
        return true;
    }

    bool RefreshLibraries(const char* primaryPath,
        const char* const* additionalPaths, size_t additionalCount) override
    {
        (void)additionalPaths;
        (void)additionalCount;
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "namespace backend owns module relocations; xhook refresh skipped for %{public}s",
            primaryPath == nullptr ? "<null>" : primaryPath);
        return true;
    }

    int32_t RequireSystemModule(const char* moduleName,
        const char* const* additionalDepLibs, size_t additionalDepCount) override
    {
        if (moduleName == nullptr || moduleName[0] == '\0') {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "namespace module load failed: invalid module name");
            return -1;
        }
        if (additionalDepCount > 0 && additionalDepLibs == nullptr) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "namespace module load failed: invalid additional dependency array");
            return -1;
        }
        if (!requireSystemModule_(moduleName, additionalDepLibs, additionalDepCount)) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "namespace module load failed without xhook fallback: module=%{public}s dependencies=%{public}zu",
                moduleName, additionalDepCount);
            return -1;
        }
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "namespace module load succeeded: module=%{public}s dependencies=%{public}zu",
            moduleName, additionalDepCount);
        return 1;
    }

    bool FindModule(const char* moduleName, NativeModule* result) override
    {
        if (moduleName == nullptr || moduleName[0] == '\0' || result == nullptr) {
            return false;
        }
        CjenvModuleInfo info {};
        if (!getModuleInfo_(moduleName, &info)) {
            return false;
        }
        *result = NativeModule {};
        result->name = info.name;
        result->moduleName = info.moduleName;
        result->fileName = info.fileName;
        result->systemFilePath = info.systemFilePath;
        result->registerCallback = info.registerCallback;
        result->getABCCode = info.getABCCode;
        result->getJSCode = info.getJSCode;
        result->version = info.version;
        result->flags = info.flags;
        result->refCount = info.refCount;
        result->moduleLoaded = info.moduleLoaded;
        result->isAppModule = info.isAppModule;
        return true;
    }

    void CloseAfterFailedInitialization()
    {
        if (handle_ != nullptr) {
            dlclose(handle_);
            handle_ = nullptr;
        }
    }

private:
    // Kept open for the process lifetime after successful initialization:
    // ace_napi stores callbacks exported by cj_environment.
    void* handle_;
    RegisterNamespaceHookFn registerHook_;
    ValidateNamespaceHooksFn validateHooks_;
    RequireSystemModuleFn requireSystemModule_;
    GetModuleInfoFn getModuleInfo_;
};

class XHookBackend final : public NapiHookBackend {
public:
    const char* Name() const override
    {
        return "xhook";
    }

    bool Initialize(const DispatchMethods& methods) override
    {
        methods_ = &methods;
        xhook_enable_debug(0);
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "xhook compatibility backend enabled");
        return true;
    }

    bool RefreshLibraries(const char* primaryPath,
        const char* const* additionalPaths, size_t additionalCount) override
    {
        if (primaryPath == nullptr || primaryPath[0] == '\0' ||
            (additionalCount > 0 && additionalPaths == nullptr)) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "xhook refresh failed: invalid library arguments");
            return false;
        }

        std::vector<std::string> requestedSonames;
        std::unordered_set<std::string> seen;
        auto appendSoname = [&requestedSonames, &seen](const char* path) {
            const char* soname = Basename(path);
            if (soname == nullptr || soname[0] == '\0') {
                return false;
            }
            if (seen.emplace(soname).second) {
                requestedSonames.emplace_back(soname);
            }
            return true;
        };
        if (!appendSoname(primaryPath)) {
            return false;
        }
        for (size_t i = 0; i < additionalCount; ++i) {
            if (!appendSoname(additionalPaths[i])) {
                return false;
            }
        }

        std::lock_guard<std::mutex> lock(refreshMutex_);
        std::vector<std::string> sonamesToHook = hookedSonames_;
        for (const auto& soname : requestedSonames) {
            if (hookedSonameSet_.count(soname) == 0) {
                sonamesToHook.emplace_back(soname);
            }
        }

        xhook_clear();
        for (const auto& method : *methods_) {
            int status = xhook_register(method.first.c_str(), method.second);
            if (status != 0) {
                HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                    "xhook registration failed: symbol=%{public}s status=%{public}d",
                    method.first.c_str(), status);
                return false;
            }
        }
        std::vector<const char*> sonamePointers;
        sonamePointers.reserve(sonamesToHook.size());
        for (const auto& soname : sonamesToHook) {
            sonamePointers.emplace_back(soname.c_str());
        }
        int refreshStatus = xhook_refresh_libraries(
            sonamePointers.data(), sonamePointers.size(), 0);
        if (refreshStatus != 0) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "xhook refresh failed: library_count=%{public}zu status=%{public}d",
                sonamePointers.size(), refreshStatus);
            return false;
        }
        for (const auto& soname : requestedSonames) {
            if (hookedSonameSet_.emplace(soname).second) {
                hookedSonames_.emplace_back(soname);
            }
        }
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "xhook refresh succeeded: hooked %{public}zu libraries",
            hookedSonames_.size());
        return true;
    }

    int32_t RequireSystemModule(const char* moduleName,
        const char* const* additionalDepLibs, size_t additionalDepCount) override
    {
        (void)moduleName;
        (void)additionalDepLibs;
        (void)additionalDepCount;
        return 0;
    }

    bool FindModule(const char* moduleName, NativeModule* result) override
    {
        return FindPlatformNativeModule(moduleName, result);
    }

private:
    const DispatchMethods* methods_ {nullptr};
    std::vector<std::string> hookedSonames_;
    std::unordered_set<std::string> hookedSonameSet_;
    std::mutex refreshMutex_;
};

} // namespace

std::unique_ptr<NapiHookBackend> TryCreateNamespaceBackend(
    const DispatchMethods& methods)
{
    dlerror();
    void* handle = dlopen(CJ_ENVIRONMENT_LIBRARY, RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        const char* error = dlerror();
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "namespace backend unavailable: dlopen %{public}s failed: %{public}s",
            CJ_ENVIRONMENT_LIBRARY, error == nullptr ? "unknown error" : error);
        return nullptr;
    }

    auto registerHook = ResolveRequiredSymbol<RegisterNamespaceHookFn>(
        handle, REGISTER_HOOK_SYMBOL);
    auto validateHooks = ResolveRequiredSymbol<ValidateNamespaceHooksFn>(
        handle, VALIDATE_HOOKS_SYMBOL);
    auto requireSystemModule = ResolveRequiredSymbol<RequireSystemModuleFn>(
        handle, REQUIRE_SYSTEM_MODULE_SYMBOL);
    auto getModuleInfo = ResolveRequiredSymbol<GetModuleInfoFn>(
        handle, GET_MODULE_INFO_SYMBOL);
    if (registerHook == nullptr || validateHooks == nullptr ||
        requireSystemModule == nullptr || getModuleInfo == nullptr) {
        dlclose(handle);
        return nullptr;
    }

    auto backend = std::make_unique<NamespaceBackend>(
        handle, registerHook, validateHooks, requireSystemModule, getModuleInfo);
    if (!backend->Initialize(methods)) {
        backend->CloseAfterFailedInitialization();
        return nullptr;
    }
    return backend;
}

std::unique_ptr<NapiHookBackend> CreateXHookBackend(
    const DispatchMethods& methods)
{
    auto backend = std::make_unique<XHookBackend>();
    if (!backend->Initialize(methods)) {
        return nullptr;
    }
    return backend;
}

} // namespace ace_napi_hook
