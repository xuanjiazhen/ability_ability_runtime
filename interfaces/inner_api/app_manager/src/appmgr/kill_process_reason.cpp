/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "kill_process_reason.h"

#include "hilog_tag_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
bool KillProcessReason::ReadFromParcel(Parcel &parcel)
{
    int32_t reason = 0;
    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    killReason = static_cast<KillReason>(reason);

    if (!parcel.ReadInt32(reason)) {
        return false;
    }
    subKillReason = reason;

    killMsg = Str16ToStr8(parcel.ReadString16());

    return true;
}

KillProcessReason *KillProcessReason::Unmarshalling(Parcel &parcel)
{
    KillProcessReason *reason = new (std::nothrow) KillProcessReason();
    if (reason == nullptr) {
        return nullptr;
    }

    if (!reason->ReadFromParcel(parcel)) {
        delete reason;
        reason = nullptr;
    }
    return reason;
}

bool KillProcessReason::Marshalling(Parcel &parcel) const
{
    // write killReason
    if (!parcel.WriteInt32(static_cast<int32_t>(killReason))) {
        return false;
    }
    // write subKillReason
    if (!parcel.WriteInt32(static_cast<int32_t>(subKillReason))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(killMsg))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write lastExitMessage failed");
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
