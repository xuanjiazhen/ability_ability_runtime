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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_SNAPSHOT_H
#define OHOS_ABILITY_RUNTIME_MISSION_SNAPSHOT_H

#include "parcel.h"
#include "element_name.h"
#include <memory>

namespace OHOS {
namespace AAFwk {
struct MissionSnapshot : public Parcelable {
    AppExecFwk::ElementName topAbility;

    // 使用 void* 存储快照句柄，避免编译时依赖 Media::PixelMap
    // 实际类型通过运行时动态加载的库来解析
    void* snapshotHandle = nullptr;

    // If is private, ability is secure, snapshot is a blank picture.
    bool isPrivate = false;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static MissionSnapshot *Unmarshalling(Parcel &parcel);

    /**
     * @brief 获取快照句柄（用于兼容现有代码）
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
