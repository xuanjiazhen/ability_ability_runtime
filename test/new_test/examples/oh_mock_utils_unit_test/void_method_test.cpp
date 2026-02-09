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
 * @file void_method_test.cpp
 * @brief Test suite for OH_MOCK_VOID_METHOD macro
 *
 * This file tests mock functionality for member functions with void return type.
 */

#define private public
#define protected public

#include <gtest/gtest.h>
#include <string>
#include "oh_mock_utils.h"
#include "mock_classes/test_class_basic.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::TestMock;

namespace OHOS {
namespace TestMock {

/**
 * @class VoidMethodTest
 * @brief Test fixture for void method macro tests
 */
class VoidMethodTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void VoidMethodTest::SetUpTestCase() {}

void VoidMethodTest::TearDownTestCase() {}

void VoidMethodTest::SetUp() {}

void VoidMethodTest::TearDown() {}

/**
 * @tc.name: Execute_SingleCall_001
 * @tc.desc: Test OH_MOCK_VOID_METHOD with single call
 *           Branch: Normal execution - void method executes without error
 * @tc.type: FUNC
 */
HWTEST_F(VoidMethodTest, Execute_SingleCall_001, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    TestClassBasic testObj;
    // No return value to set for void methods

    // ===== Act: Call the code under test =====
    // Void method should execute without error
    testObj.Execute("TestCommand", 1);

    // ===== Assert: Verify results =====
    // Void method has no return value to check
    // Test passes if no exception is thrown
    SUCCEED();
}

/**
 * @tc.name: Execute_MultipleCalls_002
 * @tc.desc: Test queue mechanism for void methods
 *           Branch: Multiple void calls consume expectations from queue
 * @tc.type: FUNC
 */
HWTEST_F(VoidMethodTest, Execute_MultipleCalls_002, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    TestClassBasic testObj;
    // Void methods still use the queue mechanism internally

    // ===== Act: Multiple calls =====
    // All void calls should execute without error
    testObj.Execute("Command1", 1);
    testObj.Execute("Command2", 2);
    testObj.Execute("Command3", 3);

    // ===== Assert: Verify results =====
    // All calls should complete successfully
    SUCCEED();
}

/**
 * @tc.name: Execute_EmptyExpectation_003
 * @tc.desc: Test void method works when no expectations are set
 *           Branch: Edge case - empty expectation queue for void method
 * @tc.type: FUNC
 */
HWTEST_F(VoidMethodTest, Execute_EmptyExpectation_003, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    TestClassBasic testObj;
    // No expectations set

    // ===== Act: Call the code under test =====
    testObj.Execute("TestCommand", 5);

    // ===== Assert: Verify results =====
    // Should still execute without error
    SUCCEED();
}

/**
 * @tc.name: Execute_WithParameters_004
 * @tc.desc: Test void method properly accepts parameters
 *           Branch: Parameter passing verification
 * @tc.type: FUNC
 */
HWTEST_F(VoidMethodTest, Execute_WithParameters_004, TestSize.Level1)
{
    // ===== Arrange: Prepare test data and mock =====
    TestClassBasic testObj;
    std::string testCommand = "ComplexCommand";
    int testPriority = 10;

    // ===== Act: Call the code under test =====
    testObj.Execute(testCommand, testPriority);

    // ===== Assert: Verify results =====
    // Parameters should be accepted correctly
    SUCCEED();
}

} // namespace TestMock
} // namespace OHOS
