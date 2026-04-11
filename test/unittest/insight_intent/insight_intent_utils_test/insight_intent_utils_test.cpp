/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "iservice_registry.h"
#undef private
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"
#include "insight_intent_utils.h"
#include "int_wrapper.h"
#include "insight_intent_profile.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_info_for_query.h"
#include "string_wrapper.h"
#include "want.h"
#include "mock_system_ability_manager.h"
#include "mock_bundle_manager_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "testBundleName";
const std::string TEST_ABILITY_NAME = "testAbilityName";
const std::string TEST_MODULE_NAME = "testModuleName";
const std::string TEST_INTENT_NAME = "testIntentName";
const std::string TEST_JSON_STR = "{\"intentName\":\"testIntent1\", \"domain\":\"testDomain\"}";
std::string TEST_SRC_ENTRY = "entry";
const std::string TEST_JSON_STR_ARRAY = "{"
    "\"insightIntents\":["
    "{"
        "\"intentName\":\"test1\","
        "\"domain\":\"domain1\","
        "\"intentVersion\":\"1.0\","
        "\"srcEntry\":\"entry1\","
        "\"uiAbility\":{"
            "\"ability\":\"ability1\","
            "\"executeMode\":[\"foreground\"]"
        "},"
        "\"uiExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"serviceExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"form\":{"
            "\"ability\":\"ability1\","
            "\"formName\":\"form1\""
        "}"
    "},"
    "{"
        "\"intentName\":\"testIntentName\","
        "\"domain\":\"domain1\","
        "\"intentVersion\":\"1.0\","
        "\"srcEntry\":\"entry1\","
        "\"uiAbility\":{"
            "\"ability\":\"ability1\","
            "\"executeMode\":[\"foreground\"]"
        "},"
        "\"uiExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"serviceExtension\":{"
            "\"ability\":\"ability1\""
        "},"
        "\"form\": {"
            "\"ability\":\"ability1\","
            "\"formName\":\"form1\""
        "}"
    "}"
    "]"
"}";

InsightIntentLinkInfo TEST_INSIGHT_INTENT_LINK_INFO = [] {
    InsightIntentLinkInfo tmp;
    tmp.uri = "uri_test";
    tmp.parameters = R"({"oneOf":[{"requied":["palybackSpeed"]},{"requied": ["playbackProgress"]}],"properties":
        {"playbackProgress":{"type":"number","description":"播放进度，单位秒"},"palybackSpeed":
        {"description":"播放速率","enum":[0.5,0.75,1,1.25,1.5,2],"type":"number"}},"propertiesNames":
        {"enum":["playbackProgress","palybackSpeed"]},"type":"object"})";
    return tmp;
}();

InsightIntentFormInfo TEST_INSIGHT_INTENT_FORM_INFO = [] {
    InsightIntentFormInfo tmp;
    tmp.abilityName = "abilityName";
    tmp.formName = "formName";
    tmp.parameters = R"({"oneOf":[{"requied":["palybackSpeed"]},{"requied": ["playbackProgress"]}],"properties":
        {"playbackProgress":{"type":"number","description":"播放进度，单位秒"},"palybackSpeed":
        {"description":"播放速率","enum":[0.5,0.75,1,1.25,1.5,2],"type":"number"}},"propertiesNames":
        {"enum":["playbackProgress","palybackSpeed"]},"type":"object"})";
    return tmp;
}();

ExtractInsightIntentGenericInfo TEST_INSIGHT_INTENT_GENERIC_INFO = [] {
    ExtractInsightIntentGenericInfo tmp;
    tmp.bundleName = "com.tdd.test";
    tmp.moduleName = "entry";
    tmp.intentName = "InsightIntentLink";
    tmp.displayName = "displayName_test";
    tmp.decoratorType = "decoratorType_test";
    tmp.set<InsightIntentLinkInfo>();
    tmp.data = TEST_INSIGHT_INTENT_LINK_INFO;
    return tmp;
}();

ExtractInsightIntentInfo TEST_INSIGHT_INTENT_INFO = [] {
    ExtractInsightIntentInfo tmp;
    InsightIntentEntityInfo entities;
    entities.decoratorFile = "decoratorFile_test";
    entities.className = "className_test";
    entities.decoratorType = "decoratorType_test";
    entities.entityCategory = "entityCategory_test";
    entities.parameters = "parameters_test";
    entities.parentClassName = "parentClassName_test";

    tmp.decoratorFile = "decoratorFile_test";
    tmp.decoratorClass = "decoratorClass_test";
    tmp.displayDescription = "displayDescription_test";
    tmp.domain = "domain_test";
    tmp.intentVersion = "intentVersion_test";
    tmp.schema = "schema_test";
    tmp.icon = "icon_test";
    tmp.llmDescription = "llmDescription_test";
    tmp.keywords = std::vector<std::string>{ "keywords_test1", "keywords_test2" };
    tmp.entities.push_back(entities);
    tmp.genericInfo = TEST_INSIGHT_INTENT_GENERIC_INFO;
    return tmp;
}();

constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
auto mockBundleMgr = sptr<MockBundleManagerService>::MakeSptr();
}
class InsightIntentUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MockBundleInstallerAndSA();
    void MockBundleInstaller();
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
};

void InsightIntentUtilsTest::SetUpTestCase(void)
{}

void InsightIntentUtilsTest::TearDownTestCase(void)
{}

void InsightIntentUtilsTest::SetUp()
{
    mockSystemAbility_ = sptr<MockSystemAbilityManager>::MakeSptr();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
    MockBundleInstallerAndSA();
}

void InsightIntentUtilsTest::MockBundleInstallerAndSA()
{
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockSystemAbility_, CheckSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void InsightIntentUtilsTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
    testing::Mock::AllowLeak(mockSystemAbility_);
}

/**
 * @tc.name: GetSrcEntry_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, TEST_ABILITY_NAME, TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    AppExecFwk::ElementName element2("", TEST_BUNDLE_NAME, TEST_ABILITY_NAME, "");
    result = utils.GetSrcEntry(element2, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    AppExecFwk::ElementName element3("", TEST_BUNDLE_NAME, "", TEST_MODULE_NAME);
    result = utils.GetSrcEntry(element3, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    AppExecFwk::ElementName element4("", "", TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    result = utils.GetSrcEntry(element4, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY, TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0100 end.");
}

/**
 * @tc.name: GetSrcEntry_0200
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0200 start");
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_ABILITY_NAME, TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0200 end.");
}

/**
 * @tc.name: GetSrcEntry_0300
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0300 start");
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, "ability1", TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_BUNDLE_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0300 end.");
}

/**
 * @tc.name: GetSrcEntry_0400
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetSrcEntry_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest GetSrcEntry_0400 start");
    AbilityRuntime::InsightIntentUtils utils;
    AppExecFwk::ElementName element1("", TEST_BUNDLE_NAME, "ability1", TEST_MODULE_NAME);
    auto result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_ABILITY_FOREGROUND,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);

    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_ABILITY_BACKGROUND,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::UI_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, ExecuteMode::SERVICE_EXTENSION_ABILITY,
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);
    result = utils.GetSrcEntry(element1, TEST_INTENT_NAME, static_cast<ExecuteMode>(INT_MAX),
        TEST_SRC_ENTRY);
    EXPECT_EQ(result, ERR_INSIGHT_INTENT_GET_PROFILE_FAILED);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest GetSrcEntry_0400 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentLink";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(insightIntentInfoForQuery.intentType, "@InsightIntentLink");

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0100 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0110
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0110, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0110 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentPage";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO,
        insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0110 end.");
}
/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0120
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */

HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0120, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0120 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentEntry";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO,
        insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0120 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0130
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0130, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0130 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentFunctionMethod";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO,
        insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0130 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0140
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0140, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0140 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "@InsightIntentForm";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO,
        insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0140 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentGenericInfo_0200
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentGenericInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0200 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_GENERIC_INFO.decoratorType = "test";
    auto result = utils.ConvertExtractInsightIntentGenericInfo(TEST_INSIGHT_INTENT_GENERIC_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentGenericInfo_0200 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    bool getEntity = false;
    auto result = utils.ConvertExtractInsightIntentInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery, getEntity);
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0100 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentInfo_0200
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0200 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    TEST_INSIGHT_INTENT_INFO.genericInfo.data = TEST_INSIGHT_INTENT_FORM_INFO;
    TEST_INSIGHT_INTENT_INFO.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FORM;
    bool getEntity = false;
    auto result = utils.ConvertExtractInsightIntentInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery, getEntity);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(TEST_INSIGHT_INTENT_FORM_INFO.abilityName, insightIntentInfoForQuery.formInfo.abilityName);
    EXPECT_EQ(TEST_INSIGHT_INTENT_FORM_INFO.formName, insightIntentInfoForQuery.formInfo.formName);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0200 end");
}

/**
 * @tc.name: ConvertExtractInsightIntentInfo_0300
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0300 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    bool getEntity = true;
    auto result = utils.ConvertExtractInsightIntentInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery, getEntity);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(TEST_INSIGHT_INTENT_INFO.genericInfo.bundleName, insightIntentInfoForQuery.bundleName);
    EXPECT_EQ(TEST_INSIGHT_INTENT_INFO.genericInfo.moduleName, insightIntentInfoForQuery.moduleName);
    EXPECT_EQ(TEST_INSIGHT_INTENT_INFO.genericInfo.intentName, insightIntentInfoForQuery.intentName);
    EXPECT_EQ(TEST_INSIGHT_INTENT_INFO.entities[0].className, insightIntentInfoForQuery.entities[0].className);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentInfo_0300 end");
}

/**
 * @tc.name: ConvertExtractInsightIntentEntityInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentEntityInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertExtractInsightIntentEntityInfo_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery insightIntentInfoForQuery;
    auto result = utils.ConvertExtractInsightIntentEntityInfo(TEST_INSIGHT_INTENT_INFO, insightIntentInfoForQuery);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(TEST_INSIGHT_INTENT_INFO.entities[0].className, insightIntentInfoForQuery.entities[0].className);

    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertExtractInsightIntentEntityInfo_0100 end");
}

/**
 * @tc.name: ConvertConfigInsightIntentInfo_0100
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertConfigInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST,  "InsightIntentUtilsTest ConvertConfigInsightIntentInfo_0100 start");
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery queryInfo;
    InsightIntentInfo intentInfo;
    intentInfo.bundleName = "bundle.cfg";
    intentInfo.moduleName = "entry";
    intentInfo.intentName = "PlayMedia";
    intentInfo.srcEntry = "EntryAbility";
    intentInfo.intentDomain = "media";
    intentInfo.intentVersion = "1.0";
    intentInfo.displayDescription = "desc";
    intentInfo.icon = "icon.png";
    intentInfo.keywords = {"music", "video"};
    bool getEntity = false;
    auto result = utils.ConvertConfigInsightIntentInfo(intentInfo, queryInfo, getEntity);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(queryInfo.isConfig);
    EXPECT_EQ(queryInfo.bundleName, intentInfo.bundleName);
    EXPECT_EQ(queryInfo.moduleName, intentInfo.moduleName);
    EXPECT_EQ(queryInfo.intentName, intentInfo.intentName);
    EXPECT_EQ(queryInfo.srcEntry, intentInfo.srcEntry);
    EXPECT_EQ(queryInfo.domain, intentInfo.intentDomain);
    EXPECT_EQ(queryInfo.intentVersion, intentInfo.intentVersion);
    EXPECT_EQ(queryInfo.displayDescription, intentInfo.displayDescription);
    EXPECT_EQ(queryInfo.icon, intentInfo.icon);
    ASSERT_EQ(queryInfo.keywords.size(), intentInfo.keywords.size());
    EXPECT_EQ(queryInfo.keywords[0], "music");
    EXPECT_EQ(queryInfo.keywords[1], "video");
    TAG_LOGI(AAFwkTag::TEST, "InsightIntentUtilsTest ConvertConfigInsightIntentInfo_0100 end.");
}

/**
 * @tc.name: ConvertConfigInsightIntentInfo_0200
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertConfigInsightIntentInfo_0200, TestSize.Level1)
{
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery queryInfo;
    InsightIntentInfo intentInfo;
    intentInfo.uiAbilityIntentInfo.abilityName = "UIAbility";
    intentInfo.uiAbilityIntentInfo.supportExecuteMode = {
        OHOS::AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND,
        OHOS::AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND
    };
    intentInfo.uiExtensionIntentInfo.abilityName = "UIExtension";
    intentInfo.serviceExtensionIntentInfo.abilityName = "ServiceExtension";

    uint32_t ret = utils.ConvertConfigInsightIntentInfo(intentInfo, queryInfo, false);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(queryInfo.uiAbilityIntentInfo.abilityName, "UIAbility");
    ASSERT_EQ(queryInfo.uiAbilityIntentInfo.supportExecuteMode.size(), 2);
    EXPECT_EQ(
        queryInfo.uiAbilityIntentInfo.supportExecuteMode[0], OHOS::AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
    EXPECT_EQ(
        queryInfo.uiAbilityIntentInfo.supportExecuteMode[1], OHOS::AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
    EXPECT_EQ(queryInfo.uiExtensionIntentInfo.abilityName, "UIExtension");
    EXPECT_EQ(queryInfo.serviceExtensionIntentInfo.abilityName, "ServiceExtension");
}

/**
 * @tc.name: ConvertConfigInsightIntentInfo_0300
 * @tc.desc: basic function test of convert info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertConfigInsightIntentInfo_0300, TestSize.Level1)
{
    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery queryInfo;
    InsightIntentInfo intentInfo;
    intentInfo.formIntentInfo.abilityName = "FormAbility";
    intentInfo.formIntentInfo.formName = "MusicForm";
    intentInfo.cfgEntities = "cfgEntity";

    uint32_t ret = utils.ConvertConfigInsightIntentInfo(intentInfo, queryInfo, true);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(queryInfo.formIntentInfo.abilityName, "FormAbility");
    EXPECT_EQ(queryInfo.formIntentInfo.formName, "MusicForm");
    EXPECT_EQ(queryInfo.cfgEntities, "cfgEntity");
}

/**
 * @tc.name: GetInsightIntentInfo_0100
 * @tc.desc: Test GetInsightIntentInfo with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetInsightIntentInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0100 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(TEST_BUNDLE_NAME);
    elementName.SetModuleName(TEST_MODULE_NAME);
    elementName.SetAbilityName(TEST_ABILITY_NAME);

    AppExecFwk::ExecuteMode executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::string srcEntry;
    int32_t userId = 100;

    InsightIntentInfo intentInfo;
    auto ret = utils.GetInsightIntentInfo(elementName, executeMode, srcEntry, userId, intentInfo);

    // Since test bundle doesn't exist, should return error (not ERR_OK)
    // Common error codes for missing profile/data
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0100 end.");
}

/**
 * @tc.name: GetInsightIntentInfo_0200
 * @tc.desc: Test GetInsightIntentInfo with empty element name.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetInsightIntentInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0200 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    AppExecFwk::ElementName elementName;
    // Empty element name
    elementName.SetBundleName("");
    elementName.SetModuleName("");
    elementName.SetAbilityName("");

    AppExecFwk::ExecuteMode executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::string srcEntry;
    int32_t userId = 100;

    InsightIntentInfo intentInfo;
    auto ret = utils.GetInsightIntentInfo(elementName, executeMode, srcEntry, userId, intentInfo);

    // Should fail with empty element name
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0200 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentProfile_0100
 * @tc.desc: Test ConvertExtractInsightIntentProfile with valid JSON.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentProfile_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0100 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    std::string profileJson = TEST_JSON_STR_ARRAY;
    ExtractInsightIntentProfileInfoVec profileInfos;

    auto ret = AbilityRuntime::ExtractInsightIntentProfile::TransformTo(profileJson, profileInfos);
    EXPECT_TRUE(ret);
    EXPECT_GE(profileInfos.insightIntents.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0100 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentProfile_0200
 * @tc.desc: Test ConvertExtractInsightIntentProfile with malformed JSON.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentProfile_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0200 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    std::string malformedJson = "{invalid json content";
    ExtractInsightIntentProfileInfoVec profileInfos;

    auto ret = AbilityRuntime::ExtractInsightIntentProfile::TransformTo(malformedJson, profileInfos);
    EXPECT_FALSE(ret);
    EXPECT_EQ(profileInfos.insightIntents.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0200 end.");
}

/**
 * @tc.name: ConvertExtractInsightIntentProfile_0300
 * @tc.desc: Test ConvertExtractInsightIntentProfile with empty JSON.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertExtractInsightIntentProfile_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0300 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    std::string emptyJson = "";
    ExtractInsightIntentProfileInfoVec profileInfos;

    auto ret = AbilityRuntime::ExtractInsightIntentProfile::TransformTo(emptyJson, profileInfos);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "ConvertExtractInsightIntentProfile_0300 end.");
}

/**
 * @tc.name: ExtractInsightIntentGenericInfo_0100
 * @tc.desc: Test ExtractInsightIntentGenericInfo with Link type.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ExtractInsightIntentGenericInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtractInsightGenericInfo_0100 begin.");

    ExtractInsightIntentGenericInfo genericInfo;
    genericInfo.bundleName = TEST_BUNDLE_NAME;
    genericInfo.moduleName = TEST_MODULE_NAME;
    genericInfo.intentName = TEST_INTENT_NAME;
    genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_LINK;

    genericInfo.set<InsightIntentLinkInfo>();
    auto& linkInfo = genericInfo.get<InsightIntentLinkInfo>();
    linkInfo.uri = "https://example.com/action";
    linkInfo.parameters = "{\"type\":\"object\"}";

    EXPECT_EQ(genericInfo.decoratorType, INSIGHT_INTENTS_DECORATOR_TYPE_LINK);
    EXPECT_EQ(genericInfo.bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(linkInfo.uri, "https://example.com/action");

    TAG_LOGI(AAFwkTag::TEST, "ExtractInsightGenericInfo_0100 end.");
}

/**
 * @tc.name: ExtractInsightIntentGenericInfo_0200
 * @tc.desc: Test ExtractInsightIntentGenericInfo with Function type.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ExtractInsightIntentGenericInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExtractInsightGenericInfo_0200 begin.");

    ExtractInsightIntentGenericInfo genericInfo;
    genericInfo.bundleName = TEST_BUNDLE_NAME;
    genericInfo.moduleName = TEST_MODULE_NAME;
    genericInfo.intentName = TEST_INTENT_NAME;
    genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;

    genericInfo.set<InsightIntentFunctionInfo>();
    auto& functionInfo = genericInfo.get<InsightIntentFunctionInfo>();
    functionInfo.functionName = "testFunction";
    functionInfo.functionParams = {"param1", "param2", "param3"};

    EXPECT_EQ(genericInfo.decoratorType, INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION);
    EXPECT_EQ(functionInfo.functionParams.size(), 3);
    EXPECT_EQ(functionInfo.functionName, "testFunction");

    TAG_LOGI(AAFwkTag::TEST, "ExtractInsightGenericInfo_0200 end.");
}

/**
 * @tc.name: ToJson_0100
 * @tc.desc: Test ToJson with InsightIntentInfo.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ToJson_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ToJson_0100 begin.");

    InsightIntentInfo intentInfo;
    intentInfo.intentName = TEST_INTENT_NAME;
    intentInfo.intentDomain = "test.domain";
    intentInfo.intentVersion = "1.0";
    intentInfo.srcEntry = TEST_SRC_ENTRY;
    intentInfo.displayName = "Test Intent";
    intentInfo.bundleName = TEST_BUNDLE_NAME;
    intentInfo.moduleName = TEST_MODULE_NAME;

    nlohmann::json jsonObject;
    auto ret = AbilityRuntime::InsightIntentProfile::ToJson(intentInfo, jsonObject);
    EXPECT_TRUE(ret);

    // Verify JSON contains expected fields
    EXPECT_TRUE(jsonObject.contains("intentName"));
    EXPECT_TRUE(jsonObject.contains("bundleName"));
    EXPECT_TRUE(jsonObject.contains("moduleName"));
    EXPECT_EQ(jsonObject["intentName"], TEST_INTENT_NAME);

    TAG_LOGI(AAFwkTag::TEST, "ToJson_0100 end.");
}

/**
 * @tc.name: ProfileInfoFormat_0100
 * @tc.desc: Test ProfileInfoFormat conversion.
 * @tc.type: FUNC
 */
HWTEST_F(InsightUtilsTest, ProfileInfoFormat_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ProfileInfoFormat_0100 begin.");

    ExtractInsightIntentProfileInfo profileInfo;
    profileInfo.bundleName = TEST_BUNDLE_NAME;
    profileInfo.moduleName = TEST_MODULE_NAME;
    profileInfo.intentName = TEST_INTENT_NAME;
    profileInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_LINK;
    profileInfo.uri = "https://test.com/action";

    ExtractInsightIntentInfo intentInfo;
    auto ret = AbilityRuntime::ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, intentInfo);
    EXPECT_TRUE(ret);

    EXPECT_EQ(intentInfo.genericInfo.bundleName, TEST_BUNDLE_NAME);
    EXPECT_EQ(intentInfo.genericInfo.intentName, TEST_INTENT_NAME);

    TAG_LOGI(AAFwkTag::TEST, "ProfileInfoFormat_0100 end.");
}

/**
 * @tc.name: ConvertConfigInsightIntentInfo_0400
 * @tc.desc: Test ConvertConfigInsightIntentInfo with UIExtension.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, ConvertConfigInsightIntentInfo_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConvertConfigInsightIntentInfo_0400 begin.");

    AbilityRuntime::InsightIntentUtils utils;
    InsightIntentInfoForQuery queryInfo;
    InsightIntentInfo intentInfo;
    intentInfo.uiExtensionIntentInfo.abilityName = "TestUIExtension";
    intentInfo.uiAbilityIntentInfo.supportExecuteMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};

    uint32_t ret = utils.ConvertConfigInsightIntentInfo(intentInfo, queryInfo, false);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(queryInfo.uiExtensionIntentInfo.abilityName, "TestUIExtension");

    TAG_LOGI(AAFwkTag::TEST, "ConvertConfigInsightIntentInfo_0400 end.");
}

/**
 * @tc.name: GetInsightIntentInfo_0300
 * @tc.desc: Test GetInsightIntentInfo with different execute modes.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentUtilsTest, GetInsightIntentInfo_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0300 begin.");

    AbilityRuntime::InsightIntentUtils utils;

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName(TEST_BUNDLE_NAME);
    elementName.SetModuleName(TEST_MODULE_NAME);
    elementName.SetAbilityName(TEST_ABILITY_NAME);

    std::vector<AppExecFwk::ExecuteMode> modes = {
        AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND,
        AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND,
        AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY,
        AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY
    };

    for (auto mode : modes) {
        std::string srcEntry;
        int32_t userId = 100;

        InsightIntentInfo intentInfo;
        auto ret = utils.GetInsightIntentInfo(elementName, mode, srcEntry, userId, intentInfo);
        // Since test bundle doesn't exist, should return error
        EXPECT_NE(ret, ERR_OK);
    }

    TAG_LOGI(AAFwkTag::TEST, "GetInsightIntentInfo_0300 end.");
}

} // namespace AAFwk
} // namespace OHOS
