/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mission_snapshot.h"
#include "pixel_map_loader.h"
#include "hilog/log.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD001300, "MissionSnapshot"};
} // namespace

bool MissionSnapshot::ReadFromParcel(Parcel &parcel)
{
    // 读取 ElementName
    std::unique_ptr<AppExecFwk::ElementName> ability(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (ability == nullptr) {
        return false;
    }
    topAbility = *ability;

    // 动态加载 libimage_native.z.so（如果需要）
    // 注意：第一次调用 PixelMapLoader::Load() 会触发 dlopen
    snapshotHandle = PixelMapLoader::ReadPixelMapFromParcel(&parcel, isPrivate);

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
    // 写入 ElementName
    if (!parcel.WriteParcelable(&topAbility)) {
        return false;
    }

    // 动态加载 libimage_native.z.so（如果需要）
    // 使用 PixelMapLoader 写入快照
    if (!PixelMapLoader::WritePixelMapToParcel(
        static_cast<Media::PixelMap*>(snapshotHandle), &parcel)) {
        HILOGW(LABEL, "Write PixelMap to parcel failed");
    }

    return true;
}

}  // namespace AAFwk
}  // namespace OHOS