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

#include "hilog_tag_wrapper.h"
#include "interceptor/screen_unlock_interceptor.h"
#include "ability_record.h"
#include "parameters.h"
#include "start_ability_utils.h"
#include "ability_util.h"
#include "screenlock_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* SUPPORT_SCREEN_UNLOCK_STARTUP = "persist.sys.ability.support.screen_unlock_startup";
}
class ScreenUnlockInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<AAFwk::Token> GetTestAbilityToken();
};

void ScreenUnlockInterceptorTest::SetUpTestCase() {}

void ScreenUnlockInterceptorTest::TearDownTestCase() {}

void ScreenUnlockInterceptorTest::SetUp() {}

void ScreenUnlockInterceptorTest::TearDown() {}

sptr<AAFwk::Token> ScreenUnlockInterceptorTest::GetTestAbilityToken()
{
    sptr<AAFwk::Token> token = nullptr;
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.utTest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::DATA;
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord =
        AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

/**
 * @tc.name  : DoProcess_001
 * @tc.number: DoProcess_Test_001
 * @tc.desc  : Test the scenario when supportStart is "true"
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_001, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    OHOS::system::SetParameter(SUPPORT_SCREEN_UNLOCK_STARTUP, "true");

    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name  : DoProcess_002
 * @tc.number: DoProcess_Test_002
 * @tc.desc  : Test the scenario when startAbilityInfo is not nullptr
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_002, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    OHOS::system::SetParameter(SUPPORT_SCREEN_UNLOCK_STARTUP, "false");
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();

    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name  : DoProcess_003
 * @tc.number: DoProcess_Test_003
 * @tc.desc  : Test the scenario when bundleMgrHelper is nullptr
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_003, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    StartAbilityUtils::startAbilityInfo = nullptr;
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name  : DoProcess_004
 * @tc.number: DoProcess_Test_004
 * @tc.desc  : Test the scenario when targetAbilityInfo is invalid
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_004, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    OHOS::AppExecFwk::ElementName element("deviceId", "bundleName", "abilityName", "moduleName");
    want.SetElement(element);
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    StartAbilityUtils::startAbilityInfo = nullptr;
    AbilityInfo abilityInfo;
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    bundleMgrHelper->QueryAbilityInfo(param.want, AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION,
        param.userId, abilityInfo);
    
    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name  : DoProcess_005
 * @tc.number: DoProcess_Test_005
 * @tc.desc  : Test the scenario when targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked is true
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_005, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::AbilityInfo &targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
    targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = true;

    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name  : DoProcess_006
 * @tc.number: DoProcess_Test_006
 * @tc.desc  : Test the scenario when IsScreenLocked is false
 */
HWTEST_F(ScreenUnlockInterceptorTest, DoProcess_006, TestSize.Level0)
{
    // Arrange
    ScreenUnlockInterceptor screenUnlockInterceptor;
    Want want;
    int requestCode = 1;
    int32_t userId = 100;
    bool isWithUI = false;
    sptr<IRemoteObject> token = GetTestAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    AAFwk::AbilityInterceptorParam param = AAFwk::AbilityInterceptorParam(want, requestCode, userId, isWithUI, token,
        shouldBlockFunc);
    StartAbilityUtils::startAbilityInfo = std::make_shared<StartAbilityInfo>();
    AppExecFwk::AbilityInfo &targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
    targetAbilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = false;

    // Act
    ErrCode result = screenUnlockInterceptor.DoProcess(param);

    // Assert
    EXPECT_NE(result, ERR_OK);
}
}  // namespace AbilityRuntime
}  // namespace OHOS