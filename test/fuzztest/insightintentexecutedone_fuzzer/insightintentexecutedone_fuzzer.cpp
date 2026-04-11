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

#include "insightintentexecutedone_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_result.h"
#include "iremote_object.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;

// Mock IRemoteObject for testing
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"MockIRemoteObject") {}
    ~MockIRemoteObject() override = default;
};
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (size < 10) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();

    // Create a test record first
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.moduleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.abilityName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    param.insightIntentName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = fdp.ConsumeIntegral<uint64_t>();
    std::string callerBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, false);
    uint64_t intentId = param.insightIntentId_;

    // Test ExecuteIntentDone with various result codes
    int32_t resultCodes[] = {
        0, -1, 1, 100, -100,
        ERR_OK, ERR_INVALID_VALUE, ERR_NO_INIT,
        INT32_MIN, INT32_MAX
    };

    for (auto resultCode : resultCodes) {
        AppExecFwk::InsightIntentExecuteResult result;
        result.resultCode = resultCode;

        // Add random result data
        int32_t dataCount = fdp.ConsumeIntegralInRange(0, 5);
        for (int32_t i = 0; i < dataCount; i++) {
            std::string dataKey = "key_" + std::to_string(i);
            std::string dataValue = fdp.ConsumeRandomLengthString(64);
            result.resultData[dataKey] = dataValue;
        }

        manager->ExecuteIntentDone(intentId, resultCode, result);
    }

    // Test with invalid intentId values
    uint64_t invalidIntentIds[] = {0, 1, UINT64_MAX, 99999, 88888};
    for (auto invalidId : invalidIntentIds) {
        AppExecFwk::InsightIntentExecuteResult result;
        manager->ExecuteIntentDone(invalidId, 0, result);
    }

    // Test RemoteDied
    manager->RemoteDied(intentId);

    for (auto invalidId : invalidIntentIds) {
        manager->RemoteDied(invalidId);
    }

    // Test GetBundleName
    std::string bundleName;
    manager->GetBundleName(intentId, bundleName);

    for (auto invalidId : invalidIntentIds) {
        std::string testBundleName;
        manager->GetBundleName(invalidId, testBundleName);
    }

    // Test GetCallerBundleName
    std::string retrievedCallerBundleName;
    manager->GetCallerBundleName(intentId, retrievedCallerBundleName);

    for (auto invalidId : invalidIntentIds) {
        std::string testCallerBundleName;
        manager->GetCallerBundleName(invalidId, testCallerBundleName);
    }

    // Test exemption info
    int32_t testUids[] = {0, 100, 1000, 99999, -1, INT32_MIN, INT32_MAX};
    for (auto uid : testUids) {
        manager->SetIntentExemptionInfo(uid);
        bool isExemption = manager->CheckIntentIsExemption(uid);
    }

    // Test GetAllIntentExemptionInfo
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();

    // Test OnInsightAppDied
    std::vector<std::string> testBundleNames = {
        "",
        "test.bundle",
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        std::string(500, 'x'),
        "../etc/passwd",
        "$(whoami)",
        "<script>alert(1)</script>",
        "'; DROP TABLE users; --"
    };
    for (auto bundle : testBundleNames) {
        manager->OnInsightAppDied(bundle);
    }

    // Test multiple ExecuteIntentDone calls with different result structures
    AppExecFwk::InsightIntentExecuteResult complexResult;
    complexResult.resultCode = fdp.ConsumeIntegral<int32_t>();

    // Add many result data entries
    int32_t complexDataCount = fdp.ConsumeIntegralInRange(0, 100);
    for (int32_t i = 0; i < complexDataCount; i++) {
        std::string key = fdp.ConsumeRandomLengthString(64);
        std::string value = fdp.ConsumeRandomLengthString(256);
        complexResult.resultData[key] = value;
    }
    manager->ExecuteIntentDone(intentId, complexResult.resultCode, complexResult);

    // Test with special characters in result data
    AppExecFwk::InsightIntentExecuteResult specialResult;
    specialResult.resultData["null"] = "\x00";
    specialResult.resultData["newline"] = "\n\r";
    specialResult.resultData["tab"] = "\t";
    specialResult.resultData["unicode"] = "测试中文";
    specialResult.resultData["emoji"] = "😀🎉";
    specialResult.resultData["sql"] = "'; DROP TABLE; --";
    specialResult.resultData["xss"] = "<script>alert(1)</script>";
    manager->ExecuteIntentDone(intentId, 0, specialResult);

    // Test edge case: very long bundle names in result data
    AppExecFwk::InsightIntentExecuteResult longResult;
    longResult.resultData["long_key"] = std::string(10000, 'L');
    longResult.resultData[std::string(10000, 'K')] = "long_value";
    manager->ExecuteIntentDone(intentId, 0, longResult);

    // Test rapid sequence of operations
    for (int i = 0; i < 100; i++) {
        AppExecFwk::InsightIntentExecuteParam rapidParam;
        rapidParam.bundleName_ = "rapid.bundle" + std::to_string(i);
        rapidParam.moduleName_ = "entry";
        rapidParam.abilityName_ = "RapidAbility";
        rapidParam.insightIntentName_ = "RapidIntent";

        auto rapidParamPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(rapidParam);
        sptr<IRemoteObject> rapidToken = new MockIRemoteObject();
        uint64_t rapidKey = 3000 + i;

        manager->CheckAndUpdateParam(rapidKey, rapidToken, rapidParamPtr, "caller.bundle", false);
        uint64_t rapidIntentId = rapidParam.insightIntentId_;

        AppExecFwk::InsightIntentExecuteResult rapidResult;
        rapidResult.resultCode = i % 2 == 0 ? 0 : -1;
        manager->ExecuteIntentDone(rapidIntentId, rapidResult.resultCode, rapidResult);
        manager->RemoteDied(rapidIntentId);
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
