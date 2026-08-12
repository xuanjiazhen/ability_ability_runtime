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

#define private public
#define protected public
#include "ets_startup_task_executor.h"
#undef private
#undef protected

#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class EtsStartupTaskExecutorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(EtsStartupTaskExecutorTest, EtsStartupTaskExecutorTest_RunOnTaskPool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_RunOnTaskPool_0100 start";
    int32_t ret = ETSStartupTaskExecutor::RunOnTaskPool(nullptr, nullptr, nullptr, "", "");
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_RunOnTaskPool_0100 end";
}

HWTEST_F(EtsStartupTaskExecutorTest, EtsStartupTaskExecutorTest_RunOnMainThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_RunOnMainThread_0100 start";
    int32_t ret = ETSStartupTaskExecutor::RunOnMainThread(nullptr, nullptr, nullptr, "");
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_RunOnMainThread_0100 end";
}

HWTEST_F(EtsStartupTaskExecutorTest, EtsStartupTaskExecutorTest_PostMainThreadTask_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_PostMainThreadTask_0100 start";
    ETSStartupTaskExecutor::PostMainThreadTask(nullptr);
    SUCCEED();
    GTEST_LOG_(INFO) << "EtsStartupTaskExecutorTest_PostMainThreadTask_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
