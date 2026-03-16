/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 *
 * 像素图加载器 - 运行时动态加载 libimage_native.z.so
 * 使用 dlopen/dlsym + dlclose 实现完全动态加载
 */

#include "pixel_map_loader.h"
#include <dlfcn.h>
#include <cstring>
#include <mutex>
#include "hilog/log.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD001300, "PixelMapLoader"};
constexpr const char* IMAGE_NATIVE_LIB = "libimage_native.z.so";

// 全局状态
void* g_libraryHandle = nullptr;
int g_refCount = 0;
std::mutex g_mutex;

// 函数指针类型
using PixelMapReadFunc = Media::PixelMap* (*)(void* parcel);
using PixelMapWriteFunc = bool (*)(const Media::PixelMap* pixelMap, void* parcel);
using PixelMapCreateWhiteFunc = Media::PixelMap* (*)(int32_t width, int32_t height);
using PixelMapGetPropsFunc = void (*)(const Media::PixelMap* pixelMap, int32_t* width,
                                            int32_t* height, int32_t* byteCount, const uint8_t** pixels);
using PixelMapFillFunc = bool (*)(Media::PixelMap* pixelMap, uint8_t value);
using PixelMapDeleteFunc = void (*)(Media::PixelMap* pixelMap);

// 符号指针
PixelMapReadFunc s_readFunc = nullptr;
PixelMapWriteFunc s_writeFunc = nullptr;
PixelMapCreateWhiteFunc s_createWhiteFunc = nullptr;
PixelMapGetPropsFunc s_getPropsFunc = nullptr;
PixelMapFillFunc s_fillFunc = nullptr;
PixelMapDeleteFunc s_deleteFunc = nullptr;

/**
 * @brief 加载所有需要的符号
 * @return 成功返回 true
 */
bool LoadSymbols()
{
    if (s_readFunc != nullptr) {
        return true;  // 已加载
    }

    s_readFunc = reinterpret_cast<PixelMapReadFunc>(dlsym(g_libraryHandle, "PixelMap_ReadParcelableFromParcel"));
    s_writeFunc = reinterpret_cast<PixelMapWriteFunc>(dlsym(g_libraryHandle, "PixelMap_WriteParcelableToParcel"));
    s_createWhiteFunc = reinterpret_cast<PixelMapCreateWhiteFunc>(dlsym(g_libraryHandle, "PixelMap_CreateWhite"));
    s_getPropsFunc = reinterpret_cast<PixelMapGetPropsFunc>(dlsym(g_libraryHandle, "PixelMap_GetProperties"));
    s_fillFunc = reinterpret_cast<PixelMapFillFunc>(dlsym(g_libraryHandle, "PixelMap_Fill"));
    s_deleteFunc = reinterpret_cast<PixelMapDeleteFunc>(dlsym(g_libraryHandle, "PixelMap_Delete"));

    const char* error = dlerror();
    if (error != nullptr) {
        HILOGE(LABEL, "Failed to load symbols: %{public}s", error);
        return false;
    }

    HILOGI(LABEL, "Successfully loaded PixelMap symbols");
    return true;
}

} // namespace

bool PixelMapLoader::Load()
{
    std::lock_guard<std::mutex> lock(g_mutex);

    if (g_libraryHandle != nullptr) {
        g_refCount++;
        HILOGI(LABEL, "Library already loaded, refCount: %{public}d", g_refCount);
        return true;
    }

    HILOGI(LABEL, "dlopen loading %{public}s...", IMAGE_NATIVE_LIB);

    // 使用 RTLD_NOW 立即解析符号，RTLD_GLOBAL 使符号全局可用
    g_libraryHandle = dlopen(IMAGE_NATIVE_LIB, RTLD_NOW | RTLD_GLOBAL);
    if (g_libraryHandle == nullptr) {
        const char* error = dlerror();
        HILOGE(LABEL, "Failed to dlopen %{public}s: %{public}s", IMAGE_NATIVE_LIB, error ? error : "unknown");
        return false;
    }

    HILOGI(LABEL, "Successfully loaded %{public}s", IMAGE_NATIVE_LIB);
    g_refCount = 1;

    return LoadSymbols();
}

void PixelMapLoader::Unload()
{
    std::lock_guard<std::mutex> lock(g_mutex);

    g_refCount--;
    HILOGI(LABEL, "Unload called, refCount: %{public}d", g_refCount);

    if (g_refCount > 0) {
        HILOGI(LABEL, "Library still in use, keeping loaded");
        return;
    }

    if (g_libraryHandle != nullptr) {
        HILOGI(LABEL, "dlclose unloading %{public}s...", IMAGE_NATIVE_LIB);
        dlclose(g_libraryHandle);
        g_libraryHandle = nullptr;
        s_readFunc = nullptr;
        s_writeFunc = nullptr;
        s_createWhiteFunc = nullptr;
        s_getPropsFunc = nullptr;
        s_fillFunc = nullptr;
        s_deleteFunc = nullptr;
        g_refCount = 0;
    }
}

bool PixelMapLoader::IsLoaded()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_libraryHandle != nullptr;
}

Media::PixelMap* PixelMapLoader::ReadPixelMapFromParcel(void* parcel, bool isPrivate)
{
    if (!Load()) {
        HILOGW(LABEL, "Library not available, cannot read PixelMap");
        return nullptr;
    }

    if (s_readFunc == nullptr) {
        HILOGE(LABEL, "ReadFunc not available");
        return nullptr;
    }

    // 调用动态加载的函数
    Media::PixelMap* pixelMap = s_readFunc(parcel);

    // 如果是私有窗口，创建白色快照
    if (isPrivate && pixelMap != nullptr && s_fillFunc != nullptr) {
        int32_t width = 0, height = 0, byteCount = 0;
        const uint8_t* pixels = nullptr;
        s_getPropsFunc(pixelMap, &width, &height, &byteCount, &pixels);

        if (pixels != nullptr && byteCount > 0) {
            s_fillFunc(pixelMap, 0xFF);  // 填充白色
            HILOGD(LABEL, "Created white pixel map: %{public}dx%{public}d", width, height);
        }
    }

    return pixelMap;
}

bool PixelMapLoader::WritePixelMapToParcel(const Media::PixelMap* pixelMap, void* parcel)
{
    if (!Load()) {
        HILOGW(LABEL, "Library not available, cannot write PixelMap");
        return false;
    }

    if (s_writeFunc == nullptr) {
        HILOGE(LABEL, "WriteFunc not available");
        return false;
    }

    if (pixelMap == nullptr) {
        return true;  // 空快照，视为成功
    }

    return s_writeFunc(pixelMap, parcel);
}

Media::PixelMap* PixelMapLoader::CreateWhitePixelMap(int32_t width, int32_t height)
{
    if (!Load()) {
        HILOGW(LABEL, "Library not available, cannot create white PixelMap");
        return nullptr;
    }

    if (s_createWhiteFunc == nullptr) {
        HILOGE(LABEL, "CreateWhiteFunc not available");
        return nullptr;
    }

    return s_createWhiteFunc(width, height);
}

void PixelMapLoader::GetPixelMapProperties(const Media::PixelMap* pixelMap,
                                        int32_t* width, int32_t* height,
                                        int32_t* byteCount, const uint8_t** pixels)
{
    if (s_getPropsFunc != nullptr && pixelMap != nullptr) {
        s_getPropsFunc(pixelMap, width, height, byteCount, pixels);
    } else {
        *width = 0;
        *height = 0;
        *byteCount = 0;
        *pixels = nullptr;
    }
}

bool PixelMapLoader::FillPixelMap(Media::PixelMap* pixelMap, uint8_t value)
{
    if (s_fillFunc != nullptr && pixelMap != nullptr) {
        return s_fillFunc(pixelMap, value);
    }
    return false;
}

void PixelMapLoader::DeletePixelMap(Media::PixelMap* pixelMap)
{
    if (s_deleteFunc != nullptr && pixelMap != nullptr) {
        s_deleteFunc(pixelMap);
    }
}

} // namespace AAFwk
} // namespace OHOS
