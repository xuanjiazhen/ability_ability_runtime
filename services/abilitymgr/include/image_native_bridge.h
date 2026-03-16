/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_IMAGE_NATIVE_BRIDGE_H
#define OHOS_ABILITY_RUNTIME_IMAGE_NATIVE_BRIDGE_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// C 风格接口，用于动态加载 libimage_native.z.so

/**
 * @brief 从 Parcel 读取 PixelMap
 * @param parcel Parcel 对象指针
 * @return PixelMap 指针，失败返回 nullptr
 */
void* PixelMap_ReadParcelableFromParcel(void* parcel);

/**
 * @brief 将 PixelMap 写入 Parcel
 * @param pixelMap PixelMap 指针
 * @param parcel Parcel 对象指针
 * @return 成功返回 true
 */
int PixelMap_WriteParcelableToParcel(const void* pixelMap, void* parcel);

/**
 * @brief 创建白色 PixelMap
 * @param width 宽度
 * @param height 高度
 * @return PixelMap 指针，失败返回 nullptr
 */
void* PixelMap_CreateWhite(int32_t width, int32_t height);

/**
 * @brief 获取 PixelMap 的属性
 * @param pixelMap PixelMap 指针
 * @param width 输出宽度
 * @param height 输出高度
 * @param byteCount 输出字节数
 * @param pixels 输出像素数据指针
 */
void PixelMap_GetProperties(const void* pixelMap,
                            int32_t* width, int32_t* height,
                            int32_t* byteCount, const uint8_t** pixels);

/**
 * @brief 修改 PixelMap 像素数据
 * @param pixelMap PixelMap 指针
 * @param value 填充值
 * @return 成功返回 1（非0），失败返回 0
 */
int PixelMap_Fill(void* pixelMap, uint8_t value);

/**
 * @brief 删除 PixelMap
 * @param pixelMap PixelMap 指针
 */
void PixelMap_Delete(void* pixelMap);

#ifdef __cplusplus
}
#endif

#endif // OHOS_ABILITY_RUNTIME_IMAGE_NATIVE_BRIDGE_H
