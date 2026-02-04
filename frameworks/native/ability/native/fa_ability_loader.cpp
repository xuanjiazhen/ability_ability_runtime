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

#include "fa_ability_loader.h"
#include <dlfcn.h>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

// 静态成员初始化
void* FAAbilityLoader::faLibraryHandle_ = nullptr;

// 定义从FA库中加载的函数指针类型
using CreateFAAbilityThreadFunc = sptr<AbilityThread>(*)();

sptr<AbilityThread> FAAbilityLoader::LoadFAAbilityThread(const std::string& libraryPath)
{
    TAG_LOGI(AAFwkTag::ABILITY, "Begin to load FA library: %{public}s", libraryPath.c_str());

    // 如果库已经加载，直接使用
    if (faLibraryHandle_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITY, "FA library already loaded");
    } else {
        // 加载FA模型库
        if (!LoadFALibrary(libraryPath)) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to load FA library: %{public}s", libraryPath.c_str());
            return nullptr;
        }
    }

    // 获取创建FAAbilityThread的函数
    CreateFAAbilityThreadFunc createFunc =
        reinterpret_cast<CreateFAAbilityThreadFunc>(dlsym(faLibraryHandle_, "CreateFAAbilityThread"));

    if (createFunc == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to find CreateFAAbilityThread symbol: %{public}s", dlerror());
        UnloadFALibraryInternal();
        return nullptr;
    }

    // 调用函数创建FAAbilityThread
    sptr<AbilityThread> thread = createFunc();
    if (thread == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to create FAAbilityThread");
        UnloadFALibraryInternal();
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Successfully loaded FA library and created FAAbilityThread");
    return thread;
}

void FAAbilityLoader::UnloadFALibrary()
{
    TAG_LOGI(AAFwkTag::ABILITY, "Unload FA library requested");
    UnloadFALibraryInternal();
}

bool FAAbilityLoader::LoadFALibrary(const std::string& libraryPath)
{
    TAG_LOGI(AAFwkTag::ABILITY, "Loading FA library from: %{public}s", libraryPath.c_str());

    // 使用RTLD_NOW以便立即解析符号，使用RTLD_LOCAL以避免符号污染
    faLibraryHandle_ = dlopen(libraryPath.c_str(), RTLD_NOW | RTLD_LOCAL);

    if (faLibraryHandle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "dlopen failed: %{public}s", dlerror());
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Successfully loaded FA library");
    return true;
}

void FAAbilityLoader::UnloadFALibraryInternal()
{
    if (faLibraryHandle_ != nullptr) {
        TAG_LOGI(AAFwkTag::ABILITY, "Closing FA library handle");
        dlclose(faLibraryHandle_);
        faLibraryHandle_ = nullptr;
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
