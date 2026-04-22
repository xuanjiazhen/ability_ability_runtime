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
#include "application_info.h"
#include "context.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace ArktsScript {

using OHOS::AbilityRuntime::JsRuntime;
using namespace OHOS::AbilityRuntime;

namespace {

struct CompleteCallbackData {
    ResultCallback callback;
    std::weak_ptr<AppExecFwk::EventRunner> runner;

    ~CompleteCallbackData()
    {
        TAG_LOGD(AAFwkTag::APPKIT, "CompleteCallbackData destroyed");
    }
};

static CompleteCallbackData* g_completeCallbackData = nullptr;

struct PromiseCallbackData {
    ResultCallback callback;
    std::weak_ptr<AppExecFwk::EventRunner> runner;

    ~PromiseCallbackData()
    {
        TAG_LOGD(AAFwkTag::APPKIT, "PromiseCallbackData destroyed");
    }
};

bool GetUtf8String(napi_env env, napi_value value, std::string& output);

bool SetNamedStringOrNull(napi_env env, napi_value object, const char* name, const std::string& value)
{
    if (value.empty()) {
        napi_value nullValue = nullptr;
        napi_get_null(env, &nullValue);
        return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
    }
    napi_value jsValue = nullptr;
    napi_create_string_utf8(env, value.c_str(), value.length(), &jsValue);
    return jsValue != nullptr && napi_set_named_property(env, object, name, jsValue) == napi_ok;
}

napi_value CreateApplicationInfoObject(napi_env env, const std::shared_ptr<AppExecFwk::ApplicationInfo>& appInfo)
{
    HandleEscape handleEscape(env);

    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    if (appInfo == nullptr) {
        napi_value nullValue = nullptr;
        napi_get_null(env, &nullValue);
        napi_set_named_property(env, object, "name", nullValue);
        napi_set_named_property(env, object, "bundleName", nullValue);
        napi_set_named_property(env, object, "process", nullValue);
        return handleEscape.Escape(object);
    }

    SetNamedStringOrNull(env, object, "name", appInfo->name);
    SetNamedStringOrNull(env, object, "bundleName", appInfo->bundleName);
    SetNamedStringOrNull(env, object, "process", appInfo->process);
    return handleEscape.Escape(object);
}

napi_value CreateScriptContextObject(
    napi_env env, const std::shared_ptr<AbilityRuntime::Context>& context, const std::string& bundleNameOverride)
{
    HandleScope handleScope(env);

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
        napi_value nullValue = nullptr;
        napi_get_null(env, &nullValue);
        napi_set_named_property(env, object, "applicationInfo", nullValue);
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
    napi_value nullValue = nullptr;
    napi_get_null(env, &nullValue);
    napi_set_named_property(env, object, "area", nullValue);

    napi_value getApplicationContextFunc = nullptr;
    if (napi_create_function(env, "getApplicationContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info) -> napi_value {
            auto appContext = AbilityRuntime::Context::GetApplicationContext();
            return CreateScriptContextObject(callbackEnv, appContext, "");
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

            auto appContext = AbilityRuntime::Context::GetApplicationContext();
            if (appContext != nullptr) {
                auto bundleContext = bundleName.empty() ? appContext->CreateBundleContext(appContext->GetBundleName()) :
                    appContext->CreateBundleContext(bundleName);
                if (bundleContext != nullptr) {
                    return CreateScriptContextObject(callbackEnv, bundleContext, bundleName);
                }
            }

            return CreateScriptContextObject(callbackEnv, appContext, bundleName);
        }, nullptr, &createBundleContextFunc) == napi_ok && createBundleContextFunc != nullptr) {
        napi_set_named_property(env, object, "createBundleContext", createBundleContextFunc);
    }

    HandleEscape handleEscape(env);
    return handleEscape.Escape(object);
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
    size_t argc = 2;
    napi_value argv[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < 2) {
        TAG_LOGE(AAFwkTag::APPKIT, "completeArkTsTool requires 2 arguments");
        return nullptr;
    }

    int requestCode = 0;
    napi_get_value_int32(env, argv[0], &requestCode);

    std::string resultStr = NapiValueToJsonString(env, argv[1]);
    TAG_LOGI(AAFwkTag::APPKIT, "completeArkTsTool called, requestCode=%{public}d", requestCode);

    if (g_completeCallbackData != nullptr && g_completeCallbackData->callback) {
        g_completeCallbackData->callback(true, resultStr, "");
        auto runner = g_completeCallbackData->runner.lock();
        if (runner) {
            runner->Stop();
        }
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
    }

    return nullptr;
}

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
        auto runner = callbackData->runner.lock();
        if (runner) {
            runner->Stop();
        }
        delete callbackData;
    }

    return nullptr;
}

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
        auto runner = callbackData->runner.lock();
        if (runner) {
            runner->Stop();
        }
        delete callbackData;
    }

    return nullptr;
}

bool HandlePromiseResult(napi_env env, napi_value promise, ResultCallback callback,
    const std::weak_ptr<AppExecFwk::EventRunner>& runner)
{
    auto* callbackData = new (std::nothrow) PromiseCallbackData{callback, runner};
    if (callbackData == nullptr) {
        callback(false, "", "failed to allocate callback data");
        return false;
    }

    napi_value then = nullptr;
    napi_status status = napi_get_named_property(env, promise, "then", &then);
    if (status != napi_ok || then == nullptr) {
        delete callbackData;
        callback(false, "", "failed to get Promise.then");
        return false;
    }

    napi_value resolveCb = nullptr;
    status = napi_create_function(env, "onResolve", NAPI_AUTO_LENGTH,
        PromiseResolveCallback, callbackData, &resolveCb);
    if (status != napi_ok || resolveCb == nullptr) {
        delete callbackData;
        callback(false, "", "failed to create resolve callback");
        return false;
    }

    napi_value argvThen[1] = {resolveCb};
    status = napi_call_function(env, promise, then, 1, argvThen, nullptr);
    if (status != napi_ok) {
        delete callbackData;
        callback(false, "", "failed to register Promise.then callback");
        return false;
    }

    napi_value promiseCatch = nullptr;
    status = napi_get_named_property(env, promise, "catch", &promiseCatch);
    if (status != napi_ok || promiseCatch == nullptr) {
        delete callbackData;
        callback(false, "", "failed to get Promise.catch");
        return false;
    }

    napi_value rejectCb = nullptr;
    status = napi_create_function(env, "onReject", NAPI_AUTO_LENGTH,
        PromiseRejectCallback, callbackData, &rejectCb);
    if (status != napi_ok || rejectCb == nullptr) {
        delete callbackData;
        callback(false, "", "failed to create reject callback");
        return false;
    }

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

bool BindContextToGlobal(napi_env env, JsRuntime* runtime)
{
    if (env == nullptr || runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid env or runtime when binding context");
        return false;
    }

    HandleScope handleScope(env);

    napi_value globalObj = nullptr;
    napi_status status = napi_get_global(env, &globalObj);
    if (status != napi_ok || globalObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_get_global failed");
        return false;
    }

    auto appContext = AbilityRuntime::Context::GetApplicationContext();
    napi_value scriptContext = CreateScriptContextObject(env, appContext, "");
    if (scriptContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create scriptContext");
        return false;
    }

    napi_value getApplicationContextFunc = nullptr;
    status = napi_create_function(env, "getApplicationContext", NAPI_AUTO_LENGTH,
        [](napi_env callbackEnv, napi_callback_info) -> napi_value {
            auto appContext = AbilityRuntime::Context::GetApplicationContext();
            return CreateScriptContextObject(callbackEnv, appContext, "");
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

            auto appContext = AbilityRuntime::Context::GetApplicationContext();
            if (appContext != nullptr) {
                auto bundleContext = bundleName.empty() ? appContext->CreateBundleContext(appContext->GetBundleName()) :
                    appContext->CreateBundleContext(bundleName);
                if (bundleContext != nullptr) {
                    return CreateScriptContextObject(callbackEnv, bundleContext, bundleName);
                }
            }

            return CreateScriptContextObject(callbackEnv, appContext, bundleName);
        }, nullptr, &createBundleContextFunc);
    if (status == napi_ok && createBundleContextFunc != nullptr) {
        napi_set_named_property(env, scriptContext, "createBundleContext", createBundleContextFunc);
    }

    napi_set_named_property(env, globalObj, "scriptContext", scriptContext);
    napi_set_named_property(env, globalObj, "context", scriptContext);
    return true;
}

bool BindCompleteArkTsTool(napi_env env, ResultCallback callback,
    std::shared_ptr<AppExecFwk::EventRunner> runner)
{
    if (env == nullptr) {
        return false;
    }

    HandleScope handleScope(env);

    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    if (g_completeCallbackData != nullptr) {
        delete g_completeCallbackData;
        g_completeCallbackData = nullptr;
    }
    g_completeCallbackData = new (std::nothrow) CompleteCallbackData{callback, runner};
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

napi_value ResolveFunctionForSkill(napi_env env, SkillType skillType,
    const std::string& scriptClass, const std::string& funName)
{
    if (env == nullptr) {
        return nullptr;
    }

    napi_value global = nullptr;
    napi_get_global(env, &global);

    if (skillType == SkillType::APP_SKILL) {
        napi_value func = nullptr;
        napi_status status = napi_get_named_property(env, global, funName.c_str(), &func);
        if (status != napi_ok) return nullptr;

        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, func, &valueType);
        if (valueType != napi_function) return nullptr;

        return func;
    } else if (skillType == SkillType::STANDALONE_SKILL) {
        napi_value module = nullptr;
        napi_get_named_property(env, global, scriptClass.c_str(), &module);
        if (module == nullptr) return nullptr;

        napi_value defaultExport = nullptr;
        napi_get_named_property(env, module, "default", &defaultExport);
        if (defaultExport == nullptr) return nullptr;

        napi_value instance = nullptr;
        napi_new_instance(env, defaultExport, 0, nullptr, &instance);
        if (instance == nullptr) return nullptr;

        napi_value method = nullptr;
        napi_get_named_property(env, instance, funName.c_str(), &method);
        return method;
    }
    return nullptr;
}

napi_value ConvertArgumentsToNapi(napi_env env, const std::vector<std::string>& arguments)
{
    if (env == nullptr) {
        return nullptr;
    }

    HandleScope handleScope(env);

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

    HandleEscape handleEscape(env);
    return handleEscape.Escape(argsArray);
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
