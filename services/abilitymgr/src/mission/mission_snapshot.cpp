/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "mission_snapshot.h"
#ifdef SUPPORT_SCREEN
#include "image_pixel_map_adapter.h"
#include "hilog_tag_wrapper.h"
#endif //SUPPORT_SCREEN

namespace OHOS {
namespace AAFwk {

bool MissionSnapshot::ReadFromParcel(Parcel &parcel)
{
#ifdef SUPPORT_SCREEN
    std::unique_ptr<AppExecFwk::ElementName> ability(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (ability == nullptr) {
        return false;
    }
    topAbility = *ability;

    // 动态加载 libimage_pixel_map_adapter.z.so 并读取 PixelMap
    Media::PixelMap* pixelMap = nullptr;
    if (LoadImagePixelMapAdapter()) {
        pixelMap = PixelMapReadFromParcel(parcel, isPrivate);
    }

    if (pixelMap == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to read pixel map from parcel");
        snapshot.reset();
    } else {
        snapshot.reset(pixelMap);
    }
#endif
    return true;
}

MissionSnapshot *MissionSnapshot::Unmarshalling(Parcel &parcel)
{
    MissionSnapshot *info = new (std::nothrow) MissionSnapshot();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool MissionSnapshot::Marshalling(Parcel &parcel) const
{
#ifdef SUPPORT_SCREEN
    if (!parcel.WriteParcelable(&topAbility)) {
        return false;
    }

    // 动态加载 libimage_pixel_map_adapter.z.so 并写入 PixelMap
    if (!LoadImagePixelMapAdapter()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to load image pixel map adapter");
        return true;  // 如果加载失败，视为成功（向后兼容）
    }

    if (!PixelMapWriteToParcel(snapshot.get(), parcel)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to write pixel map to parcel");
    }
#endif
    return true;
}

}  // namespace AAFwk
}  // namespace OHOS
