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

#include "ability_schedule_stub_mock.h"
#include "skill/skill_execute_param.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class AbilitySchedulerStubSkillTest : public testing::Test {
public:
    void SetUp() override
    {
        stub_ = new AbilitySchedulerStubMock();
    }
    void TearDown() override {}
    void WriteInterfaceToken(MessageParcel &data)
    {
        data.WriteInterfaceToken(AbilitySchedulerStub::GetDescriptor());
    }

    sptr<AbilitySchedulerStubMock> stub_;
};

/**
 * @tc.name: ExecuteSkillInner_001
 * @tc.desc: Normal Want triggers ExecuteSkill, returns NO_ERROR
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSkillTest, ExecuteSkillInner_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    want.SetElementName("com.example", "MainAbility");
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);

    auto res = stub_->OnRemoteRequest(
        IAbilityScheduler::SCHEDULE_EXECUTE_SKILL, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

/**
 * @tc.name: ExecuteSkillInner_002
 * @tc.desc: Null Want returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSkillTest, ExecuteSkillInner_002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    // Write null Want by not writing any Parcelable
    data.WriteParcelable(static_cast<Want *>(nullptr));

    auto res = stub_->OnRemoteRequest(
        IAbilityScheduler::SCHEDULE_EXECUTE_SKILL, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnRemoteRequest_ExecuteSkill_001
 * @tc.desc: Verify routing via SCHEDULE_EXECUTE_SKILL code
 * @tc.type: FUNC
 */
HWTEST_F(AbilitySchedulerStubSkillTest, OnRemoteRequest_ExecuteSkill_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    Want want;
    AppExecFwk::SkillExecuteParam::WriteToWant(want, "com.example", "entry",
        "testSkill", "path", "func", nullptr, {}, "req001", "");
    WriteInterfaceToken(data);
    data.WriteParcelable(&want);

    auto res = stub_->OnRemoteRequest(
        IAbilityScheduler::SCHEDULE_EXECUTE_SKILL, data, reply, option);
    EXPECT_EQ(res, NO_ERROR);
}

} // namespace AAFwk
} // namespace OHOS
