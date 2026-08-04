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

#include "image_process_state_data.h"

#include "app_mgr_constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool ImageProcessStateData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteInt32(imagePid) && parcel.WriteUint64(checkpointId) && parcel.WriteInt32(originalPid)
        && parcel.WriteInt32(uid) && parcel.WriteInt32(state) && parcel.WriteString(bundleName)
        && parcel.WriteString(abilityName));
}

bool ImageProcessStateData::ReadFromParcel(Parcel &parcel)
{
    bool ret = parcel.ReadInt32(imagePid);
    ret = ret && parcel.ReadUint64(checkpointId);
    ret = ret && parcel.ReadInt32(originalPid);
    ret = ret && parcel.ReadInt32(uid);
    ret = ret && parcel.ReadInt32(state);
    ret = ret && parcel.ReadString(bundleName);
    ret = ret && parcel.ReadString(abilityName);
    if (!ret) {
        TAG_LOGW(AAFwkTag::APPMGR, "ReadFromParcel failed");
        return false;
    }
    return true;
}

ImageProcessStateData *ImageProcessStateData::Unmarshalling(Parcel &parcel)
{
    ImageProcessStateData *imageProcessStateData = new (std::nothrow) ImageProcessStateData();
    if (imageProcessStateData && !imageProcessStateData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "imageProcessStateData failed, because ReadFromParcel failed");
        delete imageProcessStateData;
        imageProcessStateData = nullptr;
    }
    return imageProcessStateData;
}
}  // namespace AppExecFwk
}  // namespace OHOS