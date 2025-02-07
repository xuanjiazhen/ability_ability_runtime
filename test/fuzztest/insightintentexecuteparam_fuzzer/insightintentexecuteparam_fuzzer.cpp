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

#include "insightintentexecuteparam_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "insight_intent_execute_param.h"
#include "securec.h"
#include "parcel.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    std::shared_ptr<InsightIntentExecuteParam> insightIntentExecuteParam =
        std::make_shared<InsightIntentExecuteParam>();
    Parcel parcel;
    parcel.WriteString(fdp->ConsumeRandomLengthString());
    insightIntentExecuteParam->ReadFromParcel(parcel);
    insightIntentExecuteParam->Marshalling(parcel);
    insightIntentExecuteParam->Unmarshalling(parcel);
    AAFwk::Want want;
    insightIntentExecuteParam->IsInsightIntentExecute(want);
    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> executeParam;
    insightIntentExecuteParam->GenerateFromWant(want, *executeParam);
    insightIntentExecuteParam->RemoveInsightIntent(want);
    WantParams wantParams;
    WantParams insightIntentParam;
    insightIntentExecuteParam->UpdateInsightIntentCallerInfo(wantParams, insightIntentParam);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}

