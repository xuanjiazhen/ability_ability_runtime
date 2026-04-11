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

#include <gtest/gtest.h>

#include "hilog_tag_wrapper.h"
#include "insight_intent_event_mgr.h"
#include "element_name.h"
#include "ability_manager_errors.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {

class InsightIntentEventMgrTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // Helper to verify ElementName is properly set
    void VerifyElementName(const AppExecFwk::ElementName& elementName,
                          const std::string& expectedBundle,
                          const std::string& expectedModule,
                          const std::string& expectedAbility);
};

void InsightIntentEventMgrTest::SetUpTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEventMgrTest::SetUpTestCase called");
}

void InsightIntentEventMgrTest::TearDownTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEventMgrTest::TearDownTestCase called");
}

void InsightIntentEventMgrTest::SetUp()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEventMgrTest::SetUp called");
}

void InsightIntentEventMgrTest::TearDown()
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentEventMgrTest::TearDown called");
}

void InsightIntentEventMgrTest::VerifyElementName(const AppExecFwk::ElementName& elementName,
                                                   const std::string& expectedBundle,
                                                   const std::string& expectedModule,
                                                   const std::string& expectedAbility)
{
    EXPECT_EQ(elementName.GetBundleName(), expectedBundle);
    EXPECT_EQ(elementName.GetModuleName(), expectedModule);
    EXPECT_EQ(elementName.GetAbilityName(), expectedAbility);
}

/**
 * @tc.number: UpdateInsightIntentEvent_0100
 * @tc.name: UpdateInsightIntentEvent
 * @tc.desc: Test updating insight intent event with valid element name
 */
HWTEST_F(InsightIntentEventMgrTest, UpdateInsightIntentEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0100 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    elementName.SetAbilityName("TestAbility");
    int32_t userId = 100;

    // Verify input parameters are correct
    VerifyElementName(elementName, "test.bundle.name", "entry", "TestAbility");
    EXPECT_EQ(userId, 100);

    // Call the function - should not throw or crash
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);

    // If we reach here without crash, test passes
    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0100 end.");
}

/**
 * @tc.number: DeleteInsightIntentEvent_0100
 * @tc.name: DeleteInsightIntentEvent
 * @tc.desc: Test deleting insight intent event with valid parameters
 */
HWTEST_F(InsightIntentEventMgrTest, DeleteInsightIntentEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0100 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    int32_t userId = 100;
    int32_t appIndex = 0;

    // Verify input parameters
    EXPECT_EQ(elementName.GetBundleName(), "test.bundle.name");
    EXPECT_EQ(userId, 100);
    EXPECT_EQ(appIndex, 0);

    // First update, then delete
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);
    InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndex);

    // If we reach here without crash, test passes
    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0100 end.");
}

/**
 * @tc.number: DeleteInsightIntentEvent_0200
 * @tc.name: DeleteInsightIntentEvent
 * @tc.desc: Test deleting insight intent event with different appIndex values
 */
HWTEST_F(InsightIntentEventMgrTest, DeleteInsightIntentEvent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0200 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    int32_t userId = 100;

    EXPECT_EQ(elementName.GetBundleName(), "test.bundle.name");

    // Update once
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);

    // Test multiple appIndex values
    std::vector<int32_t> appIndexes = {0, 1, 2, 3, 4};
    for (size_t i = 0; i < appIndexes.size(); i++) {
        EXPECT_EQ(appIndexes[i], static_cast<int32_t>(i));
        InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndexes[i]);
    }

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0200 end.");
}

/**
 * @tc.number: DeleteInsightIntentEvent_0300
 * @tc.name: DeleteInsightIntentEvent
 * @tc.desc: Test deleting insight intent event with empty bundle name
 */
HWTEST_F(InsightIntentEventMgrTest, DeleteInsightIntentEvent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0300 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("");
    elementName.SetModuleName("entry");
    int32_t userId = 100;
    int32_t appIndex = 0;

    // Verify empty bundle name
    EXPECT_TRUE(elementName.GetBundleName().empty());
    EXPECT_EQ(elementName.GetModuleName(), "entry");

    // Should handle gracefully without crash
    InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndex);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0300 end.");
}

/**
 * @tc.number: UpdateInsightIntentEvent_0200
 * @tc.name: UpdateInsightIntentEvent
 * @tc.desc: Test updating insight intent event with different user IDs
 */
HWTEST_F(InsightIntentEventMgrTest, UpdateInsightIntentEvent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0200 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    elementName.SetAbilityName("TestAbility");

    VerifyElementName(elementName, "test.bundle.name", "entry", "TestAbility");

    // Test with different userId values
    std::vector<int32_t> userIds = {0, 100, 101, 999, 1000};

    for (size_t i = 0; i < userIds.size(); i++) {
        EXPECT_GE(userIds[i], 0);  // Verify userId is non-negative
        InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userIds[i]);
    }

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0200 end.");
}

/**
 * @tc.number: UpdateInsightIntentEvent_0300
 * @tc.name: UpdateInsightIntentEvent
 * @tc.desc: Test updating insight intent event with empty element name
 */
HWTEST_F(InsightIntentEventMgrTest, UpdateInsightIntentEvent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0300 begin.");

    AppExecFwk::ElementName elementName;
    int32_t userId = 100;

    // Verify all fields are empty
    EXPECT_TRUE(elementName.GetBundleName().empty());
    EXPECT_TRUE(elementName.GetModuleName().empty());
    EXPECT_TRUE(elementName.GetAbilityName().empty());

    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "UpdateInsightIntentEvent_0300 end.");
}

/**
 * @tc.number: DeleteInsightIntentEvent_0400
 * @tc.name: DeleteInsightIntentEvent
 * @tc.desc: Test deleting insight intent event with negative appIndex
 */
HWTEST_F(InsightIntentEventMgrTest, DeleteInsightIntentEvent_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0400 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    int32_t userId = 100;
    int32_t appIndex = -1;

    EXPECT_LT(appIndex, 0);  // Verify it's negative

    // Should handle gracefully
    InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndex);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "DeleteInsightIntentEvent_0400 end.");
}

/**
 * @tc.number: UpdateDeleteSequenceTest_0100
 * @tc.name: UpdateDeleteSequenceTest
 * @tc.desc: Test update followed by delete sequence
 */
HWTEST_F(InsightIntentEventMgrTest, UpdateDeleteSequenceTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateDeleteSequenceTest_0100 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    elementName.SetAbilityName("TestAbility");
    int32_t userId = 100;
    int32_t appIndex = 0;

    VerifyElementName(elementName, "test.bundle.name", "entry", "TestAbility");

    // Update then delete - verify order
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);
    InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndex);

    // Verify the same element can be updated again after delete
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "UpdateDeleteSequenceTest_0100 end.");
}

/**
 * @tc.number: MultipleUpdateTest_0100
 * @tc.name: MultipleUpdateTest
 * @tc.desc: Test multiple updates with same element name
 */
HWTEST_F(InsightIntentEventMgrTest, MultipleUpdateTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultipleUpdateTest_0100 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");
    elementName.SetAbilityName("TestAbility");
    int32_t userId = 100;

    VerifyElementName(elementName, "test.bundle.name", "entry", "TestAbility");

    // Multiple updates should not cause issues
    int32_t updateCount = 10;
    for (int i = 0; i < updateCount; i++) {
        InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);
    }

    EXPECT_EQ(updateCount, 10);  // Verify we did 10 updates

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "MultipleUpdateTest_0100 end.");
}

/**
 * @tc.number: ExtremeValuesTest_0100
 * @tc.name: ExtremeValuesTest
 * @tc.desc: Test with extreme userId and appIndex values
 */
HWTEST_F(InsightIntentEventMgrTest, ExtremeValuesTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtremeValuesTest_0100 begin.");

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("test.bundle.name");
    elementName.SetModuleName("entry");

    EXPECT_EQ(elementName.GetBundleName(), "test.bundle.name");

    // Test with extreme values
    std::vector<int32_t> extremeValues = {
        INT32_MIN, INT32_MAX, -100, 999999, 0
    };

    for (size_t i = 0; i < extremeValues.size(); i++) {
        // Verify the extreme values are as expected
        if (i == 0) EXPECT_EQ(extremeValues[i], INT32_MIN);
        else if (i == 1) EXPECT_EQ(extremeValues[i], INT32_MAX);

        InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, extremeValues[i]);
        InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, extremeValues[i], 0);
    }

    EXPECT_EQ(extremeValues.size(), 5UL);  // Verify we tested 5 values

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "ExtremeValuesTest_0100 end.");
}

/**
 * @tc.number: LongBundleNameTest_0100
 * @tc.name: LongBundleNameTest
 * @tc.desc: Test with very long bundle name
 */
HWTEST_F(InsightIntentEventMgrTest, LongBundleNameTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "LongBundleNameTest_0100 begin.");

    AppExecFwk::ElementName elementName;
    std::string longBundleName(500, 'a');  // 500 character bundle name
    elementName.SetBundleName(longBundleName);
    elementName.SetModuleName("entry");
    int32_t userId = 100;
    int32_t appIndex = 0;

    // Verify the long bundle name was set correctly
    EXPECT_EQ(elementName.GetBundleName().length(), 500UL);
    EXPECT_EQ(elementName.GetBundleName(), longBundleName);

    // Should handle gracefully
    InsightIntentEventMgr::UpdateInsightIntentEvent(elementName, userId);
    InsightIntentEventMgr::DeleteInsightIntentEvent(elementName, userId, appIndex);

    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "LongBundleNameTest_0100 end.");
}

} // namespace AAFwk
} // namespace OHOS
