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
#include <memory>
#include <string>

#define private public
#define protected public
#include "startup_task_manager.h"
#undef private
#undef protected

#include "startup_config.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class MockStartupTaskForManager : public StartupTask {
public:
    explicit MockStartupTaskForManager(const std::string &name) : StartupTask(name)
    {
        state_ = State::CREATED;
    }
    ~MockStartupTaskForManager() = default;
    const std::string &GetType() const override { static const std::string t = "mock"; return t; }
    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override { return ERR_OK; }
    int32_t RunTaskOnDependencyCompleted(const std::string &name,
        const std::shared_ptr<StartupTaskResult> &result) override { return ERR_OK; }
};

class StartupTaskManagerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { mgr_ = std::make_shared<StartupTaskManager>(); }
    void TearDown() { mgr_ = nullptr; }
    std::shared_ptr<StartupTaskManager> mgr_;
};

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_AddTask_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0100 start";
    auto task = std::make_shared<MockStartupTaskForManager>("A");
    int32_t ret = mgr_->AddTask(task);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(mgr_->GetStartupTaskCount(), 1u);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0100 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_AddTask_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0200 start";
    int32_t ret = mgr_->AddTask(nullptr);
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0200 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_AddTask_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0300 start";
    auto task = std::make_shared<MockStartupTaskForManager>("A");
    mgr_->AddTask(task);
    int32_t ret = mgr_->AddTask(task);
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_AddTask_0300 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_GetStartupTasks_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_GetStartupTasks_0100 start";
    EXPECT_TRUE(mgr_->GetStartupTasks().empty());
    auto task = std::make_shared<MockStartupTaskForManager>("A");
    mgr_->AddTask(task);
    EXPECT_EQ(mgr_->GetStartupTasks().size(), 1u);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_GetStartupTasks_0100 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_GetStartupTaskManagerId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_GetStartupTaskManagerId_0100 start";
    EXPECT_GE(mgr_->GetStartupTaskManagerId(), 0);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_GetStartupTaskManagerId_0100 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_SetConfig_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_SetConfig_0100 start";
    auto config = std::make_shared<StartupConfig>();
    mgr_->SetConfig(config);
    EXPECT_NE(mgr_->config_, nullptr);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_SetConfig_0100 end";
}

HWTEST_F(StartupTaskManagerTest, StartupTaskManagerTest_SetTimeoutCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_SetTimeoutCallback_0100 start";
    bool called = false;
    mgr_->SetTimeoutCallback([&called]() { called = true; });
    EXPECT_FALSE(called);
    GTEST_LOG_(INFO) << "StartupTaskManagerTest_SetTimeoutCallback_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
