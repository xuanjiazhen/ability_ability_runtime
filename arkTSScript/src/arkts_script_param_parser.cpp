#include "arkts_script_param_parser.h"

#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace ArkTSScript {
namespace {

bool GetUtf8String(napi_env env, napi_value value, std::string &output)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }

    size_t length = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &length) != napi_ok) {
        return false;
    }

    std::string buffer(length + 1, '\0');
    if (napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length) != napi_ok) {
        return false;
    }

    buffer.resize(length);
    output = buffer;
    return true;
}

bool ParseArgIndex(const std::string &key, int32_t &index)
{
    constexpr char ARG_PREFIX[] = "arg";
    if (key.rfind(ARG_PREFIX, 0) != 0 || key.size() <= sizeof(ARG_PREFIX) - 1) {
        return false;
    }

    index = 0;
    for (size_t i = sizeof(ARG_PREFIX) - 1; i < key.size(); ++i) {
        const char ch = key[i];
        if (ch < '0' || ch > '9') {
            return false;
        }
        index = index * 10 + (ch - '0');
    }
    return index > 0;
}

std::string EscapeCliValue(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '\\':
                escaped += "\\\\";
                break;
            case '"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

bool GetAndClearExceptionMessage(napi_env env, std::string &message)
{
    bool hasPending = false;
    if (napi_is_exception_pending(env, &hasPending) != napi_ok || !hasPending) {
        return false;
    }

    napi_value exception = nullptr;
    if (napi_get_and_clear_last_exception(env, &exception) != napi_ok || exception == nullptr) {
        return false;
    }

    napi_value exceptionString = nullptr;
    if (napi_coerce_to_string(env, exception, &exceptionString) != napi_ok || exceptionString == nullptr) {
        return false;
    }

    return GetUtf8String(env, exceptionString, message);
}

bool JsonStringify(napi_env env, napi_value value, std::string &jsonString, std::string &errorMessage)
{
    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok || global == nullptr) {
        errorMessage = "failed to access global object";
        return false;
    }

    napi_value jsonObj = nullptr;
    if (napi_get_named_property(env, global, "JSON", &jsonObj) != napi_ok || jsonObj == nullptr) {
        errorMessage = "failed to access JSON object";
        return false;
    }

    napi_value stringify = nullptr;
    if (napi_get_named_property(env, jsonObj, "stringify", &stringify) != napi_ok || stringify == nullptr) {
        errorMessage = "failed to access JSON.stringify";
        return false;
    }

    napi_value argv[] = { value };
    napi_value result = nullptr;
    if (napi_call_function(env, jsonObj, stringify, 1, argv, &result) != napi_ok || result == nullptr) {
        if (!GetAndClearExceptionMessage(env, errorMessage)) {
            errorMessage = "JSON.stringify failed";
        }
        return false;
    }

    if (!GetUtf8String(env, result, jsonString)) {
        errorMessage = "failed to convert JSON string to UTF-8";
        return false;
    }
    return true;
}

bool ConvertJsValueToTypedArg(
    napi_env env, const std::string &key, napi_value value, std::string &typedArg, std::string &errorMessage)
{
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, value, &valueType) != napi_ok) {
        errorMessage = "failed to inspect value type for '" + key + "'";
        return false;
    }

    switch (valueType) {
        case napi_string: {
            std::string stringValue;
            if (!GetUtf8String(env, value, stringValue)) {
                errorMessage = "failed to read string value for '" + key + "'";
                return false;
            }
            typedArg = "str:" + stringValue;
            return true;
        }
        case napi_number: {
            double numberValue = 0.0;
            if (napi_get_value_double(env, value, &numberValue) != napi_ok || !std::isfinite(numberValue)) {
                errorMessage = "invalid number value for '" + key + "'";
                return false;
            }
            std::ostringstream os;
            os << std::setprecision(15) << numberValue;
            typedArg = "num:" + os.str();
            return true;
        }
        case napi_boolean: {
            bool boolValue = false;
            if (napi_get_value_bool(env, value, &boolValue) != napi_ok) {
                errorMessage = "failed to read boolean value for '" + key + "'";
                return false;
            }
            typedArg = boolValue ? "bool:true" : "bool:false";
            return true;
        }
        case napi_object: {
            bool isArray = false;
            if (napi_is_array(env, value, &isArray) != napi_ok) {
                errorMessage = "failed to inspect object value for '" + key + "'";
                return false;
            }

            std::string jsonString;
            if (!JsonStringify(env, value, jsonString, errorMessage)) {
                errorMessage = "failed to serialize '" + key + "': " + errorMessage;
                return false;
            }
            if (jsonString.empty()) {
                errorMessage = "serialized value for '" + key + "' is empty";
                return false;
            }

            if (isArray) {
                if (jsonString.front() != '[') {
                    errorMessage = "array value for '" + key + "' did not serialize to a JSON array";
                    return false;
                }
                typedArg = "array:" + jsonString;
                return true;
            }

            if (jsonString.front() != '{') {
                errorMessage = "object value for '" + key + "' must serialize to a JSON object";
                return false;
            }
            typedArg = "jsonString:" + jsonString;
            return true;
        }
        case napi_null:
            errorMessage = "'" + key + "' does not support null";
            return false;
        case napi_undefined:
            errorMessage = "'" + key + "' does not support undefined";
            return false;
        case napi_function:
            errorMessage = "'" + key + "' does not support function values";
            return false;
        case napi_symbol:
            errorMessage = "'" + key + "' does not support symbol values";
            return false;
        case napi_bigint:
            errorMessage = "'" + key + "' does not support bigint values";
            return false;
        default:
            errorMessage = "unsupported value type for '" + key + "'";
            return false;
    }
}

} // namespace

bool ParseArktsScriptParams(
    napi_env env, napi_value jsParam, std::string &cliArgs, std::string &errorMessage)
{
    cliArgs.clear();
    errorMessage.clear();

    if (env == nullptr || jsParam == nullptr) {
        errorMessage = "arkTSScript params are missing";
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, jsParam, &valueType) != napi_ok || valueType != napi_object) {
        errorMessage = "arkTSScript params must be an object";
        return false;
    }

    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, jsParam, &propertyNames) != napi_ok || propertyNames == nullptr) {
        errorMessage = "failed to enumerate arkTSScript params";
        return false;
    }

    uint32_t propertyCount = 0;
    if (napi_get_array_length(env, propertyNames, &propertyCount) != napi_ok) {
        errorMessage = "failed to read arkTSScript param count";
        return false;
    }

    std::string path;
    std::string functionName;
    std::map<int32_t, std::string> typedArgs;

    for (uint32_t i = 0; i < propertyCount; ++i) {
        napi_value keyValue = nullptr;
        if (napi_get_element(env, propertyNames, i, &keyValue) != napi_ok || keyValue == nullptr) {
            errorMessage = "failed to read arkTSScript param key";
            return false;
        }

        std::string key;
        if (!GetUtf8String(env, keyValue, key)) {
            errorMessage = "failed to decode arkTSScript param key";
            return false;
        }

        napi_value propertyValue = nullptr;
        if (napi_get_property(env, jsParam, keyValue, &propertyValue) != napi_ok || propertyValue == nullptr) {
            errorMessage = "failed to read arkTSScript param '" + key + "'";
            return false;
        }

        if (key == "path" || key == "functionName") {
            std::string stringValue;
            if (!GetUtf8String(env, propertyValue, stringValue) || stringValue.empty()) {
                errorMessage = "'" + key + "' must be a non-empty string";
                return false;
            }
            if (key == "path") {
                path = stringValue;
            } else {
                functionName = stringValue;
            }
            continue;
        }

        int32_t argIndex = 0;
        if (!ParseArgIndex(key, argIndex)) {
            errorMessage = "unsupported arkTSScript param '" + key + "'";
            return false;
        }
        if (typedArgs.find(argIndex) != typedArgs.end()) {
            errorMessage = "duplicate arkTSScript param '" + key + "'";
            return false;
        }

        std::string typedArg;
        if (!ConvertJsValueToTypedArg(env, key, propertyValue, typedArg, errorMessage)) {
            return false;
        }
        typedArgs.emplace(argIndex, typedArg);
    }

    if (path.empty()) {
        errorMessage = "'path' is required";
        return false;
    }
    if (functionName.empty()) {
        errorMessage = "'functionName' is required";
        return false;
    }

    int32_t expectedIndex = 1;
    for (const auto &[argIndex, typedArg] : typedArgs) {
        if (argIndex != expectedIndex) {
            errorMessage = "arkTSScript args must start at arg1 and be continuous";
            return false;
        }
        ++expectedIndex;
    }

    std::ostringstream os;
    os << "--path=\"" << EscapeCliValue(path) << "\"";
    os << " --functionName=\"" << EscapeCliValue(functionName) << "\"";
    for (const auto &[argIndex, typedArg] : typedArgs) {
        os << " --arg" << argIndex << "=\"" << EscapeCliValue(typedArg) << "\"";
    }

    cliArgs = os.str();
    return true;
}

} // namespace ArkTSScript
