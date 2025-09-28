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

#include <gtest/gtest.h>

#include "ability_util.h"
#define private public
#define protected public
#include "interceptor/kiosk_interceptor.h"
#undef private
#undef protected
#include "system_ability_definition.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
constexpr char KIOSK_WHITE_LIST[] = "KioskWhitelist";

namespace OHOS {
namespace AAFwk {
class KiosInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    std::shared_ptr<AbilityRecord> abilityRecord;
    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
};

void KiosInterceptorTest::SetUpTestCase() {}

void KiosInterceptorTest::TearDownTestCase() {}

void KiosInterceptorTest::SetUp() {}

void KiosInterceptorTest::TearDown() {}

std::shared_ptr<AbilityRecord> KiosInterceptorTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> KiosInterceptorTest::MockToken(AbilityType abilityType)
{
    abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

sptr<SessionInfo> KiosInterceptorTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

AbilityRequest KiosInterceptorTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

/*
 * Feature: KioskManager
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInKioskMode
 */
HWTEST_F(KiosInterceptorTest, IsInKioskMode_001, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskManager
 * Function: IsInKioskMode
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInKioskMode
 */
HWTEST_F(KiosInterceptorTest, IsInKioskMode_002, TestSize.Level1) {
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    bool result = KioskManager::GetInstance().IsInKioskModeInner();
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInKioskMode();
    EXPECT_EQ(result, true);
}

/*
 * Feature: KioskManager
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInWhiteList
 */
HWTEST_F(KiosInterceptorTest, IsInWhiteList_001, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.clear();
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, false);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, false);
}

/*
 * Feature: KioskManager
 * Function: IsInWhiteList
 * SubFunction: NA
 * FunctionPoints: KioskManager IsInWhiteList
 */
HWTEST_F(KiosInterceptorTest, IsInWhiteList_002, TestSize.Level1) {
    std::string bundleName = "com.test.example";
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    bool result = KioskManager::GetInstance().IsInWhiteListInner(bundleName);
    EXPECT_EQ(result, true);

    result = KioskManager::GetInstance().IsInWhiteList(bundleName);
    EXPECT_EQ(result, true);
}

#ifdef SUPPORT_GRAPHICS
/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KiosInterceptorTest, KioskInterceptor_001, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = false;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KiosInterceptorTest, KioskInterceptor_002, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.clear();
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KiosInterceptorTest, KioskInterceptor_003, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    KioskManager::GetInstance().whitelist_.emplace(bundleName);
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KiosInterceptorTest, KioskInterceptor_004, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "";
    want.SetElementName(bundleName, "MainAbility");
    want.SetParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE);
    want.SetAction("com.example.myapplication");
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    KioskManager::GetInstance().kioskStatus_.isKioskMode_ = true;
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_KIOSK_MODE_NOT_IN_WHITELIST);
}

/*
 * Feature: KioskManager
 * Function: KioskInterceptor
 * SubFunction: NA
 * FunctionPoints: KioskInterceptor DoProcess
 */
HWTEST_F(KiosInterceptorTest, KioskInterceptor_005, TestSize.Level1)
{
    auto kioskInterceptor = std::make_shared<KioskInterceptor>();
    Want want;
    std::string bundleName = "com.test.example";
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param = AbilityInterceptorParam(want, 0, 0, false, nullptr,
        shouldBlockFunc);
    int32_t result = kioskInterceptor->DoProcess(param);
    EXPECT_EQ(result, ERR_OK);
}
#endif
} // namespace AAFwk
} // namespace OHOS
