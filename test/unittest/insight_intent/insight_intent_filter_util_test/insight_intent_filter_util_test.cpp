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
#include <unordered_map>
#include <vector>

#include "cli_tool_mgr_client.h"
#include "extract_insight_intent_profile.h"
#include "function_call_convert.h"
#include "function_info.h"
#include "insight_intent_profile.h"

using namespace testing;
using namespace OHOS::CliTool;
using namespace OHOS::AbilityRuntime;

namespace {
ExtractInsightIntentInfo MakeBgUiAbilityIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.genericInfo.set<InsightIntentEntryInfo>();
    auto &entry = info.genericInfo.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    return info;
}

ExtractInsightIntentInfo MakeFgUiAbilityIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY;
    info.genericInfo.set<InsightIntentEntryInfo>();
    auto &entry = info.genericInfo.get<InsightIntentEntryInfo>();
    entry.abilityName = ability;
    entry.executeMode = {AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND};
    return info;
}

InsightIntentInfo MakeConfigServiceExtension(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.serviceExtensionIntentInfo.abilityName = ability;
    return info;
}

InsightIntentInfo MakeConfigFormOnly(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.formIntentInfo.abilityName = ability;
    info.formIntentInfo.formName = "form";
    return info;
}

ExtractInsightIntentInfo MakeLinkIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &parameters)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_LINK;
    info.genericInfo.set<InsightIntentLinkInfo>();
    info.genericInfo.get<InsightIntentLinkInfo>().uri = "https://example.com";
    info.genericInfo.get<InsightIntentLinkInfo>().parameters = parameters;
    return info;
}

ExtractInsightIntentInfo MakePageIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability, const std::string &parameters)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_PAGE;
    info.genericInfo.set<InsightIntentPageInfo>();
    auto &page = info.genericInfo.get<InsightIntentPageInfo>();
    page.uiAbility = ability;
    page.pagePath = "pages/index";
    page.parameters = parameters;
    return info;
}

ExtractInsightIntentInfo MakeFunctionIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &parameters)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;
    info.genericInfo.set<InsightIntentFunctionInfo>();
    auto &func = info.genericInfo.get<InsightIntentFunctionInfo>();
    func.functionName = "doSomething";
    func.functionReturnType = "string";
    func.functionParams = {"param1"};
    func.parameters = parameters;
    return info;
}

ExtractInsightIntentInfo MakeFormIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability, const std::string &parameters)
{
    ExtractInsightIntentInfo info;
    info.genericInfo.bundleName = bundle;
    info.genericInfo.moduleName = module;
    info.genericInfo.intentName = intent;
    info.genericInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FORM;
    info.genericInfo.set<InsightIntentFormInfo>();
    auto &form = info.genericInfo.get<InsightIntentFormInfo>();
    form.abilityName = ability;
    form.formName = "form";
    form.parameters = parameters;
    return info;
}

InsightIntentInfo MakeConfigFullIntent(const std::string &bundle, const std::string &module,
    const std::string &intent, const std::string &ability)
{
    InsightIntentInfo info;
    info.bundleName = bundle;
    info.moduleName = module;
    info.intentName = intent;
    info.displayDescription = "desc";
    info.uiAbilityIntentInfo.abilityName = ability;
    info.uiAbilityIntentInfo.supportExecuteMode = {AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND};
    info.inputParams = {"inParam1", "inParam2"};
    info.outputParams = {"outParam1"};
    return info;
}
} // namespace

class InsightIntentFilterUtilTest : public Test {};

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items;
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_FgUiAbilityDropped, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items = {
        MakeFgUiAbilityIntent("bundle", "entry", "intent", "MainAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_BgUiAbilityKept, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items = {
        MakeBgUiAbilityIntent("bundle", "entry", "intent", "BgAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].genericInfo.moduleName, "entry");
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_EmptyInput_NoChange, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items;
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_ServiceExtensionKept, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items = {
        MakeConfigServiceExtension("bundle", "entry", "intent", "ServiceAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].moduleName, "entry");
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_FormOnlyDropped, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items = {
        MakeConfigFormOnly("bundle", "entry", "intent", "FormAbility"),
    };
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    EXPECT_TRUE(items.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, FilterConfig_DropsEmptyNameAndSorts, TestSize.Level1)
{
    std::vector<InsightIntentInfo> items = {
        MakeConfigServiceExtension("bundle", "zeta", "", "A"),   // empty intentName -> dropped
        MakeConfigServiceExtension("bundle", "", "intent", "B"), // empty moduleName -> dropped
        MakeConfigServiceExtension("bundle", "beta", "intent", "C"),
        MakeConfigServiceExtension("bundle", "alpha", "intent", "D"),
    };
    IntentFilterUtil filter;
    filter.FilterConfig(items);
    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0].moduleName, "alpha");
    EXPECT_EQ(items[1].moduleName, "beta");
}

HWTEST_F(InsightIntentFilterUtilTest, FilterGeneric_DropsEmptyNameAndKeepsLinkPageFunc, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> items = {
        MakeLinkIntent("bundle", "", "intent", ""),       // empty moduleName -> dropped
        MakeLinkIntent("bundle", "alpha", "", ""),        // empty intentName -> dropped
        MakeLinkIntent("bundle", "alpha", "intent", ""),
        MakePageIntent("bundle", "beta", "intent", "PgAbility", ""),
        MakeFunctionIntent("bundle", "gamma", "intent", ""),
    };
    IntentFilterUtil filter;
    filter.FilterGeneric(items);
    EXPECT_EQ(items.size(), 3u);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_EmptyInput_NoFunctions, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_SkipsEmptyIntentName, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("bundle", "module", "", "{}"),
        MakeLinkIntent("bundle", "module", "intent1", "{}"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_EQ(functions[0].functionName, "intent1");
    EXPECT_EQ(functions[0].functionNamespace, "bundle");
    EXPECT_EQ(functions[0].functionType, FunctionType::INTENT_FUNCTION);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_LinkTypePropagatesSchema, TestSize.Level1)
{
    const std::string schema = R"({"type":"object","properties":{"p":{"type":"string"}}})";
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("bundle", "module", "intent", schema),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_FALSE(functions[0].inputSchema.empty());
    EXPECT_TRUE(functions[0].inputSchema.find("ohos.insightIntent.options") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_PageTypeWithAbilityDefault, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakePageIntent("bundle", "module", "intent", "PageAbility", ""),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_TRUE(functions[0].inputSchema.find("\"abilityName\"") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_EntryTypeWithAbilityDefault, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeBgUiAbilityIntent("bundle", "module", "intent", "EntryAbility"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_TRUE(functions[0].inputSchema.find("\"abilityName\"") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_FormTypeWithAbilityDefault, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeFormIntent("bundle", "module", "intent", "FormAbility", ""),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_TRUE(functions[0].inputSchema.find("\"abilityName\"") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_FunctionTypePropagatesSchema, TestSize.Level1)
{
    const std::string schema = R"({"type":"object","properties":{"arg":{"type":"string"}}})";
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeFunctionIntent("bundle", "module", "intent", schema),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_FALSE(functions[0].inputSchema.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_InvalidSchemaResetToEmpty, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("bundle", "module", "intent", "not-a-json{"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_FALSE(functions[0].inputSchema.empty());
    EXPECT_TRUE(functions[0].inputSchema.find("ohos.insightIntent.options") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromExtractIntentInfo_NonObjectSchemaReset, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("bundle", "module", "intent", "[1,2,3]"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromExtractIntentInfo(intentInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_TRUE(functions[0].inputSchema.find("\"type\":\"object\"") != std::string::npos);
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromConfigIntent_EmptyInput_NoFunctions, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos;
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    EXPECT_TRUE(functions.empty());
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromConfigIntent_SkipsEmptyIntentName, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos = {
        MakeConfigServiceExtension("bundle", "module", "", "A"),
        MakeConfigFullIntent("bundle", "module", "intent", "B"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_EQ(functions[0].functionName, "intent");
}

HWTEST_F(InsightIntentFilterUtilTest, ConvertFromConfigIntent_BuildsInputOutputSchema, TestSize.Level1)
{
    std::vector<InsightIntentInfo> configInfos = {
        MakeConfigFullIntent("bundle", "module", "intent", "UiAbility"),
    };
    std::vector<FunctionInfo> functions;
    EXPECT_TRUE(ConvertFromConfigIntent(configInfos, functions));
    ASSERT_EQ(functions.size(), 1u);
    EXPECT_TRUE(functions[0].inputSchema.find("inParam1") != std::string::npos);
    EXPECT_TRUE(functions[0].outputSchema.find("outParam1") != std::string::npos);
    EXPECT_EQ(functions[0].functionNamespace, "bundle");
}

HWTEST_F(InsightIntentFilterUtilTest, RegisterInsightIntentFunctions_EmptyConvertReturnsTrue, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configInfos;
    EXPECT_TRUE(RegisterInsightIntentFunctions(intentInfos, configInfos, "bundle", 1));
}

HWTEST_F(InsightIntentFilterUtilTest, RegisterInsightIntentFunctions_WithFunctionsReachable, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("bundle", "module", "intent", "{}"),
    };
    std::vector<InsightIntentInfo> configInfos = {
        MakeConfigFullIntent("bundle", "module", "cfgIntent", "UiAbility"),
    };
    EXPECT_TRUE(RegisterInsightIntentFunctions(intentInfos, configInfos, "bundle", 100));
}

HWTEST_F(InsightIntentFilterUtilTest, RegisterInsightIntentFunctions_NamespaceBackfillWhenEmpty, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("", "module", "intent", "{}"),  // empty bundleName -> backfilled
    };
    std::vector<InsightIntentInfo> configInfos;
    EXPECT_TRUE(RegisterInsightIntentFunctions(intentInfos, configInfos, "backfillBundle", 1));
}

HWTEST_F(InsightIntentFilterUtilTest, UnregisterInsightIntentFunctions_EmptyBundleReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(UnregisterInsightIntentFunctions(""));
}

HWTEST_F(InsightIntentFilterUtilTest, UnregisterInsightIntentFunctions_ReachableReturnsBool, TestSize.Level1)
{
    // Host test env has no CliTool SA: UnregisterIntentFunctionsByNamespace fails (<0) -> false.
    EXPECT_FALSE(UnregisterInsightIntentFunctions("com.test.bundle"));
}

HWTEST_F(InsightIntentFilterUtilTest, BatchRegister_EmptyFunctionsReturnsTrue, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configInfos;
    std::unordered_map<std::string, uint32_t> bundleVersionMap = {{"bundle", 1}};
    int32_t successCount = -1;
    EXPECT_TRUE(BatchRegisterInsightIntentFunctions(intentInfos, configInfos, bundleVersionMap, successCount));
    EXPECT_EQ(successCount, 0);
}

HWTEST_F(InsightIntentFilterUtilTest, BatchRegister_WithFunctionsReachable, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeFunctionIntent("bundle", "module", "intent", "{}"),
    };
    std::vector<InsightIntentInfo> configInfos = {
        MakeConfigFullIntent("bundle", "module", "cfgIntent", "UiAbility"),
    };
    std::unordered_map<std::string, uint32_t> bundleVersionMap = {{"bundle", 100}};
    int32_t successCount = 0;
    bool ret = BatchRegisterInsightIntentFunctions(intentInfos, configInfos, bundleVersionMap, successCount);
    // Host env: BatchRegisterFunctions IPC fails (ret != ERR_OK) -> false; successCount unchanged (0).
    EXPECT_FALSE(ret);
    EXPECT_EQ(successCount, 0);
}

HWTEST_F(InsightIntentFilterUtilTest, BatchRegister_NamespaceBackfillSingleBundle, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("", "module", "intent", "{}"),
    };
    std::vector<InsightIntentInfo> configInfos;
    std::unordered_map<std::string, uint32_t> bundleVersionMap = {{"onlyBundle", 5}};
    int32_t successCount = 0;
    bool ret = BatchRegisterInsightIntentFunctions(intentInfos, configInfos, bundleVersionMap, successCount);
    EXPECT_FALSE(ret);
    EXPECT_EQ(successCount, 0);
}

HWTEST_F(InsightIntentFilterUtilTest, BatchRegister_NamespaceNotFoundInMap, TestSize.Level1)
{
    std::vector<ExtractInsightIntentInfo> intentInfos = {
        MakeLinkIntent("otherBundle", "module", "intent", "{}"),
    };
    std::vector<InsightIntentInfo> configInfos;
    std::unordered_map<std::string, uint32_t> bundleVersionMap = {{"bundle", 5}};
    int32_t successCount = 0;
    bool ret = BatchRegisterInsightIntentFunctions(intentInfos, configInfos, bundleVersionMap, successCount);
    EXPECT_FALSE(ret);
    EXPECT_EQ(successCount, 0);
}
