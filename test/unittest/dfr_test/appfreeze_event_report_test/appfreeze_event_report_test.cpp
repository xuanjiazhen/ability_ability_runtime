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

#include "appfreeze_event_report.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AppExecFwk {
class AppfreezeEventReportTest : public testing::Test {
public:
    AppfreezeEventReportTest()
    {}
    ~AppfreezeEventReportTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AppfreezeEventReportTest::SetUpTestCase(void)
{}

void AppfreezeEventReportTest::TearDownTestCase(void)
{}

void AppfreezeEventReportTest::SetUp(void)
{}

void AppfreezeEventReportTest::TearDown(void)
{}

/**
 * @tc.number: SendAppfreezeEvent_Test_001
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_001, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_001";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_001: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_001: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_001: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_002
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_002, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_002";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_002: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_002: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_002: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    eventInfo.applicationHeapInfo = "size:1,";
    eventInfo.processLifeTime = "2s";
    eventInfo.mainStack = "test";
    eventInfo.enableFreeze = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_003
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_003, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_HALF_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_003";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_003: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_003: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_003: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_004
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_004, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_HALF_TIMEOUT_WARNING";
    std::string testName = "SendAppfreezeEvent_Test_004";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_004: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_004: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_004: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_005
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_005, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_005";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_005: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_005: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_005: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    eventInfo.enableFreeze = false;
    eventInfo.applicationHeapInfo = "size1:1234,size2:1234";
    eventInfo.processLifeTime = "21s";
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_006
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_006, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT_WARNING";
    std::string testName = "SendAppfreezeEvent_Test_006";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_006: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_006: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_006: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_007
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_007, TestSize.Level1)
{
    std::string eventName = "APP_LIFECYCLE_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_007";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_007: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_007: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_007: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_008
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_008, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    std::string testName = "SendAppfreezeEvent_Test_008";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_008: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_008: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_008: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = "size1:1234,size2:1234";
    eventInfo.processLifeTime = "21s";
    eventInfo.dispatchedEventId = 12;
    eventInfo.processedId = 11;
    eventInfo.markedId = 10;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_009
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_009, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_009";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_009: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_009: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_009: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_010
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_010, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_010";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = "PeerBinder: test binder info \\n";
    eventInfo.freezeMemory = "memory info test";
    eventInfo.appRunningUniqueId = "id: 1234";
    eventInfo.errorStack = "SendAppfreezeEvent_Test_010: error stack";
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = "SendAppfreezeEvent_Test_010: error message";
    eventInfo.freezeInfoFile = "SendAppfreezeEvent_Test_010: freezeInfoFile";
    eventInfo.hitraceInfo = "hitraceInfo: 1234";
    eventInfo.foregroundState = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_011
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_011, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_012
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_012, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    int testValue = 120; // test value
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = testValue;
    eventInfo.pid = testValue;
    eventInfo.uid = testValue;
    eventInfo.eventId = testValue;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_013
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_013, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_013";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 120; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_014
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_014, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "SendAppfreezeEvent_Test_014";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 120; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    testName = "SendAppfreezeEvent_Test_014;SendAppfreezeEvent_Test_014";
    eventInfo.processName = testName;
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_015
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_015, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_3S";
    std::string testName = "";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 120; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_016
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_016, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_017s
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_017, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    int testValue = 120; // test value
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = testValue;
    eventInfo.pid = testValue;
    eventInfo.uid = testValue;
    eventInfo.eventId = testValue;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_018
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_018, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_018";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 69; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_019
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_019, TestSize.Level1)
{
    std::string eventName = "THREAD_BLOCK_6S";
    std::string testName = "";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = testName;
    eventInfo.processLifeTime = testName;
    eventInfo.mainStack = testName;
    eventInfo.enableFreeze = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_020
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_020, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_021
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_021, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    int testValue = 120; // test value
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = testValue;
    eventInfo.pid = testValue;
    eventInfo.uid = testValue;
    eventInfo.eventId = testValue;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_022
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_022, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    std::string testName = "SendAppfreezeEvent_Test_022";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 69; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_023
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_023, TestSize.Level1)
{
    std::string eventName = "LIFECYCLE_TIMEOUT";
    std::string testName = "";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = testName;
    eventInfo.processLifeTime = testName;
    eventInfo.enableFreeze = false;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_024
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_024, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_025
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_025, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    int testValue = 120; // test value
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = testValue;
    eventInfo.pid = testValue;
    eventInfo.uid = testValue;
    eventInfo.eventId = testValue;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_026
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_026, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    std::string testName = "SendAppfreezeEvent_Test_026";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 69; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_027
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_027, TestSize.Level1)
{
    std::string eventName = "APP_INPUT_BLOCK";
    std::string testName = "";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = -1; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo = testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = testName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    eventInfo.applicationHeapInfo = testName;
    eventInfo.processLifeTime = testName;
    eventInfo.dispatchedEventId = 0;
    eventInfo.processedId = 0;
    eventInfo.markedId = 0;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_028
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_028, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_025
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_029, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    int testValue = 120; // test value
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = testValue;
    eventInfo.pid = testValue;
    eventInfo.uid = testValue;
    eventInfo.eventId = testValue;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_030
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_030, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    std::string testName = "SendAppfreezeEvent_Test_030";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    int testValue = 69; // test value
    eventInfo.eventId = testValue;
    eventInfo.bundleName = testName;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.number: SendAppfreezeEvent_Test_031
 * @tc.desc: add testcase
 * @tc.type: FUNC
 */
HWTEST_F(AppfreezeEventReportTest, SendAppfreezeEvent_Test_031, TestSize.Level1)
{
    std::string eventName = "BUSSINESS_THREAD_BLOCK_6S";
    std::string testName = "";
    int tid = static_cast<int>(gettid());
    int pid = static_cast<int>(getpid());
    int uid = static_cast<int>(getuid());
    AppfreezeEventInfo eventInfo;
    eventInfo.tid = tid;
    eventInfo.pid = pid;
    eventInfo.uid = uid;
    eventInfo.eventId = 380; // test value
    eventInfo.bundleName = testName;
    eventInfo.processName = testName;
    eventInfo.binderInfo= testName;
    eventInfo.freezeMemory = testName;
    eventInfo.appRunningUniqueId = testName;
    eventInfo.errorStack = testName;
    eventInfo.errorName = eventName;
    eventInfo.errorMessage = testName;
    eventInfo.freezeInfoFile = testName;
    eventInfo.hitraceInfo = testName;
    eventInfo.foregroundState = true;
    int ret = AppfreezeEventReport::SendAppfreezeEvent(eventName,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT, eventInfo);
    EXPECT_EQ(ret, 0);
}
}  // namespace AppExecFwk
}  // namespace OHOS
