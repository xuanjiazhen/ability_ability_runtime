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

#include "image_pixel_map_adapter.h"
#include <dlfcn.h>
#include <cstring>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr uint32_t SLEEP_TIME_US = 10000;
constexpr uint32_t RETRY_TIMES = 3;

// 函数指针类型
using PixelMapReadFromParcelFunc = Media::PixelMap* (*)(Parcel& parcel, bool isPrivate);
using PixelMapWriteToParcelFunc = bool (*)(const Media::PixelMap* pixelMap, Parcel& parcel);
using PixelMapCreateWhiteFunc = Media::PixelMap* (*)(int32_t width, int32_t height);
using PixelMapDestroyFunc = void (*)(Media::PixelMap* pixelMap);

// 全局状态
void* g_handle = nullptr;
PixelMapReadFromParcelFunc g_readFunc = nullptr;
PixelMapWriteToParcelFunc g_writeFunc = nullptr;
PixelMapCreateWhiteFunc g_createWhiteFunc = nullptr;
PixelMapDestroyFunc g_destroyFunc = nullptr;
} // namespace

bool LoadImagePixelMapAdapter(void)
{
    if (g_handle != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Image pixel map adapter has already loaded.");
        return true;
    }

    uint32_t cnt = 0;
    const char* dlopenError = nullptr;
    do {
        cnt++;
        dlerror();
        g_handle = dlopen(IMAGE_PIXEL_MAP_ADAPTER_SO_PATH, RTLD_LAZY);
        dlopenError = dlerror();
        if (dlopenError) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dlopen error: %{public}s", dlopenError);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "dlopen %{public}s, retry cnt: %{public}d",
            IMAGE_PIXEL_MAP_ADAPTER_SO_PATH, cnt);
        usleep(SLEEP_TIME_US);
    } while (!g_handle && cnt < RETRY_TIMES);

    return g_handle != nullptr;
}

void UnloadImagePixelMapAdapter(void)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "unload image pixel map adapter.");
    if (g_handle != nullptr) {
        dlclose(g_handle);
        g_handle = nullptr;
    }
    g_readFunc = nullptr;
    g_writeFunc = nullptr;
    g_createWhiteFunc = nullptr;
    g_destroyFunc = nullptr;
}

__attribute__((no_sanitize("cfi"))) Media::PixelMap* PixelMapReadFromParcel(Parcel& parcel, bool isPrivate)
{
    if (g_handle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "g_handle is nullptr");
        return nullptr;
    }

    if (g_readFunc == nullptr) {
        dlerror();
        g_readFunc = reinterpret_cast<PixelMapReadFromParcelFunc>(
            dlsym(g_handle, "PixelMapReadFromParcelImpl"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym PixelMapReadFromParcelImpl error: %{public}s", dlsymError);
            return nullptr;
        }
    }

    if (g_readFunc == nullptr) {
        return nullptr;
    }

    return g_readFunc(parcel, isPrivate);
}

__attribute__((no_sanitize("cfi"))) bool PixelMapWriteToParcel(const Media::PixelMap* pixelMap, Parcel& parcel)
{
    if (g_handle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "g_handle is nullptr");
        return false;
    }

    if (g_writeFunc == nullptr) {
        dlerror();
        g_writeFunc = reinterpret_cast<PixelMapWriteToParcelFunc>(
            dlsym(g_handle, "PixelMapWriteToParcelImpl"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym PixelMapWriteToParcelImpl error: %{public}s", dlsymError);
            return false;
        }
    }

    if (g_writeFunc == nullptr) {
        return false;
    }

    return g_writeFunc(pixelMap, parcel);
}

__attribute__((no_sanitize("cfi"))) Media::PixelMap* PixelMapCreateWhite(int32_t width, int32_t height)
{
    if (g_handle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "g_handle is nullptr");
        return nullptr;
    }

    if (g_createWhiteFunc == nullptr) {
        dlerror();
        g_createWhiteFunc = reinterpret_cast<PixelMapCreateWhiteFunc>(
            dlsym(g_handle, "PixelMapCreateWhiteImpl"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym PixelMapCreateWhiteImpl error: %{public}s", dlsymError);
            return nullptr;
        }
    }

    if (g_createWhiteFunc == nullptr) {
        return nullptr;
    }

    return g_createWhiteFunc(width, height);
}

__attribute__((no_sanitize("cfi"))) void PixelMapDestroy(Media::PixelMap* pixelMap)
{
    if (g_handle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "g_handle is nullptr");
        return;
    }

    if (g_destroyFunc == nullptr) {
        dlerror();
        g_destroyFunc = reinterpret_cast<PixelMapDestroyFunc>(
            dlsym(g_handle, "PixelMapDestroyImpl"));
        const char* dlsymError = dlerror();
        if (dlsymError) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym PixelMapDestroyImpl error: %{public}s", dlsymError);
            return;
        }
    }

    if (g_destroyFunc == nullptr) {
        return;
    }

    g_destroyFunc(pixelMap);
}

} // namespace AAFwk
} // namespace OHOS
