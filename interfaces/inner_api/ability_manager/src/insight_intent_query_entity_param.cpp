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

#include "insight_intent/insight_intent_query_entity_param.h"

#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
bool InsightIntentQueryEntityParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName_)) {
        return false;
    }
    if (!parcel.WriteString(moduleName_)) {
        return false;
    }
    if (!parcel.WriteString(intentName_)) {
        return false;
    }
    if (!parcel.WriteString(className_)) {
        return false;
    }
    if (!parcel.WriteParcelable(queryEntityParam_.get())) {
        return false;
    }
    if (!parcel.WriteInt32(userId_)) {
        return false;
    }
    if (!parcel.WriteUint64(intentId_)) {
        return false;
    }
    return true;
}

InsightIntentQueryEntityParam *InsightIntentQueryEntityParam::Unmarshalling(Parcel &parcel)
{
    auto param = std::make_unique<InsightIntentQueryEntityParam>();
    if (param == nullptr) {
        return nullptr;
    }

    param->bundleName_ = parcel.ReadString();
    param->moduleName_ = parcel.ReadString();
    param->intentName_ = parcel.ReadString();
    param->className_ = parcel.ReadString();
    param->queryEntityParam_ = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    param->userId_ = parcel.ReadInt32();
    param->intentId_ = parcel.ReadUint64();
    return param.release();
}

bool InsightIntentQueryEntityParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = parcel.ReadString();
    moduleName_ = parcel.ReadString();
    intentName_ = parcel.ReadString();
    className_ = parcel.ReadString();
    queryEntityParam_ = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    userId_ = parcel.ReadInt32();
    intentId_ = parcel.ReadUint64();
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
