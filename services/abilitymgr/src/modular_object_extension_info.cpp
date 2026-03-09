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

#include "modular_object_extension_info.h"

namespace OHOS {
namespace AAFwk {
bool ModularObjectExtensionInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    abilityName = parcel.ReadString();
    appIndex = parcel.ReadInt32();
    launchMode = static_cast<LaunchMode>(parcel.ReadInt32());
    processMode = static_cast<ModularProcessMode>(parcel.ReadInt32());
    threadMode = static_cast<ThreadMode>(parcel.ReadInt32());
    isSelfDisabled = parcel.ReadBool();
    isSystemDisabled = parcel.ReadBool();
    return true;
}

bool ModularObjectExtensionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName)) {
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        return false;
    }
    if (!parcel.WriteString(abilityName)) {
        return false;
    }
    if (!parcel.WriteInt32(appIndex)) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(launchMode))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(processMode))) {
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(threadMode))) {
        return false;
    }
    if (!parcel.WriteBool(isSelfDisabled)) {
        return false;
    }
    if (!parcel.WriteBool(isSystemDisabled)) {
        return false;
    }
    return true;
}

ModularObjectExtensionInfo *ModularObjectExtensionInfo::Unmarshalling(Parcel &parcel)
{
    ModularObjectExtensionInfo *info = new (std::nothrow) ModularObjectExtensionInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        return nullptr;
    }
    return info;
}
} // namespace AAFwk
} // namespace OHOS