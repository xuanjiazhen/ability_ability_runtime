/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_PIXEL_MAP_LOADER_H
#define OHOS_ABILITY_RUNTIME_PIXEL_MAP_LOADER_H

#include <cstdint>
#include <functional>

namespace OHOS {
namespace Media {
// 完全前向声明，避免任何编译时依赖
class PixelMap;
} // namespace Media

namespace AAFwk {

/**
 * @brief 像素图加载器 - 运行时动态加载 libimage_native.z.so
 *
 * 使用引用计数管理库的生命周期：
 * - Load() 时调用 dlopen，增加引用计数
 * - Unload() 时减少引用计数，计数归零时调用 dlclose
 */
class PixelMapLoader {
public:
    /**
     * @brief 加载 libimage_native.z.so
     * @return 成功返回 true
     */
    static bool Load();

    /**
     * @brief 卸载 libimage_native.z.so
     * 注意：只有当所有使用者都调用 Unload 且引用计数归零时才真正卸载
     */
    static void Unload();

    /**
     * @brief 检查库是否已加载
     * @return 已加载返回 true
     */
    static bool IsLoaded();

    /**
     * @brief 从 Parcel 读取 PixelMap
     * 如果库未加载会自动调用 Load()
     * @param parcel Parcel 对象指针（使用 void* 避免类型依赖）
     * @param isPrivate 是否为私有窗口（用于生成白色快照）
     * @return PixelMap 指针，失败返回 nullptr
     */
    static Media::PixelMap* ReadPixelMapFromParcel(void* parcel, bool isPrivate = false);

    /**
     * @brief 将 PixelMap 写入 Parcel
     * 如果库未加载会自动调用 Load()
     * @param pixelMap PixelMap 指针
     * @param parcel Parcel 对象指针（使用 void* 避免类型依赖）
     * @return 成功返回 true
     */
    static bool WritePixelMapToParcel(const Media::PixelMap* pixelMap, void* parcel);

    /**
     * @brief 创建白色 PixelMap（用于私有窗口）
     * @param width 宽度
     * @param height 高度
     * @return PixelMap 指针，失败返回 nullptr
     */
    static Media::PixelMap* CreateWhitePixelMap(int32_t width, int32_t height);

    /**
     * @brief 获取 PixelMap 的属性（宽、高、字节数、像素数据）
     * @param pixelMap PixelMap 指针
     * @param width 输出宽度
     * @param height 输出高度
     * @param byteCount 输出字节数
     * @param pixels 输出像素数据指针
     */
    static void GetPixelMapProperties(const Media::PixelMap* pixelMap,
                                       int32_t* width, int32_t* height,
                                       int32_t* byteCount, const uint8_t** pixels);

    /**
     * @brief 修改 PixelMap 像素数据（用于填充白色）
     * @param pixelMap PixelMap 指针
     * @param value 填充值
     * @return 成功返回 true
     */
    static bool FillPixelMap(Media::PixelMap* pixelMap, uint8_t value);

    /**
     * @brief 删除 PixelMap
     * @param pixelMap PixelMap 指针
     */
    static void DeletePixelMap(Media::PixelMap* pixelMap);

private:
    // 禁止实例化
    PixelMapLoader() = default;
    ~PixelMapLoader() = default;
    PixelMapLoader(const PixelMapLoader&) = delete;
    PixelMapLoader& operator=(const PixelMapLoader&) = delete;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PIXEL_MAP_LOADER_H
