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

#include "abilitymgrecologicalruleinterceptorthird_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ecological_rule_interceptor.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LEN = 64;

sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.name = "FuzzAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord != nullptr) {
        token = abilityRecord->GetToken();
    }
    return token;
}

Want BuildWant(FuzzedDataProvider &fdp)
{
    Want want;
    std::string callerBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundle);
    std::string targetBundle = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    std::string targetAbility = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    ElementName element("", targetBundle, targetAbility);
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, fdp.ConsumeIntegral<int32_t>());
    want.SetParam(Want::PARAM_RESV_CALLER_PID, fdp.ConsumeIntegral<int32_t>());
    return want;
}

void DoProcessParam(EcologicalRuleInterceptor &interceptor, FuzzedDataProvider &fdp)
{
    AbilityInterceptorParam param;
    Want want = BuildWant(fdp);
    int32_t requestCode = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool isWithUI = fdp.ConsumeBool();
    bool isVisible = fdp.ConsumeBool();
    bool isStartAsCaller = fdp.ConsumeBool();
    bool isTargetPlugin = fdp.ConsumeBool();
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    std::string hostBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);

    // Remote dispatch context: DoProcess defers to the remote device immediately.
    param = InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).Visible(isVisible)
        .CallerToken(token).Context<AbilityInterceptorParam::RemoteDispatchCtx>({}).Build();
    interceptor.DoProcess(param);

    // Null abilityInfo: early skip path.
    param = InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).Visible(isVisible)
        .CallerToken(token)
        .Context<AbilityInterceptorParam::EcologicalCtx>({isStartAsCaller, isTargetPlugin, hostBundleName})
        .Build();
    interceptor.DoProcess(param);

    // Resolved abilityInfo: full processing path.
    auto abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->name = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->isStageBasedModel = fdp.ConsumeBool();
    abilityInfo->applicationInfo.bundleType =
        static_cast<BundleType>(fdp.ConsumeIntegralInRange<int32_t>(0, 3));
    abilityInfo->applicationInfo.appDistributionType = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.appProvisionType = fdp.ConsumeRandomLengthString(STRING_MAX_LEN);
    abilityInfo->applicationInfo.applicationReservedFlag = fdp.ConsumeIntegral<int32_t>();
    param = InterceptorParamBuilder(want, requestCode, userId).WithUI(isWithUI).Visible(isVisible)
        .CallerToken(token).AbilityInfo(abilityInfo)
        .Context<AbilityInterceptorParam::EcologicalCtx>({isStartAsCaller, isTargetPlugin, hostBundleName})
        .Build();
    interceptor.DoProcess(param);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < 4) {
        return false;
    }
    FuzzedDataProvider fdp(data, size);
    auto interceptor = std::make_shared<EcologicalRuleInterceptor>();
    if (interceptor == nullptr) {
        return false;
    }
    DoProcessParam(*interceptor, fdp);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}