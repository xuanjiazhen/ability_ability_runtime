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
#include "windowmanagerservicehandlerstubremaining_fuzzer.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class WindowManagerServiceHandlerStubRemainingFuzz : public WindowManagerServiceHandlerStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 3) {
        case 0: {
            actualCode =
                static_cast<uint32_t>(IWindowManagerServiceHandler::WMSCmd::ON_COLD_STARTING_WINDOW);
            AbilityTransitionInfo info;
            parcel.WriteParcelable(&info);
            parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
            break;
        }
        case 1: {
            actualCode =
                static_cast<uint32_t>(IWindowManagerServiceHandler::WMSCmd::ON_HOT_STARTING_WINDOW);
            AbilityTransitionInfo info;
            parcel.WriteParcelable(&info);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(
                IWindowManagerServiceHandler::WMSCmd::ON_MOVE_MISSIONS_TO_BACKGROUND);
            parcel.WriteInt32Vector(FuzzUtil::BuildMaliciousInt32Vector(fdp));
            break;
        }
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(WindowManagerServiceHandlerStubRemainingFuzz, FuzzUtil::Tokens::WMS_HANDLER)
} // namespace OHOS
