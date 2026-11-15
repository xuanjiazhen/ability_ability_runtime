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

#include <dlfcn.h>
#include <type_traits>

#include "hilog_tag_wrapper.h"
#include "pixel_map_taihe_ani.h"

namespace OHOS::AbilityRuntime {
namespace {
void* GetImageTaihe()
{
    // Returned ANI objects and native images may outlive the original call.
    static void* handle = []() -> void* {
        auto module = dlopen("libimage_taihe.z.so", RTLD_NOW | RTLD_LOCAL);
        if (module == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "load image Taihe failed: %{public}s", dlerror());
        }
        return module;
    }();
    return handle;
}

template<typename Function>
Function ResolveImageTaihe(const char* symbol)
{
    auto handle = GetImageTaihe();
    if (handle == nullptr) {
        return nullptr;
    }
    auto function = reinterpret_cast<Function>(dlsym(handle, symbol));
    if (function == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "resolve image Taihe symbol %{public}s failed: %{public}s", symbol, dlerror());
    }
    return function;
}
} // namespace

__attribute__((visibility("hidden"))) std::shared_ptr<Media::PixelMap> GetPixelMapLazy(ani_env* env, ani_object object)
    __asm__(
        "__wrap__ZN4OHOS5Media16PixelMapTaiheAni17GetNativePixelMapEP9__ani_envP12__ani_object");

std::shared_ptr<Media::PixelMap> GetPixelMapLazy(ani_env* env, ani_object object)
{
    using Function = decltype(&Media::PixelMapTaiheAni::GetNativePixelMap);
    static_assert(std::is_same_v<Function, decltype(&GetPixelMapLazy)>);
    static const Function function = ResolveImageTaihe<Function>(
        "_ZN4OHOS5Media16PixelMapTaiheAni17GetNativePixelMapEP9__ani_envP12__ani_object");
    return function == nullptr ? nullptr : function(env, object);
}

} // namespace OHOS::AbilityRuntime
