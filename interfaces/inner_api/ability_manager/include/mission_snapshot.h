/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#ifndef OHOS_ABILITY_RUNTIME_MISSION_SNAPSHOT_H
#define OHOS_ABILITY_RUNTIME_MISSION_SNAPSHOT_H

#include "parcel.h"
#include "element_name.h"
#include <memory>

namespace OHOS {
// 完全不引用 Media::PixelMap，避免编译时依赖

namespace AAFwk {
struct MissionSnapshot : public Parcelable {
    AppExecFwk::ElementName topAbility;

    // 使用 void* 存储快照，避免编译时依赖 Media::PixelMap
    // 实际类型通过运行时动态加载的库来解析
    void* snapshotHandle = nullptr;

    // If is private, ability is secure, snapshot is a blank picture.
    bool isPrivate = false;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionSnapshot *Unmarshalling(Parcel &parcel);

    /**
     * @brief 获取快照句柄（仅用于兼容现有代码）
     * @return 快照句柄
     */
    void* GetSnapshotHandle() const { return snapshotHandle; }

    /**
     * @brief 设置快照句柄（用于兼容现有代码）
     * @param handle 快照句柄
     */
    void SetSnapshotHandle(void* handle) { snapshotHandle = handle; }
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_MISSION_SNAPSHOT_H
