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
#include <mutex>
#include <sys/epoll.h>
#define private public
#include "app_pidfd_manager.h"
#undef private
#include "task_handler_wrap.h"

using namespace testing::ext;
namespace OHOS::AppExecFwk {
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
