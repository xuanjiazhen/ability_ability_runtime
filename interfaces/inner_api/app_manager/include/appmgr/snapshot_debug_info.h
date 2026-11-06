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

#ifndef OHOS_ABILITY_RUNTIME_SNAPSHOT_DEBUG_INFO_H
#define OHOS_ABILITY_RUNTIME_SNAPSHOT_DEBUG_INFO_H

#include <cstdint>
#include <string>
#include <vector>

#include "hyper_snap_error_record.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief Debug snapshot info of one application, mirroring the appList entry
 *        of the RSS list reply (see the JSON key of each field in comments).
 *        Used by the aa debug commands create/destroy/list-snapshot, which
 *        take effect only for debug-signed applications.
 */
struct SnapshotDebugAppInfo : public Parcelable {
    std::string bundleName;               // "name"
    int32_t uid = -1;                     // "uid"
    int32_t userId = -1;                  // "userId"
    int32_t imagePid = -1;                // "imagePid"
    int32_t templatePid = -1;             // "templatePid"
    std::string state;                    // "state", passed through as the RSS-side string
    int64_t snapshotId = -1;              // "snapshotId"
    int32_t isEnable = 0;                 // "isEnable"
    int32_t isDebug = 0;                  // "debug"
    std::string noSnapshotReason;         // "noSnapshotReason", empty when a snapshot exists
    std::vector<int32_t> runningPids;     // "runningPids" (RSS-side key name to be confirmed in joint debug)

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SnapshotDebugAppInfo *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Debug snapshot overview, mirroring the top level of the RSS list
 *        reply. "errcode" of the reply is mapped to the IPC return code by
 *        the service and is not part of this structure.
 */
struct SnapshotDebugInfo : public Parcelable {
    int32_t switchState = 0;              // "switch", snapshot feature switch
    int32_t maxCreating = 0;              // "maxCreating"
    int32_t currentCreating = 0;          // "currentCreating"
    std::vector<SnapshotDebugAppInfo> appList;    // "appList", debug-signed applications only

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SnapshotDebugInfo *Unmarshalling(Parcel &parcel);
};

inline bool SnapshotDebugAppInfo::Marshalling(Parcel &parcel) const
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

inline bool SnapshotDebugAppInfo::ReadFromParcel(Parcel &parcel)
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

inline SnapshotDebugAppInfo *SnapshotDebugAppInfo::Unmarshalling(Parcel &parcel)
{
    SnapshotDebugAppInfo *info = new (std::nothrow) SnapshotDebugAppInfo();
    if (info == nullptr || !info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}

inline bool SnapshotDebugInfo::Marshalling(Parcel &parcel) const
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

inline bool SnapshotDebugInfo::ReadFromParcel(Parcel &parcel)
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

inline SnapshotDebugInfo *SnapshotDebugInfo::Unmarshalling(Parcel &parcel)
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

#endif // OHOS_ABILITY_RUNTIME_SNAPSHOT_DEBUG_INFO_H
