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
#include "pixel_map.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

extern "C" __attribute__((visibility("default"))) Media::PixelMap* PixelMapReadFromParcelImpl(
    Parcel& parcel, bool isPrivate)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "PixelMapReadFromParcelImpl in, isPrivate: %{public}d", isPrivate);

    std::shared_ptr<Media::PixelMap> pixelMap(parcel.ReadParcelable<Media::PixelMap>());
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to read pixel map from parcel");
        return nullptr;
    }

    // 如果是私有窗口，填充白色
    if (isPrivate) {
        int32_t byteCount = pixelMap->GetByteCount();
        uint8_t* data = const_cast<uint8_t*>(pixelMap->GetPixels());
        if (byteCount > 0 && data != nullptr) {
            memset(data, 0xFF, byteCount);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Created white pixel map: %{public}dx%{public}d",
                pixelMap->GetWidth(), pixelMap->GetHeight());
        }
    }

    // 返回原始指针，调用者负责管理生命周期
    return pixelMap.release();
}

extern "C" __attribute__((visibility("default"))) bool PixelMapWriteToParcelImpl(
    const Media::PixelMap* pixelMap, Parcel& parcel)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "PixelMapWriteToParcelImpl in");

    if (pixelMap == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "pixelMap is nullptr, treat as success");
        return true;
    }

    return parcel.WriteParcelable(pixelMap);
}

extern "C" __attribute__((visibility("default"))) Media::PixelMap* PixelMapCreateWhiteImpl(
    int32_t width, int32_t height)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "PixelMapCreateWhiteImpl in, width: %{public}d, height: %{public}d", width, height);

    Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;

    Media::PixelMap* pixelMap = Media::PixelMap::Create(opts);
    if (pixelMap == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create pixel map");
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

} // namespace AAFwk
} // namespace OHOS
