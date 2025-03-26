/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "policy_data.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool ExecutionPolicy::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadUint32(executionQuantity)) {
        TAG_LOGE(AAFwkTag::APPMGR, "executionQuantity read int32 failed.");
        return false;
    }
    if (!parcel.ReadUint32(intervalTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "intervalTime read int32 failed.");
        return false;
    }

    return true;
}

ExecutionPolicy *ExecutionPolicy::Unmarshalling(Parcel &parcel)
{
    ExecutionPolicy *policy = new ExecutionPolicy();
    if (!policy->ReadFromParcel(parcel)) {
        delete policy;
        policy = nullptr;
    }
    return policy;
}

bool ExecutionPolicy::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(executionQuantity)) {
        TAG_LOGE(AAFwkTag::APPMGR, "executionQuantity [%{public}d] write int32 failed.", executionQuantity);
        return false;
    }

    if (!parcel.WriteInt32(intervalTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "intervalTime [%{public}d] write int32 failed.", intervalTime);
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS