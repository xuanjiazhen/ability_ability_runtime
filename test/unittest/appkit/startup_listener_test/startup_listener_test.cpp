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
#include "startup_listener.h"
#undef private
#undef protected

#include "startup_task_result.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class StartupListenerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(StartupListenerTest, StartupListenerTest_OnCompleted_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0100 start";
    bool called = false;
    int32_t resultCode = -1;
    auto listener = std::make_shared<StartupListener>(
        [&called, &resultCode](const std::shared_ptr<StartupTaskResult> &result) {
            called = true;
            if (result) { resultCode = result->GetResultCode(); }
        });
    auto result = std::make_shared<StartupTaskResult>();
    result->SetResult(ERR_OK);
    listener->OnCompleted(result);
    EXPECT_TRUE(called);
    EXPECT_EQ(resultCode, ERR_OK);
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0100 end";
}

HWTEST_F(StartupListenerTest, StartupListenerTest_OnCompleted_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0200 start";
    auto listener = std::make_shared<StartupListener>(nullptr);
    auto result = std::make_shared<StartupTaskResult>();
    listener->OnCompleted(result);
    SUCCEED();
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0200 end";
}

HWTEST_F(StartupListenerTest, StartupListenerTest_OnCompleted_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0300 start";
    bool called = false;
    auto listener = std::make_shared<StartupListener>(
        [&called](const std::shared_ptr<StartupTaskResult> &result) {
            called = true;
        });
    listener->OnCompleted(nullptr);
    EXPECT_TRUE(called);
    GTEST_LOG_(INFO) << "StartupListenerTest_OnCompleted_0300 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
