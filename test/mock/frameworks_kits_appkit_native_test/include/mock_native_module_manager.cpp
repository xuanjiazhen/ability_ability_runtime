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

#include "mock_native_module_manager.h"

#include <algorithm>
#include <unistd.h>

namespace {
    NativeModuleManager* g_instance = nullptr;
    NativeModule* g_module = nullptr;
    std::vector<std::string> g_loadedSystemModules;
    std::vector<std::string> g_preloadCalls;
    std::vector<std::string> g_preloadFailNames;
    int g_preloadCallTid = 0;
}

void MockSetNativeModuleManager(NativeModuleManager* instance)
{
    g_instance = instance;
}

void MockSetNativeModule(NativeModule* module)
{
    g_module = module;
}

void MockSetLoadedSystemNativeModules(const std::vector<std::string>& names)
{
    g_loadedSystemModules = names;
}

void MockSetPreloadFailNames(const std::vector<std::string>& names)
{
    g_preloadFailNames = names;
}

std::vector<std::string> MockGetPreloadCalls()
{
    return g_preloadCalls;
}

int MockGetPreloadCallTid()
{
    return g_preloadCallTid;
}

void MockResetNativeModuleManagerState()
{
    g_loadedSystemModules.clear();
    g_preloadCalls.clear();
    g_preloadFailNames.clear();
    g_preloadCallTid = 0;
    g_module = nullptr;
}

NativeModuleManager* NativeModuleManager::GetInstance()
{
    static NativeModuleManager realMockInstance;
    if (g_instance == reinterpret_cast<NativeModuleManager*>(1)) {
        return &realMockInstance;
    }
    return g_instance;
}

std::vector<std::string> NativeModuleManager::GetLoadedSystemNativeModules() const
{
    return g_loadedSystemModules;
}

int NativeModuleManager::PreloadSystemNativeModule(const char* moduleName)
{
    if (moduleName == nullptr) {
        return -1;
    }
    g_preloadCallTid = static_cast<int>(gettid());
    std::string name = moduleName;
    g_preloadCalls.emplace_back(name);
    if (std::find(g_preloadFailNames.begin(), g_preloadFailNames.end(), name) != g_preloadFailNames.end()) {
        return -1;
    }
    return 0;
}

NativeModule* NativeModuleManager::LoadNativeModuleWithErrorInfo(const char* moduleName, const char* path,
    bool isAppModule, std::string& errInfo, bool internal, const char* relativePath, std::string& loadErrInfo)
{
    return LoadNativeModule(moduleName, path, isAppModule, errInfo, internal, relativePath);
}

