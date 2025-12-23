/*
 * Copyright (c) 2025  Huawei Device Co., Ltd.
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

#include "page_config_manager.h"

#include <dlfcn.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS::AbilityRuntime {
namespace{
constexpr const char *PAGE_CONFIG_LIBNAME = "libhmos_pageconfig.z.so";
using InitializeFunc = void(*)(const std::string&, const sptr<Rosen::Window>&);
InitializeFunc g_initializeFunc = nullptr;
using NotifyPageChangedFunc = void(*)(char*, uint32_t);
NotifyPageChangedFunc g_notifyPageChangedFunc = nullptr;
}

PageConfigManager &PageConfigManager::GetInstance()
{
    static PageConfigManager instance;
    return instance;
}

void PageConfigManager::Initialize(std::string configJson, const sptr<Rosen::Window>& window)
{
    std::lock_guard guard(g_loadsoMutex);
    if (isInitialized_) {
        TAG_LOGD(AAFwkTag::ABILITY, "already initialized");
        return;
    }

    LoadPageConfigSo();
    auto symbol = dlsym(pageConfigSo_, "Initialize");
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s, %{public}s", "Initialize", dlerror());
        dlclose(pageConfigSo_);
        return;
    }
    g_initializeFunc = reinterpret_cast<InitializeFunc>(symbol);
    g_initializeFunc(configJson, window);

    isInitialized_ = true;
    return;
}

void PageConfigManager::LoadPageConfigSo()
{
    if (pageConfigSo_ == nullptr) {
        pageConfigSo_ = dlopen(PAGE_CONFIG_LIBNAME, RTLD_LAZY);
        if (pageConfigSo_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "null pageConfigSo_");
            return;
        }
    }
}

void PageConfigManager::NotifyPageChanged(char* targetPageName, uint32_t windowId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    std::lock_guard lock(g_loadsoMutex);
    LoadPageConfigSo();
    auto symbol = dlsym(pageConfigSo_, "NotifyPageChanged");
    if (symbol == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlsym failed %{public}s, %{public}s", "NotifyPageChanged", dlerror());
        dlclose(pageConfigSo_);
        return;
    }
    g_notifyPageChangedFunc = reinterpret_cast<NotifyPageChangedFunc>(symbol);
    g_notifyPageChangedFunc(targetPageName, windowId);
}

} // namespace AbilityRuntime
