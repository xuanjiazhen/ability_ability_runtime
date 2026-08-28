/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "usercallbackstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#define private public
#include "user_callback_stub.h"
#undef private

#include "ability_record.h"
#include "fuzz_util.h"

using namespace OHOS::AAFwk;

namespace OHOS {
class UserCallbackStubFuzz : public UserCallbackStub {
public:
    UserCallbackStubFuzz() = default;
    ~UserCallbackStubFuzz() = default;
    void OnUserCmdDone(int userId, int errcode) override {}
};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 1) {
        case 0: {
            actualCode = static_cast<uint32_t>(IUserCallback::ON_USER_CMD_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(UserCallbackStubFuzz, FuzzUtil::Tokens::USER_CALLBACK)
} // namespace OHOS
