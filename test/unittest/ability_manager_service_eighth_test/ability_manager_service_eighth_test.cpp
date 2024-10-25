/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_record.h"
#include "iservice_registry.h"
#include "mission_list_manager.h"
#include "ui_ability_lifecycle_manager.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_scheduler_mock.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager_service.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "mock_system_ability_manager.h"

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Invoke;
using testing::Return;

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t MOCK_MAIN_USER_ID = 100;
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
auto mockBundleMgr = sptr<MockBundleManagerService>::MakeSptr();
}  // namespace
class AbilityManagerServiceEighthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void MockBundleInstallerAndSA();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    void InitAbilityManagerService(std::shared_ptr<AbilityManagerService> abilityMs);

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
    AbilityStartSetting abilityStartSetting_;
};

AbilityRequest AbilityManagerServiceEighthTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void AbilityManagerServiceEighthTest::SetUpTestCase() {}

void AbilityManagerServiceEighthTest::TearDownTestCase() {}

void AbilityManagerServiceEighthTest::SetUp()
{
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    MockBundleInstallerAndSA();
}

void AbilityManagerServiceEighthTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

void AbilityManagerServiceEighthTest::MockBundleInstallerAndSA()
{
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockSystemAbility_, CheckSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void AbilityManagerServiceEighthTest::InitAbilityManagerService(std::shared_ptr<AbilityManagerService> abilityMs)
{
    abilityMs->taskHandler_ = TaskHandlerWrap::CreateQueueHandler("AbilityManagerServiceEighthTest");
    abilityMs->eventHandler_ = std::make_shared<AbilityEventHandler>(abilityMs->taskHandler_, abilityMs);
    abilityMs->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs);
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(nullptr, nullptr);
}

/*
 * Feature: AbilityManagerService
 * Function: StopServiceAbility
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService StopServiceAbility
 */
HWTEST_F(AbilityManagerServiceEighthTest, StopServiceAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StopServiceAbility_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo);
    abilityRecord->Init();
    auto token = abilityRecord->GetToken();
    int32_t userId = -1;
    MyFlag::flag_ = 0;
    auto ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    MyFlag::flag_ = MyFlag::IS_SHELL_CALL;
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    userId = 0;
    AppExecFwk::AbilityInfo abilityInfo1;
    abilityInfo1.name = "ability1";
    abilityInfo1.bundleName = "testBundleName";
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo1), Return(true)));
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    EXPECT_EQ(ret, RESOLVE_APP_ERR);
    abilityInfo1.applicationInfo.name = "test";
    abilityInfo1.applicationInfo.bundleName = "testBundleName";
    abilityInfo1.type = AppExecFwk::AbilityType::EXTENSION;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo1), Return(true)));
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityInfo1.type = AppExecFwk::AbilityType::SERVICE;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo1), Return(true)));
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    abilityInfo1.type = AppExecFwk::AbilityType::PAGE;
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<3>(abilityInfo1), Return(true)));
    ret = abilityMs->StopServiceAbility(want, userId, nullptr);
    EXPECT_EQ(ret, TARGET_ABILITY_NOT_SERVICE);
    TAG_LOGI(AAFwkTag::TEST, "AbilityManagerServiceSixthTest StopServiceAbility_001 end");
}
}  // namespace AAFwk
}  // namespace OHOS