/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#include "startup_utils.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS::AbilityRuntime {
class StartupUtilsTest : public Test {
};

HWTEST_F(StartupUtilsTest, GetErrorMessageTest_0100, TestSize.Level1)
{
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_INVALID_VALUE),
        "invalid parameter.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR),
        "internal error.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_DEPENDENCY_NOT_FOUND),
        "startup task or its dependency not found.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_CIRCULAR_DEPENDENCY),
        "the startup tasks have circular dependencies.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP),
        "an error occurred while running the startup tasks.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_TIMEOUT),
        "running startup tasks timeout.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_CONFIG_NOT_FOUND),
        "internal error.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_CONFIG_PATH_ERROR),
        "internal error.");
    EXPECT_EQ(StartupUtils::GetErrorMessage(ERR_STARTUP_CONFIG_PARSE_ERROR),
        "internal error.");
}
} // namespace OHOS::AbilityRuntime
