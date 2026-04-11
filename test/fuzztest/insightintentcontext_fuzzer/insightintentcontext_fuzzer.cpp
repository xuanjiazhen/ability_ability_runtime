/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "insightintentcontext_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "insight_intent_context.h"
#include "ability_window_configuration.h"
#include "want.h"
#include "element_name.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 256;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (size < 10) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Generate random context parameters
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t winMode = fdp.ConsumeIntegral<int32_t>();
    uint64_t intentId = fdp.ConsumeIntegral<uint64_t>();

    // Create context
    sptr<IRemoteObject> token = nullptr;
    auto context = std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);

    // Test setter/getter for execute mode
    int32_t executeMode = fdp.ConsumeIntegral<int32_t>();
    context->SetExecuteMode(executeMode);
    auto retrievedExecuteMode = context->GetExecuteMode();

    // Test setter/getter for delay return mode
    auto delayReturnMode = static_cast<InsightIntentReturnMode>(fdp.ConsumeIntegral<int32_t>());
    context->SetDelayReturnMode(delayReturnMode);
    auto retrievedDelayMode = context->GetDelayReturnMode();

    // Test getter methods
    auto retrievedBundleName = context->GetBundleName();
    auto retrievedWinMode = context->GetCurrentWindowMode();
    auto retrievedIntentId = context->GetIntentId();

    // Generate random Want for StartAbilityByInsightIntent
    AAFwk::Want want;
    AppExecFwk::ElementName element;
    element.SetBundleName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    element.SetModuleName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    element.SetAbilityName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    want.SetElement(element);

    // Add random parameters to want
    int32_t paramCount = fdp.ConsumeIntegralInRange(0, 10);
    for (int32_t i = 0; i < paramCount; i++) {
        std::string key = "key_" + std::to_string(i);
        std::string value = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        want.SetParam(key, value);
    }

    // Test StartAbilityByInsightIntent with various wants
    context->StartAbilityByInsightIntent(want);

    // Test with want that has matching bundle name
    AAFwk::Want matchingWant;
    AppExecFwk::ElementName matchingElement;
    matchingElement.SetBundleName(bundleName);
    matchingElement.SetAbilityName("TestAbility");
    matchingWant.SetElement(matchingElement);
    context->StartAbilityByInsightIntent(matchingWant);

    // Test with empty want
    AAFwk::Want emptyWant;
    context->StartAbilityByInsightIntent(emptyWant);

    // Test edge cases for window mode
    int32_t windowModes[] = {
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY,
        AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY,
        -1, 0, 999, INT32_MIN, INT32_MAX
    };

    for (auto testWinMode : windowModes) {
        auto testContext = std::make_shared<InsightIntentContext>(token, bundleName, testWinMode, intentId);
        testContext->StartAbilityByInsightIntent(matchingWant);
    }

    // Test with extreme intentId values
    uint64_t extremeIntentIds[] = {0, 1, UINT64_MAX - 1, UINT64_MAX};
    for (auto testIntentId : extremeIntentIds) {
        auto testContext = std::make_shared<InsightIntentContext>(token, bundleName, winMode, testIntentId);
        testContext->GetIntentId();
    }

    // Test with all execute modes
    int32_t executeModes[] = {
        static_cast<int32_t>(InsightIntentExecuteMode::UIABILITY_FOREGROUND),
        static_cast<int32_t>(InsightIntentExecuteMode::UIABILITY_BACKGROUND),
        static_cast<int32_t>(InsightIntentExecuteMode::UIEXTENSION_ABILITY),
        static_cast<int32_t>(InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY),
        -1, 100, INT32_MIN, INT32_MAX
    };

    for (auto testExecMode : executeModes) {
        auto testContext = std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
        testContext->SetExecuteMode(testExecMode);
        testContext->GetExecuteMode();
    }

    // Test with all return modes
    int32_t returnModes[] = {
        static_cast<int32_t>(InsightIntentReturnMode::CALLBACK),
        static_cast<int32_t>(InsightIntentReturnMode::FUNCTION),
        -1, 100, INT32_MIN, INT32_MAX
    };

    for (auto testReturnMode : returnModes) {
        auto testContext = std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
        testContext->SetDelayReturnMode(static_cast<InsightIntentReturnMode>(testReturnMode));
        testContext->GetDelayReturnMode();
    }

    // Test with various bundle names
    std::vector<std::string> testBundleNames = {
        "",
        "a",
        std::string(1000, 'x'),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH)
    };

    for (auto testBundle : testBundleNames) {
        auto testContext = std::make_shared<InsightIntentContext>(token, testBundle, winMode, intentId);
        AAFwk::Want testWant;
        AppExecFwk::ElementName testElement;
        testElement.SetBundleName(testBundle);
        testWant.SetElement(testElement);
        testContext->StartAbilityByInsightIntent(testWant);
    }

    return true;
}

} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
