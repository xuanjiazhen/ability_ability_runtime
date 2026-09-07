/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "gmock/gmock.h"
#include <memory>
#include <string>
#include <vector>
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
const std::string CALLER_PROCESS_NAME = "caller_process";
const std::string LEGACY_PROCESS_NAME = "com.test.demo";
const std::string LEGACY_EXIT_MSG = "legacy exit";
const std::string KILL_MSG = "low memory kill";
constexpr uint32_t ACCESS_TOKEN_ID = 123;
constexpr int32_t RECORD_PID = 1000;
constexpr int32_t RECORD_UID = 2000;
constexpr int32_t CALLER_PID = 3000;
constexpr int32_t INVALID_CALLER_PID = -1;
constexpr int64_t LEGACY_TIME_STAMP = 123;
} // namespace

class AppExitReasonDataManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppExitReasonDataManagerTest::SetUpTestCase(void) {}

void AppExitReasonDataManagerTest::TearDownTestCase(void) {}

void AppExitReasonDataManagerTest::SetUp()
{
    DelayedSingleton<AppExitReasonDataManager>::GetInstance()->DeleteAbilityRecoverInfo(ACCESS_TOKEN_ID, MODULE_NAME,
        ABILITY_NAME);
}

void AppExitReasonDataManagerTest::TearDown() {}

/**
 * @tc.name: AppExitReasonDataManager_SetAppExitReasonWithCallerInfo_0100
 * @tc.desc: SetAppExitReason with caller info, read back and verify caller fields round-trip
 * @tc.type: FUNC
 */
HWTEST_F(AppExitReasonDataManagerTest, AppExitReasonDataManager_SetAppExitReasonWithCallerInfo_0100, TestSize.Level1)
{
    auto manager = DelayedSingleton<AppExitReasonDataManager>::GetInstance();
    std::vector<std::string> abilityList = { ABILITY_NAME };
    AAFwk::ExitReason exitReason = { AAFwk::REASON_RESOURCE_CONTROL, KILL_MSG };
    AppExecFwk::RunningProcessInfo processInfo;
    processInfo.pid_ = RECORD_PID;
    processInfo.uid_ = RECORD_UID;
    ExitCallerInfo callerInfo;
    callerInfo.callerPid = CALLER_PID;
    callerInfo.callerProcessName = CALLER_PROCESS_NAME;

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
    EXPECT_EQ(defaultCallerInfo.callerPid, INVALID_CALLER_PID);
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
        { "pid", RECORD_PID },
        { "uid", RECORD_UID },
        { "reason", static_cast<int32_t>(AAFwk::REASON_RESOURCE_CONTROL) },
        { "exit_msg", LEGACY_EXIT_MSG },
        { "process_name", LEGACY_PROCESS_NAME },
        { "time_stamp", LEGACY_TIME_STAMP },
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
    EXPECT_EQ(outCallerInfo.callerPid, INVALID_CALLER_PID);
    EXPECT_TRUE(outCallerInfo.callerProcessName.empty());
    EXPECT_EQ(outProcessInfo.pid_, RECORD_PID);
    EXPECT_EQ(outProcessInfo.processName_, LEGACY_PROCESS_NAME);
    EXPECT_EQ(outReason.exitMsg, LEGACY_EXIT_MSG);
    ASSERT_EQ(outAbilityList.size(), 1U);
    EXPECT_EQ(outAbilityList[0], ABILITY_NAME);
}
} // namespace AbilityRuntime
} // namespace OHOS
