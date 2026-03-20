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

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_PIXEL_MAP_ADAPTER_H
#define OHOS_ABILITY_RUNTIME_IMAGE_PIXEL_MAP_ADAPTER_H

#include <cstdint>
#include <memory>

#include "parcel.h"

namespace OHOS {
namespace Media {
class PixelMap;
} // namespace Media

namespace AAFwk {

#if (defined(__aarch64__) || defined(__x86_64__))
constexpr const char* IMAGE_PIXEL_MAP_ADAPTER_SO_PATH = "/system/lib64/libimage_pixel_map_adapter.z.so";
#else
constexpr const char* IMAGE_PIXEL_MAP_ADAPTER_SO_PATH = "/system/lib/libimage_pixel_map_adapter.z.so";
#endif

/**
 * @brief 加载 libimage_pixel_map_adapter.z.so
 * @return 成功返回 true
 */
bool LoadImagePixelMapAdapter(void);

/**
 * @brief 卸载 libimage_pixel_map_adapter.z.so
 */
void UnloadImagePixelMapAdapter(void);

/**
 * @brief 从 Parcel 读取 PixelMap
 * @param parcel Parcel 对象引用
 * @param isPrivate 是否为私有窗口（用于生成白色快照）
 * @return PixelMap 指针，失败返回 nullptr
 */
Media::PixelMap* PixelMapReadFromParcel(Parcel& parcel, bool isPrivate = false);

/**
 * @brief 将 PixelMap 写入 Parcel
 * @param pixelMap PixelMap 指针
 * @param parcel Parcel 对象引用
 * @return 成功返回 true
 */
bool PixelMapWriteToParcel(const Media::PixelMap* pixelMap, Parcel& parcel);

/**
 * @brief 创建白色 PixelMap（用于私有窗口）
 * @param width 宽度
 * @param height 高度
 * @return PixelMap 指针，失败返回 nullptr
 */
Media::PixelMap* PixelMapCreateWhite(int32_t width, int32_t height);

/**
 * @brief 销毁 PixelMap 对象
 * @param pixelMap PixelMap 指针，可以为 nullptr
 * @note 此函数用于在动态加载的库中正确删除 PixelMap 对象
 */
void PixelMapDestroy(Media::PixelMap* pixelMap);

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_IMAGE_PIXEL_MAP_ADAPTER_H
