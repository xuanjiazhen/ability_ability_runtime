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

#include "abilityinterfacesappmanageramsmgrstubfirst_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ams_mgr_stub.h"
#include "ams_mgr_stub_mock.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

class AmsMgrStubFirstFuzz : public AmsMgrStubFuzzBase {};

void DoFuzzCases(uint32_t code, MessageParcel &parcel, FuzzedDataProvider &fdp, uint32_t &actualCode)
{
    actualCode = static_cast<uint32_t>(IAmsMgr::Message::GET_RUNNING_PROCESS_INFO_BY_TOKEN);
    parcel.WriteString(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
}

FUZZ_STUB_ENTRY_IMPL(AmsMgrStubFirstFuzz, FuzzUtil::Tokens::AMS_MGR)
} // namespace OHOS
