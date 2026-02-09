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

#include "fa_ability_thread_loader.h"

#include <dlfcn.h>
#include <mutex>
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {

// FA 动态加载相关定义
namespace {
    void* g_faLibHandle = nullptr;
    std::mutex g_faLibMutex;

    // 函数指针类型：返回 sptr<AbilityThread> 的函数
    using CreateFAAbilityThreadFunc = sptr<AbilityThread>(*)();

    // 缓存的函数指针（仅初始化一次）
    CreateFAAbilityThreadFunc g_createFunc = nullptr;

    // FA SO 文件路径
    constexpr const char* FA_SO_PATH = "libabilitykit_native_fa.so";
    constexpr const char* CREATE_FUNC_NAME = "CreateFAAbilityThread";
} // namespace

sptr<AbilityThread> CreateFAAbilityThreadDynamic()
{
    std::lock_guard<std::mutex> lock(g_faLibMutex);
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "CreateFAAbilityThreadDynamic");

    // 如果已经加载过且函数指针已缓存，直接使用
    if (g_createFunc != nullptr) {
        return g_createFunc();
    }

    // 首次加载 FA SO 文件
    TAG_LOGI(AAFwkTag::ABILITY, "Loading FA SO: %{public}s", FA_SO_PATH);

    g_faLibHandle = dlopen(FA_SO_PATH, RTLD_LAZY | RTLD_NODELETE);
    if (g_faLibHandle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load %{public}s: %{public}s",
                 FA_SO_PATH, dlerror());
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Successfully loaded %{public}s", FA_SO_PATH);

    // 获取 CreateFAAbilityThread 函数
    g_createFunc = reinterpret_cast<CreateFAAbilityThreadFunc>(
        dlsym(g_faLibHandle, CREATE_FUNC_NAME));

    if (g_createFunc == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to find %{public}s: %{public}s",
                 CREATE_FUNC_NAME, dlerror());
        dlclose(g_faLibHandle);
        g_faLibHandle = nullptr;
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "%{public}s loaded successfully", CREATE_FUNC_NAME);

    // 调用函数创建 FAAbilityThread
    return g_createFunc();
}

bool IsFaAbilitySoLoaded()
{
    std::lock_guard<std::mutex> lock(g_faLibMutex);
    return g_faLibHandle != nullptr && g_createFunc != nullptr;
}

bool UnloadFaAbilitySo()
{
    std::lock_guard<std::mutex> lock(g_faLibMutex);

    if (g_faLibHandle == nullptr) {
        return true;  // 已经是卸载状态
    }

    // 清除函数指针缓存
    g_createFunc = nullptr;

    // 卸载 SO
    int ret = dlclose(g_faLibHandle);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to unload FA SO: %{public}s", dlerror());
        return false;
    }

    g_faLibHandle = nullptr;
    TAG_LOGI(AAFwkTag::ABILITY, "FA SO unloaded successfully");
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS
