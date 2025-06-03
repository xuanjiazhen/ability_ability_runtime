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

#ifndef OHOS_ABILITY_RUNTIME_DMS_CONTINUEINFO_INFO_H
#define OHOS_ABILITY_RUNTIME_DMS_CONTINUEINFO_INFO_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
/**
 * @struct AutoStartupInfo
 * Defines auto startup info.
 */
struct ContinueMissionInfo : public Parcelable{
    std::string dstDeviceId;
    std::string srcDeviceId;
    std::string bundleName;
    std::string srcBundleName;
    std::string continueType;
    AAFwk::WantParams wantParams;

    bool Marshalling(Parcel &parcel) const override {
        if (!parcel.WriteString(dstDeviceId)) {
            // TAG_LOGW("Write isOpenAllowed failed");
            return false;
        }
        if (!parcel.WriteString(srcDeviceId)) {
            // TAG_LOGW("Write isEmbeddedAllowed failed");
            return false;
        }
        return true;
    }

    bool ReadFromParcel(Parcel &parcel)
    {
        dstDeviceId = parcel.ReadString();
        srcDeviceId = parcel.ReadString();
        bundleName = parcel.ReadString();
        srcBundleName = parcel.ReadString();
        continueType = parcel.ReadString();
        auto tempWantParams = parcel.ReadParcelable<AAFwk::WantParams>();
        if (tempWantParams == nullptr) {
            return false;
        }
        wantParams = *tempWantParams;
        delete tempWantParams;
        tempWantParams = nullptr;
        return true;
    }

    static ContinueMissionInfo *Unmarshalling(Parcel &parcel) {
        ContinueMissionInfo* continueMissionInfo = new (std::nothrow) ContinueMissionInfo();
        if (continueMissionInfo == nullptr) {
            return nullptr;
        }

        if (!continueMissionInfo->ReadFromParcel(parcel)) {
            delete continueMissionInfo;
            continueMissionInfo = nullptr;
        }
        return continueMissionInfo;
    }

};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DMS_CONTINUEINFO_INFO_H