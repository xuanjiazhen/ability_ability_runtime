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
#include "hilog_tag_wrapper.h"
#include "iremote_proxy.h"
#include "mock_ability_token.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {

const std::string BUNDLE_NAME = "com.example.test";
const std::string MODULE_NAME = "entry";
const std::string SKILL_NAME = "testSkill";
const std::string SCRIPT_PATH = "ets/skill/TestSkill";
const std::string FUNC_NAME = "handleSkill";
const std::string REQUEST_CODE = "req_123";

}  // namespace

class AbilityManagerSkillStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel &data);
    sptr<AbilityManagerStubImplMock> stub_ { nullptr };
};

void AbilityManagerSkillStubTest::SetUpTestCase(void) {}
void AbilityManagerSkillStubTest::TearDownTestCase(void) {}
void AbilityManagerSkillStubTest::TearDown() {}

void AbilityManagerSkillStubTest::SetUp()
{
    stub_ = new AbilityManagerStubImplMock();
}

void AbilityManagerSkillStubTest::WriteInterfaceToken(MessageParcel &data)
{
    data.WriteInterfaceToken(AbilityManagerStub::GetDescriptor());
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0100
 * @tc.desc: Test ExecuteInAppSkillWithTokenIdInner with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    uint32_t callerTokenId = 100;
    data.WriteUint32(callerTokenId);
    data.WriteString16(Str8ToStr16(BUNDLE_NAME));
    data.WriteString16(Str8ToStr16(MODULE_NAME));
    data.WriteString16(Str8ToStr16(SKILL_NAME));
    data.WriteString16(Str8ToStr16(SCRIPT_PATH));
    data.WriteString16(Str8ToStr16(FUNC_NAME));

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(skillArgs.get());

    data.WriteBool(false); // hasCallback

    EXPECT_CALL(*stub_, ExecuteInAppSkillWithTokenId(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0200
 * @tc.desc: Test ExecuteInAppSkillWithTokenIdInner with callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0200 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteUint32(100u);
    data.WriteString16(Str8ToStr16(BUNDLE_NAME));
    data.WriteString16(Str8ToStr16(MODULE_NAME));
    data.WriteString16(Str8ToStr16(SKILL_NAME));
    data.WriteString16(Str8ToStr16(SCRIPT_PATH));
    data.WriteString16(Str8ToStr16(FUNC_NAME));

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(skillArgs.get());

    data.WriteBool(true); // hasCallback
    // Write a null callback remote object
    data.WriteRemoteObject(nullptr);

    EXPECT_CALL(*stub_, ExecuteInAppSkillWithTokenId(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0200 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0100
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto token = new MockAbilityToken();
    data.WriteRemoteObject(token->AsObject());
    data.WriteString(REQUEST_CODE);
    data.WriteInt32(0); // resultCode

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.result->SetParam("answer", AAFwk::String::Box("done"));
    data.WriteParcelable(&result);

    EXPECT_CALL(*stub_, ExecuteSkillDone(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0200
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with null token
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0200 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteRemoteObject(nullptr); // null token

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN),
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0200 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_QuerySkillTypeInner_0100
 * @tc.desc: Test QuerySkillTypeInner with valid data
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_QuerySkillTypeInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_0100 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteString16(Str8ToStr16(BUNDLE_NAME));
    data.WriteString16(Str8ToStr16(MODULE_NAME));
    data.WriteString16(Str8ToStr16(SKILL_NAME));

    EXPECT_CALL(*stub_, QuerySkillType(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0300
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with null result (read fail)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0300 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto token = new MockAbilityToken();
    data.WriteRemoteObject(token->AsObject());
    data.WriteString(REQUEST_CODE);
    data.WriteInt32(0);

    // Write nothing for result, so ReadParcelable returns nullptr
    data.WriteParcelable(nullptr);

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN),
        data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0300 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0300
 * @tc.desc: Test ExecuteInAppSkillWithTokenIdInner with null skillArgs in parcel
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0300 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteUint32(100u);
    data.WriteString16(Str8ToStr16(BUNDLE_NAME));
    data.WriteString16(Str8ToStr16(MODULE_NAME));
    data.WriteString16(Str8ToStr16(SKILL_NAME));
    data.WriteString16(Str8ToStr16(SCRIPT_PATH));
    data.WriteString16(Str8ToStr16(FUNC_NAME));

    data.WriteParcelable(nullptr); // null skillArgs
    data.WriteBool(false);

    EXPECT_CALL(*stub_, ExecuteInAppSkillWithTokenId(_, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0300 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0400
 * @tc.desc: Test ExecuteSkillDoneWithTokenInner with error resultCode
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteSkillDoneWithTokenInner_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0400 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    auto token = new MockAbilityToken();
    data.WriteRemoteObject(token->AsObject());
    data.WriteString(REQUEST_CODE);
    data.WriteInt32(-1); // error resultCode

    AppExecFwk::SkillExecuteResult result;
    result.code = -1;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    EXPECT_CALL(*stub_, ExecuteSkillDone(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDoneWithTokenInner_0400 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_QuerySkillTypeInner_0200
 * @tc.desc: Test QuerySkillTypeInner returns error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_QuerySkillTypeInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_0200 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteString16(Str8ToStr16("com.invalid"));
    data.WriteString16(Str8ToStr16("invalid"));
    data.WriteString16(Str8ToStr16("invalidSkill"));

    EXPECT_CALL(*stub_, QuerySkillType(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillTypeInner_0200 end");
}

/**
 * @tc.name: AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0400
 * @tc.desc: Test ExecuteInAppSkillWithTokenIdInner with empty strings
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillStubTest,
    AbilityManagerSkillStub_ExecuteInAppSkillWithTokenIdInner_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0400 start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    WriteInterfaceToken(data);

    data.WriteUint32(0u); // zero tokenId
    data.WriteString16(Str8ToStr16("")); // empty bundleName
    data.WriteString16(Str8ToStr16("")); // empty moduleName
    data.WriteString16(Str8ToStr16("")); // empty skillName
    data.WriteString16(Str8ToStr16("")); // empty scriptPath
    data.WriteString16(Str8ToStr16("")); // empty functionName

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(skillArgs.get());
    data.WriteBool(false);

    EXPECT_CALL(*stub_, ExecuteInAppSkillWithTokenId(_, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));

    int res = stub_->OnRemoteRequest(
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID),
        data, reply, option);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenIdInner_0400 end");
}

}  // namespace AAFwk
}  // namespace OHOS
