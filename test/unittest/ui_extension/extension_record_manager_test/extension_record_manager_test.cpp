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

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_record.h"
#include "base_extension_record.h"
#include "extension_record.h"
#include "extension_record_factory.h"
#define private public
#include "extension_record_manager.h"
#undef private
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "preload_ui_extension_execute_callback_interface.h"
#include "ui_extension_record.h"
#include "ui_extension_migration_utils.h"
#include "ability_connect_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr int32_t EXTENSION_RECORD_KEY_0 = 0;
    constexpr int32_t EXTENSION_RECORD_KEY_1 = 1;
    constexpr int32_t EXTENSION_RECORD_KEY_2 = 2;
    constexpr const char *SEPARATOR = ":";
} // namespace
class MockPreloadCallback : public IRemoteStub<AAFwk::IPreloadUIExtensionExecuteCallback> {
public:
    MockPreloadCallback() = default;
    virtual ~MockPreloadCallback() = default;
    int32_t lastCalledId = -1;
    bool onDestroyCalled = false;
    bool onPreloadSuccessCalled = false;
    int32_t lastErrCode = -1;
    int32_t lastRequestCode = -1;

    void OnLoadedDone(int32_t extensionAbilityId) override
    {
        lastCalledId = extensionAbilityId;
    }
    void OnDestroyDone(int32_t extensionAbilityId) override
    {
        onDestroyCalled = true;
    }
    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override
    {
        onPreloadSuccessCalled = true;
        lastRequestCode = requestCode;
        lastErrCode = innerErrCode;
    }

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
};

class ExtensionRecordManagerTest : public testing::Test {
public:

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ExtensionRecordManagerTest::SetUpTestCase(void)
{}

void ExtensionRecordManagerTest::TearDownTestCase(void)
{}

void ExtensionRecordManagerTest::SetUp()
{}

void ExtensionRecordManagerTest::TearDown()
{}

/**
 * @tc.name: IsFocused_0100
 * @tc.desc: check is focused.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsFocused_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    extRecordMgr->SetCachedFocusedCallerToken(extensionRecordId, token);

    bool isFocused = extRecordMgr->IsFocused(extensionRecordId, token, token);
    EXPECT_EQ(isFocused, true);

    auto focusedToken = extRecordMgr->GetCachedFocusedCallerToken(extensionRecordId);
    EXPECT_EQ(focusedToken, token);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GetCallerTokenList_0100
 * @tc.desc: get caller token list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetCallerTokenList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(sessionInfo, nullptr);
    sptr<IRemoteObject> callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_NE(callerToken, nullptr);
    sessionInfo->callerToken = callerToken;
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);
    extRecord->SetFocusedCallerToken(callerToken);
    extRecord->GetFocusedCallerToken();

    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    std::list<sptr<IRemoteObject>> callerList;
    extRecordMgr->GetCallerTokenList(abilityRecord, callerList);
    ASSERT_NE(extRecordMgr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateExtensionRecordId_0100
 * @tc.desc: generate extension record id.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GenerateExtensionRecordId_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    int32_t extensionRecordId = 2;
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(INVALID_EXTENSION_RECORD_ID), 1);
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(extensionRecordId), 2);
    EXPECT_EQ(extRecordMgr->GenerateExtensionRecordId(extensionRecordId), 3);
}

/**
 * @tc.name: AddExtensionRecord_0100
 * @tc.desc: AddExtensionRecord RemoveExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, AddExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);
    extRecordMgr->AddExtensionRecordToTerminatedList(extensionRecordId);
    extRecordMgr->RemoveExtensionRecord(extensionRecordId);
    extRecordMgr->AddExtensionRecordToTerminatedList(extensionRecordId);

    int32_t extensionRecordId2 = 6;
    extRecordMgr->AddExtensionRecord(extensionRecordId2, extRecord);
    bool  bIsload = false;
    auto extRecord2 = std::make_shared<ExtensionRecord>(abilityRecord);
    auto result = extRecordMgr->GetExtensionRecord(extensionRecordId2, "com.example.unittest", extRecord2, bIsload);

    auto result2 = extRecordMgr->GetExtensionRecord(extensionRecordId2, "aa", extRecord2, bIsload);
    EXPECT_EQ(result, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetActiveUIExtensionList_0100
 * @tc.desc: GetActiveUIExtensionList
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionList_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);
    int32_t pId = abilityRecord->GetPid();
    std::vector<std::string> extensionList;
    extRecordMgr->GetActiveUIExtensionList(pId, extensionList);
    extRecordMgr->GetActiveUIExtensionList("com.example.unittest", extensionList);
    extRecordMgr->GetActiveUIExtensionList("aa", extensionList);
}

/**
 * @tc.name: GetActiveUIExtensionListByUid_0100
 * @tc.desc: Empty extensionRecords_, should return ERR_OK with empty list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionListByUid_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    std::vector<std::string> extensionList;
    auto result = extRecordMgr->GetActiveUIExtensionListByUid(1000, extensionList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(extensionList.size(), 0);
}

/**
 * @tc.name: GetActiveUIExtensionListByUid_0200
 * @tc.desc: uid match, should add moduleName:name to list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionListByUid_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    int32_t testUid = 2000;
    abilityRecord->SetUid(testUid);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->AddExtensionRecord(10, extRecord);

    std::vector<std::string> extensionList;
    auto result = extRecordMgr->GetActiveUIExtensionListByUid(testUid, extensionList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(extensionList.size(), 1);
    EXPECT_EQ(extensionList[0], std::string("entry") + SEPARATOR + "MainAbility");
}

/**
 * @tc.name: GetActiveUIExtensionListByUid_0300
 * @tc.desc: uid not match, should return empty list.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionListByUid_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUid(2000);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->AddExtensionRecord(10, extRecord);

    std::vector<std::string> extensionList;
    auto result = extRecordMgr->GetActiveUIExtensionListByUid(9999, extensionList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(extensionList.size(), 0);
}

/**
 * @tc.name: GetActiveUIExtensionListByUid_0400
 * @tc.desc: it.second is nullptr, should skip.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionListByUid_0400, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    int32_t testUid = 2000;
    abilityRecord->SetUid(testUid);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->AddExtensionRecord(10, extRecord);

    extRecordMgr->extensionRecords_[11] = nullptr;

    std::vector<std::string> extensionList;
    auto result = extRecordMgr->GetActiveUIExtensionListByUid(testUid, extensionList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(extensionList.size(), 1);
}

/**
 * @tc.name: GetActiveUIExtensionListByUid_0500
 * @tc.desc: it.second->abilityRecord_ is nullptr, should skip.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetActiveUIExtensionListByUid_0500, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    int32_t testUid = 2000;
    abilityRecord->SetUid(testUid);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecordMgr->AddExtensionRecord(10, extRecord);

    auto extRecordNullAbility = std::make_shared<ExtensionRecord>(nullptr);
    extRecordMgr->extensionRecords_[11] = extRecordNullAbility;

    std::vector<std::string> extensionList;
    auto result = extRecordMgr->GetActiveUIExtensionListByUid(testUid, extensionList);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(extensionList.size(), 1);
}

/**
 * @tc.name: GetAbilityRecordBySessionInfo_0100
 * @tc.desc: GetAbilityRecordBySessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetAbilityRecordBySessionInfo_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    sessionInfo->uiExtensionComponentId = 10;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 5;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    sptr<AAFwk::SessionInfo> sessionParam(new AAFwk::SessionInfo());
    sessionParam->uiExtensionComponentId = INVALID_EXTENSION_RECORD_ID;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), nullptr);

    sessionParam->uiExtensionComponentId = 10;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), abilityRecord);
    sessionParam->uiExtensionComponentId = 9;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam), nullptr);

    auto extRecord2 = std::make_shared<ExtensionRecord>(nullptr);
    int32_t extensionRecordId2 = 6;
    extRecordMgr->AddExtensionRecord(extensionRecordId2, extRecord2);
    extRecordMgr->GetAbilityRecordBySessionInfo(sessionParam);

    AAFwk::AbilityRequest abilityRequest2;
    auto abilityRecord2 = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    bool bLoaded = false;
    std::string  bundleName = "";
    extRecordMgr->GetOrCreateExtensionRecord(abilityRequest, bundleName, abilityRecord2, bLoaded);
    extRecordMgr->GetOrCreateExtensionRecord(abilityRequest2, bundleName, abilityRecord2, bLoaded);
    int32_t hostPid = 0;
    extRecordMgr->GetHostPidForExtensionId(extensionRecordId, hostPid);
}

/**
 * @tc.name: IsMigrationUpdateRequest_0100
 * @tc.desc: IsMigrationUpdateRequest detects migration flag in sessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsMigrationUpdateRequest_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(nullptr));

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));

    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));

    sessionInfo->originalCallerInfo.persistentId = 10;
    EXPECT_TRUE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));
}

/**
 * @tc.name: GetAbilityRecordByPersistentId_0100
 * @tc.desc: GetAbilityRecordByPersistentId matches by persistentId with hostBundleName ownership
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetAbilityRecordByPersistentId_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = hostBundle;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    sessionInfo->uiExtensionComponentId = 100;
    sessionInfo->persistentId = 10;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    bool terminatingMatch = false;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(nullptr, hostBundle, terminatingMatch), nullptr);

    sptr<AAFwk::SessionInfo> sessionParam(new AAFwk::SessionInfo());
    sessionParam->persistentId = 0;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), nullptr);

    sessionParam->persistentId = 10;
    terminatingMatch = true;  // reset by the call for a valid lookup
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), abilityRecord);
    EXPECT_FALSE(terminatingMatch);

    sessionParam->persistentId = 11;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), nullptr);

    // Cross-bundle host migration is legal in the same process: bundle mismatch is observed,
    // not rejected (security is enforced by callerToken reconciliation in the migration branch).
    sessionParam->persistentId = 10;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, "com.example.other", terminatingMatch),
        abilityRecord);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationUpdate_0100
 * @tc.desc: Migration update request reuses the original record matched by persistentId
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationUpdate_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = 100;
    createSessionInfo->persistentId = 10;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    // Reconciliation prerequisite: the recorded host token equals the declared previous host.
    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createSessionInfo->hostWindowId = 7;
    createdRecord->SetSessionInfo(createSessionInfo);

    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = 200;
    migrationSessionInfo->persistentId = 10;
    migrationSessionInfo->hostWindowId = 9;  // different from the recorded 7
    migrationSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();  // new host
    migrationSessionInfo->originalCallerInfo.callerToken = previousHostToken;            // declared
    migrationSessionInfo->originalCallerInfo.persistentId = 10;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = nullptr;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(isLoaded);
    EXPECT_EQ(resultRecord, createdRecord);
    ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(resultRecord, createdRecord);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationDeclarationMismatch_0100
 * @tc.desc: Migration with a forged/mismatched previous-host declaration is rejected (fallback)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationDeclarationMismatch_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = 100;
    createSessionInfo->persistentId = 10;
    createSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();  // recorded host
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<ExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    AAFwk::AbilityRequest forgedRequest;
    forgedRequest.appInfo.bundleName = hostBundle;
    forgedRequest.abilityInfo.name = "MainAbility";
    forgedRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> forgedSessionInfo(new AAFwk::SessionInfo());
    forgedSessionInfo->uiExtensionComponentId = 200;
    forgedSessionInfo->persistentId = 10;
    forgedSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    // Declared previous host token differs from the recorded one: reconciliation fails.
    forgedSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    forgedSessionInfo->originalCallerInfo.persistentId = 10;
    forgedRequest.sessionInfo = forgedSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = createdRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(forgedRequest, hostBundle, resultRecord, isLoaded);
    // Declaration mismatch must be rejected visibly (no silent fallback re-creation).
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, createdRecord);  // untouched
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationWindowUnchanged_0100
 * @tc.desc: Valid declaration with unchanged hostWindowId is rejected (no legal main-flow path)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationWindowUnchanged_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = 100;
    createSessionInfo->persistentId = 10;
    createSessionInfo->hostWindowId = 7;
    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<ExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    // Valid credential (declared previous host == recorded host) but hostWindowId unchanged:
    // a migration always changes hostWindowId, so this state has no legal main-flow path.
    AAFwk::AbilityRequest noOpRequest;
    noOpRequest.appInfo.bundleName = hostBundle;
    noOpRequest.abilityInfo.name = "MainAbility";
    noOpRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> noOpSessionInfo(new AAFwk::SessionInfo());
    noOpSessionInfo->uiExtensionComponentId = 200;
    noOpSessionInfo->persistentId = 10;
    noOpSessionInfo->hostWindowId = 7;  // unchanged
    noOpSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    noOpSessionInfo->originalCallerInfo.callerToken = previousHostToken;
    noOpSessionInfo->originalCallerInfo.persistentId = 10;
    noOpRequest.sessionInfo = noOpSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = createdRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(noOpRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, createdRecord);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationTerminatingFallback_0100
 * @tc.desc: Migration update never reuses a record whose termination has been triggered
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationTerminatingFallback_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = 100;
    createSessionInfo->persistentId = 10;
    createRequest.sessionInfo = createSessionInfo;
    auto terminatingRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(terminatingRecord, nullptr);
    terminatingRecord->SetTerminatingState();
    auto extRecord = std::make_shared<ExtensionRecord>(terminatingRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = 200;
    migrationSessionInfo->persistentId = 10;
    migrationSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.persistentId = 10;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = terminatingRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    // Terminating target is rejected visibly: no second instance is created while the old one
    // is still dying, and the window manager can retry after the record is fully destroyed.
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, terminatingRecord);  // untouched
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: UIExtensionRecord_Update_MigrationRetention_0100
 * @tc.desc: Reuse request without migration flag inherits stable originalCallerInfo from record
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UIExtensionRecord_Update_MigrationRetention_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest savedRequest;
    savedRequest.appInfo.bundleName = hostBundle;
    savedRequest.abilityInfo.name = "MainAbility";
    savedRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> savedSessionInfo(new AAFwk::SessionInfo());
    savedSessionInfo->uiExtensionComponentId = 100;
    savedSessionInfo->persistentId = 10;
    savedSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    savedSessionInfo->originalCallerInfo.persistentId = 10;
    savedRequest.sessionInfo = savedSessionInfo;
    auto previousHostToken = savedSessionInfo->originalCallerInfo.callerToken;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(savedRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetSessionInfo(savedSessionInfo);
    // G-1: the unconfirmed-migration marker lives on the record (authoritative), not in
    // the sessionInfo copy.
    abilityRecord->SetMigrationDeclaration(savedSessionInfo->originalCallerInfo, 9);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    AAFwk::AbilityRequest reuseRequest;
    reuseRequest.appInfo.bundleName = hostBundle;
    reuseRequest.abilityInfo.name = "MainAbility";
    reuseRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> reuseSessionInfo(new AAFwk::SessionInfo());
    reuseSessionInfo->uiExtensionComponentId = 100;
    reuseSessionInfo->persistentId = 10;
    reuseRequest.sessionInfo = reuseSessionInfo;

    extRecord->Update(reuseRequest);
    // At-least-once projection: while the migration is unconfirmed (marker kept on the
    // record), a reuse request carries the FULL declaration and the target host window so
    // the provider keeps getting re-parent chances.
    EXPECT_EQ(reuseSessionInfo->originalCallerInfo.callerToken, previousHostToken);
    EXPECT_EQ(reuseSessionInfo->hostWindowId, 9u);
}

/**
 * @tc.name: AddPreloadUIExtensionRecord_0100
 * @tc.desc: AddPreloadUIExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, AddPreloadUIExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);


    extRecordMgr->AddPreloadUIExtensionRecord(abilityRecord);
    int32_t extId = 12;
    abilityRecord->SetUIExtensionAbilityId(extId);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecordMgr->AddExtensionRecord(extId, extRecord);
    extRecordMgr->AddPreloadUIExtensionRecord(abilityRecord);
    extRecordMgr->StartAbility(abilityRequest);
    extRecordMgr->GetCachedFocusedCallerToken(extId);
}

/**
 * @tc.name: CreateExtensionRecord_0100
 * @tc.desc: CreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, CreateExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;

    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    std::string bundleName = "com.example.unittest";
    int32_t extRecordId = 9;
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    auto result = extRecordMgr->CreateExtensionRecord(abilityRequest, bundleName, extRecord, extRecordId);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    abilityRequest.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::PHOTO_EDITOR;
}

/**
 * @tc.name: IsHostSpecifiedProcessValid_0100
 * @tc.desc: IsHostSpecifiedProcessValid
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsHostSpecifiedProcessValid_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string process = "testProcess";

    bool result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    std::shared_ptr<ExtensionRecord> nullExtRecord = std::make_shared<ExtensionRecord>(nullptr);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, nullptr});
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_1, nullExtRecord});
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_2, extRecord});
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsHostSpecifiedProcessValid_0200
 * @tc.desc: IsHostSpecifiedProcessValid
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsHostSpecifiedProcessValid_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string process = "testProcess";

    bool result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    extRecord->abilityRecord_->SetProcessName(process);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, extRecord});
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_TRUE(result);

    abilityRequest.abilityInfo.name = "ability";
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);

    abilityRequest.abilityInfo.bundleName = "testBundleName";
    result = extRecordMgr->IsHostSpecifiedProcessValid(abilityRequest, extRecord, process);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: UpdateProcessName_0100
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_INSTANCE;
    std::string process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name
        + SEPARATOR + std::to_string(abilityRecord->GetUIExtensionAbilityId());
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_TYPE;
    process = abilityRequest.abilityInfo.bundleName + SEPARATOR + abilityRequest.abilityInfo.name;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_CUSTOM;
    process = abilityRequest.abilityInfo.bundleName + abilityRequest.customProcess;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    process = abilityRequest.want.GetStringParam(PROCESS_MODE_HOST_SPECIFIED_KEY);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    extRecord->abilityRecord_->SetProcessName(process);
    extRecordMgr->extensionRecords_.insert({EXTENSION_RECORD_KEY_0, extRecord});
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_RUN_WITH_MAIN_PROCESS;
    process = abilityRequest.abilityInfo.bundleName;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);

    abilityRequest.appInfo.process = "testProcess";
    process = abilityRequest.appInfo.process;
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UpdateProcessName_0200
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0200, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_PLUGIN;
    extRecord->hostBundleName_ = "testHostBundleName";
    const char *embeddedUI = "embeddedUI";
    std::string hostBundleName = "testHostBundleName";
    std::string process = hostBundleName + SEPARATOR + abilityRequest.abilityInfo.bundleName
        + SEPARATOR + embeddedUI + SEPARATOR + std::to_string(abilityRequest.abilityInfo.appIndex);
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(abilityRecord->GetProcessName(), process);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsPreloadExtensionRecord_0100
 * @tc.desc: IsPreloadExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsPreloadExtensionRecord_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::string hostBundleName = "testHostBundleName";
    bool isLoaded = false;
    int32_t hostPid = 0;

    bool result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);

    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto extensionRecordMapKey = std::make_tuple(abilityName, bundleName, moduleName, hostPid);
    std::vector<std::shared_ptr<ExtensionRecord>> nullExtensions;
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;

    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, extensions});
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_TRUE(isLoaded);
    EXPECT_TRUE(result);

    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = nullExtensions;
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);

    nullExtensions.push_back(nullptr);
    extRecordMgr->preloadUIExtensionMap_[extensionRecordMapKey] = nullExtensions;
    result = extRecordMgr->IsPreloadExtensionRecord(abilityRequest, hostPid, extRecord, isLoaded);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsBelongToManager_0100
 * @tc.desc: IsBelongToManager
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, IsBelongToManager_0100, TestSize.Level1)
{
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    EXPECT_TRUE(extRecordMgr->IsBelongToManager(abilityInfo));
}

/**
 * @tc.name: GetOrCreateExtensionRecord_0100
 * @tc.desc: GetOrCreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.sessionInfo->uiExtensionComponentId = 0;

    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord =
        std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->abilityRecord_ = abilityRecord;
    int32_t hostPid = 0;
    std::string hostBundleName = "testHostBundleName";
    bool isLoaded = false;

    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto extensionRecordMapKey = std::make_tuple(abilityName,
        bundleName, moduleName, hostPid);
    std::vector<std::shared_ptr<ExtensionRecord>> nullExtensions;
    std::vector<std::shared_ptr<ExtensionRecord>> extensions;

    extensions.push_back(extRecord);
    extRecordMgr->preloadUIExtensionMap_.insert({extensionRecordMapKey, extensions});
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest,
        hostBundleName, abilityRecord, isLoaded);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_0200
 * @tc.desc: GetOrCreateExtensionRecord
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_0200, TestSize.Level1)
{
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    abilityRequest.sessionInfo->uiExtensionComponentId = 0;
    std::string hostBundleName = "testHostBundleName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    bool isLoaded = false;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(abilityRequest,
        hostBundleName, abilityRecord, isLoaded);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0100
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0100, TestSize.Level1)
{
    sptr<IRemoteObject> token;
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0200
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0200, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0300
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0300, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0400
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0400, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0500
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0500, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();
    ASSERT_NE(token, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    ASSERT_NE(sessionInfo, nullptr);
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetUIExtensionSessionInfo_0600
 * @tc.desc: GetUIExtensionSessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetUIExtensionSessionInfo_0600, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string deviceId = "testDeviceId";
    std::string bundleName = "testBundleName";
    std::string abilityName = "testAbilityName";
    std::string moduleName = "testModuleName";
    abilityRequest.want.SetElementName(deviceId, bundleName, abilityName, moduleName);
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    int32_t extensionRecordId = 1;
    extRecordMgr->AddExtensionRecord(extensionRecordId, extRecord);

    auto token = abilityRecord->GetToken();

    AAFwk::AbilityRequest callerAbilityRequest;
    callerAbilityRequest.appInfo.bundleName = "com.example.unittest";
    callerAbilityRequest.abilityInfo.name = "MainAbility";
    callerAbilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    callerAbilityRequest.abilityInfo.extensionAbilityType =  AppExecFwk::ExtensionAbilityType::SYSDIALOG_COMMON;
    std::string callerDeviceId = "testDeviceId";
    std::string callerBundleName = "testBundleName";
    std::string callerAbilityName = "testAbilityName";
    std::string callerModuleName = "testModuleName";
    callerAbilityRequest.want.SetElementName(callerDeviceId, callerBundleName, callerAbilityName, callerModuleName);
    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(callerAbilityRequest);
    ASSERT_NE(callerAbilityRecord, nullptr);
    auto callerExtRecord = std::make_shared<ExtensionRecord>(callerAbilityRecord);
    int32_t callerExtensionRecordId = 2;
    extRecordMgr->AddExtensionRecord(callerExtensionRecordId, callerExtRecord);
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->persistentId  = 1;
    sessionInfo->hostWindowId  = 1;
    sessionInfo->uiExtensionUsage  = AAFwk::UIExtensionUsage::MODAL;
    sessionInfo->callerToken = callerToken;
    ASSERT_NE(sessionInfo, nullptr);
    extRecord->abilityRecord_->SetSessionInfo(sessionInfo);

    UIExtensionSessionInfo uiExtensionSessionInfo;
    auto ret = extRecordMgr->GetUIExtensionSessionInfo(token, uiExtensionSessionInfo);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRemoteCallback_0100
 * @tc.desc: Test GetRemoteCallback when uiExtensionRecord is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0100 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    auto result = extRecordMgr->GetRemoteCallback(nullptr);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0100 end.");
}

/**
 * @tc.name: GetRemoteCallback_0200
 * @tc.desc: Test GetRemoteCallback when hostPid is 0.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0200 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->hostPid_ = 0;

    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0200 end.");
}

/**
 * @tc.name: GetRemoteCallback_0300
 * @tc.desc: Test GetRemoteCallback when Map lookup fails (pid not found).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0300 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->hostPid_ = 9999;

    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0300 end.");
}

/**
 * @tc.name: GetRemoteCallback_0400
 * @tc.desc: Test GetRemoteCallback when Map lookup succeeds but Cast fails.
 *           This covers the map finding logic without needing a specific Mock class.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, GetRemoteCallback_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0400 begin.");
    
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t testPid = 1001;
    extRecord->hostPid_ = testPid;

    sptr<IRemoteObject> token = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    ASSERT_NE(token, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[testPid] = token;
    }
    auto result = extRecordMgr->GetRemoteCallback(extRecord);
    
    EXPECT_EQ(result, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "GetRemoteCallback_0400 end.");
}

/**
 * @tc.name: HandlePreloadUIExtensionLoaded_0100
 * @tc.desc: Test Success Scenario.
 *           Verify OnLoadedDone is actually called.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionLoaded_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionLoaded_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t recordId = 200;
    int32_t hostPid = 8888;
    
    extRecord->hostPid_ = hostPid;
    extRecord->extensionRecordId_ = recordId;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionLoaded(extRecord);
    EXPECT_EQ(mockCallback->lastCalledId, recordId);
    extRecordMgr->RemoveExtensionRecord(recordId);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionLoaded_0100 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionDestroyed_0100
 * @tc.desc: Test destroyed logic - Success scenario.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionDestroyed_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionDestroyed_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.destroy";
    abilityRequest.abilityInfo.name = "DestroyAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t recordId = 300;
    int32_t hostPid = 3333;
    extRecord->hostPid_ = hostPid;
    extRecord->extensionRecordId_ = recordId;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionDestroyed(extRecord);
    EXPECT_TRUE(mockCallback->onDestroyCalled);
    extRecordMgr->RemoveExtensionRecord(recordId);
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionDestroyed_0100 end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with nullptr token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    
    sptr<IRemoteObject> nullToken = nullptr;
    extRecordMgr->RegisterPreloadUIExtensionHostClient(nullToken);
    
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0100 end");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0200
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with valid token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RegisterPreloadUIExtensionHostClient_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    
    sptr<IRemoteObject> validToken = mockCallback->AsObject();
    ASSERT_NE(validToken, nullptr);
    
    extRecordMgr->RegisterPreloadUIExtensionHostClient(validToken);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_NE(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
        EXPECT_EQ(it->second, validToken);
    }
    
    TAG_LOGI(AAFwkTag::TEST, "RegisterPreloadUIExtensionHostClient_0200 end");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with non-existent key
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, UnRegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    int32_t nonExistentKey = 99999;
    extRecordMgr->UnRegisterPreloadUIExtensionHostClient(nonExistentKey, nullptr);
    
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(nonExistentKey);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0100 end");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0200
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient with valid token
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, UnRegisterPreloadUIExtensionHostClient_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    sptr<IRemoteObject> validToken = mockCallback->AsObject();
    ASSERT_NE(validToken, nullptr);
    
    extRecordMgr->RegisterPreloadUIExtensionHostClient(validToken);
    
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_NE(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
        EXPECT_EQ(it->second, validToken);
    }
    extRecordMgr->UnRegisterPreloadUIExtensionHostClient(callerPid, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        auto it = extRecordMgr->preloadUIExtensionHostClientCallerTokens_.find(callerPid);
        EXPECT_EQ(it, extRecordMgr->preloadUIExtensionHostClientCallerTokens_.end());
    }
    TAG_LOGI(AAFwkTag::TEST, "UnRegisterPreloadUIExtensionHostClient_0200 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionSuccess_0200
 * @tc.desc: Test success logic - isPreloadedSuccess = true.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionSuccess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.success";
    abilityRequest.abilityInfo.name = "SuccessAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);

    int32_t recordId = 400;
    int32_t hostPid = 4444;
    int32_t requestCode = 12345;
    
    extRecord->hostPid_ = hostPid;
    extRecord->requestCode_ = requestCode;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }

    extRecordMgr->HandlePreloadUIExtensionSuccess(recordId, true);

    EXPECT_TRUE(mockCallback->onPreloadSuccessCalled);
    EXPECT_EQ(mockCallback->lastRequestCode, requestCode);
    EXPECT_EQ(mockCallback->lastErrCode, 0);

    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0200 end");
}

/**
 * @tc.name: HandlePreloadUIExtensionSuccess_0300
 * @tc.desc: Test success logic - isPreloadedSuccess = false.
 *           Should verify error code and that unload logic is triggered (no crash).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, HandlePreloadUIExtensionSuccess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.fail";
    abilityRequest.abilityInfo.name = "FailAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);

    int32_t recordId = 500;
    int32_t hostPid = 5555;
    
    extRecord->hostPid_ = hostPid;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);
    sptr<MockPreloadCallback> mockCallback = new (std::nothrow) MockPreloadCallback();
    ASSERT_NE(mockCallback, nullptr);
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->preloadUIExtensionHostClientMutex_);
        extRecordMgr->preloadUIExtensionHostClientCallerTokens_[hostPid] = mockCallback->AsObject();
    }
    extRecordMgr->HandlePreloadUIExtensionSuccess(recordId, false);

    EXPECT_TRUE(mockCallback->onPreloadSuccessCalled);
    EXPECT_NE(mockCallback->lastErrCode, 0);
    EXPECT_EQ(mockCallback->lastErrCode, static_cast<int32_t>(AAFwk::INNER_ERR));

    TAG_LOGI(AAFwkTag::TEST, "HandlePreloadUIExtensionSuccess_0300 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0100
 * @tc.desc: Test ID not found.
 *           Expected: AAFwk::ERR_CODE_INVALID_ID
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t invalidId = 9999;
    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(invalidId);

    EXPECT_EQ(ret, AAFwk::ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0100 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0200
 * @tc.desc: Test Record is nullptr in map.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    int32_t recordId = 100;
    {
        std::lock_guard<std::mutex> lock(extRecordMgr->mutex_);
        extRecordMgr->extensionRecords_[recordId] = nullptr;
    }

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(extRecordMgr->extensionRecords_.count(recordId), 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0200 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0300
 * @tc.desc: Test PID mismatch (Safety check).
 *           Expected: AAFwk::ERR_CODE_INVALID_ID
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    int32_t currentPid = IPCSkeleton::GetCallingPid();
    extRecord->hostPid_ = currentPid + 100;

    int32_t recordId = 200;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);
    EXPECT_EQ(ret, AAFwk::ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0300 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0400
 * @tc.desc: Test BaseExtensionRecord is null inside ExtensionRecord.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0400 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test";
    abilityRequest.abilityInfo.name = "TestAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    
    extRecord->abilityRecord_ = nullptr;
    extRecord->hostPid_ = IPCSkeleton::GetCallingPid();

    int32_t recordId = 300;
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(extRecordMgr->extensionRecords_.count(recordId), 0);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0400 end");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0500
 * @tc.desc: Test RemovePreloadUIExtensionRecordById fail (Map inconsistency).
 *           Record exists in extensionRecords_ but not in the index Map.
 *           Expected: ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ClearPreloadedUIExtensionAbility_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0500 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.fail";
    abilityRequest.abilityInfo.name = "FailAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    
    extRecord->hostPid_ = IPCSkeleton::GetCallingPid();
    int32_t recordId = 400;
    
    extRecordMgr->AddExtensionRecord(recordId, extRecord);

    int32_t ret = extRecordMgr->ClearPreloadedUIExtensionAbility(recordId);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ClearPreloadedUIExtensionAbility_0500 end");
}

/**
 * @tc.name: ConvertToUnloadExtensionRecords0100
 * @tc.desc: Coverage test: Input list contains both valid record and nullptr.
 *           Covers both 'if' (valid) and 'else' (null) branches in one go.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, ConvertToUnloadExtensionRecords0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);

    AAFwk::AbilityRequest request;
    request.abilityInfo.name = "CoverageTest";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    auto validRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    std::vector<std::shared_ptr<ExtensionRecord>> sourceRecords;
    sourceRecords.push_back(validRecord);
    sourceRecords.push_back(nullptr);

    std::vector<std::shared_ptr<ExtensionRecord>> recordsToUnload;

    extRecordMgr->ConvertToUnloadExtensionRecords(sourceRecords, recordsToUnload);
    
    EXPECT_EQ(recordsToUnload.size(), 1);
    EXPECT_EQ(sourceRecords.size(), 1);
}

/**
 * @tc.name: UpdateProcessName_0800
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0800, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "testBundleName";
    abilityRequest.abilityInfo.name = "testInfoName";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName1"));
    int result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName123:1"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    extRecord->abilityRecord_->SetAppIndex(1);
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:1"));
    abilityRecord->SetProcessName("processName:1");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UpdateProcessName_0900
 * @tc.desc: UpdateProcessName
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_0900, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.ohos.sceneboard";
    abilityRequest.abilityInfo.name = "test";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName2"));
    int result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(2);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName:2"));
    abilityRecord->SetProcessName("newProcessName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);

    extRecord->processMode_ = PROCESS_MODE_HOST_SPECIFIED;
    abilityRecord->SetAppIndex(1);
    abilityRequest.want.SetParam(PROCESS_MODE_HOST_SPECIFIED_KEY, std::string("processName123:2"));
    abilityRecord->SetProcessName("processName");
    extRecordMgr->AddExtensionRecord(1, extRecord);
    result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0100
 * @tc.desc: Test CheckAgentUILaunchLimit when no record exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1001;
    std::string bundleName = "com.test.agentui";

    // No record exists, should return ERR_OK
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0100 end");
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0200
 * @tc.desc: Test CheckAgentUILaunchLimit when callerUid exists but bundleName not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1002;
    std::string bundleName1 = "com.test.bundle1";
    std::string bundleName2 = "com.test.bundle2";

    // Add record for bundle1
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName1, 1);

    // Check for bundle2 (not exists), should return ERR_OK
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName2);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0200 end");
}

/**
 * @tc.name: CheckAgentUILaunchLimit_0300
 * @tc.desc: Test CheckAgentUILaunchLimit when record exists and under limit.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, CheckAgentUILaunchLimit_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1003;
    std::string bundleName = "com.test.bundle";

    // Add one record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);

    // Should return ERR_OK (under limit)
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAgentUILaunchLimit_0300 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0100
 * @tc.desc: Test AddAgentUILaunchRecord add first record for new callerUid.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2001;
    std::string bundleName = "com.test.add";
    int32_t extensionAbilityId = 100;

    // Add first record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Verify record was added
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);

    // Check internal map size
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0100 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0200
 * @tc.desc: Test AddAgentUILaunchRecord add multiple records for same callerUid and bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2002;
    std::string bundleName = "com.test.multi";

    // Add multiple records
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 2);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 3);

    // Verify all records were added
    auto callerIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName;
        });
    ASSERT_NE(callerIt, extRecordMgr->agentUIExtensionRecords_.end());
    EXPECT_EQ(callerIt->targetRecordIds.size(), 3);

    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0200 end");
}

/**
 * @tc.name: AddAgentUILaunchRecord_0300
 * @tc.desc: Test AddAgentUILaunchRecord add duplicate record (same extensionAbilityId).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, AddAgentUILaunchRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0300 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 2003;
    std::string bundleName = "com.test.dup";
    int32_t extensionAbilityId = 100;

    // Add same record twice
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Should only have one record (duplicate ignored)
    auto callerIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName;
        });
    ASSERT_NE(callerIt, extRecordMgr->agentUIExtensionRecords_.end());
    EXPECT_EQ(callerIt->targetRecordIds.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "AddAgentUILaunchRecord_0300 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0100
 * @tc.desc: Test RemoveAgentUILaunchRecord when callerUid not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3001;
    std::string bundleName = "com.test.remove";
    int32_t extensionAbilityId = 100;

    // Remove from empty map, should not crash
    extRecordMgr->RemoveAgentUILaunchRecord(bundleName, extensionAbilityId);

    // Map should still be empty
    EXPECT_EQ(extRecordMgr->agentUIExtensionRecords_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0100 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0200
 * @tc.desc: Test RemoveAgentUILaunchRecord when bundleName not exists.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0200 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3002;
    std::string bundleName1 = "com.test.bundle1";
    std::string bundleName2 = "com.test.bundle2";
    int32_t extensionAbilityId = 100;

    // Add record for bundle1
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName1, extensionAbilityId);

    // Remove from bundle2 (not exists), should not crash
    extRecordMgr->RemoveAgentUILaunchRecord(bundleName2, extensionAbilityId);

    // bundle1 record should still exist
    auto callerIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName1](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName1;
        });
    ASSERT_NE(callerIt, extRecordMgr->agentUIExtensionRecords_.end());
    EXPECT_EQ(callerIt->targetRecordIds.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0200 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0400
 * @tc.desc: Test RemoveAgentUILaunchRecord remove one of multiple records.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0400 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3004;
    std::string bundleName = "com.test.multi";

    // Add multiple records
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 1);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 2);
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, 3);
    auto callerIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName;
        });
    ASSERT_NE(callerIt, extRecordMgr->agentUIExtensionRecords_.end());
    EXPECT_EQ(callerIt->targetRecordIds.size(), 3);

    // Remove one record
    extRecordMgr->RemoveAgentUILaunchRecord(bundleName, 2);
    callerIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName;
        });
    ASSERT_NE(callerIt, extRecordMgr->agentUIExtensionRecords_.end());
    EXPECT_EQ(callerIt->targetRecordIds.size(), 2);

    // Verify the correct record was removed
    EXPECT_NE(std::find(callerIt->targetRecordIds.begin(), callerIt->targetRecordIds.end(), 1),
        callerIt->targetRecordIds.end());
    EXPECT_EQ(std::find(callerIt->targetRecordIds.begin(), callerIt->targetRecordIds.end(), 2),
        callerIt->targetRecordIds.end());
    EXPECT_NE(std::find(callerIt->targetRecordIds.begin(), callerIt->targetRecordIds.end(), 3),
        callerIt->targetRecordIds.end());

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0400 end");
}

/**
 * @tc.name: RemoveAgentUILaunchRecord_0500
 * @tc.desc: Test RemoveAgentUILaunchRecord remove non-existent extensionAbilityId.
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RemoveAgentUILaunchRecord_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0500 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 3005;
    std::string bundleName = "com.test.remove";
    int32_t extensionAbilityId = 100;

    // Add record
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);

    // Remove non-existent extensionAbilityId
    extRecordMgr->RemoveAgentUILaunchRecord(bundleName, 999);

    // Original record should still exist
    auto remainIt = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [callerUid, &bundleName, extensionAbilityId](const auto &record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName &&
                record.targetRecordIds.size() == 1 && record.targetRecordIds[0] == extensionAbilityId;
        });
    EXPECT_NE(remainIt, extRecordMgr->agentUIExtensionRecords_.end());

    TAG_LOGI(AAFwkTag::TEST, "RemoveAgentUILaunchRecord_0500 end");
}

/**
 * @tc.name: RollbackAgentUILaunchRecord_0100
 * @tc.desc: Test RemoveAgentUILaunchRecord removes record correctly (simulating RollbackAgentUILaunchRecord).
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionRecordManagerTest, RollbackAgentUILaunchRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RollbackAgentUILaunchRecord_0100 start");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    EXPECT_NE(extRecordMgr, nullptr);

    int32_t callerUid = 1000;
    std::string bundleName = "com.test.agentui";
    int32_t extensionAbilityId = 1;
    extRecordMgr->AddAgentUILaunchRecord(callerUid, bundleName, extensionAbilityId);
    auto ret = extRecordMgr->CheckAgentUILaunchLimit(callerUid, bundleName);
    EXPECT_EQ(ret, ERR_OK);
    extRecordMgr->RemoveAgentUILaunchRecord(bundleName, extensionAbilityId);
    auto it = std::find_if(extRecordMgr->agentUIExtensionRecords_.begin(),
        extRecordMgr->agentUIExtensionRecords_.end(),
        [&callerUid, &bundleName](const auto& record) {
            return record.callerUid == callerUid && record.targetBundle == bundleName;
        });
    EXPECT_EQ(it, extRecordMgr->agentUIExtensionRecords_.end());

    TAG_LOGI(AAFwkTag::TEST, "RollbackAgentUILaunchRecord_0100 end");
}

/**
 * @tc.name: UpdateProcessName_CallerInstance_0100
 * @tc.desc: Test UpdateProcessName with PROCESS_MODE_CALLER_INSTANCE, appIndex == 0.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_CallerInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    // Create caller ability record and register with Token system
    AAFwk::AbilityRequest callerAbilityRequest;
    callerAbilityRequest.abilityInfo.bundleName = "com.test.caller";
    callerAbilityRequest.abilityInfo.name = "CallerAbility";
    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(callerAbilityRequest);
    ASSERT_NE(callerAbilityRecord, nullptr);
    callerAbilityRecord->SetPid(5678);
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);

    // Create target extension record
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.target";
    abilityRequest.abilityInfo.name = "TargetAbility";
    abilityRequest.callerToken = callerToken;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_CALLER_INSTANCE;

    std::string expectedProcess = std::string("com.test.target") + ":" + "TargetAbility" + ":5678";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_CallerInstance_0200
 * @tc.desc: Test UpdateProcessName with PROCESS_MODE_CALLER_INSTANCE, appIndex > 0.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_CallerInstance_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    // Create caller ability record
    AAFwk::AbilityRequest callerAbilityRequest;
    callerAbilityRequest.abilityInfo.bundleName = "com.test.caller";
    callerAbilityRequest.abilityInfo.name = "CallerAbility";
    auto callerAbilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(callerAbilityRequest);
    ASSERT_NE(callerAbilityRecord, nullptr);
    callerAbilityRecord->SetPid(1234);
    sptr<IRemoteObject> callerToken = callerAbilityRecord->GetToken();
    ASSERT_NE(callerToken, nullptr);

    // Create target extension record with appIndex > 0
    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.target";
    abilityRequest.abilityInfo.name = "TargetAbility";
    abilityRequest.callerToken = callerToken;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(2);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_CALLER_INSTANCE;

    // processAppIndex = abilityRecord->GetAppIndex() = 2, appendAppIndex adds ":2"
    std::string expectedProcess = std::string("com.test.target") + ":" + "TargetAbility" + ":1234" + ":" + "2";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_AgentUI_0100
 * @tc.desc: Test UpdateProcessName with PROCESS_MODE_AGENT_UI.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_AgentUI_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.agent";
    abilityRequest.abilityInfo.name = "AgentAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_AGENT_UI;

    std::string expectedProcess = std::string("com.test.agent") + ":" + "agent";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_RunWithMainProcess_0200
 * @tc.desc: Test UpdateProcessName PROCESS_MODE_RUN_WITH_MAIN_PROCESS with appIndex > 0, no appInfo.process.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_RunWithMainProcess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.mainprocess";
    abilityRequest.abilityInfo.name = "MainProcessAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(1);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_RUN_WITH_MAIN_PROCESS;

    std::string expectedProcess = std::string("com.test.mainprocess") + ":" + "1";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_RunWithMainProcess_0300
 * @tc.desc: Test UpdateProcessName PROCESS_MODE_RUN_WITH_MAIN_PROCESS with appInfo.process and appIndex > 0.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_RunWithMainProcess_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.mainprocess";
    abilityRequest.abilityInfo.name = "MainProcessAbility";
    abilityRequest.appInfo.process = "custom_process";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(3);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_RUN_WITH_MAIN_PROCESS;

    std::string expectedProcess = std::string("custom_process") + ":" + "3";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_Default_AppIndex_0100
 * @tc.desc: Test UpdateProcessName default case with empty moduleProcess and appIndex > 0.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_Default_AppIndex_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.default";
    abilityRequest.abilityInfo.name = "DefaultAbility";
    abilityRequest.abilityInfo.extensionTypeName = "UIExtension";
    abilityRequest.moduleProcess = "";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(1);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = 0;

    // default branch: moduleProcess empty, appIndex > 0
    std::string expectedProcess = std::string("com.test.default") + ":" + "UIExtension" + ":" + "1";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_Instance_AppIndex_0100
 * @tc.desc: Test UpdateProcessName PROCESS_MODE_INSTANCE with appIndex > 0 (appendAppIndex triggers).
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_Instance_AppIndex_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.instance";
    abilityRequest.abilityInfo.name = "InstanceAbility";
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetAppIndex(2);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_INSTANCE;

    auto extAbilityId = abilityRecord->GetUIExtensionAbilityId();
    std::string expectedProcess = std::string("com.test.instance") + ":" + "InstanceAbility"
        + ":" + std::to_string(extAbilityId) + ":" + "2";
    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(abilityRecord->GetProcessName(), expectedProcess);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_NullAbilityRecord_0100
 * @tc.desc: Test UpdateProcessName with null abilityRecord inside extension record.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_NullAbilityRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.null";
    abilityRequest.abilityInfo.name = "NullAbility";
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(nullptr);
    ASSERT_NE(extRecord, nullptr);
    extRecord->abilityRecord_ = nullptr;

    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: UpdateProcessName_CallerInstance_NullCaller_0100
 * @tc.desc: Test UpdateProcessName PROCESS_MODE_CALLER_INSTANCE with null callerRecord.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateProcessName_CallerInstance_NullCaller_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.test.target";
    abilityRequest.abilityInfo.name = "TargetAbility";
    // callerToken is null by default, so callerRecord will be null
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    std::shared_ptr<ExtensionRecord> extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->processMode_ = PROCESS_MODE_CALLER_INSTANCE;

    int32_t result = extRecordMgr->UpdateProcessName(abilityRequest, extRecord);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: MigrationFencing_ShouldExecute_CheckOnly_0100
 * @tc.desc: ShouldExecuteMigration is check-only on the execution branch: the fence is
 *           NOT advanced before the re-parent result is known
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_ShouldExecute_CheckOnly_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;
    state.currentParentWindowId = 5;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = 4;
    sessionInfo->hostWindowId = 9;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    // Fence must stay open until CommitMigrationState, so a failed re-parent stays retryable.
    EXPECT_EQ(state.lastGen, 3u);
    EXPECT_EQ(state.currentParentWindowId, 5u);
}

/**
 * @tc.name: MigrationFencing_CommitMigrationState_0100
 * @tc.desc: CommitMigrationState records the executed gen/parent; the committed gen and
 *           older gens are then fenced as stale (out-of-order protection regression)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_CommitMigrationState_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;
    state.currentParentWindowId = 5;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = 4;
    sessionInfo->hostWindowId = 9;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    CommitMigrationState(state, sessionInfo->originalCallerInfo.migrationGen, sessionInfo->hostWindowId);
    EXPECT_EQ(state.lastGen, 4u);
    EXPECT_EQ(state.currentParentWindowId, 9u);

    // Same gen (at-least-once replay after success) is fenced as stale.
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    // Older gen (late out-of-order command) is fenced as stale too.
    sessionInfo->originalCallerInfo.migrationGen = 3;
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    // Newer gen with a different host executes again.
    sessionInfo->originalCallerInfo.migrationGen = 5;
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
}

/**
 * @tc.name: MigrationFencing_RetryAfterFailedReParent_0100
 * @tc.desc: Without CommitMigrationState (re-parent failed), the SAME gen is re-executable;
 *           after commit (success), it is fenced. This is the RT-01 fix contract.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_RetryAfterFailedReParent_0100, TestSize.Level1)
{
    MigrationFencingState state;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = 1;
    sessionInfo->hostWindowId = 7;

    // First delivery: fence open, execution allowed.
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    // Re-parent failed: no commit. At-least-once re-delivery of the SAME gen must retry.
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    // Re-parent succeeded on retry: commit fences the gen.
    CommitMigrationState(state, sessionInfo->originalCallerInfo.migrationGen, sessionInfo->hostWindowId);
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
}

/**
 * @tc.name: MigrationFencing_NoOpBranch_AdvancesGen_0100
 * @tc.desc: A newer gen whose hostWindowId equals the current parent is a no-op: skipped
 *           but lastGen advances (no pending side effect)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_NoOpBranch_AdvancesGen_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;
    state.currentParentWindowId = 9;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = 5;
    sessionInfo->hostWindowId = 9;

    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_EQ(state.lastGen, 5u);
    EXPECT_EQ(state.currentParentWindowId, 9u);
}

/**
 * @tc.name: MigrationFencing_Guards_0100
 * @tc.desc: Non-migration inputs (null sessionInfo / null declared callerToken) never migrate
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_Guards_0100, TestSize.Level1)
{
    MigrationFencingState state;
    EXPECT_FALSE(ShouldExecuteMigration(nullptr, state));

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.migrationGen = 1;
    sessionInfo->hostWindowId = 7;
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_EQ(state.lastGen, 0u);
    EXPECT_EQ(state.currentParentWindowId, 0u);
}

/**
 * @tc.name: UpdateWindowParentForMigration_NullGuards_0100
 * @tc.desc: UpdateWindowParentForMigration rejects null sessionInfo / null window
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UpdateWindowParentForMigration_NullGuards_0100, TestSize.Level1)
{
    EXPECT_FALSE(UpdateWindowParentForMigration(nullptr, nullptr));
}

// Exposes the protected AddToServiceMap wrapper for the suffixed-key regression test.
class TestServiceMapManager : public AAFwk::AbilityConnectManager {
public:
    explicit TestServiceMapManager(int userId) : AbilityConnectManager(userId) {}
    void AddRecord(const std::string &key, std::shared_ptr<AAFwk::BaseExtensionRecord> record)
    {
        CallAddToServiceMap(key, record);
    }
};

/**
 * @tc.name: ServiceMap_UIExtensionSuffixedKey_LookupRegression_0100
 * @tc.desc: UIExtension records are keyed by element.GetURI()+UIExtensionAbilityId; the
 *           bare-key lookup used by the old replay path always misses them, while the
 *           queue-key (suffixed) lookup used by CompleteStartServiceReq hits (RT-03 fix)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, ServiceMap_UIExtensionSuffixedKey_LookupRegression_0100, TestSize.Level1)
{
    auto connectManager = std::make_shared<TestServiceMapManager>(100);
    ASSERT_NE(connectManager, nullptr);

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.abilityInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.moduleName = "entry";
    abilityRequest.appInfo.bundleName = abilityRequest.abilityInfo.bundleName;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);

    // UIExtension records enter serviceMap_ with the suffixed key (see
    // UIExtensionAbilityManager::GetOrCreateExtensionRecord / EnqueueStartServiceReq).
    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId,
        abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name,
        abilityRequest.abilityInfo.moduleName);
    const std::string suffixedKey = element.GetURI() + "7";
    connectManager->AddRecord(suffixedKey, abilityRecord);

    // Bare-key lookup rebuilt from the request must miss UIExtension records...
    EXPECT_EQ(connectManager->GetServiceRecordByAbilityRequest(abilityRequest), nullptr);
    // ...while the queue-key lookup (the fixed replay path) must hit.
    EXPECT_EQ(connectManager->GetServiceRecordByElementName(suffixedKey), abilityRecord);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationWindowZero_0100
 * @tc.desc: Valid declaration but target hostWindowId==0 is rejected (invalid target; also
 *           prevents the zero-sentinel silently confirming a no-op migration) (RT-04 fix)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationWindowZero_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = 100;
    createSessionInfo->persistentId = 10;
    createSessionInfo->hostWindowId = 7;
    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<ExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(5, extRecord);

    // Valid credential (declared previous host == recorded host) but target hostWindowId == 0.
    AAFwk::AbilityRequest zeroRequest;
    zeroRequest.appInfo.bundleName = hostBundle;
    zeroRequest.abilityInfo.name = "MainAbility";
    zeroRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> zeroSessionInfo(new AAFwk::SessionInfo());
    zeroSessionInfo->uiExtensionComponentId = 200;
    zeroSessionInfo->persistentId = 10;
    zeroSessionInfo->hostWindowId = 0;  // invalid target
    zeroSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    zeroSessionInfo->originalCallerInfo.callerToken = previousHostToken;
    zeroSessionInfo->originalCallerInfo.persistentId = 10;
    zeroRequest.sessionInfo = zeroSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = createdRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(zeroRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, createdRecord);  // untouched
    EXPECT_FALSE(isLoaded);
    // The gen must not be stamped for a rejected target.
    EXPECT_EQ(zeroSessionInfo->originalCallerInfo.migrationGen, 0u);
}

/**
 * @tc.name: MigrationFencing_HasHostCallerChanged_0100
 * @tc.desc: onCallerChanged latch semantics: fires once per identity change; a notified
 *           identity never notifies again; same-identity and null cases never fire
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationFencing_HasHostCallerChanged_0100, TestSize.Level1)
{
    MigrationFencingState state;
    // Null session never fires.
    EXPECT_FALSE(HasHostCallerChanged(nullptr, state));

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    // Both-null identity (unseeded latch, null caller): no notification.
    EXPECT_FALSE(HasHostCallerChanged(sessionInfo, state));

    // Creation seeding: the initial caller becomes known without notification.
    auto firstCaller = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    state.lastCallerToken = firstCaller;
    sessionInfo->callerToken = firstCaller;
    // Same-ability dual-window migration: identity unchanged, no notification.
    EXPECT_FALSE(HasHostCallerChanged(sessionInfo, state));

    // Host ability changed: notify once...
    auto secondCaller = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->callerToken = secondCaller;
    EXPECT_TRUE(HasHostCallerChanged(sessionInfo, state));
    // ...and the notified identity never notifies again.
    EXPECT_FALSE(HasHostCallerChanged(sessionInfo, state));
    EXPECT_EQ(state.lastCallerToken, secondCaller);

    // Migration back to the first caller is a new identity change: notifies again.
    sessionInfo->callerToken = firstCaller;
    EXPECT_TRUE(HasHostCallerChanged(sessionInfo, state));
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationRecordNotFound_0100
 * @tc.desc: A migration declaration whose persistentId matches no record is rejected
 *           visibly (ERR_INVALID_VALUE) — never a silent fallback that would create a
 *           new instance while SCB believes the migration succeeded (N-1)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, GetOrCreateExtensionRecord_MigrationRecordNotFound_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    // No record is registered for persistentId 42.
    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = 300;
    migrationSessionInfo->persistentId = 42;
    migrationSessionInfo->hostWindowId = 9;
    migrationSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.persistentId = 42;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = nullptr;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    // Rejected visibly: SCB gets the error and decides recovery, instead of AMS silently
    // creating a fresh instance (component restart) behind an ERR_OK.
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, nullptr);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: MigrationState_SetSessionInfoDoesNotRollback_0100
 * @tc.desc: G-1 invariant: replacing the record's sessionInfo never rolls the migration
 *           gen/marker back (the authority lives on the record, not the sessionInfo)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationState_SetSessionInfoDoesNotRollback_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = "com.example.unittest";
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);

    AAFwk::OriginalCallerInfo declaration;
    declaration.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    declaration.persistentId = 10;
    declaration.migrationGen = 5;
    record->SetMigrationDeclaration(declaration, 9);
    EXPECT_EQ(record->GetMigrationGen(), 5u);

    // A plain/stale sessionInfo (no declaration, old host window) replaces the record's
    // sessionInfo — the marker and gen must survive.
    auto plainInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    plainInfo->hostWindowId = 7;
    record->SetSessionInfo(plainInfo);

    AAFwk::OriginalCallerInfo restored;
    uint32_t targetHostWindowId = 0;
    EXPECT_TRUE(record->GetMigrationDeclaration(restored, targetHostWindowId));
    EXPECT_EQ(restored.migrationGen, 5u);
    EXPECT_EQ(targetHostWindowId, 9u);
    EXPECT_EQ(record->GetMigrationGen(), 5u);
}

/**
 * @tc.name: MigrationState_ConfirmByGen_0100
 * @tc.desc: Gen-matched confirm-clear: only the CURRENT generation's completion clears the
 *           marker; the gen itself is retained (monotonic across the record's lifetime)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, MigrationState_ConfirmByGen_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = "com.example.unittest";
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);

    EXPECT_EQ(record->AdvanceMigrationGen(), 1u);
    EXPECT_EQ(record->AdvanceMigrationGen(), 2u);

    AAFwk::OriginalCallerInfo declaration;
    declaration.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    declaration.persistentId = 10;
    declaration.migrationGen = 2;
    record->SetMigrationDeclaration(declaration, 9);

    // Stale generation completion: rejected, marker survives.
    EXPECT_FALSE(record->ConfirmMigrationDeclaration(1));
    AAFwk::OriginalCallerInfo restored;
    uint32_t target = 0;
    EXPECT_TRUE(record->GetMigrationDeclaration(restored, target));

    // Current generation completion: clears the marker, keeps the gen.
    EXPECT_TRUE(record->ConfirmMigrationDeclaration(2));
    EXPECT_FALSE(record->GetMigrationDeclaration(restored, target));
    EXPECT_EQ(record->GetMigrationGen(), 2u);
    EXPECT_EQ(record->AdvanceMigrationGen(), 3u);  // monotonic after confirm
}

/**
 * @tc.name: UIExtensionRecord_Update_StaleDeclarationStripped_0100
 * @tc.desc: With no pending migration on the record, a stale declaration riding a request
 *           is stripped — declarations only ever leave AMS stamped from the record state
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UIExtensionRecord_Update_StaleDeclarationStripped_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    AAFwk::AbilityRequest staleRequest = request;
    sptr<AAFwk::SessionInfo> staleSessionInfo(new AAFwk::SessionInfo());
    staleSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    staleSessionInfo->originalCallerInfo.migrationGen = 3;
    staleRequest.sessionInfo = staleSessionInfo;

    extRecord->Update(staleRequest);
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.callerToken, nullptr);
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.migrationGen, 3u);  // gen untouched, token stripped
}

/**
 * @tc.name: UIExtensionRecord_Update_ProjectionCarriesConsistentPair_0100
 * @tc.desc: While a migration is pending, the projection overwrites BOTH the declaration
 *           and the target hostWindowId on the request — the delivered pair is always the
 *           record's consistent pair (stale host window can never re-mount) (gen① closure)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerTest, UIExtensionRecord_Update_ProjectionCarriesConsistentPair_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetSessionInfo(request.sessionInfo);
    ASSERT_EQ(abilityRecord->AdvanceMigrationGen(), 1u);

    AAFwk::OriginalCallerInfo declaration;
    declaration.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    declaration.persistentId = 10;
    declaration.migrationGen = 1;
    abilityRecord->SetMigrationDeclaration(declaration, 9);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);

    // Stale request: old host window + old declaration gen.
    AAFwk::AbilityRequest staleRequest = request;
    sptr<AAFwk::SessionInfo> staleSessionInfo(new AAFwk::SessionInfo());
    staleSessionInfo->hostWindowId = 7;  // stale target
    staleSessionInfo->originalCallerInfo.callerToken = declaration.callerToken;
    staleSessionInfo->originalCallerInfo.migrationGen = 0;  // stale gen
    staleRequest.sessionInfo = staleSessionInfo;

    extRecord->Update(staleRequest);
    EXPECT_EQ(staleSessionInfo->hostWindowId, 9u);  // record's target wins
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.migrationGen, 1u);  // record's gen wins
}
} // namespace AbilityRuntime
} // namespace OHOS
