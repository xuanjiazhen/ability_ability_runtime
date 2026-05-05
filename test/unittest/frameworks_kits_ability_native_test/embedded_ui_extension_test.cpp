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
#include "extension_base.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "embedded_ui_extension.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;

class EmbeddedUIExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void EmbeddedUIExtensionTest::SetUpTestCase(void)
{}

void EmbeddedUIExtensionTest::TearDownTestCase(void)
{}

void EmbeddedUIExtensionTest::SetUp(void)
{}

void EmbeddedUIExtensionTest::TearDown(void)
{}

/**
 * @tc.number: EmbeddedUI_Extension_0100
 * @tc.name: Create
 * @tc.desc: The runtime is nullptr, and the verification of create succeeds.
 */
HWTEST_F(EmbeddedUIExtensionTest, EmbeddedUI_Extension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0100 start";
    std::unique_ptr<Runtime> runtime;
    auto actionExtension = EmbeddedUIExtension::Create(runtime);
    EXPECT_TRUE(actionExtension != nullptr);
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0100 end";
}

/**
 * @tc.number: EmbeddedUI_Extension_0200
 * @tc.name: Create
 * @tc.desc: The language is js, and the verification of create is successful.
 */
HWTEST_F(EmbeddedUIExtensionTest, EmbeddedUI_Extension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0200 start";
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    auto actionExtension = EmbeddedUIExtension::Create(runtime);
    EXPECT_TRUE(actionExtension != nullptr);
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0200 end";
}

/**
 * @tc.number: EmbeddedUI_Extension_0300
 * @tc.name: Init
 * @tc.desc: Validation initialization succeeded.
 */
HWTEST_F(EmbeddedUIExtensionTest, EmbeddedUI_Extension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0300 start";
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityInfo, token, nullptr, 0);
    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto contextImpl = std::make_shared<ContextImpl>();
    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    Runtime::Options options;
    std::unique_ptr<Runtime> runtime = Runtime::Create(options);
    auto actionExtension = EmbeddedUIExtension::Create(runtime);
    actionExtension->Init(record, application, handler, token);
    EXPECT_TRUE(actionExtension != nullptr);
    GTEST_LOG_(INFO) << "EmbeddedUI_Extension_0300 end";
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0100
 * @tc.name: CreateModalUIExtension when context is null
 * @tc.desc: CreateModalUIExtension returns ERR_INVALID_VALUE when context is null
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(EmbeddedUIExtensionTest, UIExtension_CreateModalUIExtension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0100 start";

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "TestUIExtension";
    info->type = AppExecFwk::AbilityType::EXTENSION;

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    auto uiExtension = std::make_shared<UIExtension>();
    ASSERT_NE(uiExtension, nullptr);

    // Context should be null initially
    EXPECT_EQ(uiExtension->GetContext(), nullptr);

    AAFwk::Want want;
    AppExecFwk::ElementName element("device", "com.test.modal", "entry", "ModalExtension");
    want.SetElement(element);

    int result = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0100 end";
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0200
 * @tc.name: CreateModalUIExtension when handler_ is null
 * @tc.desc: CreateModalUIExtension returns ERR_INVALID_VALUE when handler_ is null
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(EmbeddedUIExtensionTest, UIExtension_CreateModalUIExtension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0200 start";

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "TestUIExtension";
    info->type = AppExecFwk::AbilityType::EXTENSION;

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    auto uiExtension = std::make_shared<UIExtension>();
    ASSERT_NE(uiExtension, nullptr);

    // Set context but not handler
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->context_ = context;

    EXPECT_NE(uiExtension->GetContext(), nullptr);
    EXPECT_EQ(uiExtension->handler_, nullptr);

    AAFwk::Want want;
    AppExecFwk::ElementName element("device", "com.test.modal", "entry", "ModalExtension");
    want.SetElement(element);

    int result = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0200 end";
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0300
 * @tc.name: CreateModalUIExtension Init sets handler_
 * @tc.desc: CreateModalUIExtension Init properly stores handler in handler_ member
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(EmbeddedUIExtensionTest, UIExtension_CreateModalUIExtension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0300 start";

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "TestUIExtension";
    info->type = AppExecFwk::AbilityType::EXTENSION;

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    std::shared_ptr<EventRunner> runner = EventRunner::Create(info->name);
    auto handler = std::make_shared<AbilityHandler>(runner);

    auto uiExtension = std::make_shared<UIExtension>();
    ASSERT_NE(uiExtension, nullptr);

    // Before Init, handler_ should be null
    EXPECT_EQ(uiExtension->handler_, nullptr);

    uiExtension->Init(record, application, handler, token);

    // After Init, handler_ should be set
    EXPECT_NE(uiExtension->handler_, nullptr);
    EXPECT_EQ(uiExtension->handler_, handler);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0300 end";
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0400
 * @tc.name: CreateModalUIExtension with valid context and handler
 * @tc.desc: CreateModalUIExtension posts task to handler when context and handler are valid
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(EmbeddedUIExtensionTest, UIExtension_CreateModalUIExtension_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0400 start";

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "TestUIExtension";
    info->type = AppExecFwk::AbilityType::EXTENSION;

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    auto uiExtension = std::make_shared<UIExtension>();
    ASSERT_NE(uiExtension, nullptr);

    std::shared_ptr<EventRunner> runner = EventRunner::Create(info->name);
    auto handler = std::make_shared<AbilityHandler>(runner);

    uiExtension->Init(record, application, handler, token);

    // Set context
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->context_ = context;

    ASSERT_NE(uiExtension->GetContext(), nullptr);
    ASSERT_NE(uiExtension->handler_, nullptr);

    AAFwk::Want want;
    AppExecFwk::ElementName element("device", "com.test.modal", "entry", "ModalExtension");
    want.SetElement(element);

    int result = uiExtension->CreateModalUIExtension(want);

    // Task should be posted
    EXPECT_EQ(result, ERR_OK);

    // Give task time to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0400 end";
}

/**
 * @tc.number: UIExtension_CreateModalUIExtension_0500
 * @tc.name: CreateModalUIExtension PostTask fails
 * @tc.desc: CreateModalUIExtension returns ERR_INVALID_VALUE when PostTask fails
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(EmbeddedUIExtensionTest, UIExtension_CreateModalUIExtension_0500, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0500 start";

    auto application = std::make_shared<AppExecFwk::OHOSApplication>();
    auto info = std::make_shared<AppExecFwk::AbilityInfo>();
    info->name = "TestUIExtension";
    info->type = AppExecFwk::AbilityType::EXTENSION;

    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    auto record = std::make_shared<AppExecFwk::AbilityLocalRecord>(info, token, nullptr, 0);

    // Create handler with null EventRunner to cause PostTask failure
    std::shared_ptr<EventRunner> runner;
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);

    auto uiExtension = std::make_shared<UIExtension>();
    ASSERT_NE(uiExtension, nullptr);

    uiExtension->Init(record, application, handler, token);

    // Set context
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->context_ = context;

    // Verify PostTask will fail with null EventRunner
    auto testHandler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    EXPECT_FALSE(testHandler->PostTask([] {}));

    // Set the invalid handler to test PostTask failure path
    uiExtension->handler_ = testHandler;

    ASSERT_NE(uiExtension->GetContext(), nullptr);

    AAFwk::Want want;
    AppExecFwk::ElementName element("device", "com.test.modal", "entry", "ModalExtension");
    want.SetElement(element);

    int result = uiExtension->CreateModalUIExtension(want);

    // Should return ERR_INVALID_VALUE when PostTask fails
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0500 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
