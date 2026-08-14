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
#include "js_startup_task_executor.h"
#undef private
#undef protected

#include "startup_task_result.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class JsStartupTaskExecutorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(JsStartupTaskExecutorTest, JsStartupTaskExecutorTest_ReplyFailed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsStartupTaskExecutorTest_ReplyFailed_0100 start";
    StartupTaskResultCallback *rawCallback = nullptr;
    std::unique_ptr<StartupTaskResultCallback> callback(rawCallback);
    EXPECT_NO_FATAL_FAILURE(
        JsStartupTaskExecutor::ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR, "test error"));
    GTEST_LOG_(INFO) << "JsStartupTaskExecutorTest_ReplyFailed_0100 end";
}

HWTEST_F(JsStartupTaskExecutorTest, JsStartupTaskExecutorTest_ReplyFailed_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsStartupTaskExecutorTest_ReplyFailed_0200 start";
    bool called = false;
    auto callback = std::make_unique<StartupTaskResultCallback>();
    callback->Push([&called](const std::shared_ptr<StartupTaskResult> &r) {
        called = true;
        if (r) {
            EXPECT_EQ(r->GetResultCode(), ERR_STARTUP_INTERNAL_ERROR);
        }
    });
    JsStartupTaskExecutor::ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR, "test error");
    EXPECT_TRUE(called);
    GTEST_LOG_(INFO) << "JsStartupTaskExecutorTest_ReplyFailed_0200 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
