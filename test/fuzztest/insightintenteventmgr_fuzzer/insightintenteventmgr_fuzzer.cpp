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

#include "insightintenteventmgr_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "insight_intent_event_mgr.h"
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

    // Generate random ElementName
    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    elementName.SetModuleName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    elementName.SetAbilityName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

    // Generate random userId
    int32_t userId = fdp.ConsumeIntegral<int32_t>();

    // Generate random appIndex
    int32_t appIndex = fdp.ConsumeIntegral<int32_t>();

    auto eventMgr = std::make_shared<InsightIntentEventMgr>();

    // Test UpdateInsightIntentEvent
    eventMgr->UpdateInsightIntentEvent(elementName, userId);

    // Test DeleteInsightIntentEvent
    eventMgr->DeleteInsightIntentEvent(elementName, userId, appIndex);

    // Test with empty strings
    AppExecFwk::ElementName emptyElementName;
    eventMgr->UpdateInsightIntentEvent(emptyElementName, userId);
    eventMgr->DeleteInsightIntentEvent(emptyElementName, userId, appIndex);

    // Test with extreme userId values
    int32_t extremeUserIds[] = {INT32_MIN, INT32_MAX, -1, 0, 100, 999999};
    for (auto testUserId : extremeUserIds) {
        eventMgr->UpdateInsightIntentEvent(elementName, testUserId);
        eventMgr->DeleteInsightIntentEvent(elementName, testUserId, appIndex);
    }

    // Test with extreme appIndex values
    int32_t extremeAppIndexes[] = {INT32_MIN, INT32_MAX, -1, 0, 1, 100};
    for (auto testAppIndex : extremeAppIndexes) {
        eventMgr->DeleteInsightIntentEvent(elementName, userId, testAppIndex);
    }

    // Test with various bundle name lengths
    std::vector<std::string> bundleNameLengths = {
        "",
        "a",
        "test.bundle",
        std::string(100, 'a'),
        std::string(500, 'b'),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH)
    };

    for (auto testBundleName : bundleNameLengths) {
        AppExecFwk::ElementName testElement;
        testElement.SetBundleName(testBundleName);
        testElement.SetModuleName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
        testElement.SetAbilityName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

        eventMgr->UpdateInsightIntentEvent(testElement, userId);
        eventMgr->DeleteInsightIntentEvent(testElement, userId, appIndex);
    }

    // Test with special characters in bundle names
    std::vector<std::string> specialBundleNames = {
        "../test.bundle",
        "../../../etc/passwd",
        "test\x00.bundle",
        "test\r\n.bundle",
        "test\t.bundle",
        "test<script>.bundle",
        "test;bundle",
        "$(whoami)",
        "`id`",
        "${HOME}",
        "%PATH%"
    };

    for (auto testBundleName : specialBundleNames) {
        AppExecFwk::ElementName testElement;
        testElement.SetBundleName(testBundleName);
        testElement.SetModuleName("entry");
        testElement.SetAbilityName("TestAbility");

        eventMgr->UpdateInsightIntentEvent(testElement, userId);
        eventMgr->DeleteInsightIntentEvent(testElement, userId, appIndex);
    }

    // Test sequential updates and deletes
    for (int32_t i = 0; i < 50; i++) {
        AppExecFwk::ElementName testElement;
        testElement.SetBundleName("test.bundle" + std::to_string(i));
        testElement.SetModuleName("entry");
        testElement.SetAbilityName("TestAbility");

        eventMgr->UpdateInsightIntentEvent(testElement, userId + i);
        eventMgr->DeleteInsightIntentEvent(testElement, userId + i, i % 5);
    }

    // Test with null-like strings
    AppExecFwk::ElementName nullLikeElement;
    nullLikeElement.SetBundleName(std::string(1, '\0'));
    nullLikeElement.SetModuleName(std::string(1, '\0'));
    nullLikeElement.SetAbilityName(std::string(1, '\0'));
    eventMgr->UpdateInsightIntentEvent(nullLikeElement, userId);
    eventMgr->DeleteInsightIntentEvent(nullLikeElement, userId, appIndex);

    // Test with unicode characters
    AppExecFwk::ElementName unicodeElement;
    unicodeElement.SetBundleName("测试.bundle.name");
    unicodeElement.SetModuleName("模块");
    unicodeElement.SetAbilityName("能力");
    eventMgr->UpdateInsightIntentEvent(unicodeElement, userId);
    eventMgr->DeleteInsightIntentEvent(unicodeElement, userId, appIndex);

    // Test negative scenario combinations
    int32_t negativeCombinations[][3] = {
        {-1, -1, -1},
        {INT32_MIN, INT32_MIN, INT32_MIN},
        {INT32_MAX, INT32_MAX, INT32_MAX},
        {0, 0, 0},
        {100, -50, 200}
    };

    for (auto combo : negativeCombinations) {
        AppExecFwk::ElementName testElement;
        testElement.SetBundleName("test.bundle");
        testElement.SetModuleName("entry");
        testElement.SetAbilityName("TestAbility");

        eventMgr->UpdateInsightIntentEvent(testElement, combo[0]);
        eventMgr->DeleteInsightIntentEvent(testElement, combo[1], combo[2]);
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
