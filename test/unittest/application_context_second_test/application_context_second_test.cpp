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
class ApplicationContextSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<ApplicationContext> context_ = nullptr;
    std::shared_ptr<MockContextImpl> mock_ = nullptr;
};

void ApplicationContextSecondTest::SetUpTestCase(void)
{}

void ApplicationContextSecondTest::TearDownTestCase(void)
{}

void ApplicationContextSecondTest::SetUp()
{
    context_ = std::make_shared<ApplicationContext>();
    mock_ = std::make_shared<MockContextImpl>();
}

void ApplicationContextSecondTest::TearDown()
{}

/**
 * @tc.number:SetConfiguration_0100
 * @tc.name: SetConfiguration
 * @tc.desc: SetConfiguration fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, SetConfiguration_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetConfiguration_0100 start";
    std::shared_ptr<AppExecFwk::Configuration> config = nullptr;
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->SetConfiguration(config);
    EXPECT_EQ(config, nullptr);
    GTEST_LOG_(INFO) << "SetConfiguration_0100 end";
}

/**
 * @tc.number:AppHasDarkRes_0100
 * @tc.name: AppHasDarkRes
 * @tc.desc: AppHasDarkRes fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, AppHasDarkRes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppHasDarkRes_0100 start";
    bool darkRes = true;
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->AppHasDarkRes(darkRes);
    EXPECT_EQ(darkRes, true);
    GTEST_LOG_(INFO) << "AppHasDarkRes_0100 end";
}

/**
 * @tc.number:RegisterProcessSecurityExit_0100
 * @tc.name: RegisterProcessSecurityExit
 * @tc.desc: RegisterProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, RegisterProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 start";
    AppProcessExitCallback appProcessExitCallback = [](const AAFwk::ExitReason &exitReason){};
    context_->contextImpl_= std::make_shared<AbilityRuntime::ContextImpl>();
    context_->RegisterProcessSecurityExit(appProcessExitCallback);
    EXPECT_NE(appProcessExitCallback, nullptr);
    GTEST_LOG_(INFO) << "RegisterProcessSecurityExit_0100 end";
}

/**
 * @tc.number:ProcessSecurityExit_0100
 * @tc.name: ProcessSecurityExit
 * @tc.desc: ProcessSecurityExit fail with no permission
 */
HWTEST_F(ApplicationContextSecondTest, ProcessSecurityExit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 start";
    AAFwk::ExitReason exitReason = { AAFwk::Reason::REASON_JS_ERROR, "Js Error." };
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->ProcessSecurityExit(exitReason);
    EXPECT_NE(exitReason.exitMsg, "");
    GTEST_LOG_(INFO) << "ProcessSecurityExit_0100 end";
}

/**
 * @tc.number:GetPluginExtensionInfo_0100
 * @tc.name: GetPluginExtensionInfo
 * @tc.desc: GetPluginExtensionInfo
 */
HWTEST_F(ApplicationContextSecondTest, GetPluginExtensionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPluginExtensionInfo_0100 start";
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto result = context_->contextImpl_->GetPluginExtensionInfo();
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "GetPluginExtensionInfo_0100 end";
}

/**
 * @tc.number:InitPluginExtensionInfo_0100
 * @tc.name: InitPluginExtensionInfo
 * @tc.desc: InitPluginExtensionInfo
 */
HWTEST_F(ApplicationContextSecondTest, InitPluginExtensionInfo_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitPluginExtensionInfo_0100 start";
    context_->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    context_->contextImpl_->InitPluginExtensionInfo(nullptr, "");
    EXPECT_EQ(context_->contextImpl_->GetPluginExtensionInfo(), nullptr);
    GTEST_LOG_(INFO) << "InitPluginExtensionInfo_0100 end";
}

/**
 * @tc.number: ApplicationContext_CreateModuleOrPluginContext_001
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleOrPluginContext_001, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";

    auto context = applicationContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleOrPluginContext_002
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleOrPluginContext_002, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string bundleName = "";
    std::string moduleName = "moduleName";

    auto context = applicationContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleOrPluginContext_003
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleOrPluginContext_003, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string bundleName = "com.example.hostBundleName";
    std::string moduleName = "";

    auto context = applicationContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleOrPluginContext_004
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleOrPluginContext_004, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string bundleName = "";
    std::string moduleName = "";

    auto context = applicationContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleOrPluginContext_005
 * @tc.name: CreateModuleOrPluginContext
 * @tc.desc: Test CreateModuleOrPluginContext.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleOrPluginContext_005, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::string bundleName = "";
    std::string moduleName = "";
    applicationContext->contextImpl_ = nullptr;

    auto context = applicationContext->CreateModuleOrPluginContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_001
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test ApplicationContext::CreateModuleContext(bundleName, moduleName) with null contextImpl_.
 *           Covers: contextImpl_ == nullptr -> return nullptr
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_001, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = nullptr;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_002
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test ApplicationContext::CreateModuleContext(bundleName, moduleName) with mock contextImpl_.
 *           Covers: contextImpl_ != nullptr -> delegate to ContextImpl::CreateModuleContext
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_002, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = mock_;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_003
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test ApplicationContext::CreateModuleContext(moduleName) single-arg with null contextImpl_.
 *           Covers: contextImpl_ == nullptr -> return nullptr (single-arg overload)
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_003, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = nullptr;
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_004
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test ApplicationContext::CreateModuleContext(moduleName) single-arg with mock contextImpl_.
 *           Covers: contextImpl_ != nullptr -> delegate (single-arg overload)
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_004, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = mock_;
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(moduleName);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_005
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext(bundleName, moduleName) with mock contextImpl_, empty bundleName.
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_005, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = mock_;
    std::string bundleName = "";
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_006
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext(bundleName, moduleName) with mock contextImpl_, empty moduleName.
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_006, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = mock_;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_007
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext(bundleName, moduleName) with mock contextImpl_, both empty.
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_007, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = mock_;
    std::string bundleName = "";
    std::string moduleName = "";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_NE(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_008
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext(bundleName, moduleName) with real ContextImpl (no BMS), returns nullptr.
 *           Covers: contextImpl_ != nullptr but inner GetBundleInfo fails -> nullptr
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_008, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(bundleName, moduleName);
    EXPECT_EQ(context, nullptr);
}

/**
 * @tc.number: ApplicationContext_CreateModuleContextSync_009
 * @tc.name: CreateModuleContextSync
 * @tc.desc: Test CreateModuleContext(moduleName) single-arg with real ContextImpl (no BMS).
 */
HWTEST_F(ApplicationContextSecondTest, ApplicationContext_CreateModuleContextSync_009, TestSize.Level1)
{
    auto applicationContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    applicationContext->contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    std::string moduleName = "moduleName";
    auto context = applicationContext->CreateModuleContext(moduleName);
    EXPECT_EQ(context, nullptr);
}
}  // namespace AbilityRuntime
}  // namespace OHOS