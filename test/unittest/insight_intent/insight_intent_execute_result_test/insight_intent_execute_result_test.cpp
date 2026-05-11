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
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;

namespace {
} // namespace

class InsightIntentExecuteResultTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteResultTest::SetUpTestCase(void)
{}

void InsightIntentExecuteResultTest::TearDownTestCase(void)
{}

void InsightIntentExecuteResultTest::SetUp()
{}

void InsightIntentExecuteResultTest::TearDown()
{}

/**
 * @tc.name: ReadFromParcel_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    WantParams result;
    WantParams queryResult;
    std::vector<std::string> uris = { "uri_1", "uri_2" };

    parcel.WriteInt32(0);
    parcel.WriteInt32(1);
    parcel.WriteParcelable(&result);
    parcel.WriteStringVector(uris);
    parcel.WriteInt32(2);
    parcel.WriteBool(true);
    parcel.WriteBool(true);
    parcel.WriteInt32(1);
    parcel.WriteParcelable(&queryResult);

    auto ret = testclass.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
    EXPECT_EQ(testclass.innerErr, 0);
    EXPECT_EQ(testclass.code, 1);
    EXPECT_EQ(testclass.flags, 2);
    EXPECT_TRUE(testclass.isDecorator);
    EXPECT_TRUE(testclass.isQueryEntity);
    EXPECT_EQ(testclass.uris.size(), 2);
    EXPECT_EQ(testclass.queryResults.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ReadFromParcel_0200
 * @tc.desc: resultSize is negative.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_0200, TestSize.Level1)
{
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    WantParams result;

    parcel.WriteInt32(0);
    parcel.WriteInt32(1);
    parcel.WriteParcelable(&result);
    parcel.WriteStringVector({});
    parcel.WriteInt32(0);
    parcel.WriteBool(false);
    parcel.WriteBool(false);
    parcel.WriteInt32(-1);

    auto ret = testclass.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadFromParcel_0300
 * @tc.desc: resultSize exceeds cycle limit.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_0300, TestSize.Level1)
{
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    WantParams result;

    parcel.WriteInt32(0);
    parcel.WriteInt32(1);
    parcel.WriteParcelable(&result);
    parcel.WriteStringVector({});
    parcel.WriteInt32(0);
    parcel.WriteBool(false);
    parcel.WriteBool(false);
    parcel.WriteInt32(1001);

    auto ret = testclass.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: ReadFromParcel_0400
 * @tc.desc: query result parcelable is missing.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteResultTest, ReadFromParcel_0400, TestSize.Level1)
{
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    WantParams result;

    parcel.WriteInt32(0);
    parcel.WriteInt32(1);
    parcel.WriteParcelable(&result);
    parcel.WriteStringVector({});
    parcel.WriteInt32(0);
    parcel.WriteBool(false);
    parcel.WriteBool(true);
    parcel.WriteInt32(1);

    auto ret = testclass.ReadFromParcel(parcel);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: Marshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    InsightIntentExecuteResult testclass;
    Parcel parcel;
    auto ret = testclass.Marshalling(parcel);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: Unmarshalling_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Parcel parcel;
    auto ret = InsightIntentExecuteResult::Unmarshalling(parcel);
    EXPECT_TRUE(ret != nullptr);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CheckResult_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InsightIntentExecuteResultTest, CheckResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    std::shared_ptr<const WantParams> result;
    auto ret = InsightIntentExecuteResult::CheckResult(result);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AAFwk
} // namespace OHOS
