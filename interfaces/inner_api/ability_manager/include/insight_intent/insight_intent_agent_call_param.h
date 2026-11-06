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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AGENT_CALL_PARAM_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AGENT_CALL_PARAM_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

/**
 * @struct InsightIntentAgentCallParam
 * IPC carrier for the third-party agent invoke entry (ExecuteIntentByAgent).
 * Bundles the callback key, caller token, target intent identity and execution
 * arguments into a single Parcelable so the IPC signature stays one parameter.
 */
class InsightIntentAgentCallParam : public Parcelable {
public:
    InsightIntentAgentCallParam() = default;
    ~InsightIntentAgentCallParam() override = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static InsightIntentAgentCallParam *Unmarshalling(Parcel &parcel);

    uint64_t key = 0;
    sptr<IRemoteObject> callerToken;
    std::string bundleName;
    std::string intentName;
    WantParams wantParam;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_AGENT_CALL_PARAM_H
