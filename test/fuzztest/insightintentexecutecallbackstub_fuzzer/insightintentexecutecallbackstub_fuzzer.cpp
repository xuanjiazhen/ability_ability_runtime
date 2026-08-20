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
#include "fuzz_util.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "insight_intent_execute_callback_stub.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
const std::u16string APPMGR_INTERFACE_TOKEN = u"ohos.aafwk.AppManager";

class InsightIntentExecuteCallbackStubFuzzTest : public InsightIntentExecuteCallbackStub {
public:
    InsightIntentExecuteCallbackStubFuzzTest() = default;
    virtual ~InsightIntentExecuteCallbackStubFuzzTest()
    {}
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override {}
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    std::shared_ptr<InsightIntentExecuteCallbackStub> backStub =
        std::make_shared<InsightIntentExecuteCallbackStubFuzzTest>();
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;
    dataParcel.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);
    dataParcel.WriteBuffer(data, size);
    dataParcel.RewindRead(0);
    backStub->OnRemoteRequest(code, dataParcel, reply, option);
    backStub->OnExecuteDoneInner(dataParcel, reply);

    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
