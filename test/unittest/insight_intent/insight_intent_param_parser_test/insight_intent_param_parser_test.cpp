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

#include <memory>
#include <string>
#include <vector>

#include "array_wrapper.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_param_parser.h"
#include "integer_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
ExtractInsightIntentGenericInfo MakeBgUiAbilityCandidate(const std::string &bundle,
    const std::string &module, const std::string &intentName, const std::string &ability)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.set<InsightIntentEntryInfo>();
    auto &entry = info.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    return info;
}

ExtractInsightIntentGenericInfo MakeFgUiAbilityCandidate(const std::string &bundle,
    const std::string &module, const std::string &intentName, const std::string &ability)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.set<InsightIntentEntryInfo>();
    auto &entry = info.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};
    return info;
}

ExtractInsightIntentGenericInfo MakeMultiModeEntryCandidate(const std::string &bundle,
    const std::string &module, const std::string &intentName, const std::string &ability,
    const std::vector<AppExecFwk::ExecuteMode> &modes)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.set<InsightIntentEntryInfo>();
    auto &entry = info.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = modes;
    return info;
}

ExtractInsightIntentGenericInfo MakeLinkCandidate(const std::string &bundle, const std::string &module,
    const std::string &intentName, const std::string &parameters)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_LINK;
    info.set<InsightIntentLinkInfo>();
    info.get<InsightIntentLinkInfo>().uri = "https://example.com";
    info.get<InsightIntentLinkInfo>().parameters = parameters;
    return info;
}

ExtractInsightIntentGenericInfo MakePageCandidate(const std::string &bundle, const std::string &module,
    const std::string &intentName, const std::string &ability, const std::string &parameters)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_PAGE;
    info.set<InsightIntentPageInfo>();
    auto &page = info.get<InsightIntentPageInfo>();
    page.uiAbility = ability;
    page.parameters = parameters;
    return info;
}

ExtractInsightIntentGenericInfo MakeFunctionCandidate(const std::string &bundle, const std::string &module,
    const std::string &intentName, const std::string &parameters)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;
    info.set<InsightIntentFunctionInfo>();
    auto &func = info.get<InsightIntentFunctionInfo>();
    func.functionName = "doSomething";
    func.functionReturnType = "string";
    func.functionParams = {"arg1"};
    func.parameters = parameters;
    return info;
}

ExtractInsightIntentGenericInfo MakeFormCandidate(const std::string &bundle, const std::string &module,
    const std::string &intentName, const std::string &ability, const std::string &parameters)
{
    ExtractInsightIntentGenericInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intentName;
    info.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FORM;
    info.set<InsightIntentFormInfo>();
    auto &form = info.get<InsightIntentFormInfo>();
    form.abilityName = ability;
    form.formName = "form";
    form.parameters = parameters;
    return info;
}
} // namespace

class InsightIntentParamParserTest : public Test {};

HWTEST_F(InsightIntentParamParserTest, Build_EmptyBundleName_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("", "intentName", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_EmptyIntentName_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_NoCandidate_ReturnsInvalidValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, {}, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_AllFgCandidates_ReturnsInvalidValue, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeFgUiAbilityCandidate("bundle", "entry", "intentName", "MainAbility"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_NE(ret, 0);
}

HWTEST_F(InsightIntentParamParserTest, Build_SingleBgUiAbilityCandidate_ReturnsParam, TestSize.Level1)
{
    const std::string expectedModule = "entry";
    const std::string expectedAbility = "BgAbility";
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeBgUiAbilityCandidate("bundle", expectedModule, "intentName", expectedAbility),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->moduleName_, expectedModule);
    EXPECT_EQ(out.param->abilityName_, expectedAbility);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
    EXPECT_FALSE(out.ignoreAbilityName);
    EXPECT_FALSE(out.openLinkExecuteFlag);
}

HWTEST_F(InsightIntentParamParserTest, Build_MultiModeEntryWithBg_PrefersBg, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeMultiModeEntryCandidate("bundle", "entry", "intentName", "MainAbility",
            {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND}),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
}

HWTEST_F(InsightIntentParamParserTest, Build_MultiModeEntryWithoutBg_FallsBackToFront, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeMultiModeEntryCandidate("bundle", "entry", "intentName", "MainAbility",
            {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND, AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY}),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
}

HWTEST_F(InsightIntentParamParserTest, Build_LinkCandidate_SetsOpenLinkFlag, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeLinkCandidate("bundle", "entry", "intentName", "{}"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_TRUE(out.ignoreAbilityName);
    EXPECT_TRUE(out.openLinkExecuteFlag);
}

HWTEST_F(InsightIntentParamParserTest, Build_PageCandidate_SetsIgnoreAbilityName, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakePageCandidate("bundle", "entry", "intentName", "PageAbility", "{}"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_TRUE(out.ignoreAbilityName);
    EXPECT_FALSE(out.openLinkExecuteFlag);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND);
}

HWTEST_F(InsightIntentParamParserTest, Build_FunctionCandidate_ForcesBgMode, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeFunctionCandidate("bundle", "entry", "intentName", "{}"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_TRUE(out.ignoreAbilityName);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
}

HWTEST_F(InsightIntentParamParserTest, Build_FormCandidate_PropagatesAbilityName, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeFormCandidate("bundle", "entry", "intentName", "FormAbility", "{}"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    // Form candidates are dropped by FilterGeneric (not Link/Page/Function/qualified-Entry).
    EXPECT_NE(ret, 0);
    EXPECT_EQ(out.param, nullptr);
}

HWTEST_F(InsightIntentParamParserTest, Build_EntrySingleMode_ReturnsThatMode, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeMultiModeEntryCandidate("bundle", "entry", "intentName", "MainAbility",
            {AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY}),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->executeMode_, AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY);
}

HWTEST_F(InsightIntentParamParserTest, ResolveUris_NoUrisKey_LeavesEmpty, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    std::vector<std::string> out;
    parser.ResolveUris(opts, out);
    EXPECT_TRUE(out.empty());
}

HWTEST_F(InsightIntentParamParserTest, ResolveUris_NonArrayValue_LeavesEmpty, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    opts.SetParam("uris", AAFwk::String::Box("not-an-array"));
    std::vector<std::string> out;
    parser.ResolveUris(opts, out);
    EXPECT_TRUE(out.empty());
}

HWTEST_F(InsightIntentParamParserTest, ResolveUris_EmptyArray_LeavesEmpty, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    sptr<AAFwk::IArray> emptyArray = new (std::nothrow) AAFwk::Array(0, AAFwk::g_IID_IString);
    ASSERT_NE(emptyArray, nullptr);
    opts.SetParam("uris", emptyArray);
    std::vector<std::string> out;
    parser.ResolveUris(opts, out);
    EXPECT_TRUE(out.empty());
}

HWTEST_F(InsightIntentParamParserTest, ResolveUris_StringArrayWithValues_PopulatesOut, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    sptr<AAFwk::Array> array = new (std::nothrow) AAFwk::Array(2, AAFwk::g_IID_IString);
    ASSERT_NE(array, nullptr);
    array->Set(0, AAFwk::String::Box("uri1").GetRefPtr());
    array->Set(1, AAFwk::String::Box("uri2").GetRefPtr());
    opts.SetParam("uris", array);
    std::vector<std::string> out;
    parser.ResolveUris(opts, out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "uri1");
    EXPECT_EQ(out[1], "uri2");
}

HWTEST_F(InsightIntentParamParserTest, ResolveFlags_HasFlags_ReturnsValue, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    opts.SetParam("flags", AAFwk::Integer::Box(7));
    int32_t out = 0;
    parser.ResolveFlags(opts, out);
    EXPECT_EQ(out, 7);
}

HWTEST_F(InsightIntentParamParserTest, ResolveFlags_NoFlags_ReturnsDefault, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams opts;
    int32_t out = 9;
    parser.ResolveFlags(opts, out);
    EXPECT_EQ(out, 0);
}

HWTEST_F(InsightIntentParamParserTest, ExtractOptions_NoOptionsKey_ReturnsEmpty, TestSize.Level1)
{
    InsightIntentParamParser parser;
    AAFwk::WantParams wantParam;
    auto opts = parser.ExtractOptions(wantParam);
    ASSERT_NE(opts, nullptr);
    EXPECT_TRUE(opts->KeySet().empty());
}

HWTEST_F(InsightIntentParamParserTest, Build_KeepsMatchedModuleWhenNoOption, TestSize.Level1)
{
    std::vector<ExtractInsightIntentGenericInfo> candidates = {
        MakeBgUiAbilityCandidate("bundle", "customModule", "intentName", "BgAbility"),
    };
    InsightIntentParamParser parser;
    WantParams wantParam;
    InsightIntentParamParser::ParseResult out;
    auto ret = parser.Build("bundle", "intentName", wantParam, candidates, 0, out);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(out.param, nullptr);
    EXPECT_EQ(out.param->moduleName_, "customModule");
    EXPECT_EQ(out.param->bundleName_, "bundle");
    EXPECT_EQ(out.param->insightIntentName_, "intentName");
    EXPECT_EQ(out.param->userId_, -1);
}
