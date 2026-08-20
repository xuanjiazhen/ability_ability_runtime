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
#include "atomicservicestatuscallbackstub_fuzzer.h"
#include "attack_vectors.h"
#include "fuzz_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
class AtomicServiceStatusCallbackStubFuzz : public AtomicServiceStatusCallbackStub {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    switch (code % 2) {
        case 0:
            actualCode = static_cast<uint32_t>(IAtomicServiceStatusCallback::ON_FREE_INSTALL_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IAtomicServiceStatusCallback::ON_REMOTE_FREE_INSTALL_DONE);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(OHOS::FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        default:
            break;
    }
}

FUZZ_STUB_ENTRY_IMPL(AtomicServiceStatusCallbackStubFuzz, FuzzUtil::Tokens::ATOMIC_SERVICE_CB)
} // namespace OHOS
