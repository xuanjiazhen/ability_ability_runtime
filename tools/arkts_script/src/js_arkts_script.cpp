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
#include <iostream>
#include <cstring>

#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace ArktsScript {

using namespace OHOS::AbilityRuntime;

namespace {

// 回调数据结构，用于 Promise 处理
struct PromiseCallbackData {
    napi_env env;
    ResultCallback callback;
};

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

// 将 JSON 值转换为 NAPI 值
napi_value JsonToNapiValue(napi_env env, const JsonValue& jsonValue)
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

// Promise resolve 回调
napi_value PromiseResolveCallback(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    
    if (argc > 0) {
        result = argv[0];
    }
    
    auto* callbackData = static_cast<PromiseCallbackData*>(data);
    if (callbackData != nullptr && callbackData->callback) {
        std::string resultStr = NapiValueToJsonString(env, result);
        TAG_LOGI(AAFwkTag::APPKIT, "Promise resolve callback triggered");
        callbackData->callback(true, resultStr, "");
        delete callbackData;
    }
    
    return nullptr;
}

// Promise reject 回调
napi_value PromiseRejectCallback(napi_env env, napi_callback_info info)
{
    napi_value error = nullptr;
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    
    if (argc > 0) {
        error = argv[0];
    }
    
    auto* callbackData = static_cast<PromiseCallbackData*>(data);
    if (callbackData != nullptr && callbackData->callback) {
        std::string errorStr = NapiValueToJsonString(env, error);
        TAG_LOGW(AAFwkTag::APPKIT, "Promise reject callback triggered");
        callbackData->callback(false, "", errorStr);
        delete callbackData;
    }
    
    return nullptr;
}

// 处理同步结果
void HandleSyncResult(napi_env env, napi_value result, ResultCallback callback)
{
    std::string resultStr = NapiValueToJsonString(env, result);
    callback(true, resultStr, "");
}

// 处理 Promise 结果（参考 js_insight_intent_executor）
bool HandlePromiseResult(napi_env env, napi_value promise, ResultCallback callback)
{
    HandleScope handleScope(env);
    
    // 创建回调数据
    auto* callbackData = new (std::nothrow) PromiseCallbackData{env, callback};
    if (callbackData == nullptr) {
        callback(false, "", "failed to allocate callback data");
        return false;
    }
    
    // 获取 then 方法
    napi_value then = nullptr;
    napi_status status = napi_get_named_property(env, promise, "then", &then);
    if (status != napi_ok || then == nullptr) {
        delete callbackData;
        callback(false, "", "failed to get Promise.then");
        return false;
    }
    
    // 创建 resolve 回调
    napi_value resolveCb = nullptr;
    status = napi_create_function(env, "onResolve", NAPI_AUTO_LENGTH,
        PromiseResolveCallback, callbackData, &resolveCb);
    if (status != napi_ok || resolveCb == nullptr) {
        delete callbackData;
        callback(false, "", "failed to create resolve callback");
        return false;
    }
    
    // 获取 catch 方法
    napi_value promiseCatch = nullptr;
    status = napi_get_named_property(env, promise, "catch", &promiseCatch);
    if (status != napi_ok || promiseCatch == nullptr) {
        delete callbackData;
        callback(false, "", "failed to get Promise.catch");
        return false;
    }
    
    // 创建 reject 回调
    napi_value rejectCb = nullptr;
    status = napi_create_function(env, "onReject", NAPI_AUTO_LENGTH,
        PromiseRejectCallback, callbackData, &rejectCb);
    if (status != napi_ok || rejectCb == nullptr) {
        delete callbackData;
        callback(false, "", "failed to create reject callback");
        return false;
    }
    
    // 调用 then(resolveCb)
    napi_value argvThen[1] = {resolveCb};
    status = napi_call_function(env, promise, then, 1, argvThen, nullptr);
    if (status != napi_ok) {
        delete callbackData;
        callback(false, "", "failed to register Promise.then callback");
        return false;
    }
    
    // 调用 catch(rejectCb)
    napi_value argvCatch[1] = {rejectCb};
    status = napi_call_function(env, promise, promiseCatch, 1, argvCatch, nullptr);
    if (status != napi_ok) {
        delete callbackData;
        callback(false, "", "failed to register Promise.catch callback");
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Promise callbacks registered successfully");
    return true;
}

} // namespace

bool ConvertParamsToNapiArgs(napi_env env,
    const std::map<std::string, JsonValue>& params,
    std::vector<napi_value>& napiArgs, ResultCallback callback)
{
    std::vector<std::pair<std::string, JsonValue>> sortedParams(params.begin(), params.end());
    std::sort(sortedParams.begin(), sortedParams.end(),
        [](const auto& a, const auto& b) {
            auto extractNumber = [](const std::string& key) -> int {
                if (key.rfind("argv", 0) != 0) return 0;
                std::string numStr = key.substr(4);
                try { return std::stoi(numStr); } catch (...) { return 0; }
            };
            return extractNumber(a.first) < extractNumber(b.first);
        });

    napiArgs.reserve(sortedParams.size());
    for (const auto& [key, jsonValue] : sortedParams) {
        napi_value argValue = JsonToNapiValue(env, jsonValue);
        if (argValue == nullptr) {
            callback(false, "", "failed to convert argument " + key + ": " + jsonValue.dump());
            return false;
        }
        napiArgs.push_back(argValue);
    }
    return true;
}

bool CallFunctionAndHandleResult(napi_env env, napi_value thisObj,
    napi_value func, const std::string& funName,
    const std::vector<napi_value>& napiArgs, ResultCallback callback)
{
    napi_value result = nullptr;
    napi_status status = napi_call_function(env, thisObj, func, napiArgs.size(), napiArgs.data(), &result);
    if (status != napi_ok) {
        callback(false, "", "failed to call '" + funName + "': " + GetNapiErrorMessage(env));
        return false;
    }

    bool isPromise = false;
    status = napi_is_promise(env, result, &isPromise);
    if (status != napi_ok) {
        callback(false, "", "failed to check if result is Promise: " + GetNapiErrorMessage(env));
        return false;
    }

    if (isPromise) {
        TAG_LOGI(AAFwkTag::APPKIT, "Function returned a Promise, handling asynchronously");
        return HandlePromiseResult(env, result, callback);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Function returned synchronously");
    HandleSyncResult(env, result, callback);
    return true;
}

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

bool ExecuteFunction(napi_env env, const std::string& funName,
    const std::map<std::string, JsonValue>& params, ResultCallback callback)
{
    if (env == nullptr) {
        callback(false, "", "napi_env is null");
        return false;
    }

    if (callback == nullptr) {
        return false;
    }

    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        callback(false, "", "failed to get global object: " + GetNapiErrorMessage(env));
        return false;
    }

    napi_value func = nullptr;
    status = napi_get_named_property(env, globalObj, funName.c_str(), &func);
    if (status != napi_ok) {
        callback(false, "", "failed to get function '" + funName + "': " + GetNapiErrorMessage(env));
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, func, &valueType);
    if (status != napi_ok) {
        callback(false, "", "failed to inspect function '" + funName + "': " + GetNapiErrorMessage(env));
        return false;
    }

    if (valueType == napi_undefined) {
        callback(false, "", "function '" + funName + "' was not found on globalThis");
        return false;
    }

    if (valueType != napi_function) {
        callback(false, "", "'" + funName + "' exists on globalThis, but it is not a function");
        return false;
    }

    std::vector<napi_value> napiArgs;
    if (!ConvertParamsToNapiArgs(env, params, napiArgs, callback)) {
        return false;
    }

    return CallFunctionAndHandleResult(env, globalObj, func, funName, napiArgs, callback);
}

bool LoadAndExecuteModule(JsRuntime* runtime,
    const std::string& moduleName, const std::string& srcPath,
    const std::string& hapPath, const std::string& srcEntrance,
    bool isEsModule, const std::string& funcName,
    const std::map<std::string, JsonValue>& params,
    ResultCallback callback)
{
    if (runtime == nullptr) {
        callback(false, "", "runtime is null");
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT,
        "LoadAndExecuteModule: moduleName=%{public}s, srcPath=%{private}s, "
        "hapPath=%{private}s, isEsModule=%{public}d, funcName=%{public}s",
        moduleName.c_str(), srcPath.c_str(), hapPath.c_str(), isEsModule, funcName.c_str());

    auto moduleRef = runtime->LoadModule(moduleName, srcPath, hapPath, isEsModule, false, srcEntrance);
    if (moduleRef == nullptr) {
        callback(false, "", "LoadModule failed for " + srcPath);
        return false;
    }

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        callback(false, "", "napi_env is null after LoadModule");
        return false;
    }

    napi_value moduleObj = moduleRef->GetNapiValue();
    if (moduleObj == nullptr) {
        callback(false, "", "module object is null");
        return false;
    }

    return ExecuteModuleFunction(env, moduleObj, funcName, params, callback);
}

bool ExecuteModuleFunction(napi_env env, napi_value moduleObj,
    const std::string& funcName, const std::map<std::string, JsonValue>& params,
    ResultCallback callback)
{
    napi_value func = nullptr;
    napi_status status = napi_get_named_property(env, moduleObj, funcName.c_str(), &func);
    if (status != napi_ok || func == nullptr) {
        callback(false, "", "failed to get '" + funcName + "' from module: " + GetNapiErrorMessage(env));
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, func, &valueType);
    if (status != napi_ok) {
        callback(false, "", "failed to inspect '" + funcName + "': " + GetNapiErrorMessage(env));
        return false;
    }

    if (valueType == napi_undefined) {
        callback(false, "", "function '" + funcName + "' not found in module exports");
        return false;
    }

    if (valueType != napi_function) {
        callback(false, "", "'" + funcName + "' is not a function in module exports");
        return false;
    }

    std::vector<napi_value> napiArgs;
    if (!ConvertParamsToNapiArgs(env, params, napiArgs, callback)) {
        return false;
    }

    return CallFunctionAndHandleResult(env, moduleObj, func, funcName, napiArgs, callback);
}

std::string NapiValueToJsonString(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return "null";
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_typeof failed: %{public}s", GetNapiErrorMessage(env).c_str());
        return "null";
    }

    // 构建 JSON 输出
    nlohmann::json output;
    output["type"] = "unknown";
    output["value"] = nullptr;

    if (valueType == napi_undefined) {
        output["type"] = "undefined";
        output["value"] = "undefined";
        return output.dump();
    }
    
    if (valueType == napi_null) {
        output["type"] = "null";
        output["value"] = nullptr;
        return output.dump();
    }
    
    if (valueType == napi_boolean) {
        bool boolValue = false;
        if (napi_get_value_bool(env, value, &boolValue) == napi_ok) {
            output["type"] = "boolean";
            output["value"] = boolValue;
        }
        return output.dump();
    }
    
    if (valueType == napi_number) {
        double numberValue = 0.0;
        if (napi_get_value_double(env, value, &numberValue) == napi_ok) {
            output["type"] = "number";
            output["value"] = numberValue;
        }
        return output.dump();
    }
    
    if (valueType == napi_string) {
        std::string stringValue;
        if (GetUtf8String(env, value, stringValue)) {
            output["type"] = "string";
            output["value"] = stringValue;
        }
        return output.dump();
    }
    
    if (valueType == napi_object) {
        // 尝试使用 JSON.stringify
        napi_value global = nullptr;
        status = napi_get_global(env, &global);
        if (status == napi_ok && global != nullptr) {
            napi_value jsonObj = nullptr;
            status = napi_get_named_property(env, global, "JSON", &jsonObj);
            if (status == napi_ok && jsonObj != nullptr) {
                napi_value stringifyFunc = nullptr;
                status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
                if (status == napi_ok && stringifyFunc != nullptr) {
                    napi_value argv[] = { value };
                    napi_value jsonString = nullptr;
                    status = napi_call_function(env, jsonObj, stringifyFunc, 1, argv, &jsonString);
                    if (status == napi_ok && jsonString != nullptr) {
                        std::string objectString;
                        if (GetUtf8String(env, jsonString, objectString)) {
                            output["type"] = "object";
                            // 解析 JSON 字符串作为值
                            try {
                                output["value"] = nlohmann::json::parse(objectString);
                            } catch (...) {
                                output["value"] = objectString;
                            }
                            return output.dump();
                        }
                    }
                }
            }
        }
        output["type"] = "object";
        output["value"] = "[object]";
        return output.dump();
    }
    
    if (valueType == napi_function) {
        output["type"] = "function";
        output["value"] = "[function]";
        return output.dump();
    }
    
    if (valueType == napi_symbol) {
        output["type"] = "symbol";
        output["value"] = "[symbol]";
        return output.dump();
    }
    
    if (valueType == napi_bigint) {
        output["type"] = "bigint";
        output["value"] = "[bigint]";
        return output.dump();
    }

    return output.dump();
}

void OutputResult(const std::string& result)
{
    std::cout << result << std::endl;
}

void OutputError(const std::string& error, const std::string& errorType)
{
    nlohmann::json errorOutput;
    errorOutput["success"] = false;
    errorOutput["errorType"] = errorType;
    errorOutput["error"] = error;
    std::cerr << errorOutput.dump() << std::endl;
}

} // namespace ArktsScript
} // namespace OHOS
