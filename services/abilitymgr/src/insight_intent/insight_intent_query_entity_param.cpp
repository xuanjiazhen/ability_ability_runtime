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

#include "insight_intent_query_param.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

bool InsightIntentQueryParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    intentName_ = Str16ToStr8(parcel.ReadString16());
    className_ = Str16ToStr8(parcel.ReadString16());
    queryEntityParam_.queryType_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams != nullptr) {
        queryEntityParam_.parameters_ = wantParams;
    }
    userId_ = parcel.ReadInt32();
    intentId_ = parcel.ReadUint64();
    return true;
}

InsightIntentQueryParam *InsightIntentQueryParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentQueryParam *param = new (std::nothrow) InsightIntentQueryParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentQueryParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(intentName_));
    parcel.WriteString16(Str8ToStr16(className_));
    parcel.WriteString16(Str8ToStr16(queryEntityParam_.queryType_));
    parcel.WriteParcelable(queryEntityParam_.parameters_.get());
    parcel.WriteInt32(userId_);
    parcel.WriteUint64(intentId_);
    return true;
}

} // namespace AppExecFwk
} // namespace OHOS
