/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "memory_level_info.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int MAX_PARCEL_SIZE = 100000;
}

MemoryLevelInfo::MemoryLevelInfo(const std::map<pid_t, MemoryLevel> &procLevelMap) : procLevelMap_(procLevelMap)
{
}

bool MemoryLevelInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(procLevelMap_.size())) {
        return false;
    }
    for (auto it = procLevelMap_.begin(); it != procLevelMap_.end(); ++it) {
        if (!parcel.WriteInt32(it->first)) {
            return false;
        }
        if (!parcel.WriteInt32(it->second)) {
            return false;
        }
    }
    return true;
}

MemoryLevelInfo *MemoryLevelInfo::Unmarshalling(Parcel &parcel)
{
    MemoryLevelInfo *object = new (std::nothrow) MemoryLevelInfo();
    if ((object != nullptr) && !object->ReadFromParcel(parcel)) {
        delete object;
        object = nullptr;
    }

    return object;
}

const std::map<pid_t, MemoryLevel> &MemoryLevelInfo::GetProcLevelMap() const
{
    return procLevelMap_;
}


bool MemoryLevelInfo::ReadFromParcel(Parcel &parcel)
{
    uint32_t count = 0;
    if (!parcel.ReadUint32(count)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadUint32 count failed");
        return false;
    }
    if (count > MAX_PARCEL_SIZE) {
        TAG_LOGE(AAFwkTag::APPMGR, "count is too large: %{public}u", count);
        return false;
    }

    for (uint32_t i = 0; i < count; ++i) {
        pid_t pid = 0;
        if (!parcel.ReadInt32(pid)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadInt32 pid failed");
            return false;
        }
        int32_t tempLevel = 0;
        if (!parcel.ReadInt32(tempLevel)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ReadInt32 tempLevel failed");
            return false;
        }
        MemoryLevel level = MEMORY_LEVEL_MODERATE;
        switch (tempLevel) {
            case MEMORY_LEVEL_MODERATE:
            case MEMORY_LEVEL_LOW:
            case MEMORY_LEVEL_CRITICAL:
            case MEMORY_LEVEL_UI_HIDDEN:
            case MEMORY_LEVEL_BACKGROUND_MODERATE:
            case MEMORY_LEVEL_BACKGROUND_LOW:
            case MEMORY_LEVEL_BACKGROUND_CRITICAL:
                level = static_cast<MemoryLevel>(tempLevel);
                break;
            default:
                TAG_LOGE(AAFwkTag::APPMGR, "temp memory level=%{public}d is not valid.", tempLevel);
                continue;
        }
        procLevelMap_[pid] = level;
    }
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS