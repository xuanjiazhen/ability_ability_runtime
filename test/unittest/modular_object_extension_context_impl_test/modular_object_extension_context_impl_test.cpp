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

#include "modular_object_extension_context_impl.h"
#include "extension_context.h"

using namespace testing::ext;

namespace OHOS {
class MockRemoteObject : public IRemoteObject {
public:
    explicit MockRemoteObject(std::u16string desc) : IRemoteObject(desc) {}
    int GetObjectRefCount() override { return 1; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }
};
} // namespace OHOS

namespace OHOS {
namespace AbilityRuntime {

const size_t ModularObjectExtensionContext::CONTEXT_TYPE_ID =
    std::hash<const char*> {} ("ModularObjectExtensionContext");

} // namespace AbilityRuntime

namespace AAFwk {
ErrCode AbilityManagerClient::g_startSelfUIAbilityResult = ERR_OK;
ErrCode AbilityManagerClient::g_startSelfUIAbilityWithStartOptionsResult = ERR_OK;
ErrCode AbilityManagerClient::g_terminateResult = ERR_OK;
bool AbilityManagerClient::g_terminateCalled = false;
IRemoteObject *AbilityManagerClient::g_lastToken = nullptr;
} // namespace AAFwk

namespace AbilityRuntime {

class ModularObjectExtensionContextImplTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        AAFwk::AbilityManagerClient::Reset();
    }
    void TearDown() override {}
};

// ==================== StartSelfUIAbility ====================

HWTEST_F(ModularObjectExtensionContextImplTest, StartSelfUIAbility_Success_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    AAFwk::Want want;
    auto ret = context->StartSelfUIAbility(want);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(ModularObjectExtensionContextImplTest, StartSelfUIAbility_Error_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    AAFwk::AbilityManagerClient::g_startSelfUIAbilityResult = -1;
    AAFwk::Want want;
    auto ret = context->StartSelfUIAbility(want);
    EXPECT_EQ(ret, -1);
}

// ==================== StartSelfUIAbilityWithStartOptions ====================

HWTEST_F(ModularObjectExtensionContextImplTest, StartSelfUIAbilityWithStartOptions_Success_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    AAFwk::Want want;
    AAFwk::StartOptions options;
    auto ret = context->StartSelfUIAbilityWithStartOptions(want, options);
    EXPECT_EQ(ret, ERR_OK);
}

HWTEST_F(ModularObjectExtensionContextImplTest, StartSelfUIAbilityWithStartOptions_Error_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    AAFwk::AbilityManagerClient::g_startSelfUIAbilityWithStartOptionsResult = -2;
    AAFwk::Want want;
    AAFwk::StartOptions options;
    auto ret = context->StartSelfUIAbilityWithStartOptions(want, options);
    EXPECT_EQ(ret, -2);
}

// ==================== TerminateSelf ====================

HWTEST_F(ModularObjectExtensionContextImplTest, TerminateSelf_Success_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(AAFwk::AbilityManagerClient::g_terminateCalled);
}

HWTEST_F(ModularObjectExtensionContextImplTest, TerminateSelf_Error_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    AAFwk::AbilityManagerClient::g_terminateResult = -3;
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, -3);
}

HWTEST_F(ModularObjectExtensionContextImplTest, TerminateSelf_TokenPassed_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    sptr<IRemoteObject> token = sptr<IRemoteObject>(new MockRemoteObject(u"test_token"));
    context->token_ = token;
    auto ret = context->TerminateSelf();
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(AAFwk::AbilityManagerClient::g_lastToken, token.GetRefPtr());
}

// ==================== CONTEXT_TYPE_ID ====================

HWTEST_F(ModularObjectExtensionContextImplTest, ContextTypeId_NonZero_001, TestSize.Level1)
{
    EXPECT_NE(ModularObjectExtensionContext::CONTEXT_TYPE_ID, static_cast<size_t>(0));
}

HWTEST_F(ModularObjectExtensionContextImplTest, IsContext_SelfType_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    EXPECT_TRUE(context->IsContext(ModularObjectExtensionContext::CONTEXT_TYPE_ID));
}

HWTEST_F(ModularObjectExtensionContextImplTest, IsContext_InvalidType_001, TestSize.Level1)
{
    auto context = std::make_shared<ModularObjectExtensionContext>();
    EXPECT_FALSE(context->IsContext(0));
    EXPECT_FALSE(context->IsContext(99999));
}

} // namespace AbilityRuntime
} // namespace OHOS
