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
#include "statusbardelegatestub_fuzzer.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS;

namespace OHOS {
class StatusBarDelegateStubFuzzTest : public StatusBarDelegateStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::CHECK_IF_STATUS_BAR_ITEM_EXISTS);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteString(OHOS::FuzzUtil::BuildSpecialCharString(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::ATTACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        case 2:
            actualCode = static_cast<uint32_t>(StatusBarDelegateCmd::DETACH_PID_TO_STATUS_BAR_ITEM);
            parcel.WriteUint32(static_cast<uint32_t>(OHOS::FuzzUtil::BuildIntegerOverflow(fdp)));
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(OHOS::FuzzUtil::BuildSandboxEscapePath(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(StatusBarDelegateStubFuzzTest, FuzzUtil::Tokens::STATUS_BAR_DELEGATE)
} // namespace OHOS
