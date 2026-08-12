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
#include "ets_startup_task_result.h"
#undef private
#undef protected

#include "startup_task_result.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class EtsStartupTaskResultTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(EtsStartupTaskResultTest, EtsStartupTaskResultTest_GetResultType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_GetResultType_0100 start";
    EtsStartupTaskResult result;
    EXPECT_EQ(result.GetResultType(), StartupTaskResult::ResultType::ETS);
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_GetResultType_0100 end";
}

HWTEST_F(EtsStartupTaskResultTest, EtsStartupTaskResultTest_DefaultConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_DefaultConstructor_0100 start";
    auto result = std::make_shared<EtsStartupTaskResult>();
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetResultType(), StartupTaskResult::ResultType::ETS);
    EXPECT_EQ(result->GetEtsStartupResultRef(), nullptr);
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_DefaultConstructor_0100 end";
}

HWTEST_F(EtsStartupTaskResultTest, EtsStartupTaskResultTest_ErrorConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_ErrorConstructor_0100 start";
    EtsStartupTaskResult result(ERR_STARTUP_INTERNAL_ERROR, "test error");
    EXPECT_EQ(result.GetResultCode(), ERR_STARTUP_INTERNAL_ERROR);
    EXPECT_EQ(result.GetResultMessage(), "test error");
    EXPECT_EQ(result.GetResultType(), StartupTaskResult::ResultType::ETS);
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_ErrorConstructor_0100 end";
}

HWTEST_F(EtsStartupTaskResultTest, EtsStartupTaskResultTest_NullVmConstructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_NullVmConstructor_0100 start";
    ani_object obj = nullptr;
    EtsStartupTaskResult result(nullptr, obj);
    EXPECT_EQ(result.GetResultType(), StartupTaskResult::ResultType::ETS);
    GTEST_LOG_(INFO) << "EtsStartupTaskResultTest_NullVmConstructor_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
