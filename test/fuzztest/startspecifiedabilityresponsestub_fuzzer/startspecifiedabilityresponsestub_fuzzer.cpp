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
#include "attack_vectors.h"
#include "fuzz_util.h"
#include "startspecifiedabilityresponsestub_fuzzer.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
class StartSpecifiedAbilityResponseStubFuzz : public StartSpecifiedAbilityResponseStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 5) {
        case 0: {
            actualCode =
                static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_ACCEPT_WANT_RESPONSE);
            AAFwk::Want want;
            want.SetAction(FuzzUtil::BuildSpecialCharString(fdp));
            want.SetUri(FuzzUtil::BuildSpecialCharString(fdp));
            want.SetEntity(FuzzUtil::BuildSpecialCharString(fdp));
            want.SetBundle(FuzzUtil::BuildOversizedString(fdp, FuzzUtil::DEFAULT_MAX_STR_LEN,
                FuzzUtil::DEFAULT_MAX_STR_LEN));
            want.SetFlags(fdp.ConsumeIntegral<uint32_t>());
            parcel.WriteParcelable(&want);
            std::string flag = FuzzUtil::BuildSpecialCharString(fdp);
            parcel.WriteString16(std::u16string(flag.begin(), flag.end()));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 1: {
            actualCode =
                static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_TIMEOUT_RESPONSE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(
                IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_RESPONSE);
            std::string flag = FuzzUtil::BuildSpecialCharString(fdp);
            parcel.WriteString16(std::u16string(flag.begin(), flag.end()));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            std::string caller = FuzzUtil::BuildSpecialCharString(fdp);
            parcel.WriteString16(std::u16string(caller.begin(), caller.end()));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(
                IStartSpecifiedAbilityResponse::Message::ON_NEW_PROCESS_REQUEST_TIMEOUT_RESPONSE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode =
                static_cast<uint32_t>(IStartSpecifiedAbilityResponse::Message::ON_START_SPECIFIED_FAILED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(StartSpecifiedAbilityResponseStubFuzz, FuzzUtil::Tokens::START_SPECIFIED_RESPONSE)
} // namespace OHOS
