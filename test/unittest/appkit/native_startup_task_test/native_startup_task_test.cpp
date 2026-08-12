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
#include <functional>

#define private public
#define protected public
#include "native_startup_task.h"
#undef private
#undef protected

#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class NativeStartupTaskTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { task_ = std::make_shared<NativeStartupTask>("NativeTask"); }
    void TearDown() { task_ = nullptr; }
    std::shared_ptr<NativeStartupTask> task_;
};

HWTEST_F(NativeStartupTaskTest, NativeStartupTaskTest_GetType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_GetType_0100 start";
    EXPECT_EQ(task_->GetType(), "Native");
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_GetType_0100 end";
}

HWTEST_F(NativeStartupTaskTest, NativeStartupTaskTest_RunTaskInit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_RunTaskInit_0100 start";
    bool called = false;
    task_->taskInitFunc_ = [&called](std::unique_ptr<StartupTaskResultCallback> cb) {
        called = true;
        auto result = std::make_shared<StartupTaskResult>();
        result->SetResult(ERR_OK);
        if (cb) { cb->Call(result); }
    };
    auto callback = std::make_unique<StartupTaskResultCallback>();
    int32_t ret = task_->RunTaskInit(std::move(callback));
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(called);
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_RunTaskInit_0100 end";
}

HWTEST_F(NativeStartupTaskTest, NativeStartupTaskTest_RunTaskOnDependencyCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_RunTaskOnDependencyCompleted_0100 start";
    auto result = std::make_shared<StartupTaskResult>();
    int32_t ret = task_->RunTaskOnDependencyCompleted("dep", result);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "NativeStartupTaskTest_RunTaskOnDependencyCompleted_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
