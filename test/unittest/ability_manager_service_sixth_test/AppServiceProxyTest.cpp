/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <mutex>
#define private public
#define protected public
#include "ability_manager_service.h"
#include "app_utils.h"
#include "extension_record/base_extension_record.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"

using namespace testing::ext;
namespace OHOS::AAFwk {
class AppServiceProxyTest : public testing::Test {
public:
    void SetUp() override
    {
        service_ = std::make_shared<AbilityManagerService>();
        service_->isParamStartAbilityEnable_ = false;
        oldFlag_ = MyFlag::flag_;
        oldParameterValue_ = OHOS::system::GetBoolParameter("", false);
        MyFlag::flag_ = MyFlag::IS_SA_CALL;
        g_appServicePermissionState = {};
        g_appServicePermissionState.enabled = true;
        g_appServicePermissionState.targetTokenId = TARGET_TOKEN;
        caller_.tokenID = CALLER_TOKEN;
        caller_.uid = 20001001;
        caller_.bundleName = "com.example.appservice.caller";
        request_.appInfo.accessTokenId = TARGET_TOKEN;
        request_.uid = caller_.uid;
        request_.abilityInfo.visible = false;
        request_.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::APP_SERVICE;
        auto &utils = AppUtils::GetInstance();
        oldLoaded_ = utils.isSupportAppServiceExtension_.isLoaded;
        oldValue_ = utils.isSupportAppServiceExtension_.value;
        oldFull_ = utils.appServiceFullCapability_;
        utils.isSupportAppServiceExtension_.isLoaded = true;
        utils.isSupportAppServiceExtension_.value = true;
    }
    void TearDown() override
    {
        MyFlag::flag_ = oldFlag_;
        g_appServicePermissionState = {};
        auto &utils = AppUtils::GetInstance();
        utils.isSupportAppServiceExtension_.isLoaded = oldLoaded_;
        utils.isSupportAppServiceExtension_.value = oldValue_;
        utils.appServiceFullCapability_ = oldFull_;
        OHOS::system::SetBoolParameter("", oldParameterValue_);
        service_.reset();
    }
    static constexpr uint32_t CALLER_TOKEN = 111;
    static constexpr uint32_t TARGET_TOKEN = 222;
    std::shared_ptr<AbilityManagerService> service_;
    Security::AccessToken::HapTokenInfo caller_;
    AbilityRequest request_;
    int oldFlag_ = 0;
    bool oldLoaded_ = false;
    bool oldValue_ = false;
    bool oldParameterValue_ = false;
    DeviceConfiguration<bool> oldFull_ = {false, false};
};

/**
 * @tc.name: IndirectIdentity_001
 * @tc.desc: Verified application supplies the caller identity for permission checks.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, IndirectIdentity_001, TestSize.Level1)
{
    request_.specifiedFullTokenId = 333;
    ASSERT_EQ(ERR_OK, service_->CheckCallServiceExtensionPermission(request_, &caller_));
    EXPECT_EQ(CALLER_TOKEN, g_appServicePermissionState.verification.specifyTokenId);
    EXPECT_EQ(TARGET_TOKEN, g_appServicePermissionState.verification.accessTokenId);
    EXPECT_FALSE(g_appServicePermissionState.verification.visible);
}

/**
 * @tc.name: IndirectIdentity_002
 * @tc.desc: Existing identity selection is retained without the AppService proxy context.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, IndirectIdentity_002, TestSize.Level1)
{
    request_.specifiedFullTokenId = 333;
    request_.skillCallerTokenId = 444;
    ASSERT_EQ(ERR_OK, service_->CheckCallServiceExtensionPermission(request_));
    EXPECT_EQ(333U, g_appServicePermissionState.verification.specifyTokenId);
    EXPECT_EQ(444U, g_appServicePermissionState.verification.skillCallerTokenId);
}

/**
 * @tc.name: IndirectIdentity_003
 * @tc.desc: A failed application access check cannot fall back to the SA identity.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, IndirectIdentity_003, TestSize.Level1)
{
    g_appServicePermissionState.result = ABILITY_VISIBLE_FALSE_DENY_REQUEST;
    EXPECT_EQ(ABILITY_VISIBLE_FALSE_DENY_REQUEST,
        service_->CheckCallAppServiceExtensionPermission(request_, nullptr, true, &caller_));
}

/**
 * @tc.name: ComponentPermission_001
 * @tc.desc: A new-permission-only target is accepted with a verified caller.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, ComponentPermission_001, TestSize.Level1)
{
    g_appServicePermissionState.advancedPermission = true;
    EXPECT_EQ(ERR_OK, service_->CheckCallAppServiceExtensionPermission(request_, nullptr, true, &caller_));
}

/**
 * @tc.name: ComponentPermission_002
 * @tc.desc: The old permission remains sufficient.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, ComponentPermission_002, TestSize.Level1)
{
    g_appServicePermissionState.oldPermission = true;
    EXPECT_EQ(ERR_OK, service_->CheckCallAppServiceExtensionPermission(request_, nullptr, true, &caller_));
}

/**
 * @tc.name: ComponentPermission_003
 * @tc.desc: An SA proxy cannot bypass the target component permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, ComponentPermission_003, TestSize.Level1)
{
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE,
        service_->CheckCallAppServiceExtensionPermission(request_, nullptr, true, &caller_));
}

/**
 * @tc.name: ComponentPermission_004
 * @tc.desc: A running target still requires its component permission.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, ComponentPermission_004, TestSize.Level1)
{
    auto target = std::make_shared<BaseExtensionRecord>(request_.want, request_.abilityInfo, request_.appInfo);
    target->SetAbilityState(AbilityState::ACTIVE);
    request_.uid += 1;
    EXPECT_EQ(TARGET_ABILITY_NOT_SERVICE,
        service_->CheckCallAppServiceExtensionPermission(request_, target, true, &caller_));
    g_appServicePermissionState.advancedPermission = true;
    EXPECT_EQ(ERR_OK, service_->CheckCallAppServiceExtensionPermission(request_, target, true, &caller_));
}

/**
 * @tc.name: Capability_001
 * @tc.desc: The proxy cannot use the old native-SA early return to bypass the base capability.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, Capability_001, TestSize.Level1)
{
    AppUtils::GetInstance().isSupportAppServiceExtension_.value = false;
    g_appServicePermissionState.advancedPermission = true;
    EXPECT_EQ(ERR_CAPABILITY_NOT_SUPPORT,
        service_->CheckCallAppServiceExtensionPermission(request_, nullptr, true, &caller_));
    EXPECT_EQ(ERR_OK, service_->CheckCallAppServiceExtensionPermission(request_, nullptr, false));
}

/**
 * @tc.name: ValidateCaller_001
 * @tc.desc: Zero or malformed explicit identity is rejected.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, ValidateCaller_001, TestSize.Level1)
{
    IndirectCallerInfo info;
    Security::AccessToken::HapTokenInfo callerInfo;
    EXPECT_FALSE(service_->ValidateAppServiceIndirectCaller(info, callerInfo));
    info.tokenId = CALLER_TOKEN;
    info.callerPid = -1;
    EXPECT_FALSE(service_->ValidateAppServiceIndirectCaller(info, callerInfo));
}

/**
 * @tc.name: FullCapability_001
 * @tc.desc: The full capability is independent and cached for the service lifetime.
 * @tc.type: FUNC
 */
HWTEST_F(AppServiceProxyTest, FullCapability_001, TestSize.Level1)
{
    auto &utils = AppUtils::GetInstance();
    utils.appServiceFullCapability_.isLoaded = false;
    OHOS::system::SetBoolParameter("", false);
    EXPECT_FALSE(utils.IsAppServiceFullCapability());
    OHOS::system::SetBoolParameter("", true);
    EXPECT_FALSE(utils.IsAppServiceFullCapability());
    utils.appServiceFullCapability_.isLoaded = false;
    EXPECT_TRUE(utils.IsAppServiceFullCapability());
    utils.isSupportAppServiceExtension_.value = false;
    EXPECT_TRUE(utils.IsAppServiceFullCapability());
}

} // namespace OHOS::AAFwk
