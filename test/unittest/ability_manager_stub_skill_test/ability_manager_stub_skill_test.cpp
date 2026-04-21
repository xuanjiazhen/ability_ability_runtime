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

#include "ability_manager_stub_impl_mock.h"
#include "mock_ability_token.h"
#include "skill/skill_execute_callback_interface.h"
#include "skill/skill_execute_result.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class AbilityManagerStubSkillTest : public testing::Test {
public:
    void SetUp() override
    {
        stub_ = new AbilityManagerStubImplMock();
    }
    void TearDown() override {}
    void WriteInterfaceToken(MessageParcel &data)
    {
        data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor());
    }

    sptr<AbilityManagerStubImplMock> stub_;
};

/**
 * @tc.name: ExecuteInAppSkillInner_001
 * @tc.desc: Normal data triggers ExecuteInAppSkill call
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, ExecuteInAppSkillInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));
    data.WriteString16(Str8ToStr16("path"));
    data.WriteString16(Str8ToStr16("func"));
    AAFwk::WantParams params;
    data.WriteParcelable(&params);
    data.WriteBool(false); // no callback

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(reply.ReadInt32(), 0); // mock returns 0
}

/**
 * @tc.name: ExecuteInAppSkillInner_002
 * @tc.desc: Null skillArgs creates empty WantParams
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, ExecuteInAppSkillInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));
    data.WriteString16(Str8ToStr16(""));
    data.WriteString16(Str8ToStr16(""));
    data.WriteParcelable(nullptr); // null WantParams
    data.WriteBool(false);

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: ExecuteInAppSkillInner_003
 * @tc.desc: hasCallback=false does not read RemoteObject
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, ExecuteInAppSkillInner_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));
    data.WriteString16(Str8ToStr16(""));
    data.WriteString16(Str8ToStr16(""));
    AAFwk::WantParams params;
    data.WriteParcelable(&params);
    data.WriteBool(false); // hasCallback = false

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: ExecuteSkillDoneWithTokenInner_001
 * @tc.desc: Normal data triggers ExecuteSkillDone
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, ExecuteSkillDoneWithTokenInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    auto token = new (std::nothrow) MockAbilityToken();
    data.WriteRemoteObject(token);
    data.WriteString("req001");
    data.WriteInt32(0);
    AppExecFwk::SkillExecuteResult result;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: ExecuteSkillDoneWithTokenInner_002
 * @tc.desc: Null token returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, ExecuteSkillDoneWithTokenInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    // Write null remote object
    data.WriteRemoteObject(nullptr);
    data.WriteString("req001");
    data.WriteInt32(0);

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN), data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QuerySkillTypeInner_001
 * @tc.desc: Normal query returns skillType
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, QuerySkillTypeInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));

    auto res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE), data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: OnRemoteRequest_ExecuteInAppSkill_001
 * @tc.desc: Verify OnRemoteRequest routes to ExecuteInAppSkillInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, OnRemoteRequest_ExecuteInAppSkill_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));
    data.WriteString16(Str8ToStr16(""));
    data.WriteString16(Str8ToStr16(""));
    AAFwk::WantParams params;
    data.WriteParcelable(&params);
    data.WriteBool(false);

    auto code = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL);
    auto res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: OnRemoteRequest_ExecuteSkillDone_001
 * @tc.desc: Verify OnRemoteRequest routes to ExecuteSkillDoneWithTokenInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, OnRemoteRequest_ExecuteSkillDone_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    auto token = new (std::nothrow) MockAbilityToken();
    data.WriteRemoteObject(token);
    data.WriteString("req001");
    data.WriteInt32(0);
    AppExecFwk::SkillExecuteResult result;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    auto code = static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN);
    auto res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: OnRemoteRequest_QuerySkillType_001
 * @tc.desc: Verify OnRemoteRequest routes to QuerySkillTypeInner
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerStubSkillTest, OnRemoteRequest_QuerySkillType_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString16(Str8ToStr16("com.example"));
    data.WriteString16(Str8ToStr16("entry"));
    data.WriteString16(Str8ToStr16("testSkill"));

    auto code = static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE);
    auto res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

} // namespace AAFwk
} // namespace OHOS
