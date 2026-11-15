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

#include "updatecallerinfoutilsecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#define private public
#include "update_caller_info_util.h"
#undef private
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    ElementName elementName;
    AbilityFuzzUtil::GenerateElementName(fdp, elementName);
    Want want;
    want.SetElement(elementName);
    want.SetParam("ohos.insightIntent." + fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    want.SetParam("ohos.skill.executeParam." + fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

    UpdateCallerInfoUtil &util = UpdateCallerInfoUtil::GetInstance();

    auto indirectCallerInfo = std::make_shared<IndirectCallerInfo>();
    indirectCallerInfo->tokenId = fdp.ConsumeIntegral<uint32_t>();
    indirectCallerInfo->callerUid = fdp.ConsumeIntegral<int32_t>();
    indirectCallerInfo->callerPid = fdp.ConsumeIntegral<int32_t>();
    int32_t tokenId = 0;
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    util.UpdateByIndirectCallerInfo(indirectCallerInfo, tokenId, callerUid, callerPid);

    uint32_t callerTokenId = fdp.ConsumeIntegral<uint32_t>();
    util.UpdateCallerInfoByHapTokenId(want, callerTokenId);

    AbilitySessionInfo info;
    info.callerBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.callerTokenId = fdp.ConsumeIntegral<uint32_t>();
    info.callerNativeName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.hasCallerNativeName = fdp.ConsumeBool();
    info.callerAbilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.callerAppId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.callerAppIdentifier = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.taskId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    util.CacheCallerInfoFromWant(want, info);
    util.RestoreCallerInfoToWant(want, info);
    util.ClearCallerIdentityParam(want);
    util.CacheIntentAndSkillParams(want, info);
    util.RemoveIntentAndSkillParams(want);
    util.RestoreIntentAndSkillParams(want, info);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
