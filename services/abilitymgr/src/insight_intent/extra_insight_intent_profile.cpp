/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "extra_insight_intent_profile.h"

#include "hilog_tag_wrapper.h"
#include "json_util.h"

namespace OHOS {
namespace AbilityRuntime {
using JsonType = AppExecFwk::JsonType;
using ArrayType = AppExecFwk::ArrayType;

namespace {
int32_t g_extraParseResult = ERR_OK;
std::mutex g_extraMutex;

const std::string INSIGHT_INTENTS = "insightIntents";
const std::string INSIGHT_INTENT_NAME = "intentName";
const std::string INSIGHT_INTENT_DOMAIN = "domain";
const std::string INSIGHT_INTENT_VERSION = "intentVersion";
const std::string INSIGHT_INTENT_EXECUTE_MODE = "executeMode";
const std::string INSIGHT_INTENT_DECORETOR_FILE = "decoratorFile";
const std::string INSIGHT_INTENT_DECORETOR_CLASS = "decoratorClass";
const std::string INSIGHT_INTENT_DECORETOR_TYPE = "decoratorType";
const std::string INSIGHT_INTENT_BUNDLE_NAME = "bundleName";
const std::string INSIGHT_INTENT_MODULE_NAME = "moduleName";
const std::string INSIGHT_INTENT_DISPLAY_NAME = "displayName";
const std::string INSIGHT_INTENT_DISPLAY_DESCRIPTION = "displayDescription";
const std::string INSIGHT_INTENT_SCHEMA = "schema";
const std::string INSIGHT_INTENT_ICON = "icon";
const std::string INSIGHT_INTENT_LLM_DESCRIPTION = "llmDescription";
const std::string INSIGHT_INTENT_KEYWORDS = "keywords";
const std::string INSIGHT_INTENT_PARAMETERS = "parameters";
const std::string INSIGHT_INTENT_URI = "uri";
const std::string INSIGHT_INTENT_PARAM_MAPPING = "paramMapping";
const std::string INSIGHT_INTENT_UI_ABILITY = "uiAbility";
const std::string INSIGHT_INTENT_PAGE_ROUTE_NAME = "pageRouteName";
const std::string INSIGHT_INTENT_NAVIGATION_ID = "navigationId";
const std::string INSIGHT_INTENT_NAV_DESTINATION = "navDestination";
const std::string INSIGHT_INTENT_ABILITY_NAME = "abilityName";
const std::string INSIGHT_INTENT_FUNCTION_NAME = "functionName";
const std::string INSIGHT_INTENT_FUNCTION_PARAMS = "functionParams";
const std::string INSIGHT_INTENT_PARAM_NAME = "paramName";
const std::string INSIGHT_INTENT_PARAM_MAPPING_NAME = "paramMappingName";
const std::string INSIGHT_INTENT_PARAM_CATEGORY = "paramCategory";

enum DecoratorType {
    DECORATOR_LINK = 0,
    DECORATOR_PAGE,
    DECORATOR_ENTRY,
    DECORATOR_FUNCTION,
    DECORATOR_FORM,
    DECORATOR_UNKNOWN
};

DecoratorType StringToEnum(const std::string& input)
{
    static const std::unordered_map<std::string, DecoratorType> mapping = {
        {"InsightIntentLinkDecorator", DecoratorType::DECORATOR_LINK},
        {"InsightIntentPageDecorator", DecoratorType::DECORATOR_PAGE},
        {"InsightIntentEntryDecorator", DecoratorType::DECORATOR_ENTRY},
        {"InsightIntentFunctionDecorator", DecoratorType::DECORATOR_FUNCTION},
        {"InsightIntentFormDecorator", DecoratorType::DECORATOR_FORM}
    };
    auto it = mapping.find(input);
    return (it != mapping.end()) ? it->second : DecoratorType::DECORATOR_UNKNOWN;
}

const std::map<std::string, ExecuteMode> executeModeMap = {
    {"foreground", ExecuteMode::UI_ABILITY_FOREGROUND},
    {"background", ExecuteMode::UI_ABILITY_BACKGROUND}
};
} // namespace

void from_json(const nlohmann::json &jsonObject, LinkIntentParamProfileMapping &paramMapping)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_NAME,
        paramMapping.paramName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_MAPPING_NAME,
        paramMapping.paramMappingName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_CATEGORY,
        paramMapping.paramCategory,
        false,
        g_extraParseResult);
}

void from_json(const nlohmann::json &jsonObject, ExtraInsightIntentProfileInfo &insightIntentInfo)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_FILE,
        insightIntentInfo.decoratorFile,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_CLASS,
        insightIntentInfo.decoratorClass,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DECORETOR_TYPE,
        insightIntentInfo.decoratorType,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_BUNDLE_NAME,
        insightIntentInfo.bundleName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_MODULE_NAME,
        insightIntentInfo.moduleName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAME,
        insightIntentInfo.intentName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DOMAIN,
        insightIntentInfo.domain,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_VERSION,
        insightIntentInfo.intentVersion,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_NAME,
        insightIntentInfo.displayName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_DISPLAY_DESCRIPTION,
        insightIntentInfo.displayDescription,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SCHEMA,
        insightIntentInfo.schema,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ICON,
        insightIntentInfo.icon,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_LLM_DESCRIPTION,
        insightIntentInfo.llmDescription,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_KEYWORDS,
        insightIntentInfo.keywords,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAMETERS,
        insightIntentInfo.parameters,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_URI,
        insightIntentInfo.uri,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<LinkIntentParamProfileMapping>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAM_MAPPING,
        insightIntentInfo.paramMapping,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::OBJECT);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_ABILITY,
        insightIntentInfo.uiAbility,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PAGE_ROUTE_NAME,
        insightIntentInfo.pageRouteName,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAVIGATION_ID,
        insightIntentInfo.navigationId,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAV_DESTINATION,
        insightIntentInfo.navDestination,
        false,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY_NAME,
        insightIntentInfo.abilityName,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_EXECUTE_MODE,
        insightIntentInfo.executeMode,
        JsonType::ARRAY,
        true,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_NAME,
        insightIntentInfo.functionName,
        false,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_PARAMS,
        insightIntentInfo.functionParams,
        JsonType::ARRAY,
        true,
        g_extraParseResult,
        ArrayType::STRING);
}

void from_json(const nlohmann::json &jsonObject, ExtraInsightIntentProfileInfoVec &infos)
{
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::GetValueIfFindKey<std::vector<ExtraInsightIntentProfileInfo>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENTS,
        infos.insightIntents,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::OBJECT);
}

void to_json(nlohmann::json& jsonObject, const LinkIntentParamProfileMapping &info)
{
    TAG_LOGI(AAFwkTag::INTENT, "call to link mapping");
    jsonObject = nlohmann::json {
        {"paramName", info.paramName},
        {"paramMappingName", info.paramMappingName},
        {"paramCategory", info.paramCategory}
    };
}

void to_json(nlohmann::json& jsonObject, const ExtraInsightIntentProfileInfo& info)
{
    TAG_LOGI(AAFwkTag::INTENT, "call to ExtraInsightIntentProfileInfo");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_DECORETOR_FILE, info.decoratorFile},
        {INSIGHT_INTENT_DECORETOR_CLASS, info.decoratorClass},
        {INSIGHT_INTENT_DECORETOR_TYPE, info.decoratorType},
        {INSIGHT_INTENT_BUNDLE_NAME, info.bundleName},
        {INSIGHT_INTENT_MODULE_NAME, info.moduleName},
        {INSIGHT_INTENT_NAME, info.intentName},
        {INSIGHT_INTENT_DOMAIN, info.domain},
        {INSIGHT_INTENT_VERSION, info.intentVersion},
        {INSIGHT_INTENT_DISPLAY_NAME, info.displayName},
        {INSIGHT_INTENT_DISPLAY_DESCRIPTION, info.displayDescription},
        {INSIGHT_INTENT_SCHEMA, info.schema},
        {INSIGHT_INTENT_ICON, info.icon},
        {INSIGHT_INTENT_LLM_DESCRIPTION, info.llmDescription},
        {INSIGHT_INTENT_KEYWORDS, info.keywords},
        {INSIGHT_INTENT_PARAMETERS, info.parameters},
        {INSIGHT_INTENT_URI, info.uri},
        {INSIGHT_INTENT_PARAM_MAPPING, info.paramMapping},
        {INSIGHT_INTENT_UI_ABILITY, info.uiAbility},
        {INSIGHT_INTENT_PAGE_ROUTE_NAME, info.pageRouteName},
        {INSIGHT_INTENT_NAVIGATION_ID, info.navigationId},
        {INSIGHT_INTENT_NAV_DESTINATION, info.navDestination},
        {INSIGHT_INTENT_ABILITY_NAME, info.abilityName},
        {INSIGHT_INTENT_EXECUTE_MODE, info.executeMode},
        {INSIGHT_INTENT_FUNCTION_NAME, info.functionName},
        {INSIGHT_INTENT_FUNCTION_PARAMS, info.functionParams}
    };
}

bool CheckProfileSubIntentInfo(const ExtraInsightIntentProfileInfo &insightIntent)
{
    switch (StringToEnum(insightIntent.decoratorType)) {
        case DecoratorType::DECORATOR_LINK:
            if (insightIntent.uri.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty uri, intentName:%{public}s", insightIntent.intentName.c_str());
                return false;
            }
            for (const auto &paramMapping: insightIntent.paramMapping) {
                if (paramMapping.paramName.empty()) {
                    TAG_LOGE(AAFwkTag::INTENT, "empty paramName, intentName:%{public}s",
                             insightIntent.intentName.c_str());
                    return false;
                }
            }
            break;
        case DecoratorType::DECORATOR_PAGE:
            if (insightIntent.pageRouteName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty pageRouteName, intentName:%{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_ENTRY:
            if (insightIntent.abilityName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty abilityName, intentName:%{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_FUNCTION:
            if (insightIntent.functionName.empty()) {
                TAG_LOGE(AAFwkTag::INTENT, "empty functionName, intentName:%{public}s",
                         insightIntent.intentName.c_str());
                return false;
            }
            break;
        case DecoratorType::DECORATOR_FORM:
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "invalid decoratorType:%{public}s", insightIntent.decoratorType.c_str());
            return false;
    }

    return true;
}

bool CheckProfileInfo(const ExtraInsightIntentProfileInfo &insightIntent)
{
    if (insightIntent.decoratorFile.empty() || insightIntent.decoratorClass.empty() ||
        insightIntent.decoratorType.empty() || insightIntent.bundleName.empty() ||
        insightIntent.moduleName.empty() || insightIntent.intentName.empty() || insightIntent.domain.empty() ||
        insightIntent.intentVersion.empty() || insightIntent.displayName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "exist empty param, decoratorFile:%s, decoratorClass:%{public}s, "
            "decoratorType:%{public}s, bundleName:%{public}s, moduleName:%{public}s, intentName:%{public}s, "
            "domain:%{public}s, intentVersion:%{public}s, displayName:%{public}s",
            insightIntent.decoratorFile.c_str(), insightIntent.decoratorClass.c_str(),
            insightIntent.decoratorType.c_str(), insightIntent.bundleName.c_str(), insightIntent.moduleName.c_str(),
            insightIntent.intentName.c_str(), insightIntent.domain.c_str(), insightIntent.intentVersion.c_str(),
            insightIntent.displayName.c_str());
        return false;
    }

    return CheckProfileSubIntentInfo(insightIntent);
}

bool TransformToLinkInfo(const ExtraInsightIntentProfileInfo &insightIntent, LinkInfo &info)
{
    info.uri = const_cast<char*>(insightIntent.uri.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "uri :%{public}s", info.uri);
    for (std::vector<LinkIntentParamProfileMapping>::const_iterator iter = insightIntent.paramMapping.begin();
        iter != insightIntent.paramMapping.end(); iter++) {
        LinkIntentParamMapping paramMapping;
        paramMapping.paramName = const_cast<char*>((*iter).paramName.c_str());
        TAG_LOGI(AAFwkTag::INTENT, "paramName :%{public}s", paramMapping.paramName);
        paramMapping.paramMappingName = const_cast<char*>((*iter).paramMappingName.c_str());
        TAG_LOGI(AAFwkTag::INTENT, "paramMappingName :%{public}s", paramMapping.paramMappingName);
        paramMapping.paramCategory = const_cast<char*>((*iter).paramCategory.c_str());
        TAG_LOGI(AAFwkTag::INTENT, "paramCategory :%{public}s", paramMapping.paramCategory);

        info.paramMapping.push_back(paramMapping);
    }
    // todo: schema模块将insightIntent.parameters解析成info.params
    info.parameters = const_cast<char*>(insightIntent.parameters.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "link parameters :%{public}s", info.parameters);
    return true;
}

bool TransformToPageInfo(const ExtraInsightIntentProfileInfo &insightIntent, PageInfo &info)
{
    info.uiAbility = const_cast<char*>(insightIntent.uiAbility.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "uiAbility :%{public}s", info.uiAbility);
    info.pageRouteName = const_cast<char*>(insightIntent.pageRouteName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "pageRouteName :%{public}s", info.pageRouteName);
    info.navigationId = const_cast<char*>(insightIntent.navigationId.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "navigationId :%{public}s", info.navigationId);
    info.navDestination = const_cast<char*>(insightIntent.navDestination.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "navDestination :%{public}s", info.navDestination);
    // todo: schema模块将insightIntent.parameters解析成info.params
    info.parameters = const_cast<char*>(insightIntent.parameters.c_str());
    return true;
}

bool TransformToEntryInfo(const ExtraInsightIntentProfileInfo &insightIntent, EntryInfo &info)
{
    info.abilityName = const_cast<char*>(insightIntent.abilityName.c_str());
    for (const auto &executeMode: insightIntent.executeMode) {
        auto mode = std::find_if(std::begin(executeModeMap), std::end(executeModeMap),
            [&executeMode](const auto &item) {
                return item.first == executeMode;
            });
        if (mode == executeModeMap.end()) {
            TAG_LOGW(AAFwkTag::INTENT, "not support execute mode :%{public}s", executeMode.c_str());
            continue;
        }
        info.executeMode.emplace_back(mode->second);
        TAG_LOGI(AAFwkTag::INTENT, "mode :%{public}s", mode->first.c_str());
    }

    // todo: schema模块将insightIntent.parameters解析成info.params
    info.parameters = const_cast<char*>(insightIntent.parameters.c_str());
    return true;
}

bool TransformToFunctionInfo(const ExtraInsightIntentProfileInfo &insightIntent, FunctionInfo &info)
{
    info.functionName = const_cast<char*>(insightIntent.functionName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "functionName :%{public}s", info.functionName);
    info.functionParams.assign(insightIntent.functionParams.begin(), insightIntent.functionParams.end());
    for (size_t i = 0; i < info.functionParams.size(); i++) {
        TAG_LOGI(AAFwkTag::INTENT, "functionParams[%{public}zu] :%{public}s", i, info.functionParams[i].c_str());
    }
    // todo: schema模块将insightIntent.parameters解析成info.params
    info.parameters = const_cast<char*>(insightIntent.parameters.c_str());
    return true;
}

bool TransformToFormInfo(const ExtraInsightIntentProfileInfo &insightIntent, FormInfo &info)
{
    // todo: schema模块将insightIntent.parameters解析成info.params
    info.parameters = const_cast<char*>(insightIntent.parameters.c_str());
    return true;
}

bool ExtraInsightIntentProfile::TransformTo(const std::string &profileStr,
    ExtraInsightIntentProfileInfoVec &intentInfos)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    auto jsonObject = nlohmann::json::parse(profileStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "discarded jsonObject");
        return false;
    }

    std::lock_guard<std::mutex> lock(g_extraMutex);
    g_extraParseResult = ERR_OK;
    intentInfos = jsonObject.get<ExtraInsightIntentProfileInfoVec>();
    if (g_extraParseResult != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "parse result :%{public}d", g_extraParseResult);
        // need recover parse result to ERR_OK
        g_extraParseResult = ERR_OK;
        return false;
    }

    for (const auto &insightIntent : intentInfos.insightIntents) {
        if (!CheckProfileInfo(insightIntent)) {
            return false;
        }
    }

    return true;
}

bool ExtraInsightIntentProfile::ToJson(const ExtraInsightIntentProfileInfo &info, nlohmann::json &jsonObject)
{
    TAG_LOGD(AAFwkTag::INTENT, "to json");
    jsonObject = info;
    if (jsonObject.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "bad insight intent info");
        return false;
    }

    TAG_LOGI(AAFwkTag::INTENT, "json string:%{public}s", jsonObject.dump().c_str());
    return true;
}

bool ExtraInsightIntentProfile::ProfileInfoFormat(const ExtraInsightIntentProfileInfo &insightIntent,
    ExtraInsightIntentInfo &info)
{
    bool ret = false;
    if (!CheckProfileInfo(insightIntent)) {
        return false;
    }

    info.decoratorFile = const_cast<char*>(insightIntent.decoratorFile.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "decoratorFile :%{public}s", info.decoratorFile);
    info.decoratorClass = const_cast<char*>(insightIntent.decoratorClass.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "decoratorClass :%{public}s", info.decoratorClass);
    info.displayDescription = const_cast<char*>(insightIntent.displayDescription.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "displayDescription :%{public}s", info.displayDescription);
    info.domain = const_cast<char*>(insightIntent.domain.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "domain :%{public}s", info.domain);
    info.intentVersion = const_cast<char*>(insightIntent.intentVersion.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "intentVersion :%{public}s", info.intentVersion);
    info.schema = const_cast<char*>(insightIntent.schema.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "schema :%{public}s", info.schema);
    info.icon = const_cast<char*>(insightIntent.icon.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "schema :%{public}s", info.schema);
    info.llmDescription = const_cast<char*>(insightIntent.llmDescription.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "llmDescription :%{public}s", info.llmDescription);
    info.keywords.assign(insightIntent.keywords.begin(), insightIntent.keywords.end());
    for (size_t i = 0; i < info.keywords.size(); i++) {
        TAG_LOGI(AAFwkTag::INTENT, "keywords[%{public}zu] :%{public}s", i, info.keywords[i].c_str());
    }

    info.genericInfo.bundleName = const_cast<char*>(insightIntent.bundleName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "bundleName :%{public}s", info.genericInfo.bundleName);
    info.genericInfo.moduleName = const_cast<char*>(insightIntent.moduleName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "moduleName :%{public}s", info.genericInfo.moduleName);
    info.genericInfo.intentName = const_cast<char*>(insightIntent.intentName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "intentName :%{public}s", info.genericInfo.intentName);
    info.genericInfo.displayName = const_cast<char*>(insightIntent.displayName.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "displayName :%{public}s", info.genericInfo.displayName);
    info.genericInfo.decoratorType = const_cast<char*>(insightIntent.decoratorType.c_str());
    TAG_LOGI(AAFwkTag::INTENT, "decoratorType :%{public}s", info.genericInfo.decoratorType);

    switch (StringToEnum(insightIntent.decoratorType)) {
        case DecoratorType::DECORATOR_LINK:
            ret = TransformToLinkInfo(insightIntent, info.genericInfo.linkInfo);
            break;
        case DecoratorType::DECORATOR_PAGE:
            ret = TransformToPageInfo(insightIntent, info.genericInfo.pageInfo);
            break;
        case DecoratorType::DECORATOR_ENTRY:
            ret = TransformToEntryInfo(insightIntent, info.genericInfo.entryInfo);
            break;
        case DecoratorType::DECORATOR_FUNCTION:
            ret = TransformToFunctionInfo(insightIntent, info.genericInfo.functionInfo);
            break;
        case DecoratorType::DECORATOR_FORM:
            ret = TransformToFormInfo(insightIntent, info.genericInfo.formInfo);
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "invalid decoratorType:%{public}s", insightIntent.decoratorType.c_str());
            return false;
    }

    return ret;
}
} // namespace AbilityRuntime
} // namespace OHOS
