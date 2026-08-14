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
#include "preload_so_startup_task.h"
#undef private
#undef protected

#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class PreloadSoStartupTaskTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { task_ = std::make_shared<PreloadSoStartupTask>("test", "test_url"); }
    void TearDown() { task_ = nullptr; }
    std::shared_ptr<PreloadSoStartupTask> task_;
};

HWTEST_F(PreloadSoStartupTaskTest, PreloadSoStartupTaskTest_GetType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_GetType_0100 start";
    EXPECT_EQ(task_->GetType(), "PreloadSo");
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_GetType_0100 end";
}

HWTEST_F(PreloadSoStartupTaskTest, PreloadSoStartupTaskTest_RunTaskInit_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_RunTaskInit_0100 start";
    auto callback = std::make_unique<StartupTaskResultCallback>();
    int32_t ret = task_->RunTaskInit(std::move(callback));
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_RunTaskInit_0100 end";
}

HWTEST_F(PreloadSoStartupTaskTest, PreloadSoStartupTaskTest_RunTaskOnDependencyCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_RunTaskOnDependencyCompleted_0100 start";
    auto result = std::make_shared<StartupTaskResult>();
    int32_t ret = task_->RunTaskOnDependencyCompleted("dep", result);
    EXPECT_EQ(ret, ERR_OK);
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_RunTaskOnDependencyCompleted_0100 end";
}

HWTEST_F(PreloadSoStartupTaskTest, PreloadSoStartupTaskTest_Flags_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_Flags_0100 start";
    EXPECT_FALSE(task_->GetWaitOnMainThread());
    EXPECT_FALSE(task_->GetCallCreateOnMainThread());
    GTEST_LOG_(INFO) << "PreloadSoStartupTaskTest_Flags_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
