/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <cstring>
#include <vector>

#include "ets_context_utils.h"
#include "ani_base_context.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
namespace {

// Minimal ANI mock framework for testing ets_context_utils and ani_base_context.
// Only the function pointers actually used by the code under test are populated.

struct MockAniApi {
    // --- State tracking for test assertions ---
    int deleteWeakPtrCallCount = 0;
    int deleteEtsBaseContextCallCount = 0;
    std::vector<ani_long> setFieldValues; // records values set via SetFieldByName_Long

    // --- Configurable return values and side effects ---
    ani_status findClassStatus = ANI_OK;
    ani_status classFindFieldStatus = ANI_OK;
    ani_status classFindMethodStatus = ANI_OK;
    ani_status objectGetFieldLongStatus = ANI_OK;
    ani_status objectGetFieldByNameLongStatus = ANI_OK;
    ani_status objectSetFieldByNameLongStatus = ANI_OK;
    ani_status objectCallMethodVoidVStatus = ANI_OK;

    ani_long objectGetFieldLongValue = 0;
    // Per-field return values for Object_GetFieldByName_Long (takes precedence over default).
    // Fields not set here return objectGetFieldByNameLongValue.
    ani_long objectGetFieldByNameLong_PtrValue = -1;       // for "ptr" field; -1 = use default
    ani_long objectGetFieldByNameLong_NativePtrValue = -1; // for "nativePtr" field; -1 = use default
    ani_long objectGetFieldByNameLongValue = 0;
    ani_class findClassResult = nullptr;
    ani_field classFindFieldResult = nullptr;
    ani_method classFindMethodResult = nullptr;

    // Call tracking
    int findClassCallCount = 0;
    int classFindMethodCallCount = 0;
    int objectCallMethodVoidVCallCount = 0;
    int objectSetFieldByNameLongCallCount = 0;
    int objectGetFieldByNameLongCallCount = 0;
    const char *lastSetFieldName = nullptr;
    const char *lastGetFieldName = nullptr;
    const char *lastFindMethodName = nullptr;
};

// ----- Static mock function implementations -----
// These call through to a global or TLS mock state pointer.
static MockAniApi *g_mockApi = nullptr;

static void SetMockApi(MockAniApi *api) { g_mockApi = api; }

static ani_status MockFindClass(ani_env *env, const char *name, ani_class *cls)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    g_mockApi->findClassCallCount++;
    if (g_mockApi->findClassStatus != ANI_OK) return g_mockApi->findClassStatus;
    if (cls != nullptr) *cls = g_mockApi->findClassResult;
    return ANI_OK;
}

static ani_status MockClassFindField(ani_env *env, ani_class cls, const char *name, ani_field *field)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    if (g_mockApi->classFindFieldStatus != ANI_OK) return g_mockApi->classFindFieldStatus;
    if (field != nullptr) *field = g_mockApi->classFindFieldResult;
    return ANI_OK;
}

static ani_status MockClassFindMethod(ani_env *env, ani_class cls, const char *name,
    const char *signature, ani_method *method)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    g_mockApi->classFindMethodCallCount++;
    g_mockApi->lastFindMethodName = name;
    if (g_mockApi->classFindMethodStatus != ANI_OK) return g_mockApi->classFindMethodStatus;
    if (method != nullptr) *method = g_mockApi->classFindMethodResult;
    return ANI_OK;
}

static ani_status MockObjectGetFieldLong(ani_env *env, ani_object obj, ani_field field, ani_long *result)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    if (g_mockApi->objectGetFieldLongStatus != ANI_OK) return g_mockApi->objectGetFieldLongStatus;
    if (result != nullptr) *result = g_mockApi->objectGetFieldLongValue;
    return ANI_OK;
}

static ani_status MockObjectGetFieldByNameLong(ani_env *env, ani_object object, const char *name,
    ani_long *result)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    g_mockApi->objectGetFieldByNameLongCallCount++;
    g_mockApi->lastGetFieldName = name;
    if (g_mockApi->objectGetFieldByNameLongStatus != ANI_OK)
        return g_mockApi->objectGetFieldByNameLongStatus;
    if (result != nullptr) {
        // Honor per-field overrides first.
        if (strcmp(name, "ptr") == 0 && g_mockApi->objectGetFieldByNameLong_PtrValue >= 0) {
            *result = g_mockApi->objectGetFieldByNameLong_PtrValue;
        } else if (strcmp(name, "nativePtr") == 0 && g_mockApi->objectGetFieldByNameLong_NativePtrValue >= 0) {
            *result = g_mockApi->objectGetFieldByNameLong_NativePtrValue;
        } else {
            *result = g_mockApi->objectGetFieldByNameLongValue;
        }
    }
    return ANI_OK;
}

static ani_status MockObjectSetFieldByNameLong(ani_env *env, ani_object object, const char *name,
    ani_long value)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    g_mockApi->objectSetFieldByNameLongCallCount++;
    g_mockApi->lastSetFieldName = name;
    g_mockApi->setFieldValues.push_back(value);
    return g_mockApi->objectSetFieldByNameLongStatus;
}

static ani_status MockObjectCallMethodVoidV(ani_env *env, ani_object object, ani_method method,
    va_list args)
{
    if (g_mockApi == nullptr) return ANI_ERROR;
    g_mockApi->objectCallMethodVoidVCallCount++;
    return g_mockApi->objectCallMethodVoidVStatus;
}

// Build a minimal ani_env pointing to our mock API table.
// We only populate the slots actually accessed by the tested code paths.
ani_env BuildMockEnv(MockAniApi *api)
{
    static __ani_interaction_api apiTable {};
    // Zero-initialize the whole table; fill in only what we need.
    memset(&apiTable, 0, sizeof(apiTable));

    apiTable.FindClass = reinterpret_cast<ani_status (*)(ani_env*, const char*, ani_class*)>(MockFindClass);
    apiTable.Class_FindField = reinterpret_cast<ani_status (*)(ani_env*, ani_class, const char*, ani_field*)>(
        MockClassFindField);
    apiTable.Class_FindMethod =
        reinterpret_cast<ani_status (*)(ani_env*, ani_class, const char*, const char*, ani_method*)>(
            MockClassFindMethod);
    apiTable.Object_GetField_Long =
        reinterpret_cast<ani_status (*)(ani_env*, ani_object, ani_field, ani_long*)>(MockObjectGetFieldLong);
    apiTable.Object_GetFieldByName_Long =
        reinterpret_cast<ani_status (*)(ani_env*, ani_object, const char*, ani_long*)>(
            MockObjectGetFieldByNameLong);
    apiTable.Object_SetFieldByName_Long =
        reinterpret_cast<ani_status (*)(ani_env*, ani_object, const char*, ani_long)>(
            MockObjectSetFieldByNameLong);
    apiTable.Object_CallMethod_Void_V =
        reinterpret_cast<ani_status (*)(ani_env*, ani_object, ani_method, va_list)>(
            MockObjectCallMethodVoidV);

    static __ani_env envStruct;
    envStruct.c_api = &apiTable;

    SetMockApi(api);
    return &envStruct;
}
} // anonymous namespace

// ============================================================================
// Test fixture
// ============================================================================
class EtsContextUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

protected:
    MockAniApi mockApi_;
    ani_env mockEnv_ = nullptr;
};

void EtsContextUtilsTest::SetUpTestCase(void) {}

void EtsContextUtilsTest::TearDownTestCase(void) {}

void EtsContextUtilsTest::SetUp()
{
    mockApi_ = MockAniApi{};  // Reset mock state
    g_mockApi = nullptr;
    mockEnv_ = BuildMockEnv(&mockApi_);
}

void EtsContextUtilsTest::TearDown()
{
    g_mockApi = nullptr;
}

// ============================================================================
// ContextUtil::Clean tests
// ============================================================================

/**
 * @tc.name: Clean_NullEnv_001
 * @tc.desc: Clean with nullptr env should return immediately without crash.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, Clean_NullEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Clean_NullEnv_001 start");
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    // Should not crash when env is null.
    EXPECT_NO_FATAL_FAILURE(ContextUtil::Clean(nullptr, dummy));
    TAG_LOGI(AAFwkTag::TEST, "Clean_NullEnv_001 end");
}

/**
 * @tc.name: Clean_GetFieldByNameLong_Fails_001
 * @tc.desc: Clean when GetFieldByNameLong for "ptr" fails should return early.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, Clean_GetFieldByNameLong_Fails_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Clean_GetFieldByNameLong_Fails_001 start");
    mockApi_.objectGetFieldByNameLongStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    EXPECT_NO_FATAL_FAILURE(ContextUtil::Clean(mockEnv_, dummy));
    // Should have tried to read "ptr" and failed; no delete should have happened.
    EXPECT_EQ(mockApi_.objectGetFieldByNameLongCallCount, 1);
    TAG_LOGI(AAFwkTag::TEST, "Clean_GetFieldByNameLong_Fails_001 end");
}

/**
 * @tc.name: Clean_PtrIsZero_001
 * @tc.desc: Clean when ptr field is 0 should skip the weak_ptr delete.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, Clean_PtrIsZero_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Clean_PtrIsZero_001 start");
    // Both "ptr" and "nativePtr" return 0 — no deletes should happen.
    mockApi_.objectGetFieldByNameLongValue = 0;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    EXPECT_NO_FATAL_FAILURE(ContextUtil::Clean(mockEnv_, dummy));
    // Both "ptr" and "nativePtr" fields should have been queried.
    EXPECT_GE(mockApi_.objectGetFieldByNameLongCallCount, 2);
    // SetFieldByNameLong must NOT be called since no delete path was entered.
    EXPECT_EQ(mockApi_.objectSetFieldByNameLongCallCount, 0);
    TAG_LOGI(AAFwkTag::TEST, "Clean_PtrIsZero_001 end");
}

/**
 * @tc.name: Clean_Idempotency_001
 * @tc.desc: After Clean frees ptr, it sets it to 0. Calling Clean again should be
 *           safe (no double-free).
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, Clean_Idempotency_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Clean_Idempotency_001 start");

    // Allocate a heap weak_ptr for the "ptr" field; "nativePtr" stays 0.
    auto *heapWeakPtr = new std::weak_ptr<OHOS::AbilityRuntime::Context>();
    ani_long heapPtr = reinterpret_cast<ani_long>(heapWeakPtr);

    mockApi_.objectGetFieldByNameLongStatus = ANI_OK;
    mockApi_.objectSetFieldByNameLongStatus = ANI_OK;
    // Per-field: "ptr" returns heapPtr (non-zero → triggers delete+clear),
    // "nativePtr" returns 0 (skips delete).
    mockApi_.objectGetFieldByNameLong_PtrValue = heapPtr;
    mockApi_.objectGetFieldByNameLong_NativePtrValue = 0;

    // First Clean call: ptr != 0 → delete heapWeakPtr → set "ptr" to 0.
    // nativePtr == 0 → skip delete.
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    ContextUtil::Clean(mockEnv_, dummy);

    // Verify SetFieldByNameLong was called to clear "ptr" to 0 (exactly once).
    EXPECT_EQ(mockApi_.objectSetFieldByNameLongCallCount, 1);
    EXPECT_GT(mockApi_.setFieldValues.size(), static_cast<size_t>(0));
    EXPECT_EQ(mockApi_.setFieldValues[0], static_cast<ani_long>(0))
        << "Clean should have set ptr field to 0 after delete";

    // Second Clean call: "ptr" reads 0 now, skips delete.
    mockApi_.objectGetFieldByNameLong_PtrValue = 0;
    mockApi_.objectGetFieldByNameLong_NativePtrValue = 0;
    mockApi_.objectSetFieldByNameLongCallCount = 0;
    mockApi_.objectGetFieldByNameLongCallCount = 0;
    EXPECT_NO_FATAL_FAILURE(ContextUtil::Clean(mockEnv_, dummy));
    // Both fields should be queried again, but no SetField call.
    EXPECT_GE(mockApi_.objectGetFieldByNameLongCallCount, 2);
    EXPECT_EQ(mockApi_.objectSetFieldByNameLongCallCount, 0);

    TAG_LOGI(AAFwkTag::TEST, "Clean_Idempotency_001 end");
}

// ============================================================================
// ContextUtil::SetNativeContextLong tests
// ============================================================================

/**
 * @tc.name: SetNativeContextLong_NullEnv_001
 * @tc.desc: SetNativeContextLong with nullptr env should return false.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, SetNativeContextLong_NullEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_NullEnv_001 start");
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    bool ret = ContextUtil::SetNativeContextLong(nullptr, dummy, 0xDEAD);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_NullEnv_001 end");
}

/**
 * @tc.name: SetNativeContextLong_SetFieldFailed_001
 * @tc.desc: SetNativeContextLong when SetFieldByNameLong fails should return false
 *           before calling setEtsContextPtr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, SetNativeContextLong_SetFieldFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_SetFieldFailed_001 start");
    mockApi_.objectSetFieldByNameLongStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    bool ret = ContextUtil::SetNativeContextLong(mockEnv_, dummy, 0x1234);
    EXPECT_FALSE(ret);
    // Should NOT have tried to find class or call setEtsContextPtr.
    EXPECT_EQ(mockApi_.findClassCallCount, 0);
    EXPECT_EQ(mockApi_.objectCallMethodVoidVCallCount, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_SetFieldFailed_001 end");
}

/**
 * @tc.name: SetNativeContextLong_FindClassFailed_001
 * @tc.desc: SetNativeContextLong when FindClass fails should return false.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, SetNativeContextLong_FindClassFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_FindClassFailed_001 start");
    mockApi_.findClassStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    bool ret = ContextUtil::SetNativeContextLong(mockEnv_, dummy, 0x1234);
    EXPECT_FALSE(ret);
    EXPECT_EQ(mockApi_.findClassCallCount, 1);
    // Should not have called setEtsContextPtr since FindClass failed.
    EXPECT_EQ(mockApi_.objectCallMethodVoidVCallCount, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_FindClassFailed_001 end");
}

/**
 * @tc.name: SetNativeContextLong_FindMethodFailed_001
 * @tc.desc: SetNativeContextLong when Class_FindMethod for setEtsContextPtr fails
 *           should return false.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, SetNativeContextLong_FindMethodFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_FindMethodFailed_001 start");
    mockApi_.classFindMethodStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    bool ret = ContextUtil::SetNativeContextLong(mockEnv_, dummy, 0x1234);
    EXPECT_FALSE(ret);
    EXPECT_EQ(mockApi_.classFindMethodCallCount, 1);
    // Should not have called setEtsContextPtr since FindMethod failed.
    EXPECT_EQ(mockApi_.objectCallMethodVoidVCallCount, 0);
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_FindMethodFailed_001 end");
}

/**
 * @tc.name: SetNativeContextLong_CallsSetEtsContextPtr_OnAniObj_001
 * @tc.desc: SetNativeContextLong should call setEtsContextPtr directly on aniObj
 *           (the passed-in object), NOT on a newly created Context object.
 *           This verifies the root cause fix: no Extra Context is created.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, SetNativeContextLong_CallsSetEtsContextPtr_OnAniObj_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_CallsSetEtsContextPtr_OnAniObj_001 start");
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    bool ret = ContextUtil::SetNativeContextLong(mockEnv_, dummy, 0x5678);
    EXPECT_TRUE(ret);

    // "nativeContext" field should have been set exactly once.
    EXPECT_EQ(mockApi_.objectSetFieldByNameLongCallCount, 1);

    // Class_FindMethod should have been called exactly once (to look up
    // "setEtsContextPtr"). In the buggy code, it would be called twice:
    // once for "<ctor>" and once for "setEtsContextPtr".
    EXPECT_EQ(mockApi_.classFindMethodCallCount, 1);
    // Verify it searched for setEtsContextPtr (not ctor).
    EXPECT_STREQ(mockApi_.lastFindMethodName, "setEtsContextPtr");

    // Object_CallMethod_Void_V should have been called exactly once (to invoke
    // setEtsContextPtr on aniObj). If an Extra Context had been created, there
    // would have been additional calls to Object_New / ctor.
    EXPECT_EQ(mockApi_.objectCallMethodVoidVCallCount, 1);

    TAG_LOGI(AAFwkTag::TEST, "SetNativeContextLong_CallsSetEtsContextPtr_OnAniObj_001 end");
}

// ============================================================================
// ContextUtil::GetBaseContext tests
// ============================================================================

/**
 * @tc.name: GetBaseContext_NullEnv_001
 * @tc.desc: GetBaseContext with nullptr env should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_NullEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_NullEnv_001 start");
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(nullptr, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_NullEnv_001 end");
}

/**
 * @tc.name: GetBaseContext_NativeContextIsZero_001
 * @tc.desc: GetBaseContext should return nullptr when nativeContext field is 0.
 *           This tests the newly added null validation.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_NativeContextIsZero_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_NativeContextIsZero_001 start");
    mockApi_.objectGetFieldLongValue = 0;  // nativeContext == 0
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_NativeContextIsZero_001 end");
}

/**
 * @tc.name: GetBaseContext_FindClassFailed_001
 * @tc.desc: GetBaseContext when FindClass fails should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_FindClassFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_FindClassFailed_001 start");
    mockApi_.findClassStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_FindClassFailed_001 end");
}

/**
 * @tc.name: GetBaseContext_ClassFindFieldFailed_001
 * @tc.desc: GetBaseContext when Class_FindField fails should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_ClassFindFieldFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ClassFindFieldFailed_001 start");
    mockApi_.classFindFieldStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ClassFindFieldFailed_001 end");
}

/**
 * @tc.name: GetBaseContext_ObjectGetFieldLongFailed_001
 * @tc.desc: GetBaseContext when Object_GetField_Long fails should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_ObjectGetFieldLongFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ObjectGetFieldLongFailed_001 start");
    mockApi_.objectGetFieldLongStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ObjectGetFieldLongFailed_001 end");
}

/**
 * @tc.name: GetBaseContext_ExpiredWeakPtr_001
 * @tc.desc: GetBaseContext with a non-zero nativeContext pointing to an expired
 *           weak_ptr should return nullptr (lock fails).
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetBaseContext_ExpiredWeakPtr_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ExpiredWeakPtr_001 start");

    // Heap-allocate an empty weak_ptr (already expired — no shared_ptr owns it).
    auto *heapWeakPtr = new std::weak_ptr<OHOS::AbilityRuntime::Context>();
    mockApi_.objectGetFieldLongValue = reinterpret_cast<ani_long>(heapWeakPtr);

    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = ContextUtil::GetBaseContext(mockEnv_, dummy);

    // The weak_ptr is expired (no shared_ptr reference), so lock() returns nullptr.
    EXPECT_EQ(ctx, nullptr);

    delete heapWeakPtr;
    TAG_LOGI(AAFwkTag::TEST, "GetBaseContext_ExpiredWeakPtr_001 end");
}

// ============================================================================
// GetStageModeContext tests (ani_base_context.cpp)
// ============================================================================

/**
 * @tc.name: GetStageModeContext_NullEnv_001
 * @tc.desc: GetStageModeContext with nullptr env should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetStageModeContext_NullEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_NullEnv_001 start");
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = OHOS::AbilityRuntime::GetStageModeContext(nullptr, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_NullEnv_001 end");
}

/**
 * @tc.name: GetStageModeContext_NativeContextIsZero_001
 * @tc.desc: GetStageModeContext should return nullptr when nativeContext field is 0.
 *           This tests the newly added null validation.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetStageModeContext_NativeContextIsZero_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_NativeContextIsZero_001 start");
    mockApi_.objectGetFieldByNameLongValue = 0;  // nativeContext == 0
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = OHOS::AbilityRuntime::GetStageModeContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_NativeContextIsZero_001 end");
}

/**
 * @tc.name: GetStageModeContext_GetFieldFailed_001
 * @tc.desc: GetStageModeContext when Object_GetFieldByName_Long fails should
 *           return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetStageModeContext_GetFieldFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_GetFieldFailed_001 start");
    mockApi_.objectGetFieldByNameLongStatus = ANI_ERROR;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = OHOS::AbilityRuntime::GetStageModeContext(mockEnv_, dummy);
    EXPECT_EQ(ctx, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_GetFieldFailed_001 end");
}

/**
 * @tc.name: GetStageModeContext_ExpiredWeakPtr_001
 * @tc.desc: GetStageModeContext when the weak_ptr is expired should return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, GetStageModeContext_ExpiredWeakPtr_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_ExpiredWeakPtr_001 start");

    auto *heapWeakPtr = new std::weak_ptr<OHOS::AbilityRuntime::Context>();
    mockApi_.objectGetFieldByNameLongValue = reinterpret_cast<ani_long>(heapWeakPtr);

    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    auto ctx = OHOS::AbilityRuntime::GetStageModeContext(mockEnv_, dummy);

    EXPECT_EQ(ctx, nullptr);

    delete heapWeakPtr;
    TAG_LOGI(AAFwkTag::TEST, "GetStageModeContext_ExpiredWeakPtr_001 end");
}

// ============================================================================
// IsStageContext tests (ani_base_context.cpp)
// ============================================================================

/**
 * @tc.name: IsStageContext_WithNullEnv_001
 * @tc.desc: IsStageContext should return ANI_OK with stageMode=true even
 *           with null env (currently always true for ArkTS 1.2).
 * @tc.type: FUNC
 */
HWTEST_F(EtsContextUtilsTest, IsStageContext_WithNullEnv_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsStageContext_WithNullEnv_001 start");
    ani_boolean stageMode = false;
    ani_object dummy = reinterpret_cast<ani_object>(0x1000);
    ani_status status = OHOS::AbilityRuntime::IsStageContext(nullptr, dummy, stageMode);
    EXPECT_EQ(status, ANI_OK);
    EXPECT_TRUE(stageMode);
    TAG_LOGI(AAFwkTag::TEST, "IsStageContext_WithNullEnv_001 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
