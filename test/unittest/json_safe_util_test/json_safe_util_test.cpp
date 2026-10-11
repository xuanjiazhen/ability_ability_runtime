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

#include "json_safe_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing;
using namespace testing::ext;

class JsonSafeUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsonSafeUtilTest::SetUpTestCase()
{}
void JsonSafeUtilTest::TearDownTestCase()
{}
void JsonSafeUtilTest::SetUp()
{}
void JsonSafeUtilTest::TearDown()
{}

/**
 * @tc.name: SafeParse_0100
 * @tc.desc: Parse valid object / array / scalar inputs.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeParse_0100, TestSize.Level1)
{
    nlohmann::json jsonObject;
    EXPECT_TRUE(SafeParse(R"({"key":"value"})", jsonObject));
    EXPECT_TRUE(jsonObject.is_object());
    EXPECT_EQ(jsonObject["key"].get<std::string>(), "value");

    EXPECT_TRUE(SafeParse(R"([1,2,3])", jsonObject));
    EXPECT_TRUE(jsonObject.is_array());
    EXPECT_EQ(jsonObject.size(), 3U);

    EXPECT_TRUE(SafeParse("42", jsonObject));
    EXPECT_TRUE(jsonObject.is_number_integer());
}

/**
 * @tc.name: SafeParse_0200
 * @tc.desc: Parse invalid json returns false and out stays discarded-free.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeParse_0200, TestSize.Level1)
{
    nlohmann::json jsonObject;
    EXPECT_FALSE(SafeParse(R"({"key":})", jsonObject));
    EXPECT_FALSE(SafeParse("", jsonObject));
    EXPECT_FALSE(SafeParse("not a json", jsonObject));
}

/**
 * @tc.name: SafeParse_0300
 * @tc.desc: Parsing a moderately large (multi-MB) but valid input succeeds,
 *           since the length limit was raised to 128MB.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeParse_0300, TestSize.Level1)
{
    nlohmann::json jsonObject;
    std::string okJson = R"({"key":")" + std::string(64, 'a') + "\"}";
    EXPECT_TRUE(SafeParse(okJson, jsonObject));
    std::string bigJson = R"({"key":")" + std::string(2 * 1024 * 1024, 'a') + "\"}";
    EXPECT_TRUE(SafeParse(bigJson, jsonObject));
}

/**
 * @tc.name: SafeParse_0400
 * @tc.desc: Input deeper than the built-in depth limit is rejected by the SAX pre-check.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeParse_0400, TestSize.Level1)
{
    nlohmann::json jsonObject;
    std::string shallowPrefix(JSON_SAFE_MAX_DEPTH, '[');
    std::string shallowSuffix(JSON_SAFE_MAX_DEPTH, ']');
    EXPECT_TRUE(SafeParse(shallowPrefix + R"("deep")" + shallowSuffix, jsonObject));
    std::string deepPrefix(JSON_SAFE_MAX_DEPTH + 1, '[');
    std::string deepSuffix(JSON_SAFE_MAX_DEPTH + 1, ']');
    EXPECT_FALSE(SafeParse(deepPrefix + R"("deep")" + deepSuffix, jsonObject));
}

/**
 * @tc.name: SafeDump_0100
 * @tc.desc: Dump object without truncation round-trips through SafeParse.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeDump_0100, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"key":"value","num":3})", jsonObject));
    std::string out;
    EXPECT_TRUE(SafeDump(jsonObject, out));
    EXPECT_EQ(out, R"({"key":"value","num":3})");

    nlohmann::json reparsed;
    EXPECT_TRUE(SafeParse(out, reparsed));
    EXPECT_EQ(reparsed, jsonObject);
}

/**
 * @tc.name: SafeDump_0200
 * @tc.desc: Dump truncates with ellipsis when maxLen is exceeded.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeDump_0200, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"key":"value"})", jsonObject));
    std::string out;
    EXPECT_TRUE(SafeDump(jsonObject, out, 8));
    EXPECT_EQ(out.size(), 11U);  // 8 chars + "..."
    EXPECT_TRUE(out.rfind("...") == out.size() - 3);
}

/**
 * @tc.name: SafeDump_0300
 * @tc.desc: The string-returning SafeDump overload dumps normally and
 *           truncates when maxLen is exceeded.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeDump_0300, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"key":"value"})", jsonObject));

    std::string dumped = SafeDump(jsonObject);
    EXPECT_EQ(dumped, R"({"key":"value"})");

    std::string truncated = SafeDump(jsonObject, 8);
    EXPECT_EQ(truncated.size(), 11U);  // 8 chars + "..."
    EXPECT_TRUE(truncated.rfind("...") == truncated.size() - 3);
}

/**
 * @tc.name: SafeDump_0400
 * @tc.desc: Depth counts object/array nesting levels (matching SafeParse).
 *           A value exactly at the limit passes, one level beyond fails.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeDump_0400, TestSize.Level1)
{
    // Build nested json without going through SafeParse (which already rejects
    // deep input).
    nlohmann::json okJson = nlohmann::json("deep");
    for (size_t i = 0; i < JSON_SAFE_MAX_DEPTH; ++i) {
        nlohmann::json wrapper = nlohmann::json::array();
        wrapper.push_back(okJson);
        okJson = wrapper;
    }
    std::string out;
    EXPECT_TRUE(SafeDump(okJson, out));
    EXPECT_FALSE(out.empty());

    // One nesting level beyond the limit fails.
    nlohmann::json deepJson = nlohmann::json("deep");
    for (size_t i = 0; i <= JSON_SAFE_MAX_DEPTH; ++i) {
        nlohmann::json wrapper = nlohmann::json::array();
        wrapper.push_back(deepJson);
        deepJson = wrapper;
    }
    EXPECT_FALSE(SafeDump(deepJson, out));
    EXPECT_EQ(SafeDump(deepJson), "");
}

/**
 * @tc.name: SafeJsonGet_0100
 * @tc.desc: SafeJsonGet returns the typed value for a matching json node.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeJsonGet_0100, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"str":"value","num":42})", jsonObject));

    std::string strValue;
    EXPECT_TRUE(SafeJsonGet(jsonObject["str"], strValue, "str"));
    EXPECT_EQ(strValue, "value");

    int intValue = 0;
    EXPECT_TRUE(SafeJsonGet(jsonObject["num"], intValue, "num"));
    EXPECT_EQ(intValue, 42);
}

/**
 * @tc.name: SafeJsonGet_0200
 * @tc.desc: SafeJsonGet returns false when the json type does not match the
 *           requested type.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, SafeJsonGet_0200, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"str":"value"})", jsonObject));

    int intValue = 0;
    EXPECT_FALSE(SafeJsonGet(jsonObject["str"], intValue, "str"));
}

struct TestSkillInfo {
    std::string uri;
    int32_t num = 0;
};

void from_json(const nlohmann::json &jsonObject, TestSkillInfo &info)
{
    auto end = jsonObject.end();
    if (jsonObject.find("uri") != end) {
        info.uri = jsonObject.at("uri").get<std::string>();
    }
    if (jsonObject.find("num") != end) {
        info.num = jsonObject.at("num").get<int32_t>();
    }
}

/**
 * @tc.name: GetStrValueIfFindKey_0100
 * @tc.desc: Get string value, type mismatch, missing required/optional, and short-circuit.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, GetStrValueIfFindKey_0100, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"name":"value","num":42})", jsonObject));

    std::string value;
    int32_t parseResult = JSON_PARSE_OK;
    GetStrValueIfFindKey(jsonObject, "name", value, true, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);
    EXPECT_EQ(value, "value");

    parseResult = JSON_PARSE_OK;
    GetStrValueIfFindKey(jsonObject, "num", value, false, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_TYPE_ERROR);

    parseResult = JSON_PARSE_OK;
    GetStrValueIfFindKey(jsonObject, "missing", value, true, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_MISSING);

    parseResult = JSON_PARSE_OK;
    GetStrValueIfFindKey(jsonObject, "missing", value, false, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);

    // 短路：parseResult 已非 0，直接返回不覆盖
    parseResult = JSON_PARSE_PROP_TYPE_ERROR;
    GetStrValueIfFindKey(jsonObject, "name", value, true, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_TYPE_ERROR);
}

/**
 * @tc.name: GetStrValueIfFindKey_0200
 * @tc.desc: Reject string longer than JSON_SAFE_MAX_STRING_LENGTH.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, GetStrValueIfFindKey_0200, TestSize.Level1)
{
    std::string longStr(JSON_SAFE_MAX_STRING_LENGTH + 1, 'a');
    nlohmann::json jsonObject = nlohmann::json::object();
    jsonObject["long"] = longStr;

    std::string value;
    int32_t parseResult = JSON_PARSE_OK;
    GetStrValueIfFindKey(jsonObject, "long", value, false, parseResult);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_SIZE_CHECK_ERROR);
}

/**
 * @tc.name: GetValueIfFindKey_0100
 * @tc.desc: Get object and string-array values.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, GetValueIfFindKey_0100, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"link":{"uri":"u1"},"tags":["a","b"]})", jsonObject));
    auto end = jsonObject.end();

    TestSkillInfo info;
    int32_t parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<TestSkillInfo>(jsonObject, end, "link", info, JsonType::OBJECT,
        false, parseResult, ArrayType::NOT_ARRAY);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);
    EXPECT_EQ(info.uri, "u1");

    std::vector<std::string> tags;
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, end, "tags", tags, JsonType::ARRAY,
        false, parseResult, ArrayType::STRING);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);
    ASSERT_EQ(tags.size(), 2U);
    EXPECT_EQ(tags[0], "a");
    EXPECT_EQ(tags[1], "b");
}

/**
 * @tc.name: GetValueIfFindKey_0200
 * @tc.desc: Get object-array / number values, type mismatch, and missing required.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, GetValueIfFindKey_0200, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"ents":[{"uri":"a"},{"uri":"b"}],"num":42})", jsonObject));
    auto end = jsonObject.end();

    std::vector<TestSkillInfo> ents;
    int32_t parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<std::vector<TestSkillInfo>>(jsonObject, end, "ents", ents, JsonType::ARRAY,
        false, parseResult, ArrayType::OBJECT);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);
    ASSERT_EQ(ents.size(), 2U);
    EXPECT_EQ(ents[0].uri, "a");

    int32_t num = 0;
    GetValueIfFindKey<int32_t>(jsonObject, end, "num", num, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    EXPECT_EQ(parseResult, JSON_PARSE_OK);
    EXPECT_EQ(num, 42);

    TestSkillInfo info;
    parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<TestSkillInfo>(jsonObject, end, "num", info, JsonType::OBJECT,
        false, parseResult, ArrayType::NOT_ARRAY);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_TYPE_ERROR);

    parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<TestSkillInfo>(jsonObject, end, "missing", info, JsonType::OBJECT,
        true, parseResult, ArrayType::NOT_ARRAY);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_MISSING);
}

/**
 * @tc.name: GetValueIfFindKey_0300
 * @tc.desc: Reject array element type mismatch and oversized array.
 * @tc.type: FUNC
 */
HWTEST_F(JsonSafeUtilTest, GetValueIfFindKey_0300, TestSize.Level1)
{
    nlohmann::json jsonObject;
    ASSERT_TRUE(SafeParse(R"({"tags":[1,2]})", jsonObject));

    std::vector<std::string> tags;
    int32_t parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObject.end(), "tags", tags,
        JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_TYPE_ERROR);

    nlohmann::json bigArray = nlohmann::json::array();
    for (size_t i = 0; i < JSON_SAFE_MAX_ARRAY_LENGTH + 1; ++i) {
        bigArray.push_back("x");
    }
    nlohmann::json bigObj = nlohmann::json::object();
    bigObj["arr"] = bigArray;
    parseResult = JSON_PARSE_OK;
    GetValueIfFindKey<std::vector<std::string>>(bigObj, bigObj.end(), "arr", tags,
        JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    EXPECT_EQ(parseResult, JSON_PARSE_PROP_SIZE_CHECK_ERROR);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
