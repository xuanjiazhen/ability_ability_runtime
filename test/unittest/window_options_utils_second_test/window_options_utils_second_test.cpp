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
#include "ability_manager_service.h"
#include "app_utils.h"
#include "mock_parameters.h"
#include "utils/window_options_utils.h"
#include "hilog_tag_wrapper.h"
using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::ExtensionAbilityType;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string PARAM_RESV_WINDOW_LEFT("ohos.aafwk.param.windowLeft");
const std::string PARAM_RESV_WINDOW_TOP("ohos.aafwk.param.windowTop");
const std::string PARAM_RESV_WINDOW_WIDTH("ohos.aafwk.param.windowWidth");
const std::string PARAM_RESV_WINDOW_HEIGHT("ohos.aafwk.param.windowHeight");
const std::string PARAM_RESV_WITH_ANIMATION("ohos.aafwk.param.withAnimation");
constexpr const char* START_OPTIONS_WITH_ANIMATION = "persist.sys.abilityms.start_options_with_animation";
const std::string PARAM_RESV_DISPLAY_ID("ohos.aafwk.param.displayId");
}
class WindowOptionsUtilsSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void WindowOptionsUtilsSecondTest::SetUpTestCase() {}

void WindowOptionsUtilsSecondTest::TearDownTestCase() {}

void WindowOptionsUtilsSecondTest::SetUp() {}

void WindowOptionsUtilsSecondTest::TearDown() {}

/*
 * Feature: WindowOptionsUtils
 * Function: SetWindowPositionAndSize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetWindowPositionAndSize
 */
HWTEST_F(WindowOptionsUtilsSecondTest, SetWindowPositionAndSize_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest SetWindowPositionAndSize_001 start");
    Want want;
    sptr<IRemoteObject> callerToken;
    StartOptions startOptions;
    startOptions.SetWindowLeft(1);
    startOptions.SetWindowTop(1);
    startOptions.SetWindowWidth(10); // 10 means window width
    startOptions.SetWindowHeight(10); // 10 means window height
    auto Window_ = std::make_shared<WindowOptionsUtils>();

    startOptions.SetWithAnimation(false);
    Window_->SetWindowPositionAndSize(want, callerToken, startOptions);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_LEFT), 0);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_TOP), 0);
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    startOptions.windowLeftUsed_ = true;
    startOptions.windowTopUsed_ = true;
    startOptions.windowWidthUsed_ = true;
    startOptions.windowHeightUsed_ = true;
    startOptions.SetWithAnimation(true);
    Window_->SetWindowPositionAndSize(want, callerToken, startOptions);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_LEFT), 1);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_TOP), 1);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_WIDTH), 10); // 10 means window height
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_HEIGHT), 10); // 10 means window height

    startOptions.SetWithAnimation(false);
    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    want.SetParam(PARAM_RESV_WITH_ANIMATION, Boolean::Box(true));
    Window_->SetWindowPositionAndSize(want, launcher->GetToken(), startOptions);
    EXPECT_TRUE(want.GetBoolParam(PARAM_RESV_WITH_ANIMATION));
    want.SetBundle("com.test");
    Window_->SetWindowPositionAndSize(want, launcher->GetToken(), startOptions);
    EXPECT_FALSE(want.GetBoolParam(PARAM_RESV_WITH_ANIMATION));
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest SetWindowPositionAndSize_001 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: UpdateWantToSetDisplayID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateWantToSetDisplayID
 */
HWTEST_F(WindowOptionsUtilsSecondTest, UpdateWantToSetDisplayID_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest UpdateWantToSetDisplayID_001 start");
    Want want;
    sptr<IRemoteObject> callerToken;
    auto Window_ = std::make_shared<WindowOptionsUtils>();
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    OHOS::AAFwk::WantParams params = want.GetParams();
    params.SetParam(PARAM_RESV_DISPLAY_ID, AAFwk::String::Box("1"));
    want.SetParams(params);
    Window_->UpdateWantToSetDisplayID(want, nullptr);
    EXPECT_EQ(want.getStringParam(PARAM_RESV_DISPLAY_ID), "0");

    want.SetParam(PARAM_RESV_DISPLAY_ID, "0");
    AbilityRequest abilityRequest;
    AbilityRequest.want.SetParam(PARAM_RESV_DISPLAY_ID, "1");
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    Window_->UpdateWantToSetDisplayID(want, launcher->GetToken());
    EXPECT_EQ(want.getStringParam(PARAM_RESV_DISPLAY_ID), "1");

    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest UpdateWantToSetDisplayID_001 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: UpdateStartOptionsToSetDisplayID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateStartOptionsToSetDisplayID
 */
HWTEST_F(WindowOptionsUtilsSecondTest, UpdateStartOptionsToSetDisplayID_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest UpdateStartOptionsToSetDisplayID_001 start");
    sptr<IRemoteObject> callerToken;
    StartOptions startOptions;
    auto Window_ = std::make_shared<WindowOptionsUtils>();
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    Window_->UpdateStartOptionsToSetDisplayID(startOptions, callerToken);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    Window_->UpdateStartOptionsToSetDisplayID(startOptions, launcher->GetToken());
    EXPECT_NE(nullptr, Window_);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsSecondTest UpdateStartOptionsToSetDisplayID_001 end");
}
} // namespace AAFwk
} // namespace OHOS
