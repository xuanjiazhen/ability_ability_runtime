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
#include <map>
#include <atomic>

#define private public
#define protected public
#include "startup_task_dispatcher.h"
#include "startup_task.h"
#undef private
#undef protected

#include "startup_sort_result.h"
#include "startup_task_result.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class DispatcherMockTask : public StartupTask {
public:
    explicit DispatcherMockTask(const std::string &name) : StartupTask(name)
    {
        state_ = State::CREATED;
    }
    ~DispatcherMockTask() = default;

    const std::string &GetType() const override
    {
        static const std::string type = "dispatcher_mock";
        return type;
    }

    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override
    {
        initCallCount_++;
        auto result = std::make_shared<StartupTaskResult>();
        result->SetResult(ERR_OK);
        if (callback) {
            callback->Call(result);
        }
        return ERR_OK;
    }

    int32_t RunTaskOnDependencyCompleted(const std::string &name,
        const std::shared_ptr<StartupTaskResult> &result) override
    {
        depCompletedCount_++;
        return ERR_OK;
    }

    int32_t GetInitCallCount() const { return initCallCount_; }

private:
    int32_t initCallCount_ = 0;
    int32_t depCompletedCount_ = 0;
};

class StartupTaskDispatcherTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}

    static std::shared_ptr<DispatcherMockTask> MakeTask(const std::string &name,
        const std::vector<std::string> &deps = {})
    {
        auto task = std::make_shared<DispatcherMockTask>(name);
        task->SetDependencies(deps);
        return task;
    }

    static std::shared_ptr<OnCompletedCallback> MakeCallback(bool &called, int32_t &resultCode)
    {
        called = false;
        resultCode = -1;
        return std::make_shared<OnCompletedCallback>(
            [&called, &resultCode](const std::shared_ptr<StartupTaskResult> &result) {
                called = true;
                if (result) {
                    resultCode = result->GetResultCode();
                }
            });
    }
};

/**
 * @tc.number: StartupTaskDispatcherTest_Run_0100
 * @tc.name: Run with null sortResult
 * @tc.desc: Verify Run returns ERR_STARTUP_INTERNAL_ERROR when sortResult is null.
 */
HWTEST_F(StartupTaskDispatcherTest, StartupTaskDispatcherTest_Run_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0100 start";
    std::map<std::string, std::shared_ptr<StartupTask>> tasks;
    tasks["A"] = MakeTask("A");
    auto dispatcher = std::make_shared<StartupTaskDispatcher>(tasks, nullptr);
    bool called = false;
    int32_t resultCode = -1;
    auto cb = MakeCallback(called, resultCode);
    int32_t ret = dispatcher->Run(cb, nullptr);
    EXPECT_EQ(ret, ERR_STARTUP_INTERNAL_ERROR);
    EXPECT_FALSE(called);
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0100 end";
}

/**
 * @tc.number: StartupTaskDispatcherTest_Run_0200
 * @tc.name: Run with null task in map
 * @tc.desc: Verify Run returns error when a task in the map is null.
 */
HWTEST_F(StartupTaskDispatcherTest, StartupTaskDispatcherTest_Run_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0200 start";
    std::map<std::string, std::shared_ptr<StartupTask>> tasks;
    tasks["A"] = nullptr;
    auto sortResult = std::make_shared<StartupSortResult>();
    sortResult->zeroDequeResult_.push_back("A");
    auto dispatcher = std::make_shared<StartupTaskDispatcher>(tasks, sortResult);
    bool called = false;
    int32_t resultCode = -1;
    auto cb = MakeCallback(called, resultCode);
    int32_t ret = dispatcher->Run(cb, nullptr);
    EXPECT_EQ(ret, ERR_STARTUP_INTERNAL_ERROR);
    EXPECT_FALSE(called);
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0200 end";
}

/**
 * @tc.number: StartupTaskDispatcherTest_Run_0300
 * @tc.name: Run single task with no dependencies
 * @tc.desc: Verify Run dispatches a single task and invokes completedCallback.
 */
HWTEST_F(StartupTaskDispatcherTest, StartupTaskDispatcherTest_Run_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0300 start";
    std::map<std::string, std::shared_ptr<StartupTask>> tasks;
    tasks["A"] = MakeTask("A");
    auto sortResult = std::make_shared<StartupSortResult>();
    sortResult->zeroDequeResult_.push_back("A");
    sortResult->startupChildrenMap_["A"] = {};
    auto dispatcher = std::make_shared<StartupTaskDispatcher>(tasks, sortResult);
    bool called = false;
    int32_t resultCode = -1;
    auto cb = MakeCallback(called, resultCode);
    int32_t ret = dispatcher->Run(cb, nullptr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(called);
    EXPECT_EQ(resultCode, ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0300 end";
}

/**
 * @tc.number: StartupTaskDispatcherTest_TimeoutStop_0100
 * @tc.name: TimeoutStop sets flag
 * @tc.desc: Verify TimeoutStop sets isTimeoutStopped_ to true.
 */
HWTEST_F(StartupTaskDispatcherTest, StartupTaskDispatcherTest_TimeoutStop_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_TimeoutStop_0100 start";
    std::map<std::string, std::shared_ptr<StartupTask>> tasks;
    auto sortResult = std::make_shared<StartupSortResult>();
    auto dispatcher = std::make_shared<StartupTaskDispatcher>(tasks, sortResult);
    EXPECT_FALSE(dispatcher->isTimeoutStopped_);
    dispatcher->TimeoutStop();
    EXPECT_TRUE(dispatcher->isTimeoutStopped_);
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_TimeoutStop_0100 end";
}

/**
 * @tc.number: StartupTaskDispatcherTest_Run_0400
 * @tc.name: Run with empty tasks and sortResult
 * @tc.desc: Verify Run with empty task map returns ERR_OK.
 */
HWTEST_F(StartupTaskDispatcherTest, StartupTaskDispatcherTest_Run_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0400 start";
    std::map<std::string, std::shared_ptr<StartupTask>> tasks;
    auto sortResult = std::make_shared<StartupSortResult>();
    auto dispatcher = std::make_shared<StartupTaskDispatcher>(tasks, sortResult);
    bool called = false;
    int32_t resultCode = -1;
    auto cb = MakeCallback(called, resultCode);
    int32_t ret = dispatcher->Run(cb, nullptr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(called);
    GTEST_LOG_(INFO) << "StartupTaskDispatcherTest_Run_0400 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
