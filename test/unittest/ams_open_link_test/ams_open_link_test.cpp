/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "ability_manager_service.h"
#include "ability_connect_manager.h"
#include "ability_connection.h"
#include "ability_start_setting.h"
#include "recovery_param.h"
#undef private
#undef protected

#include "ability_bundle_event_callback.h"
#include "ability_manager_errors.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_info.h"
#include "connection_observer_errors.h"
#include "free_install_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_sa_call.h"
#include "session/host/include/session.h"
#include "scene_board_judgement.h"
#include "system_ability_definition.h"
#include "uri.h"
#include "mock_ability_connect_callback.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_execute_param.h"
#include "mock_want_utils.h"
#include "mock_free_install_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
const uint32_t TOKENID = 211;
const std::string EMPTY_DEVICE_ID = "";
const std::string  SESSIONID = "sessionId";
const std::string  APPID = "1003";
const int REQUESTCODE = 10;
const std::string NOT_SHORTURL = "NOT_SHORTURL";
const std::string IS_SHORTURL = "IS_SHORTURL";
const std::string CONVERT_FAILED = "CONVERT_FAILED";
const std::string ATOMIC_SERVICE = "ATOMIC_SERVICE";
const std::string APP = "APP";
const std::string FREE_INSTALL_ERR_OK = "FREE_INSTALL_ERR_OK";
const std::string FREE_INSTALL_FAIL = "FREE_INSTALL_FAIL";
const std::string FREE_INSTALL_NOT_TOP_ABILITY = "FREE_INSTALL_NOT_TOP_ABILITY";
const std::string APP_LINKING_ONLY = "appLinkingOnly";
}  // namespace
class AmsOpenLinkTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<AbilityRecord> MockAbilityRecord(AbilityType);
    sptr<Token> MockToken(AbilityType);
    void SetUp();
    void TearDown();

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
public:
    AbilityRequest abilityRequest_{};
    Want want_{};
};

AbilityRequest AmsOpenLinkTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    want_.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    appinfo.bundleName = bundleName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want_;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;

    return abilityRequest;
}

void AmsOpenLinkTest::SetUpTestCase() {}

void AmsOpenLinkTest::TearDownTestCase() {}

void AmsOpenLinkTest::SetUp() {}

void AmsOpenLinkTest::TearDown() {}

std::shared_ptr<AbilityRecord> AmsOpenLinkTest::MockAbilityRecord(AbilityType abilityType)
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = abilityType;
    return AbilityRecord::CreateAbilityRecord(abilityRequest);
}

sptr<Token> AmsOpenLinkTest::MockToken(AbilityType abilityType)
{
    std::shared_ptr<AbilityRecord> abilityRecord = MockAbilityRecord(abilityType);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: AbilityManagerService OpenLink
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_001, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri("");
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest OpenLink_001 call result %{public}d", result);
}

/*
* Feature: AbilityManagerService
* Function: OpenLink
* FunctionPoints: AbilityManagerService OpenLink
*/
HWTEST_F(AmsOpenLinkTest, OpenLink_002, TestSize.Level1)
{
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri("");
    want.SetUri(uri);

    auto result = abilityMs_->OpenLinkInner(want, token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest OpenLink_002 call result %{public}d", result);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink not shortUrl
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(NOT_SHORTURL);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_003 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink CONVERT_FAILED
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(CONVERT_FAILED);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_004 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl ATOMIC_SERVICE
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_005 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(ATOMIC_SERVICE);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_005 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_005 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl APP
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_006 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(APP);
    want.SetUri(uri);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_006 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_006 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl not APP or ATOMIC_SERVICE; appLinkingOnly false
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_007, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_007 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(IS_SHORTURL);
    want.SetUri(uri);
    want.SetParam(APP_LINKING_ONLY, false);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_007 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_007 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLink
 * FunctionPoints: OpenLink shortUrl not APP or ATOMIC_SERVICE; appLinkingOnly true
 */
HWTEST_F(AmsOpenLinkTest, OpenLink_008, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_008 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    Uri uri(IS_SHORTURL);
    want.SetUri(uri);
    want.SetParam(APP_LINKING_ONLY, true);

    auto result = abilityMs_->OpenLink(want, token, USER_ID_U100, REQUESTCODE);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_008 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "AmsOpenLinkTest_OpenLink_008 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService No freeInstallManager_ FREE_INSTALL_FAIL
 */
HWTEST_F(AmsOpenLinkTest, OpenLinkFreeInstallAtomicService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_FAIL);
    convertedWant.SetUri(uri);
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(convertedWant, want,
        token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_FAIL
 */
HWTEST_F(AmsOpenLinkTest, OpenLinkFreeInstallAtomicService_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_FAIL);
    convertedWant.SetUri(uri);
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(convertedWant, want,
        token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_NOT_TOP_ABILITY
 */
HWTEST_F(AmsOpenLinkTest, OpenLinkFreeInstallAtomicService_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_NOT_TOP_ABILITY);
    convertedWant.SetUri(uri);
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(convertedWant, want,
        token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: OpenLinkFreeInstallAtomicService FREE_INSTALL_ERR_OK
 */
HWTEST_F(AmsOpenLinkTest, OpenLinkFreeInstallAtomicService_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 start");
    auto abilityMs_ = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs_, nullptr);
    abilityMs_->freeInstallManager_ = std::make_shared<FreeInstallManager>(abilityMs_);
    sptr<IRemoteObject> token = MockToken(AbilityType::PAGE);
    AAFwk::Want want;
    AAFwk::Want convertedWant;
    Uri uri(FREE_INSTALL_ERR_OK);
    convertedWant.SetUri(uri);
    auto result = abilityMs_->OpenLinkFreeInstallAtomicService(convertedWant, want,
        token, USER_ID_U100, REQUESTCODE, true);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 call result %{public}d", result);
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkFreeInstallAtomicService_004 end");
}
}  // namespace AAFwk
}  // namespace OHOS
