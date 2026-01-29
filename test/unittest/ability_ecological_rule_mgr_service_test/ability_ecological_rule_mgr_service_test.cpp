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

#include "ability_manager_errors.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "message_parcel.h"
#include "iservice_registry.h"
#include "oh_mock_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace EcologicalRuleMgrService {
namespace {
constexpr int ERR_NONE = IAbilityEcologicalRuleMgrService::ERR_OK;

class FakeRemoteObject : public IRemoteObject {
public:
    explicit FakeRemoteObject(int sendResult = IAbilityEcologicalRuleMgrService::ERR_OK)
        : sendResult_(sendResult) {}

    int SendRequest(uint32_t, MessageParcel &, MessageParcel &, MessageOption &) override
    {
        return sendResult_;
    }

private:
    int sendResult_ = IAbilityEcologicalRuleMgrService::ERR_OK;
};

class FakeService : public IAbilityEcologicalRuleMgrService {
public:
    int32_t QueryStartExperience(const Want &, const AbilityCallerInfo &, AbilityExperienceRule &rule) override
    {
        rule = rule_;
        return result_;
    }

    int32_t EvaluateResolveInfos(const Want &, const AbilityCallerInfo &, int32_t,
        std::vector<AbilityInfo> &) override
    {
        return result_;
    }

    int32_t result_ = ERR_OK;
    AbilityExperienceRule rule_ {};
};
} // namespace

class AbilityEcologicalRuleMgrServiceTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetMessageParcelState();
        SetReadParcelableValue<AbilityExperienceRule>(sptr<AbilityExperienceRule>());
        g_mockMap.clear();
        AbilityEcologicalRuleMgrServiceClient::ecologicalRuleMgrServiceProxy_ = nullptr;
        AbilityEcologicalRuleMgrServiceClient::instance_ = nullptr;
    }
};

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0100
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write interface token failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0100,
    Function | MediumTest | Level1)
{
    MockWriteInterfaceToken(false);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0200
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write want failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0200,
    Function | MediumTest | Level1)
{
    MockWriteParcelable(true, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0300
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write type failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0300,
    Function | MediumTest | Level1)
{
    MockWriteInt32(true, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0400
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write abilityInfos size failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0400,
    Function | MediumTest | Level1)
{
    MockWriteInt32(false, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0500
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write abilityInfo failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0500,
    Function | MediumTest | Level1)
{
    MockWriteParcelable(false, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos(1);
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0600
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test write callerInfo failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0600,
    Function | MediumTest | Level1)
{
    MockWriteParcelable(false, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0700
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test remote is null.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0700,
    Function | MediumTest | Level1)
{
    AbilityEcologicalRuleMgrServiceProxy proxy(nullptr);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0800
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test SendRequest failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0800,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(-1);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_0900
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test ReadParcelableVector with oversize.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_0900,
    Function | MediumTest | Level1)
{
    MockReadInt32(1001);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_OK);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1000
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test ReadParcelableVector with null info.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1000,
    Function | MediumTest | Level1)
{
    MockReadInt32(1);
    MockReadParcelable(false);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_OK);
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1100
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test ReadParcelableVector success.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1100,
    Function | MediumTest | Level1)
{
    MockReadInt32(1);
    MockReadParcelable(true);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = proxy.EvaluateResolveInfos(want, callerInfo, 1, infos);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_OK);
    EXPECT_EQ(infos.size(), 1u);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1200
 * @tc.name: QueryStartExperience
 * @tc.desc: Test write interface token failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1200,
    Function | MediumTest | Level1)
{
    MockWriteInterfaceToken(false);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1300
 * @tc.name: QueryStartExperience
 * @tc.desc: Test write want failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1300,
    Function | MediumTest | Level1)
{
    MockWriteParcelable(true, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1400
 * @tc.name: QueryStartExperience
 * @tc.desc: Test write callerInfo failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1400,
    Function | MediumTest | Level1)
{
    MockWriteParcelable(false, 1);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1500
 * @tc.name: QueryStartExperience
 * @tc.desc: Test remote is null.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1500,
    Function | MediumTest | Level1)
{
    AbilityEcologicalRuleMgrServiceProxy proxy(nullptr);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1600
 * @tc.name: QueryStartExperience
 * @tc.desc: Test SendRequest failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1600,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(-1);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1700
 * @tc.name: QueryStartExperience
 * @tc.desc: Test read rule failed.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1700,
    Function | MediumTest | Level1)
{
    MockReadParcelable(false);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceProxy_1800
 * @tc.name: QueryStartExperience
 * @tc.desc: Test success path.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceProxy_1800,
    Function | MediumTest | Level1)
{
    MockReadParcelable(true);
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    AbilityEcologicalRuleMgrServiceProxy proxy(remote);
    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = proxy.QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_OK);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceClient_0100
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test connect service failed when samgr is null.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceClient_0100,
    Function | MediumTest | Level1)
{
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = AbilityEcologicalRuleMgrServiceClient::GetInstance()->EvaluateResolveInfos(
        want, callerInfo, 1, infos);
    EXPECT_EQ(ret, AAFwk::ERR_CONNECT_ERMS_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceClient_0200
 * @tc.name: EvaluateResolveInfos
 * @tc.desc: Test connect service failed when system ability is null.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceClient_0200,
    Function | MediumTest | Level1)
{
    sptr<ISystemAbilityManager> samgr = new (std::nothrow) ISystemAbilityManager();
    g_mockMap["SystemAbilityManagerClient_GetSystemAbilityManager_"] = { ResultWrap(samgr) };
    g_mockMap["ISystemAbilityManager_CheckSystemAbility_int32_t"] = { ResultWrap(sptr<IRemoteObject>(nullptr)) };
    Want want;
    AbilityCallerInfo callerInfo;
    std::vector<AbilityInfo> infos;
    auto ret = AbilityEcologicalRuleMgrServiceClient::GetInstance()->EvaluateResolveInfos(
        want, callerInfo, 1, infos);
    EXPECT_EQ(ret, AAFwk::ERR_CONNECT_ERMS_FAILED);
}

/**
 * @tc.number: AbilityEcologicalRuleMgrServiceClient_0300
 * @tc.name: QueryStartExperience
 * @tc.desc: Test replaceWant param is set when rule contains replaceWant.
 */
HWTEST_F(AbilityEcologicalRuleMgrServiceTest, AbilityEcologicalRuleMgrServiceClient_0300,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> remote = new (std::nothrow) FakeRemoteObject(ERR_NONE);
    sptr<ISystemAbilityManager> samgr = new (std::nothrow) ISystemAbilityManager();
    g_mockMap["SystemAbilityManagerClient_GetSystemAbilityManager_"] = { ResultWrap(samgr) };
    g_mockMap["ISystemAbilityManager_CheckSystemAbility_int32_t"] = { ResultWrap(remote) };

    auto replaceWant = sptr<Want>(new (std::nothrow) Want());
    auto rulePtr = sptr<AbilityExperienceRule>(new (std::nothrow) AbilityExperienceRule());
    rulePtr->replaceWant = replaceWant;
    MockReadParcelable(true);
    SetReadParcelableValue<AbilityExperienceRule>(rulePtr);

    Want want;
    AbilityCallerInfo callerInfo;
    AbilityExperienceRule rule;
    auto ret = AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(want, callerInfo, rule);
    EXPECT_EQ(ret, IAbilityEcologicalRuleMgrService::ERR_OK);
    ASSERT_NE(rule.replaceWant, nullptr);
    EXPECT_TRUE(rule.replaceWant->HasParameter(AbilityEcologicalRuleMgrServiceClient::ERMS_ORIGINAL_TARGET));
    EXPECT_EQ(rule.replaceWant->GetStringParam(AbilityEcologicalRuleMgrServiceClient::ERMS_ORIGINAL_TARGET),
        want.ToString());
}
} // namespace EcologicalRuleMgrService
} // namespace OHOS
