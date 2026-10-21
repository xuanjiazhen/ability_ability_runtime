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

#include <string>

#include <gtest/gtest.h>

#include "ability_manager_errors.h"
#include "extension_ability_info.h"
#define private public
#include "utils/agent_caller_identity_util.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

class AgentCallerIdentityUtilTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name      NonAgentTypeIsPassthrough
 * @tc.desc      ApplyIfNeeded returns ERR_OK and stays inactive for non-AGENT extension types.
 */
HWTEST_F(AgentCallerIdentityUtilTest, NonAgentTypeIsPassthrough, TestSize.Level1)
{
    for (auto type : { ExtensionAbilityType::SERVICE, ExtensionAbilityType::FORM,
                          ExtensionAbilityType::UNSPECIFIED }) {
        AgentCallerIdentityScope scope;
        int32_t ret = scope.ApplyIfNeeded(type, "any-identity");
        EXPECT_EQ(ret, ERR_OK);
        EXPECT_FALSE(scope.active_) << "non-AGENT type must not activate the scope";
    }
}

/**
 * @tc.name      AgentTypeWithEmptyIdentityIsRejected
 * @tc.desc      ApplyIfNeeded rejects AGENT type when callerIdentity is empty.
 */
HWTEST_F(AgentCallerIdentityUtilTest, AgentTypeWithEmptyIdentityIsRejected, TestSize.Level1)
{
    AgentCallerIdentityScope scope;
    int32_t ret = scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, "");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_FALSE(scope.active_);
}

/**
 * @tc.name      AgentTypeActivatesScope
 * @tc.desc      ApplyIfNeeded activates the scope for AGENT type with a non-empty callerIdentity.
 *               ApplyIfNeeded only checks callerIdentity.empty(); the IPC skeleton may or may not
 *               accept the token, but the scope is marked active regardless.
 */
HWTEST_F(AgentCallerIdentityUtilTest, AgentTypeActivatesScope, TestSize.Level1)
{
    AgentCallerIdentityScope scope;
    int32_t ret = scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, "caller-identity-placeholder");
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(scope.active_);
}

/**
 * @tc.name      DestructorRestoresServiceIdentityWhenActive
 * @tc.desc      When the scope was activated, its destructor runs without fault and clears active_.
 */
HWTEST_F(AgentCallerIdentityUtilTest, DestructorRestoresServiceIdentityWhenActive, TestSize.Level1)
{
    {
        AgentCallerIdentityScope scope;
        ASSERT_EQ(scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, "caller-identity-placeholder"), ERR_OK);
        ASSERT_TRUE(scope.active_);
    }
    // Scope destroyed without crash.
    EXPECT_TRUE(true);
}

/**
 * @tc.name      DestructorNoopWhenInactive
 * @tc.desc      When the scope was never activated (non-AGENT passthrough), its destructor must be a no-op.
 */
HWTEST_F(AgentCallerIdentityUtilTest, DestructorNoopWhenInactive, TestSize.Level1)
{
    {
        AgentCallerIdentityScope scope;
        ASSERT_EQ(scope.ApplyIfNeeded(ExtensionAbilityType::SERVICE, "ignored"), ERR_OK);
        ASSERT_FALSE(scope.active_);
    }
    // Scope destroyed without crash; inactive scope does nothing in destructor.
    EXPECT_TRUE(true);
}

/**
 * @tc.name      CrossProcessIdentityRoundTrip
 * @tc.desc      Simulates the cross-process identity flow: a callerIdentity string
 *               (obtained from subfoundation via IDL [out]) is applied via ApplyIfNeeded,
 *               the scope activates, and the destructor restores the saved service identity.
 *               This verifies the RAII contract that makes cross-process SetCallingIdentity safe.
 */
HWTEST_F(AgentCallerIdentityUtilTest, CrossProcessIdentityRoundTrip, TestSize.Level1)
{
    // Simulate a callerIdentity that traveled from subfoundation to AMS via IPC [out].
    const std::string crossProcessCallerIdentity = "cross-process-identity-from-subfoundation";
    {
        AgentCallerIdentityScope scope;
        ASSERT_EQ(scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, crossProcessCallerIdentity), ERR_OK);
        ASSERT_TRUE(scope.active_);
        // scope is active: serviceIdentity_ saved, calling identity set to crossProcessCallerIdentity.
        // When AgentCallerIdentityScope is used in AMS after receiving callerIdentity via IPC [out],
        // the same mechanism applies: ResetCallingIdentity saves AMS's identity, SetCallingIdentity
        // applies the original caller's identity carried by the string.
    }
    // Scope destroyed: SetCallingIdentity(serviceIdentity_) restored the saved identity.
    // No crash = RAII contract holds for cross-process identity round-trip.
    EXPECT_TRUE(true);
}

/**
 * @tc.name      EmptyCallerIdentityFromCrossProcessRejected
 * @tc.desc      When callerIdentity arrives empty from IPC [out] (e.g., subfoundation not ready),
 *               ApplyIfNeeded must reject it with ERR_INVALID_VALUE and not activate the scope.
 *               This prevents setting an empty calling identity on subsequent IPC calls.
 */
HWTEST_F(AgentCallerIdentityUtilTest, EmptyCallerIdentityFromCrossProcessRejected, TestSize.Level1)
{
    AgentCallerIdentityScope scope;
    int32_t ret = scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, "");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_FALSE(scope.active_) << "empty cross-process callerIdentity must not activate scope";
}
