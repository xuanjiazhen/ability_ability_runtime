/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "insight_intent_execute_result.h"

#include "array_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;

namespace {
constexpr const char *KEY_INNER_ERR = "innerErr";
constexpr const char *KEY_CODE = "code";
constexpr const char *KEY_FLAGS = "flags";
constexpr const char *KEY_RESULT = "result";
constexpr const char *KEY_URIS = "uris";
constexpr const char *KEY_IS_DECORATOR = "isDecorator";
constexpr const char *KEY_IS_NEED_DELAY_RESULT = "isNeedDelayResult";
constexpr const char *KEY_IS_QUERY_ENTITY = "isQueryEntity";
constexpr const char *KEY_QUERY_RESULTS = "queryResults";
constexpr const char *KEY_INTERACTION_INFO = "interactionInfo";
constexpr const char *KEY_INTERACTION_UI = "interactionUI";
constexpr const char *KEY_INTERACTION_UI_TYPE = "interactionUIType";
constexpr const char *KEY_BUNDLE_NAME = "bundleName";
constexpr const char *KEY_MODULE_NAME = "moduleName";
constexpr const char *KEY_ABILITY_NAME = "abilityName";
constexpr const char *KEY_UI_EXTENSION_TYPE = "uiExtensionType";
constexpr int32_t CYCLE_LIMIT = 1000;
} // namespace

bool InteractionUI::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(interactionUIType);
}

std::shared_ptr<InteractionUI> InteractionUI::Unmarshalling(Parcel &parcel)
{
    std::string type;
    if (!parcel.ReadString(type)) {
        return nullptr;
    }
    if (type == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        return InteractionModalUIExtension::UnmarshallingModal(parcel);
    }
    auto ui = std::make_shared<InteractionUI>();
    ui->interactionUIType = type;
    return ui;
}

bool InteractionModalUIExtension::Marshalling(Parcel &parcel) const
{
    if (!InteractionUI::Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteString(bundleName)) { return false; }
    if (!parcel.WriteString(abilityName)) { return false; }
    if (!parcel.WriteString(moduleName)) { return false; }
    if (!parcel.WriteString(uiExtensionType)) { return false; }
    return true;
}

std::shared_ptr<InteractionModalUIExtension> InteractionModalUIExtension::UnmarshallingModal(Parcel &parcel)
{
    auto modal = std::make_shared<InteractionModalUIExtension>();
    modal->interactionUIType = INTERACTION_UI_TYPE_MODAL_UIEXTENSION;
    if (!parcel.ReadString(modal->bundleName)) { return nullptr; }
    if (!parcel.ReadString(modal->abilityName)) { return nullptr; }
    if (!parcel.ReadString(modal->moduleName)) { return nullptr; }
    if (!parcel.ReadString(modal->uiExtensionType)) { return nullptr; }
    return modal;
}

bool InteractionInfo::Marshalling(Parcel &parcel) const
{
    bool hasUI = (interactionUI != nullptr);
    if (!parcel.WriteBool(hasUI)) { return false; }
    if (hasUI) {
        if (!interactionUI->Marshalling(parcel)) { return false; }
    }
    return true;
}

bool InteractionInfo::ReadFromParcel(Parcel &parcel)
{
    bool hasUI = false;
    if (!parcel.ReadBool(hasUI)) { return false; }
    if (hasUI) {
        interactionUI = InteractionUI::Unmarshalling(parcel);
        if (interactionUI == nullptr) { return false; }
    }
    return true;
}

bool InsightIntentExecuteResult::ReadFromParcel(Parcel &parcel)
{
    innerErr = parcel.ReadInt32();
    code = parcel.ReadInt32();
    result = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    if (!parcel.ReadStringVector(&uris)) {
        return false;
    }
    flags = parcel.ReadInt32();
    isDecorator = parcel.ReadBool();
    isQueryEntity = parcel.ReadBool();
    int32_t resultSize = parcel.ReadInt32();
    if (resultSize < 0 || resultSize > CYCLE_LIMIT) {
        return false;
    }
    queryResults.clear();
    for (int32_t i = 0; i < resultSize; i++) {
        auto temp = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
        if (temp == nullptr) {
            return false;
        }
        queryResults.push_back(temp);
    }
    if (!interactionInfo.ReadFromParcel(parcel)) {
        return false;
    }
    return true;
}

bool InsightIntentExecuteResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(innerErr)) {
        return false;
    }
    if (!parcel.WriteInt32(code)) {
        return false;
    }
    if (!parcel.WriteParcelable(result.get())) {
        return false;
    }
    if (!parcel.WriteStringVector(uris)) {
        return false;
    }
    if (!parcel.WriteInt32(flags)) {
        return false;
    }
    if (!parcel.WriteBool(isDecorator)) {
        return false;
    }
    if (!parcel.WriteBool(isQueryEntity)) {
        return false;
    }
    if (!parcel.WriteInt32(queryResults.size())) {
        return false;
    }
    for (const auto &item : queryResults) {
        if (!parcel.WriteParcelable(item.get())) {
            return false;
        }
    }
    if (!interactionInfo.Marshalling(parcel)) {
        return false;
    }
    return true;
}

InsightIntentExecuteResult *InsightIntentExecuteResult::Unmarshalling(Parcel &parcel)
{
    auto res = new (std::nothrow) InsightIntentExecuteResult();
    if (res == nullptr) {
        return nullptr;
    }

    if (!res->ReadFromParcel(parcel)) {
        delete res;
        res = nullptr;
    }
    return res;
}

static void ParseWantParamsField(const nlohmann::json &jsonObject,
    const char *key, std::shared_ptr<WantParams> &field)
{
    if (!jsonObject.contains(key)) {
        return;
    }
    const auto &json = jsonObject.at(key);
    if (json.is_object()) {
        field = std::make_shared<WantParams>();
        OHOS::AAFwk::from_json(json, *field);
    } else if (json.is_null()) {
        field = nullptr;
    }
}

void InsightIntentExecuteResult::FromJsonString(const std::string &jsonStr)
{
    nlohmann::json jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded() || !jsonObject.is_object()) {
        return;
    }

    if (jsonObject.contains(KEY_INNER_ERR) && jsonObject.at(KEY_INNER_ERR).is_number_integer()) {
        innerErr = jsonObject.at(KEY_INNER_ERR).get<int32_t>();
    }
    if (jsonObject.contains(KEY_CODE) && jsonObject.at(KEY_CODE).is_number_integer()) {
        code = jsonObject.at(KEY_CODE).get<int32_t>();
    }
    if (jsonObject.contains(KEY_FLAGS) && jsonObject.at(KEY_FLAGS).is_number_integer()) {
        flags = jsonObject.at(KEY_FLAGS).get<int32_t>();
    }
    if (jsonObject.contains(KEY_IS_DECORATOR) && jsonObject.at(KEY_IS_DECORATOR).is_boolean()) {
        isDecorator = jsonObject.at(KEY_IS_DECORATOR).get<bool>();
    }
    if (jsonObject.contains(KEY_IS_NEED_DELAY_RESULT) && jsonObject.at(KEY_IS_NEED_DELAY_RESULT).is_boolean()) {
        isNeedDelayResult = jsonObject.at(KEY_IS_NEED_DELAY_RESULT).get<bool>();
    }
    if (jsonObject.contains(KEY_IS_QUERY_ENTITY) && jsonObject.at(KEY_IS_QUERY_ENTITY).is_boolean()) {
        isQueryEntity = jsonObject.at(KEY_IS_QUERY_ENTITY).get<bool>();
    }

    if (jsonObject.contains(KEY_URIS) && jsonObject.at(KEY_URIS).is_array()) {
        uris.clear();
        for (const auto &item : jsonObject.at(KEY_URIS)) {
            if (item.is_string()) {
                uris.emplace_back(item.get<std::string>());
            }
        }
    }

    ParseWantParamsField(jsonObject, KEY_RESULT, result);
    if (jsonObject.contains(KEY_QUERY_RESULTS) && jsonObject.at(KEY_QUERY_RESULTS).is_array()) {
        queryResults.clear();
        for (const auto &item : jsonObject.at(KEY_QUERY_RESULTS)) {
            if (!item.is_object()) {
                continue;
            }
            auto queryResult = std::make_shared<WantParams>();
            OHOS::AAFwk::from_json(item, *queryResult);
            queryResults.emplace_back(queryResult);
        }
    }

    if (jsonObject.contains(KEY_INTERACTION_INFO) && jsonObject.at(KEY_INTERACTION_INFO).is_object()) {
        const auto &infoJson = jsonObject.at(KEY_INTERACTION_INFO);
        if (infoJson.contains(KEY_INTERACTION_UI) && infoJson.at(KEY_INTERACTION_UI).is_object()) {
            const auto &uiJson = infoJson.at(KEY_INTERACTION_UI);
            std::string uiType;
            if (uiJson.contains(KEY_INTERACTION_UI_TYPE) && uiJson.at(KEY_INTERACTION_UI_TYPE).is_string()) {
                uiType = uiJson.at(KEY_INTERACTION_UI_TYPE).get<std::string>();
            }
            if (uiType == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
                auto modal = std::make_shared<InteractionModalUIExtension>();
                modal->interactionUIType = uiType;
                if (uiJson.contains(KEY_BUNDLE_NAME) && uiJson.at(KEY_BUNDLE_NAME).is_string()) {
                    modal->bundleName = uiJson.at(KEY_BUNDLE_NAME).get<std::string>();
                }
                if (uiJson.contains(KEY_ABILITY_NAME) && uiJson.at(KEY_ABILITY_NAME).is_string()) {
                    modal->abilityName = uiJson.at(KEY_ABILITY_NAME).get<std::string>();
                }
                if (uiJson.contains(KEY_MODULE_NAME) && uiJson.at(KEY_MODULE_NAME).is_string()) {
                    modal->moduleName = uiJson.at(KEY_MODULE_NAME).get<std::string>();
                }
                if (uiJson.contains(KEY_UI_EXTENSION_TYPE) && uiJson.at(KEY_UI_EXTENSION_TYPE).is_string()) {
                    modal->uiExtensionType = uiJson.at(KEY_UI_EXTENSION_TYPE).get<std::string>();
                }
                interactionInfo.interactionUI = modal;
            } else {
                auto ui = std::make_shared<InteractionUI>();
                ui->interactionUIType = uiType;
                interactionInfo.interactionUI = ui;
            }
        }
    }
}

std::string InsightIntentExecuteResult::ToJsonString() const
{
    nlohmann::json jsonObject;
    jsonObject[KEY_INNER_ERR] = innerErr;
    jsonObject[KEY_CODE] = code;
    jsonObject[KEY_FLAGS] = flags;
    jsonObject[KEY_URIS] = uris;
    jsonObject[KEY_IS_DECORATOR] = isDecorator;
    jsonObject[KEY_IS_NEED_DELAY_RESULT] = isNeedDelayResult;
    jsonObject[KEY_IS_QUERY_ENTITY] = isQueryEntity;

    if (result != nullptr) {
        nlohmann::json resultJson;
        OHOS::AAFwk::to_json(resultJson, *result);
        jsonObject[KEY_RESULT] = resultJson;
    } else {
        jsonObject[KEY_RESULT] = nullptr;
    }

    nlohmann::json queryResultsJson = nlohmann::json::array();
    for (const auto &item : queryResults) {
        if (item == nullptr) {
            continue;
        }
        nlohmann::json itemJson;
        OHOS::AAFwk::to_json(itemJson, *item);
        queryResultsJson.emplace_back(itemJson);
    }
    jsonObject[KEY_QUERY_RESULTS] = queryResultsJson;
    if (interactionInfo.interactionUI != nullptr) {
        nlohmann::json infoJson;
        nlohmann::json uiJson;
        uiJson[KEY_INTERACTION_UI_TYPE] = interactionInfo.interactionUI->interactionUIType;
        auto modalUI = std::dynamic_pointer_cast<InteractionModalUIExtension>(
            interactionInfo.interactionUI);
        if (modalUI != nullptr) {
            uiJson[KEY_BUNDLE_NAME] = modalUI->bundleName;
            uiJson[KEY_ABILITY_NAME] = modalUI->abilityName;
            uiJson[KEY_MODULE_NAME] = modalUI->moduleName;
            uiJson[KEY_UI_EXTENSION_TYPE] = modalUI->uiExtensionType;
        }
        infoJson[KEY_INTERACTION_UI] = uiJson;
        jsonObject[KEY_INTERACTION_INFO] = infoJson;
    }
    return jsonObject.dump();
}

bool InsightIntentExecuteResult::CheckResult(std::shared_ptr<const WantParams> result)
{
    return true;
}

bool InsightIntentExecuteResult::CheckInteractionInfo(const InteractionInfo &intent)
{
    if (intent.interactionUI == nullptr) {
        return true;
    }
    const auto &type = intent.interactionUI->interactionUIType;
    if (type.empty()) {
        return false;
    }
    if (type == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        auto modal = std::dynamic_pointer_cast<InteractionModalUIExtension>(intent.interactionUI);
        if (modal == nullptr) {
            return false;
        }
        return !modal->bundleName.empty() && !modal->moduleName.empty() &&
            !modal->abilityName.empty() && !modal->uiExtensionType.empty();
    }
    return true;
}

std::shared_ptr<WantParams> InsightIntentExecuteResult::BuildFunctionResult() const
{
    auto resultParams = std::make_shared<WantParams>();
    resultParams->SetParam(KEY_FLAGS, OHOS::AAFwk::Integer::Box(flags));
    if (!uris.empty()) {
        auto uriSize = uris.size();
        sptr<OHOS::AAFwk::IArray> uriArray = sptr<AAFwk::Array>::MakeSptr(uriSize, AAFwk::g_IID_IString);
        if (uriArray != nullptr) {
            for (std::size_t i = 0; i < uriSize; i++) {
                uriArray->Set(i, OHOS::AAFwk::String::Box(uris[i]));
            }
            resultParams->SetParam(KEY_URIS, uriArray);
        }
    }
    if (result != nullptr) {
        resultParams->SetParam(KEY_RESULT, OHOS::AAFwk::WantParamWrapper::Box(*result));
    }
    if (interactionInfo.interactionUI != nullptr) {
        auto infoParams = std::make_shared<WantParams>();
        auto uiParams = std::make_shared<WantParams>();
        uiParams->SetParam(KEY_INTERACTION_UI_TYPE,
            OHOS::AAFwk::String::Box(interactionInfo.interactionUI->interactionUIType));
        auto modalUI = std::dynamic_pointer_cast<InteractionModalUIExtension>(
            interactionInfo.interactionUI);
        if (modalUI != nullptr) {
            uiParams->SetParam(KEY_BUNDLE_NAME, OHOS::AAFwk::String::Box(modalUI->bundleName));
            uiParams->SetParam(KEY_ABILITY_NAME, OHOS::AAFwk::String::Box(modalUI->abilityName));
            uiParams->SetParam(KEY_MODULE_NAME, OHOS::AAFwk::String::Box(modalUI->moduleName));
            uiParams->SetParam(KEY_UI_EXTENSION_TYPE, OHOS::AAFwk::String::Box(modalUI->uiExtensionType));
        }
        infoParams->SetParam(KEY_INTERACTION_UI, OHOS::AAFwk::WantParamWrapper::Box(*uiParams));
        resultParams->SetParam(KEY_INTERACTION_INFO, OHOS::AAFwk::WantParamWrapper::Box(*infoParams));
    }
    return resultParams;
}
} // namespace AppExecFwk
} // namespace OHOS
