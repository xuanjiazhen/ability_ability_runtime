/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "session_info.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionPreloadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionPreloadTest::SetUpTestCase(void)
{}

void UIExtensionPreloadTest::TearDownTestCase(void)
{}

void UIExtensionPreloadTest::SetUp()
{}

void UIExtensionPreloadTest::TearDown()
{}

/**
 * @tc.name: PermissionCheck_0100
 * @tc.desc: permission check test.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto ret = AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(providerWant, hostBundleName,
        DEFAULT_INVAL_VALUE);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PermissionCheck_002
 * @tc.desc: Call PreloadUIExtensionAbilityWithId with invalid userId, should return error.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");

    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);

    std::string hostBundleName = "com.ohos.uiextensionuser";
    int32_t preloadId = 0;

    auto ret = AbilityManagerClient::GetInstance()->PreloadUIExtensionAbilityWithId(
        providerWant, hostBundleName, preloadId, DEFAULT_INVAL_VALUE, 1234);

    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PermissionCheck_003
 * @tc.desc: Call UnPreloadUIExtensionAbility with invalid userId, should return error.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");

    int32_t extensionAbilityId = 100; 
    int32_t invalidUserId = DEFAULT_INVAL_VALUE;
    auto ret = AbilityManagerClient::GetInstance()->UnPreloadUIExtensionAbility(extensionAbilityId, invalidUserId);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PermissionCheck_004
 * @tc.desc: Call ClearAllPreloadUIExtensionAbility with empty hostBundleName, should return error.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");

    std::string emptyHostBundleName = "";
    int32_t userId = AppExecFwk::Constants::DEFAULT_USERID; 
    auto ret = AbilityManagerClient::GetInstance()->ClearAllPreloadUIExtensionAbility(emptyHostBundleName, userId);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PermissionCheck_005
 * @tc.desc: Call RegisterPreloadUIExtensionHostClient with null callerToken, should return error.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");

    sptr<IRemoteObject> nullCallerToken = nullptr;
    std::string hostBundleName = "com.ohos.test";
    int32_t hostPid = 1234;
    auto ret = AbilityManagerClient::GetInstance()->RegisterPreloadUIExtensionHostClient(
        nullCallerToken, hostBundleName, hostPid);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PermissionCheck_006
 * @tc.desc: Call UnRegisterPreloadUIExtensionHostClient with invalid hostBundleName, should return error.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");

    std::string hostBundleName = "com.ohos.test"; 
    int32_t hostPid = 1234;
    auto ret = AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(hostBundleName, hostPid);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

} // namespace AAFwk
} // namespace OHOS
