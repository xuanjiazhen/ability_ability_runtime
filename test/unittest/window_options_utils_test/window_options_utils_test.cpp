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

#include <optional>

#include <gtest/gtest.h>

#include "ability_manager_service.h"
#define private public
#include "app_utils.h"
#undef private
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
const std::string PARAM_RESV_DISPLAY_ID("ohos.aafwk.param.displayId");
constexpr int32_t DEFAULT_WINDOW_OPTIONS_LEFT = 5;
constexpr int32_t DEFAULT_WINDOW_OPTIONS_TOP = 5;
constexpr int32_t DEFAULT_WINDOW_OPTIONS_WIDTH = 5;
constexpr int32_t DEFAULT_WINDOW_OPTIONS_HEIGHT = 5;
constexpr const char* START_OPTIONS_WITH_ANIMATION = "persist.sys.abilityms.start_options_with_animation";
}

struct WindowOptionsPara {
    std::optional<int32_t> windowLeft_;
    std::optional<int32_t> windowTop_;
    std::optional<int32_t> windowWidth_;
    std::optional<int32_t> windowHeight_;
};

class WindowOptionsUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void InitWantWithWindowOptions(Want &want, const WindowOptionsPara &para);
};

void WindowOptionsUtilsTest::SetUpTestCase() {}

void WindowOptionsUtilsTest::TearDownTestCase() {}

void WindowOptionsUtilsTest::SetUp() {}

void WindowOptionsUtilsTest::TearDown() {}

void WindowOptionsUtilsTest::InitWantWithWindowOptions(Want &want, const WindowOptionsPara &para)
{
    if (para.windowLeft_.has_value()) {
        want.SetParam(PARAM_RESV_WINDOW_LEFT, para.windowLeft_.value());
    }
    if (para.windowTop_.has_value()) {
        want.SetParam(PARAM_RESV_WINDOW_TOP, para.windowTop_.value());
    }
    if (para.windowWidth_.has_value()) {
        want.SetParam(PARAM_RESV_WINDOW_WIDTH, para.windowWidth_.value());
    }
    if (para.windowHeight_.has_value()) {
        want.SetParam(PARAM_RESV_WINDOW_HEIGHT, para.windowHeight_.value());
    }
}

/*
 * Feature: WindowOptionsUtils
 * Function: SetWindowPositionAndSize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetWindowPositionAndSize
 */
HWTEST_F(WindowOptionsUtilsTest, SetWindowPositionAndSize_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0100 start");
    Want want;
    WindowOptionsPara para {
        .windowLeft_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_LEFT),
        .windowTop_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_TOP),
        .windowWidth_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_WIDTH),
        .windowHeight_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_HEIGHT)
    };
    InitWantWithWindowOptions(want, para);
    auto windowOptionsUtils = std::make_shared<WindowOptionsUtils>();
    StartOptions startOptions;
    startOptions.SetWindowLeft(1);
    startOptions.SetWindowTop(1);
    startOptions.SetWindowWidth(1);
    startOptions.SetWindowHeight(1);
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, FALSE);
    windowOptionsUtils->SetWindowPositionAndSize(want, nullptr, startOptions);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_LEFT, 0), DEFAULT_WINDOW_OPTIONS_LEFT);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_TOP, 0), DEFAULT_WINDOW_OPTIONS_TOP);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_WIDTH, 0), DEFAULT_WINDOW_OPTIONS_WIDTH);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_HEIGHT, 0), DEFAULT_WINDOW_OPTIONS_HEIGHT);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0100 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: SetWindowPositionAndSize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetWindowPositionAndSize
 */
HWTEST_F(WindowOptionsUtilsTest, SetWindowPositionAndSize_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0200 start");
    Want want;
    WindowOptionsPara para {
        .windowLeft_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_LEFT),
        .windowTop_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_TOP),
        .windowWidth_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_WIDTH),
        .windowHeight_ = std::make_optional<int32_t>(DEFAULT_WINDOW_OPTIONS_HEIGHT)
    };
    InitWantWithWindowOptions(want, para);
    auto windowOptionsUtils = std::make_shared<WindowOptionsUtils>();
    StartOptions startOptions;
    startOptions.windowLeftUsed_ = true;
    startOptions.windowTopUsed_ = true;
    startOptions.windowWidthUsed_ = true;
    startOptions.windowHeightUsed_ = true;
    startOptions.SetWindowLeft(1);
    startOptions.SetWindowTop(1);
    startOptions.SetWindowWidth(1);
    startOptions.SetWindowHeight(1);
    startOptions.SetWithAnimation(true);
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = false;
    windowOptionsUtils->SetWindowPositionAndSize(want, nullptr, startOptions); // null token
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_LEFT, 0), 1);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_TOP, 0), 1);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_WIDTH, 0), 1);
    EXPECT_EQ(want.GetIntParam(PARAM_RESV_WINDOW_HEIGHT, 0), 1);

    // SetWithAnimation false.
    startOptions.SetWithAnimation(false);
    want.SetParam(PARAM_RESV_WITH_ANIMATION, true);
    windowOptionsUtils->SetWindowPositionAndSize(want, nullptr, startOptions);
    EXPECT_TRUE(want.GetBoolParam(PARAM_RESV_WITH_ANIMATION, false));
    AbilityRequest abilityRequest;
    auto abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    auto callerToken = abilityRecord->GetToken();
    windowOptionsUtils->SetWindowPositionAndSize(want, callerToken, startOptions);
    EXPECT_FALSE(want.GetBoolParam(PARAM_RESV_WITH_ANIMATION, true)); // bundleName is ""
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0200 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: SetWindowPositionAndSize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService SetWindowPositionAndSize
 */
HWTEST_F(WindowOptionsUtilsTest, SetWindowPositionAndSize_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0300 start");
    Want want;
    WindowOptionsPara para;
    InitWantWithWindowOptions(want, para);
    auto windowOptionsUtils = std::make_shared<WindowOptionsUtils>();
    StartOptions startOptions;
    // SetWithAnimation always false.
    startOptions.SetWithAnimation(false);
    want.SetParam(PARAM_RESV_WITH_ANIMATION, true);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = "testBundle";
    AppExecFwk::ApplicationInfo applicationInfo;
    auto abilityRecord = std::make_shared<AbilityRecord>(want, abilityInfo, applicationInfo, -1);
    auto callerToken = abilityRecord->GetToken();
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = false;
    windowOptionsUtils->SetWindowPositionAndSize(want, callerToken, startOptions);
    EXPECT_TRUE(want.GetBoolParam(PARAM_RESV_WITH_ANIMATION, false));
    want.SetBundle(abilityInfo.bundleName);
    windowOptionsUtils->SetWindowPositionAndSize(want, callerToken, startOptions);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest SetWindowPositionAndSize_0300 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: UpdateWantToSetDisplayID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateWantToSetDisplayID
 */
HWTEST_F(WindowOptionsUtilsTest, UpdateWantToSetDisplayID_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest UpdateWantToSetDisplayID_001 start");
    Want want;
    sptr<IRemoteObject> callerToken;
    auto windowOptionsUtils = std::make_shared<WindowOptionsUtils>();
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    OHOS::AAFwk::WantParams params = want.GetParams();
    params.SetParam(PARAM_RESV_DISPLAY_ID, AAFwk::String::Box("1"));
    want.SetParams(params);
    windowOptionsUtils->UpdateWantToSetDisplayID(want, nullptr);
    EXPECT_EQ(want.GetStringParam(PARAM_RESV_DISPLAY_ID), "0");

    want.SetParam(PARAM_RESV_DISPLAY_ID, std::string("0"));
    AbilityRequest abilityRequest;
    abilityRequest.want.SetParam(PARAM_RESV_DISPLAY_ID, std::string("1"));
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    windowOptionsUtils->UpdateWantToSetDisplayID(want, launcher->GetToken());
    EXPECT_EQ(want.GetStringParam(PARAM_RESV_DISPLAY_ID), "1");

    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest UpdateWantToSetDisplayID_001 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: UpdateStartOptionsToSetDisplayID
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService UpdateStartOptionsToSetDisplayID
 */
HWTEST_F(WindowOptionsUtilsTest, UpdateStartOptionsToSetDisplayID_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest UpdateStartOptionsToSetDisplayID_001 start");
    sptr<IRemoteObject> callerToken;
    StartOptions startOptions;
    auto windowOptionsUtils = std::make_shared<WindowOptionsUtils>();
    OHOS::system::SetBoolParameter(START_OPTIONS_WITH_ANIMATION, true);
    windowOptionsUtils->UpdateStartOptionsToSetDisplayID(startOptions, callerToken);

    AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.name = "com.test.rootLauncher";
    abilityRequest.abilityInfo.bundleName = "com.test";
    abilityRequest.appInfo.isLauncherApp = true;
    abilityRequest.appInfo.name = "com.test";
    auto launcher = AbilityRecord::CreateAbilityRecord(abilityRequest);
    windowOptionsUtils->UpdateStartOptionsToSetDisplayID(startOptions, launcher->GetToken());
    EXPECT_NE(nullptr, windowOptionsUtils);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest UpdateStartOptionsToSetDisplayID_001 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: WindowModeMap
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService WindowModeMap
 */
HWTEST_F(WindowOptionsUtilsTest, WindowModeMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_001 start");
    WindowOptionsUtils  Window_;
    int32_t windowMode = MULTI_WINDOW_DISPLAY_FULLSCREEN;
    auto result = Window_.WindowModeMap(windowMode);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, AppExecFwk::SupportWindowMode::FULLSCREEN);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_001 end");
}

/*
 * Feature: WindowModeMap
 * Function: SetWindowPositionAndSize
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService WindowModeMap
 */
HWTEST_F(WindowOptionsUtilsTest, WindowModeMap_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_002 start");
    WindowOptionsUtils  Window_;
    int32_t windowMode = MULTI_WINDOW_DISPLAY_PRIMARY;
    auto result = Window_.WindowModeMap(windowMode);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, AppExecFwk::SupportWindowMode::SPLIT);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_002 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: WindowModeMap
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService WindowModeMap
 */
HWTEST_F(WindowOptionsUtilsTest, WindowModeMap_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_003 start");
    WindowOptionsUtils  Window_;
    int32_t windowMode = MULTI_WINDOW_DISPLAY_SECONDARY;
    auto result = Window_.WindowModeMap(windowMode);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, AppExecFwk::SupportWindowMode::SPLIT);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_003 end");
}

/*
 * Feature: WindowOptionsUtils
 * Function: WindowModeMap
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService WindowModeMap
 */
HWTEST_F(WindowOptionsUtilsTest, WindowModeMap_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_004 start");
    WindowOptionsUtils  Window_;
    int32_t windowMode = MULTI_WINDOW_DISPLAY_FLOATING;
    auto result = Window_.WindowModeMap(windowMode);
    EXPECT_TRUE(result.first);
    EXPECT_EQ(result.second, AppExecFwk::SupportWindowMode::FLOATING);
    TAG_LOGI(AAFwkTag::TEST, "WindowOptionsUtilsTest WindowModeMap_004 end");
}
} // namespace AAFwk
} // namespace OHOS
