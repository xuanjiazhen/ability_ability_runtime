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
#include <cstdarg>
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
int32_t openResult = -1;
int32_t openError = ESRCH;
int32_t epollResult = 0;
int32_t closeCalls = 0;
int32_t lastClosedFd = -1;
ffrt_poller_cb registeredCallback = nullptr;
void *registeredData = nullptr;
}
extern "C" long __wrap_syscall(long, ...)
{
    errno = openError;
    return openResult;
}
extern "C" int __wrap_ffrt_epoll_ctl(int, int op, int, uint32_t, void *data, ffrt_poller_cb callback)
{
    if (op == EPOLL_CTL_ADD) {
        registeredCallback = callback;
        registeredData = data;
        return epollResult;
    }
    return 0;
}
extern "C" void __wrap_fdsan_exchange_owner_tag(int, uint64_t, uint64_t) {}
extern "C" int __wrap_fdsan_close_with_tag(int fd, uint64_t)
{
    ++closeCalls;
    lastClosedFd = fd;
    return 0;
}
namespace OHOS::AppExecFwk {
class LaunchServicePidfdTest : public testing::Test {
public:
    void SetUp() override
    {
        openResult = -1;
        openError = ESRCH;
        epollResult = 0;
        closeCalls = 0;
        lastClosedFd = -1;
        registeredCallback = nullptr;
        registeredData = nullptr;
        manager_.Init({}, {});
    }
    void TearDown() override
    {
        manager_.pidfdMap_.clear();
        openResult = -1;
        openError = ESRCH;
    }
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
    openError = EMFILE;
    EXPECT_FALSE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    openError = ESRCH;
    EXPECT_TRUE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
    EXPECT_EQ(closeCalls, 0);
}

/**
 * @tc.name: AddWatcher_EpollFailure_001
 * @tc.desc: Failed epoll registration removes the entry and closes its fd once.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_EpollFailure_001, TestSize.Level1)
{
    openResult = 42;
    epollResult = -1;
    EXPECT_FALSE(manager_.AddWatcher(123, PidFdType::LAUNCH_SERVICE));
    EXPECT_TRUE(manager_.pidfdMap_.empty());
    EXPECT_EQ(closeCalls, 1);
    EXPECT_EQ(lastClosedFd, 42);
}

/**
 * @tc.name: AddWatcher_Dispatch_001
 * @tc.desc: Each process type registers its own callback and closes its fd on exit.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_Dispatch_001, TestSize.Level1)
{
    for (auto type : {PidFdType::CHILD, PidFdType::RENDER, PidFdType::LAUNCH_SERVICE}) {
        openResult = 42;
        closeCalls = 0;
        ASSERT_TRUE(manager_.AddWatcher(123, type));
        ASSERT_NE(registeredCallback, nullptr);
        ASSERT_NE(registeredData, nullptr);
        if (type == PidFdType::LAUNCH_SERVICE) {
            EXPECT_EQ(registeredCallback, &AppPidFdManager::OnLaunchServicePidfdEvent);
        } else if (type == PidFdType::CHILD) {
            EXPECT_EQ(registeredCallback, &AppPidFdManager::OnChildPidfdEvent);
        } else {
            EXPECT_EQ(registeredCallback, &AppPidFdManager::OnRenderPidfdEvent);
        }
        registeredCallback(registeredData, EPOLLIN);
        EXPECT_TRUE(manager_.pidfdMap_.empty());
        EXPECT_EQ(closeCalls, 1);
        manager_.OnPidfdFired(123, type);
        EXPECT_EQ(closeCalls, 1);
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
    EXPECT_EQ(closeCalls, 1);
    EXPECT_EQ(lastClosedFd, 42);
    manager_.OnPidfdFired(123, PidFdType::LAUNCH_SERVICE);
    EXPECT_EQ(closeCalls, 2);
    EXPECT_EQ(lastClosedFd, 43);
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
    EXPECT_EQ(closeCalls, 0);
    manager_.Init({}, {});
}
} // namespace OHOS::AppExecFwk
