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

#include "agentabilityutil_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#include "agent_ability_util.h"

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
    want.SetParam(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

    AppExecFwk::AbilityInfo abilityInfo;
    AbilityFuzzUtil::GenerateAbilityInfo(fdp, abilityInfo);

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.type = static_cast<AppExecFwk::ExtensionAbilityType>(
        fdp.ConsumeIntegralInRange<int32_t>(0, 25));
    extensionInfo.applicationInfo.bundleType = static_cast<AppExecFwk::BundleType>(
        fdp.ConsumeIntegralInRange<int32_t>(0, 3));

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, 4);
    for (size_t i = 0; i < count; ++i) {
        AppExecFwk::ExtensionAbilityInfo info;
        info.type = static_cast<AppExecFwk::ExtensionAbilityType>(
            fdp.ConsumeIntegralInRange<int32_t>(0, 25));
        info.applicationInfo.bundleType = static_cast<AppExecFwk::BundleType>(
            fdp.ConsumeIntegralInRange<int32_t>(0, 3));
        extensionInfos.emplace_back(info);
    }

    auto extensionType = static_cast<AppExecFwk::ExtensionAbilityType>(
        fdp.ConsumeIntegralInRange<int32_t>(0, 25));
    int64_t nonce = fdp.ConsumeIntegral<int64_t>();

    (void)AgentAbilityUtil::IsAgentExtensionType(extensionType);
    (void)AgentAbilityUtil::IsAgentExtensionAbilityInfo(abilityInfo);
    (void)AgentAbilityUtil::IsAgentExtensionInfo(extensionInfo);
    (void)AgentAbilityUtil::IsAtomicServiceAgentExtensionInfo(extensionInfo);
    (void)AgentAbilityUtil::HasAtomicServiceAgentExtensionInfo(extensionInfos);
    (void)AgentAbilityUtil::HasAgentOnlyParams(want);
    AgentAbilityUtil::SetAgentVerificationNonceParam(want, nonce);
    (void)AgentAbilityUtil::GetAgentVerificationNonceParam(want);
    (void)AgentAbilityUtil::CheckAgentConnectEntry(want, extensionType);
    (void)AgentAbilityUtil::CheckConnectAgentResolvedTarget(extensionType, abilityInfo);

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
