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

#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "start_ability_handler.h"
#include "ability_info.h"
#include "hilog_tag_wrapper.h"
#include "process_options.h"
#include "ipc_skeleton.h"
#include "start_ability_utils.h"


using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceEighthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> GetAbilityRecord();
};

void AbilityManagerServiceEighthTest::SetUpTestCase() {}

void AbilityManagerServiceEighthTest::TearDownTestCase() {}

void AbilityManagerServiceEighthTest::SetUp() {}

void AbilityManagerServiceEighthTest::TearDown(){}

std::shared_ptr<AbilityRecord> AbilityManagerServiceEighthTest::GetAbilityRecord()
{
    Want want;
    OHOS::AppExecFwk::AbilityInfo abilityInfo;
    OHOS::AppExecFwk::ApplicationInfo applicationInfo;
    return std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
}

/*
 * Feature: AbilityManagerService
 * Name: InitialAbilityRequest_001
 * Function: InitialAbilityRequest
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, InitialAbilityRequest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest InitialAbilityRequest_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest request;
    StartAbilityInfo abilityInfo;
    auto result = abilityMs_->InitialAbilityRequest(request, abilityInfo);
    EXPECT_EQ(result, RESOLVE_APP_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest InitialAbilityRequest_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: InitialAbilityRequest_002
 * Function: InitialAbilityRequest
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, InitialAbilityRequest_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest InitialAbilityRequest_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    AbilityRequest request;
    StartAbilityInfo startAbilityInfo;
    startAbilityInfo.abilityInfo.applicationInfo.name = "applicationInfo";
    startAbilityInfo.abilityInfo.applicationInfo.bundleName = "bundleName";
    startAbilityInfo.abilityInfo.moduleName = "moduleName";
    auto result = abilityMs_->InitialAbilityRequest(request, startAbilityInfo);
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest InitialAbilityRequest_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_001
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, GetShareDataPairAndReturnData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    int32_t resultCode = 1;
    int32_t uniqueId = 1;
    WantParams wantParam;

    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_002
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, GetShareDataPairAndReturnData_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_002 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    auto pair = std::make_pair(66, shareData);
	abilityMs_->iAcquireShareDataMap_.insert(std::make_pair(6, pair));
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->recordId_ = 8;
    int32_t resultCode = 1;
    int32_t uniqueId = 6;
    WantParams wantParam;
    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_002 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetShareDataPairAndReturnData_003
 * Function: GetShareDataPairAndReturnData
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, GetShareDataPairAndReturnData_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_003 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IAcquireShareDataCallback> shareData = nullptr;
    auto pair = std::make_pair(66, shareData);
	abilityMs_->iAcquireShareDataMap_.insert(std::make_pair(6, pair));
    std::shared_ptr<AbilityRecord> abilityRecord = GetAbilityRecord();
    abilityRecord->recordId_ = 66;
    int32_t resultCode = 1;
    int32_t uniqueId = 6;
    WantParams wantParam;
    auto result = abilityMs_->GetShareDataPairAndReturnData(abilityRecord, resultCode, uniqueId, wantParam);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetShareDataPairAndReturnData_003 end");
}

/*
 * Feature: AbilityManagerService
 * Name: GetScreenUnlockCallback_001
 * Function: GetScreenUnlockCallback
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, GetScreenUnlockCallback_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetScreenUnlockCallback_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->StartAutoStartupApps();
    abilityMs_->abilityAutoStartupService_ = std::make_shared<AbilityAutoStartupService>();
    abilityMs_->StartAutoStartupApps();
    int32_t userId = 100;
    abilityMs_->StartKeepAliveApps(userId);
    auto screenUnlockCallback = abilityMs_->GetScreenUnlockCallback();
    screenUnlockCallback();
    EXPECT_NE(screenUnlockCallback, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest GetScreenUnlockCallback_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: SubscribeScreenUnlockedEvent_001
 * Function: SubscribeScreenUnlockedEvent
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, SubscribeScreenUnlockedEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest SubscribeScreenUnlockedEvent_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    abilityMs_->StartResidentApps(userId);
    std::queue<AutoStartupInfo> infoQueue;
    abilityMs_->StartAutoStartupApps(infoQueue);
    AutoStartupInfo autoStartupInfo;
    infoQueue.push(autoStartupInfo);
    abilityMs_->StartAutoStartupApps(infoQueue);
    abilityMs_->SubscribeScreenUnlockedEvent();
    EXPECT_NE(abilityMs_->screenSubscriber_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest SubscribeScreenUnlockedEvent_001 end");
}

/*
 * Feature: AbilityManagerService
 * Name: StartHighestPriorityAbility_001
 * Function: StartHighestPriorityAbility
 * SubFunction: NA
 */
HWTEST_F(AbilityManagerServiceEighthTest, StartHighestPriorityAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest StartHighestPriorityAbility_001 start");
    std::shared_ptr<AbilityManagerService> abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    int32_t userId = 100;
    bool isBoot = 1;
    bool isAppRecovery = false;
    auto ret = abilityMs_->StartHighestPriorityAbility(userId, isBoot, isAppRecovery);
    EXPECT_NE(ret, RESOLVE_ABILITY_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceEighthTest StartHighestPriorityAbility_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS