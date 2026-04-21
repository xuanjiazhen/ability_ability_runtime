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

#include "modular_object_utils.h"
#include "ability_manager_errors.h"
#include "mock_flag.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
void ResetFlags()
{
    MockFlag::isSupportModularObjectExtension = true;
    MockFlag::callingUid = 1000;
    MockFlag::callingPid = 1234;
    MockFlag::getRunningProcessInfoRet = 0;
    MockFlag::processState = 2; // APP_STATE_FOREGROUND
    MockFlag::isPreForeground = false;
    MockFlag::isDeveloperMode = false;
    MockFlag::queryDataRet = 0;
    MockFlag::extensionFound = true;
    MockFlag::extensionDisabled = false;
    MockFlag::bundleMgrHelperNull = false;
    MockFlag::getNameAndIndexRet = 0;
    MockFlag::getOsAccountRet = 0;
    MockFlag::getApplicationInfoRet = true;
    MockFlag::amsNull = false;
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = true;
    MockFlag::hasRunningUIExtension = false;
    MockFlag::missionListMgrNull = false;
    MockFlag::uiAbilityMgrNull = false;
    MockFlag::uiExtMgrNull = false;
}
} // namespace

class ModularObjectUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override { ResetFlags(); }
    void TearDown() override {}
};

// ==================== CheckPermission ====================

HWTEST_F(ModularObjectUtilsTest, CheckPermission_001, TestSize.Level1)
{
    // Device not supported
    MockFlag::isSupportModularObjectExtension = false;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_CAPABILITY_NOT_SUPPORT);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_002, TestSize.Level1)
{
    // GetTargetExtensionInfoFromDb fails
    MockFlag::queryDataRet = -1;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_003, TestSize.Level1)
{
    // Extension not found in db
    MockFlag::extensionFound = false;
    AbilityRequest request;
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_004, TestSize.Level1)
{
    // Extension disabled and different uid
    MockFlag::extensionFound = true;
    MockFlag::extensionDisabled = true;
    MockFlag::callingUid = 999; // different from request.uid
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_MODULAR_OBJECT_DISABLED);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_005, TestSize.Level1)
{
    // Caller not foreground
    MockFlag::processState = 4; // APP_STATE_BACKGROUND
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_006, TestSize.Level1)
{
    // Caller is preForeground
    MockFlag::isPreForeground = true;
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_007, TestSize.Level1)
{
    // GetCallerAppInfo fails - BundleMgrHelper GetNameAndIndexForUid fails
    MockFlag::getNameAndIndexRet = -1;
    AbilityRequest request;
    request.uid = 100;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_008, TestSize.Level1)
{
    // Distribution type check - caller is "none", not developer mode
    MockFlag::isDeveloperMode = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    request.appInfo.appDistributionType = "none";
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_009, TestSize.Level1)
{
    // Distribution type check - target is "none"
    MockFlag::isDeveloperMode = false;
    MockFlag::hasRunningUIAbility = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    request.appInfo.appDistributionType = "debug";
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
}

HWTEST_F(ModularObjectUtilsTest, CheckPermission_010, TestSize.Level1)
{
    // Target has no running ability
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    AbilityRequest request;
    request.uid = 1000;
    request.want.SetElement(ElementName("", "com.test.bundle", "TestAbility"));
    auto ret = ModularObjectUtils::CheckPermission(request);
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
}

// ==================== CheckAppDistributionType ====================

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_001, TestSize.Level1)
{
    // Developer mode - allow
    MockFlag::isDeveloperMode = true;
    auto ret = ModularObjectUtils::CheckAppDistributionType("none", "none");
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_002, TestSize.Level1)
{
    // Caller distribution type is "none"
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("none", "debug");
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_003, TestSize.Level1)
{
    // Target distribution type is "none"
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("debug", "none");
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

HWTEST_F(ModularObjectUtilsTest, CheckAppDistributionType_004, TestSize.Level1)
{
    // Normal - both have valid distribution types
    MockFlag::isDeveloperMode = false;
    auto ret = ModularObjectUtils::CheckAppDistributionType("debug", "release");
    EXPECT_EQ(ret, ERR_OK);
}

// ==================== CheckExtensionEnabled ====================

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_001, TestSize.Level1)
{
    // Disabled and different uid
    MockFlag::callingUid = 999;
    ModularObjectExtensionInfo info;
    info.isDisabled = true;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_MODULAR_OBJECT_DISABLED);
}

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_002, TestSize.Level1)
{
    // Disabled but same uid - should pass
    MockFlag::callingUid = 100;
    ModularObjectExtensionInfo info;
    info.isDisabled = true;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(ModularObjectUtilsTest, CheckExtensionEnabled_003, TestSize.Level1)
{
    // Not disabled - should pass regardless of uid
    MockFlag::callingUid = 999;
    ModularObjectExtensionInfo info;
    info.isDisabled = false;
    info.bundleName = "com.test";
    info.abilityName = "TestAbility";
    AbilityRequest request;
    request.uid = 100;
    auto ret = ModularObjectUtils::CheckExtensionEnabled(info, request);
    EXPECT_EQ(ret, ERR_OK);
}

// ==================== CheckCallerForeground ====================

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_001, TestSize.Level1)
{
    // GetRunningProcessInfoByPid fails
    MockFlag::getRunningProcessInfoRet = -1;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, -1);
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_002, TestSize.Level1)
{
    // Caller not foreground (background state)
    MockFlag::processState = 4; // APP_STATE_BACKGROUND
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_003, TestSize.Level1)
{
    // Caller is preForeground
    MockFlag::isPreForeground = true;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, NOT_TOP_ABILITY);
}

HWTEST_F(ModularObjectUtilsTest, CheckCallerForeground_004, TestSize.Level1)
{
    // Caller is foreground
    MockFlag::processState = 2; // APP_STATE_FOREGROUND
    MockFlag::isPreForeground = false;
    auto ret = ModularObjectUtils::CheckCallerForeground();
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_001, TestSize.Level1)
{
    // SceneBoard enabled with running UIAbility
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = true;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_002, TestSize.Level1)
{
    // SceneBoard enabled, no UIAbility but has UIExtension
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = true;
    MockFlag::uiExtMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_003, TestSize.Level1)
{
    // No running abilities at all
    MockFlag::isSceneBoardEnabled = true;
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    MockFlag::uiExtMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_FALSE(ret);
}

HWTEST_F(ModularObjectUtilsTest, HasRunningUIAbilityOrExtension_004, TestSize.Level1)
{
    // SceneBoard disabled with running ability via MissionListManager
    MockFlag::isSceneBoardEnabled = false;
    MockFlag::hasRunningUIAbility = true;
    MockFlag::missionListMgrNull = false;
    auto ret = ModularObjectUtils::HasRunningUIAbilityOrExtension(1000, 100);
    EXPECT_TRUE(ret);
}

// ==================== CheckTargetHasRunningAbility ====================

HWTEST_F(ModularObjectUtilsTest, CheckTargetHasRunningAbility_001, TestSize.Level1)
{
    // No running ability
    MockFlag::hasRunningUIAbility = false;
    MockFlag::hasRunningUIExtension = false;
    auto ret = ModularObjectUtils::CheckTargetHasRunningAbility(1000, 100, "com.test");
    EXPECT_EQ(ret, ERR_NO_RUNNING_ABILITIES_WITH_UI);
}

HWTEST_F(ModularObjectUtilsTest, CheckTargetHasRunningAbility_002, TestSize.Level1)
{
    // Has running ability
    MockFlag::hasRunningUIAbility = true;
    auto ret = ModularObjectUtils::CheckTargetHasRunningAbility(1000, 100, "com.test");
    EXPECT_EQ(ret, ERR_OK);
}

// ==================== GetTargetExtensionInfoFromDb ====================

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_001, TestSize.Level1)
{
    // QueryData fails
    MockFlag::queryDataRet = -1;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb("bundle", "ability", 0, 100, targetInfo);
    EXPECT_EQ(ret, -1);
}

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_002, TestSize.Level1)
{
    // Extension not found
    MockFlag::extensionFound = false;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb("bundle", "ability", 0, 100, targetInfo);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, GetTargetExtensionInfoFromDb_003, TestSize.Level1)
{
    // Success - extension found
    MockFlag::extensionFound = true;
    ModularObjectExtensionInfo targetInfo;
    auto ret = ModularObjectUtils::GetTargetExtensionInfoFromDb(
        "com.test.bundle", "TestAbility", 0, 100, targetInfo);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(targetInfo.bundleName, "com.test.bundle");
    EXPECT_EQ(targetInfo.abilityName, "TestAbility");
}

// ==================== GetCallerAppInfo ====================

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_001, TestSize.Level1)
{
    // BundleMgrHelper is null - can't easily test since singleton always returns instance
    // GetNameAndIndexForUid fails
    MockFlag::getNameAndIndexRet = -1;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_002, TestSize.Level1)
{
    // GetOsAccountLocalIdFromUid fails
    MockFlag::getOsAccountRet = -1;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_003, TestSize.Level1)
{
    // GetApplicationInfoWithAppIndex fails
    MockFlag::getApplicationInfoRet = false;
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, INNER_ERR);
}

HWTEST_F(ModularObjectUtilsTest, GetCallerAppInfo_004, TestSize.Level1)
{
    // Success
    ApplicationInfo callerAppInfo;
    auto ret = ModularObjectUtils::GetCallerAppInfo(callerAppInfo);
    EXPECT_EQ(ret, ERR_OK);
}
