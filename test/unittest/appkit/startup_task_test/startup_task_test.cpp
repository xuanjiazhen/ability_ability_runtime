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
#include <vector>

#define private public
#define protected public
#include "startup_task.h"
#undef private
#undef protected

#include "startup_task_result.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class ConcreteStartupTask : public StartupTask {
public:
    explicit ConcreteStartupTask(const std::string &name) : StartupTask(name) {}
    ~ConcreteStartupTask() = default;
    const std::string &GetType() const override 
    { 
        static const std::string t = "concrete";
        return t; 
    }
    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override
    {
        return ERR_OK;
    }
    int32_t RunTaskOnDependencyCompleted(
        const std::string &name,
        const std::shared_ptr<StartupTaskResult> &result) override
        {
            return ERR_OK;
        }
};

class StartupTaskTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { task_ = std::make_shared<ConcreteStartupTask>("TestTask"); }
    void TearDown() { task_ = nullptr; }
    std::shared_ptr<ConcreteStartupTask> task_;
};

HWTEST_F(StartupTaskTest, StartupTaskTest_GetName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_GetName_0100 start";
    EXPECT_EQ(task_->GetName(), "TestTask");
    GTEST_LOG_(INFO) << "StartupTaskTest_GetName_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_State_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_State_0100 start";
    EXPECT_EQ(task_->GetState(), StartupTask::State::CREATED);
    GTEST_LOG_(INFO) << "StartupTaskTest_State_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_Dependencies_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_Dependencies_0100 start";
    EXPECT_TRUE(task_->GetDependencies().empty());
    EXPECT_EQ(task_->GetDependenciesCount(), 0u);
    std::vector<std::string> deps = {"A", "B", "C"};
    task_->SetDependencies(deps);
    auto result = task_->GetDependencies();
    EXPECT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "A");
    EXPECT_EQ(result[2], "C");
    EXPECT_EQ(task_->GetDependenciesCount(), 3u);
    GTEST_LOG_(INFO) << "StartupTaskTest_Dependencies_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_WaitOnMainThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_WaitOnMainThread_0100 start";
    EXPECT_TRUE(task_->GetWaitOnMainThread());
    task_->SetWaitOnMainThread(false);
    EXPECT_FALSE(task_->GetWaitOnMainThread());
    GTEST_LOG_(INFO) << "StartupTaskTest_WaitOnMainThread_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_CallCreateOnMainThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_CallCreateOnMainThread_0100 start";
    EXPECT_TRUE(task_->GetCallCreateOnMainThread());
    task_->SetCallCreateOnMainThread(false);
    EXPECT_FALSE(task_->GetCallCreateOnMainThread());
    GTEST_LOG_(INFO) << "StartupTaskTest_CallCreateOnMainThread_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_SaveResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0100 start";
    auto result = std::make_shared<StartupTaskResult>();
    result->SetResult(ERR_OK);
    task_->SaveResult(result);
    EXPECT_EQ(task_->GetState(), StartupTask::State::INITIALIZED);
    EXPECT_EQ(task_->GetResult()->GetResultCode(), ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_SaveResult_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0200 start";
    auto result = std::make_shared<StartupTaskResult>();
    result->SetResult(ERR_STARTUP_INTERNAL_ERROR);
    task_->SaveResult(result);
    EXPECT_EQ(task_->GetState(), StartupTask::State::CREATED);
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0200 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_SaveResult_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0300 start";
    task_->SaveResult(nullptr);
    EXPECT_EQ(task_->GetState(), StartupTask::State::CREATED);
    GTEST_LOG_(INFO) << "StartupTaskTest_SaveResult_0300 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_RemoveResult_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_RemoveResult_0100 start";
    EXPECT_NE(task_->RemoveResult(), ERR_OK);
    auto result = std::make_shared<StartupTaskResult>();
    result->SetResult(ERR_OK);
    task_->SaveResult(result);
    EXPECT_EQ(task_->RemoveResult(), ERR_OK);
    EXPECT_EQ(task_->GetState(), StartupTask::State::CREATED);
    GTEST_LOG_(INFO) << "StartupTaskTest_RemoveResult_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_AddExtraCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_AddExtraCallback_0100 start";
    auto cb = std::make_unique<StartupTaskResultCallback>();
    EXPECT_NE(task_->AddExtraCallback(std::move(cb)), ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskTest_AddExtraCallback_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_RunTaskPreInit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_RunTaskPreInit_0100 start";
    auto cb = std::make_unique<StartupTaskResultCallback>();
    int32_t ret = task_->RunTaskPreInit(cb);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(task_->GetState(), StartupTask::State::INITIALIZING);
    GTEST_LOG_(INFO) << "StartupTaskTest_RunTaskPreInit_0100 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_RunTaskPreInit_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_RunTaskPreInit_0200 start";
    task_->state_ = StartupTask::State::INITIALIZED;
    auto cb = std::make_unique<StartupTaskResultCallback>();
    int32_t ret = task_->RunTaskPreInit(cb);
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "StartupTaskTest_RunTaskPreInit_0200 end";
}

HWTEST_F(StartupTaskTest, StartupTaskTest_DumpDependencies_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTaskTest_DumpDependencies_0100 start";
    EXPECT_EQ(task_->DumpDependencies(), "");
    std::vector<std::string> deps = {"A", "B"};
    task_->SetDependencies(deps);
    std::string dump = task_->DumpDependencies();
    EXPECT_FALSE(dump.empty());
    EXPECT_NE(dump.find("A"), std::string::npos);
    EXPECT_NE(dump.find("B"), std::string::npos);
    GTEST_LOG_(INFO) << "StartupTaskTest_DumpDependencies_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
