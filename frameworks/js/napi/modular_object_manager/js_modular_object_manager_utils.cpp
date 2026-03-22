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

#include "js_modular_object_manager_utils.h"

#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
bool UnwrapStringOfModularObjectExtensionInfo(napi_env env, napi_value param, AAFwk::ModularObjectExtensionInfo &info)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parameter error");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "bundleName") &&
        !UnwrapStringByPropertyName(env, param, "bundleName", info.bundleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert bundleName failed");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "moduleName") &&
        !UnwrapStringByPropertyName(env, param, "moduleName", info.moduleName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert moduleName failed");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "abilityName") &&
        !UnwrapStringByPropertyName(env, param, "abilityName", info.abilityName)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert abilityName failed");
        return false;
    }

    return true;
}

bool UnwrapIntOfModularObjectExtensionInfo(napi_env env, napi_value param, AAFwk::ModularObjectExtensionInfo &info)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parameter error");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "appIndex") &&
        !UnwrapInt32ByPropertyName(env, param, "appIndex", info.appIndex)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert appIndex failed");
        return false;
    }

    return true;
}

bool UnwrapBoolOfModularObjectExtensionInfo(napi_env env, napi_value param, AAFwk::ModularObjectExtensionInfo &info)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parameter error");
        return false;
    }

    if (IsExistsByPropertyName(env, param, "isSystemDisabled") &&
        !UnwrapBooleanByPropertyName(env, param, "isSystemDisabled", info.isSystemDisabled)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "convert isSystemDisabled failed");
        return false;
    }

    return true;
}

bool WrapStringOfModularObjectExtensionInfo(napi_env env, napi_value jsObject,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (jsObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsObject");
        return false;
    }

    napi_value jsValue = nullptr;
    jsValue = WrapStringToJS(env, info.bundleName);
    if (jsValue == nullptr || !SetPropertyValueByPropertyName(env, jsObject, "bundleName", jsValue)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set bundleName failed");
        return false;
    }

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, info.moduleName);
    if (jsValue == nullptr || !SetPropertyValueByPropertyName(env, jsObject, "moduleName", jsValue)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set moduleName failed");
        return false;
    }

    jsValue = nullptr;
    jsValue = WrapStringToJS(env, info.abilityName);
    if (jsValue == nullptr || !SetPropertyValueByPropertyName(env, jsObject, "abilityName", jsValue)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set abilityName failed");
        return false;
    }

    return true;
}

bool WrapIntOfModularObjectExtensionInfo(napi_env env, napi_value jsObject,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (jsObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsObject");
        return false;
    }

    napi_value jsValue = nullptr;
    jsValue = WrapInt32ToJS(env, info.appIndex);
    if (jsValue == nullptr || !SetPropertyValueByPropertyName(env, jsObject, "appIndex", jsValue)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set appIndex failed");
        return false;
    }

    return true;
}

bool WrapBoolOfModularObjectExtensionInfo(napi_env env, napi_value jsObject,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (jsObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsObject");
        return false;
    }

    napi_value jsValue = nullptr;
    jsValue = nullptr;
    jsValue = WrapBoolToJS(env, info.isSystemDisabled);
    if (jsValue == nullptr || !SetPropertyValueByPropertyName(env, jsObject, "isSystemDisabled", jsValue)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set isSystemDisabled failed");
        return false;
    }

    return true;
}
}  // namespace
bool UnwrapModularObjectExtensionInfo(napi_env env, napi_value param, AAFwk::ModularObjectExtensionInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called UnwrapModularObjectExtensionInfo");

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parameter error");
        return false;
    }

    if (!UnwrapStringOfModularObjectExtensionInfo(env, param, info)) {
        return false;
    }

    if (!UnwrapIntOfModularObjectExtensionInfo(env, param, info)) {
        return false;
    }

    if (!UnwrapBoolOfModularObjectExtensionInfo(env, param, info)) {
        return false;
    }

    return true;
}

napi_value WrapModularObjectExtensionInfo(napi_env env, const AAFwk::ModularObjectExtensionInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called WrapModularObjectExtensionInfo");

    HandleEscape handleEscape(env);
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));
    if (jsObject == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null jsObject");
        return nullptr;
    }

    if (!WrapStringOfModularObjectExtensionInfo(env, jsObject, info)) {
        return nullptr;
    }

    if (!WrapIntOfModularObjectExtensionInfo(env, jsObject, info)) {
        return nullptr;
    }

    if (!WrapBoolOfModularObjectExtensionInfo(env, jsObject, info)) {
        return nullptr;
    }

    return handleEscape.Escape(jsObject);
}

napi_value WrapModularObjectExtensionInfoArray(napi_env env,
    const std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called WrapModularObjectExtensionInfoArray");

    HandleEscape handleEscape(env);
    napi_value arrayValue = nullptr;
    napi_status createStatus = napi_create_array_with_length(env, infos.size(), &arrayValue);
    if (createStatus != napi_ok || arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "napi_create_array_with_length failed, %{public}d", createStatus);
        return nullptr;
    }
    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_set_element(env, arrayValue, index++, WrapModularObjectExtensionInfo(env, info));
    }
    return handleEscape.Escape(arrayValue);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
