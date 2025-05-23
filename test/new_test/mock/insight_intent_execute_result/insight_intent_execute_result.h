/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_RESULT_H
#define MOCK_OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_RESULT_H

#include "parcel.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

struct InsightIntentExecuteResult : public Parcelable {
public:
    int32_t innerErr = 0;
    int32_t code = 0;
    int32_t flags = 0;
    std::shared_ptr<WantParams> result = nullptr;
    std::vector<std::string> uris;

    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }
    bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }
    static InsightIntentExecuteResult *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    static bool CheckResult(std::shared_ptr<const WantParams> result)
    {
        return false;
    }
};
} // namespace AppExecFwk
} // namespace OHOS
#endif
