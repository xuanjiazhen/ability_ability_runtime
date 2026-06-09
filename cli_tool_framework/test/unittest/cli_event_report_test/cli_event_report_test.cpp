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
}

class CliEventReportTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ReportCliExecuteFailed_0100
 * @tc.desc: Test CLI execution failed event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0100, TestSize.Level1)
{
    // Should not crash when reporting execute CLI failed event
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, TEST_CLI_NAME, REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliExecuteFailed_0200
 * @tc.desc: Test CLI execution failed event reporting with empty cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0200, TestSize.Level1)
{
    // Should not crash when reporting execute CLI failed event with empty cliName
    // Empty cliName should be replaced with "<empty>" placeholder
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, "", REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliExecuteFailed_0300
 * @tc.desc: Test CLI execution failed event reporting with "undefined" cliName
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliExecuteFailed_0300, TestSize.Level1)
{
    // Should not crash when reporting execute CLI failed event with "undefined" cliName
    // "undefined" cliName should be replaced with "<empty>" placeholder
    ReportCliExecuteFailed(TEST_BUNDLE_NAME, "undefined", REASON_INVALID_PARAM, "");
    SUCCEED();
}

/**
 * @tc.name: ReportCliTimeout_0100
 * @tc.desc: Test CLI timeout event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliTimeout_0100, TestSize.Level1)
{
    // Should not crash when reporting CLI timeout event
    ReportCliTimeout(TEST_BUNDLE_NAME, TEST_CLI_NAME, "5000");
    SUCCEED();
}

/**
 * @tc.name: ReportCliSignal_0100
 * @tc.desc: Test CLI signal event reporting
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, ReportCliSignal_0100, TestSize.Level1)
{
    // Should not crash when reporting CLI signal event
    ReportCliSignal(TEST_CLI_NAME, "9");
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
    EXPECT_EQ(GetFailureReason(ERR_TOOL_NOT_EXIST), REASON_TOOL_NOT_FOUND);
    EXPECT_EQ(GetFailureReason(ERR_SUBCOMMAND_NOT_FOUND), REASON_SUBCOMMAND_NOT_FOUND);
    EXPECT_EQ(GetFailureReason(ERR_SUBCOMMAND_NOT_SUPPORTED), REASON_SUBCOMMAND_NOT_SUPPORTED);
    EXPECT_EQ(GetFailureReason(ERR_SESSION_LIMIT_EXCEEDED), REASON_SESSION_LIMIT_EXCEEDED);
    EXPECT_EQ(GetFailureReason(ERR_NO_INIT), REASON_PROCESS_CREATE_FAILED);
    EXPECT_EQ(GetFailureReason(ERR_INVALID_PARAM), REASON_INVALID_PARAM);
}

/**
 * @tc.name: GetFailureReason_0200
 * @tc.desc: Test failure reason mapping for unknown error codes
 * @tc.type: FUNC
 */
HWTEST_F(CliEventReportTest, GetFailureReason_0200, TestSize.Level1)
{
    EXPECT_EQ(GetFailureReason(-1), REASON_INVALID_PARAM);
    EXPECT_EQ(GetFailureReason(99999), REASON_INVALID_PARAM);
}

} // namespace CliTool
} // namespace OHOS
