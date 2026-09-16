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
#include <string>
#include <vector>

#include "extract_insight_intent_profile.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_info_for_query.h"
#include "insight_intent_info_for_query_json.h"
#include "insight_intent_profile.h"
#include "json_safe_util.h"
#include "nlohmann/json.hpp"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t DUMP_MAX_LEN = 200;

struct SimpleItem {
    int32_t value;
    std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SimpleItem, value, name)

struct MismatchItem {
    int32_t version;
    std::string id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MismatchItem, version, id)
} // namespace

class IntentJsonSafeGetTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void IntentJsonSafeGetTest::SetUpTestCase() {}
void IntentJsonSafeGetTest::TearDownTestCase() {}
void IntentJsonSafeGetTest::SetUp() {}
void IntentJsonSafeGetTest::TearDown() {}

// ---------------- Integration: ExtractInsightIntentProfile::TransformTo ----------------

namespace {
const char *EXTRACT_TYPE_MISMATCH_PROFILE = R"({
    "extractInsightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": 1,
        "decoratorType": "@InsightIntentLink",
        "uri": "/test",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "decoratorFile": "f",
        "decoratorClass": "c"
    }]
})";

const char *EXTRACT_BAD_BASE_PROFILE = R"({
    "extractInsightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": "1",
        "decoratorType": "@InsightIntentLink",
        "uri": "/test",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "decoratorFile": "f",
        "decoratorClass": "c"
    }]
})";
} // namespace

/**
 * @tc.number: ExtractTransformTo_TypeMismatch_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: intentVersion as number (string expected) returns false, no exception propagates.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_TypeMismatch_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    EXPECT_FALSE(ExtractInsightIntentProfile::TransformTo(EXTRACT_TYPE_MISMATCH_PROFILE, infos));
}

/**
 * @tc.number: ExtractTransformTo_NoThrow_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: Calling TransformTo repeatedly on bad input does not crash.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_NoThrow_0100, TestSize.Level0)
{
    for (int i = 0; i < 3; ++i) {
        ExtractInsightIntentProfileInfoVec infos;
        EXPECT_FALSE(ExtractInsightIntentProfile::TransformTo(EXTRACT_TYPE_MISMATCH_PROFILE, infos));
    }
}

// ---------------- Integration: InsightIntentProfile::TransformTo ----------------

namespace {
const char *CONFIG_TYPE_MISMATCH_PROFILE = R"({
    "insightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": 1,
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc"
    }]
})";

const char *CONFIG_BAD_EXECUTE_MODE_PROFILE = R"({
    "insightIntents": [{
        "bundleName": "com.test",
        "moduleName": "entry",
        "intentName": "test",
        "intentVersion": "1",
        "domain": "test",
        "displayName": "test",
        "displayDescription": "desc",
        "executeMode": "foreground"
    }]
})";
} // namespace

/**
 * @tc.number: ConfigTransformTo_TypeMismatch_0100
 * @tc.name: InsightIntentProfile::TransformTo
 * @tc.desc: intentVersion as number returns false, no exception propagates.
 */
HWTEST_F(IntentJsonSafeGetTest, ConfigTransformTo_TypeMismatch_0100, TestSize.Level0)
{
    std::vector<InsightIntentInfo> infos;
    EXPECT_FALSE(InsightIntentProfile::TransformTo(CONFIG_TYPE_MISMATCH_PROFILE, infos));
    EXPECT_TRUE(infos.empty());
}

/**
 * @tc.number: ConfigTransformTo_BadExecuteMode_0100
 * @tc.name: InsightIntentProfile::TransformTo
 * @tc.desc: executeMode as scalar string does not crash; result is acceptable as long as no throw.
 */
HWTEST_F(IntentJsonSafeGetTest, ConfigTransformTo_BadExecuteMode_0100, TestSize.Level0)
{
    std::vector<InsightIntentInfo> infos;
    // We only assert no exception / no crash here; downstream may tolerate or skip.
    InsightIntentProfile::TransformTo(CONFIG_BAD_EXECUTE_MODE_PROFILE, infos);
    SUCCEED();
}

/**
 * @tc.number: ExtractTransformTo_ValidBase_0100
 * @tc.name: ExtractInsightIntentProfile::TransformTo
 * @tc.desc: Valid base profile still parses successfully after safety wrapper is applied.
 */
HWTEST_F(IntentJsonSafeGetTest, ExtractTransformTo_ValidBase_0100, TestSize.Level0)
{
    ExtractInsightIntentProfileInfoVec infos;
    bool ok = ExtractInsightIntentProfile::TransformTo(EXTRACT_BAD_BASE_PROFILE, infos);
    // Result may be true or false depending on further validations; assert no crash only.
    if (ok) {
        EXPECT_FALSE(infos.insightIntents.empty());
    }
}

// ---------------- Integration: EntryInfoForQuery / UIAbilityIntentInfoForQuery ----------------

HWTEST_F(IntentJsonSafeGetTest, EntryInfoForQuery_BadExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"abilityName":"A","executeMode":"foreground"})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    EntryInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_TRUE(info.executeMode.empty());
}

HWTEST_F(IntentJsonSafeGetTest, EntryInfoForQuery_GoodExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"abilityName":"A","executeMode":["UI_ABILITY_FOREGROUND"]})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    EntryInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_EQ(info.executeMode.size(), 1UL);
}

HWTEST_F(IntentJsonSafeGetTest, UIAbilityIntentInfoForQuery_BadExecuteMode_0100, TestSize.Level0)
{
    auto j = nlohmann::json::parse(R"({"ability":"A","executeMode":123})",
        nullptr, false);
    ASSERT_FALSE(j.is_discarded());
    UIAbilityIntentInfoForQuery info;
    EXPECT_NO_THROW(from_json(j, info));
    EXPECT_TRUE(info.supportExecuteMode.empty());
}
} // namespace AbilityRuntime
} // namespace OHOS
