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

#include <chrono>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include "agent_pre_install_event_receiver.h"
#include "mock_my_flag.h"
#include "common_event_data.h"
#include "common_event_subscribe_info.h"
#include "common_event_support.h"
#include "matching_skills.h"
#include "want.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;
using namespace std::chrono;

namespace OHOS {
namespace AgentRuntime {
namespace {
// Must match the MAIN_USER_ID the receiver passes to HandlePreInstallBackfill.
constexpr int32_t EXPECTED_PRE_INSTALL_USER_ID = 100;

void ResetFlags()
{
    MyFlag::preInstallBackfillCalled.store(false);
    MyFlag::preInstallBackfillUserId.store(-1);
    MyFlag::preInstallBackfillCallCount.store(0);
}

// The receiver dispatches via ffrt::submit, so the mock call happens on a worker thread.
// Poll until HandlePreInstallBackfill has been invoked at least |expected| times, or time out.
bool WaitForCallCount(int32_t expected, milliseconds timeout)
{
    auto deadline = steady_clock::now() + timeout;
    while (steady_clock::now() < deadline) {
        if (MyFlag::preInstallBackfillCallCount.load() >= expected) {
            return true;
        }
        std::this_thread::sleep_for(milliseconds(10));
    }
    return MyFlag::preInstallBackfillCallCount.load() >= expected;
}

std::shared_ptr<AgentPreInstallEventReceiver> MakeReceiver()
{
    EventFwk::MatchingSkills skills;
    skills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(skills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    return std::make_shared<AgentPreInstallEventReceiver>(subscribeInfo);
}

void FillEventData(const std::string &action, EventFwk::CommonEventData &data)
{
    AAFwk::Want want;
    want.SetAction(action);
    data.SetWant(want);
}
}  // namespace

class AgentPreInstallEventReceiverTest : public testing::Test {
public:
    void SetUp() override { ResetFlags(); }
    void TearDown() override {}
};

/**
 * @tc.name: OnReceiveEvent_001
 * @tc.desc: COMMON_EVENT_BUNDLE_SCAN_FINISHED triggers HandlePreInstallBackfill(MAIN_USER_ID)
 * @tc.type: FUNC
 */
HWTEST_F(AgentPreInstallEventReceiverTest, OnReceiveEvent_001, TestSize.Level1)
{
    auto receiver = MakeReceiver();
    EventFwk::CommonEventData data;
    FillEventData(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED, data);

    receiver->OnReceiveEvent(data);

    EXPECT_TRUE(WaitForCallCount(1, seconds(5)));
    EXPECT_EQ(MyFlag::preInstallBackfillUserId.load(), EXPECTED_PRE_INSTALL_USER_ID);
    EXPECT_EQ(MyFlag::preInstallBackfillCallCount.load(), 1);
}

/**
 * @tc.name: OnReceiveEvent_002
 * @tc.desc: A non-matching action (PACKAGE_ADDED) must not trigger the pre-install backfill
 * @tc.type: FUNC
 */
HWTEST_F(AgentPreInstallEventReceiverTest, OnReceiveEvent_002, TestSize.Level1)
{
    auto receiver = MakeReceiver();
    EventFwk::CommonEventData data;
    FillEventData(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED, data);

    receiver->OnReceiveEvent(data);

    // Give any stray worker a grace window; no ffrt task is submitted for a non-matching action.
    std::this_thread::sleep_for(milliseconds(200));
    EXPECT_EQ(MyFlag::preInstallBackfillCallCount.load(), 0);
    EXPECT_FALSE(MyFlag::preInstallBackfillCalled.load());
}

/**
 * @tc.name: OnReceiveEvent_003
 * @tc.desc: An empty action must not trigger the pre-install backfill
 * @tc.type: FUNC
 */
HWTEST_F(AgentPreInstallEventReceiverTest, OnReceiveEvent_003, TestSize.Level1)
{
    auto receiver = MakeReceiver();
    EventFwk::CommonEventData data;
    FillEventData("", data);

    receiver->OnReceiveEvent(data);

    std::this_thread::sleep_for(milliseconds(200));
    EXPECT_EQ(MyFlag::preInstallBackfillCallCount.load(), 0);
}

/**
 * @tc.name: OnReceiveEvent_004
 * @tc.desc: Repeated BUNDLE_SCAN_FINISHED events each dispatch one backfill
 * @tc.type: FUNC
 */
HWTEST_F(AgentPreInstallEventReceiverTest, OnReceiveEvent_004, TestSize.Level1)
{
    auto receiver = MakeReceiver();
    EventFwk::CommonEventData data;
    FillEventData(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED, data);

    receiver->OnReceiveEvent(data);
    EXPECT_TRUE(WaitForCallCount(1, seconds(5)));
    EXPECT_EQ(MyFlag::preInstallBackfillUserId.load(), EXPECTED_PRE_INSTALL_USER_ID);

    receiver->OnReceiveEvent(data);
    EXPECT_TRUE(WaitForCallCount(2, seconds(5)));
    EXPECT_EQ(MyFlag::preInstallBackfillUserId.load(), EXPECTED_PRE_INSTALL_USER_ID);
    EXPECT_GE(MyFlag::preInstallBackfillCallCount.load(), 2);
}
}  // namespace AgentRuntime
}  // namespace OHOS
