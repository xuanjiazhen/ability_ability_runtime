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

#include "js_common_utils.h"
#include <nlohmann/json.hpp>
#include "napi_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
napi_value ConvertJsonValue(napi_env env, const nlohmann::json &value)
{
    if (value.is_string()) {
        return AppExecFwk::WrapStringToJS(env, value.get<std::string>());
    }
    if (value.is_number_integer()) {
        return AppExecFwk::WrapInt32ToJS(env, value.get<int32_t>());
    }
    if (value.is_boolean()) {
        return AppExecFwk::WrapBoolToJS(env, value.get<bool>());
    }
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}
}

napi_value ParseJsonStringToJsObject(napi_env env, const std::string &jsonStr)
{
    napi_value jsObj = nullptr;
    napi_create_object(env, &jsObj);

    if (jsonStr.empty()) {
        return jsObj;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        return jsObj;
    }

    napi_value jsValue = nullptr;
    if (jsonObj.is_object()) {
        napi_create_object(env, &jsValue);
        for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
            napi_value jsItem = nullptr;
            if (it.value().is_object() || it.value().is_array()) {
                jsItem = ParseJsonStringToJsObject(env, it.value().dump());
            } else {
                jsItem = ConvertJsonValue(env, it.value());
            }
            napi_set_named_property(env, jsValue, it.key().c_str(), jsItem);
        }
    } else if (jsonObj.is_array()) {
        napi_create_array(env, &jsValue);
        size_t index = 0;
        for (auto &item : jsonObj) {
            napi_value jsItem = nullptr;
            if (item.is_object() || item.is_array()) {
                jsItem = ParseJsonStringToJsObject(env, item.dump());
            } else {
                jsItem = ConvertJsonValue(env, item);
            }
            napi_set_element(env, jsValue, index++, jsItem);
        }
    } else {
        napi_create_object(env, &jsValue);
    }

    return jsValue;
}

} // namespace CliTool
} // namespace OHOS
