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

#include <cJSON.h>

#include "array_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "nlohmann/json.hpp"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using WantParams = OHOS::AAFwk::WantParams;
constexpr int32_t CYCLE_LIMIT = 1000;
constexpr size_t MAX_BUNDLE_NAME_LEN = 127;
constexpr size_t MAX_MODULE_NAME_LEN = 31;
constexpr size_t MAX_ABILITY_NAME_LEN = 255;
constexpr size_t MAX_UI_EXTENSION_TYPE_LEN = 255;
constexpr size_t MAX_RESULT_JSON_LEN = 1024 * 1024;
constexpr size_t MAX_RESULT_JSON_DEPTH = 100;
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
constexpr const char *KEY_URI = "uri";
constexpr const char *KEY_PARAMETERS = "parameters";

struct CJsonDeleter {
    void operator()(cJSON *p) const
    {
        if (p != nullptr) {
            cJSON_Delete(p);
        }
    }
};
using cJSONUniquePtr = std::unique_ptr<cJSON, CJsonDeleter>;
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
    if (!parcel.WriteString(uri)) { return false; }
    return parcel.WriteParcelable(parameters.get());
}

std::shared_ptr<InteractionModalUIExtension> InteractionModalUIExtension::UnmarshallingModal(Parcel &parcel)
{
    auto modal = std::make_shared<InteractionModalUIExtension>();
    modal->interactionUIType = INTERACTION_UI_TYPE_MODAL_UIEXTENSION;
    if (!parcel.ReadString(modal->bundleName)) { return nullptr; }
    if (!parcel.ReadString(modal->abilityName)) { return nullptr; }
    if (!parcel.ReadString(modal->moduleName)) { return nullptr; }
    if (!parcel.ReadString(modal->uiExtensionType)) { return nullptr; }
    if (!parcel.ReadString(modal->uri)) { return nullptr; }
    modal->parameters = std::shared_ptr<WantParams>(parcel.ReadParcelable<WantParams>());
    return modal;
}

bool InteractionInfo::Marshalling(Parcel &parcel) const
{
    bool hasUI = (interactionUI != nullptr);
    if (!parcel.WriteBool(hasUI)) { return false; }
    if (hasUI) {
        if (interactionUI->interactionUIType == INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
            auto modal = std::static_pointer_cast<InteractionModalUIExtension>(interactionUI);
            if (!modal->Marshalling(parcel)) { return false; }
        } else {
            if (!interactionUI->Marshalling(parcel)) { return false; }
        }
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
    if (parcel.GetReadableBytes() == 0) {
        interactionInfo = nullptr;
        return true;
    }
    auto info = std::make_shared<InteractionInfo>();
    if (!info->ReadFromParcel(parcel)) {
        return false;
    }
    if (!CheckInteractionInfo(*info)) {
        interactionInfo = nullptr;
        return false;
    }
    interactionInfo = (info->interactionUI != nullptr) ? info : nullptr;
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
    if (interactionInfo != nullptr) {
        if (!interactionInfo->Marshalling(parcel)) { return false; }
    } else {
        InteractionInfo empty;
        if (!empty.Marshalling(parcel)) { return false; }
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

static void ParseWantParamsField(const cJSON *root, const char *key, std::shared_ptr<WantParams> &field)
{
    cJSON *node = cJSON_GetObjectItem(root, key);
    if (node == nullptr) {
        return;
    }
    if (cJSON_IsNull(node)) {
        field = nullptr;
        return;
    }
    if (!cJSON_IsObject(node)) {
        return;
    }
    char *nodeStr = cJSON_PrintUnformatted(node);
    if (nodeStr == nullptr) {
        return;
    }
    nlohmann::json j = nlohmann::json::parse(nodeStr, nullptr, false);
    free(nodeStr);
    if (j.is_discarded() || !j.is_object()) {
        return;
    }
    field = std::make_shared<WantParams>();
    OHOS::AAFwk::from_json(j, *field);
}

static bool ParseInteractionInfo(const cJSON *root, std::shared_ptr<InteractionInfo> &interactionInfo)
{
    cJSON *infoNode = cJSON_GetObjectItem(root, KEY_INTERACTION_INFO);
    if (infoNode == nullptr || !cJSON_IsObject(infoNode)) {
        return true;
    }
    cJSON *uiNode = cJSON_GetObjectItem(infoNode, KEY_INTERACTION_UI);
    if (uiNode == nullptr || !cJSON_IsObject(uiNode)) {
        return true;
    }
    std::string uiType;
    cJSON *typeNode = cJSON_GetObjectItem(uiNode, KEY_INTERACTION_UI_TYPE);
    if (cJSON_IsString(typeNode) && typeNode->valuestring != nullptr) {
        uiType = typeNode->valuestring;
    }
    auto info = std::make_shared<InteractionInfo>();
    if (uiType != INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        auto ui = std::make_shared<InteractionUI>();
        ui->interactionUIType = uiType;
        info->interactionUI = ui;
    } else {
        auto modal = std::make_shared<InteractionModalUIExtension>();
        modal->interactionUIType = uiType;
        cJSON *bundleNode = cJSON_GetObjectItem(uiNode, KEY_BUNDLE_NAME);
        if (cJSON_IsString(bundleNode) && bundleNode->valuestring != nullptr) {
            modal->bundleName = bundleNode->valuestring;
        }
        cJSON *abilityNode = cJSON_GetObjectItem(uiNode, KEY_ABILITY_NAME);
        if (cJSON_IsString(abilityNode) && abilityNode->valuestring != nullptr) {
            modal->abilityName = abilityNode->valuestring;
        }
        cJSON *moduleNode = cJSON_GetObjectItem(uiNode, KEY_MODULE_NAME);
        if (cJSON_IsString(moduleNode) && moduleNode->valuestring != nullptr) {
            modal->moduleName = moduleNode->valuestring;
        }
        cJSON *uiExtTypeNode = cJSON_GetObjectItem(uiNode, KEY_UI_EXTENSION_TYPE);
        if (cJSON_IsString(uiExtTypeNode) && uiExtTypeNode->valuestring != nullptr) {
            modal->uiExtensionType = uiExtTypeNode->valuestring;
        }
        cJSON *uriNode = cJSON_GetObjectItem(uiNode, KEY_URI);
        if (cJSON_IsString(uriNode) && uriNode->valuestring != nullptr) {
            modal->uri = uriNode->valuestring;
        }
        ParseWantParamsField(uiNode, KEY_PARAMETERS, modal->parameters);
        info->interactionUI = modal;
    }
    if (!InsightIntentExecuteResult::CheckInteractionInfo(*info)) {
        interactionInfo = nullptr;
        return false;
    }
    interactionInfo = info;
    return true;
}

namespace {
bool CheckCJsonDepthRecursive(const cJSON *node, size_t currentDepth, size_t maxDepth)
{
    if (currentDepth > maxDepth) {
        return false;
    }
    if (node == nullptr) {
        return true;
    }
    if (node->child != nullptr) {
        if (!CheckCJsonDepthRecursive(node->child, currentDepth + 1, maxDepth)) {
            return false;
        }
    }
    return CheckCJsonDepthRecursive(node->next, currentDepth, maxDepth);
}

bool CheckCJsonDepth(const cJSON *root, size_t maxDepth)
{
    if (root == nullptr) {
        return false;
    }
    // depth 从 1 开始（root 算第 1 层）
    if (!CheckCJsonDepthRecursive(root->child, 1, maxDepth)) {
        TAG_LOGW(AAFwkTag::INTENT, "json depth exceeds limit");
        return false;
    }
    return true;
}
} // namespace

static void ParseScalarFields(const cJSON *root, InsightIntentExecuteResult &result)
{
    cJSON *innerErrNode = cJSON_GetObjectItem(root, KEY_INNER_ERR);
    if (cJSON_IsNumber(innerErrNode)) {
        result.innerErr = static_cast<int32_t>(innerErrNode->valuedouble);
    }
    cJSON *codeNode = cJSON_GetObjectItem(root, KEY_CODE);
    if (cJSON_IsNumber(codeNode)) {
        result.code = static_cast<int32_t>(codeNode->valuedouble);
    }
    cJSON *flagsNode = cJSON_GetObjectItem(root, KEY_FLAGS);
    if (cJSON_IsNumber(flagsNode)) {
        result.flags = static_cast<int32_t>(flagsNode->valuedouble);
    }
    cJSON *isDecoratorNode = cJSON_GetObjectItem(root, KEY_IS_DECORATOR);
    if (cJSON_IsBool(isDecoratorNode)) {
        result.isDecorator = cJSON_IsTrue(isDecoratorNode);
    }
    cJSON *isNeedDelayNode = cJSON_GetObjectItem(root, KEY_IS_NEED_DELAY_RESULT);
    if (cJSON_IsBool(isNeedDelayNode)) {
        result.isNeedDelayResult = cJSON_IsTrue(isNeedDelayNode);
    }
    cJSON *isQueryEntityNode = cJSON_GetObjectItem(root, KEY_IS_QUERY_ENTITY);
    if (cJSON_IsBool(isQueryEntityNode)) {
        result.isQueryEntity = cJSON_IsTrue(isQueryEntityNode);
    }
}

static void ParseCollectionFields(const cJSON *root, InsightIntentExecuteResult &result)
{
    cJSON *urisArray = cJSON_GetObjectItem(root, KEY_URIS);
    if (cJSON_IsArray(urisArray)) {
        result.uris.clear();
        cJSON *uriItem = nullptr;
        cJSON_ArrayForEach(uriItem, urisArray)
        {
            if (cJSON_IsString(uriItem) && uriItem->valuestring != nullptr) {
                result.uris.emplace_back(uriItem->valuestring);
            }
        }
    }
    ParseWantParamsField(root, KEY_RESULT, result.result);
    cJSON *queryArray = cJSON_GetObjectItem(root, KEY_QUERY_RESULTS);
    if (cJSON_IsArray(queryArray)) {
        result.queryResults.clear();
        cJSON *queryItem = nullptr;
        cJSON_ArrayForEach(queryItem, queryArray)
        {
            if (!cJSON_IsObject(queryItem)) {
                continue;
            }
            char *itemStr = cJSON_PrintUnformatted(queryItem);
            if (itemStr == nullptr) {
                continue;
            }
            nlohmann::json itemJson = nlohmann::json::parse(itemStr, nullptr, false);
            free(itemStr);
            if (itemJson.is_discarded() || !itemJson.is_object()) {
                continue;
            }
            auto queryResult = std::make_shared<WantParams>();
            OHOS::AAFwk::from_json(itemJson, *queryResult);
            result.queryResults.emplace_back(queryResult);
        }
    }
}

bool InsightIntentExecuteResult::FromJsonString(const std::string &jsonStr)
{
    if (jsonStr.size() > MAX_RESULT_JSON_LEN) {
        TAG_LOGW(AAFwkTag::INTENT, "json size exceeds limit: %{public}zu", jsonStr.size());
        return false;
    }
    cJSON *rootRaw = cJSON_Parse(jsonStr.c_str());
    if (rootRaw == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "failed to parse json");
        return false;
    }
    cJSONUniquePtr root(rootRaw);
    if (!cJSON_IsObject(rootRaw)) {
        return false;
    }
    if (!CheckCJsonDepth(rootRaw, MAX_RESULT_JSON_DEPTH)) {
        return false;
    }
    ParseScalarFields(rootRaw, *this);
    ParseCollectionFields(rootRaw, *this);
    return ParseInteractionInfo(rootRaw, interactionInfo);
}

static cJSON *BuildInteractionUICJson(const std::shared_ptr<InteractionUI> &ui)
{
    cJSON *uiJson = cJSON_CreateObject();
    if (uiJson == nullptr) {
        return nullptr;
    }
    cJSON_AddStringToObject(uiJson, KEY_INTERACTION_UI_TYPE, ui->interactionUIType.c_str());
    if (ui->interactionUIType != INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        return uiJson;
    }
    auto modalUI = std::static_pointer_cast<InteractionModalUIExtension>(ui);
    cJSON_AddStringToObject(uiJson, KEY_BUNDLE_NAME, modalUI->bundleName.c_str());
    cJSON_AddStringToObject(uiJson, KEY_ABILITY_NAME, modalUI->abilityName.c_str());
    cJSON_AddStringToObject(uiJson, KEY_MODULE_NAME, modalUI->moduleName.c_str());
    cJSON_AddStringToObject(uiJson, KEY_UI_EXTENSION_TYPE, modalUI->uiExtensionType.c_str());
    cJSON_AddStringToObject(uiJson, KEY_URI, modalUI->uri.c_str());
    if (modalUI->parameters != nullptr) {
        nlohmann::json paramsJson;
        OHOS::AAFwk::to_json(paramsJson, *modalUI->parameters);
        cJSON *paramsNode = cJSON_Parse(paramsJson.dump().c_str());
        if (paramsNode != nullptr) {
            cJSON_AddItemToObject(uiJson, KEY_PARAMETERS, paramsNode);
        }
    }
    return uiJson;
}

std::string InsightIntentExecuteResult::ToJsonString() const
{
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        return "";
    }
    cJSONUniquePtr rootPtr(root);

    cJSON_AddNumberToObject(root, KEY_INNER_ERR, innerErr);
    cJSON_AddNumberToObject(root, KEY_CODE, code);
    cJSON_AddNumberToObject(root, KEY_FLAGS, flags);
    cJSON_AddBoolToObject(root, KEY_IS_DECORATOR, isDecorator);
    cJSON_AddBoolToObject(root, KEY_IS_NEED_DELAY_RESULT, isNeedDelayResult);
    cJSON_AddBoolToObject(root, KEY_IS_QUERY_ENTITY, isQueryEntity);

    cJSON *urisArray = cJSON_CreateArray();
    if (urisArray != nullptr) {
        for (const auto &uri : uris) {
            cJSON_AddItemToArray(urisArray, cJSON_CreateString(uri.c_str()));
        }
        cJSON_AddItemToObject(root, KEY_URIS, urisArray);
    }

    if (result != nullptr) {
        nlohmann::json resultJson;
        OHOS::AAFwk::to_json(resultJson, *result);
        cJSON *resultNode = cJSON_Parse(resultJson.dump().c_str());
        if (resultNode != nullptr) {
            cJSON_AddItemToObject(root, KEY_RESULT, resultNode);
        }
    } else {
        cJSON_AddNullToObject(root, KEY_RESULT);
    }

    cJSON *queryArray = cJSON_CreateArray();
    if (queryArray != nullptr) {
        for (const auto &item : queryResults) {
            if (item == nullptr) {
                continue;
            }
            nlohmann::json itemJson;
            OHOS::AAFwk::to_json(itemJson, *item);
            cJSON *itemNode = cJSON_Parse(itemJson.dump().c_str());
            if (itemNode != nullptr) {
                cJSON_AddItemToArray(queryArray, itemNode);
            }
        }
        cJSON_AddItemToObject(root, KEY_QUERY_RESULTS, queryArray);
    }

    if (interactionInfo != nullptr && interactionInfo->interactionUI != nullptr) {
        cJSON *infoJson = cJSON_CreateObject();
        if (infoJson != nullptr) {
            cJSON *uiJson = BuildInteractionUICJson(interactionInfo->interactionUI);
            if (uiJson != nullptr) {
                cJSON_AddItemToObject(infoJson, KEY_INTERACTION_UI, uiJson);
                cJSON_AddItemToObject(root, KEY_INTERACTION_INFO, infoJson);
            } else {
                cJSON_Delete(infoJson);
            }
        }
    }

    char *str = cJSON_PrintUnformatted(root);
    std::string out = (str != nullptr) ? str : "";
    free(str);
    return out;
}

bool InsightIntentExecuteResult::CheckResult(std::shared_ptr<const WantParams> result)
{
    return true;
}

static bool HasControlChars(const std::string &s)
{
    return s.find('\r') != std::string::npos ||
        s.find('\n') != std::string::npos ||
        s.find('\0') != std::string::npos;
}

static bool CheckField(const std::string &s, size_t maxLen, const char *name)
{
    if (s.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "CheckInteractionInfo failed: empty %{public}s", name);
        return false;
    }
    if (s.size() > maxLen) {
        TAG_LOGW(AAFwkTag::INTENT, "CheckInteractionInfo failed: %{public}s too long", name);
        return false;
    }
    if (HasControlChars(s)) {
        TAG_LOGW(AAFwkTag::INTENT, "CheckInteractionInfo failed: %{public}s has control char", name);
        return false;
    }
    return true;
}

bool InsightIntentExecuteResult::CheckInteractionInfo(const InteractionInfo &intent)
{
    if (intent.interactionUI == nullptr) {
        return true;
    }
    const auto &type = intent.interactionUI->interactionUIType;
    if (type.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "CheckInteractionInfo failed: empty interactionUIType");
        return false;
    }
    if (type != INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
        TAG_LOGW(AAFwkTag::INTENT, "CheckInteractionInfo failed: unsupported interactionUIType");
        return false;
    }
    auto modal = std::static_pointer_cast<InteractionModalUIExtension>(intent.interactionUI);
    if (!CheckField(modal->bundleName, MAX_BUNDLE_NAME_LEN, "bundleName")) {
        return false;
    }
    if (!CheckField(modal->moduleName, MAX_MODULE_NAME_LEN, "moduleName")) {
        return false;
    }
    if (!CheckField(modal->abilityName, MAX_ABILITY_NAME_LEN, "abilityName")) {
        return false;
    }
    if (!CheckField(modal->uiExtensionType, MAX_UI_EXTENSION_TYPE_LEN, "uiExtensionType")) {
        return false;
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
    if (interactionInfo != nullptr && interactionInfo->interactionUI != nullptr) {
        auto infoParams = std::make_shared<WantParams>();
        auto uiParams = std::make_shared<WantParams>();
        uiParams->SetParam(KEY_INTERACTION_UI_TYPE,
            OHOS::AAFwk::String::Box(interactionInfo->interactionUI->interactionUIType));
        if (interactionInfo->interactionUI->interactionUIType ==
            INTERACTION_UI_TYPE_MODAL_UIEXTENSION) {
            auto modalUI = std::static_pointer_cast<InteractionModalUIExtension>(
                interactionInfo->interactionUI);
            uiParams->SetParam(KEY_BUNDLE_NAME, OHOS::AAFwk::String::Box(modalUI->bundleName));
            uiParams->SetParam(KEY_ABILITY_NAME, OHOS::AAFwk::String::Box(modalUI->abilityName));
            uiParams->SetParam(KEY_MODULE_NAME, OHOS::AAFwk::String::Box(modalUI->moduleName));
            uiParams->SetParam(KEY_UI_EXTENSION_TYPE, OHOS::AAFwk::String::Box(modalUI->uiExtensionType));
            uiParams->SetParam(KEY_URI, OHOS::AAFwk::String::Box(modalUI->uri));
            if (modalUI->parameters != nullptr) {
                uiParams->SetParam(KEY_PARAMETERS,
                    OHOS::AAFwk::WantParamWrapper::Box(*modalUI->parameters));
            }
        }
        infoParams->SetParam(KEY_INTERACTION_UI, OHOS::AAFwk::WantParamWrapper::Box(*uiParams));
        resultParams->SetParam(KEY_INTERACTION_INFO, OHOS::AAFwk::WantParamWrapper::Box(*infoParams));
    }
    return resultParams;
}
} // namespace AppExecFwk
} // namespace OHOS
