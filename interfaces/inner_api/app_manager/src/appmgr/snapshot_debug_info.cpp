/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "snapshot_debug_info.h"

#include <new>

namespace OHOS {
namespace AppExecFwk {

bool SnapshotDebugAppInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName) || !parcel.WriteInt32(uid) || !parcel.WriteInt32(userId) ||
        !parcel.WriteInt32(imagePid) || !parcel.WriteInt32(templatePid) || !parcel.WriteString(state) ||
        !parcel.WriteInt64(snapshotId) || !parcel.WriteInt32(isEnable) || !parcel.WriteInt32(isDebug) ||
        !parcel.WriteString(noSnapshotReason)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(runningPids.size()))) {
        return false;
    }
    for (int32_t pid : runningPids) {
        if (!parcel.WriteInt32(pid)) {
            return false;
        }
    }
    return true;
}

bool SnapshotDebugAppInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadString(bundleName) || !parcel.ReadInt32(uid) || !parcel.ReadInt32(userId) ||
        !parcel.ReadInt32(imagePid) || !parcel.ReadInt32(templatePid) || !parcel.ReadString(state) ||
        !parcel.ReadInt64(snapshotId) || !parcel.ReadInt32(isEnable) || !parcel.ReadInt32(isDebug) ||
        !parcel.ReadString(noSnapshotReason)) {
        return false;
    }
    int32_t size = 0;
    if (!parcel.ReadInt32(size) || size < 0) {
        return false;
    }
    runningPids.clear();
    runningPids.reserve(static_cast<size_t>(size));
    for (int32_t i = 0; i < size; ++i) {
        int32_t pid = -1;
        if (!parcel.ReadInt32(pid)) {
            return false;
        }
        runningPids.push_back(pid);
    }
    return true;
}

SnapshotDebugAppInfo *SnapshotDebugAppInfo::Unmarshalling(Parcel &parcel)
{
    SnapshotDebugAppInfo *info = new (std::nothrow) SnapshotDebugAppInfo();
    if (info == nullptr || !info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

bool SnapshotDebugInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(switchState) || !parcel.WriteInt32(maxCreating) ||
        !parcel.WriteInt32(currentCreating)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(appList.size()))) {
        return false;
    }
    for (const auto &app : appList) {
        if (!app.Marshalling(parcel)) {
            return false;
        }
    }
    return true;
}

bool SnapshotDebugInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(switchState) || !parcel.ReadInt32(maxCreating) ||
        !parcel.ReadInt32(currentCreating)) {
        return false;
    }
    int32_t size = 0;
    if (!parcel.ReadInt32(size) || size < 0) {
        return false;
    }
    appList.clear();
    appList.reserve(static_cast<size_t>(size));
    for (int32_t i = 0; i < size; ++i) {
        SnapshotDebugAppInfo app;
        if (!app.ReadFromParcel(parcel)) {
            return false;
        }
        appList.push_back(app);
    }
    return true;
}

SnapshotDebugInfo *SnapshotDebugInfo::Unmarshalling(Parcel &parcel)
{
    SnapshotDebugInfo *info = new (std::nothrow) SnapshotDebugInfo();
    if (info == nullptr || !info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

} // namespace AppExecFwk
} // namespace OHOS
