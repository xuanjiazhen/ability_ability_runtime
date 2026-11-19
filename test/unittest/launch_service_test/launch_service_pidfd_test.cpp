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
#include <csignal>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#define private public
#include "app_pidfd_manager.h"
#undef private
#include "task_handler_wrap.h"
#include "unique_fd.h"

using namespace testing::ext;
namespace OHOS::AppExecFwk {
namespace {
class ScopedTestProcess {
public:
    ScopedTestProcess() : pid_(fork())
    {
        if (pid_ == 0) {
            for (;;) {
                pause();
            }
        }
    }
    ~ScopedTestProcess()
    {
        if (pid_ > 0) {
            AppPidFdManager::GetInstance().OnPidfdFired(pid_, PidFdType::LAUNCH_SERVICE);
            kill(pid_, SIGKILL);
            while (waitpid(pid_, nullptr, 0) < 0 && errno == EINTR) {}
        }
    }
    pid_t GetPid() const { return pid_; }
private:
    pid_t pid_;
};
} // namespace
class LaunchServicePidfdTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_.Init({}, {});
    }
    void TearDown() override
    {
        while (!manager_.pidfdMap_.empty()) {
            manager_.OnPidfdFired(manager_.pidfdMap_.begin()->first, PidFdType::LAUNCH_SERVICE);
        }
        manager_.Init({}, {});
    }
    AppPidFdManager &manager_ = AppPidFdManager::GetInstance();
};

/**
 * @tc.name: AddWatcher_InvalidPid_001
 * @tc.desc: Invalid process IDs return no descriptor and create no watcher.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_InvalidPid_001, TestSize.Level1)
{
    EXPECT_EQ(manager_.AddWatcher(0, PidFdType::LAUNCH_SERVICE), -1);
    EXPECT_EQ(manager_.AddWatcher(-1, PidFdType::LAUNCH_SERVICE), -1);
    EXPECT_TRUE(manager_.pidfdMap_.empty());
}

/**
 * @tc.name: AddWatcher_Dispatch_001
 * @tc.desc: Watcher cleanup preserves the independently owned launch-service descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_Dispatch_001, TestSize.Level1)
{
    ScopedTestProcess process;
    const pid_t pid = process.GetPid();
    ASSERT_GT(pid, 0);
    for (auto type : {PidFdType::CHILD, PidFdType::RENDER, PidFdType::LAUNCH_SERVICE}) {
        const int32_t fd = manager_.AddWatcher(pid, type);
        ASSERT_GE(fd, 0);
        UniqueFd owned(type == PidFdType::LAUNCH_SERVICE ? fd : -1);
        const int32_t watcherFd = manager_.pidfdMap_.at(pid).pidfd;
        EXPECT_EQ(fd == watcherFd, type != PidFdType::LAUNCH_SERVICE);
        // The child stays alive, so no real exit callback races this cleanup.
        manager_.OnPidfdFired(pid, type);
        EXPECT_TRUE(manager_.pidfdMap_.empty());
        EXPECT_EQ(fcntl(watcherFd, F_GETFD), -1);
        EXPECT_EQ(errno, EBADF);
        if (type == PidFdType::LAUNCH_SERVICE) {
            EXPECT_NE(fcntl(owned.Get(), F_GETFD), -1);
        }
        manager_.OnPidfdFired(pid, type);
    }
}

/**
 * @tc.name: AddWatcher_Replace_001
 * @tc.desc: Re-registering a live process replaces the watcher without invalidating returned descriptors.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, AddWatcher_Replace_001, TestSize.Level1)
{
    ScopedTestProcess process;
    const pid_t pid = process.GetPid();
    ASSERT_GT(pid, 0);
    UniqueFd first(manager_.AddWatcher(pid, PidFdType::LAUNCH_SERVICE));
    ASSERT_GE(first.Get(), 0);
    const int32_t firstWatcher = manager_.pidfdMap_.at(pid).pidfd;
    UniqueFd second(manager_.AddWatcher(pid, PidFdType::LAUNCH_SERVICE));
    ASSERT_GE(second.Get(), 0);
    EXPECT_EQ(manager_.pidfdMap_.size(), 1);
    EXPECT_EQ(fcntl(firstWatcher, F_GETFD), -1);
    EXPECT_EQ(errno, EBADF);
    EXPECT_NE(fcntl(first.Get(), F_GETFD), -1);
    EXPECT_NE(fcntl(second.Get(), F_GETFD), -1);
    manager_.OnPidfdFired(pid, PidFdType::LAUNCH_SERVICE);
}

/**
 * @tc.name: Init_OriginalHandler_001
 * @tc.desc: Child and render cleanup retains the original appmgr handler.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServicePidfdTest, Init_OriginalHandler_001, TestSize.Level1)
{
    auto original = AAFwk::TaskHandlerWrap::CreateQueueHandler("pidfd_original_test");
    manager_.Init({}, original);
    EXPECT_EQ(manager_.taskHandler_.lock(), original);
    manager_.OnLaunchServicePidfdEvent(nullptr, EPOLLIN);
}
} // namespace OHOS::AppExecFwk
