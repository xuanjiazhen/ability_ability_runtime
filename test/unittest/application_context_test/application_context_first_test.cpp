/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "application_context.h"
#undef private
#include "mock_ability_token.h"
#include "mock_application_state_change_callback.h"
#include "mock_context_impl.h"
#include "running_process_info.h"
#include "want.h"
#include "configuration_convertor.h"
#include "ability_manager_errors.h"
#include "exit_reason.h"
#include "configuration.h"
using namespace testing::ext;


namespace OHOS {
namespace AbilityRuntime {
class ApplicationContextFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<MockContextImpl> mock_ = nullptr;
};

void ApplicationContextFirstTest::SetUpTestCase(void)
{}

void ApplicationContextFirstTest::TearDownTestCase(void)
{}

void ApplicationContextFirstTest::SetUp()
{
    context_ = std::make_shared<ApplicationContext>();
    mock_ = std::make_shared<MockContextImpl>();
}

void ApplicationContextFirstTest::TearDown()
{}

/**
 * @tc.number: UnregisterAbilityLifecycleCallback_0100
 * @tc.name: UnregisterAbilityLifecycleCallback
 * @tc.desc: UnRegister Ability Lifecycle Callback
 */
HWTEST_F(ApplicationContextFirstTest, UnregisterAbilityLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterAbilityLifecycleCallback_0100 start";
    context_->callbacks_.clear();
    std::shared_ptr<AbilityLifecycleCallback> abilityLifecycleCallback = nullptr;
    context_->RegisterAbilityLifecycleCallback(abilityLifecycleCallback);
    EXPECT_TRUE(context_->IsAbilityLifecycleCallbackEmpty());
    context_->UnregisterAbilityLifecycleCallback(abilityLifecycleCallback);
    EXPECT_TRUE(context_->callbacks_.empty());
    GTEST_LOG_(INFO) << "UnregisterAbilityLifecycleCallback_0100 end";
}

/**
 * @tc.number: IsModuleExist_0100
 * @tc.name: IsModuleExist
 * @tc.desc: Is Module Exist
 */
HWTEST_F(ApplicationContextFirstTest, IsModuleExist_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsModuleExist_0100 start";
    context_->AttachContextImpl(mock_);
    auto ret = context_->IsModuleExist("");
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsModuleExist_0100 end";
}

/**
 * @tc.number: IsModuleExist_0100
 * @tc.name: IsModuleExist
 * @tc.desc: Is Module Exist
 */
HWTEST_F(ApplicationContextFirstTest, IsModuleExist_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsModuleExist_0200 start";
    context_->AttachContextImpl(mock_);
    std::string moduleName = "moduleName";
    auto ret = context_->IsModuleExist(moduleName);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsModuleExist_0200 end";
}

/**
 * @tc.number: SetConfiguration_0100
 * @tc.name: SetConfiguration
 * @tc.desc: Test SetConfiguration method of ApplicationContext
 */
HWTEST_F(ApplicationContextFirstTest, SetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetConfiguration_0100 start";
    context_ = std::make_shared<ApplicationContext>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    context_->SetConfiguration(config);
    auto ret = context_->GetConfiguration();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "SetConfiguration_0100 end";
}

/**
 * @tc.number: SetConfiguration_0200
 * @tc.name: SetConfiguration
 * @tc.desc: Test SetConfiguration method of ApplicationContext
 */
HWTEST_F(ApplicationContextFirstTest, SetConfiguration_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetConfiguration_0200 start";
    context_ = std::make_shared<ApplicationContext>();
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    context_->SetConfiguration(config);
    auto ret = context_->GetConfiguration();
    EXPECT_EQ(ret, nullptr);
    GTEST_LOG_(INFO) << "SetConfiguration_0200 end";
}

/**
 * @tc.number: SetColorMode_0200
 * @tc.name: SetColorMode with invalid mode
 * @tc.desc: Verify  with negative value
 */
HWTEST_F(ApplicationContextFirstTest, SetColorMode_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetColorMode_0200 start";
    int32_t invalidMode = -1;
    context_->SetColorMode(invalidMode);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "SetColorMode_0200 end";
}

/**
 * @tc.number: SetColorMode_0300
 * @tc.name: SetColorMode with max value
 * @tc.desc: Verify SetColorMode with INT_MAX
 */
HWTEST_F(ApplicationContextFirstTest, SetColorMode_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetColorMode_0300 start";
    int32_t maxMode = INT_MAX;
    context_->SetColorMode(maxMode);
    EXPECT_TRUE(context_ != nullptr);
    GTEST_LOG_(INFO) << "SetColorMode_0300 end";
}

/**
 * @tc.number: RegisterAppFontObserver_0100
 * @tc.name: RegisterAppFontObserver
 * @tc.desc: Test RegisterAppFontObserver method of ApplicationContext
 */
HWTEST_F(ApplicationContextFirstTest, RegisterAppFontObserver_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAppFontObserver_0100 start";
    EXPECT_NE(context_, nullptr);
    AppConfigUpdateCallback appFontCallback = [](const AppExecFwk::Configuration &config) {};
    context_->appFontCallback_ = nullptr;
    context_->RegisterAppFontObserver(appFontCallback);
    EXPECT_TRUE(context_->appFontCallback_ != nullptr);
    GTEST_LOG_(INFO) << "RegisterAppFontObserver_0100 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS