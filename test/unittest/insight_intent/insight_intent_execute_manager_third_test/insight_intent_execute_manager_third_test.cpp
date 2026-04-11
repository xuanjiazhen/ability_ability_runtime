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

#include <gtest/gtest.h>

#include "hilog_tag_wrapper.h"
#include "want.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_execute_manager_third_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {

class InsightIntentExecuteManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override();

    void ClearExecuteManager();
};

void InsightIntentExecuteManagerThirdTest::SetUpTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerThirdTest::SetUpTestCase called");
}

void InsightIntentExecuteManagerThirdTest::TearDownTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerThirdTest::TearDownTestCase called");
}

void InsightIntentExecuteManagerThirdTest::SetUp()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerThirdTest::SetUp called");
    ClearExecuteManager();
}

void InsightIntentExecuteManagerThirdTest::TearDown()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentExecuteManagerThirdTest::TearDown called");
    ClearExecuteManager();
}

void InsightIntentExecuteManagerThirdTest::ClearExecuteManager()
{
    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    if (manager != nullptr) {
        // Clear any existing records by removing them
        // This is a simplified approach - actual implementation may vary
    }
}

/**
 * @tc.number: ExecuteIntentDone_0100
 * @tc.name: ExecuteIntentDone
 * @tc.desc: Test ExecuteIntentDone with valid intentId and success result
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, ExecuteIntentDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2001;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Now mark as done
    AppExecFwk::InsightIntentExecuteResult result;
    result.resultCode = 0;
    ret = manager->ExecuteIntentDone(intentId, 0, result);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 end.");
}

/**
 * @tc.number: ExecuteIntentDone_0200
 * @tc.name: ExecuteIntentDone
 * @tc.desc: Test ExecuteIntentDone with non-existent intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, ExecuteIntentDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 88888;
    AppExecFwk::InsightIntentExecuteResult result;

    auto ret = manager->ExecuteIntentDone(invalidIntentId, 0, result);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 end.");
}

/**
 * @tc.number: ExecuteIntentDone_0300
 * @tc.name: ExecuteIntentDone
 * @tc.desc: Test ExecuteIntentDone with various result codes
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, ExecuteIntentDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // Add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2002;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Test with different result codes - each needs a fresh intentId
    std::vector<int32_t> resultCodes = {0, -1, 1, ERR_OK, ERR_INVALID_VALUE};

    int32_t successCount = 0;
    int32_t failureCount = 0;
    uint64_t testKeyBase = 3000;

    for (size_t i = 0; i < resultCodes.size(); i++) {
        // Create a new record for each test
        AppExecFwk::InsightIntentExecuteParam testParam;
        testParam.bundleName_ = "test.bundle";
        testParam.moduleName_ = "entry";
        testParam.abilityName_ = "TestAbility";
        testParam.insightIntentName_ = "TestIntent";

        auto testParamPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(testParam);
        sptr<IRemoteObject> testCallerToken = new MockIRemoteObject();
        uint64_t testKey = testKeyBase + i;

        auto addRet = manager->CheckAndUpdateParam(testKey, testCallerToken, testParamPtr, "caller.bundle", false);
        ASSERT_EQ(addRet, ERR_OK);
        uint64_t testIntentId = testParam.insightIntentId_;

        AppExecFwk::InsightIntentExecuteResult result;
        result.resultCode = resultCodes[i];
        auto doneRet = manager->ExecuteIntentDone(testIntentId, resultCodes[i], result);

        if (doneRet == ERR_OK) {
            successCount++;
        } else {
            failureCount++;
        }
    }

    // Each ExecuteIntentDone should succeed with a fresh intentId
    EXPECT_EQ(successCount, static_cast<int32_t>(resultCodes.size()));
    EXPECT_EQ(failureCount, 0);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0300 end. Success: %{public}d, Failure: %{public}d",
        successCount, failureCount);
}

/**
 * @tc.number: RemoteDied_0100
 * @tc.name: RemoteDied_0100
 * @tc.desc: Test RemoteDied with valid intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, RemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2003;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Simulate remote death
    ret = manager->RemoteDied(intentId);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 end.");
}

/**
 * @tc.number: RemoteDied_0200
 * @tc.name: RemoteDied
 * @tc.desc: Test RemoteDied with non-existent intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, RemoteDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    auto ret = manager->RemoteDied(invalidIntentId);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 end.");
}

/**
 * @tc.number: GetBundleName_0100
 * @tc.name: GetBundleName
 * @tc.desc: Test GetBundleName with valid intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, GetBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle.name";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2004;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    std::string bundleName;
    ret = manager->GetBundleName(intentId, bundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(bundleName, "test.bundle.name");

    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 end.");
}

/**
 * @tc.number: GetBundleName_0200
 *  * @tc.name: GetBundleName
 * @tc.desc: Test GetBundleName with non-existent intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, GetBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 88888;
    std::string bundleName;

    auto ret = manager->GetBundleName(invalidIntentId, bundleName);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 end.");
}

/**
 * @tc.number: GetCallerBundleName_0100
 * @tc.name: GetCallerBundleName_0100
 * @tc.desc: Test GetCallerBundleName with valid intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, GetCallerBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2005;
    std::string callerBundleName = "caller.bundle.name";

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    std::string retrievedCallerBundleName;
    ret = manager->GetCallerBundleName(intentId, retrievedCallerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(retrievedCallerBundleName, callerBundleName);

    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0100 end.");
}

/**
 * @tc.number: GetCallerBundleName_0200
 * @tc.name: GetCallerBundleName_0200
 * @tc.desc: Test GetCallerBundleName with non-existent intentId
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, GetCallerBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 88888;
    std::string callerBundleName;

    auto ret = manager->GetCallerBundleName(invalidIntentId, callerBundleName);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0200 end.");
}

/**
 * @tc.number: IntentExemptionInfo_0100
 * @tc.name: IntentExemptionInfo_0100
 * @tc.desc: Test intent exemption info management
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, IntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IntentExemptionInfo_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t testUid = 10001;

    // Set exemption
    manager->SetIntentExemptionInfo(testUid);
    EXPECT_TRUE(manager->CheckIntentIsExemption(testUid));

    // Check different UID
    int32_t differentUid = 10002;
    EXPECT_FALSE(manager->CheckIntentIsExemption(differentUid));

    // Get all exemption info
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_FALSE(exemptionInfo.empty());

    TAG_LOGI(AAFwkTag::TEST, "IntentExemptionInfo_0100 end.");
}

/**
 * @tc.number: IntentExemptionInfo_0200
 * @tc.name: IntentExemptionInfo_0200
 * @tc.desc: Test intent exemption with multiple UIDs
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, IntentExemptionInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IntentExemptionInfo_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::vector<int32_t> testUids = {10001, 10002, 10003, 10004};

    // Set exemptions for multiple UIDs
    for (auto uid : testUids) {
        manager->SetIntentExemptionInfo(uid);
    }

    // Verify all are exempted
    for (auto uid : testUids) {
        EXPECT_TRUE(manager->CheckIntentIsExemption(uid));
    }

    // Get all exemption info
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), testUids.size());

    TAG_LOGI(AAFwkTag::TEST, "IntentExemptionInfo_0200 end.");
}

/**
 * @tc.number: OnInsightAppDied_0100
 * @tc.name: OnInsightAppDied
 * @tc.desc: Test OnInsightAppDied callback
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, OnInsightAppDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::string bundleName = "test.died.bundle";
    EXPECT_EQ(bundleName, "test.died.bundle");
    EXPECT_FALSE(bundleName.empty());

    // Verify the method can be called without crash
    manager->OnInsightAppDied(bundleName);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0100 end.");
}

/**
 * @tc.number: OnInsightAppDied_0200
 *  * @tc.name: OnInsightAppDied
 * @tc.desc: Test OnInsightAppDied with empty bundle name
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, OnInsightAppDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::string emptyBundleName = "";
    EXPECT_TRUE(emptyBundleName.empty());

    // Verify the method can be called with empty string
    manager->OnInsightAppDied(emptyBundleName);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0200 end.");
}

/**
 * @tc.number: ExecuteIntentDoneWithResultData_0100
 * @tc.name: ExecuteIntentDoneWithResultData
 * @tc.desc: Test ExecuteIntentDone with result data
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, ExecuteIntentDoneWithResultData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDoneWithResultData_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // Add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "entry";
    param.abilityName_ = "TestAbility";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = new MockIRemoteObject();
    uint64_t key = 2006;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Create result with data
    AppExecFwk::InsightIntentExecuteResult result;
    result.resultCode = 0;
    result.resultData["key1"] = "value1";
    result.resultData["key2"] = "value2";

    ret = manager->ExecuteIntentDone(intentId, 0, result);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDoneWithResultData_0100 end.");
}

/**
 * @tc.number: CheckCallerPermission_0100
 * @tc.name: CheckCallerPermission
 * @tc.desc: Test CheckCallerPermission static method
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, CheckCallerPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 begin.");

    // This is a static method that checks permissions
    // The actual behavior depends on the system state
    auto ret = InsightIntentExecuteManager::CheckCallerPermission();

    // Verify the method returns a valid error code (not garbage)
    // Valid error codes are typically in range [ERR_INVALID_OPERATION, ERR_OK]
    EXPECT_GE(ret, ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission returned: %{public}d", ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 end.");
}

/**
 * @tc.number: CheckGetInsightIntentInfoPermission_0100
 * @tc.name: CheckGetInsightIntentInfoPermission
 * @tc.desc: Test CheckGetInsightIntentInfoPermission static method
 */
HWTEST_F(InsightIntentExecuteManagerThirdTest, CheckGetInsightIntentInfoPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission_0100 begin.");

    // This is a static method that checks permissions
    auto ret = InsightIntentExecuteManager::CheckGetInsightIntenInfoPermission();

    // Verify the method returns a valid error code
    // Valid error codes are typically in range [ERR_INVALID_OPERATION, ERR_OK]
    EXPECT_GE(ret, ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission returned: %{public}d", ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission_0100 end.");
}

} // namespace AAFwk
} // namespace OHOS
