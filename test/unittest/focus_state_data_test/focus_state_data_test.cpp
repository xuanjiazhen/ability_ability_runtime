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

#define private public
#define protected public
#include "focus_state_data.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusStateDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FocusStateDataTest::SetUpTestCase(void) {}
void FocusStateDataTest::TearDownTestCase(void) {}
void FocusStateDataTest::SetUp() {}
void FocusStateDataTest::TearDown() {}

/**
 * @tc.number: Marshalling_0100
 * @tc.desc: Test Marshalling with valid FocusStateData.
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateDataTest, Marshalling_0100, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = 10002;
    data.focusedBundleName = "com.example.focused";
    data.unfocusedBundleName = "com.example.unfocused";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
}

/**
 * @tc.number: Marshalling_0200
 * @tc.desc: Test Marshalling with default values (-1 and empty strings).
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateDataTest, Marshalling_0200, TestSize.Level1)
{
    FocusStateData data;
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    EXPECT_EQ(data.focusedUid, -1);
    EXPECT_EQ(data.unfocusedUid, -1);
}

/**
 * @tc.number: Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateDataTest, Unmarshalling_0100, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = 10002;
    data.focusedBundleName = "com.example.focused";
    data.unfocusedBundleName = "com.example.unfocused";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = FocusStateData::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->focusedUid, 10001);
    EXPECT_EQ(result->unfocusedUid, 10002);
    EXPECT_EQ(result->focusedBundleName, "com.example.focused");
    EXPECT_EQ(result->unfocusedBundleName, "com.example.unfocused");
    delete result;
}

/**
 * @tc.number: Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with first-focus values (unfocusedUid = -1).
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateDataTest, Unmarshalling_0200, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = -1;
    data.focusedBundleName = "com.example.focused";
    data.unfocusedBundleName = "";
    MessageParcel parcel;
    EXPECT_TRUE(data.Marshalling(parcel));
    auto result = FocusStateData::Unmarshalling(parcel);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->focusedUid, 10001);
    EXPECT_EQ(result->unfocusedUid, -1);
    EXPECT_EQ(result->unfocusedBundleName, "");
    delete result;
}
}  // namespace AppExecFwk
}  // namespace OHOS
