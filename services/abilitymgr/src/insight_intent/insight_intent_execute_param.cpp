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

#include "insight_intent_execute_param.h"

#include <charconv>

#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_constant.h"
#include "int_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

namespace {
constexpr const char *PARAM_INFO_NAME = "paramName";
constexpr const char *PARAM_INFO_TYPE = "type";
constexpr const char *PARAM_INFO_REQUIRED = "isRequired";

bool IsValidParamType(ParamType type)
{
    switch (type) {
        case ParamType::STRING:
        case ParamType::NUMBER:
        case ParamType::BOOLEAN:
        case ParamType::OBJECT:
        case ParamType::ARRAY:
        case ParamType::INTEGER:
        case ParamType::UNKNOWN:
            return true;
        default:
            return false;
    }
}

uint8_t GetParamTypeValue(ParamType type)
{
    if (IsValidParamType(type)) {
        return static_cast<uint8_t>(type);
    }
    TAG_LOGW(AAFwkTag::INTENT, "invalid param type enum, fallback to unknown");
    return static_cast<uint8_t>(ParamType::UNKNOWN);
}

bool ParseParamType(const nlohmann::json &item, ParamType &type)
{
    if (!item.contains(PARAM_INFO_TYPE)) {
        return true;
    }
    if (!item[PARAM_INFO_TYPE].is_number_unsigned()) {
        TAG_LOGE(AAFwkTag::INTENT, "param type is not unsigned number");
        return false;
    }
    uint8_t typeValue = item[PARAM_INFO_TYPE].get<uint8_t>();
    ParamType parsedType = static_cast<ParamType>(typeValue);
    if (!IsValidParamType(parsedType)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid param type value: %{public}u", typeValue);
        return false;
    }
    type = parsedType;
    return true;
}

bool ParseFunctionParamInfo(const nlohmann::json &item, InsightIntentParam &param)
{
    if (item.contains(PARAM_INFO_NAME) && item[PARAM_INFO_NAME].is_string()) {
        param.paramName = item[PARAM_INFO_NAME].get<std::string>();
    }
    if (!ParseParamType(item, param.type)) {
        return false;
    }
    if (item.contains(PARAM_INFO_REQUIRED) && item[PARAM_INFO_REQUIRED].is_boolean()) {
        param.isRequired = item[PARAM_INFO_REQUIRED].get<bool>();
    }
    return true;
}

bool DeserializeFunctionParamInfos(const std::string &paramInfosStr, std::vector<InsightIntentParam> &params)
{
    params.clear();
    if (paramInfosStr.empty()) {
        return true;
    }
    auto jsonObj = nlohmann::json::parse(paramInfosStr, nullptr, false);
    if (jsonObj.is_discarded() || !jsonObj.is_array()) {
        TAG_LOGE(AAFwkTag::INTENT, "param infos parse failed or not array: %{public}s", paramInfosStr.c_str());
        return false;
    }
    for (const auto &item : jsonObj) {
        if (!item.is_object()) {
            TAG_LOGW(AAFwkTag::INTENT, "skip non-object param info");
            continue;
        }
        InsightIntentParam param;
        if (!ParseFunctionParamInfo(item, param)) {
            return false;
        }
        params.push_back(param);
    }
    return true;
}

std::string SerializeFunctionParamInfos(const std::vector<InsightIntentParam> &params)
{
    nlohmann::json paramsJson = nlohmann::json::array();
    for (const auto &param : params) {
        nlohmann::json paramJson;
        paramJson[PARAM_INFO_NAME] = param.paramName;
        paramJson[PARAM_INFO_TYPE] = GetParamTypeValue(param.type);
        paramJson[PARAM_INFO_REQUIRED] = param.isRequired;
        paramsJson.push_back(paramJson);
    }
    return paramsJson.dump();
}
} // namespace

bool InsightIntentExecuteParam::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    moduleName_ = Str16ToStr8(parcel.ReadString16());
    abilityName_ = Str16ToStr8(parcel.ReadString16());
    insightIntentName_ = Str16ToStr8(parcel.ReadString16());
    std::shared_ptr<WantParams> wantParams(parcel.ReadParcelable<WantParams>());
    if (wantParams == nullptr) {
        return false;
    }
    insightIntentParam_ = wantParams;
    executeMode_ = parcel.ReadInt32();
    insightIntentId_ = parcel.ReadUint64();
    displayId_ = parcel.ReadInt32();
    parcel.ReadStringVector(&uris_);
    flags_ = parcel.ReadInt32();
    userId_ = parcel.ReadInt32();
    decoratorType_ = parcel.ReadInt8();
    srcEntrance_ = Str16ToStr8(parcel.ReadString16());
    className_ = Str16ToStr8(parcel.ReadString16());
    methodName_ = Str16ToStr8(parcel.ReadString16());
    methodReturnType_ = Str16ToStr8(parcel.ReadString16());
    parcel.ReadStringVector(&methodParams_);
    pagePath_ = Str16ToStr8(parcel.ReadString16());
    navigationId_ = Str16ToStr8(parcel.ReadString16());
    navDestinationName_ = Str16ToStr8(parcel.ReadString16());
    isServiceMatch_ = parcel.ReadBool();

    std::string paramInfosStr = Str16ToStr8(parcel.ReadString16());
    if (!DeserializeFunctionParamInfos(paramInfosStr, functionParamInfos_)) {
        TAG_LOGE(AAFwkTag::INTENT, "read function param infos failed");
        return false;
    }
    return true;
}

InsightIntentExecuteParam *InsightIntentExecuteParam::Unmarshalling(Parcel &parcel)
{
    InsightIntentExecuteParam *param = new (std::nothrow) InsightIntentExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }

    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool InsightIntentExecuteParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString16(Str8ToStr16(bundleName_));
    parcel.WriteString16(Str8ToStr16(moduleName_));
    parcel.WriteString16(Str8ToStr16(abilityName_));
    parcel.WriteString16(Str8ToStr16(insightIntentName_));
    parcel.WriteParcelable(insightIntentParam_.get());
    parcel.WriteInt32(executeMode_);
    parcel.WriteUint64(insightIntentId_);
    parcel.WriteInt32(displayId_);
    parcel.WriteStringVector(uris_);
    parcel.WriteInt32(flags_);
    parcel.WriteInt32(userId_);
    parcel.WriteInt8(decoratorType_);
    parcel.WriteString16(Str8ToStr16(srcEntrance_));
    parcel.WriteString16(Str8ToStr16(className_));
    parcel.WriteString16(Str8ToStr16(methodName_));
    parcel.WriteString16(Str8ToStr16(methodReturnType_));
    parcel.WriteStringVector(methodParams_);
    parcel.WriteString16(Str8ToStr16(pagePath_));
    parcel.WriteString16(Str8ToStr16(navigationId_));
    parcel.WriteString16(Str8ToStr16(navDestinationName_));
    parcel.WriteBool(isServiceMatch_);
    parcel.WriteString16(Str8ToStr16(SerializeFunctionParamInfos(functionParamInfos_)));
    return true;
}

bool InsightIntentExecuteParam::IsInsightIntentExecute(const AAFwk::Want &want)
{
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        return true;
    }
    return false;
}

bool InsightIntentExecuteParam::IsInsightIntentPage(const AAFwk::Want &want)
{
    if (!want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    auto type = wantParams.GetIntParam(INSIGHT_INTENT_DECORATOR_TYPE, -1);
    if (type == static_cast<int>(AbilityRuntime::InsightIntentType::DECOR_PAGE)) {
        return true;
    }

    return false;
}

bool InsightIntentExecuteParam::GenerateFromWant(const AAFwk::Want &want,
    InsightIntentExecuteParam &executeParam)
{
    const WantParams &wantParams = want.GetParams();
    if (!wantParams.HasParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        TAG_LOGE(AAFwkTag::INTENT, "empty want");
        return false;
    }
    uint64_t insightIntentId = 0;
    std::string idStr = wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_ID);
    auto res = std::from_chars(idStr.c_str(), idStr.c_str() + idStr.size(), insightIntentId);
    if (res.ec != std::errc()) {
        TAG_LOGE(AAFwkTag::ABILITY, "invalid insight intent ID");
        return false;
    }

    AppExecFwk::ElementName elementName = want.GetElement();
    executeParam.bundleName_ = elementName.GetBundleName();
    executeParam.moduleName_ = elementName.GetModuleName();
    executeParam.abilityName_ = elementName.GetAbilityName();
    executeParam.insightIntentName_ = wantParams.GetStringParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    executeParam.insightIntentId_ = insightIntentId;
    executeParam.executeMode_ = wantParams.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE, 0);

    auto insightIntentParam = wantParams.GetWantParams(INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    UpdateInsightIntentCallerInfo(wantParams, insightIntentParam);
    executeParam.insightIntentParam_ = std::make_shared<WantParams>(insightIntentParam);
    executeParam.uris_ = want.GetStringArrayParam(INSIGHT_INTENT_EXECUTE_PARAM_URI);
    executeParam.flags_ = wantParams.GetIntParam(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, 0);
    executeParam.decoratorType_ = wantParams.GetIntParam(INSIGHT_INTENT_DECORATOR_TYPE, 0);
    executeParam.srcEntrance_ = wantParams.GetStringParam(INSIGHT_INTENT_SRC_ENTRANCE);
    executeParam.className_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME);
    executeParam.methodName_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_METHODNAME);
    executeParam.methodReturnType_ = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE);
    executeParam.methodParams_ = want.GetStringArrayParam(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS);
    executeParam.pagePath_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH);
    executeParam.navigationId_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID);
    executeParam.navDestinationName_ = wantParams.GetStringParam(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME);
    executeParam.queryEntityClassName_ = wantParams.GetStringParam(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME);
    executeParam.queryType_ = wantParams.GetStringParam(INSIGHT_INTENT_QUERY_TYPE);
    auto queryParams = wantParams.GetWantParams(INSIGHT_INTENT_QUERY_ENTITY_PARAM_PARAM);
    executeParam.queryParams_ = std::make_shared<WantParams>(queryParams);

    std::string paramInfosStr = wantParams.GetStringParam(INSIGHT_INTENT_FUNC_PARAM_SCHEMAS);
    if (!DeserializeFunctionParamInfos(paramInfosStr, executeParam.functionParamInfos_)) {
        TAG_LOGE(AAFwkTag::INTENT, "generate function param infos failed");
        return false;
    }
    return true;
}

bool InsightIntentExecuteParam::RemoveInsightIntent(AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_NAME)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_ID)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_ID);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_MODE)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_MODE);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_PARAM)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_PARAM);
    }
    if (want.HasParameter(INSIGHT_INTENT_SRC_ENTRY)) {
        want.RemoveParam(INSIGHT_INTENT_SRC_ENTRY);
    }
    if (want.HasParameter(INSIGHT_INTENT_ARKTS_MODE)) {
        want.RemoveParam(INSIGHT_INTENT_ARKTS_MODE);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_URI)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_URI);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_PARAM_FLAGS);
    }
    if (want.HasParameter(INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG)) {
        want.RemoveParam(INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG);
    }
    if (want.HasParameter(INSIGHT_INTENT_DECORATOR_TYPE)) {
	    want.RemoveParam(INSIGHT_INTENT_DECORATOR_TYPE);
    }
    if (want.HasParameter(INSIGHT_INTENT_SRC_ENTRANCE)) {
        want.RemoveParam(INSIGHT_INTENT_SRC_ENTRANCE);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_CLASSNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_METHODNAME)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_METHODNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_RETURNTYPE);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_PAGEPATH);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID);
    }
    if (want.HasParameter(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME)) {
        want.RemoveParam(INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME)) {
        want.RemoveParam(INSIGHT_INTENT_QUERY_ENTITY_CLASS_NAME);
    }
    if (want.HasParameter(INSIGHT_INTENT_FUNC_PARAM_SCHEMAS)) {
        want.RemoveParam(INSIGHT_INTENT_FUNC_PARAM_SCHEMAS);
    }
    return true;
}

void InsightIntentExecuteParam::UpdateInsightIntentCallerInfo(const WantParams &wantParams,
    WantParams &insightIntentParam)
{
    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_TOKEN);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_TOKEN,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_TOKEN, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_UID);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_UID,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_UID, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_PID);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_PID,
        AAFwk::Integer::Box(wantParams.GetIntParam(AAFwk::Want::PARAM_RESV_CALLER_PID, 0)));

    insightIntentParam.Remove(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    insightIntentParam.SetParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME,
        AAFwk::String::Box(wantParams.GetStringParam(AAFwk::Want::PARAM_RESV_CALLER_BUNDLE_NAME)));
}
} // namespace AppExecFwk
} // namespace OHOS
