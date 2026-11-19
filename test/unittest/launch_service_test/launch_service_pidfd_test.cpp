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
#include <cerrno>
#include <map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include "ffrt.h"
#define private public
#include "app_pidfd_manager.h"
#undef private
#include "task_handler_wrap.h"
using namespace testing::ext;
namespace {
int32_t g_openResult = -1;
int32_t g_openError = ESRCH;
int32_t g_epollResult = 0;
int32_t g_closeCalls = 0;
int32_t g_lastClosedFd = -1;
ffrt_poller_cb g_registeredCallback = nullptr;
void *g_registeredData = nullptr;
int32_t MockOpenPidFd(pid_t, unsigned int)
{
    errno = g_openError;
    return g_openResult;
}
int32_t MockPoll(int32_t op, int32_t, uint32_t, void *data, ffrt_poller_cb callback)
{
    if (op == EPOLL_CTL_ADD) {
        g_registeredCallback = callback;
        g_registeredData = data;
        return g_epollResult;
    }
    return 0;
}
void MockExchangeOwnerTag(int32_t, uint64_t, uint64_t) {}
int32_t MockCloseWithTag(int32_t fd, uint64_t)
{
    ++g_closeCalls;
    g_lastClosedFd = fd;
    return 0;
}
} // namespace
namespace OHOS::AppExecFwk {
class LaunchServicePidfdTest : public testing::Test {
public:
    void SetUp() override
    {
        originalOperations_ = manager_.pidFdOperations_;
        manager_.pidFdOperations_ = {MockOpenPidFd, MockPoll, MockExchangeOwnerTag, MockCloseWithTag};
        g_openResult = -1;
        g_openError = ESRCH;
        g_epollResult = 0;
        g_closeCalls = 0;
        g_lastClosedFd = -1;
        g_registeredCallback = nullptr;
        g_registeredData = nullptr;
        manager_.Init({}, {});
    }
    void TearDown() override
    {
        manager_.pidfdMap_.clear();
        manager_.pidFdOperations_ = originalOperations_;
        g_openResult = -1;
        g_openError = ESRCH;
    }
    AppPidFdManager::PidFdOperations originalOperations_ {};
    AppPidFdManager &manager_ = AppPidFdManager::GetInstance();
};

/**
 * @tc.name: AddWatcher_OpenFailure_001
 * @tc.desc: ESRCH is treated as already exited; resource failures reject launch-service monitoring.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_OpenFailure_001, TestSize.Level1)
{
    EXPECT_FALSE(manager_.AddWatcher(0, PidFdType::LAUNCH_SERVICE));
    EXPECT_FALSE(manager_.AddWatcher(-1, PidFdType::LAUNCH_SERVICE));
    g_openError = EMFILE;
    EXPECT_FALSE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    g_openError = ESRCH;
    EXPECT_TRUE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
    EXPECT_EQ(g_closeCalls, 0);
}

/**
 * @tc.name: AddWatcher_EpollFailure_001
 * @tc.desc: Failed epoll registration removes the entry and closes its fd once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_EpollFailure_001, TestSize.Level1)
{
    g_openResult = 42;
    g_epollResult = -1;
    EXPECT_FALSE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
    EXPECT_EQ(g_closeCalls, 1);
    EXPECT_EQ(g_lastClosedFd, 42);
}

/**
 * @tc.name: AddWatcher_Dispatch_001
 * @tc.desc: Each process type registers its own callback and closes its fd on exit.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_Dispatch_001, TestSize.Level1)
{
    for (auto type : {PidFdType::CHILD, PidFdType::RENDER, PidFdType::LAUNCH_SERVICE}) {
        g_openResult = 42;
        g_closeCalls = 0;
        ASSERT_TRUE(manager_.AddWatcher(123, type));
        ASSERT_NE(g_registeredCallback, nullptr);
        ASSERT_NE(g_registeredData, nullptr);
        if (type == PidFdType::LAUNCH_SERVICE) {
            EXPECT_EQ(g_registeredCallback, &AppPidFdManager::OnLaunchServicePidfdEvent);
        } else if (type == PidFdType::CHILD) {
            EXPECT_EQ(g_registeredCallback, &AppPidFdManager::OnChildPidfdEvent);
        } else {
            EXPECT_EQ(g_registeredCallback, &AppPidFdManager::OnRenderPidfdEvent);
        }
        g_registeredCallback(g_registeredData, EPOLLIN);
        EXPECT_TRUE(manager_.pidfdMap_.empty());
        EXPECT_EQ(g_closeCalls, 1);
        manager_.OnPidfdFired(123, type);
        EXPECT_EQ(g_closeCalls, 1);
    }
}

/**
 * @tc.name: StorePidFd_001
 * @tc.desc: Duplicate pid replaces its stale fd while preserving the callback address.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, StorePidFd_001, TestSize.Level1)
{
    void *first = manager_.StorePidFd(123, 42);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(*static_cast<int32_t *>(first), 123);
    void *second = manager_.StorePidFd(123, 43);
    EXPECT_EQ(second, first);
    EXPECT_EQ(manager_.pidfdMap_.size(), 1);
    EXPECT_EQ(manager_.pidfdMap_.at(123).pidfd, 43);
    EXPECT_EQ(g_closeCalls, 1);
    EXPECT_EQ(g_lastClosedFd, 42);
    manager_.OnPidfdFired(123, PidFdType::LAUNCH_SERVICE);
    EXPECT_EQ(g_closeCalls, 2);
    EXPECT_EQ(g_lastClosedFd, 43);
}

/**
 * @tc.name: Init_OriginalHandler_001
 * @tc.desc: Pidfd retains only the original appmgr handler.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, Init_OriginalHandler_001, TestSize.Level1)
{
    auto original = AAFwk::TaskHandlerWrap::CreateQueueHandler("pidfd_original_test");
    manager_.Init({}, original);
    EXPECT_EQ(manager_.taskHandler_.lock(), original);
    manager_.OnLaunchServicePidfdEvent(nullptr, EPOLLIN);
    EXPECT_EQ(g_closeCalls, 0);
    manager_.Init({}, {});
}
} // namespace OHOS::AppExecFwk
