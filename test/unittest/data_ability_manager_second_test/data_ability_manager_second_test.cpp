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

#include <chrono>
#include <thread>
#include <gtest/gtest.h>

#include "data_ability_manager.h"
#include "app_scheduler.h"
#include "ability_scheduler_mock.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_client.h"
#include "ability_manager_errors.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class DataAbilityManagerSecondTest : public testing::TestWithParam<OHOS::AAFwk::AbilityState> {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    sptr<AbilitySchedulerMock> abilitySchedulerMock_{ nullptr };
    AbilityRequest abilityRequest_{};
    std::shared_ptr<AbilityRecord> abilityRecordClient_{ nullptr };
    OHOS::AAFwk::AbilityState abilityState_{};
};

void DataAbilityManagerSecondTest::SetUpTestCase(void)
{}
void DataAbilityManagerSecondTest::TearDownTestCase(void)
{}

void DataAbilityManagerSecondTest::SetUp(void)
{
    if (abilitySchedulerMock_ == nullptr) {
        abilitySchedulerMock_ = new AbilitySchedulerMock();
    }

    abilityRequest_.appInfo.bundleName = "com.test.data_ability";
    abilityRequest_.appInfo.name = "com.test.data_ability";
    abilityRequest_.abilityInfo.name = "DataAbilityHiworld";
    abilityRequest_.abilityInfo.type = AbilityType::DATA;
    abilityRequest_.abilityInfo.bundleName = "com.test.data_ability";
    abilityRequest_.abilityInfo.deviceId = "device";

    if (abilityRecordClient_ == nullptr) {
        OHOS::AppExecFwk::AbilityInfo abilityInfo;
        abilityInfo.name = "DataAbilityClient";
        abilityInfo.type = AbilityType::PAGE;
        abilityInfo.bundleName = "com.test.request";
        abilityInfo.deviceId = "device";
        OHOS::AppExecFwk::ApplicationInfo applicationInfo;
        applicationInfo.bundleName = "com.test.request";
        applicationInfo.name = "com.test.request";
        const Want want;
        abilityRecordClient_ = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
        abilityRecordClient_->Init();
    }
    abilityState_ = INITIAL;
}

void DataAbilityManagerSecondTest::TearDown(void)
{
    abilitySchedulerMock_.clear();
}


/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: AbilityTransitionDone
 * FunctionPoints: The parameter of function AbilityTransitionDone.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function AbilityTransitionDone ability record invalid
 */
HWTEST_F(DataAbilityManagerSecondTest, AaFwk_DataAbilityManager_AbilityTransitionDone_001, TestSize.Level1)
{
    std::shared_ptr<DataAbilityRecord> dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest_);
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    dataAbilityRecord->ability_ = abilityRecordClient_;
    dataAbilityRecord->ability_->SetAbilityState(OHOS::AAFwk::AbilityState::ACTIVATING);
    dataAbilityRecord->scheduler_ = abilitySchedulerMock_;
    dataAbilityManager->dataAbilityRecordsLoading_["om.test.data_ability"] = dataAbilityRecord;

    EXPECT_EQ(
        dataAbilityManager->AbilityTransitionDone(abilityRecordClient_->GetToken(), ABILITY_STATE_ACTIVE), ERR_OK);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordById
 * FunctionPoints: The parameter of function GetAbilityRecordById.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordById
 */
HWTEST_F(DataAbilityManagerSecondTest, AaFwk_DataAbilityManager_GetAbilityRecordById_001, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    int64_t id = 1;
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    abilityRecord->recordId_ = 1;
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord;
    auto res = dataAbilityManager->GetAbilityRecordById(id);
    EXPECT_NE(res, nullptr);
}

/*
 * Feature: AbilityManager
 * Function: DataAbility
 * SubFunction: GetAbilityRecordByToken
 * FunctionPoints: The parameter of function GetAbilityRecordByToken.
 * EnvConditions: Can run ohos test framework
 * CaseDescription: Verify function GetAbilityRecordByToken token is nullptr.
 */
HWTEST_F(DataAbilityManagerSecondTest, AaFwk_DataAbilityManager_GetAbilityRecordByToken_002, TestSize.Level1)
{
    std::unique_ptr<DataAbilityManager> dataAbilityManager = std::make_unique<DataAbilityManager>();
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    abilityRequest.abilityInfo.bundleName = "bundleName";
    abilityRequest.abilityInfo.name = "name";
    auto dataAbilityRecord = std::make_shared<DataAbilityRecord>(abilityRequest);
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    dataAbilityRecord->ability_ = abilityRecord;
    dataAbilityManager->dataAbilityRecordsLoaded_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = dataAbilityRecord;
    dataAbilityManager->dataAbilityRecordsLoading_["a"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["b"] = dataAbilityRecord;
    auto res = dataAbilityManager->GetAbilityRecordByToken(dataAbilityRecord->GetToken());
    EXPECT_NE(res, nullptr);

    dataAbilityManager->dataAbilityRecordsLoaded_["b"] = nullptr;
    dataAbilityManager->dataAbilityRecordsLoading_["b"] = nullptr;
    res = dataAbilityManager->GetAbilityRecordByToken(dataAbilityRecord->GetToken());
    EXPECT_EQ(res, nullptr);
}
}  // namespace AAFwk
}  // namespace OHOS