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
    // The write/read order matches the declared field order and must not
    // change, or the IPC wire format breaks.
    return parcel.WriteString(bundleName) && parcel.WriteInt32(uid) &&
        parcel.WriteInt32(imagePid) && parcel.WriteString(state) &&
        parcel.WriteInt32(isEnable);
}

bool SnapshotDebugAppInfo::ReadFromParcel(Parcel &parcel)
{
    return parcel.ReadString(bundleName) && parcel.ReadInt32(uid) &&
        parcel.ReadInt32(imagePid) && parcel.ReadString(state) &&
        parcel.ReadInt32(isEnable);
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

} // namespace AppExecFwk
} // namespace OHOS
