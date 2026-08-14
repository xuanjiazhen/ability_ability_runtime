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
#include "ets_startup_config.h"
#undef private
#undef protected

#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class EtsStartupConfigTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(EtsStartupConfigTest, EtsStartupConfigTest_Init_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_Init_0100 start";
    ETSStartupConfig config(nullptr);
    int32_t ret = config.Init(nullptr, nullptr, "", nullptr);
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_Init_0100 end";
}

HWTEST_F(EtsStartupConfigTest, EtsStartupConfigTest_Init_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_Init_0200 start";
    ETSStartupConfig config(nullptr);
    auto runtime = std::unique_ptr<Runtime>(nullptr);
    auto context = std::shared_ptr<Context>(nullptr);
    int32_t ret = config.Init(runtime, context, "src", nullptr);
    EXPECT_NE(ret, ERR_OK);
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_Init_0200 end";
}

HWTEST_F(EtsStartupConfigTest, EtsStartupConfigTest_GetAwaitTimeoutMs_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_GetAwaitTimeoutMs_0100 start";
    ETSStartupConfig config(nullptr);
    EXPECT_EQ(config.GetAwaitTimeoutMs(), StartupConfig::DEFAULT_AWAIT_TIMEOUT_MS);
    GTEST_LOG_(INFO) << "EtsStartupConfigTest_GetAwaitTimeoutMs_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
