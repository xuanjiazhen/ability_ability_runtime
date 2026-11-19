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
#include <map>
#include <memory>
#include <set>
#define private public
#include "app_spawn_client.h"
#undef private
#include "errors.h"
using namespace testing::ext;
namespace {
int calls = 0;
int failAt = 0;
constexpr int MOCK_FAILURE = -123;
AppDacInfo recordedDac {};
std::string recordedName;
std::string recordedValue;
int SpawnResult() { return ++calls == failAt ? MOCK_FAILURE : 0; }
}
extern "C" int AppSpawnReqMsgSetAppDacInfo(AppSpawnReqMsgHandle, const AppDacInfo *info)
{
    recordedDac = *info;
    return SpawnResult();
}
extern "C" int AppSpawnReqMsgSetBundleInfo(AppSpawnReqMsgHandle, uint32_t, const char *) { return SpawnResult(); }
extern "C" int AppSpawnReqMsgSetAppInternetPermissionInfo(AppSpawnReqMsgHandle, uint8_t, uint8_t)
{
    return SpawnResult();
}
extern "C" int AppSpawnReqMsgSetAppOwnerId(AppSpawnReqMsgHandle, const char *) { return SpawnResult(); }
extern "C" int AppSpawnReqMsgSetAppAccessToken(AppSpawnReqMsgHandle, uint64_t) { return SpawnResult(); }
extern "C" int AppSpawnReqMsgSetAppDomainInfo(AppSpawnReqMsgHandle, uint32_t, const char *) { return SpawnResult(); }
extern "C" int AppSpawnReqMsgAddStringInfo(AppSpawnReqMsgHandle, const char *name, const char *value)
{
    recordedName = name;
    recordedValue = value;
    return SpawnResult();
}
extern "C" int AppSpawnReqMsgSetAppFlag(AppSpawnReqMsgHandle, AppFlagsIndex) { return SpawnResult(); }
namespace OHOS::AppExecFwk {
class LaunchServiceSpawnTest : public testing::Test {
public:
    void SetUp() override { calls = 0; failAt = 0; recordedName.clear(); recordedValue.clear(); }
    AppSpawnClient client_ {"appspawn"};
};

/**
 * @tc.name: SetDefaultMsgIdentity_001
 * @tc.desc: Each external AppSpawn identity failure is returned immediately; success preserves DAC.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceSpawnTest, SetDefaultMsgIdentity_001, TestSize.Level1)
{
    AppSpawnStartMsg message {};
    message.code = MSG_SPAWN_SERVICE_BIN;
    message.uid = 20000001;
    message.gid = 20000001;
    message.appUid = 20000002;
    message.ownerId = "owner";
    for (int step = 1; step <= 6; ++step) {
        calls = 0;
        failAt = step;
        EXPECT_EQ(client_.SetDefaultMsgIdentity(message, nullptr), MOCK_FAILURE);
        EXPECT_EQ(calls, step);
    }
    calls = 0;
    failAt = 0;
    EXPECT_EQ(client_.SetDefaultMsgIdentity(message, nullptr), ERR_OK);
    EXPECT_EQ(calls, 6);
    EXPECT_EQ(recordedDac.uid, message.uid);
    EXPECT_EQ(recordedDac.appUid, message.appUid);
}

/**
 * @tc.name: SetDaemonMsgOptions_001
 * @tc.desc: Daemon path and start-flag failures propagate without constructing later fields.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceSpawnTest, SetDaemonMsgOptions_001, TestSize.Level1)
{
    AppSpawnStartMsg message {};
    message.code = MSG_SPAWN_SERVICE_BIN;
    message.daemonBinPath = "/data/bundles/worker";
    failAt = 1;
    EXPECT_EQ(client_.SetDaemonMsgOptions(message, nullptr), MOCK_FAILURE);
    EXPECT_EQ(recordedName, MSG_EXT_NAME_DAEMON_BIN_PATH);
    EXPECT_EQ(recordedValue, message.daemonBinPath);
    calls = 0;
    failAt = 2;
    EXPECT_EQ(client_.SetDaemonMsgOptions(message, nullptr), MOCK_FAILURE);
    EXPECT_EQ(calls, 2);
}

/**
 * @tc.name: SetDefaultMsgOptions_001
 * @tc.desc: Ordinary process start-flag failure is preserved.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceSpawnTest, SetDefaultMsgOptions_001, TestSize.Level1)
{
    AppSpawnStartMsg message {};
    message.flags = 1;
    failAt = 1;
    EXPECT_EQ(client_.SetDefaultMsgOptions(message, nullptr), MOCK_FAILURE);
    EXPECT_EQ(calls, 1);
}

/**
 * @tc.name: VerifyStartMsgGroups_001
 * @tc.desc: Reject negative gids and combined group-count overflow.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceSpawnTest, VerifyStartMsgGroups_001, TestSize.Level1)
{
    AppSpawnStartMsg message {};
    EXPECT_TRUE(client_.VerifyStartMsgGroups(message));
    message.gids = {-1};
    EXPECT_FALSE(client_.VerifyStartMsgGroups(message));
    message.gids.assign(APP_MAX_GIDS, 1);
    EXPECT_TRUE(client_.VerifyStartMsgGroups(message));
    message.dataGroupInfoList.resize(1);
    EXPECT_FALSE(client_.VerifyStartMsgGroups(message));
    message.gids.clear();
    message.dataGroupInfoList.resize(APP_MAX_GIDS + 1);
    EXPECT_FALSE(client_.VerifyStartMsgGroups(message));
}

/**
 * @tc.name: VerifyProcessStartMsg_001
 * @tc.desc: Validate daemon absolute path, process name and numeric identities.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceSpawnTest, VerifyProcessStartMsg_001, TestSize.Level1)
{
    AppSpawnStartMsg message {};
    message.code = MSG_SPAWN_SERVICE_BIN;
    message.uid = 20000001;
    message.gid = 20000001;
    message.procName = "com.example.daemon:worker";
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
    message.daemonBinPath = "relative/worker";
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
    message.daemonBinPath = "/data/bundles/worker";
    EXPECT_TRUE(client_.VerifyProcessStartMsg(message));
    message.uid = -1;
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
    message.uid = 20000001;
    message.gid = -1;
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
    message.gid = 20000001;
    message.procName.assign(MAX_PROC_NAME_LEN, 'a');
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
    message.procName.clear();
    EXPECT_FALSE(client_.VerifyProcessStartMsg(message));
}
} // namespace OHOS::AppExecFwk
