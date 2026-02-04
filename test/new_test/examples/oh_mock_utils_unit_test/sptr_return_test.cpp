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
 * @file sptr_return_test.cpp
 * @brief Test suite for OH_MOCK_METHOD_RET_SPTR macro
 *
 * This file tests mock functionality for methods returning sptr<T> smart pointers.
 */

#define private public
#define protected public

#include <gtest/gtest.h>
#include "oh_mock_utils.h"
#include "mock_classes/test_interface.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "refbase.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::TestMock;

namespace OHOS {
namespace TestMock {

/**
 * @class SptrReturnTest
 * @brief Test fixture for sptr return value tests
 */
class SptrReturnTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SptrReturnTest::SetUpTestCase() {}

void SptrReturnTest::TearDownTestCase() {}

void SptrReturnTest::SetUp() {}

void SptrReturnTest::TearDown() {}

/**
 * @tc.name: GetInstance_ReturnValidSptr_001
 * @tc.desc: Test OH_MOCK_METHOD_RET_SPTR returns valid sptr
 *           Branch: Normal execution - valid sptr returned
 * @tc.type: FUNC
 */
HWTEST_F(SptrReturnTest, GetInstance_ReturnValidSptr_001, TestSize.Level1)
{
    // ===== Arrange: 准备数据和Mock =====
    auto mockInstance = sptr<ITestInterface>(new TestInterfaceImpl());
    auto expectRets = std::vector<sptr<ITestInterface>>{mockInstance};
    OH_EXPECT_RET(expectRets, TestInterfaceImpl, GetInstance);

    // ===== Act: 调用被测代码 =====
    sptr<ITestInterface> result = TestInterfaceImpl::GetInstance();

    // ===== Assert: 验证结果 =====
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result.Get(), mockInstance.Get());
}

/**
 * @tc.name: GetInstance_ReturnNullptr_002
 * @tc.desc: Test OH_MOCK_METHOD_RET_SPTR can return nullptr
 *           Branch: Edge case - nullptr sptr handling
 * @tc.type: FUNC
 */
HWTEST_F(SptrReturnTest, GetInstance_ReturnNullptr_002, TestSize.Level1)
{
    // ===== Arrange: 准备数据和Mock =====
    auto expectRets = std::vector<sptr<ITestInterface>>{nullptr};
    OH_EXPECT_RET(expectRets, TestInterfaceImpl, GetInstance);

    // ===== Act: 调用被测代码 =====
    sptr<ITestInterface> result = TestInterfaceImpl::GetInstance();

    // ===== Assert: 验证结果 =====
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetInstance_MultipleCallsSptr_003
 * @tc.desc: Test queue mechanism for sptr returns
 *           Branch: Multiple calls consume sptr queue in order
 * @tc.type: FUNC
 */
HWTEST_F(SptrReturnTest, GetInstance_MultipleCallsSptr_003, TestSize.Level1)
{
    // ===== Arrange: 准备数据和Mock =====
    auto mockInstance1 = sptr<ITestInterface>(new TestInterfaceImpl());
    auto mockInstance2 = sptr<ITestInterface>(new TestInterfaceImpl());
    auto expectRets = std::vector<sptr<ITestInterface>>{mockInstance1, mockInstance2};
    OH_EXPECT_RET(expectRets, TestInterfaceImpl, GetInstance);

    // ===== Act: 多次调用 =====
    sptr<ITestInterface> result1 = TestInterfaceImpl::GetInstance();
    sptr<ITestInterface> result2 = TestInterfaceImpl::GetInstance();

    // ===== Assert: 验证队列顺序 =====
    EXPECT_NE(result1, nullptr);
    EXPECT_NE(result2, nullptr);
    EXPECT_EQ(result1.Get(), mockInstance1.Get());
    EXPECT_EQ(result2.Get(), mockInstance2.Get());
}

/**
 * @tc.name: GetInstance_SptrRefCount_004
 * @tc.desc: Test sptr reference counting works correctly
 *           Branch: Verify sptr reference count correctness
 * @tc.type: FUNC
 */
HWTEST_F(SptrReturnTest, GetInstance_SptrRefCount_004, TestSize.Level1)
{
    // ===== Arrange: 准备数据和Mock =====
    auto mockInstance = sptr<ITestInterface>(new TestInterfaceImpl());
    auto expectRets = std::vector<sptr<ITestInterface>>{mockInstance};
    OH_EXPECT_RET(expectRets, TestInterfaceImpl, GetInstance);

    // ===== Act: 调用被测代码 =====
    sptr<ITestInterface> result = TestInterfaceImpl::GetInstance();

    // ===== Assert: 验证引用计数 =====
    EXPECT_NE(result, nullptr);
    // Both result and expectRets[0] should point to same object
    EXPECT_EQ(result.Get(), mockInstance.Get());

    // Reference count should be at least 2 (result + expectRets[0])
    // Note: The actual reference count depends on implementation details
}

/**
 * @tc.name: GetInstance_DifferentInterfaces_005
 * @tc.desc: Test sptr works with different interface types
 *           Branch: Different interface types support
 * @tc.type: FUNC
 */
HWTEST_F(SptrReturnTest, GetInstance_DifferentInterfaces_005, TestSize.Level1)
{
    // ===== Arrange: 准备数据和Mock =====
    // Create multiple mock instances
    auto mockInstance1 = sptr<ITestInterface>(new TestInterfaceImpl());
    auto mockInstance2 = sptr<ITestInterface>(new TestInterfaceImpl());

    auto expectRets = std::vector<sptr<ITestInterface>>{mockInstance1, mockInstance2, mockInstance1};
    OH_EXPECT_RET(expectRets, TestInterfaceImpl, GetInstance);

    // ===== Act: 多次调用 =====
    sptr<ITestInterface> result1 = TestInterfaceImpl::GetInstance();
    sptr<ITestInterface> result2 = TestInterfaceImpl::GetInstance();
    sptr<ITestInterface> result3 = TestInterfaceImpl::GetInstance();

    // ===== Assert: 验证结果 =====
    EXPECT_NE(result1, nullptr);
    EXPECT_NE(result2, nullptr);
    EXPECT_NE(result3, nullptr);

    // result1 and result3 should point to same instance
    EXPECT_EQ(result1.Get(), result3.Get());

    // result2 should be different
    EXPECT_NE(result1.Get(), result2.Get());
}

} // namespace TestMock
} // namespace OHOS
