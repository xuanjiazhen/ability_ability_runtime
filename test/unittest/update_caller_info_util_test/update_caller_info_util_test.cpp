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
#include "update_caller_info_util.h"
#include "ability_manager_service.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* COMPONENT_STARTUP_NEW_RULES = "component.startup.newRules";
}

class UpdateCallerInfoUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);

    void SetWant(Want &want) const;
};

std::shared_ptr<AbilityRecord> UpdateCallerInfoUtilTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> UpdateCallerInfoUtilTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> UpdateCallerInfoUtilTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

void UpdateCallerInfoUtilTest::SetWant(Want &want) const
{
    want.SetParam(Want::PARAM_ABILITY_RECOVERY_RESTART, true);
    want.SetParam(Want::PARAM_RESV_CALLER_TOKEN, true);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, true);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, true);
    want.SetParam(Want::PARAMS_REAL_CALLER_KEY, true);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, true);
    want.SetParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, true);
    want.SetParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, true);
    want.SetParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, true);
    want.SetParam(COMPONENT_STARTUP_NEW_RULES, true);
}
void UpdateCallerInfoUtilTest::SetUpTestCase() {}

void UpdateCallerInfoUtilTest::TearDownTestCase() {}

void UpdateCallerInfoUtilTest::SetUp() {}

void UpdateCallerInfoUtilTest::TearDown() {}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerInfoFromCallerRecord_0001
 * @tc.desc: Test the state of QueryAllAutoStartupApplications
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerInfoFromCallerRecord_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    callerToken = abilityRecord->GetToken();

    updateCallerUtil->UpdateAsCallerInfoFromCallerRecord(want, callerToken);
    EXPECT_NE(callerToken, nullptr);
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerInfoFromToken_0001
 * @tc.desc: Test the state of UpdateCallerInfoFromToken
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfoFromToken_0001, TestSize.Level1)
{
    std::shared_ptr<UpdateCallerInfoUtil> updateCallerUtil = std::make_shared<UpdateCallerInfoUtil>();
    Want want;
    SetWant(want);
    updateCallerUtil->UpdateCallerInfoFromToken(want, nullptr);
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_TOKEN, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_UID, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_PID, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAMS_REAL_CALLER_KEY, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_RESV_CALLER_NATIVE_NAME, false));
    EXPECT_TRUE(want.GetBoolParam(COMPONENT_STARTUP_NEW_RULES, false));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false));

    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    auto callerToken = abilityRecord->GetToken();

    updateCallerUtil->UpdateCallerInfoFromToken(want, callerToken, true);
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_TOKEN, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_UID, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_PID, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAMS_REAL_CALLER_KEY, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_ABILITY_NAME, false));
    EXPECT_FALSE(want.GetBoolParam(Want::PARAM_RESV_CALLER_APP_CLONE_INDEX, false));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    EXPECT_FALSE(want.HasParameter(COMPONENT_STARTUP_NEW_RULES));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false));

    updateCallerUtil->UpdateCallerInfoFromToken(want, callerToken, false);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_ABILITY_RECOVERY_RESTART));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateCallerInfo_0001
 * @tc.desc: Test the state of UpdateCallerInfo
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateCallerInfo_0001, TestSize.Level1)
{
    UpdateCallerInfoUtil updateCallerUtil;
    Want want;
    SetWant(want);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    auto callerToken = abilityRecord->GetToken();
    updateCallerUtil.UpdateCallerInfo(want, callerToken, false);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    EXPECT_FALSE(want.HasParameter(COMPONENT_STARTUP_NEW_RULES));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_ABILITY_RECOVERY_RESTART));

    SetWant(want);
    updateCallerUtil.UpdateCallerInfo(want, callerToken, true);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    EXPECT_FALSE(want.HasParameter(COMPONENT_STARTUP_NEW_RULES));
    EXPECT_TRUE(want.GetBoolParam(Want::PARAM_ABILITY_RECOVERY_RESTART, false));
}

/**
 * @tc.name: UpdateCallerInfoUtilTest_UpdateAsCallerSourceInfo_0001
 * @tc.desc: Test the state of UpdateAsCallerSourceInfo
 * @tc.type: FUNC
 */
HWTEST_F(UpdateCallerInfoUtilTest, UpdateAsCallerSourceInfo_0001, TestSize.Level1)
{
    UpdateCallerInfoUtil updateCallerUtil;
    Want want;
    SetWant(want);
    auto abilityRecord = MockAbilityRecord(AbilityType::PAGE);
    auto callerToken = abilityRecord->GetToken();
    updateCallerUtil.UpdateAsCallerSourceInfo(want, callerToken, callerToken);
    EXPECT_FALSE(want.HasParameter(Want::PARAM_RESV_CALLER_NATIVE_NAME));
    EXPECT_FALSE(want.HasParameter(COMPONENT_STARTUP_NEW_RULES));
    EXPECT_FALSE(want.HasParameter(Want::PARAM_ABILITY_RECOVERY_RESTART));
}
} // namespace AAFwk
} // namespace OHOS
