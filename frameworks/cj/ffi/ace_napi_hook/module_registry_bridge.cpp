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

#include "module_registry_bridge.h"

#include <hilog/log.h>

#include <dlfcn.h>
#include <string.h>

#include <memory>
#include <map>
#include <mutex>

namespace ace_napi_hook {

namespace {

constexpr uint32_t HOOK_LOG_DOMAIN = 0xD001150;
constexpr const char* HOOK_LOG_TAG = "AbilityAceNapiHook";

using GetInstanceFn = void* (*)();

struct ApiAllowListChecker;
struct NativeModuleNode {
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
    NativeModuleNode* next;
    const char* jsCode;
    const uint8_t* jsABCCode;
    int32_t jsCodeLen;
    bool moduleLoaded;
    bool isAppModule;
    std::unique_ptr<ApiAllowListChecker> apiAllowListChecker;
};

typedef struct {
    char name[256];
} Dl_namespace;

struct NativeModuleManagerView {
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM) && !defined(__BIONIC__) && !defined(IOS_PLATFORM) && \
    !defined(LINUX_PLATFORM)
    char* sharedLibsSonames_ = nullptr;
    std::map<std::string, Dl_namespace> nsMap_;
#endif

    std::mutex nativeModuleListMutex_;
    NativeModuleNode* headNativeModule_;
    NativeModuleNode* tailNativeModule_;
};

GetInstanceFn g_get_instance = nullptr;
std::once_flag g_init_once;

bool StartsWithDefaultPrefix(const char* name)
{
    static constexpr const char* kPrefix = "default/";
    if (name == nullptr) {
        return false;
    }
    return strncmp(name, kPrefix, strlen(kPrefix)) == 0;
}

bool MatchName(const NativeModuleNode* n, const char* name)
{
    if (n == nullptr || name == nullptr || name[0] == '\0') {
        return false;
    }
    if (n->name != nullptr && strcmp(n->name, name) == 0) {
        return true;
    }
    if (n->moduleName != nullptr && strcmp(n->moduleName, name) == 0) {
        return true;
    }
    if (!StartsWithDefaultPrefix(name) && n->moduleName != nullptr) {
        static constexpr const char* kPrefix = "default/";
        if (strncmp(n->moduleName, kPrefix, strlen(kPrefix)) == 0 &&
            strcmp(n->moduleName + strlen(kPrefix), name) == 0) {
            return true;
        }
    }
    return false;
}

void InitGetInstance()
{
    std::call_once(g_init_once, []() {
        // Newer HarmonyOS versions do not expose this symbol through dlsym(nullptr).
        void* handle = dlopen("libace_napi.z.so", RTLD_NOW);
        if (handle == nullptr) {
            handle = dlopen("libace_napi.so", RTLD_NOW);
        }
        if (handle == nullptr) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "Failed to open the platform or desktop ace_napi library");
            abort();
        }
        // g++ Itanium ABI for: NativeModuleManager::GetInstance()
        // Keep strict lookup: if missing, bridge returns empty results.
        g_get_instance = reinterpret_cast<GetInstanceFn>(
            dlsym(handle, "_ZN19NativeModuleManager11GetInstanceEv"));
        if (g_get_instance == nullptr) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "Failed to resolve NativeModuleManager::GetInstance");
            abort();
        }
        dlclose(handle);
    });
}

NativeModuleManagerView* GetManager()
{
    InitGetInstance();
    if (g_get_instance == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<NativeModuleManagerView*>(g_get_instance());
}

bool FillNativeModuleView(const NativeModuleNode* node, NativeModule* out)
{
    if (node == nullptr || out == nullptr) {
        return false;
    }
    *out = NativeModule {};
    out->name = node->name;
    out->moduleName = node->moduleName;
    out->fileName = node->fileName;
    out->systemFilePath = node->systemFilePath;
    out->registerCallback = node->registerCallback;
    out->getABCCode = node->getABCCode;
    out->getJSCode = node->getJSCode;
    out->version = node->version;
    out->flags = node->flags;
    out->refCount = node->refCount;
    out->next = nullptr;
    out->jsCode = node->jsCode;
    out->jsABCCode = node->jsABCCode;
    out->jsCodeLen = node->jsCodeLen;
    out->moduleLoaded = node->moduleLoaded;
    out->isAppModule = node->isAppModule;
    return true;
}

}  // namespace

}  // namespace ace_napi_hook

bool ace_napi_hook::FindPlatformNativeModule(const char* name, NativeModule* result)
{
    if (result == nullptr) {
        return 0;
    }
    *result = NativeModule {};
    auto* manager = ace_napi_hook::GetManager();
    if (manager == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(manager->nativeModuleListMutex_);
    // HarmonyOS links newly registered modules from tailNativeModule_ through
    // NativeModule::next. headNativeModule_ is not the traversal root on the
    // platform versions supported by the xhook compatibility backend.
    for (auto* cur = manager->tailNativeModule_; cur != nullptr; cur = cur->next) {
        if (ace_napi_hook::MatchName(cur, name)) {
            if (!ace_napi_hook::FillNativeModuleView(cur, result)) {
                return 0;
            }
            return 1;
        }
    }
    return 0;
}
