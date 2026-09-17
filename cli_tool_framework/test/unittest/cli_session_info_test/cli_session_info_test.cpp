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
#include <parcel.h>

#include "cli_session_info.h"

using namespace testing::ext;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t TEST_EXIT_CODE = 7;
const std::string TEST_TOOL_CALL_ID = "tool-call-123_ABC";
const std::string TEST_DM_SESSION_ID = "dm-session-456_xyz";
}

class CliSessionInfoTest : public testing::Test {};

/**
 * @tc.name: CliSessionInfo_Parcelable_0100
 * @tc.desc: Test CliSessionInfo optional result marshalling branches
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Parcelable_0100, TestSize.Level1)
{
    CliSessionInfo runningInfo;
    runningInfo.sessionId = "session-running";
    runningInfo.toolName = "tool";
    runningInfo.status = "running";

    Parcel runningParcel;
    ASSERT_TRUE(runningInfo.Marshalling(runningParcel));
    runningParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> runningResult(CliSessionInfo::Unmarshalling(runningParcel));
    ASSERT_NE(runningResult, nullptr);
    EXPECT_EQ(runningResult->sessionId, "session-running");
    EXPECT_EQ(runningResult->toolName, "tool");
    EXPECT_EQ(runningResult->status, "running");
    EXPECT_EQ(runningResult->result, nullptr);

    CliSessionInfo completedInfo;
    completedInfo.sessionId = "session-completed";
    completedInfo.toolName = "tool";
    completedInfo.status = "completed";
    completedInfo.result = std::make_shared<ExecResult>();
    completedInfo.result->exitCode = TEST_EXIT_CODE;
    completedInfo.result->outputText = "ok";

    Parcel completedParcel;
    ASSERT_TRUE(completedInfo.Marshalling(completedParcel));
    completedParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> completedResult(CliSessionInfo::Unmarshalling(completedParcel));
    ASSERT_NE(completedResult, nullptr);
    ASSERT_NE(completedResult->result, nullptr);
    EXPECT_EQ(completedResult->status, "completed");
    EXPECT_EQ(completedResult->result->exitCode, TEST_EXIT_CODE);
    EXPECT_EQ(completedResult->result->outputText, "ok");
}

/**
 * @tc.name: CliSessionInfo_Unmarshalling_0200
 * @tc.desc: Test CliSessionInfo unmarshalling failure branches
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Unmarshalling_0200, TestSize.Level1)
{
    Parcel emptyParcel;
    EXPECT_EQ(CliSessionInfo::Unmarshalling(emptyParcel), nullptr);

    Parcel missingStatusParcel;
    ASSERT_TRUE(missingStatusParcel.WriteString("session"));
    ASSERT_TRUE(missingStatusParcel.WriteString("tool"));
    missingStatusParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingStatusParcel), nullptr);

    Parcel missingToolNameParcel;
    ASSERT_TRUE(missingToolNameParcel.WriteString("session"));
    missingToolNameParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingToolNameParcel), nullptr);

    Parcel missingHasResultParcel;
    ASSERT_TRUE(missingHasResultParcel.WriteString("session"));
    ASSERT_TRUE(missingHasResultParcel.WriteString("tool"));
    ASSERT_TRUE(missingHasResultParcel.WriteString("running"));
    missingHasResultParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingHasResultParcel), nullptr);

    Parcel missingResultParcel;
    ASSERT_TRUE(missingResultParcel.WriteString("session"));
    ASSERT_TRUE(missingResultParcel.WriteString("tool"));
    ASSERT_TRUE(missingResultParcel.WriteString("completed"));
    ASSERT_TRUE(missingResultParcel.WriteBool(true));
    missingResultParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(missingResultParcel), nullptr);
}

/**
 * @tc.name: CliSessionInfo_Parcelable_0300
 * @tc.desc: Test CliSessionInfo round-trip with both / no trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Parcelable_0300, TestSize.Level1)
{
    // Both identifiers provided.
    CliSessionInfo withIds;
    withIds.sessionId = "session-with-ids";
    withIds.toolName = "tool";
    withIds.status = "running";
    withIds.toolCallId = TEST_TOOL_CALL_ID;
    withIds.dmSessionId = TEST_DM_SESSION_ID;

    Parcel withIdsParcel;
    ASSERT_TRUE(withIds.Marshalling(withIdsParcel));
    withIdsParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> withIdsResult(CliSessionInfo::Unmarshalling(withIdsParcel));
    ASSERT_NE(withIdsResult, nullptr);
    EXPECT_EQ(withIdsResult->sessionId, "session-with-ids");
    EXPECT_EQ(withIdsResult->toolName, "tool");
    EXPECT_EQ(withIdsResult->status, "running");
    EXPECT_EQ(withIdsResult->toolCallId, TEST_TOOL_CALL_ID);
    EXPECT_EQ(withIdsResult->dmSessionId, TEST_DM_SESSION_ID);
    EXPECT_EQ(withIdsResult->result, nullptr);

    // Identifiers not provided: empty strings survive a round trip.
    CliSessionInfo withoutIds;
    withoutIds.sessionId = "session-without-ids";
    withoutIds.toolName = "tool";
    withoutIds.status = "failed";

    Parcel withoutIdsParcel;
    ASSERT_TRUE(withoutIds.Marshalling(withoutIdsParcel));
    withoutIdsParcel.RewindRead(0);
    std::unique_ptr<CliSessionInfo> withoutIdsResult(CliSessionInfo::Unmarshalling(withoutIdsParcel));
    ASSERT_NE(withoutIdsResult, nullptr);
    EXPECT_EQ(withoutIdsResult->sessionId, "session-without-ids");
    EXPECT_TRUE(withoutIdsResult->toolCallId.empty());
    EXPECT_TRUE(withoutIdsResult->dmSessionId.empty());
}

/**
 * @tc.name: CliSessionInfo_Unmarshalling_0300
 * @tc.desc: Test CliSessionInfo strict tail read rejects legacy parcels without trace identifiers
 * @tc.type: FUNC
 */
HWTEST_F(CliSessionInfoTest, CliSessionInfo_Unmarshalling_0300, TestSize.Level1)
{
    // Legacy sender format: sessionId + toolName + status + hasResult(false), no trace ids.
    Parcel legacyNoResultParcel;
    ASSERT_TRUE(legacyNoResultParcel.WriteString("session"));
    ASSERT_TRUE(legacyNoResultParcel.WriteString("tool"));
    ASSERT_TRUE(legacyNoResultParcel.WriteString("running"));
    ASSERT_TRUE(legacyNoResultParcel.WriteBool(false));
    legacyNoResultParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(legacyNoResultParcel), nullptr);

    // Legacy sender format with an ExecResult parcelable but no trace ids.
    Parcel legacyResultParcel;
    ASSERT_TRUE(legacyResultParcel.WriteString("session"));
    ASSERT_TRUE(legacyResultParcel.WriteString("tool"));
    ASSERT_TRUE(legacyResultParcel.WriteString("completed"));
    ASSERT_TRUE(legacyResultParcel.WriteBool(true));
    ExecResult execResult;
    execResult.exitCode = TEST_EXIT_CODE;
    execResult.outputText = "ok";
    ASSERT_TRUE(legacyResultParcel.WriteParcelable(&execResult));
    legacyResultParcel.RewindRead(0);
    EXPECT_EQ(CliSessionInfo::Unmarshalling(legacyResultParcel), nullptr);
}
} // namespace CliTool
} // namespace OHOS
