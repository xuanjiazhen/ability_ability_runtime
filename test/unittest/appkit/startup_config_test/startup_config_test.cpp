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
#include "startup_config.h"
#include "startup_listener.h"
#undef private
#undef protected

#include "startup_task_result.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class StartupConfigTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(StartupConfigTest, StartupConfigTest_DefaultConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_DefaultConstructor_0100 start";
    StartupConfig config;
    EXPECT_EQ(config.GetAwaitTimeoutMs(), StartupConfig::DEFAULT_AWAIT_TIMEOUT_MS);
    GTEST_LOG_(INFO) << "StartupConfigTest_DefaultConstructor_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_TimeoutConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_TimeoutConstructor_0100 start";
    int32_t timeout = 5000;
    StartupConfig config(timeout);
    EXPECT_EQ(config.GetAwaitTimeoutMs(), timeout);
    GTEST_LOG_(INFO) << "StartupConfigTest_TimeoutConstructor_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_ListenerConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_ListenerConstructor_0100 start";
    auto listener = std::make_shared<StartupListener>(nullptr);
    StartupConfig config(listener);
    EXPECT_EQ(config.GetAwaitTimeoutMs(), StartupConfig::DEFAULT_AWAIT_TIMEOUT_MS);
    GTEST_LOG_(INFO) << "StartupConfigTest_ListenerConstructor_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_BothConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_BothConstructor_0100 start";
    int32_t timeout = 3000;
    auto listener = std::make_shared<StartupListener>(nullptr);
    StartupConfig config(timeout, listener);
    EXPECT_EQ(config.GetAwaitTimeoutMs(), timeout);
    GTEST_LOG_(INFO) << "StartupConfigTest_BothConstructor_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_Init_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_Init_0100 start";
    StartupConfig config;
    EXPECT_EQ(config.GetAwaitTimeoutMs(), StartupConfig::DEFAULT_AWAIT_TIMEOUT_MS);
    GTEST_LOG_(INFO) << "StartupConfigTest_Init_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_ListenerOnCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_ListenerOnCompleted_0100 start";
    bool called = false;
    int32_t resultCode = -1;
    auto listener = std::make_shared<StartupListener>(
        [&called, &resultCode](const std::shared_ptr<StartupTaskResult> &result) {
            called = true;
            if (result) { resultCode = result->GetResultCode(); }
        });
    StartupConfig config(listener);
    auto result = std::make_shared<StartupTaskResult>();
    result->SetResult(ERR_OK);
    config.ListenerOnCompleted(result);
    EXPECT_TRUE(called);
    EXPECT_EQ(resultCode, ERR_OK);
    GTEST_LOG_(INFO) << "StartupConfigTest_ListenerOnCompleted_0100 end";
}

HWTEST_F(StartupConfigTest, StartupConfigTest_GetCustomization_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigTest_GetCustomization_0100 start";
    StartupConfig config;
    EXPECT_TRUE(config.GetCustomization().empty());
    GTEST_LOG_(INFO) << "StartupConfigTest_GetCustomization_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
