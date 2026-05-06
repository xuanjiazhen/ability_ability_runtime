/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "ui_extension.h"
#include "ui_extension_context.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

class UIExtensionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionTest::SetUpTestCase(void)
{}

void UIExtensionTest::TearDownTestCase(void)
{}

void UIExtensionTest::SetUp()
{}

void UIExtensionTest::TearDown()
{}

/**
 * @tc.name: UIExtension_CreateModalUIExtension_0100
 * @tc.desc: Test CreateModalUIExtension when context is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0100 start";

    auto uiExtension = std::make_shared<UIExtension>();
    EXPECT_NE(uiExtension, nullptr);

    AAFwk::Want want;
    int result = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0100 end";
}

/**
 * @tc.name: UIExtension_CreateModalUIExtension_0200
 * @tc.desc: Test CreateModalUIExtension when handler_ is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0200, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0200 start";

    auto uiExtension = std::make_shared<UIExtension>();
    EXPECT_NE(uiExtension, nullptr);

    // Create and set context
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->AttachContext(context);

    // handler_ is nullptr by default
    EXPECT_EQ(uiExtension->handler_, nullptr);

    AAFwk::Want want;
    int result = uiExtension->CreateModalUIExtension(want);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0200 end";
}

/**
 * @tc.name: UIExtension_CreateModalUIExtension_0300
 * @tc.desc: Test CreateModalUIExtension when PostTask fails
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0300, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0300 start";

    auto uiExtension = std::make_shared<UIExtension>();
    EXPECT_NE(uiExtension, nullptr);

    // Create and set context
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->AttachContext(context);

    // Create handler with no event runner (PostTask will fail)
    auto handler = std::make_shared<AbilityHandler>(nullptr);
    uiExtension->handler_ = handler;

    AAFwk::Want want;
    int result = uiExtension->CreateModalUIExtension(want);
    // PostTask should fail when handler has no event runner
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0300 end";
}

/**
 * @tc.name: UIExtension_CreateModalUIExtension_0400
 * @tc.desc: Test CreateModalUIExtension success path
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionTest, UIExtension_CreateModalUIExtension_0400, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0400 start";

    auto uiExtension = std::make_shared<UIExtension>();
    EXPECT_NE(uiExtension, nullptr);

    // Create and set context
    auto context = std::make_shared<UIExtensionContext>();
    uiExtension->AttachContext(context);

    // Create event runner and handler
    auto runner = EventRunner::Create("UIExtensionTestRunner");
    auto handler = std::make_shared<AbilityHandler>(runner);
    uiExtension->handler_ = handler;

    AAFwk::Want want;
    int result = uiExtension->CreateModalUIExtension(want);
    // Should succeed (PostTask succeeds, but CreateModalUIExtensionWithApp will fail due to null uiContent)
    EXPECT_EQ(result, ERR_OK);

    // Give time for the task to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    GTEST_LOG_(INFO) << "UIExtension_CreateModalUIExtension_0400 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
