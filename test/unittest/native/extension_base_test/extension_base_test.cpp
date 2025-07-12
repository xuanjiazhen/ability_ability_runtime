/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "extension_base.h"
#include "element_name.h"
#include "extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string TEST_REQUEST_ID = "test_request_id";
    const std::string TEST_MESSAGE = "test_message";
    const std::string TEST_DEVICE_ID = "test_device_id";
    const std::string TEST_BUNDLE_NAME = "test_bundle_name";
    const std::string TEST_ABILITY_NAME = "test_ability_name";
}

class MockExtensionContext : public ExtensionContext {
public:
    MockExtensionContext() = default;
    virtual ~MockExtensionContext() = default;
};

class ExtensionBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ExtensionBaseTest::SetUpTestCase()
{}

void ExtensionBaseTest::TearDownTestCase()
{}

void ExtensionBaseTest::SetUp()
{}

void ExtensionBaseTest::TearDown()
{}

/**
 * @tc.name: OnExtensionAbilityRequestFailure_0100
 * @tc.desc: Test OnExtensionAbilityRequestFailure with empty parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestFailure_0100)
{
    AppExecFwk::ElementName element;
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestFailure("", element, "");
}

/**
 * @tc.name: OnExtensionAbilityRequestFailure_0200
 * @tc.desc: Test OnExtensionAbilityRequestFailure with normal parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestFailure_0200)
{
    AppExecFwk::ElementName element(TEST_DEVICE_ID, TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestFailure(TEST_REQUEST_ID, element, TEST_MESSAGE);
}

/**
 * @tc.name: OnExtensionAbilityRequestFailure_0300
 * @tc.desc: Test OnExtensionAbilityRequestFailure with long parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestFailure_0300)
{
    std::string longRequestId(1024, 'a');
    std::string longMessage(1024, 'b');
    AppExecFwk::ElementName element(TEST_DEVICE_ID, TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestFailure(longRequestId, element, longMessage);
}

/**
 * @tc.name: OnExtensionAbilityRequestSuccess_0100
 * @tc.desc: Test OnExtensionAbilityRequestSuccess with empty parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestSuccess_0100)
{
    AppExecFwk::ElementName element;
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestSuccess("", element, "");
}

/**
 * @tc.name: OnExtensionAbilityRequestSuccess_0200
 * @tc.desc: Test OnExtensionAbilityRequestSuccess with normal parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestSuccess_0200)
{
    AppExecFwk::ElementName element(TEST_DEVICE_ID, TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestSuccess(TEST_REQUEST_ID, element, TEST_MESSAGE);
}

/**
 * @tc.name: OnExtensionAbilityRequestSuccess_0300
 * @tc.desc: Test OnExtensionAbilityRequestSuccess with long parameters
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(ExtensionBaseTest, OnExtensionAbilityRequestSuccess_0300)
{
    std::string longRequestId(1024, 'a');
    std::string longMessage(1024, 'b');
    AppExecFwk::ElementName element(TEST_DEVICE_ID, TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    ExtensionBase<MockExtensionContext> extension;
    extension.OnExtensionAbilityRequestSuccess(longRequestId, element, longMessage);
}
} // namespace AbilityRuntime
} // namespace OHOS