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

#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "skill/skill_execute_param.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

class SkillExecuteParamTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: IsSkillExecute_001
 * @tc.desc: Want contains SKILL_EXECUTE_PARAM_SKILL_NAME
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_001, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, std::string("testSkill"));
    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));
}

/**
 * @tc.name: IsSkillExecute_002
 * @tc.desc: Empty Want returns false
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_002, TestSize.Level1)
{
    AAFwk::Want want;
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));
}

/**
 * @tc.name: IsSkillExecute_003
 * @tc.desc: Want with other params but no skillName returns false
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, IsSkillExecute_003, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, std::string("com.example"));
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));
}

/**
 * @tc.name: WriteToWant_001
 * @tc.desc: Write basic params and verify all fields readable
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_001, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "path/to/script", "funcName", nullptr, {}, "req001", "/data/app");

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME), "com.example");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_MODULE_NAME), "entry");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME), "testSkill");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_ARKTS_PATH), "path/to/script");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_FUNC_NAME), "funcName");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE), "req001");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_HAP_PATH), "/data/app");
}

/**
 * @tc.name: WriteToWant_002
 * @tc.desc: Write with skillArgs containing multiple types
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_002, TestSize.Level1)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("name", AAFwk::String::Box(std::string("hello")));
    skillArgs->SetParam("count", AAFwk::Integer::Box(42));
    skillArgs->SetParam("score", AAFwk::Double::Box(3.14));
    skillArgs->SetParam("valid", AAFwk::Boolean::Box(true));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", skillArgs, {}, "req002", "");

    auto params = want.GetParams();
    EXPECT_FALSE(params.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS).empty());

    auto argsKey = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + "name";
    EXPECT_NE(params.GetParams().find(argsKey), params.GetParams().end());
}

/**
 * @tc.name: WriteToWant_003
 * @tc.desc: Write with nullptr skillArgs does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_003, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, {}, "", "");
    auto params = want.GetParams();
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS).empty());
}

/**
 * @tc.name: WriteToWant_004
 * @tc.desc: Write with empty skillArgs (no entries) does not write argsKeys
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_004, TestSize.Level1)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", skillArgs, {}, "", "");
    auto params = want.GetParams();
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_ARGS_KEYS).empty());
}

/**
 * @tc.name: WriteToWant_005
 * @tc.desc: Write with srcEntries
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_005, TestSize.Level1)
{
    std::vector<std::string> srcEntries = { "/src/main/ets/a.ets", "/src/main/ets/b.ets" };
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, srcEntries, "", "");

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT), "2");
    auto key0 = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "0";
    auto key1 = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "1";
    EXPECT_EQ(params.GetStringParam(key0), "/src/main/ets/a.ets");
    EXPECT_EQ(params.GetStringParam(key1), "/src/main/ets/b.ets");
}

/**
 * @tc.name: WriteToWant_006
 * @tc.desc: Write with requestCode and hapPath
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_006, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, {}, "req123", "/data/app/com.example.hap");

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE), "req123");
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_HAP_PATH), "/data/app/com.example.hap");
}

/**
 * @tc.name: WriteToWant_007
 * @tc.desc: Empty arkTSPath and funcName are not written
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, WriteToWant_007, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, {}, "", "");

    auto params = want.GetParams();
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_ARKTS_PATH).empty());
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_FUNC_NAME).empty());
}

/**
 * @tc.name: GenerateFromWant_001
 * @tc.desc: Generate from complete Want with all fields
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_001, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "path/to/script", "funcName", nullptr, {}, "req001", "/data/app");

    SkillExecuteParam param;
    bool ret = SkillExecuteParam::GenerateFromWant(want, param);
    EXPECT_TRUE(ret);
    EXPECT_EQ(param.bundleName_, "com.example");
    EXPECT_EQ(param.moduleName_, "entry");
    EXPECT_EQ(param.skillName_, "testSkill");
    EXPECT_EQ(param.arkTSPath_, "path/to/script");
    EXPECT_EQ(param.funcName_, "funcName");
    EXPECT_EQ(param.requestCode_, "req001");
    EXPECT_EQ(param.hapPath_, "/data/app");
}

/**
 * @tc.name: GenerateFromWant_002
 * @tc.desc: Generate with skillArgs, verify key mapping
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_002, TestSize.Level1)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("name", AAFwk::String::Box(std::string("hello")));
    skillArgs->SetParam("count", AAFwk::Integer::Box(42));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", skillArgs, {}, "", "");

    SkillExecuteParam param;
    bool ret = SkillExecuteParam::GenerateFromWant(want, param);
    EXPECT_TRUE(ret);
    ASSERT_NE(param.skillArgs_, nullptr);
    EXPECT_EQ(param.skillArgs_->GetParams().size(), 2u);

    auto nameIt = param.skillArgs_->GetParams().find("name");
    ASSERT_NE(nameIt, param.skillArgs_->GetParams().end());
    auto nameVal = AAFwk::String::Query(nameIt->second);
    ASSERT_NE(nameVal, nullptr);
    EXPECT_EQ(AAFwk::String::Unbox(nameVal), "hello");

    auto countIt = param.skillArgs_->GetParams().find("count");
    ASSERT_NE(countIt, param.skillArgs_->GetParams().end());
    auto countVal = AAFwk::Integer::Query(countIt->second);
    ASSERT_NE(countVal, nullptr);
    EXPECT_EQ(AAFwk::Integer::Unbox(countVal), 42);
}

/**
 * @tc.name: GenerateFromWant_003
 * @tc.desc: Want without skillName returns false
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_003, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME, std::string("com.example"));
    SkillExecuteParam param;
    EXPECT_FALSE(SkillExecuteParam::GenerateFromWant(want, param));
}

/**
 * @tc.name: GenerateFromWant_004
 * @tc.desc: Want without skillArgs results in nullptr skillArgs_
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_004, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, {}, "", "");

    SkillExecuteParam param;
    bool ret = SkillExecuteParam::GenerateFromWant(want, param);
    EXPECT_TRUE(ret);
    EXPECT_EQ(param.skillArgs_, nullptr);
}

/**
 * @tc.name: GenerateFromWant_005
 * @tc.desc: Generate with srcEntries
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, GenerateFromWant_005, TestSize.Level1)
{
    std::vector<std::string> srcEntries = { "/src/a.ets", "/src/b.ets" };
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, srcEntries, "", "");

    SkillExecuteParam param;
    bool ret = SkillExecuteParam::GenerateFromWant(want, param);
    EXPECT_TRUE(ret);
    EXPECT_EQ(param.srcEntries_.size(), 2u);
    EXPECT_EQ(param.srcEntries_[0], "/src/a.ets");
    EXPECT_EQ(param.srcEntries_[1], "/src/b.ets");
}

/**
 * @tc.name: RoundTrip_001
 * @tc.desc: WriteToWant then GenerateFromWant round trip, all fields match
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RoundTrip_001, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.test", "mod", "skill1",
        "path", "func", nullptr, { "/a.ets", "/b.ets" }, "req100", "/hap");

    SkillExecuteParam param;
    ASSERT_TRUE(SkillExecuteParam::GenerateFromWant(want, param));
    EXPECT_EQ(param.bundleName_, "com.test");
    EXPECT_EQ(param.moduleName_, "mod");
    EXPECT_EQ(param.skillName_, "skill1");
    EXPECT_EQ(param.arkTSPath_, "path");
    EXPECT_EQ(param.funcName_, "func");
    EXPECT_EQ(param.requestCode_, "req100");
    EXPECT_EQ(param.hapPath_, "/hap");
    EXPECT_EQ(param.srcEntries_.size(), 2u);
}

/**
 * @tc.name: RoundTrip_002
 * @tc.desc: Round trip with multiple skillArgs types
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RoundTrip_002, TestSize.Level1)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("str", AAFwk::String::Box(std::string("text")));
    skillArgs->SetParam("num", AAFwk::Integer::Box(100));
    skillArgs->SetParam("dbl", AAFwk::Double::Box(2.718));
    skillArgs->SetParam("flag", AAFwk::Boolean::Box(true));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.test", "mod", "skill2",
        "", "", skillArgs, {}, "", "");

    SkillExecuteParam param;
    ASSERT_TRUE(SkillExecuteParam::GenerateFromWant(want, param));
    ASSERT_NE(param.skillArgs_, nullptr);
    EXPECT_EQ(param.skillArgs_->GetParams().size(), 4u);

    auto strVal = AAFwk::String::Query(param.skillArgs_->GetParam("str"));
    ASSERT_NE(strVal, nullptr);
    EXPECT_EQ(AAFwk::String::Unbox(strVal), "text");

    auto numVal = AAFwk::Integer::Query(param.skillArgs_->GetParam("num"));
    ASSERT_NE(numVal, nullptr);
    EXPECT_EQ(AAFwk::Integer::Unbox(numVal), 100);

    auto dblVal = AAFwk::IDouble::Query(param.skillArgs_->GetParam("dbl"));
    ASSERT_NE(dblVal, nullptr);
    EXPECT_DOUBLE_EQ(AAFwk::Double::Unbox(dblVal), 2.718);

    auto flagVal = AAFwk::IBoolean::Query(param.skillArgs_->GetParam("flag"));
    ASSERT_NE(flagVal, nullptr);
    EXPECT_TRUE(AAFwk::Boolean::Unbox(flagVal));
}

/**
 * @tc.name: RemoveSkillParam_001
 * @tc.desc: Remove basic params from Want
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_001, TestSize.Level1)
{
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "path", "func", nullptr, {}, "req001", "/hap");

    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));
    auto params = want.GetParams();
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_BUNDLE_NAME).empty());
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_SKILL_NAME).empty());
    EXPECT_TRUE(params.GetStringParam(SKILL_EXECUTE_PARAM_REQUEST_CODE).empty());
}

/**
 * @tc.name: RemoveSkillParam_002
 * @tc.desc: Remove skillArgs params from Want
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_002, TestSize.Level1)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box(std::string("val1")));
    skillArgs->SetParam("key2", AAFwk::Integer::Box(10));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", skillArgs, {}, "", "");

    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));
    auto params = want.GetParams();
    auto argsKey1 = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + "key1";
    auto argsKey2 = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + "key2";
    EXPECT_EQ(params.GetParams().find(argsKey1), params.GetParams().end());
    EXPECT_EQ(params.GetParams().find(argsKey2), params.GetParams().end());
}

/**
 * @tc.name: RemoveSkillParam_003
 * @tc.desc: Remove srcEntries params from Want
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_003, TestSize.Level1)
{
    std::vector<std::string> srcEntries = { "/a.ets", "/b.ets" };
    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, "com.example", "entry", "testSkill",
        "", "", nullptr, srcEntries, "", "");

    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));
    auto params = want.GetParams();
    auto key0 = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "0";
    auto key1 = std::string(SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX) + "1";
    EXPECT_EQ(params.GetParams().find(key0), params.GetParams().end());
    EXPECT_EQ(params.GetParams().find(key1), params.GetParams().end());
}

/**
 * @tc.name: RemoveSkillParam_004
 * @tc.desc: Remove from empty Want does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, RemoveSkillParam_004, TestSize.Level1)
{
    AAFwk::Want want;
    EXPECT_TRUE(SkillExecuteParam::RemoveSkillParam(want));
}

/**
 * @tc.name: Marshalling_001
 * @tc.desc: Parcel serialization round trip
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, Marshalling_001, TestSize.Level1)
{
    SkillExecuteParam src;
    src.bundleName_ = "com.example";
    src.moduleName_ = "entry";
    src.skillName_ = "testSkill";
    src.arkTSPath_ = "path";
    src.funcName_ = "func";
    src.requestCode_ = "req001";
    src.hapPath_ = "/hap";
    src.srcEntries_ = { "/a.ets" };
    src.skillArgs_ = std::make_shared<AAFwk::WantParams>();
    src.skillArgs_->SetParam("key", AAFwk::String::Box(std::string("val")));

    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    auto *dst = SkillExecuteParam::Unmarshalling(parcel);
    ASSERT_NE(dst, nullptr);
    EXPECT_EQ(dst->bundleName_, "com.example");
    EXPECT_EQ(dst->moduleName_, "entry");
    EXPECT_EQ(dst->skillName_, "testSkill");
    EXPECT_EQ(dst->arkTSPath_, "path");
    EXPECT_EQ(dst->funcName_, "func");
    EXPECT_EQ(dst->requestCode_, "req001");
    EXPECT_EQ(dst->hapPath_, "/hap");
    EXPECT_EQ(dst->srcEntries_.size(), 1u);
    ASSERT_NE(dst->skillArgs_, nullptr);
    auto val = AAFwk::String::Query(dst->skillArgs_->GetParam("key"));
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(AAFwk::String::Unbox(val), "val");
    delete dst;
}

} // namespace AppExecFwk
} // namespace OHOS
