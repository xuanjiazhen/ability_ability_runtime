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
#include <gtest/gtest.h>
#include "want.h"
#include "cj_runtime.h"
#include "runtime.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "ability_local_record.h"
#include "ability_info.h"
#include "mock_ability_token.h"
#include "cj_ui_extension_base.h"
#include "ui_extension_context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class CJUIExtensionBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void CJUIExtensionBaseTest::SetUpTestCase(void)
{}

void CJUIExtensionBaseTest::TearDownTestCase(void)
{}

void CJUIExtensionBaseTest::SetUp()
{}

void CJUIExtensionBaseTest::TearDown()
{}

/**
 * @tc.number: Init_0100
 * @tc.name: Init
 * @tc.desc: Init.
 */
HWTEST_F(CJUIExtensionBaseTest, Init_0100, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "CJUIExtensionBaseTest";
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetAbilityInfo(nullptr);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr, nullptr, 0);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    EXPECT_EQ(uiExtensionBaseImpl->Init(record, application, handler, 0), nullptr);

    uiExtensionBaseImpl->SetAbilityInfo(info);
    uiExtensionBaseImpl->Init(record, application, handler, 0);
    int type = 5;
    uiExtensionBaseImpl->SetExtType(static_cast<CJExtensionAbilityType>(type));
    EXPECT_EQ(uiExtensionBaseImpl->Init(record, application, handler, 0), nullptr);

    type = 3;
    uiExtensionBaseImpl->SetExtType(static_cast<CJExtensionAbilityType>(type));
    uiExtensionBaseImpl->Init(record, application, handler, 0);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: OnStart_0100
 * @tc.name: OnStart
 * @tc.desc: OnStart.
 */
HWTEST_F(CJUIExtensionBaseTest, OnStart_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    sessionInfo->persistentId = 0;
    EXPECT_NE(sessionInfo, nullptr);

    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    EXPECT_NE(context, nullptr);

    AAFwk::LaunchParam launchParam;
    launchParam.launchReason = LaunchReason::LAUNCHREASON_START_ABILITY;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->OnStart(want, launchParam, sessionInfo);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: OnStop_0100
 * @tc.name: OnStop
 * @tc.desc: OnStop.
 */
HWTEST_F(CJUIExtensionBaseTest, OnStop_0100, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "CJUIExtensionBaseTest";
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetAbilityInfo(info);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr, nullptr, 0);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    int type = 3;
    uiExtensionBaseImpl->SetExtType(static_cast<CJExtensionAbilityType>(type));
    uiExtensionBaseImpl->Init(record, application, handler, 0);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->OnStop();
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
    TAG_LOGD(AAFwkTag::TEST, "OnStart_0100 end");
}


/**
 * @tc.number: OnStop_0200
 * @tc.name: OnStop
 * @tc.desc: OnStop.
 */
HWTEST_F(CJUIExtensionBaseTest, OnStop_0200, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "CJUIExtensionBaseTest";
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetAbilityInfo(info);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr, nullptr, 0);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    int type = 3;
    uiExtensionBaseImpl->SetExtType(static_cast<CJExtensionAbilityType>(type));
    uiExtensionBaseImpl->Init(record, application, handler, 0);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    bool isAsyncCallback = true;
    uiExtensionBaseImpl->OnStop(nullptr, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);

    isAsyncCallback = true;
    auto *callbackInfo = AbilityTransactionCallbackInfo<>::Create();
    uiExtensionBaseImpl->OnStop(callbackInfo, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);
}

/**
 * @tc.number: OnStopCallBack_0100
 * @tc.name: OnStopCallBack
 * @tc.desc: OnStopCallBack.
 */
HWTEST_F(CJUIExtensionBaseTest, OnStopCallBack_0100, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "CJUIExtensionBaseTest";
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, nullptr, nullptr, 0);
    std::shared_ptr<AppExecFwk::OHOSApplication> application = std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    int type = 3;
    uiExtensionBaseImpl->SetExtType(static_cast<CJExtensionAbilityType>(type));
    uiExtensionBaseImpl->SetAbilityInfo(info);
    uiExtensionBaseImpl->Init(record, application, handler, 0);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->SetContext(nullptr);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    EXPECT_NE(context, nullptr);

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);

    context->SetToken(token);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->OnStopCallBack();
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: OnCommandWindow_0100
 * @tc.name: OnCommandWindow
 * @tc.desc: OnCommandWindow.
 */
HWTEST_F(CJUIExtensionBaseTest, OnCommandWindow_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->OnCommandWindow(want, nullptr, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 0;
    EXPECT_NE(session, nullptr);

    uiExtensionBaseImpl->OnCommandWindow(want, session, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->OnCommandWindow(want, session, AAFwk::WIN_CMD_BACKGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: ForegroundWindowWithInsightIntent_0100
 * @tc.name: ForegroundWindowWithInsightIntent
 * @tc.desc: ForegroundWindowWithInsightIntent.
 */
HWTEST_F(CJUIExtensionBaseTest, ForegroundWindowWithInsightIntent_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    EXPECT_FALSE(uiExtensionBaseImpl->ForegroundWindowWithInsightIntent(want, nullptr, false));

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);
    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    EXPECT_NE(context, nullptr);
    uiExtensionBaseImpl->SetContext(context);
    EXPECT_TRUE(uiExtensionBaseImpl->ForegroundWindowWithInsightIntent(want, session, false));
}

/**
 * @tc.number: ForegroundWindow_0100
 * @tc.name: ForegroundWindow
 * @tc.desc: ForegroundWindow.
 */
HWTEST_F(CJUIExtensionBaseTest, ForegroundWindow_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->ForegroundWindow(want, nullptr);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);
    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    uiExtensionBaseImpl->ForegroundWindow(want, session);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: BackgroundWindow_0100
 * @tc.name: BackgroundWindow
 * @tc.desc: BackgroundWindow.
 */
HWTEST_F(CJUIExtensionBaseTest, BackgroundWindow_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->BackgroundWindow(nullptr);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);

    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->BackgroundWindow(session);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    uiExtensionBaseImpl->BackgroundWindow(session);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: DestroyWindow_0100
 * @tc.name: DestroyWindow
 * @tc.desc: DestroyWindow.
 */
HWTEST_F(CJUIExtensionBaseTest, DestroyWindow_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->DestroyWindow(nullptr);
 
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);

    uiExtensionBaseImpl->DestroyWindow(session);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    uiExtensionBaseImpl->DestroyWindow(session);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: DOnCommandWindowDone_0100
 * @tc.name: OnCommandWindowDone
 * @tc.desc: OnCommandWindowDone.
 */
HWTEST_F(CJUIExtensionBaseTest, DOnCommandWindowDone_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->OnCommandWindowDone(nullptr, AAFwk::WindowCommand::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);

    uiExtensionBaseImpl->uiWindowMap_.clear();
    uiExtensionBaseImpl->OnCommandWindowDone(session, AAFwk::WindowCommand::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    uiExtensionBaseImpl->foregroundWindows_.clear();
    uiExtensionBaseImpl->OnCommandWindowDone(session, AAFwk::WindowCommand::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->foregroundWindows_.insert(session->persistentId);
    uiExtensionBaseImpl->OnCommandWindowDone(session, AAFwk::WindowCommand::WIN_CMD_FOREGROUND);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: HandleSessionCreate_0100
 * @tc.name: HandleSessionCreate
 * @tc.desc: HandleSessionCreate.
 */
HWTEST_F(CJUIExtensionBaseTest, HandleSessionCreate_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    EXPECT_FALSE(uiExtensionBaseImpl->HandleSessionCreate(want, nullptr));

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);
    uiExtensionBaseImpl->uiWindowMap_.clear();
    uiExtensionBaseImpl->SetContext(nullptr);
    EXPECT_FALSE(uiExtensionBaseImpl->HandleSessionCreate(want, session));

    sptr<Rosen::Window> window = new Rosen::Window();
    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    context->SetWindow(nullptr);
    context->SetAbilityInfo(std::make_shared<AppExecFwk::AbilityInfo>());
    uiExtensionBaseImpl->SetContext(context);
    EXPECT_TRUE(uiExtensionBaseImpl->HandleSessionCreate(want, session));
}

/**
 * @tc.number: PostInsightIntentExecuted_0100
 * @tc.name: PostInsightIntentExecuted
 * @tc.desc: PostInsightIntentExecuted.
 */
HWTEST_F(CJUIExtensionBaseTest, PostInsightIntentExecuted_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    AppExecFwk::InsightIntentExecuteResult result;
    uiExtensionBaseImpl->PostInsightIntentExecuted(session, result, false);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    uiExtensionBaseImpl->PostInsightIntentExecuted(session, result, true);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);
}

/**
 * @tc.number: OnInsightIntentExecuteDone_0100
 * @tc.name: OnInsightIntentExecuteDone
 * @tc.desc: OnInsightIntentExecuteDone.
 */
HWTEST_F(CJUIExtensionBaseTest, OnInsightIntentExecuteDone_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    AppExecFwk::InsightIntentExecuteResult result;
    uiExtensionBaseImpl->OnInsightIntentExecuteDone(nullptr, result);
    EXPECT_TRUE(uiExtensionBaseImpl != nullptr);

    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    sptr<Rosen::Window> window = new Rosen::Window();
    uiExtensionBaseImpl->uiWindowMap_[session->uiExtensionComponentId] = window;
    result.result = std::make_shared<WantParams>();
    uiExtensionBaseImpl->OnInsightIntentExecuteDone(session, result);
    EXPECT_TRUE(result.result != nullptr);
}

/**
 * @tc.number: OnConfigurationUpdated_0100
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: OnConfigurationUpdated.
 */
HWTEST_F(CJUIExtensionBaseTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    Configuration configuration;
    uiExtensionBaseImpl->SetContext(nullptr);
    uiExtensionBaseImpl->OnConfigurationUpdated(configuration);
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->OnConfigurationUpdated(configuration);
    EXPECT_TRUE(uiExtensionBaseImpl->context_ != nullptr);
}

/**
 * @tc.number: OnForeground_0100
 * @tc.name: OnForeground
 * @tc.desc: OnForeground.
 */
HWTEST_F(CJUIExtensionBaseTest, OnForeground_0100, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ohos.insightIntent.executeParam.name", true);
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    sptr<AAFwk::SessionInfo> session = new (std::nothrow) AAFwk::SessionInfo();
    session->persistentId = 1;
    session->uiExtensionComponentId = 1;
    EXPECT_NE(session, nullptr);

    uiExtensionBaseImpl->OnForeground(want, session);
    EXPECT_FALSE(uiExtensionBaseImpl->ForegroundWindowWithInsightIntent(want, session, true));
}

/**
 * @tc.number: OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: OnAbilityResult.
 */
HWTEST_F(CJUIExtensionBaseTest, OnAbilityResult_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(nullptr);
    int32_t requestCode = 0;
    int32_t resultCode = 0;
    Want want;
    uiExtensionBaseImpl->OnAbilityResult(requestCode, resultCode, want);
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    uiExtensionBaseImpl->abilityResultListeners_ = nullptr;
    uiExtensionBaseImpl->OnAbilityResult(requestCode, resultCode, want);
    EXPECT_TRUE(uiExtensionBaseImpl->abilityResultListeners_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    uiExtensionBaseImpl->OnAbilityResult(requestCode, resultCode, want);
    uiExtensionBaseImpl->OnBackground();
    EXPECT_TRUE(uiExtensionBaseImpl->context_ != nullptr);
}

/**
 * @tc.number: ConfigurationUpdated_0100
 * @tc.name: ConfigurationUpdated
 * @tc.desc: ConfigurationUpdated.
 */
HWTEST_F(CJUIExtensionBaseTest, ConfigurationUpdated_0100, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(nullptr);
    uiExtensionBaseImpl->ConfigurationUpdated();
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    context->SetConfiguration(nullptr);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->ConfigurationUpdated();
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetConfiguration() == nullptr);

    context->SetConfiguration(config);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->ConfigurationUpdated();
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetConfiguration() != nullptr);
}

/**
 * @tc.number: OnDisplayInfoChange_0100
 * @tc.name: OnDisplayInfoChange
 * @tc.desc: OnDisplayInfoChange.
 */
HWTEST_F(CJUIExtensionBaseTest, OnDisplayInfoChange_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    uint64_t displayId = 0;
    float density = 0.8;
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(nullptr);
    uiExtensionBaseImpl->OnDisplayInfoChange(token, displayId, density, Rosen::DisplayOrientation::PORTRAIT);
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    context->SetConfiguration(nullptr);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->OnDisplayInfoChange(token, displayId, density, Rosen::DisplayOrientation::PORTRAIT);
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetConfiguration() == nullptr);

    context->SetConfiguration(config);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->OnDisplayInfoChange(token, displayId, density, Rosen::DisplayOrientation::PORTRAIT);
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetConfiguration() != nullptr);
}

/**
 * @tc.number: RegisterDisplayInfoChangedListener_0100
 * @tc.name: RegisterDisplayInfoChangedListener
 * @tc.desc: RegisterDisplayInfoChangedListener.
 */
HWTEST_F(CJUIExtensionBaseTest, RegisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(nullptr);
    uiExtensionBaseImpl->RegisterDisplayInfoChangedListener();
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    context->SetToken(token);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->RegisterDisplayInfoChangedListener();
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetToken() != nullptr);
}

/**
 * @tc.number: UnregisterDisplayInfoChangedListener_0100
 * @tc.name: UnregisterDisplayInfoChangedListener
 * @tc.desc: UnregisterDisplayInfoChangedListener.
 */
HWTEST_F(CJUIExtensionBaseTest, UnregisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);

    std::shared_ptr<CJUIExtensionBase> uiExtensionBaseImpl = std::make_shared<CJUIExtensionBase>(runtime);
    uiExtensionBaseImpl->SetContext(nullptr);
    uiExtensionBaseImpl->UnregisterDisplayInfoChangedListener();
    EXPECT_TRUE(uiExtensionBaseImpl->context_ == nullptr);

    std::shared_ptr<UIExtensionContext> uiExtensionContext = std::make_shared<UIExtensionContext>();
    auto context = Context::ConvertTo<UIExtensionContext>(uiExtensionContext);
    context->SetToken(token);
    uiExtensionBaseImpl->SetContext(context);
    uiExtensionBaseImpl->UnregisterDisplayInfoChangedListener();
    EXPECT_TRUE(uiExtensionBaseImpl->context_->GetToken() != nullptr);
}
} // namespace AbilityRuntime
} // namespace OHOS