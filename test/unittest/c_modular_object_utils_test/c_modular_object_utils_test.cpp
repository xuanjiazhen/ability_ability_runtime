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
#include <gmock/gmock.h>

#include <cstring>
#include <memory>
#include <string>

#include "ability_manager_errors.h"
#include "ability_runtime_common.h"
#include "c_modular_object_utils.h"
#include "element_name.h"
#include "iremote_object.h"
#include "iremote_broker.h"
#include "message_parcel.h"
#include "message_option.h"
#include "mock_context_base.h"
#include "mock_my_flag.h"
#include "native_extension/context_impl.h"
#include "want_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

// Complete the forward-declared AbilityBase_Want at global scope
struct AbilityBase_Want {
    AbilityBase_Element element;
    int dummy = 0;
};

namespace OHOS {
namespace AbilityRuntime {
namespace {

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"mock_descriptor") {}
    ~MockRemoteObject() = default;

    int32_t GetObjectRefCount() override { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        (void)code;
        (void)data;
        (void)reply;
        (void)option;
        return 0;
    }
    bool IsProxyObject() const override { return true; }
    bool CheckObjectLegality() const override { return true; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override
    {
        (void)recipient;
        return true;
    }
    bool Marshalling(Parcel &parcel) const override
    {
        (void)parcel;
        return true;
    }
    sptr<IRemoteBroker> AsInterface() override { return nullptr; }
    int Dump(int fd, const std::vector<std::u16string> &args) override
    {
        (void)fd;
        (void)args;
        return 0;
    }
    std::u16string GetObjectDescriptor() const { return std::u16string(); }
};

} // namespace

class CModularObjectUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override
    {
        MyFlag::retCheckWant = 0;
        MyFlag::retTransformToWant = 0;
        MyFlag::retConvertToCommonBusinessErrorCode = 0;
    }
    void TearDown() override {}
};

// ==================== ConvertConnectBusinessErrorCode ====================

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_001, TestSize.Level1)
{
    // ABILITY_VISIBLE_FALSE_DENY_REQUEST -> VISIBILITY_VERIFICATION_FAILED
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ABILITY_VISIBLE_FALSE_DENY_REQUEST);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_VISIBILITY_VERIFICATION_FAILED);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_002, TestSize.Level1)
{
    // ERR_STATIC_CFG_PERMISSION -> STATIC_CFG_PERMISSION
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_STATIC_CFG_PERMISSION);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_STATIC_CFG_PERMISSION);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_003, TestSize.Level1)
{
    // ERR_CROSS_USER -> CROSS_USER_OPERATION
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_CROSS_USER);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CROSS_USER_OPERATION);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_004, TestSize.Level1)
{
    // ERR_CHECK_CALL_FROM_BACKGROUND_FAILED -> NO_RUNNING_ABILITIES_WITH_UI
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_CHECK_CALL_FROM_BACKGROUND_FAILED);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_005, TestSize.Level1)
{
    // ERR_FREQ_START_ABILITY -> UPPER_RATE_LIMIT
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_FREQ_START_ABILITY);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_UPPER_RATE_LIMIT);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_006, TestSize.Level1)
{
    // ERR_REACH_UPPER_LIMIT -> UPPER_CONNECTION_NUMBER_LIMIT
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_REACH_UPPER_LIMIT);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_UPPER_CONNECTION_NUMBER_LIMIT);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_007, TestSize.Level1)
{
    // ERR_UPPER_LIMIT -> UPPER_CONNECTION_NUMBER_LIMIT (same as ERR_REACH_UPPER_LIMIT)
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_UPPER_LIMIT);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_UPPER_CONNECTION_NUMBER_LIMIT);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_008, TestSize.Level1)
{
    // ERR_MODULAR_OBJECT_DISABLED -> MODULAR_OBJECT_EXTENSION_DISABLED
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_MODULAR_OBJECT_EXTENSION_DISABLED);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_009, TestSize.Level1)
{
    // ERR_NO_RUNNING_ABILITIES_WITH_UI -> NO_RUNNING_ABILITIES_WITH_UI
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(ERR_NO_RUNNING_ABILITIES_WITH_UI);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_010, TestSize.Level1)
{
    // default case -> calls ConvertToCommonBusinessErrorCode
    MyFlag::retConvertToCommonBusinessErrorCode = ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED;
    int32_t unknownErrCode = 9999999;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(unknownErrCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
}

HWTEST_F(CModularObjectUtilsTest, ConvertConnectBusinessErrorCode_011, TestSize.Level1)
{
    // default case with mock returning default value
    MyFlag::retConvertToCommonBusinessErrorCode = 0;
    int32_t unknownErrCode = 1;
    auto ret = CModularObjectUtils::ConvertConnectBusinessErrorCode(unknownErrCode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
}

// ==================== CopyToCString ====================

HWTEST_F(CModularObjectUtilsTest, CopyToCString_001, TestSize.Level1)
{
    // success: normal string
    char *dst = nullptr;
    std::string src = "hello";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, "hello");
    delete[] dst;
}

HWTEST_F(CModularObjectUtilsTest, CopyToCString_002, TestSize.Level1)
{
    // success: empty string
    char *dst = nullptr;
    std::string src = "";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, "");
    delete[] dst;
}

HWTEST_F(CModularObjectUtilsTest, CopyToCString_003, TestSize.Level1)
{
    // success: string with special characters
    char *dst = nullptr;
    std::string src = "com.example.test/Module:Ability";
    bool ret = CModularObjectUtils::CopyToCString(src, dst);
    EXPECT_TRUE(ret);
    ASSERT_NE(dst, nullptr);
    EXPECT_STREQ(dst, src.c_str());
    delete[] dst;
}

// ==================== BuildElement ====================

HWTEST_F(CModularObjectUtilsTest, BuildElement_001, TestSize.Level1)
{
    // success: all fields populated
    ElementName elementName("", "com.test.bundle", "com.test.module", "TestAbility");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = CModularObjectUtils::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    ASSERT_NE(element.bundleName, nullptr);
    EXPECT_STREQ(element.bundleName, "com.test.bundle");
    ASSERT_NE(element.moduleName, nullptr);
    EXPECT_STREQ(element.moduleName, "com.test.module");
    ASSERT_NE(element.abilityName, nullptr);
    EXPECT_STREQ(element.abilityName, "TestAbility");
    CModularObjectUtils::DestroyElement(element);
}

HWTEST_F(CModularObjectUtilsTest, BuildElement_002, TestSize.Level1)
{
    // success: fields with empty strings
    ElementName elementName("", "", "", "");
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    bool ret = CModularObjectUtils::BuildElement(elementName, element);
    EXPECT_TRUE(ret);
    ASSERT_NE(element.bundleName, nullptr);
    EXPECT_STREQ(element.bundleName, "");
    ASSERT_NE(element.moduleName, nullptr);
    EXPECT_STREQ(element.moduleName, "");
    ASSERT_NE(element.abilityName, nullptr);
    EXPECT_STREQ(element.abilityName, "");
    CModularObjectUtils::DestroyElement(element);
}

// ==================== DestroyElement ====================

HWTEST_F(CModularObjectUtilsTest, DestroyElement_001, TestSize.Level1)
{
    // destroy populated element
    AbilityBase_Element element;
    element.bundleName = new char[5];
    strcpy(element.bundleName, "test");
    element.moduleName = new char[4];
    strcpy(element.moduleName, "mod");
    element.abilityName = new char[4];
    strcpy(element.abilityName, "abc");

    CModularObjectUtils::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
}

HWTEST_F(CModularObjectUtilsTest, DestroyElement_002, TestSize.Level1)
{
    // destroy element with null fields (no crash)
    AbilityBase_Element element = {nullptr, nullptr, nullptr};
    CModularObjectUtils::DestroyElement(element);
    EXPECT_EQ(element.bundleName, nullptr);
    EXPECT_EQ(element.moduleName, nullptr);
    EXPECT_EQ(element.abilityName, nullptr);
}

// ==================== NotifyFailed ====================

// Global callback tracking for NotifyFailed tests
namespace {
OH_AbilityRuntime_ConnectOptions *g_capturedOwner = nullptr;
AbilityRuntime_ErrorCode g_capturedErrorCode = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
int32_t g_callbackCallCount = 0;

void ResetCallbackState()
{
    g_capturedOwner = nullptr;
    g_capturedErrorCode = ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    g_callbackCallCount = 0;
}

void MockOnFailedCallback(OH_AbilityRuntime_ConnectOptions *owner, AbilityRuntime_ErrorCode code)
{
    g_capturedOwner = owner;
    g_capturedErrorCode = code;
    g_callbackCallCount++;
}
} // namespace

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_001, TestSize.Level1)
{
    // state is nullptr -> early return, no crash
    ResetCallbackState();
    CModularObjectUtils::NotifyFailed(nullptr, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_002, TestSize.Level1)
{
    // state alive is false -> early return
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = false;
    state->onFailedCallback = MockOnFailedCallback;
    CModularObjectUtils::NotifyFailed(state, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_003, TestSize.Level1)
{
    // callback is nullptr -> no call
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->onFailedCallback = nullptr;
    CModularObjectUtils::NotifyFailed(state, ERR_MODULAR_OBJECT_DISABLED);
    EXPECT_EQ(g_callbackCallCount, 0);
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_004, TestSize.Level1)
{
    // callback is non-null -> callback invoked with correct error code
    ResetCallbackState();
    OH_AbilityRuntime_ConnectOptions owner;
    owner.state = nullptr;

    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = &owner;
    state->onFailedCallback = MockOnFailedCallback;

    CModularObjectUtils::NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
    EXPECT_EQ(g_callbackCallCount, 1);
    EXPECT_EQ(g_capturedOwner, &owner);
    EXPECT_EQ(g_capturedErrorCode, ABILITY_RUNTIME_ERROR_CODE_PERMISSION_DENIED);
}

HWTEST_F(CModularObjectUtilsTest, NotifyFailed_005, TestSize.Level1)
{
    // verify error code is cast correctly from int32_t
    ResetCallbackState();
    auto state = std::make_shared<OH_AbilityRuntime_ConnectOptionsState>();
    state->alive = true;
    state->owner = nullptr;
    state->onFailedCallback = MockOnFailedCallback;

    int32_t businessCode = ABILITY_RUNTIME_ERROR_CODE_MODULAR_OBJECT_EXTENSION_DISABLED;
    CModularObjectUtils::NotifyFailed(state, businessCode);
    EXPECT_EQ(g_callbackCallCount, 1);
    EXPECT_EQ(g_capturedErrorCode, ABILITY_RUNTIME_ERROR_CODE_MODULAR_OBJECT_EXTENSION_DISABLED);
}

// ==================== TransformWant ====================

HWTEST_F(CModularObjectUtilsTest, TransformWant_001, TestSize.Level1)
{
    // CheckWant returns error -> propagate error
    MyFlag::retCheckWant = ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(CModularObjectUtilsTest, TransformWant_002, TestSize.Level1)
{
    // CheckWant succeeds, TransformToWant returns error
    MyFlag::retCheckWant = 0;
    MyFlag::retTransformToWant = ABILITY_BASE_ERROR_CODE_PARAM_INVALID;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(CModularObjectUtilsTest, TransformWant_003, TestSize.Level1)
{
    // success path
    MyFlag::retCheckWant = 0;
    MyFlag::retTransformToWant = 0;
    AbilityBase_Want want;
    want.element = {nullptr, nullptr, nullptr};
    AAFwk::Want abilityWant;
    auto ret = CModularObjectUtils::TransformWant(reinterpret_cast<AbilityBase_Want *>(&want), abilityWant);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// ==================== CheckContextAndToken ====================

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_001, TestSize.Level1)
{
    // context is nullptr
    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(nullptr, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_002, TestSize.Level1)
{
    // context->context.lock() returns nullptr (expired weak_ptr)
    AbilityRuntime_Context context;
    context.type = 0;
    // weak_ptr is default-constructed (expired)
    context.context = std::weak_ptr<ContextBase>();

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_003, TestSize.Level1)
{
    // GetToken returns nullptr
    auto mockContext = std::make_shared<MockContext>();
    EXPECT_CALL(*mockContext, GetToken()).Times(1).WillOnce(Return(sptr<IRemoteObject>(nullptr)));

    AbilityRuntime_Context context;
    context.type = 0;
    context.context = mockContext;

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
}

HWTEST_F(CModularObjectUtilsTest, CheckContextAndToken_004, TestSize.Level1)
{
    // success path: GetToken returns valid token
    sptr<IRemoteObject> expectedToken = sptr<IRemoteObject>(new (std::nothrow) MockRemoteObject());
    ASSERT_NE(expectedToken, nullptr);
    auto mockContext = std::make_shared<MockContext>();
    EXPECT_CALL(*mockContext, GetToken()).Times(1).WillOnce(Return(expectedToken));

    AbilityRuntime_Context context;
    context.type = 0;
    context.context = mockContext;

    sptr<IRemoteObject> token;
    auto ret = CModularObjectUtils::CheckContextAndToken(&context, token);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(token.GetRefPtr(), expectedToken.GetRefPtr());
}

} // namespace AbilityRuntime
} // namespace OHOS
