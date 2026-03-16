/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * ImageNative Bridge - 提供 C 风格的接口包装 PixelMap 操作
 * 这样可以通过 dlopen/dlsym 动态加载 libimage_native.z.so
 */

#include "image_native_bridge.h"
#include <dlfcn.h>
#include <mutex>
#include "hilog/log.h"
#include "parcel.h"
#include "pixel_map.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD001300, "ImageNativeBridge"};
constexpr const char* IMAGE_NATIVE_LIB = "libimage_native.z.so";

// 全局状态
void* g_handle = nullptr;
std::mutex g_mutex;

// 函数指针
using ReadParcelableFunc = Media::PixelMap* (*)(Parcel&);
using WriteParcelableFunc = bool (*)(const Media::PixelMap*, Parcel&);

ReadParcelableFunc s_readFunc = nullptr;
WriteParcelableFunc s_writeFunc = nullptr;

// 加载库
bool LoadLibrary()
{
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_handle != nullptr) {
        return true;
    }

    HILOGI(LABEL, "Loading %{public}s...", IMAGE_NATIVE_LIB);
    g_handle = dlopen(IMAGE_NATIVE_LIB, RTLD_LAZY | RTLD_GLOBAL);
    if (g_handle == nullptr) {
        const char* error = dlerror();
        HILOGE(LABEL, "dlopen failed: %{public}s", error ? error : "unknown");
        return false;
    }

    // 获取模板实例化的符号名
    s_readFunc = reinterpret_cast<ReadParcelableFunc>(dlsym(g_handle, "_ZN3OHOS5Media8PixelMapC1Ev"));  // PixelMap 构造函数
    if (s_readFunc == nullptr) {
        HILOGE(LABEL, "Failed to get PixelMap symbols");
        dlclose(g_handle);
        g_handle = nullptr;
        return false;
    }

    HILOGI(LABEL, "Successfully loaded %{public}s", IMAGE_NATIVE_LIB);
    return true;
}

// 检查是否需要加载库
bool EnsureLibraryLoaded()
{
    return LoadLibrary();
}

} // namespace

extern "C" {

// 实现 C 风格接口
void* PixelMap_ReadParcelableFromParcel(void* parcel)
{
    if (!EnsureLibraryLoaded()) {
        HILOGW(LABEL, "Library not available");
        return nullptr;
    }

    Parcel* p = static_cast<Parcel*>(parcel);
    if (p == nullptr) {
        HILOGE(LABEL, "Invalid parcel");
        return nullptr;
    }

    return p->ReadParcelable<Media::PixelMap>();
}

int PixelMap_WriteParcelableToParcel(const void* pixelMap, void* parcel)
{
    if (!EnsureLibraryLoaded()) {
        HILOGW(LABEL, "Library not available");
        return 0;
    }

    const Media::PixelMap* pm = static_cast<const Media::PixelMap*>(pixelMap);
    Parcel* p = static_cast<Parcel*>(parcel);

    if (pm == nullptr || p == nullptr) {
        return 0;
    }

    return p->WriteParcelable(pm);
}

void* PixelMap_CreateWhite(int32_t width, int32_t height)
{
    if (!EnsureLibraryLoaded()) {
        HILOGW(LABEL, "Library not available");
        return nullptr;
    }

    // 创建 PixelMap 并填充白色
    Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;

    Media::PixelMap* pixelMap = Media::PixelMap::Create(opts);
    if (pixelMap == nullptr) {
        HILOGE(LABEL, "Failed to create PixelMap");
        return nullptr;
    }

    // 填充白色
    int32_t byteCount = pixelMap->GetByteCount();
    uint8_t* data = const_cast<uint8_t*>(pixelMap->GetPixels());
    if (byteCount > 0 && data != nullptr) {
        memset(data, 0xFF, byteCount);
    }

    return pixelMap;
}

void PixelMap_GetProperties(const void* pixelMap,
                           int32_t* width, int32_t* height,
                           int32_t* byteCount, const uint8_t** pixels)
{
    const Media::PixelMap* pm = static_cast<const Media::PixelMap*>(pixelMap);

    if (pm != nullptr) {
        *width = pm->GetWidth();
        *height = pm->GetHeight();
        *byteCount = pm->GetByteCount();
        *pixels = pm->GetPixels();
    } else {
        *width = 0;
        *height = 0;
        *byteCount = 0;
        *pixels = nullptr;
    }
}

int PixelMap_Fill(void* pixelMap, uint8_t value)
{
    Media::PixelMap* pm = static_cast<Media::PixelMap*>(pixelMap);
    if (pm != nullptr) {
        int32_t byteCount = pm->GetByteCount();
        uint8_t* data = const_cast<uint8_t*>(pm->GetPixels());
        if (data != nullptr && byteCount > 0) {
            memset(data, value, byteCount);
            return 1;
        }
    }
    return 0;
}

void PixelMap_Delete(void* pixelMap)
{
    Media::PixelMap* pm = static_cast<Media::PixelMap*>(pixelMap);
    if (pm != nullptr) {
        // PixelMap 使用引用计数，delete 会自动管理
        delete pm;
    }
}

} // extern "C"

} // namespace AAFwk
} // namespace OHOS