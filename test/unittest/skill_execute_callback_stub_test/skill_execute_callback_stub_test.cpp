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

#include "skill_execute_callback_stub_mock.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class SkillExecuteCallbackStubTest : public testing::Test {
public:
    void SetUp() override
    {
        stub_ = new SkillExecuteCallbackStubMock();
    }
    void TearDown() override {}
    void WriteInterfaceToken(MessageParcel &data)
    {
        data.WriteInterfaceToken(ISkillExecuteCallback::GetDescriptor());
    }
    sptr<SkillExecuteCallbackStubMock> stub_;
};

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Valid code with correct token invokes OnExecuteDone
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    data.WriteString("req001");
    data.WriteInt32(0);
    AppExecFwk::SkillExecuteResult result;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    EXPECT_CALL(*stub_, OnExecuteDone(_, _, _)).Times(1);
    auto res = stub_->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: Wrong interface token returns ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"wrong.token");
    data.WriteString("req001");
    data.WriteInt32(0);

    auto res = stub_->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_STATE);
}

/**
 * @tc.name: OnRemoteRequest_003
 * @tc.desc: Unknown code falls through to base class
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnRemoteRequest_003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    uint32_t unknownCode = 9999;
    auto res = stub_->OnRemoteRequest(unknownCode, data, reply, option);
    EXPECT_NE(res, ERR_OK);
}

/**
 * @tc.name: OnExecuteDoneInner_001
 * @tc.desc: Valid data passes correct requestCode, resultCode, result
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnExecuteDoneInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString("req002");
    data.WriteInt32(100);
    AppExecFwk::SkillExecuteResult result;
    result.code = 42;
    result.result = std::make_shared<AAFwk::WantParams>();
    data.WriteParcelable(&result);

    MessageOption option;
    EXPECT_CALL(*stub_, OnExecuteDone(StrEq("req002"), Eq(100), _)).Times(1);
    auto res = stub_->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: OnExecuteDoneInner_002
 * @tc.desc: Null result parcel returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteCallbackStubTest, OnExecuteDoneInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteString("req003");
    data.WriteInt32(0);
    // No SkillExecuteResult written

    MessageOption option;
    auto res = stub_->OnRemoteRequest(ISkillExecuteCallback::ON_SKILL_EXECUTE_DONE, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

} // namespace AAFwk
} // namespace OHOS
