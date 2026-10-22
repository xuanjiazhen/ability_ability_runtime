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

#include "ui_extension/ui_extension_host_info.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
bool UIExtensionHostInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<AppExecFwk::ElementName> abilityInfo(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read hostInfo failed");
        return false;
    }

    elementName_ = *abilityInfo;
    sessionName = parcel.ReadString();
    if (!parcel.ReadInt32(persistentId_)) {
        persistentId_ = 0;
        TAG_LOGW(AAFwkTag::ABILITYMGR, "read persistentId failed, use default value");
    }
    if (!parcel.ReadString(moduleName)) {
        moduleName.clear();
        TAG_LOGW(AAFwkTag::ABILITYMGR, "read moduleName failed, use default value");
    }
    // Backfill: ElementName marshalling drops moduleName (known ability_base behavior),
    // so restore the saved moduleName into elementName_ to keep both carriers consistent.
    elementName_.SetModuleName(moduleName);
    return true;
}

UIExtensionHostInfo *UIExtensionHostInfo::Unmarshalling(Parcel &parcel)
{
    UIExtensionHostInfo *hostInfo = new (std::nothrow) UIExtensionHostInfo();
    if (hostInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create hostInfo failed");
        return nullptr;
    }

    if (!hostInfo->ReadFromParcel(parcel)) {
        delete hostInfo;
        hostInfo = nullptr;
    }

    return hostInfo;
}

bool UIExtensionHostInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&elementName_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write elementName failed");
        return false;
    }

    if (!parcel.WriteString(sessionName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write sessionName failed");
        return false;
    }

    if (!parcel.WriteInt32(persistentId_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write persistentId failed");
        return false;
    }

    // Read the original moduleName from elementName_ and carry it via the standalone slot;
    // ReadFromParcel saves it into moduleName and backfills it into elementName_.
    if (!parcel.WriteString(elementName_.GetModuleName())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write moduleName failed");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
