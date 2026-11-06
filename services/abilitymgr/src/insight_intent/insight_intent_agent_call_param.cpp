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

#include "insight_intent_agent_call_param.h"

#include "message_parcel.h"

namespace OHOS {
namespace AppExecFwk {
bool InsightIntentAgentCallParam::ReadFromParcel(Parcel &parcel)
{
    key = parcel.ReadUint64();
    callerToken = (static_cast<MessageParcel*>(&parcel))->ReadRemoteObject();
    bundleName = parcel.ReadString();
    intentName = parcel.ReadString();
    std::unique_ptr<WantParams> wp(parcel.ReadParcelable<WantParams>());
    if (wp == nullptr) {
        return false;
    }
    wantParam = *wp;
    return true;
}

InsightIntentAgentCallParam *InsightIntentAgentCallParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentAgentCallParam *param = new (std::nothrow) InsightIntentAgentCallParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentAgentCallParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint64(key);
    (static_cast<MessageParcel*>(&parcel))->WriteRemoteObject(callerToken);
    parcel.WriteString(bundleName);
    parcel.WriteString(intentName);
    parcel.WriteParcelable(&wantParam);
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
