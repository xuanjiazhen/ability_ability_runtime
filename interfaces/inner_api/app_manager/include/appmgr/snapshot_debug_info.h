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
    int32_t autoMakeImage = 0;            // "autoMakeImage"
    int32_t defaultEnable = 0;            // "defaultEnable"
    int32_t maxRecords = 0;               // "maxRecords"
    int32_t recordTimeMs = 0;             // "recordTimeMs"
    int32_t lowMemWatermark = 0;          // "lowMemWatermark"
    int64_t delayedSeconds = 0;           // "delayedSeconds"
    int64_t imageExpireSeconds = 0;       // "imageExpireSeconds"
    int64_t imageExpireRemaining = 0;     // "imageExpireRemaining"
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

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SNAPSHOT_DEBUG_INFO_H
