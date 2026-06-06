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

#include "cli_event_report.h"
#include "cli_error_code.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* TEST_BUNDLE_NAME = "com.test.example";
constexpr const char* TEST_CLI_NAME = "test_tool";
constexpr const char* TEST_ARGS = "arg1=value1 arg2=value2";
constexpr int32_t TEST_SIGNAL_NUM = 9;  // SIGKILL
constexpr int64_t TEST_DURATION_MS = 5000;
}

class CliEventReportTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ReportExecuteCli_0100
 * @tc.desc: Test CLI execution event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportExecuteCli_0100, TestSize.Level1)
{
    // Should not crash when reporting execute CLI event
    ReportExecuteCli(TEST_BUNDLE_NAME, TEST_CLI_NAME, TEST_ARGS);
    SUCCEED();
}

/**
 * @tc.name: ReportExecuteCli_0200
 * @tc.desc: Test CLI execution event reporting without args
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportExecuteCli_0200, TestSize.Level1)
{
    // Should not crash when reporting execute CLI event without args
    ReportExecuteCli(TEST_BUNDLE_NAME, TEST_CLI_NAME, "");
    SUCCEED();
}

/**
 * @tc.name: ReportExecuteCliFailed_0100
 * @tc.desc: Test CLI execution failure event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportExecuteCliFailed_0100, TestSize.Level1)
{
    // Should not crash when reporting execute CLI failed event
    ReportExecuteCliFailed(TEST_BUNDLE_NAME, TEST_CLI_NAME, REASON_PERMISSION_DENIED);
    SUCCEED();
}

/**
 * @tc.name: ReportCliRunTimeout_0100
 * @tc.desc: Test CLI timeout event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliRunTimeout_0100, TestSize.Level1)
{
    // Should not crash when reporting CLI timeout event
    ReportCliRunTimeout(TEST_BUNDLE_NAME, TEST_CLI_NAME, TEST_DURATION_MS);
    SUCCEED();
}

/**
 * @tc.name: ReportCliRunError_0100
 * @tc.desc: Test CLI error exit event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliRunError_0100, TestSize.Level1)
{
    // Should not crash when reporting CLI error exit event
    ReportCliRunError(TEST_CLI_NAME, TEST_SIGNAL_NUM);
    SUCCEED();
}

/**
 * @tc.name: GetFailureReason_0100
 * @tc.desc: Test failure reason mapping for known error codes
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, GetFailureReason_0100, TestSize.Level1)
{
    EXPECT_EQ(GetFailureReason(ERR_PERMISSION_DENIED), REASON_PERMISSION_DENIED);
    EXPECT_EQ(GetFailureReason(ERR_TOOL_NOT_EXIST), REASON_TOOL_NOT_EXIST);
    EXPECT_EQ(GetFailureReason(ERR_SESSION_LIMIT_EXCEEDED), REASON_SESSION_LIMIT_EXCEEDED);
    EXPECT_EQ(GetFailureReason(ERR_NO_INIT), REASON_PROCESS_CREATE_FAILED);
    EXPECT_EQ(GetFailureReason(ERR_INVALID_PARAM), REASON_INVALID_PARAM);
    EXPECT_EQ(GetFailureReason(ERR_NOT_HAP), REASON_NOT_HAP);
    EXPECT_EQ(GetFailureReason(ERR_NOT_SYSTEM_APP), REASON_NOT_SYSTEM_APP);
}

/**
 * @tc.name: GetFailureReason_0200
 * @tc.desc: Test failure reason mapping for unknown error codes
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, GetFailureReason_0200, TestSize.Level1)
{
    EXPECT_EQ(GetFailureReason(-1), REASON_UNKNOWN);
    EXPECT_EQ(GetFailureReason(99999), REASON_UNKNOWN);
}

} // namespace CliTool
} // namespace OHOS
