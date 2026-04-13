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

#include "arkts_script.h"

#include <sstream>

#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"

#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"

namespace OHOS {
namespace ArktsScript {

using namespace OHOS::AbilityRuntime;

namespace {

std::string GetNapiErrorMessage(napi_env env)
{
    if (env == nullptr) {
        return "napi_env is null";
    }

    const napi_extended_error_info* errorInfo = nullptr;
    napi_status status = napi_get_last_error_info(env, &errorInfo);
    if (status != napi_ok || errorInfo == nullptr || errorInfo->error_message == nullptr) {
        return "unknown napi error";
    }
    return errorInfo->error_message;
}

bool GetUtf8String(napi_env env, napi_value value, std::string& output)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }

    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(length) failed: %{public}s",
            GetNapiErrorMessage(env).c_str());
        return false;
    }

    std::string buffer(length + 1, '\0');
    status = napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(data) failed: %{public}s",
            GetNapiErrorMessage(env).c_str());
        return false;
    }

    buffer.resize(length);
    output = buffer;
    return true;
}

bool RemoveNamedProperty(napi_env env, napi_value object, const char* name)
{
    bool hasProperty = false;
    napi_status status = napi_has_named_property(env, object, name, &hasProperty);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_has_named_property failed for %{public}s: %{public}s",
            name, GetNapiErrorMessage(env).c_str());
        return false;
    }
    if (!hasProperty) {
        return true;
    }

    napi_value key = nullptr;
    status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &key);
    if (status != napi_ok || key == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_create_string_utf8 failed for %{public}s: %{public}s",
            name, GetNapiErrorMessage(env).c_str());
        return false;
    }

    bool deleted = false;
    status = napi_delete_property(env, object, key, &deleted);
    if (status != napi_ok || !deleted) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_delete_property failed for %{public}s: %{public}s",
            name, GetNapiErrorMessage(env).c_str());
        return false;
    }
    return true;
}

bool ArgValueToJson(const ArgValue& arg, nlohmann::json& jsonValue, bool& isUndefined)
{
    isUndefined = false;
    switch (arg.type) {
        case ArgType::STRING:
            jsonValue = arg.GetString();
            return true;
        case ArgType::NUMBER:
            jsonValue = arg.GetNumber();
            return true;
        case ArgType::BOOLEAN:
            jsonValue = arg.GetBool();
            return true;
        case ArgType::NULL_VAL:
            jsonValue = nullptr;
            return true;
        case ArgType::UNDEFINED:
            isUndefined = true;
            return true;
        case ArgType::ARRAY:
        case ArgType::JSON_OBJECT: {
            const std::string serializedJsonValue = arg.GetString();
            nlohmann::json jsonObject = nlohmann::json::parse(serializedJsonValue, nullptr, false);
            if (jsonObject.is_discarded()) {
                TAG_LOGE(AAFwkTag::APPKIT, "invalid JSON argument: %{public}s", serializedJsonValue.c_str());
                return false;
            }
            if (arg.type == ArgType::ARRAY && !jsonObject.is_array()) {
                TAG_LOGE(AAFwkTag::APPKIT, "array argument must be a top-level array: %{public}s",
                    serializedJsonValue.c_str());
                return false;
            }
            if (arg.type == ArgType::JSON_OBJECT && !jsonObject.is_object()) {
                TAG_LOGE(AAFwkTag::APPKIT, "jsonString argument must be a top-level object: %{public}s",
                    serializedJsonValue.c_str());
                return false;
            }
            jsonValue = std::move(jsonObject);
            return true;
        }
        default:
            isUndefined = true;
            return true;
    }
}

napi_value JsonToNapiValue(napi_env env, const nlohmann::json& jsonValue)
{
    if (jsonValue.is_null()) {
        return CreateJsNull(env);
    }
    if (jsonValue.is_boolean()) {
        return CreateJsValue(env, jsonValue.get<bool>());
    }
    if (jsonValue.is_number_integer()) {
        return CreateJsValue(env, jsonValue.get<int64_t>());
    }
    if (jsonValue.is_number_unsigned()) {
        return CreateJsValue(env, static_cast<double>(jsonValue.get<uint64_t>()));
    }
    if (jsonValue.is_number_float()) {
        return CreateJsValue(env, jsonValue.get<double>());
    }
    if (jsonValue.is_string()) {
        return CreateJsValue(env, jsonValue.get<std::string>());
    }
    if (jsonValue.is_array()) {
        napi_value arrayValue = nullptr;
        napi_status status = napi_create_array_with_length(env, jsonValue.size(), &arrayValue);
        if (status != napi_ok || arrayValue == nullptr) {
            return nullptr;
        }

        uint32_t index = 0;
        for (const auto& item : jsonValue) {
            napi_value elementValue = JsonToNapiValue(env, item);
            if (elementValue == nullptr) {
                return nullptr;
            }
            status = napi_set_element(env, arrayValue, index++, elementValue);
            if (status != napi_ok) {
                return nullptr;
            }
        }
        return arrayValue;
    }
    if (jsonValue.is_object()) {
        napi_value objectValue = nullptr;
        napi_status status = napi_create_object(env, &objectValue);
        if (status != napi_ok || objectValue == nullptr) {
            return nullptr;
        }
        for (const auto& item : jsonValue.items()) {
            napi_value propertyValue = JsonToNapiValue(env, item.value());
            if (propertyValue == nullptr) {
                return nullptr;
            }
            status = napi_set_named_property(env, objectValue, item.key().c_str(), propertyValue);
            if (status != napi_ok) {
                return nullptr;
            }
        }
        return objectValue;
    }
    return CreateJsUndefined(env);
}

} // namespace

bool BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid env or context when binding scriptContext");
        return false;
    }

    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_global failed: %{public}s", GetNapiErrorMessage(env).c_str());
        return false;
    }

    std::shared_ptr<Context> baseContext = context;
    napi_value contextObj = CreateJsBaseContext(env, baseContext, true);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "CreateJsBaseContext failed: %{public}s", GetNapiErrorMessage(env).c_str());
        return false;
    }

    constexpr const char* EXCLUDED_PROPERTIES[] = {
        "resourceManager",
        "eventHub",
        "currentHapModuleInfo",
        "index",
        "contextType",
        "switchArea",
        "getArea",
        "getGroupDir",
        "createModuleContext",
        "createSystemHspModuleResourceManager",
        "createModuleResourceManager",
        "createDisplayContext",
        "createAreaModeContext",
    };
    for (const char* property : EXCLUDED_PROPERTIES) {
        if (!RemoveNamedProperty(env, contextObj, property)) {
            return false;
        }
    }

    status = napi_set_named_property(env, globalObj, "scriptContext", contextObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to bind scriptContext: %{public}s", GetNapiErrorMessage(env).c_str());
        return false;
    }

    return true;
}

bool LoadAbcFile(JsRuntime* runtime, const std::string& path)
{
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime is null when loading abc");
        return false;
    }

    bool loaded = runtime->RunScript(path, "", false);
    if (!loaded) {
        napi_env env = runtime->GetNapiEnv();
        TAG_LOGE(AAFwkTag::APPKIT, "RunScript failed for %{private}s: %{public}s",
            path.c_str(), GetNapiErrorMessage(env).c_str());
    }
    return loaded;
}

napi_value ArgValueToNapi(napi_env env, const ArgValue& arg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is null when converting argument");
        return nullptr;
    }

    nlohmann::json jsonValue;
    bool isUndefined = false;
    if (!ArgValueToJson(arg, jsonValue, isUndefined)) {
        return nullptr;
    }

    if (isUndefined) {
        napi_value result = nullptr;
        napi_status status = napi_get_undefined(env, &result);
        if (status != napi_ok || result == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "argument conversion failed: %{public}s", GetNapiErrorMessage(env).c_str());
            return nullptr;
        }
        return result;
    }

    napi_value result = JsonToNapiValue(env, jsonValue);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "argument conversion failed: %{public}s", GetNapiErrorMessage(env).c_str());
        return nullptr;
    }
    return result;
}

CallResult CallFunction(napi_env env, const std::string& funName, const std::vector<ArgValue>& argv)
{
    CallResult callResult;
    if (env == nullptr) {
        callResult.status = CallStatus::INVALID_ENV;
        callResult.errorMessage = "napi_env is null";
        return callResult;
    }

    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        callResult.status = CallStatus::INVALID_ENV;
        callResult.errorMessage = "failed to get global object: " + GetNapiErrorMessage(env);
        return callResult;
    }

    napi_value func = nullptr;
    status = napi_get_named_property(env, globalObj, funName.c_str(), &func);
    if (status != napi_ok) {
        callResult.status = CallStatus::PROPERTY_ACCESS_FAILED;
        callResult.errorMessage = "failed to get function '" + funName + "': " + GetNapiErrorMessage(env);
        return callResult;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, func, &valueType);
    if (status != napi_ok) {
        callResult.status = CallStatus::PROPERTY_ACCESS_FAILED;
        callResult.errorMessage = "failed to inspect function '" + funName + "': " + GetNapiErrorMessage(env);
        return callResult;
    }

    if (valueType == napi_undefined) {
        callResult.status = CallStatus::FUNCTION_NOT_FOUND;
        callResult.errorMessage = "function '" + funName + "' was not found on globalThis";
        return callResult;
    }

    if (valueType != napi_function) {
        callResult.status = CallStatus::NOT_A_FUNCTION;
        callResult.errorMessage = "'" + funName + "' exists on globalThis, but it is not a function";
        return callResult;
    }

    std::vector<napi_value> napiArgs;
    napiArgs.reserve(argv.size());
    for (const auto& arg : argv) {
        napi_value argValue = ArgValueToNapi(env, arg);
        if (argValue == nullptr) {
            callResult.status = CallStatus::ARGUMENT_CONVERSION_FAILED;
            callResult.errorMessage = "failed to convert argument: " + arg.ToString();
            return callResult;
        }
        napiArgs.push_back(argValue);
    }

    napi_value result = nullptr;
    status = napi_call_function(env, globalObj, func, napiArgs.size(), napiArgs.data(), &result);
    if (status != napi_ok) {
        callResult.status = CallStatus::CALL_FAILED;
        callResult.errorMessage = "failed to call '" + funName + "': " + GetNapiErrorMessage(env);
        return callResult;
    }

    callResult.status = CallStatus::SUCCESS;
    callResult.value = result;
    return callResult;
}

std::string NapiValueToString(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return "undefined";
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_typeof failed: %{public}s", GetNapiErrorMessage(env).c_str());
        return "[napi-error]";
    }

    if (valueType == napi_undefined) {
        return "undefined";
    }
    if (valueType == napi_null) {
        return "null";
    }
    if (valueType == napi_boolean) {
        bool boolValue = false;
        if (napi_get_value_bool(env, value, &boolValue) != napi_ok) {
            return "[boolean]";
        }
        return boolValue ? "true" : "false";
    }
    if (valueType == napi_number) {
        double numberValue = 0.0;
        if (napi_get_value_double(env, value, &numberValue) != napi_ok) {
            return "[number]";
        }
        std::ostringstream os;
        os << numberValue;
        return os.str();
    }
    if (valueType == napi_string) {
        std::string stringValue;
        if (!GetUtf8String(env, value, stringValue)) {
            return "[string]";
        }
        return "\"" + stringValue + "\"";
    }
    if (valueType == napi_object) {
        napi_value global = nullptr;
        status = napi_get_global(env, &global);
        if (status != napi_ok || global == nullptr) {
            return "[object]";
        }

        napi_value jsonObj = nullptr;
        status = napi_get_named_property(env, global, "JSON", &jsonObj);
        if (status != napi_ok || jsonObj == nullptr) {
            return "[object]";
        }

        napi_value stringifyFunc = nullptr;
        status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
        if (status != napi_ok || stringifyFunc == nullptr) {
            return "[object]";
        }

        napi_value argv[] = { value };
        napi_value jsonString = nullptr;
        status = napi_call_function(env, jsonObj, stringifyFunc, 1, argv, &jsonString);
        if (status != napi_ok || jsonString == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "JSON.stringify failed: %{public}s", GetNapiErrorMessage(env).c_str());
            return "[object]";
        }

        std::string objectString;
        if (!GetUtf8String(env, jsonString, objectString)) {
            return "[object]";
        }
        return objectString;
    }
    if (valueType == napi_function) {
        return "[function]";
    }
    if (valueType == napi_symbol) {
        return "[symbol]";
    }
    if (valueType == napi_bigint) {
        return "[bigint]";
    }

    return "unknown";
}

} // namespace ArktsScript
} // namespace OHOS
