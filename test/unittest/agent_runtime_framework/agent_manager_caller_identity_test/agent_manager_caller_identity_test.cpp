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

#include "agent_manager_caller_identity.h"
#include "ipc_skeleton.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;

namespace {
// Reads the active calling identity. ResetCallingIdentity() also clears it, so the
// helper restores the value before returning.
std::string ReadCallingIdentity()
{
    std::string id = IPCSkeleton::ResetCallingIdentity();
    IPCSkeleton::SetCallingIdentity(id);
    return id;
}

void SetCallingIdentityMutable(const std::string &id)
{
    std::string mutableId = id;
    IPCSkeleton::SetCallingIdentity(mutableId);
}
}  // namespace

class AgentManagerCallerIdentityTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name      ScopeCapturesOriginalIdentity
 * @tc.desc      Constructing AgentManagerCallerIdentityScope captures the active IPC identity
 *               and exposes it via GetOriginalIdentity().
 */
HWTEST_F(AgentManagerCallerIdentityTest, ScopeCapturesOriginalIdentity, TestSize.Level1)
{
    const std::string before = ReadCallingIdentity();
    {
        AgentManagerCallerIdentityScope scope;
        const std::string &captured = scope.GetOriginalIdentity();
        EXPECT_FALSE(captured.empty());
        EXPECT_EQ(captured, before);
    }
}

/**
 * @tc.name      ScopeRestoresIdentityOnDestruction
 * @tc.desc      When the scope exits, the IPC identity that was active at construction
 *               must be restored even if inner code mutated it.
 */
HWTEST_F(AgentManagerCallerIdentityTest, ScopeRestoresIdentityOnDestruction, TestSize.Level1)
{
    const std::string original = ReadCallingIdentity();
    {
        AgentManagerCallerIdentityScope scope;
        SetCallingIdentityMutable("caller-identity-token");
        // Inside the scope, a mutated identity is visible.
        EXPECT_EQ(ReadCallingIdentity(), "caller-identity-token");
    }
    // After the scope exits, the identity captured at construction (== original) is restored.
    EXPECT_EQ(ReadCallingIdentity(), original);
}

/**
 * @tc.name      NestedScopesRestoreInLifoOrder
 * @tc.desc      Nested scopes must restore identities in LIFO order so that inner-scope
 *               mutations do not leak past the outer scope.
 */
HWTEST_F(AgentManagerCallerIdentityTest, NestedScopesRestoreInLifoOrder, TestSize.Level1)
{
    const std::string outer = ReadCallingIdentity();
    {
        AgentManagerCallerIdentityScope outerScope;
        SetCallingIdentityMutable("inner-marker");
        {
            AgentManagerCallerIdentityScope innerScope;
            EXPECT_EQ(innerScope.GetOriginalIdentity(), "inner-marker");
            SetCallingIdentityMutable("deepest");
        }
        // After the inner scope exits, the inner marker is restored.
        EXPECT_EQ(ReadCallingIdentity(), "inner-marker");
    }
    // After the outer scope exits, the original identity is restored.
    EXPECT_EQ(ReadCallingIdentity(), outer);
}
