/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "gmock/gmock.h"

#define private public
#define protected public
#include "app_exit_reason_data_manager.h"
#include "mock_single_kv_store.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using testing::_;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string MODULE_NAME = "module_name";
const std::string ABILITY_NAME = "ability_name";
const std::string BUNDLE_NAME = "bundle_name";
constexpr uint32_t ACCESS_TOKEN_ID = 123;
const int SESSION_ID = 111;
}  // namespace

class AppExitReasonDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonDataManagerTest::SetUpTestCase(void)
{}

void AppExitReasonDataManagerTest::TearDownTestCase(void)
{}

void AppExitReasonDataManagerTest::SetUp()
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
}

void AppExitReasonDataManagerTest::TearDown()
{}

/**
 * @tc.name: AppExitReasonDataManager_AddAbilityRecoverInfo_001
 * @tc.desc: AddAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_AddAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAbilityRecoverInfo_001
 * @tc.desc: DeleteAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAbilityRecoverInfo_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001
 * @tc.desc: SetUIExtensionAbilityExitReason
 * @tc.type: FUNC
 */
HWTEST_F(
    AppExitReasonDataManagerTest, AppExitReasonDataManager_SetUIExtensionAbilityExitReason_001, TestSize.Level1)
{
    std::string bundleName = "com.test.demo";
    std::vector<std::string> extensionList;
    extensionList.push_back("testEntryUIExtAbility");
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetUIExtensionAbilityExitReason(
        bundleName, extensionList, exitReason, {}, false);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAbilityRecoverInfo_001
 * @tc.desc: GetAbilityRecoverInfo
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAbilityRecoverInfo_001, TestSize.Level1)
{
    bool hasRecoverInfo = false;
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    EXPECT_EQ(hasRecoverInfo, false);

    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->AddAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, SESSION_ID);
    EXPECT_EQ(result, ERR_OK);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAbilityRecoverInfo(
        ACCESS_TOKEN_ID, MODULE_NAME, ABILITY_NAME, hasRecoverInfo);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(hasRecoverInfo, true);
}

/**
 * @tc.name: AppExitReasonDataManager_SetAppExitReason_001
 * @tc.desc: SetAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetAppExitReason_001, TestSize.Level1)
{
    std::vector<std::string> abilityList;
    abilityList.push_back(ABILITY_NAME);
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        "", ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, 0, abilityList, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->SetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, abilityList, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAppExitReason_001
 * @tc.desc: DeleteAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAppExitReason_001, TestSize.Level1)
{
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, -1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME, 1, 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason("", 0);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME,
        ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAppExitReason(BUNDLE_NAME,
        ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_GetAppExitReason_001
 * @tc.desc: GetAppExitReason
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAppExitReason_001, TestSize.Level1)
{
    bool isSetReason = false;
    AAFwk::ExitReason exitReason = { AAFwk::REASON_JS_ERROR, "Js Error." };
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        "", ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, 0, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = kvStorePtr;
    EXPECT_CALL(*kvStorePtr, GetEntries(_, _)).Times(1)
        .WillOnce(DoAll(Return(DistributedKv::Status::ERROR)));
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    DistributedKv::Entry entry;
    entry.key = std::to_string(ACCESS_TOKEN_ID);;
    entry.value = "test_value";
    std::vector<DistributedKv::Entry> allEntries;
    allEntries.push_back(entry);
    EXPECT_CALL(*kvStorePtr, GetEntries(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->GetAppExitReason(
        BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, exitReason);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001
 * @tc.desc: DeleteAllRecoverInfoByTokenId
 * @tc.type: FUNC
 * @tc.require: issuesI7N79U
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_DeleteAllRecoverInfoByTokenId_001, TestSize.Level1)
{
    auto tempKv = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_;
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = nullptr;
    auto& tempStoreId =
        const_cast<DistributedKv::StoreId&>(DelayedSingleton<AppExitReasonDataManager>::GetInstance()->storeId_);
    tempStoreId.storeId = "app_**exit_reason_infos";
    auto result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->
        DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_NO_INIT);

    tempStoreId.storeId = "app_exit_reason_infos";
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->kvStorePtr_ = tempKv;
    result = DelayedSingleton<AppExitReasonDataManager>::GetInstance()->
        DeleteAllRecoverInfoByTokenId(ACCESS_TOKEN_ID);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: AppExitReasonDataManager_SetAppExitReasonWithCallerInfo_0100
 * @tc.desc: SetAppExitReason with caller info, read back and verify caller fields round-trip
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetAppExitReasonWithCallerInfo_0100, TestSize.Level1)
{
    auto manager = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    std::vector<std::string> abilityList = { ABILITY_NAME };
    AAFwk::ExitReason exitReason = { AAFwk::REASON_RESOURCE_CONTROL, "low memory kill" };
    AppExecFwk::RunningProcessInfo processInfo;
    processInfo.pid_ = 1000;
    processInfo.uid_ = 2000;
    ExitCallerInfo callerInfo;
    callerInfo.callerPid = 3000;
    callerInfo.callerProcessName = "caller_process";

    DistributedKv::Value value = manager->ConvertAppExitReasonInfoToValue(
        abilityList, exitReason, processInfo, false, callerInfo);
    AAFwk::ExitReason outReason;
    AppExecFwk::RunningProcessInfo outProcessInfo;
    std::vector<std::string> outAbilityList;
    int64_t timeStamp = 0;
    bool withKillMsg = false;
    ExitCallerInfo outCallerInfo;
    manager->ConvertAppExitReasonInfoFromValue(
        value, outReason, timeStamp, outAbilityList, outProcessInfo, withKillMsg, &outCallerInfo);
    EXPECT_EQ(outCallerInfo.callerPid, callerInfo.callerPid);
    EXPECT_EQ(outCallerInfo.callerProcessName, callerInfo.callerProcessName);
    EXPECT_EQ(outProcessInfo.pid_, processInfo.pid_);
    EXPECT_EQ(outReason.exitMsg, exitReason.exitMsg);

    DistributedKv::Value defaultValue = manager->ConvertAppExitReasonInfoToValue(
        abilityList, exitReason, processInfo, false);
    ExitCallerInfo defaultCallerInfo;
    manager->ConvertAppExitReasonInfoFromValue(
        defaultValue, outReason, timeStamp, outAbilityList, outProcessInfo, withKillMsg, &defaultCallerInfo);
    EXPECT_EQ(defaultCallerInfo.callerPid, -1);
    EXPECT_TRUE(defaultCallerInfo.callerProcessName.empty());

    auto tempKv = manager->kvStorePtr_;
    std::shared_ptr<MockSingleKvStore> kvStorePtr = std::make_shared<MockSingleKvStore>();
    manager->kvStorePtr_ = kvStorePtr;
    DistributedKv::Entry entry;
    entry.key = std::to_string(ACCESS_TOKEN_ID);
    entry.value = value;
    std::vector<DistributedKv::Entry> allEntries;
    allEntries.push_back(entry);
    EXPECT_CALL(*kvStorePtr, GetEntries(_, _)).Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(allEntries), Return(DistributedKv::Status::SUCCESS)));
    bool isSetReason = false;
    AAFwk::ExitReason readReason;
    AppExecFwk::RunningProcessInfo readProcessInfo;
    int64_t readTimeStamp = 0;
    bool readWithKillMsg = false;
    ExitCallerInfo readCallerInfo;
    auto result = manager->GetAppExitReason(BUNDLE_NAME, ACCESS_TOKEN_ID, ABILITY_NAME, isSetReason, readReason,
        readProcessInfo, readTimeStamp, readWithKillMsg, false, &readCallerInfo);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(isSetReason);
    EXPECT_EQ(readCallerInfo.callerPid, callerInfo.callerPid);
    EXPECT_EQ(readCallerInfo.callerProcessName, callerInfo.callerProcessName);
    manager->kvStorePtr_ = tempKv;
}

/**
 * @tc.name: AppExitReasonDataManager_GetAppExitReasonLegacyWithoutCallerFields_0200
 * @tc.desc: read back legacy record without caller fields, caller info falls back to defaults
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_GetAppExitReasonLegacyWithoutCallerFields_0200,
    TestSize.Level1)
{
    auto manager = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    nlohmann::json legacyJson = nlohmann::json {
        { "pid", 1000 },
        { "uid", 2000 },
        { "reason", static_cast<int32_t>(AAFwk::REASON_RESOURCE_CONTROL) },
        { "exit_msg", "legacy exit" },
        { "process_name", "com.test.demo" },
        { "time_stamp", 123 },
        { "ability_list", std::vector<std::string>{ ABILITY_NAME } },
    };
    DistributedKv::Value legacyValue(legacyJson.dump());
    AAFwk::ExitReason outReason;
    AppExecFwk::RunningProcessInfo outProcessInfo;
    std::vector<std::string> outAbilityList;
    int64_t timeStamp = 0;
    bool withKillMsg = false;
    ExitCallerInfo outCallerInfo;
    manager->ConvertAppExitReasonInfoFromValue(
        legacyValue, outReason, timeStamp, outAbilityList, outProcessInfo, withKillMsg, &outCallerInfo);
    EXPECT_EQ(outCallerInfo.callerPid, -1);
    EXPECT_TRUE(outCallerInfo.callerProcessName.empty());
    EXPECT_EQ(outProcessInfo.pid_, 1000);
    EXPECT_EQ(outProcessInfo.processName_, "com.test.demo");
    EXPECT_EQ(outReason.exitMsg, "legacy exit");
    ASSERT_EQ(outAbilityList.size(), 1U);
    EXPECT_EQ(outAbilityList[0], ABILITY_NAME);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
