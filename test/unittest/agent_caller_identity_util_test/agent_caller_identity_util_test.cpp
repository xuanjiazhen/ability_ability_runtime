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
#include "ipc_skeleton.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace {
// Captures a real calling-identity token from the IPC skeleton to use as a valid
// callerIdentity argument. SetCallingIdentity rejects strings that are not in the
// skeleton's internal "PID UID TOKEN" format, so we must pass a real token.
std::string CaptureRealIdentity()
{
    std::string id = IPCSkeleton::ResetCallingIdentity();
    IPCSkeleton::SetCallingIdentity(id);
    return id;
}

void RestoreIdentity(const std::string &id)
{
    std::string mutableId = id;
    IPCSkeleton::SetCallingIdentity(mutableId);
}
}  // namespace

class AgentCallerIdentityUtilTest : public testing::Test {
public:
    void SetUp() override
    {
        savedIdentity_ = CaptureRealIdentity();
    }
    void TearDown() override
    {
        RestoreIdentity(savedIdentity_);
    }

private:
    std::string savedIdentity_;
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
    const std::string before = CaptureRealIdentity();
    AgentCallerIdentityScope scope;
    int32_t ret = scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, "");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_FALSE(scope.active_);
    // Identity should not have changed.
    std::string after = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(after);
    EXPECT_EQ(after, before);
}

/**
 * @tc.name      AgentTypeActivatesScope
 * @tc.desc      ApplyIfNeeded activates the scope and installs the caller identity for AGENT type.
 */
HWTEST_F(AgentCallerIdentityUtilTest, AgentTypeActivatesScope, TestSize.Level1)
{
    // Use a real identity token so SetCallingIdentity accepts the format.
    const std::string realIdentity = CaptureRealIdentity();
    AgentCallerIdentityScope scope;
    int32_t ret = scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, realIdentity);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(scope.active_);
    std::string current = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(current);
    EXPECT_EQ(current, realIdentity);
}

/**
 * @tc.name      DestructorRestoresServiceIdentityWhenActive
 * @tc.desc      When the scope was activated, its destructor restores the service identity
 *               captured at activation time.
 */
HWTEST_F(AgentCallerIdentityUtilTest, DestructorRestoresServiceIdentityWhenActive, TestSize.Level1)
{
    const std::string serviceIdentity = CaptureRealIdentity();
    const std::string callerIdentity = CaptureRealIdentity();
    {
        AgentCallerIdentityScope scope;
        ASSERT_EQ(scope.ApplyIfNeeded(ExtensionAbilityType::AGENT, callerIdentity), ERR_OK);
        ASSERT_TRUE(scope.active_);
    }
    // After scope destruction, the service identity captured at activation is restored.
    std::string restored = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(restored);
    EXPECT_EQ(restored, serviceIdentity);
}

/**
 * @tc.name      DestructorNoopWhenInactive
 * @tc.desc      When the scope was never activated (non-AGENT passthrough), its destructor must not
 *               mutate the calling identity.
 */
HWTEST_F(AgentCallerIdentityUtilTest, DestructorNoopWhenInactive, TestSize.Level1)
{
    const std::string staged = CaptureRealIdentity();
    RestoreIdentity(staged);
    {
        AgentCallerIdentityScope scope;
        ASSERT_EQ(scope.ApplyIfNeeded(ExtensionAbilityType::SERVICE, "ignored"), ERR_OK);
        ASSERT_FALSE(scope.active_);
    }
    // The staged identity is preserved because the scope was inactive (destructor is a no-op).
    std::string after = IPCSkeleton::ResetCallingIdentity();
    RestoreIdentity(after);
    EXPECT_EQ(after, staged);
}
