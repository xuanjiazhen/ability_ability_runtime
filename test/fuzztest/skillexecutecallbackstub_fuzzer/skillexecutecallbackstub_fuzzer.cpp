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

#include "skillexecutecallbackstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "parcel.h"
#include "securec.h"
#include "skill/skill_execute_callback_stub.h"
#include "skill/skill_execute_result.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.AAFwk.SkillExecuteCallback";
}

class SkillExecuteCallbackStubFuzz : public SkillExecuteCallbackStub {
public:
    SkillExecuteCallbackStubFuzz() = default;
    ~SkillExecuteCallbackStubFuzz() = default;

    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override {}
};

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 1) {
        case 0:
            actualCode = static_cast<uint32_t>(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE);
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp, OHOS::FuzzUtil::DEFAULT_MAX_STR_LEN));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            {
                AppExecFwk::SkillExecuteResult result;
                parcel.WriteParcelable(result);
            }
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<SkillExecuteCallbackStub> stub = std::make_shared<SkillExecuteCallbackStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
