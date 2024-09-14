/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "app_spawn_client.h"
#include "appspawn_client.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppSpawnClientSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppSpawnClientSecondTest::SetUpTestCase(void)
{}

void AppSpawnClientSecondTest::TearDownTestCase(void)
{}

void AppSpawnClientSecondTest::SetUp()
{}

void AppSpawnClientSecondTest::TearDown()
{}

/**
 * @tc.name: SetStartFlags_001
 * @tc.desc: SetStartFlags.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetStartFlags_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.flags = 17;
    startMsg.atomicServiceFlag = true;
    startMsg.strictMode = true;
    startMsg.isolatedExtension = true;
    int ret = asc->SetStartFlags(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetStartFlags_002
 * @tc.desc: SetStartFlags.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetStartFlags_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.flags = 0;
    startMsg.atomicServiceFlag = false;
    startMsg.strictMode = false;
    startMsg.isolatedExtension = false;
    int ret = asc->SetStartFlags(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetStartFlags_003
 * @tc.desc: SetStartFlags.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetStartFlags_003, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgHandle reqHandle = nullptr;
    AppSpawnStartMsg startMsg;
    startMsg.flags = 17;
    int ret = asc->SetStartFlags(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppspawnSetExtMsgMore_001
 * @tc.desc: AppspawnSetExtMsgMore.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, AppspawnSetExtMsgMore_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.maxChildProcess = 2;
    startMsg.provisionType = "testprovisionType";
    startMsg.extensionSandboxPath = "testexternsionSandboxPath";
    startMsg.processType = "testProcessType";
    startMsg.fds["fds"] = 123;
    int ret = asc->AppspawnSetExtMsgMore(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppspawnSetExtMsg_001
 * @tc.desc: AppspawnSetExtMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, AppspawnSetExtMsg_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.renderParam = "test";
    int ret = asc->AppspawnSetExtMsg(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppspawnSetExtMsg_002
 * @tc.desc: AppspawnSetExtMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, AppspawnSetExtMsg_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.renderParam = "test";
    BaseSharedBundleInfo bsbi;
    startMsg.hspList.push_back(bsbi);
    DataGroupInfo dgi;
    startMsg.dataGroupInfoList.push_back(dgi);
    startMsg.overlayInfo = "testOverlayinfo";
    startMsg.appEnv["one"] = "testAppEnv";
    startMsg.atomicAccount = "testAtomicAccount";
    int ret = asc->AppspawnSetExtMsg(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppspawnCreateDefaultMsg_001
 * @tc.desc: AppspawnCreateDefaultMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, AppspawnCreateDefaultMsg_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = nullptr;
    AppSpawnStartMsg startMsg;
    int ret = asc->AppspawnCreateDefaultMsg(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: AppspawnCreateDefaultMsg_002
 * @tc.desc: AppspawnCreateDefaultMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, AppspawnCreateDefaultMsg_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    struct ListNode msgBlocks2;
    AppSpawnMsgBlock asmb;
    asmb.blockSize = 123;
    asmb.currentIndex = 1;
    asrmn.msgBlocks.next = &msgBlocks2;
    //asrmn.msgBlocks.prev = &asrmn.msgBlocks;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    printf("asrmn.msgBlocks:%d....\n",(int)&asrmn.msgBlocks);
    printf("msgBlocks2:%d.......\n", (int)&msgBlocks2);
    printf("******************************\n");

    AppSpawnMsgBlock *block = NULL;
    struct ListNode *node = asrmn.msgBlocks.next;
    printf("node:%d.......\n", (int)node);
    while (node != &asrmn.msgBlocks) {
        block = ListEntry(node, AppSpawnMsgBlock, node);
        printf("block.......\n");
        printf("size:%d..index:%d.....\n", block->blockSize, block->currentIndex);
        if ((block->blockSize - block->currentIndex) >= 10) {
            printf("1111\n");
        }
        node = node->next;
        printf("node:%d.......\n", (int)node);
         printf("2222\n");
    }
     printf("333\n");

    printf("******************************\n");

    startMsg.uid = 1;
    startMsg.gid = 1;
    startMsg.gids.push_back(1);
    DataGroupInfo dgi;
    startMsg.dataGroupInfoList.push_back(dgi);
    int ret = asc->AppspawnCreateDefaultMsg(startMsg, reqHandle);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: VerifyMsg_001
 * @tc.desc: VerifyMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, VerifyMsg_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnStartMsg startMsg;
    startMsg.code = MSG_APP_SPAWN;
    startMsg.uid = -1;
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
    
    startMsg.uid = 0;
    for (int i = 0; i <= 66; i++) {
        startMsg.gids.push_back(i);
    }
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
    
    startMsg.uid = 0;
    startMsg.gids.push_back(-1);
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
    
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "";
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
    
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "testProcName";
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
}

/**
 * @tc.name: VerifyMsg_002
 * @tc.desc: VerifyMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, VerifyMsg_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnStartMsg startMsg;
    startMsg.code = MSG_GET_RENDER_TERMINATION_STATUS;
    startMsg.pid = -1;
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
    
    startMsg.pid = 1;
    EXPECT_TRUE(asc->VerifyMsg(startMsg));
}

/**
 * @tc.name: VerifyMsg_003
 * @tc.desc: VerifyMsg.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, VerifyMsg_003, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnStartMsg startMsg;
    startMsg.code = MSG_BEGET_SPAWNTIME;
    EXPECT_FALSE(asc->VerifyMsg(startMsg));
}

/**
 * @tc.name: StartProcess_001
 * @tc.desc: StartProcess.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, StartProcess_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnStartMsg startMsg;
    pid_t pid;
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "";
    int ret = asc->StartProcess(startMsg, pid);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "testProcName";
    ret = asc->GetRenderProcessTerminationStatus(startMsg, pid);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetIsolationModeFlag_001
 * @tc.desc: SetIsolationModeFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetIsolationModeFlag_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgHandle reqHandle = nullptr;
    AppSpawnStartMsg startMsg;
    startMsg.isolationMode = false;
    int ret = asc->SetIsolationModeFlag(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetIsolationModeFlag_002
 * @tc.desc: SetIsolationModeFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetIsolationModeFlag_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    AppSpawnStartMsg startMsg;
    startMsg.isolationMode = true;
    int ret = asc->SetIsolationModeFlag(startMsg, reqHandle);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetExtMsgFds_001
 * @tc.desc: SetExtMsgFds.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetExtMsgFds_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    std::map<std::string, int32_t> fds;
    fds["one"] = 1;
    int ret = asc->SetExtMsgFds(reqHandle, fds);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: SetChildProcessTypeStartFlag_001
 * @tc.desc: SetChildProcessTypeStartFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetChildProcessTypeStartFlag_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    int32_t childProcessType = CHILD_PROCESS_TYPE_NOT_CHILD;
    int ret = asc->SetChildProcessTypeStartFlag(reqHandle, childProcessType);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: SetChildProcessTypeStartFlag_002
 * @tc.desc: SetChildProcessTypeStartFlag.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, SetChildProcessTypeStartFlag_002, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnReqMsgNode asrmn;
    AppSpawnMsg msg;
    msg.msgLen = 2;
    msg.msgType = MSG_APP_SPAWN;
    asrmn.msg = &msg;
    asrmn.msgBlocks.next = &asrmn.msgBlocks;
    AppSpawnMsgFlags asmf;
    asmf.flags[0] = 1;
    asmf.count = 1;
    asrmn.msgFlags = &asmf;
    AppSpawnReqMsgHandle reqHandle = (AppSpawnReqMsgHandle)&asrmn;
    int32_t childProcessType = CHILD_PROCESS_TYPE_JS;
    int ret = asc->SetChildProcessTypeStartFlag(reqHandle, childProcessType);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GetRenderProcessTerminationStatus_001
 * @tc.desc: GetRenderProcessTerminationStatus.
 * @tc.type: FUNC
 */
HWTEST_F(AppSpawnClientSecondTest, GetRenderProcessTerminationStatus_001, TestSize.Level0)
{
    auto asc = std::make_shared<AppSpawnClient>(true);
    AppSpawnStartMsg startMsg;
    int status;
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "";
    int ret = asc->GetRenderProcessTerminationStatus(startMsg, status);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    
    startMsg.uid = 0;
    startMsg.gids.push_back(1);
    startMsg.procName = "testProcName";
    ret = asc->GetRenderProcessTerminationStatus(startMsg, status);
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS
