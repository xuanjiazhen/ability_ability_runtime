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

#define private public
#define protected public
#include "ability_manager_service.h"
#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "mission_list_manager.h"
#include "ui_ability_lifecycle_manager.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "ability_manager_stub_mock_test.h"
#include "ability_scheduler_mock.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "insight_intent_db_cache.h"
#include "insight_intent_utils.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager_proxy.h"
#include "mock_task_handler_wrap.h"
#include "process_options.h"
#include "recovery_param.h"
#include "scene_board_judgement.h"
#include "ui_service_extension_connection_constants.h"

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Invoke;
using testing::Return;

using namespace OHOS::AppExecFwk;
using OHOS::AppExecFwk::AbilityType;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_INVALID_USER_ID = -1;
const int32_t USER_ID_U100 = 100;
const int32_t APP_MEMORY_SIZE = 512;
constexpr const char* DEBUG_APP = "debugApp";
const std::string TEST_BUNDLE_NAME = "testBundleName";
const std::string TEST_ABILITY_NAME = "testAbilityName";
const std::string CONTACTS_BUNDLE_NAME = "com.ohos.contacts";
const std::string CONTACTS_ABILITY_NAME = "com.ohos.contacts.MainAbility";
const std::string DEVICE_MANAGER_BUNDLE_NAME = "com.ohos.devicemanagerui";
const std::string DEVICE_MANAGER_NAME = "com.ohos.devicemanagerui.MainAbility";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
#ifdef SUPPORT_AUTO_FILL
const std::string AUTO_FILL_PASSWORD_TPYE = "autoFill/password";
const std::string AUTO_FILL_SMART_TPYE = "autoFill/smart";
const std::string AUTO_FILL_PASSWORD_ABILITY_NAME = "AutoFillAbility";
const std::string AUTO_FILL_PASSWORD_BUNDLE_NAME = "com.ohos.passwordbox";
const std::string AUTO_FILL_MODULE_NAME = "entry";
const std::string AUTO_FILL_SMART_ABILITY_NAME = "TextAutoFillAbility";
const std::string AUTO_FILL_SMART_BUNDLE_NAME = "com.ohos.textautofill";
#endif // SUPPORT_AUTO_FILL
const int32_t MAIN_USER_ID = 100;
const std::string EMPTY_DEVICE_ID = "";
constexpr const char* BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* BUNDLE_NAME_TEST = "com.huawei.hmos.passwordvault";
constexpr const char* BUNDLE_NAME_SMART_TEST = "com.huawei.hms.textautofill";
const std::string APP_INSTANCE_KEY("ohos.extra.param.key.appInstance");
const std::string CREATE_APP_INSTANCE_KEY("ohos.extra.param.key.createAppInstance");
constexpr const char* CALLER_REQUEST_CODE = "ohos.extra.param.key.callerRequestCode";
}  // namespace
class InsightIntentQueryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    std::shared_ptr<AbilityManagerService> MockAbilityManagerService();
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };

public:
    AbilityRequest abilityRequest_{};
    Want want_{};
    AbilityStartSetting abilityStartSetting_;
};

AbilityRequest InsightIntentQueryTest::GenerateAbilityRequest(const std::string& deviceName,
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

std::shared_ptr<AbilityManagerService> InsightIntentQueryTest::MockAbilityManagerService()
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("InsightIntentQueryTest");
    auto eventHandler = std::make_shared<AbilityEventHandler>(taskHandler, abilityMs);
    abilityMs->taskHandler_ = taskHandler;
    abilityMs->interceptorExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->afterCheckExecuter_ = std::make_shared<AbilityInterceptorExecuter>();
    abilityMs->subManagersHelper_ = std::make_shared<SubManagersHelper>(taskHandler, eventHandler);
    return abilityMs;
}

void InsightIntentQueryTest::SetUpTestCase() {}

void InsightIntentQueryTest::TearDownTestCase() {}

void InsightIntentQueryTest::SetUp()
{
    bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
}

void InsightIntentQueryTest::TearDown()
{}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(InsightIntentQueryTest, GetAllInsightIntentInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(InsightIntentQueryTest, GetAllInsightIntentInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(InsightIntentQueryTest, GetAllInsightIntentInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetAllInsightIntentInfo
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetAllInsightIntentInfo
 */
HWTEST_F(InsightIntentQueryTest, GetAllInsightIntentInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetAllInsightIntentInfo(flag, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetAllInsightIntentInfo_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByBundleName
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByBundleName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByBundleName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::string bundleName = "com.example.bundleName";
    std::vector<InsightIntentInfoForQuery> infos;
    auto ret = abilityMs->GetInsightIntentInfoByBundleName(flag, bundleName, infos);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByBundleName_004 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: NA
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByIntentName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_001 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_001 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: flag is GET_FULL_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByIntentName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_002 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_002 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: flag is GET_SUMMARY_INSIGHT_INTENT or GET_ENTITY_INFO
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByIntentName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_003 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = static_cast<GetInsightIntentFlag>(AbilityRuntime::GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT |
                AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO);
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_003 end");
}

/*
 * Feature: AbilityManagerService
 * Function: GetInsightIntentInfoByIntentName
 * SubFunction: invalid flag
 * FunctionPoints: AbilityManagerService GetInsightIntentInfoByIntentName
 */
HWTEST_F(InsightIntentQueryTest, GetInsightIntentInfoByIntentName_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_004 start");
    auto abilityMs = std::make_shared<AbilityManagerService>();
    EXPECT_NE(abilityMs, nullptr);
    auto flag = AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO;
    std::string bundleName = "com.example.bundleName";
    std::string moduleName = "entry";
    std::string intentName = "test";
    InsightIntentInfoForQuery info;
    auto ret = abilityMs->GetInsightIntentInfoByIntentName(flag, bundleName, moduleName, intentName, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentQueryTest GetInsightIntentInfoByIntentName_004 end");
}
}  // namespace AAFwk
}  // namespace OHOS