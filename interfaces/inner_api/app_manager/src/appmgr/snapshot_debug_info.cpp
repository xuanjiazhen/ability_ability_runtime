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

namespace {
// Field groups are split only to keep each function's cyclomatic complexity
// under the gate limit; the write/read order matches the declared field order
// and must not change, or the IPC wire format breaks.
bool WriteAppStateFields(const SnapshotDebugAppInfo &info, Parcel &parcel)
{
    return parcel.WriteString(info.bundleName) && parcel.WriteInt32(info.uid) &&
        parcel.WriteInt32(info.userId) && parcel.WriteInt32(info.imagePid) &&
        parcel.WriteInt32(info.templatePid) && parcel.WriteString(info.state) &&
        parcel.WriteInt64(info.snapshotId) && parcel.WriteInt32(info.isEnable) &&
        parcel.WriteInt32(info.isDebug);
}

bool WriteImagePolicyFields(const SnapshotDebugAppInfo &info, Parcel &parcel)
{
    return parcel.WriteInt32(info.autoMakeImage) && parcel.WriteInt32(info.defaultEnable) &&
        parcel.WriteInt32(info.maxRecords) && parcel.WriteInt32(info.recordTimeMs) &&
        parcel.WriteInt32(info.lowMemWatermark) && parcel.WriteInt64(info.delayedSeconds) &&
        parcel.WriteInt64(info.imageExpireSeconds) && parcel.WriteInt64(info.imageExpireRemaining) &&
        parcel.WriteString(info.noSnapshotReason);
}

bool WriteRunningPids(const SnapshotDebugAppInfo &info, Parcel &parcel)
{
    if (!parcel.WriteInt32(static_cast<int32_t>(info.runningPids.size()))) {
        return false;
    }
    for (int32_t pid : info.runningPids) {
        if (!parcel.WriteInt32(pid)) {
            return false;
        }
    }
    return true;
}

bool ReadAppStateFields(SnapshotDebugAppInfo &info, Parcel &parcel)
{
    return parcel.ReadString(info.bundleName) && parcel.ReadInt32(info.uid) &&
        parcel.ReadInt32(info.userId) && parcel.ReadInt32(info.imagePid) &&
        parcel.ReadInt32(info.templatePid) && parcel.ReadString(info.state) &&
        parcel.ReadInt64(info.snapshotId) && parcel.ReadInt32(info.isEnable) &&
        parcel.ReadInt32(info.isDebug);
}

bool ReadImagePolicyFields(SnapshotDebugAppInfo &info, Parcel &parcel)
{
    return parcel.ReadInt32(info.autoMakeImage) && parcel.ReadInt32(info.defaultEnable) &&
        parcel.ReadInt32(info.maxRecords) && parcel.ReadInt32(info.recordTimeMs) &&
        parcel.ReadInt32(info.lowMemWatermark) && parcel.ReadInt64(info.delayedSeconds) &&
        parcel.ReadInt64(info.imageExpireSeconds) && parcel.ReadInt64(info.imageExpireRemaining) &&
        parcel.ReadString(info.noSnapshotReason);
}

bool ReadRunningPids(SnapshotDebugAppInfo &info, Parcel &parcel)
{
    int32_t size = 0;
    if (!parcel.ReadInt32(size) || size < 0) {
        return false;
    }
    info.runningPids.clear();
    info.runningPids.reserve(static_cast<size_t>(size));
    for (int32_t i = 0; i < size; ++i) {
        int32_t pid = -1;
        if (!parcel.ReadInt32(pid)) {
            return false;
        }
        info.runningPids.push_back(pid);
    }
    return true;
}
} // namespace

bool SnapshotDebugAppInfo::Marshalling(Parcel &parcel) const
{
    return WriteAppStateFields(*this, parcel) && WriteImagePolicyFields(*this, parcel) &&
        WriteRunningPids(*this, parcel);
}

bool SnapshotDebugAppInfo::ReadFromParcel(Parcel &parcel)
{
    return ReadAppStateFields(*this, parcel) && ReadImagePolicyFields(*this, parcel) &&
        ReadRunningPids(*this, parcel);
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
