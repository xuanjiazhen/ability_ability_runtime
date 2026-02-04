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
 * @file resultwrap_test.cpp
 * @brief Test suite for ResultWrap class and CreateResultWrap utility function
 *
 * This file tests the core functionality of the ResultWrap class, which is the
 * foundation for storing and retrieving mock expectations with type safety.
 */

#define private public
#define protected public

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <any>
#include "oh_mock_utils.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace TestMock {

/**
 * @class ResultWrapTest
 * @brief Test fixture for ResultWrap class tests
 */
class ResultWrapTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ResultWrapTest::SetUpTestCase() {}

void ResultWrapTest::TearDownTestCase() {}

void ResultWrapTest::SetUp() {}

void ResultWrapTest::TearDown() {}

/**
 * @tc.name: ResultWrap_StoreAndGetInt_001
 * @tc.desc: Test ResultWrap can store and retrieve int32_t value correctly
 *           Branch: Normal storage and retrieval of basic type
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_StoreAndGetInt_001, TestSize.Level1)
{
    // Arrange
    int32_t testValue = 42;

    // Act
    ResultWrap wrap(testValue);
    int32_t result = wrap.Get<int32_t>();

    // Assert
    EXPECT_EQ(result, 42);
}

/**
 * @tc.name: ResultWrap_StoreAndGetString_002
 * @tc.desc: Test ResultWrap can store and retrieve std::string value correctly
 *           Branch: Normal storage and retrieval of string type
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_StoreAndGetString_002, TestSize.Level1)
{
    // Arrange
    std::string testValue = "Hello, Mock World!";

    // Act
    ResultWrap wrap(testValue);
    std::string result = wrap.Get<std::string>();

    // Assert
    EXPECT_EQ(result, "Hello, Mock World!");
}

/**
 * @tc.name: ResultWrap_StoreAndGetVector_003
 * @tc.desc: Test ResultWrap can store and retrieve std::vector<int> value correctly
 *           Branch: Normal storage and retrieval of vector type
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_StoreAndGetVector_003, TestSize.Level1)
{
    // Arrange
    std::vector<int> testValue = {1, 2, 3, 4, 5};

    // Act
    ResultWrap wrap(testValue);
    std::vector<int> result = wrap.Get<std::vector<int>>();

    // Assert
    EXPECT_EQ(result.size(), 5U);
    EXPECT_EQ(result[0], 1);
    EXPECT_EQ(result[1], 2);
    EXPECT_EQ(result[2], 3);
    EXPECT_EQ(result[3], 4);
    EXPECT_EQ(result[4], 5);
}

/**
 * @tc.name: ResultWrap_VectorBoolReference_004
 * @tc.desc: Test CreateResultWrap handles vector<bool>::reference correctly
 *           Branch: Special handling for vector<bool>::reference type
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_VectorBoolReference_004, TestSize.Level1)
{
    // Arrange
    std::vector<bool> boolVector = {true, false, true};
    std::vector<bool>::reference ref = boolVector[1];  // false

    // Act
    ResultWrap wrap = CreateResultWrap(ref);
    bool result = wrap.Get<bool>();

    // Assert
    EXPECT_FALSE(result);  // Should be false
}

/**
 * @tc.name: ResultWrap_GetAnyTypeMismatch_005
 * @tc.desc: Test std::any_cast throws on type mismatch
 *           Branch: Error handling - attempting to get wrong type
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_GetAnyTypeMismatch_005, TestSize.Level1)
{
    // Arrange
    int32_t testValue = 100;
    ResultWrap wrap(testValue);

    // Act & Assert
    // Attempting to get wrong type should throw std::any_bad_cast
    EXPECT_THROW(wrap.Get<std::string>(), std::any_bad_cast);
}

/**
 * @tc.name: ResultWrap_DefaultConstructor_006
 * @tc.desc: Test default-constructed ResultWrap contains default value
 *           Branch: Edge case - default initialization
 * @tc.type: FUNC
 */
HWTEST_F(ResultWrapTest, ResultWrap_DefaultConstructor_006, TestSize.Level1)
{
    // Arrange & Act
    ResultWrap wrap;

    // Assert
    // Default-constructed ResultWrap should contain default int32_t value (0)
    int32_t result = wrap.Get<int32_t>();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: ResultWrap_CustomStruct_007
 * @tc.desc: Test ResultWrap works with custom struct types
 *           Branch: Custom structure type support
 * @tc.type: FUNC
 */
struct TestStruct {
    int id;
    std::string name;
    double value;
};

HWTEST_F(ResultWrapTest, ResultWrap_CustomStruct_007, TestSize.Level1)
{
    // Arrange
    TestStruct testValue;
    testValue.id = 1001;
    testValue.name = "TestStruct";
    testValue.value = 3.14;

    // Act
    ResultWrap wrap(testValue);
    TestStruct result = wrap.Get<TestStruct>();

    // Assert
    EXPECT_EQ(result.id, 1001);
    EXPECT_EQ(result.name, "TestStruct");
    EXPECT_DOUBLE_EQ(result.value, 3.14);
}

} // namespace TestMock
} // namespace OHOS
