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

#include "insight_intent_query_entity_param.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

bool InsightIntentQueryEntityParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    intentName_ = Str16ToStr8(parcel.ReadString16());
    className_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        return false;
    }
    queryEntityParam_ = wantParams;
    userId_ = parcel.ReadInt32();
    intentId_ = parcel.ReadUint64();
    return true;
}

InsightIntentQueryEntityParam *InsightIntentQueryEntityParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentQueryEntityParam *param = new (std::nothrow) InsightIntentQueryEntityParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentQueryEntityParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(intentName_));
    parcel.WriteString16(Str8ToStr16(className_));
    parcel.WriteParcelable(queryEntityParam_.get());
    parcel.WriteInt32(userId_);
    parcel.WriteUint64(intentId_);
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS
