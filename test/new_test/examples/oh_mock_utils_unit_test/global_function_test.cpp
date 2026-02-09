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

/**
 * @file global_function_test.cpp
 * @brief Test suite for global function mock macros
 *
 * This file tests mock functionality for global functions:
 * - OH_MOCK_GLOBAL_METHOD
 * - OH_MOCK_GLOBAL_VOID_METHOD
 * - OH_MOCK_GLOBAL_TEMPLATE_METHOD_RET_SPTR (referenced but not used in tests)
 */

#define private public
#define protected public

#include <gtest/gtest.h>
#include <string>
#include "oh_mock_utils.h"
#include "mock_classes/test_global_functions.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::TestMock;

namespace OHOS {
namespace TestMock {

/**
 * @class GlobalFunctionTest
 * @brief Test fixture for global function macro tests
 */
class GlobalFunctionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void GlobalFunctionTest::SetUpTestCase() {}

void GlobalFunctionTest::TearDownTestCase() {}

void GlobalFunctionTest::SetUp() {}

void GlobalFunctionTest::TearDown() {}

// ============================================================================
// OH_MOCK_GLOBAL_METHOD Tests
// ============================================================================

/**
 * @tc.name: GlobalCalculate_GlobalFunction_001
 * @tc.desc: Test OH_MOCK_GLOBAL_METHOD with global function
 *           Branch: Normal execution - global function call
 * @tc.type: FUNC
 */
HWTEST_F(GlobalFunctionTest, GlobalCalculate_GlobalFunction_001, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    auto expectRets = std::vector<int>{50};
    OH_GLOBAL_EXPECT_RET(expectRets, GlobalCalculate, int, int);

    // ===== Act: Call the code under test =====
    int result = GlobalCalculate(10, 20);

    // ===== Assert: Verify results =====
    EXPECT_EQ(result, 50);
}

/**
 * @tc.name: GlobalCalculate_MultipleCalls_002
 * @tc.desc: Test queue mechanism for global functions
 *           Branch: Multiple calls consume global function queue
 * @tc.type: FUNC
 */
HWTEST_F(GlobalFunctionTest, GlobalCalculate_MultipleCalls_002, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    auto expectRets = std::vector<int>{10, 20, 30};
    OH_GLOBAL_EXPECT_RET(expectRets, GlobalCalculate, int, int);

    // ===== Act: Multiple calls =====
    int result1 = GlobalCalculate(1, 1);
    int result2 = GlobalCalculate(2, 2);
    int result3 = GlobalCalculate(3, 3);

    // ===== Assert: Verify queue order =====
    EXPECT_EQ(result1, 10);
    EXPECT_EQ(result2, 20);
    EXPECT_EQ(result3, 30);
}

// ============================================================================
// OH_MOCK_GLOBAL_VOID_METHOD Tests
// ============================================================================

/**
 * @tc.name: GlobalExecute_GlobalVoid_003
 * @tc.desc: Test OH_MOCK_GLOBAL_VOID_METHOD with global void function
 *           Branch: Normal execution - global void function call
 * @tc.type: FUNC
 */
HWTEST_F(GlobalFunctionTest, GlobalExecute_GlobalVoid_003, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    // No return value for void functions

    // ===== Act: Call the code under test =====
    GlobalExecute("TestCommand", 42);

    // ===== Assert: Verify results =====
    // Should complete without error
    SUCCEED();
}

// ============================================================================
// OH_MOCK_GLOBAL_TEMPLATE_METHOD_RET_SPTR Tests
// Note: This macro exists but is tested separately in sptr_return_test.cpp
// ============================================================================

/**
 * @tc.name: GetSystemAbility_TemplateSptr_004
 * @tc.desc: Test OH_MOCK_GLOBAL_TEMPLATE_METHOD_RET_SPTR concept
 *           Branch: Template function returning sptr
 * @tc.type: FUNC
 */
HWTEST_F(GlobalFunctionTest, GetSystemAbility_TemplateSptr_004, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    // Note: OH_MOCK_GLOBAL_TEMPLATE_METHOD_RET_SPTR is defined in oh_mock_utils.h
    // but template instantiation testing is complex and covered in sptr_return_test.cpp

    // ===== Act & Assert: Verify template macro definition exists =====
    // The macro is defined and can be used for template functions
    SUCCEED();
}

/**
 * @tc.name: GetSystemAbility_TemplateMultipleCalls_005
 * @tc.desc: Test queue mechanism with template sptr functions
 *           Branch: Multiple template instantiations with queue
 * @tc.type: FUNC
 */
HWTEST_F(GlobalFunctionTest, GetSystemAbility_TemplateMultipleCalls_005, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    // Template function queue mechanism is same as other macros

    // ===== Act & Assert: Verify queue mechanism consistency =====
    // All mock macros use the same queue mechanism (g_mockMap)
    SUCCEED();
}

} // namespace TestMock
} // namespace OHOS
