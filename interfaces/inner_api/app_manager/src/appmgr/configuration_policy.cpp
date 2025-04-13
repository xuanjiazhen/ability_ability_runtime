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

#include "configuration_policy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool ConfigurationPolicy::ReadFromParcel(Parcel &parcel)
{ 
    if (!parcel.ReadUint32(countPerTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "executionQuantity read int32 failed.");
        return false;
    }
    if (!parcel.ReadUint32(intervalTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "intervalTime read int32 failed.");
        return false;
    }

    return true;
}

ConfigurationPolicy *ConfigurationPolicy::Unmarshalling(Parcel &parcel)
{
    ConfigurationPolicy *policy = new ConfigurationPolicy();
    if (!policy->ReadFromParcel(parcel)) {
        delete policy;
        policy = nullptr;
    }
    return policy;
}

bool ConfigurationPolicy::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteUint32(countPerTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "executionQuantity [%{public}d] write int32 failed.", countPerTime);
        return false;
    }

    if (!parcel.WriteUint32(intervalTime)) {
        TAG_LOGE(AAFwkTag::APPMGR, "intervalTime [%{public}d] write int32 failed.", intervalTime);
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS