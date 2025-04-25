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

#include "js_insight_intent_driver_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string INSIGHT_INTENTS_DECORATOR_TYPE_LINK = "@InsightIntentLink";
const std::string INSIGHT_INTENTS_DECORATOR_TYPE_PAGE = "@InsightIntentPage";
const std::string INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY = "@InsightIntentEntry";
const std::string INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION = "@InsightIntentFunction";
const std::string INSIGHT_INTENTS_DECORATOR_TYPE_FORM = "@InsightIntentForm";
}

napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "code", CreateJsValue(env, result.code));
    if (result.result != nullptr) {
        napi_set_named_property(env, objValue, "result",
            OHOS::AppExecFwk::CreateJsWantParams(env, *result.result));
    }
    if (result.uris.size() > 0) {
        napi_set_named_property(env, objValue, "uris", CreateNativeArray(env, result.uris));
    }
    napi_set_named_property(env, objValue, "flags", CreateJsValue(env, result.flags));
    return objValue;
}

napi_value CreateJsLinkInfo(napi_env env, const JsLinkInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "uri", CreateJsValue(env, info.uri));
    return objValue;
}

napi_value CreateJsPageInfo(napi_env env, const JsPageInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "uiAbility", CreateJsValue(env, info.uiAbility));
    napi_set_named_property(env, objValue, "pageRouterName", CreateJsValue(env, info.pageRouterName));
    napi_set_named_property(env, objValue, "navigationId", CreateJsValue(env, info.navigationId));
    napi_set_named_property(env, objValue, "navDestination", CreateJsValue(env, info.navDestination));
    return objValue;
}

napi_value CreateJsEntryInfo(napi_env env, const JsEntryInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "abilityName", CreateJsValue(env, info.abilityName));
    napi_set_named_property(env, objValue, "executeMode", CreateNativeArray(env, info.executeMode));
    return objValue;
}

napi_value CreateJsFunctionInfo(napi_env env, const JsFunctionInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    return objValue;
}

napi_value CreateJsFormInfo(napi_env env, const JsFormInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    return objValue;
}

napi_value CreateJsInsightIntentInfo(napi_env env, const JsInsightIntentInfo &info)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, info.bundleName));
    napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, info.moduleName));
    napi_set_named_property(env, objValue, "intentName", CreateJsValue(env, info.intentName));
    napi_set_named_property(env, objValue, "domain", CreateJsValue(env, info.domain));
    napi_set_named_property(env, objValue, "intentVersion", CreateJsValue(env, info.intentVersion));
    napi_set_named_property(env, objValue, "displayName", CreateJsValue(env, info.displayName));
    napi_set_named_property(env, objValue, "displayDescription", CreateJsValue(env, info.displayDescription));
    napi_set_named_property(env, objValue, "schema", CreateJsValue(env, info.schema));
    napi_set_named_property(env, objValue, "icon", CreateJsValue(env, info.icon));
    napi_set_named_property(env, objValue, "llmDescription", CreateJsValue(env, info.llmDescription));
    napi_set_named_property(env, objValue, "intentType", CreateJsValue(env, info.intentType));
    napi_set_named_property(env, objValue, "params", CreateJsValue(env, info.params));
    napi_set_named_property(env, objValue, "keywords", CreateNativeArray(env, info.keywords));
    if (info.intentType == INSIGHT_INTENTS_DECORATOR_TYPE_LINK) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateJsLinkInfo(env, info.linkInfo));
    } else if (info.intentType == INSIGHT_INTENTS_DECORATOR_TYPE_PAGE) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateJsPageInfo(env, info.pageInfo));
    } else if (info.intentType == INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateJsEntryInfo(env, info.entryInfo));
    } else if (info.intentType == INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateJsFunctionInfo(env, info.functionInfo));
    } else if (info.intentType == INSIGHT_INTENTS_DECORATOR_TYPE_FORM) {
        napi_set_named_property(env, objValue, "subIntentInfo", CreateJsFormInfo(env, info.formInfo));
    } else {
        napi_value objVal = nullptr;
        napi_create_object(env, &objValue);
        napi_set_named_property(env, objValue, "subIntentInfo", objVal);
    }
    return objValue;
}

napi_value CreateJsInsightIntentInfoArray(napi_env env, const std::vector<JsInsightIntentInfo> &infos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, infos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &info : infos) {
        napi_set_element(env, arrayValue, index++, CreateJsInsightIntentInfo(env, info));
    }
    return arrayValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
