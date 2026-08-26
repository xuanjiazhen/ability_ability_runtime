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
#include "insightintentexecutecallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

#include "insight_intent_execute_callback_stub.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
class InsightIntentExecuteCallbackStubFuzz : public InsightIntentExecuteCallbackStub {
public:
    InsightIntentExecuteCallbackStubFuzz() = default;
    ~InsightIntentExecuteCallbackStubFuzz() = default;
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE);
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            InsightIntentExecuteResult executeResult;
            parcel.WriteParcelable(&executeResult);
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(InsightIntentExecuteCallbackStubFuzz, FuzzUtil::Tokens::INSIGHT_INTENT_EXEC_CB)
} // namespace OHOS
