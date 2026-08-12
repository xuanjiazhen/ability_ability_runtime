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
#include "startup_config_instance.h"
#undef private
#undef protected

#include "runtime.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class StartupConfigInstanceTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.number: StartupConfigInstanceTest_CreateStartupConfig_0100
 * @tc.name: CreateStartupConfig with null runtime
 * @tc.desc: Verify CreateStartupConfig returns nullptr when runtime is null.
 */
HWTEST_F(StartupConfigInstanceTest, StartupConfigInstanceTest_CreateStartupConfig_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigInstanceTest_CreateStartupConfig_0100 start";
    std::unique_ptr<Runtime> runtime = nullptr;
    auto result = StartupConfigInstance::CreateStartupConfig(runtime, CODE_LANGUAGE_ARKTS_1_0);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "StartupConfigInstanceTest_CreateStartupConfig_0100 end";
}

/**
 * @tc.number: StartupConfigInstanceTest_CreateJsStartupConfig_0100
 * @tc.name: CreateJsStartupConfig with null runtime
 * @tc.desc: Verify CreateJsStartupConfig returns nullptr when runtime is null.
 */
HWTEST_F(StartupConfigInstanceTest, StartupConfigInstanceTest_CreateJsStartupConfig_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupConfigInstanceTest_CreateJsStartupConfig_0100 start";
    std::unique_ptr<Runtime> runtime = nullptr;
    auto result = StartupConfigInstance::CreateJsStartupConfig(runtime);
    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "StartupConfigInstanceTest_CreateJsStartupConfig_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
