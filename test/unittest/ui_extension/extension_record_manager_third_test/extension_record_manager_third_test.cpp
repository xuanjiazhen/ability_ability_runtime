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
#define private public
#include "ability_connect_manager.h"
#undef private
#include "ability_record.h"
#include "base_extension_record.h"
#include "extension_record.h"
#define private public
#include "extension_record_manager.h"
#undef private
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "scene_board_judgement.h"
#include "session_info.h"
#include "ui_extension_migration_utils.h"
#include "ui_extension_record.h"

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

namespace {
constexpr const char *MIGRATION_TEST_BUNDLE = "com.example.unittest";
constexpr int32_t MIGRATION_PERSISTENT_ID = 10;
constexpr uint64_t MIGRATION_CREATE_COMPONENT_ID = 100;
constexpr uint64_t MIGRATION_REQUEST_COMPONENT_ID = 200;
constexpr uint32_t MIGRATION_HOST_WINDOW_A = 7;
constexpr uint32_t MIGRATION_HOST_WINDOW_B = 8;
constexpr uint32_t MIGRATION_HOST_WINDOW_C = 9;
constexpr int32_t MIGRATION_RECORD_ID = 5;
constexpr uint32_t MIGRATION_TEST_GEN = 5;
constexpr int32_t MIGRATION_TEST_PERSISTENT_ID_ALT = 42;
constexpr uint32_t MIGRATION_STALE_TARGET_WINDOW = 50;
constexpr uint32_t MIGRATION_DECLARATION_TARGET = 20;
constexpr uint32_t MIGRATION_DECLARATION_TARGET_ALT = 77;
constexpr uint32_t MIGRATION_DECLARATION_TARGET_ALT2 = 99;
struct MigrationRecordFixture {
    std::shared_ptr<ExtensionRecordManager> mgr;
    std::shared_ptr<AAFwk::BaseExtensionRecord> record;
    std::shared_ptr<UIExtensionRecord> wrapper;
    sptr<AppExecFwk::MockAbilityToken> hostToken;
};

MigrationRecordFixture SetupMigrationRecord(uint32_t hostWindowId)
{
    MigrationRecordFixture fx;
    fx.mgr = std::make_shared<ExtensionRecordManager>(0);
    fx.hostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = MIGRATION_TEST_BUNDLE;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createSessionInfo->hostWindowId = hostWindowId;
    createSessionInfo->callerToken = fx.hostToken;
    createRequest.sessionInfo = createSessionInfo;
    fx.record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    fx.record->SetSessionInfo(createSessionInfo);
    fx.wrapper = std::make_shared<UIExtensionRecord>(fx.record);
    fx.wrapper->hostBundleName_ = MIGRATION_TEST_BUNDLE;
    fx.record->SetUIExtensionAbilityId(MIGRATION_RECORD_ID);
    fx.mgr->AddExtensionRecord(MIGRATION_RECORD_ID, fx.wrapper);
    return fx;
}

AAFwk::AbilityRequest MakeMigrationRequest(uint32_t targetWindowId, const sptr<IRemoteObject> &prevHostToken)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = MIGRATION_TEST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    sessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    sessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    sessionInfo->hostWindowId = targetWindowId;
    sessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.callerToken = prevHostToken;
    sessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    request.sessionInfo = sessionInfo;
    return request;
}
} // namespace


/**
 * @tc.name: IsMigrationUpdateRequest_0100
 * @tc.desc: IsMigrationUpdateRequest detects migration flag in sessionInfo
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, IsMigrationUpdateRequest_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);

    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(nullptr));

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));

    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    EXPECT_FALSE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));

    sessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    EXPECT_TRUE(extRecordMgr->IsMigrationUpdateRequest(sessionInfo));
}

/**
 * @tc.name: GetAbilityRecordByPersistentId_0100
 * @tc.desc: GetAbilityRecordByPersistentId matches by persistentId with hostBundleName ownership
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetAbilityRecordByPersistentId_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = hostBundle;
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    sessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    sessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    abilityRequest.sessionInfo = sessionInfo;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    ASSERT_NE(abilityRecord, nullptr);
    auto extRecord = std::make_shared<ExtensionRecord>(abilityRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    bool terminatingMatch = false;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(nullptr, hostBundle, terminatingMatch), nullptr);

    sptr<AAFwk::SessionInfo> sessionParam(new AAFwk::SessionInfo());
    sessionParam->persistentId = 0;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), nullptr);

    sessionParam->persistentId = MIGRATION_PERSISTENT_ID;
    terminatingMatch = true;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), abilityRecord);
    EXPECT_FALSE(terminatingMatch);

    sessionParam->persistentId = 11;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, hostBundle, terminatingMatch), nullptr);

    sessionParam->persistentId = MIGRATION_PERSISTENT_ID;
    EXPECT_EQ(extRecordMgr->GetAbilityRecordByPersistentId(sessionParam, "com.example.other", terminatingMatch),
        nullptr);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationUpdate_0100
 * @tc.desc: Migration update request reuses the original record matched by persistentId
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationUpdate_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    createdRecord->SetUIExtensionAbilityId(MIGRATION_RECORD_ID);
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    createdRecord->SetSessionInfo(createSessionInfo);

    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    migrationSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    migrationSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_C;
    migrationSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.callerToken = previousHostToken;
    migrationSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = nullptr;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(isLoaded);
    EXPECT_EQ(resultRecord, createdRecord);
    auto migrationRetry = MakeMigrationRequest(MIGRATION_HOST_WINDOW_C, previousHostToken);
    ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRetry, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(resultRecord, createdRecord);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationDeclarationMismatch_0100
 * @tc.desc: Migration with a forged/mismatched previous-host declaration is rejected (fallback)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationDeclarationMismatch_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<UIExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    AAFwk::AbilityRequest forgedRequest;
    forgedRequest.appInfo.bundleName = hostBundle;
    forgedRequest.abilityInfo.name = "MainAbility";
    forgedRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> forgedSessionInfo(new AAFwk::SessionInfo());
    forgedSessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    forgedSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    forgedSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    forgedSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    forgedSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    forgedRequest.sessionInfo = forgedSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = createdRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(forgedRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, createdRecord);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationWindowUnchanged_0100
 * @tc.desc: Valid declaration with unchanged hostWindowId is rejected (no legal main-flow path)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationWindowUnchanged_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<UIExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    createdRecord->SetUIExtensionAbilityId(MIGRATION_RECORD_ID);
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    AAFwk::AbilityRequest noOpRequest;
    noOpRequest.appInfo.bundleName = hostBundle;
    noOpRequest.abilityInfo.name = "MainAbility";
    noOpRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> noOpSessionInfo(new AAFwk::SessionInfo());
    noOpSessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    noOpSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    noOpSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    noOpSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    noOpSessionInfo->originalCallerInfo.callerToken = previousHostToken;
    noOpSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
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
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationTerminatingFallback_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createRequest.sessionInfo = createSessionInfo;
    auto terminatingRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(terminatingRecord, nullptr);
    terminatingRecord->SetTerminatingState();
    auto extRecord = std::make_shared<UIExtensionRecord>(terminatingRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    migrationSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    migrationSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = terminatingRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, terminatingRecord);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: UIExtensionRecord_Update_MigrationRetention_0100
 * @tc.desc: Reuse request without migration flag inherits stable originalCallerInfo from record
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UIExtensionRecord_Update_MigrationRetention_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest savedRequest;
    savedRequest.appInfo.bundleName = hostBundle;
    savedRequest.abilityInfo.name = "MainAbility";
    savedRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> savedSessionInfo(new AAFwk::SessionInfo());
    savedSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    savedSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    savedSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    savedSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    savedRequest.sessionInfo = savedSessionInfo;
    auto previousHostToken = savedSessionInfo->originalCallerInfo.callerToken;
    auto abilityRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(savedRequest);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetSessionInfo(savedSessionInfo);
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);
    AAFwk::OriginalCallerInfo storedDeclaration = savedSessionInfo->originalCallerInfo;
    storedDeclaration.callerToken = nullptr;
    storedDeclaration.migrationGen = extRecord->AdvanceMigrationGen();
    extRecord->SetMigrationDeclaration(storedDeclaration, 9);
    auto delivery = sptr<AAFwk::SessionInfo>::MakeSptr(*savedSessionInfo);
    ASSERT_NE(delivery, nullptr);
    delivery->hostWindowId = MIGRATION_HOST_WINDOW_C;
    delivery->originalCallerInfo = storedDeclaration;
    extRecord->SetMigrationDeliverySessionInfo(delivery);

    AAFwk::AbilityRequest reuseRequest;
    reuseRequest.appInfo.bundleName = hostBundle;
    reuseRequest.abilityInfo.name = "MainAbility";
    reuseRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> reuseSessionInfo(new AAFwk::SessionInfo());
    reuseSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    reuseSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    reuseRequest.sessionInfo = reuseSessionInfo;

    extRecord->Update(reuseRequest);
    EXPECT_EQ(reuseSessionInfo->originalCallerInfo.callerToken, nullptr);
    EXPECT_EQ(reuseSessionInfo->hostWindowId, MIGRATION_HOST_WINDOW_C);
}

/**
 * @tc.name: MigrationFencing_ShouldExecute_CheckOnly_0100
 * @tc.desc: Checking a new generation does not advance the fence before the window API call.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationFencing_ShouldExecute_CheckOnly_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.migrationGen = 4;
    sessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_C;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_EQ(state.lastGen, 3u);
}

/**
 * @tc.name: MigrationFencing_AttemptedGeneration_0100
 * @tc.desc: Once the window API has been called, the same and older generations are fenced.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationFencing_AttemptedGeneration_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.migrationGen = 4;
    sessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_C;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    state.lastGen = sessionInfo->originalCallerInfo.migrationGen;
    EXPECT_EQ(state.lastGen, 4u);

    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    sessionInfo->originalCallerInfo.migrationGen = 3;
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
    sessionInfo->originalCallerInfo.migrationGen = MIGRATION_TEST_GEN;
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
}

/**
 * @tc.name: MigrationFencing_NoWindowRetryAfterAttempt_0100
 * @tc.desc: A window API error still consumes the generation. Re-delivery only resends the report.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationFencing_NoWindowRetryAfterAttempt_0100, TestSize.Level1)
{
    MigrationFencingState state;
    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = 1;
    sessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    state.lastGen = sessionInfo->originalCallerInfo.migrationGen;
    EXPECT_FALSE(ShouldExecuteMigration(sessionInfo, state));
}

/**
 * @tc.name: MigrationFencing_NewGenEligibleRegardlessOfTarget_0100
 * @tc.desc: A new generation is eligible without a cached-parent shortcut.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationFencing_NewGenEligibleRegardlessOfTarget_0100, TestSize.Level1)
{
    MigrationFencingState state;
    state.lastGen = 3;

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    sessionInfo->originalCallerInfo.migrationGen = MIGRATION_TEST_GEN;
    sessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_C;

    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_EQ(state.lastGen, 3u);
}

/**
 * @tc.name: MigrationFencing_Guards_0100
 * @tc.desc: Null sessionInfo never migrates; a new gen is eligible without changing the fence.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationFencing_Guards_0100, TestSize.Level1)
{
    MigrationFencingState state;
    EXPECT_FALSE(ShouldExecuteMigration(nullptr, state));

    sptr<AAFwk::SessionInfo> sessionInfo(new AAFwk::SessionInfo());
    ASSERT_NE(sessionInfo, nullptr);
    sessionInfo->originalCallerInfo.migrationGen = 1;
    sessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    EXPECT_TRUE(ShouldExecuteMigration(sessionInfo, state));
    EXPECT_EQ(state.lastGen, 0u);
}

/**
 * @tc.name: UpdateWindowParentForMigration_NullGuards_0100
 * @tc.desc: UpdateWindowParentForMigration rejects null window / null option
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UpdateWindowParentForMigration_NullGuards_0100, TestSize.Level1)
{
    EXPECT_FALSE(UpdateWindowParentForMigration(nullptr, nullptr));
}

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
HWTEST_F(ExtensionRecordManagerThirdTest, ServiceMap_UIExtensionSuffixedKey_LookupRegression_0100, TestSize.Level1)
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

    AppExecFwk::ElementName element(abilityRequest.abilityInfo.deviceId,
        abilityRequest.abilityInfo.bundleName, abilityRequest.abilityInfo.name,
        abilityRequest.abilityInfo.moduleName);
    const std::string suffixedKey = element.GetURI() + "7";
    connectManager->AddRecord(suffixedKey, abilityRecord);

    EXPECT_EQ(connectManager->GetServiceRecordByAbilityRequest(abilityRequest), nullptr);
    EXPECT_EQ(connectManager->GetServiceRecordByElementName(suffixedKey), abilityRecord);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationWindowZero_0100
 * @tc.desc: Valid declaration but target hostWindowId==0 is rejected (invalid target; also
 *           prevents the zero-sentinel silently confirming a no-op migration) (RT-04 fix)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationWindowZero_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest createRequest;
    createRequest.appInfo.bundleName = hostBundle;
    createRequest.abilityInfo.name = "MainAbility";
    createRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> createSessionInfo(new AAFwk::SessionInfo());
    createSessionInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    createSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    createSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    auto previousHostToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    createSessionInfo->callerToken = previousHostToken;
    createRequest.sessionInfo = createSessionInfo;
    auto createdRecord = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(createRequest);
    ASSERT_NE(createdRecord, nullptr);
    createdRecord->SetSessionInfo(createSessionInfo);
    auto extRecord = std::make_shared<UIExtensionRecord>(createdRecord);
    extRecord->hostBundleName_ = hostBundle;
    extRecordMgr->AddExtensionRecord(MIGRATION_RECORD_ID, extRecord);

    AAFwk::AbilityRequest zeroRequest;
    zeroRequest.appInfo.bundleName = hostBundle;
    zeroRequest.abilityInfo.name = "MainAbility";
    zeroRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> zeroSessionInfo(new AAFwk::SessionInfo());
    zeroSessionInfo->uiExtensionComponentId = MIGRATION_REQUEST_COMPONENT_ID;
    zeroSessionInfo->persistentId = MIGRATION_PERSISTENT_ID;
    zeroSessionInfo->hostWindowId = 0;
    zeroSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    zeroSessionInfo->originalCallerInfo.callerToken = previousHostToken;
    zeroSessionInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    zeroRequest.sessionInfo = zeroSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = createdRecord;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(zeroRequest, hostBundle, resultRecord, isLoaded);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(resultRecord, createdRecord);
    EXPECT_FALSE(isLoaded);
    EXPECT_EQ(zeroSessionInfo->originalCallerInfo.migrationGen, 0u);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationRecordNotFound_0100
 * @tc.desc: A migration declaration whose persistentId matches no record is rejected
 *           visibly (ERR_INVALID_VALUE) — never a silent fallback that would create a
 *           new instance while SCB believes the migration succeeded (N-1)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationRecordNotFound_0100, TestSize.Level1)
{
    auto extRecordMgr = std::make_shared<ExtensionRecordManager>(0);
    ASSERT_NE(extRecordMgr, nullptr);
    const std::string hostBundle = "com.example.unittest";

    AAFwk::AbilityRequest migrationRequest;
    migrationRequest.appInfo.bundleName = hostBundle;
    migrationRequest.abilityInfo.name = "MainAbility";
    migrationRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    sptr<AAFwk::SessionInfo> migrationSessionInfo(new AAFwk::SessionInfo());
    migrationSessionInfo->uiExtensionComponentId = 300;
    migrationSessionInfo->persistentId = MIGRATION_TEST_PERSISTENT_ID_ALT;
    migrationSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_C;
    migrationSessionInfo->callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    migrationSessionInfo->originalCallerInfo.persistentId = MIGRATION_TEST_PERSISTENT_ID_ALT;
    migrationRequest.sessionInfo = migrationSessionInfo;

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = nullptr;
    bool isLoaded = false;
    auto ret = extRecordMgr->GetOrCreateExtensionRecord(migrationRequest, hostBundle, resultRecord, isLoaded);
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
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationState_SetSessionInfoDoesNotRollback_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = "com.example.unittest";
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto uiRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(uiRecord, nullptr);

    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = MIGRATION_TEST_GEN;
    uiRecord->SetMigrationDeclaration(declaration, 9);
    EXPECT_EQ(uiRecord->GetMigrationGen(), 5u);

    auto plainInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    plainInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    record->SetSessionInfo(plainInfo);

    AAFwk::OriginalCallerInfo restored;
    uint32_t targetHostWindowId = 0;
    EXPECT_TRUE(uiRecord->GetMigrationDeclaration(restored, targetHostWindowId));
    EXPECT_EQ(restored.migrationGen, 5u);
    EXPECT_EQ(targetHostWindowId, MIGRATION_HOST_WINDOW_C);
    EXPECT_EQ(uiRecord->GetMigrationGen(), 5u);
}

/**
 * @tc.name: MigrationState_ConfirmByGen_0100
 * @tc.desc: Gen-matched confirm-clear: only the CURRENT generation's completion clears the
 *           marker; the gen itself is retained (monotonic across the record's lifetime)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationState_ConfirmByGen_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = "com.example.unittest";
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto uiRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(uiRecord, nullptr);

    EXPECT_EQ(uiRecord->AdvanceMigrationGen(), 1u);
    EXPECT_EQ(uiRecord->AdvanceMigrationGen(), 2u);

    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = 2;
    uiRecord->SetMigrationDeclaration(declaration, 9);

    EXPECT_FALSE(uiRecord->ConfirmMigrationDeclaration(1, 10));
    AAFwk::OriginalCallerInfo restored;
    uint32_t target = 0;
    EXPECT_TRUE(uiRecord->GetMigrationDeclaration(restored, target));

    EXPECT_TRUE(uiRecord->ConfirmMigrationDeclaration(2, 10));
    EXPECT_FALSE(uiRecord->GetMigrationDeclaration(restored, target));
    EXPECT_EQ(uiRecord->GetMigrationGen(), 2u);
    EXPECT_EQ(uiRecord->AdvanceMigrationGen(), 3u);
}

/**
 * @tc.name: UIExtensionRecord_Update_StaleDeclarationStripped_0100
 * @tc.desc: With no pending migration on the record, a stale declaration riding a request
 *           is stripped — declarations only ever leave the service stamped from the record state
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UIExtensionRecord_Update_StaleDeclarationStripped_0100, TestSize.Level1)
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
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.migrationGen, 0u);
}

/**
 * @tc.name: UIExtensionRecord_Update_ProjectionCarriesConsistentPair_0100
 * @tc.desc: While a migration is pending, the projection overwrites BOTH the declaration
 *           and the target hostWindowId on the request — the delivered pair is always the
 *           record's consistent pair (stale host window can never re-mount) (gen① closure)
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UIExtensionRecord_Update_ProjectionCarriesConsistentPair_0100,
    TestSize.Level1)
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
    auto extRecord = std::make_shared<UIExtensionRecord>(abilityRecord);
    ASSERT_NE(extRecord, nullptr);
    ASSERT_EQ(extRecord->AdvanceMigrationGen(), 1u);

    AAFwk::OriginalCallerInfo declaration;
    declaration.callerToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = 1;
    extRecord->SetMigrationDeclaration(declaration, 9);

    auto targetToken = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    auto delivery = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(delivery, nullptr);
    delivery->hostWindowId = MIGRATION_HOST_WINDOW_C;
    delivery->realHostWindowId = MIGRATION_DECLARATION_TARGET_ALT;
    delivery->displayId = MIGRATION_DECLARATION_TARGET_ALT2;
    delivery->callerToken = targetToken;
    extRecord->SetMigrationDeliverySessionInfo(delivery);

    AAFwk::AbilityRequest staleRequest = request;
    sptr<AAFwk::SessionInfo> staleSessionInfo(new AAFwk::SessionInfo());
    staleSessionInfo->hostWindowId = MIGRATION_HOST_WINDOW_A;
    staleSessionInfo->originalCallerInfo.callerToken = declaration.callerToken;
    staleSessionInfo->originalCallerInfo.migrationGen = 0;
    staleRequest.sessionInfo = staleSessionInfo;

    extRecord->Update(staleRequest);
    EXPECT_EQ(staleSessionInfo->hostWindowId, MIGRATION_HOST_WINDOW_C);
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.migrationGen, 1u);
    EXPECT_EQ(staleSessionInfo->originalCallerInfo.callerToken, nullptr);
    EXPECT_EQ(staleSessionInfo->callerToken.GetRefPtr(), targetToken.GetRefPtr());
    EXPECT_EQ(staleSessionInfo->realHostWindowId, static_cast<int32_t>(MIGRATION_DECLARATION_TARGET_ALT));
    EXPECT_EQ(staleSessionInfo->displayId, MIGRATION_DECLARATION_TARGET_ALT2);
}

/**
 * @tc.name: MigrationChain_ParentUpdateInvoked_0100
 * @tc.desc: A-1 integration: full migration lifecycle — declare → advance gen → project →
 *           confirm-clear. Simulates the UEA parent-update invocation report.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationChain_ParentUpdateInvoked_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(extRecord, nullptr);

    uint32_t gen = extRecord->AdvanceMigrationGen();
    ASSERT_EQ(gen, 1u);
    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = gen;
    extRecord->SetMigrationDeclaration(declaration, MIGRATION_TEST_PERSISTENT_ID_ALT);

    AAFwk::OriginalCallerInfo projected;
    uint32_t targetHostWindowId = 0;
    ASSERT_TRUE(extRecord->GetMigrationDeclaration(projected, targetHostWindowId));
    EXPECT_EQ(projected.migrationGen, gen);
    EXPECT_EQ(targetHostWindowId, 42u);

    EXPECT_FALSE(extRecord->ConfirmMigrationDeclaration(0, 10));
    EXPECT_FALSE(extRecord->ConfirmMigrationDeclaration(gen + 1, 10));
    EXPECT_TRUE(extRecord->ConfirmMigrationDeclaration(gen, 10));

    AAFwk::OriginalCallerInfo emptyDecl;
    uint32_t emptyTarget = 0;
    EXPECT_FALSE(extRecord->GetMigrationDeclaration(emptyDecl, emptyTarget));

    EXPECT_EQ(extRecord->AdvanceMigrationGen(), 2u);
}

/**
 * @tc.name: MigrationChain_ReplayStripAndProject_0100
 * @tc.desc: A-1 integration: simulates the BACKGROUNDING → enqueue → replay flow —
 *           a queued stale declaration is stripped, then the projection re-arms it
 *           with the record's current (gen, target) pair.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationChain_ReplayStripAndProject_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(extRecord, nullptr);

    uint32_t gen = extRecord->AdvanceMigrationGen();
    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = gen;
    extRecord->SetMigrationDeclaration(declaration, MIGRATION_DECLARATION_TARGET_ALT2);

    sptr<AAFwk::SessionInfo> replayInfo(new AAFwk::SessionInfo());
    replayInfo->hostWindowId = MIGRATION_STALE_TARGET_WINDOW;
    replayInfo->originalCallerInfo.migrationGen = gen;
    AAFwk::AbilityRequest replayRequest = request;
    replayRequest.sessionInfo = replayInfo;

    replayInfo->originalCallerInfo.migrationGen = 0;

    AAFwk::OriginalCallerInfo projected;
    uint32_t targetHostWindowId = 0;
    ASSERT_TRUE(extRecord->GetMigrationDeclaration(projected, targetHostWindowId));
    replayInfo->originalCallerInfo = projected;
    replayInfo->hostWindowId = targetHostWindowId;

    EXPECT_EQ(replayInfo->originalCallerInfo.migrationGen, gen);
    EXPECT_EQ(replayInfo->hostWindowId, 99u);

    MigrationFencingState fence;
    EXPECT_TRUE(ShouldExecuteMigration(replayInfo, fence));

    fence.lastGen = gen;
    EXPECT_TRUE(extRecord->ConfirmMigrationDeclaration(gen, 10));
    AAFwk::OriginalCallerInfo empty;
    uint32_t emptyTarget = 0;
    EXPECT_FALSE(extRecord->GetMigrationDeclaration(empty, emptyTarget));
}

/**
 * @tc.name: MigrationChain_PreInvocationFailureKeepsMarker_0100
 * @tc.desc: If window preparation fails before UEA invokes the window API, UEA sends no
 *           report. The pending declaration remains available for a later dispatch.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationChain_PreInvocationFailureKeepsMarker_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(extRecord, nullptr);

    uint32_t gen = extRecord->AdvanceMigrationGen();
    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = gen;
    extRecord->SetMigrationDeclaration(declaration, MIGRATION_DECLARATION_TARGET_ALT);

    AAFwk::OriginalCallerInfo still;
    uint32_t stillTarget = 0;
    EXPECT_TRUE(extRecord->GetMigrationDeclaration(still, stillTarget));
    EXPECT_EQ(still.migrationGen, gen);

    AAFwk::OriginalCallerInfo retry;
    uint32_t retryTarget = 0;
    EXPECT_TRUE(extRecord->GetMigrationDeclaration(retry, retryTarget));
    EXPECT_EQ(retry.migrationGen, gen);
    EXPECT_EQ(retryTarget, 77u);

    EXPECT_TRUE(extRecord->ConfirmMigrationDeclaration(gen, 10));
}

/**
 * @tc.name: MigrationChain_RetryAfterProjectionPollution_0100
 * @tc.desc: R1 fix: after migration declaration + projection (which pollutes
 *           record.sessionInfo_->hostWindowId to target), a retry with the same
 *           target must still pass the window gate — confirmedHostWindowId_ is
 *           only updated on ConfirmMigrationDeclaration, not on SetSessionInfo.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, MigrationChain_RetryAfterProjectionPollution_0100, TestSize.Level1)
{
    const std::string hostBundle = "com.example.unittest";
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = hostBundle;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    request.sessionInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto extRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(extRecord, nullptr);

    uint32_t gen = extRecord->AdvanceMigrationGen();
    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = gen;
    extRecord->SetMigrationDeclaration(declaration, MIGRATION_DECLARATION_TARGET);

    sptr<AAFwk::SessionInfo> projectedInfo(new AAFwk::SessionInfo(*request.sessionInfo));
    projectedInfo->hostWindowId = MIGRATION_DECLARATION_TARGET;
    record->SetSessionInfo(projectedInfo);

    EXPECT_EQ(extRecord->GetConfirmedHostWindowId(), 0u);

    EXPECT_NE(extRecord->GetConfirmedHostWindowId(), 20u);

    uint32_t retryGen = extRecord->AdvanceMigrationGen();
    EXPECT_EQ(retryGen, gen + 1);

    extRecord->SetMigrationDeclaration(declaration, MIGRATION_DECLARATION_TARGET);
    declaration.migrationGen = retryGen;
    extRecord->SetMigrationDeclaration(declaration, MIGRATION_DECLARATION_TARGET);
    EXPECT_TRUE(extRecord->ConfirmMigrationDeclaration(retryGen, 10));
    EXPECT_EQ(extRecord->GetConfirmedHostWindowId(), 20u);

    EXPECT_EQ(extRecord->GetConfirmedHostWindowId(), 20u);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationRetrySameTarget_0100
 * @tc.desc: Unconfirmed-migration state — while a migration is unconfirmed the record
 *           keeps its committed sessionInfo, so an at-least-once retry of the SAME target
 *           passes both the credential gate and the confirmed-window gate
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationRetrySameTarget_0100, TestSize.Level1)
{
    auto fx = SetupMigrationRecord(MIGRATION_HOST_WINDOW_A);
    ASSERT_NE(fx.mgr, nullptr);
    ASSERT_NE(fx.record, nullptr);

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;
    auto first = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(first, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_TRUE(isLoaded);

    EXPECT_EQ(fx.record->GetSessionInfo()->hostWindowId, MIGRATION_HOST_WINDOW_A);
    EXPECT_EQ(fx.record->GetSessionInfo()->callerToken.GetRefPtr(), fx.hostToken.GetRefPtr());

    auto retry = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    auto latestTokenB = retry.sessionInfo->callerToken;
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(retry, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_TRUE(isLoaded);

    auto advancedHostRetry = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, latestTokenB);
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(
        advancedHostRetry, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_TRUE(isLoaded);
}

/**
 * @tc.name: UIExtensionRecord_ConfirmAdoptsDeliverySession_0100
 * @tc.desc: Gen-matched confirmation adopts a sanitized copy of the delivered (new-host)
 *           session as the record's committed sessionInfo — token and window advance, the
 *           pending marker does not leak into the committed state
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, UIExtensionRecord_ConfirmAdoptsDeliverySession_0100, TestSize.Level1)
{
    AAFwk::AbilityRequest request;
    request.appInfo.bundleName = MIGRATION_TEST_BUNDLE;
    request.abilityInfo.name = "MainAbility";
    request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto record = AAFwk::BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(record, nullptr);
    auto uiRecord = std::make_shared<UIExtensionRecord>(record);
    ASSERT_NE(uiRecord, nullptr);

    auto fact = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    fact->hostWindowId = MIGRATION_HOST_WINDOW_A;
    auto tokenA = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    fact->callerToken = tokenA;
    record->SetSessionInfo(fact);

    uiRecord->SeedConfirmedHostWindowId(7);
    EXPECT_EQ(uiRecord->GetConfirmedHostWindowId(), MIGRATION_HOST_WINDOW_A);

    uint32_t gen = uiRecord->AdvanceMigrationGen();
    AAFwk::OriginalCallerInfo declaration;
    declaration.persistentId = MIGRATION_PERSISTENT_ID;
    declaration.migrationGen = gen;
    uiRecord->SetMigrationDeclaration(declaration, 9);

    auto delivery = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    delivery->hostWindowId = MIGRATION_HOST_WINDOW_C;
    auto tokenB = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    delivery->callerToken = tokenB;
    delivery->originalCallerInfo.migrationGen = gen;
    uiRecord->SetMigrationDeliverySessionInfo(delivery);

    EXPECT_EQ(record->GetSessionInfo()->hostWindowId, MIGRATION_HOST_WINDOW_A);
    EXPECT_EQ(record->GetSessionInfo()->callerToken.GetRefPtr(), tokenA.GetRefPtr());

    EXPECT_FALSE(uiRecord->ConfirmMigrationDeclaration(gen, MIGRATION_TEST_PERSISTENT_ID_ALT));
    AAFwk::OriginalCallerInfo pendingDeclaration;
    uint32_t pendingTarget = 0;
    EXPECT_TRUE(uiRecord->GetMigrationDeclaration(pendingDeclaration, pendingTarget));

    EXPECT_TRUE(uiRecord->ConfirmMigrationDeclaration(gen, MIGRATION_PERSISTENT_ID));
    EXPECT_EQ(record->GetSessionInfo()->hostWindowId, MIGRATION_HOST_WINDOW_C);
    EXPECT_EQ(record->GetSessionInfo()->callerToken.GetRefPtr(), tokenB.GetRefPtr());
    EXPECT_EQ(record->GetSessionInfo()->originalCallerInfo.migrationGen, 0u);
    EXPECT_EQ(uiRecord->GetConfirmedHostWindowId(), MIGRATION_HOST_WINDOW_C);
    EXPECT_TRUE(uiRecord->ConfirmMigrationDeclaration(gen, MIGRATION_PERSISTENT_ID));
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationPendingCredentialChain_0100
 * @tc.desc: A pending migration accepts the pending target credential for a new target and
 *           keeps the committed fact frozen until confirmation.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationPendingCredentialChain_0100,
    TestSize.Level1)
{
    auto fx = SetupMigrationRecord(MIGRATION_HOST_WINDOW_A);
    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;

    auto first = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    auto tokenB = first.sessionInfo->callerToken;
    ASSERT_EQ(fx.mgr->GetOrCreateExtensionRecord(first, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    uint32_t firstGen = fx.wrapper->GetMigrationGen();

    auto second = MakeMigrationRequest(MIGRATION_HOST_WINDOW_C, tokenB);
    auto tokenC = second.sessionInfo->callerToken;
    ASSERT_EQ(fx.mgr->GetOrCreateExtensionRecord(second, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_EQ(fx.wrapper->GetMigrationGen(), firstGen + 1);

    auto delivery = fx.wrapper->BuildMigrationDeliverySessionInfo();
    ASSERT_NE(delivery, nullptr);
    EXPECT_EQ(delivery->hostWindowId, MIGRATION_HOST_WINDOW_C);
    EXPECT_EQ(delivery->callerToken.GetRefPtr(), tokenC.GetRefPtr());
    EXPECT_EQ(delivery->originalCallerInfo.callerToken, nullptr);
    EXPECT_EQ(delivery->originalCallerInfo.migrationGen, firstGen + 1);
    EXPECT_EQ(fx.record->GetSessionInfo()->hostWindowId, MIGRATION_HOST_WINDOW_A);
    EXPECT_EQ(fx.record->GetSessionInfo()->callerToken.GetRefPtr(), fx.hostToken.GetRefPtr());
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationPendingRejectsStaleSource_0100
 * @tc.desc: A stale committed-host credential may retry the same target but cannot replace a
 *           pending migration with a different target.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationPendingRejectsStaleSource_0100,
    TestSize.Level1)
{
    auto fx = SetupMigrationRecord(MIGRATION_HOST_WINDOW_A);
    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;

    auto first = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    ASSERT_EQ(fx.mgr->GetOrCreateExtensionRecord(first, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    uint32_t firstGen = fx.wrapper->GetMigrationGen();

    auto staleSource = MakeMigrationRequest(MIGRATION_HOST_WINDOW_C, fx.hostToken);
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(staleSource, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded),
        ERR_INVALID_VALUE);
    EXPECT_EQ(fx.wrapper->GetMigrationGen(), firstGen);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationReturnsToConfirmedHost_0100
 * @tc.desc: A pending target may be superseded by the confirmed host using the pending target
 *           credential; a new generation prevents the old completion from winning.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationReturnsToConfirmedHost_0100,
    TestSize.Level1)
{
    auto fx = SetupMigrationRecord(MIGRATION_HOST_WINDOW_A);
    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;

    auto first = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    auto tokenB = first.sessionInfo->callerToken;
    ASSERT_EQ(fx.mgr->GetOrCreateExtensionRecord(first, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    uint32_t firstGen = fx.wrapper->GetMigrationGen();

    auto backToConfirmed = MakeMigrationRequest(MIGRATION_HOST_WINDOW_A, tokenB);
    ASSERT_EQ(fx.mgr->GetOrCreateExtensionRecord(
        backToConfirmed, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_EQ(fx.wrapper->GetMigrationGen(), firstGen + 1);
    auto delivery = fx.wrapper->BuildMigrationDeliverySessionInfo();
    ASSERT_NE(delivery, nullptr);
    EXPECT_EQ(delivery->hostWindowId, MIGRATION_HOST_WINDOW_A);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationRedeclareConfirmedTarget_0100
 * @tc.desc: After confirmation the committed sessionInfo carries the NEW host token and
 *           window, so a re-declaration of the confirmed target passes the credential gate
 *           but is rejected by the confirmed-window gate
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationRedeclareConfirmedTarget_0100,
    TestSize.Level1)
{
    auto fx = SetupMigrationRecord(MIGRATION_HOST_WINDOW_A);
    ASSERT_NE(fx.mgr, nullptr);
    ASSERT_NE(fx.record, nullptr);

    auto first = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, fx.hostToken);
    sptr<IRemoteObject> tokenB = first.sessionInfo->callerToken;
    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(first, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded), ERR_OK);
    EXPECT_TRUE(fx.wrapper->ConfirmMigrationDeclaration(fx.wrapper->GetMigrationGen(), 10));
    EXPECT_EQ(fx.record->GetSessionInfo()->originalCallerInfo.callerToken, nullptr);

    auto redeclare = MakeMigrationRequest(MIGRATION_HOST_WINDOW_B, tokenB);
    isLoaded = false;
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(redeclare, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded),
        ERR_INVALID_VALUE);
    EXPECT_FALSE(isLoaded);
}

/**
 * @tc.name: GetOrCreateExtensionRecord_MigrationNoRecordedSession_0100
 * @tc.desc: Migration is rejected when the record has no recorded session info
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(ExtensionRecordManagerThirdTest, GetOrCreateExtensionRecord_MigrationNoRecordedSession_0100, TestSize.Level1)
{
    auto fx = SetupMigrationRecord(7);
    ASSERT_NE(fx.mgr, nullptr);
    ASSERT_NE(fx.record, nullptr);

    AAFwk::AbilityRequest stripRequest;
    stripRequest.appInfo.bundleName = MIGRATION_TEST_BUNDLE;
    stripRequest.abilityInfo.name = "MainAbility";
    stripRequest.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    auto emptyInfo = sptr<AAFwk::SessionInfo>(new AAFwk::SessionInfo());
    emptyInfo->uiExtensionComponentId = MIGRATION_CREATE_COMPONENT_ID;
    emptyInfo->persistentId = MIGRATION_PERSISTENT_ID;
    emptyInfo->hostWindowId = MIGRATION_HOST_WINDOW_B;
    emptyInfo->originalCallerInfo.callerToken = fx.hostToken;
    emptyInfo->originalCallerInfo.persistentId = MIGRATION_PERSISTENT_ID;
    stripRequest.sessionInfo = emptyInfo;
    fx.record->SetSessionInfo(nullptr);

    std::shared_ptr<AAFwk::BaseExtensionRecord> resultRecord = fx.record;
    bool isLoaded = false;
    EXPECT_EQ(fx.mgr->GetOrCreateExtensionRecord(stripRequest, MIGRATION_TEST_BUNDLE, resultRecord, isLoaded),
        ERR_INVALID_VALUE);
    EXPECT_FALSE(isLoaded);
}
} // namespace AbilityRuntime
} // namespace OHOS
