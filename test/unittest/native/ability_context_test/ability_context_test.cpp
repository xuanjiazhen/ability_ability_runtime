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
#include "ability_context_impl.h"
#include "ability_connect_callback.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string TEST_BUNDLE_NAME = "com.example.test";
    const std::string TEST_ABILITY_NAME = "TestAbility";
}

class MockAbilityConnectCallback : public AbilityConnectCallback {
public:
    void OnAbilityConnectDone(const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject,
        int resultCode) override {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override {}
};

class AbilityContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityContextTest::SetUpTestCase()
{}

void AbilityContextTest::TearDownTestCase()
{}

void AbilityContextTest::SetUp()
{}

void AbilityContextTest::TearDown()
{}

/**
 * @tc.name: ConnectAppServiceExtensionAbility_0100
 * @tc.desc: Test ConnectAppServiceExtensionAbility with empty want
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(AbilityContextTest, ConnectAppServiceExtensionAbility_0100)
{
    AAFwk::Want want;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AbilityContextImpl context;
    ErrCode result = context.ConnectAppServiceExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ConnectAppServiceExtensionAbility_0200
 * @tc.desc: Test ConnectAppServiceExtensionAbility with normal want
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(AbilityContextTest, ConnectAppServiceExtensionAbility_0200)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AbilityContextImpl context;
    ErrCode result = context.ConnectAppServiceExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ConnectAppServiceExtensionAbility_0300
 * @tc.desc: Test ConnectAppServiceExtensionAbility with null callback
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(AbilityContextTest, ConnectAppServiceExtensionAbility_0300)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    AbilityContextImpl context;
    ErrCode result = context.ConnectAppServiceExtensionAbility(want, nullptr);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: ConnectAppServiceExtensionAbility_0400
 * @tc.desc: Test ConnectAppServiceExtensionAbility with invalid bundle name
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(AbilityContextTest, ConnectAppServiceExtensionAbility_0400)
{
    AAFwk::Want want;
    want.SetElementName("", TEST_ABILITY_NAME);
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AbilityContextImpl context;
    ErrCode result = context.ConnectAppServiceExtensionAbility(want, callback);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: ConnectAppServiceExtensionAbility_0500
 * @tc.desc: Test ConnectAppServiceExtensionAbility with invalid ability name
 * @tc.type: FUNC
 * @tc.require: issueI6K4G7
 */
TEST_F(AbilityContextTest, ConnectAppServiceExtensionAbility_0500)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, "");
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AbilityContextImpl context;
    ErrCode result = context.ConnectAppServiceExtensionAbility(want, callback);
    EXPECT_NE(result, ERR_OK);
}
} // namespace AbilityRuntime
} // namespace OHOS