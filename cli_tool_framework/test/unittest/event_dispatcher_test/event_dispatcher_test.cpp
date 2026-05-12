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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "event_dispatcher.h"
#undef private

#include "cli_tool_event.h"
#include "cli_session_info.h"
#include "i_cli_tool_manager_scheduler.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_remote_object") {}

    int32_t GetObjectRefCount() override
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override
    {
        return ERR_OK;
    }

    bool IsProxyObject() const override
    {
        return true;
    }

    bool CheckObjectLegality() const override
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient) override
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient>& recipient) override
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient>& recipient) override
    {
        return true;
    }

    bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface() override
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args) override
    {
        return 0;
    }

    std::u16string GetObjectDescriptor() const
    {
        return std::u16string();
    }
};

class MockCliToolManagerScheduler : public ICliToolManagerScheduler {
public:
    MockCliToolManagerScheduler() : remoteObject_(new MockRemoteObject()) {}

    MOCK_METHOD3(SchedulerSessionEvent, int32_t(const std::string&, const std::string&, const CliToolEvent&));
    MOCK_METHOD2(SchedulerInputReplyEvent, int32_t(const std::string&, int32_t));
    MOCK_METHOD3(SchedulerExecToolReplyEvent, int32_t(const std::string&, int32_t, const CliSessionInfo&));

    sptr<IRemoteObject> AsObject() override
    {
        return remoteObject_;
    }

private:
    sptr<MockRemoteObject> remoteObject_;
};

class EventDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void EventDispatcherTest::SetUpTestCase(void)
{
}

void EventDispatcherTest::TearDownTestCase(void)
{
}

void EventDispatcherTest::SetUp()
{
    EventDispatcher::GetInstance().ClearAll();
}

void EventDispatcherTest::TearDown()
{
    EventDispatcher::GetInstance().ClearAll();
}

HWTEST_F(EventDispatcherTest, GetInstance_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_GetInstance_0100 start - Verify singleton pattern returns same instance";

    auto& instance1 = EventDispatcher::GetInstance();
    auto& instance2 = EventDispatcher::GetInstance();

    EXPECT_EQ(&instance1, &instance2);

    GTEST_LOG_(INFO) << "EventDispatcher_GetInstance_0100 end";
}

HWTEST_F(EventDispatcherTest, GetInstance_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_GetInstance_0200 start - Verify multiple calls return identical references";

    EventDispatcher& dispatcher1 = EventDispatcher::GetInstance();
    EventDispatcher& dispatcher2 = EventDispatcher::GetInstance();
    EventDispatcher& dispatcher3 = EventDispatcher::GetInstance();

    EXPECT_EQ(&dispatcher1, &dispatcher2);
    EXPECT_EQ(&dispatcher2, &dispatcher3);

    GTEST_LOG_(INFO) << "EventDispatcher_GetInstance_0200 end";
}

HWTEST_F(EventDispatcherTest, RegisterScheduler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_0100 start - Verify invalid pid/nullptr rejected";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.RegisterScheduler(-1, nullptr));
    EXPECT_FALSE(dispatcher.RegisterScheduler(0, nullptr));

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_0100 end";
}

HWTEST_F(EventDispatcherTest, RegisterScheduler_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_0200 start - Verify nullptr scheduler rejected for valid pid";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.RegisterScheduler(1001, nullptr));

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_0200 end";
}

HWTEST_F(EventDispatcherTest, UnregisterScheduler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterScheduler_0100 start - Verify unregister nonexistent scheduler is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.UnregisterScheduler(9999);

    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterScheduler_0100 end";
}

HWTEST_F(EventDispatcherTest, RegisterSubscriber_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_0100 start - Verify invalid parameters rejected";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.RegisterSubscriber("", "sub1", 1001));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session1", "", 1001));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session1", "sub1", -1));
    EXPECT_FALSE(dispatcher.RegisterSubscriber("session1", "sub1", 0));

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_0100 end";
}

HWTEST_F(EventDispatcherTest, RegisterSubscriber_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_0200 start - Verify register fails without scheduler";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.RegisterSubscriber("session1", "sub1", 1001));

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_0200 end";
}

HWTEST_F(EventDispatcherTest, UnregisterSubscriber_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterSubscriber_0100 start - Verify invalid parameters rejected";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.UnregisterSubscriber("", "sub1", 1001));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session1", "", 1001));
    EXPECT_FALSE(dispatcher.UnregisterSubscriber("session1", "sub1", -1));

    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterSubscriber_0100 end";
}

HWTEST_F(EventDispatcherTest, UnregisterSubscriber_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterSubscriber_0200 start - Verify unregister nonexistent subscriber succeeds";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_TRUE(dispatcher.UnregisterSubscriber("nonexistent_session", "sub1", 1001));

    GTEST_LOG_(INFO) << "EventDispatcher_UnregisterSubscriber_0200 end";
}

HWTEST_F(EventDispatcherTest, DispatchIOEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_0100 start - Verify dispatch without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.DispatchIOEvent("test_session", "stdout", "test_data");

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchIOEvent_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_0200 start - Verify stderr dispatch without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.DispatchIOEvent("nonexistent_session", "stderr", "error_data");

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_0200 end";
}

HWTEST_F(EventDispatcherTest, DispatchErrorEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchErrorEvent_0100 start - Verify error event dispatch without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.DispatchErrorEvent("test_session", "test_error_message");

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchErrorEvent_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchExitEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExitEvent_0100 start - Verify exit code 0 dispatch without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.DispatchExitEvent("test_session", 0);

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExitEvent_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchExitEvent_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExitEvent_0200 start - Verify nonzero exit code dispatch without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.DispatchExitEvent("test_session", 1);

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExitEvent_0200 end";
}

HWTEST_F(EventDispatcherTest, DispatchInputReplyEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_0100 start - Verify invalid pid rejected";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(-1, "event1", 0));
    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(0, "event1", 0));

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchInputReplyEvent_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_0200 start - Verify dispatch fails without scheduler";

    auto& dispatcher = EventDispatcher::GetInstance();

    EXPECT_FALSE(dispatcher.DispatchInputReplyEvent(1001, "event1", 0));

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_0200 end";
}

HWTEST_F(EventDispatcherTest, DispatchExecToolReplyEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExecToolReplyEvent_0100 start - Verify invalid pid rejected";

    auto& dispatcher = EventDispatcher::GetInstance();

    CliSessionInfo session;
    session.sessionId = "test_session";
    session.toolName = "test_tool";
    session.status = "completed";

    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(-1, "event1", 0, session));
    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(0, "event1", 0, session));

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExecToolReplyEvent_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchExecToolReplyEvent_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExecToolReplyEvent_0200 start - Verify dispatch fails without scheduler";

    auto& dispatcher = EventDispatcher::GetInstance();

    CliSessionInfo session;
    session.sessionId = "test_session";
    session.toolName = "test_tool";
    session.status = "completed";

    EXPECT_FALSE(dispatcher.DispatchExecToolReplyEvent(1001, "event1", 0, session));

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchExecToolReplyEvent_0200 end";
}

HWTEST_F(EventDispatcherTest, ClearSessionSubscribers_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_ClearSessionSubscribers_0100 start - Verify clearing nonexistent session is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.ClearSessionSubscribers("test_session");

    GTEST_LOG_(INFO) << "EventDispatcher_ClearSessionSubscribers_0100 end";
}

HWTEST_F(EventDispatcherTest, ClearSessionSubscribers_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_ClearSessionSubscribers_0200 start - Verify clearing empty/nonexistent session is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.ClearSessionSubscribers("");
    dispatcher.ClearSessionSubscribers("nonexistent_session");

    GTEST_LOG_(INFO) << "EventDispatcher_ClearSessionSubscribers_0200 end";
}

HWTEST_F(EventDispatcherTest, ClearAll_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_ClearAll_0100 start";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.ClearAll();

    EXPECT_TRUE(dispatcher.schedulers_.empty());
    EXPECT_TRUE(dispatcher.sessionSubscribers_.empty());

    GTEST_LOG_(INFO) << "EventDispatcher_ClearAll_0100 end";
}

HWTEST_F(EventDispatcherTest, RegisterScheduler_WithMock_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_WithMock_0100 start - Test scheduler registration and replacement";

    auto& dispatcher = EventDispatcher::GetInstance();
    MockCliToolManagerScheduler mockScheduler1;
    MockCliToolManagerScheduler mockScheduler2;

    EXPECT_TRUE(dispatcher.RegisterScheduler(1001, &mockScheduler1));
    EXPECT_TRUE(dispatcher.RegisterScheduler(1001, &mockScheduler2));

    dispatcher.UnregisterScheduler(1001);

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterScheduler_WithMock_0100 end";
}

HWTEST_F(EventDispatcherTest, RegisterSubscriber_Success_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_Success_0100 start - Test successful subscriber registration";

    auto& dispatcher = EventDispatcher::GetInstance();
    MockCliToolManagerScheduler mockScheduler;

    ASSERT_TRUE(dispatcher.RegisterScheduler(2001, &mockScheduler));
    EXPECT_TRUE(dispatcher.RegisterSubscriber("session_success", "sub_success", 2001));

    dispatcher.UnregisterSubscriber("session_success", "sub_success", 2001);
    dispatcher.UnregisterScheduler(2001);

    GTEST_LOG_(INFO) << "EventDispatcher_RegisterSubscriber_Success_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchIOEvent_WithMock_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_WithMock_0100 start - Test IO event dispatch with mock verification";

    auto& dispatcher = EventDispatcher::GetInstance();
    MockCliToolManagerScheduler mockScheduler;

    ASSERT_TRUE(dispatcher.RegisterScheduler(3001, &mockScheduler));
    ASSERT_TRUE(dispatcher.RegisterSubscriber("io_session", "io_sub", 3001));

    using ::testing::_;
    using ::testing::Return;
    EXPECT_CALL(mockScheduler, SchedulerSessionEvent("io_session", "io_sub", _))
        .Times(1)
        .WillOnce(Return(0));

    dispatcher.DispatchIOEvent("io_session", "stdout", "test_io_data");

    dispatcher.UnregisterSubscriber("io_session", "io_sub", 3001);
    dispatcher.UnregisterScheduler(3001);

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchIOEvent_WithMock_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchInputReplyEvent_Success_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_Success_0100 start - Test successful input reply dispatch";

    auto& dispatcher = EventDispatcher::GetInstance();
    MockCliToolManagerScheduler mockScheduler;

    ASSERT_TRUE(dispatcher.RegisterScheduler(4001, &mockScheduler));

    using ::testing::_;
    using ::testing::Return;
    EXPECT_CALL(mockScheduler, SchedulerInputReplyEvent("event_reply", 0))
        .Times(1)
        .WillOnce(Return(0));

    EXPECT_TRUE(dispatcher.DispatchInputReplyEvent(4001, "event_reply", 0));

    dispatcher.UnregisterScheduler(4001);

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchInputReplyEvent_Success_0100 end";
}

HWTEST_F(EventDispatcherTest, ClearAll_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_ClearAll_0200 start - Verify repeated clear is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    dispatcher.ClearAll();
    dispatcher.ClearAll();

    EXPECT_TRUE(dispatcher.schedulers_.empty());
    EXPECT_TRUE(dispatcher.sessionSubscribers_.empty());

    GTEST_LOG_(INFO) << "EventDispatcher_ClearAll_0200 end";
}

HWTEST_F(EventDispatcherTest, SubscriberKey_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_SubscriberKey_0100 start - Verify SubscriberKey equality operator";

    EventDispatcher::SubscriberKey key1;
    key1.callerPid = 1001;
    key1.subscriptionId = "sub1";

    EventDispatcher::SubscriberKey key2;
    key2.callerPid = 1001;
    key2.subscriptionId = "sub1";

    EXPECT_TRUE(key1 == key2);

    EventDispatcher::SubscriberKey key3;
    key3.callerPid = 1002;
    key3.subscriptionId = "sub1";

    EXPECT_FALSE(key1 == key3);

    EventDispatcher::SubscriberKey key4;
    key4.callerPid = 1001;
    key4.subscriptionId = "sub2";

    EXPECT_FALSE(key1 == key4);

    GTEST_LOG_(INFO) << "EventDispatcher_SubscriberKey_0100 end";
}

HWTEST_F(EventDispatcherTest, SubscriberKeyHash_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_SubscriberKeyHash_0100 start - Verify hash function works correctly";

    EventDispatcher::SubscriberKeyHash hasher;

    EventDispatcher::SubscriberKey key1;
    key1.callerPid = 1001;
    key1.subscriptionId = "sub1";

    size_t hash1 = hasher(key1);

    EventDispatcher::SubscriberKey key2;
    key2.callerPid = 1002;
    key2.subscriptionId = "sub2";

    size_t hash2 = hasher(key2);

    EventDispatcher::SubscriberKey key3;
    key3.callerPid = 1001;
    key3.subscriptionId = "sub1";

    size_t hash3 = hasher(key3);

    EXPECT_EQ(hash1, hash3);
    EXPECT_NE(hash1, hash2);

    GTEST_LOG_(INFO) << "EventDispatcher_SubscriberKeyHash_0100 end";
}

HWTEST_F(EventDispatcherTest, RemoveSubscribersForPidLocked_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_RemoveSubscribersForPidLocked_0100 start - Verify removing nonexistent pid subscribers is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    std::lock_guard<std::mutex> lock(dispatcher.mutex_);
    dispatcher.RemoveSubscribersForPidLocked(1001);

    GTEST_LOG_(INFO) << "EventDispatcher_RemoveSubscribersForPidLocked_0100 end";
}

HWTEST_F(EventDispatcherTest, DispatchEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EventDispatcher_DispatchEvent_0100 start - Verify dispatch event without subscriber is safe";

    auto& dispatcher = EventDispatcher::GetInstance();

    CliToolEvent event;
    event.type = "stdout";
    event.eventData = "test_data";
    event.exitCode = 0;
    event.timestamp = 0;

    dispatcher.DispatchEvent("nonexistent_session", event);

    GTEST_LOG_(INFO) << "EventDispatcher_DispatchEvent_0100 end";
}

} // namespace CliTool
} // namespace OHOS