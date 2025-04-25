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

#include "js_insight_intent_info.h"

#include "string_wrapper.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
int32_t g_extraParseResult = ERR_OK;

const std::string INSIGHT_INTENTS_URI = "uri";
const std::string INSIGHT_INTENT_UI_ABILITY = "uiAbility";
const std::string INSIGHT_INTENT_PAGE_ROUTER_NAME = "pageRouterName";
const std::string INSIGHT_INTENT_NAVIGATION_ID = "navigationId";
const std::string INSIGHT_INTENT_NAV_DESTINATION = "navDestination";
const std::string INSIGHT_INTENT_ABILITY_NAME = "abilityName";
const std::string INSIGHT_INTENT_EXECUTE_MODE = "executeMode";
const std::string INSIGHT_INTENT_BUNDLE_NAME = "bundleName";
const std::string INSIGHT_INTENT_MODULE_NAME = "moduleName";
const std::string INSIGHT_INTENT_INTENT_NAME = "intentName";
const std::string INSIGHT_INTENT_DOMAIN = "domain";
const std::string INSIGHT_INTENT_INTENT_VERSION = "intentVersion";
const std::string INSIGHT_INTENT_DISPLAY_NAME = "displayName";
const std::string INSIGHT_INTENT_DISPLAY_DESCRIPTION = "displayDescription";
const std::string INSIGHT_INTENT_SCHEMA = "schema";
const std::string INSIGHT_INTENT_ICON = "icon";
const std::string INSIGHT_INTENT_LLM_DESCRIPTION = "llmDescription";
const std::string INSIGHT_INTENT_INTENT_TYPE = "intentType";
const std::string INSIGHT_INTENT_PARAMS = "params";
const std::string INSIGHT_INTENT_KEYWORDS = "keywords";
const std::string INSIGHT_INTENT_LINK_INFO = "linkInfo";
const std::string INSIGHT_INTENT_PAGE_INFO = "pageInfo";
const std::string INSIGHT_INTENT_ENTRY_INFO = "entryInfo";
const std::string INSIGHT_INTENT_FUNCTION_INFO = "functionInfo";
const std::string INSIGHT_INTENT_FORM_INFO = "formInfo";

#define WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(type, parcel, data)           \
    do {                                                                    \
        if (!(parcel).Write##type(data)) {                                  \
            TAG_LOGE(AAFwkTag::INTENT, "write %{public}s type fail", #type);\
            return false;                                                   \
        }                                                                   \
    } while (0)
}
template<typename T>
void ReadParcelableInfo(T &parceable, Parcel &parcel)
{
    std::unique_ptr<T> info(parcel.ReadParcelable<T>());
    if (!info) {
        TAG_LOGE(AAFwkTag::INTENT, "failed, info is nullptr");
    }
    parceable = *info;
}

void from_json(const nlohmann::json &jsonObject, JsLinkInfo &linkInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsLinkInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENTS_URI,
        linkInfo.uri,
        true,
        g_extraParseResult);
}

void to_json(nlohmann::json& jsonObject, const JsLinkInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsLinkInfo to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENTS_URI, info.uri}
    };
}

void from_json(const nlohmann::json &jsonObject, JsPageInfo &pageInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsPageInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_UI_ABILITY,
        pageInfo.uiAbility,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PAGE_ROUTER_NAME,
        pageInfo.pageRouterName,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAVIGATION_ID,
        pageInfo.navigationId,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_NAV_DESTINATION,
        pageInfo.navDestination,
        true,
        g_extraParseResult);
}

void to_json(nlohmann::json& jsonObject, const JsPageInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsPageInfo to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_UI_ABILITY, info.uiAbility},
        {INSIGHT_INTENT_PAGE_ROUTER_NAME, info.pageRouterName},
        {INSIGHT_INTENT_NAVIGATION_ID, info.navigationId},
        {INSIGHT_INTENT_NAV_DESTINATION, info.navDestination}
    };
}

void from_json(const nlohmann::json &jsonObject, JsEntryInfo &entryInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsEntryInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ABILITY_NAME,
        entryInfo.abilityName,
        true,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_EXECUTE_MODE,
        entryInfo.executeMode,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
}

void to_json(nlohmann::json& jsonObject, const JsEntryInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsEntryInfo to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_ABILITY_NAME, info.abilityName},
        {INSIGHT_INTENT_EXECUTE_MODE, info.executeMode}
    };
}

void from_json(const nlohmann::json &jsonObject, JsFunctionInfo &functionInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsFunctionInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
}

void to_json(nlohmann::json& jsonObject, const JsFunctionInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsFunctionInfo to json");
    jsonObject = nlohmann::json {};
}

void from_json(const nlohmann::json &jsonObject, JsFormInfo &formInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsFormInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
}

void to_json(nlohmann::json& jsonObject, const JsFormInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsFormInfo to json");
    jsonObject = nlohmann::json {};
}

void from_json(const nlohmann::json &jsonObject, JsInsightIntentInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsInsightIntentInfo from json");
    const auto &jsonObjectEnd = jsonObject.end();
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
        INSIGHT_INTENT_INTENT_NAME,
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
        INSIGHT_INTENT_INTENT_VERSION,
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
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_SCHEMA,
        insightIntentInfo.schema,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ICON,
        insightIntentInfo.icon,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_LLM_DESCRIPTION,
        insightIntentInfo.llmDescription,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_INTENT_TYPE,
        insightIntentInfo.intentType,
        true,
        g_extraParseResult);
    AppExecFwk::BMSJsonUtil::GetStrValueIfFindKey(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PARAMS,
        insightIntentInfo.params,
        true,
        g_extraParseResult);
    AppExecFwk::GetValueIfFindKey<std::vector<std::string>>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_KEYWORDS,
        insightIntentInfo.keywords,
        JsonType::ARRAY,
        false,
        g_extraParseResult,
        ArrayType::STRING);
    AppExecFwk::GetValueIfFindKey<JsLinkInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_LINK_INFO,
        insightIntentInfo.linkInfo,
        JsonType::OBJECT,
        false,
        g_extraParseResult,
        ArrayType::NO_ARRAY);
    AppExecFwk::GetValueIfFindKey<JsPageInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_PAGE_INFO,
        insightIntentInfo.pageInfo,
        JsonType::OBJECT,
        false,
        g_extraParseResult,
        ArrayType::NO_ARRAY);
    AppExecFwk::GetValueIfFindKey<JsEntryInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_ENTRY_INFO,
        insightIntentInfo.entryInfo,
        JsonType::OBJECT,
        false,
        g_extraParseResult,
        ArrayType::NO_ARRAY);
    AppExecFwk::GetValueIfFindKey<JsFunctionInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FUNCTION_INFO,
        insightIntentInfo.functionInfo,
        JsonType::OBJECT,
        false,
        g_extraParseResult,
        ArrayType::NO_ARRAY);
    AppExecFwk::GetValueIfFindKey<JsFormInfo>(jsonObject,
        jsonObjectEnd,
        INSIGHT_INTENT_FORM_INFO,
        insightIntentInfo.formInfo,
        JsonType::OBJECT,
        false,
        g_extraParseResult,
        ArrayType::NO_ARRAY);
}

void to_json(nlohmann::json& jsonObject, const JsInsightIntentInfo &info)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsEntryInfo to json");
    jsonObject = nlohmann::json {
        {INSIGHT_INTENT_BUNDLE_NAME, info.bundleName},
        {INSIGHT_INTENT_MODULE_NAME, info.moduleName},
        {INSIGHT_INTENT_INTENT_NAME, info.intentName},
        {INSIGHT_INTENT_DOMAIN, info.domain},
        {INSIGHT_INTENT_INTENT_VERSION, info.intentVersion},
        {INSIGHT_INTENT_DISPLAY_NAME, info.displayName},
        {INSIGHT_INTENT_DISPLAY_DESCRIPTION, info.displayDescription},
        {INSIGHT_INTENT_SCHEMA, info.schema},
        {INSIGHT_INTENT_ICON, info.icon},
        {INSIGHT_INTENT_LLM_DESCRIPTION, info.llmDescription},
        {INSIGHT_INTENT_INTENT_TYPE, info.intentType},
        {INSIGHT_INTENT_PARAMS, info.params},
        {INSIGHT_INTENT_KEYWORDS, info.keywords},
        {INSIGHT_INTENT_LINK_INFO, info.linkInfo},
        {INSIGHT_INTENT_PAGE_INFO, info.pageInfo},
        {INSIGHT_INTENT_ENTRY_INFO, info.entryInfo},
        {INSIGHT_INTENT_FUNCTION_INFO, info.functionInfo},
        {INSIGHT_INTENT_FORM_INFO, info.formInfo}
    };
}

bool JsInsightIntentInfo::ReadFromParcel(Parcel &parcel)
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        APP_LOGE("Type conversion failed");
        return false;
    }
    uint32_t length = messageParcel->ReadUint32();
    if (length == 0) {
        APP_LOGE("Invalid data length");
        return false;
    }
    const char *data = reinterpret_cast<const char *>(messageParcel->ReadRawData(length));
    if (!data) {
        APP_LOGE("Fail read raw length = %{public}d", length);
        return false;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(data, nullptr, false);
    if (jsonObject.is_discarded()) {
        APP_LOGE("failed to parse BundleInfo");
        return false;
    }
    *this = jsonObject.get<JsInsightIntentInfo>();
    return true;
}

bool JsInsightIntentInfo::Marshalling(Parcel &parcel) const
{
    MessageParcel *messageParcel = reinterpret_cast<MessageParcel *>(&parcel);
    if (!messageParcel) {
        APP_LOGE("Type conversion failed");
        return false;
    }
    nlohmann::json jsonObject = *this;
    std::string str = jsonObject.dump();
    if (!messageParcel->WriteUint32(str.size() + 1)) {
        APP_LOGE("Failed to write;data size");
        return false;
    }
    if (!messageParcel->WriteRawData(str.c_str(), str.size() + 1)) {
        APP_LOGE("Failed to write data");
        return false;
    }
    return true;
}

JsInsightIntentInfo *JsInsightIntentInfo::Unmarshalling(Parcel &parcel)
{
    JsInsightIntentInfo *info = new (std::nothrow) JsInsightIntentInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace AbilityRuntime
} // namespace OHOS
 