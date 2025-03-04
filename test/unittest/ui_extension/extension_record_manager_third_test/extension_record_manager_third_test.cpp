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

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_record.h"
#include "extension_record.h"
#include "extension_record_factory.h"
#define private public
#include "extension_record_manager.h"
#undef private
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"
#include "session_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class ExtensionRecordManagerThirdTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerThirdTest::SetUpTestCase(void)
{}

void ExtensionRecordManagerThirdTest::TearDownTestCase(void)
{}

void ExtensionRecordManagerThirdTest::SetUp()
{}

void ExtensionRecordManagerThirdTest::TearDown()
{}

/**
 * @tc.name: GetOrCreateExtensionRecord_0100
 * @tc.desc: Test GetOrCreateExtensionRecord.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_0100 begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    std::string hostBundleName = "com.example.unittest";
    bool isLoaded = false;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);

    sessionInfo->uiExtensionComponentId = 10; // 10 is the ui extension component id
    abilityRequest.sessionInfo = sessionInfo;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->extensionRecords_.insert({0, extRecord});
    int32_t ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest, hostBundleName, abilityRecord, isLoaded);
    EXPECT_NE(abilityRecord, nullptr);
    EXPECT_EQ(ret, ERR_OK);

    abilityRequest.sessionInfo->uiExtensionComponentId = INVALID_EXTENSION_RECORD_ID;
    ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest, hostBundleName, abilityRecord, isLoaded);
    EXPECT_EQ(abilityRecord, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostBundleName);
    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, extensions});
    ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest, hostBundleName, abilityRecord, isLoaded);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetOrCreateExtensionRecord_0100 end.");
}

/**
 * @tc.name: RemovePreloadUIExtensionRecordById_0100
 * @tc.desc: Test RemovePreloadUIExtensionRecordById.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, RemovePreloadUIExtensionRecordById_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemovePreloadUIExtensionRecordById_0100 begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    std::string hostBundleName = "com.example.unittest";
    std::tuple<std::string, std::string, std::string, std::string> extensionRecordMapKey =
        std::make_tuple(abilityName, bundleName, moduleName, hostBundleName);
    int32_t extensionRecordId = 10; // 10 is the extension record id
    auto res = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
    EXPECT_FALSE(res);

    std::vector<std::shared_ptr<ExtensionRecord>> nullExtensions;
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, nullExtensions});
    res = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
    EXPECT_FALSE(res);

    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;
    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = extensions;
    res = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
    EXPECT_FALSE(res);

    extensions.at(0)->extensionRecordId_ = extensionRecordId;
    res = extRecordMgr->RemovePreloadUIExtensionRecordById(extensionRecordMapKey, extensionRecordId);
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "RemovePreloadUIExtensionRecordById_0100 end.");
}

/**
 * @tc.name: RemovePreloadUIExtensionRecord_0100
 * @tc.desc: Test RemovePreloadUIExtensionRecord.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, RemovePreloadUIExtensionRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemovePreloadUIExtensionRecord_0100 begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    std::shared_ptr<AAFwk::AbilityRecord> abilityRecord = AAFwk::AbilityRecord::CreateAbilityRecord(abilityRequest);
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    std::string hostBundleName = "com.example.unittest";
    std::tuple<std::string, std::string, std::string, std::string> extensionRecordMapKey =
        std::make_tuple(abilityName, bundleName, moduleName, hostBundleName);
    auto res = extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey);
    EXPECT_FALSE(res);

    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;
    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = extensions;
    res = extRecordMgr->RemovePreloadUIExtensionRecord(extensionRecordMapKey);
    EXPECT_TRUE(res);
    EXPECT_TRUE(extRecordMgr->preloadUIExtensionMap_.empty());
    TAG_LOGI(AAFwkTag::TEST, "RemovePreloadUIExtensionRecord_0100 end.");
}
} // namespace AbilityRuntime
} // namespace OHOS
