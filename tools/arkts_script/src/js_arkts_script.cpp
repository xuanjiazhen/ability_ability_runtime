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

#include <cstring>

#include "application_context.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace ArktsScript {

using OHOS::AbilityRuntime::JsRuntime;
using OHOS::AbilityRuntime::CreateJsNull;
using OHOS::AbilityRuntime::CreateJsValue;

namespace {

struct CompleteCallbackData {
    ResultCallback callback;

    ~CompleteCallbackData()
    {
        TAG_LOGD(AAFwkTag::APPKIT, "CompleteCallbackData destroyed");
    }
};

static CompleteCallbackData* g_completeCallbackData = nullptr;

bool GetUtf8String(napi_env env, napi_value value, std::string& output);

std::string NormalizeScriptName(const std::string& scriptName)
{
    if (scriptName.size() > 4 && scriptName.substr(scriptName.size() - 4) == ".ets") {
        return scriptName.substr(0, scriptName.size() - 4);
    }
    return scriptName;
}

bool SetNamedStringOrNull(napi_env env, napi_value object, const char* name, const std::string& value)
{
    if (value.empty()) {
        napi_value nullValue = CreateJsNull(env);
        return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
    }
    napi_value jsValue = CreateJsValue(env, value);
    return jsValue != nullptr && napi_set_named_property(env, object, name, jsValue) == napi_ok;
}

bool SetNamedNull(napi_env env, napi_value object, const char* name)
{
    napi_value nullValue = CreateJsNull(env);
    return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
}

napi_value CreateApplicationInfoObject(napi_env env, const std::shared_ptr<AppExecFwk::ApplicationInfo>& appInfo)
{
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    if (appInfo == nullptr) {
        SetNamedNull(env, object, "name");
        SetNamedNull(env, object, "bundleName");
        SetNamedNull(env, object, "process");
        return object;
    }

    SetNamedStringOrNull(env, object, "name", appInfo->name);
    SetNamedStringOrNull(env, object, "bundleName", appInfo->bundleName);
    SetNamedStringOrNull(env, object, "process", appInfo->process);
    return object;
}

napi_value CreateScriptContextObject(napi_env env,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    const std::string& bundleNameOverride = std::string())
{
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    std::shared_ptr<AppExecFwk::ApplicationInfo> appInfo = nullptr;
    if (context != nullptr) {
        appInfo = context->GetApplicationInfo();
    }
    napi_value appInfoObject = CreateApplicationInfoObject(env, appInfo);
    if (appInfoObject != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", appInfoObject);
    } else {
        SetNamedNull(env, object, "applicationInfo");
    }

    const std::string bundleName = !bundleNameOverride.empty() ? bundleNameOverride :
        (context != nullptr ? context->GetBundleName() : "");
    const std::string processName = context != nullptr ? context->GetProcessName() : "";

    SetNamedStringOrNull(env, object, "cacheDir", context != nullptr ? context->GetCacheDir() : "");
    SetNamedStringOrNull(env, object, "tempDir", context != nullptr ? context->GetTempDir() : "");
    SetNamedStringOrNull(env, object, "filesDir", context != nullptr ? context->GetFilesDir() : "");
    SetNamedStringOrNull(env, object, "databaseDir", context != nullptr ? context->GetDatabaseDir() : "");
    SetNamedStringOrNull(env, object, "preferencesDir", context != nullptr ? context->GetPreferencesDir() : "");
    SetNamedStringOrNull(env, object, "bundleCodeDir", context != nullptr ? context->GetBundleCodeDir() : "");
    SetNamedStringOrNull(env, object, "distributedFilesDir",
        context != nullptr ? context->GetDistributedFilesDir() : "");
    SetNamedStringOrNull(env, object, "resourceDir", context != nullptr ? context->GetResourceDir() : "");
    SetNamedStringOrNull(env, object, "cloudFileDir", context != nullptr ? context->GetCloudFileDir() : "");
    SetNamedStringOrNull(env, object, "logFileDir", context != nullptr ? context->GetLogFileDir() : "");
    SetNamedStringOrNull(env, object, "bundleName", bundleName);
    SetNamedStringOrNull(env, object, "processName", processName);
    napi_set_named_property(env, object, "area", CreateJsNull(env));

    napi_value getApplicationContextFunc = nullptr;
    if (napi_create_function(env, "getApplicationContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info) -> napi_value {
            auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
            return CreateScriptContextObject(callbackEnv,
                std::static_pointer_cast<OHOS::AbilityRuntime::Context>(appContext));
        }, nullptr, &getApplicationContextFunc) == napi_ok && getApplicationContextFunc != nullptr) {
        napi_set_named_property(env, object, "getApplicationContext", getApplicationContextFunc);
    }

    napi_value createBundleContextFunc = nullptr;
    if (napi_create_function(env, "createBundleContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info info) -> napi_value {
            size_t argc = 1;
            napi_value argv[1] = {nullptr};
            napi_get_cb_info(callbackEnv, info, &argc, argv, nullptr, nullptr);

            std::string bundleName;
            if (argc > 0) {
                GetUtf8String(callbackEnv, argv[0], bundleName);
            }

            auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
            if (appContext != nullptr) {
                auto bundleContext = bundleName.empty() ? appContext->CreateBundleContext(appContext->GetBundleName()) :
                    appContext->CreateBundleContext(bundleName);
                if (bundleContext != nullptr) {
                    return CreateScriptContextObject(callbackEnv, bundleContext, bundleName);
                }
            }

            return CreateScriptContextObject(callbackEnv,
                std::static_pointer_cast<OHOS::AbilityRuntime::Context>(appContext), bundleName);
        }, nullptr, &createBundleContextFunc) == napi_ok && createBundleContextFunc != nullptr) {
        napi_set_named_property(env, object, "createBundleContext", createBundleContextFunc);
    }

    return object;
}

bool GetUtf8String(napi_env env, napi_value value, std::string& output)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }

    size_t length = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(length) failed");
        return false;
    }

    std::string buffer(length + 1, '\0');
    status = napi_get_value_string_utf8(env, value, buffer.data(), buffer.size(), &length);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_value_string_utf8(data) failed");
        return false;
    }

    buffer.resize(length);
    output = buffer;
    return true;
}

napi_value CompleteArkTsTool(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 1) {
        TAG_LOGE(AAFwkTag::APPKIT, "completeArkTsTool requires a result argument");
        if (g_completeCallbackData != nullptr && g_completeCallbackData->callback) {
            g_completeCallbackData->callback(false, "", "completeArkTsTool requires a result argument",
                "ARGUMENT_ERROR");
            delete g_completeCallbackData;
            g_completeCallbackData = nullptr;
        }
        return nullptr;
    }

    std::string resultStr = NapiValueToJsonString(env, argv[0]);
    TAG_LOGI(AAFwkTag::APPKIT, "completeArkTsTool called");

    if (g_completeCallbackData != nullptr && g_completeCallbackData->callback) {
        g_completeCallbackData->callback(true, resultStr, "", "");
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
    }

    return nullptr;
}

} // namespace

bool BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid env when binding context");
        return false;
    }

    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_global failed");
        return false;
    }

    napi_value scriptContext = CreateScriptContextObject(env, context);
    if (scriptContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create scriptContext");
        return false;
    }

    napi_value getApplicationContextFunc = nullptr;
    status = napi_create_function(env, "getApplicationContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info) -> napi_value {
            auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
            return CreateScriptContextObject(callbackEnv,
                std::static_pointer_cast<OHOS::AbilityRuntime::Context>(appContext));
        }, nullptr, &getApplicationContextFunc);
    if (status == napi_ok && getApplicationContextFunc != nullptr) {
        napi_set_named_property(env, scriptContext, "getApplicationContext", getApplicationContextFunc);
    }

    napi_value createBundleContextFunc = nullptr;
    status = napi_create_function(env, "createBundleContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info info) -> napi_value {
            size_t argc = 1;
            napi_value argv[1] = {nullptr};
            napi_get_cb_info(callbackEnv, info, &argc, argv, nullptr, nullptr);

            std::string bundleName;
            if (argc > 0) {
                GetUtf8String(callbackEnv, argv[0], bundleName);
            }

            auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
            if (appContext != nullptr) {
                auto bundleContext = bundleName.empty() ? appContext->CreateBundleContext(appContext->GetBundleName()) :
                    appContext->CreateBundleContext(bundleName);
                if (bundleContext != nullptr) {
                    return CreateScriptContextObject(callbackEnv, bundleContext, bundleName);
                }
            }

            return CreateScriptContextObject(callbackEnv,
                std::static_pointer_cast<OHOS::AbilityRuntime::Context>(appContext), bundleName);
        }, nullptr, &createBundleContextFunc);
    if (status == napi_ok && createBundleContextFunc != nullptr) {
        napi_set_named_property(env, scriptContext, "createBundleContext", createBundleContextFunc);
    }

    napi_set_named_property(env, globalObj, "scriptContext", scriptContext);
    napi_set_named_property(env, globalObj, "context", scriptContext);
    return true;
}

bool BindCompleteArkTsTool(napi_env env, const ResultCallback& callback)
{
    if (env == nullptr) {
        return false;
    }

    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    if (g_completeCallbackData != nullptr) {
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
    }
    g_completeCallbackData = new (std::nothrow) CompleteCallbackData{callback};
    if (g_completeCallbackData == nullptr) {
        return false;
    }

    napi_value func = nullptr;
    status = napi_create_function(env, "completeArkTsTool", NAPI_AUTO_LENGTH,
        CompleteArkTsTool, nullptr, &func);
    if (status != napi_ok || func == nullptr) {
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
        return false;
    }

    status = napi_set_named_property(env, global, "completeArkTsTool", func);
    if (status != napi_ok) {
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
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
        TAG_LOGE(AAFwkTag::APPKIT, "RunScript failed for %{private}s", path.c_str());
    }
    return loaded;
}

bool ResolveFunction(napi_env env, const std::string& scriptName, const std::string& funName,
    napi_value& receiver, napi_value& func)
{
    if (env == nullptr) {
        return false;
    }

    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    receiver = global;
    func = nullptr;
    if (scriptName.empty()) {
        status = napi_get_named_property(env, global, funName.c_str(), &func);
        if (status != napi_ok || func == nullptr) {
            return false;
        }
    } else {
        const std::string normalizedScriptName = NormalizeScriptName(scriptName);
        napi_value scriptCtor = nullptr;
        status = napi_get_named_property(env, global, normalizedScriptName.c_str(), &scriptCtor);
        if (status != napi_ok || scriptCtor == nullptr) {
            return false;
        }

        napi_valuetype ctorType = napi_undefined;
        napi_typeof(env, scriptCtor, &ctorType);
        if (ctorType != napi_function) {
            return false;
        }

        status = napi_new_instance(env, scriptCtor, 0, nullptr, &receiver);
        if (status != napi_ok || receiver == nullptr) {
            return false;
        }

        status = napi_get_named_property(env, receiver, funName.c_str(), &func);
        if (status != napi_ok || func == nullptr) {
            return false;
        }
    }

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, func, &valueType);
    if (valueType != napi_function) {
        return false;
    }

    return true;
}

napi_value ConvertArgumentsToNapi(napi_env env, const std::vector<std::string>& arguments)
{
    if (env == nullptr) {
        return nullptr;
    }

    napi_value argsArray = nullptr;
    napi_status status = napi_create_array_with_length(env, arguments.size(), &argsArray);
    if (status != napi_ok || argsArray == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < arguments.size(); i++) {
        napi_value strVal = nullptr;
        status = napi_create_string_utf8(env, arguments[i].c_str(),
            arguments[i].size(), &strVal);
        if (status != napi_ok || strVal == nullptr) {
            return nullptr;
        }
        status = napi_set_element(env, argsArray, i, strVal);
        if (status != napi_ok) {
            return nullptr;
        }
    }

    return argsArray;
}

std::string NapiValueToJsonString(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return "null";
    }

    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, value, &valueType);
    if (status != napi_ok) {
        return "null";
    }

    if (valueType == napi_null || valueType == napi_undefined) {
        return (valueType == napi_null) ? "null" : "undefined";
    }

    if (valueType == napi_boolean) {
        bool boolValue = false;
        napi_get_value_bool(env, value, &boolValue);
        return boolValue ? "true" : "false";
    }

    if (valueType == napi_number) {
        double numberValue = 0.0;
        napi_get_value_double(env, value, &numberValue);
        nlohmann::json numberJson = numberValue;
        return numberJson.dump();
    }

    if (valueType == napi_string) {
        std::string stringValue;
        if (GetUtf8String(env, value, stringValue)) {
            return stringValue;
        }
        return "";
    }

    if (valueType == napi_object) {
        napi_value global = nullptr;
        status = napi_get_global(env, &global);
        if (status == napi_ok && global != nullptr) {
            napi_value jsonObj = nullptr;
            status = napi_get_named_property(env, global, "JSON", &jsonObj);
            if (status == napi_ok && jsonObj != nullptr) {
                napi_value stringifyFunc = nullptr;
                status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
                if (status == napi_ok && stringifyFunc != nullptr) {
                    napi_value argv[] = {value};
                    napi_value jsonString = nullptr;
                    status = napi_call_function(env, jsonObj, stringifyFunc, 1, argv, &jsonString);
                    if (status == napi_ok && jsonString != nullptr) {
                        std::string objectString;
                        if (GetUtf8String(env, jsonString, objectString)) {
                            return objectString;
                        }
                    }
                }
            }
        }
        return "[object]";
    }

    if (valueType == napi_function) {
        return "[function]";
    }

    return "";
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
