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

#include "sainterceptorstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "sa_interceptor_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.AbilityRuntime.ISAInterceptor";
}

class SAInterceptorStubFuzzTest : public SAInterceptorStub {
public:
    SAInterceptorStubFuzzTest() = default;
    virtual ~SAInterceptorStubFuzzTest()
    {}
    int32_t OnCheckStarting(const std::string &params, Rule &rule) override
    {
        return 0;
    }
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(ISAInterceptor::SAInterceptorCmd::ON_DO_CHECK_STARTING);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ISAInterceptor::SAInterceptorCmd::ON_DO_CHECK_STARTING);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<SAInterceptorStub> sainterceptorstub = std::make_shared<SAInterceptorStubFuzzTest>();
    sainterceptorstub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

/* Fuzzer entry point */
FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
