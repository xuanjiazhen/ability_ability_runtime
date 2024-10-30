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

#include "hilog_tag_wrapper.h"
#include "native_child_callback.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class NativeChildProcessTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NativeChildProcessTest::SetUpTestCase() {}

void NativeChildProcessTest::TearDownTestCase() {}

void NativeChildProcessTest::SetUp() {}

void NativeChildProcessTest::TearDown() {}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_001
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_001, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_002
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_002, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    char temp = '\0';
    entry = &temp;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_003
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_003, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    char temp = '1';
    entry = &temp;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_004
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_004, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    char temp = ':';
    entry = &temp;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_005
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_005, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    int32_t tempid = 1;
    pid = &tempid;
    NativeChildProcess_Fd head;
    head.fdName = nullptr;
    args.fdList.head = &head;
    char temp = ':';
    entry = &temp;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_006
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_006, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;
    

    int32_t tempid = 1;
    pid = &tempid;
    NativeChildProcess_Fd head;
    char temp2[50] = {"123456789012345678901234567890"};
    head.fdName = temp2;
    args.fdList.head = &head;
    char temp = ':';
    entry = &temp;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_007
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_007, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;
    
    int32_t tempid = 1;
    pid = &tempid;
    NativeChildProcess_Fd head;
    char fdName = ':';
    entry = &fdName;
    head.fdName = &fdName;
    NativeChildProcess_Fd *temphead = &head;
    NativeChildProcess_Fd head2[20];
    char key[20][10];
    for (int i = 0; i < 20; i++) {
        key[i][0] = i;
        key[i][1] = 0;
        head2[i].fdName =  key[i];
        head2[i].fd = i;
        temphead->next = &head2[i];
        temphead = &head2[i];
    }
    args.fdList.head = &head;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_008
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_008, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;
    
    int32_t tempid = 1;
    pid = &tempid;
    NativeChildProcess_Fd head;
    char fdName = ':';
    entry = &fdName;
    head.fdName = &fdName;
    head.next = nullptr;
    args.fdList.head = &head;
    args.entryParams = &fdName;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_009
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_009, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    int32_t tempid = 1;
    pid = &tempid;
    NativeChildProcess_Fd head;
    char fdName = ':';
    entry = &fdName;
    head.fdName = &fdName;
    head.next = nullptr;
    args.fdList.head = &head;
    args.entryParams = &fdName;
    char temp2 = '\0';
    args.entryParams = &temp2;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}

/**
 * @tc.name: OH_Ability_StartNativeChildProcess_0010
 * @tc.desc: Test OH_Ability_StartNativeChildProcess.
 * @tc.type: FUNC
 */
HWTEST_F(NativeChildProcessTest, OH_Ability_StartNativeChildProcess_010, TestSize.Level1)
{
    char* entry = nullptr;
    NativeChildProcess_Args args;
    NativeChildProcess_Options options;
    int32_t *pid = nullptr;

    int32_t tempid = 1;
    pid = &tempid;
    char temp = ':';
    entry = &temp;
    args.entryParams = nullptr;
    Ability_NativeChildProcess_ErrCode err = OH_Ability_StartNativeChildProcess(entry, args, options, pid);
    EXPECT_NE(err, 0);
}
}  // namespace AbilityRuntime
}  // namespace OHOS