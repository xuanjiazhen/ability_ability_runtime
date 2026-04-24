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

#include "js_arkts_script.h"

#include <algorithm>
#include <cstring>
#include <exception>

#include "application_context.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace ArktsScript {

using OHOS::AbilityRuntime::JsRuntime;
using OHOS::AbilityRuntime::CreateJsNull;
using OHOS::AbilityRuntime::CreateJsValue;

namespace {

constexpr char SCRIPT_EXTENSION[] = ".ets";
constexpr size_t SCRIPT_EXTENSION_LENGTH = sizeof(SCRIPT_EXTENSION) - 1;
constexpr size_t CREATE_BUNDLE_CONTEXT_ARGC = 1;

struct CompleteCallbackData {
    ResultCallback callback;
};

static CompleteCallbackData* g_completeCallbackData = nullptr;

bool IsFunction(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return false;
    }
    napi_valuetype valueType = napi_undefined;
    return napi_typeof(env, value, &valueType) == napi_ok && valueType == napi_function;
}

bool GetNamedPropertyHelper(napi_env env, napi_value object,
    const std::string& name, napi_value& property)
{
    property = nullptr;
    if (env == nullptr || object == nullptr || name.empty()) {
        return false;
    }
    napi_status status = napi_get_named_property(env, object, name.c_str(), &property);
    return status == napi_ok && property != nullptr;
}

std::vector<std::string> BuildModulePathCandidates(
    const std::string& abcPath, const std::string& scriptName)
{
    std::vector<std::string> candidates;
    auto addCandidate = [&candidates](const std::string& candidate) {
        if (!candidate.empty() &&
            std::find(candidates.begin(), candidates.end(), candidate) == candidates.end()) {
            candidates.emplace_back(candidate);
        }
    };
    addCandidate(abcPath);
    if (!scriptName.empty()) {
        std::string normalized = scriptName;
        if (normalized.size() > SCRIPT_EXTENSION_LENGTH &&
            normalized.substr(normalized.size() - SCRIPT_EXTENSION_LENGTH) == SCRIPT_EXTENSION) {
            normalized = normalized.substr(0, normalized.size() - SCRIPT_EXTENSION_LENGTH);
        }
        addCandidate(normalized);
        addCandidate(scriptName);
    }
    return candidates;
}

bool GetExportObject(JsRuntime* runtime, napi_env env, const std::string& modulePath,
    const std::string& exportName, napi_value& exportValue)
{
    exportValue = nullptr;
    if (runtime == nullptr || env == nullptr || modulePath.empty() || exportName.empty()) {
        return false;
    }
    auto vm = runtime->GetEcmaVm();
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ecma vm when resolving export");
        return false;
    }
    panda::Local<panda::ObjectRef> exportObj =
        panda::JSNApi::GetExportObject(vm, modulePath, exportName);
    if (exportObj->IsNull()) {
        TAG_LOGD(AAFwkTag::APPKIT, "export not found, module:%{public}s key:%{public}s",
            modulePath.c_str(), exportName.c_str());
        return false;
    }
    exportValue = ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
    return exportValue != nullptr;
}

} // namespace

std::string JsArktsScript::NormalizeScriptName(const std::string& scriptName)
{
    if (scriptName.size() > SCRIPT_EXTENSION_LENGTH &&
        scriptName.substr(scriptName.size() - SCRIPT_EXTENSION_LENGTH) == SCRIPT_EXTENSION) {
        return scriptName.substr(0, scriptName.size() - SCRIPT_EXTENSION_LENGTH);
    }
    return scriptName;
}

bool JsArktsScript::SetNamedStringOrNull(napi_env env, napi_value object, const char* name,
    const std::string& value)
{
    AbilityRuntime::HandleScope handleScope(env);
    if (value.empty()) {
        napi_value nullValue = CreateJsNull(env);
        return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
    }
    napi_value jsValue = CreateJsValue(env, value);
    return jsValue != nullptr && napi_set_named_property(env, object, name, jsValue) == napi_ok;
}

bool JsArktsScript::SetNamedNull(napi_env env, napi_value object, const char* name)
{
    AbilityRuntime::HandleScope handleScope(env);
    napi_value nullValue = CreateJsNull(env);
    return nullValue != nullptr && napi_set_named_property(env, object, name, nullValue) == napi_ok;
}

napi_value JsArktsScript::CreateApplicationInfoObject(napi_env env,
    const std::shared_ptr<AppExecFwk::ApplicationInfo>& appInfo)
{
    AbilityRuntime::HandleEscape escapeScope(env);
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    if (appInfo == nullptr) {
        SetNamedNull(env, object, "name");
        SetNamedNull(env, object, "bundleName");
        SetNamedNull(env, object, "process");
        return escapeScope.Escape(object);
    }

    SetNamedStringOrNull(env, object, "name", appInfo->name);
    SetNamedStringOrNull(env, object, "bundleName", appInfo->bundleName);
    SetNamedStringOrNull(env, object, "process", appInfo->process);

    return escapeScope.Escape(object);
}

void JsArktsScript::SetApplicationInfoProperty(napi_env env, napi_value object,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context)
{
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
}

void JsArktsScript::SetNullContextProperties(napi_env env, napi_value object)
{
    const char* properties[] = {
        "cacheDir", "tempDir", "filesDir", "databaseDir", "preferencesDir", "bundleCodeDir",
        "distributedFilesDir", "resourceDir", "cloudFileDir", "logFileDir"
    };
    for (const auto* property : properties) {
        SetNamedNull(env, object, property);
    }
}

void JsArktsScript::SetContextDirectoryProperties(napi_env env, napi_value object,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context)
{
    if (context == nullptr) {
        SetNullContextProperties(env, object);
        return;
    }

    SetNamedStringOrNull(env, object, "cacheDir", context->GetCacheDir());
    SetNamedStringOrNull(env, object, "tempDir", context->GetTempDir());
    SetNamedStringOrNull(env, object, "filesDir", context->GetFilesDir());
    SetNamedStringOrNull(env, object, "databaseDir", context->GetDatabaseDir());
    SetNamedStringOrNull(env, object, "preferencesDir", context->GetPreferencesDir());
    SetNamedStringOrNull(env, object, "bundleCodeDir", context->GetBundleCodeDir());
    SetNamedStringOrNull(env, object, "distributedFilesDir", context->GetDistributedFilesDir());
    SetNamedStringOrNull(env, object, "resourceDir", context->GetResourceDir());
    SetNamedStringOrNull(env, object, "cloudFileDir", context->GetCloudFileDir());
    SetNamedStringOrNull(env, object, "logFileDir", context->GetLogFileDir());
}

napi_value JsArktsScript::CreateApplicationContextCallback(napi_env callbackEnv, napi_callback_info)
{
    auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
    return CreateScriptContextObject(callbackEnv, appContext);
}

napi_value JsArktsScript::CreateBundleContextCallback(napi_env callbackEnv, napi_callback_info info)
{
    size_t argc = CREATE_BUNDLE_CONTEXT_ARGC;
    napi_value argv[CREATE_BUNDLE_CONTEXT_ARGC] = {nullptr};
    napi_get_cb_info(callbackEnv, info, &argc, argv, nullptr, nullptr);

    std::string bundleName;
    if (argc > 0) {
        GetUtf8String(callbackEnv, argv[0], bundleName);
    }

    auto appContext = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (appContext == nullptr) {
        return CreateScriptContextObject(callbackEnv, appContext, bundleName);
    }

    const std::string targetBundleName = bundleName.empty() ? appContext->GetBundleName() : bundleName;
    auto bundleContext = appContext->CreateBundleContext(targetBundleName);
    if (bundleContext != nullptr) {
        return CreateScriptContextObject(callbackEnv, bundleContext, targetBundleName);
    }
    return CreateScriptContextObject(callbackEnv, appContext, targetBundleName);
}

void JsArktsScript::BindContextFunction(napi_env env, napi_value object, const char* name, napi_callback callback)
{
    napi_value func = nullptr;
    if (napi_create_function(env, name, NAPI_AUTO_LENGTH, callback, nullptr, &func) == napi_ok && func != nullptr) {
        napi_set_named_property(env, object, name, func);
    }
}

napi_value JsArktsScript::CreateScriptContextObject(napi_env env,
    const std::shared_ptr<OHOS::AbilityRuntime::Context>& context,
    const std::string& bundleNameOverride)
{
    AbilityRuntime::HandleEscape escapeScope(env);
    napi_value object = nullptr;
    if (napi_create_object(env, &object) != napi_ok || object == nullptr) {
        return nullptr;
    }

    const std::string bundleName = !bundleNameOverride.empty() ? bundleNameOverride :
        (context != nullptr ? context->GetBundleName() : "");
    const std::string processName = context != nullptr ? context->GetProcessName() : "";
    SetApplicationInfoProperty(env, object, context);
    SetContextDirectoryProperties(env, object, context);
    SetNamedStringOrNull(env, object, "bundleName", bundleName);
    SetNamedStringOrNull(env, object, "processName", processName);
    napi_set_named_property(env, object, "area", CreateJsNull(env));
    BindContextFunction(env, object, "getApplicationContext", CreateApplicationContextCallback);
    BindContextFunction(env, object, "createBundleContext", CreateBundleContextCallback);
    return escapeScope.Escape(object);
}

bool JsArktsScript::GetUtf8String(napi_env env, napi_value value, std::string& output)
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

bool JsArktsScript::GetInt32Property(napi_env env, napi_value object, const char* name, int32_t& output)
{
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType != napi_number) {
        return false;
    }

    status = napi_get_value_int32(env, property, &output);
    return status == napi_ok;
}

bool JsArktsScript::GetOptionalJsonObjectProperty(napi_env env, napi_value object, const char* name, nlohmann::json& output,
    bool& hasValue)
{
    hasValue = false;
    if (env == nullptr || object == nullptr || name == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env, object, name, &property);
    if (status != napi_ok || property == nullptr) {
        return true;
    }

    napi_valuetype valueType = napi_undefined;
    status = napi_typeof(env, property, &valueType);
    if (status != napi_ok || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }

    std::string rawValue = JsArktsScript::StringifyObject(env, property);
    if (rawValue.empty()) {
        return true;
    }

    try {
        output = nlohmann::json::parse(rawValue);
        hasValue = true;
        return true;
    } catch (const std::exception&) {
        output = rawValue;
        hasValue = true;
        return true;
    }
}

void JsArktsScript::ClearCompleteCallbackData()
{
    delete g_completeCallbackData;
    g_completeCallbackData = nullptr;
}

void JsArktsScript::FinishCompleteCallback(bool success, const std::string& result, const std::string& error,
    const std::string& errorType)
{
    if (g_completeCallbackData != nullptr && g_completeCallbackData->callback) {
        g_completeCallbackData->callback(success, result, error, errorType);
    }
    ClearCompleteCallbackData();
}

napi_value JsArktsScript::CompleteArkTsToolWithError(const std::string& error)
{
    TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", error.c_str());
    FinishCompleteCallback(false, "", error, "ARGUMENT_ERROR");
    return nullptr;
}

napi_value JsArktsScript::CompleteArkTsTool(napi_env env, napi_callback_info info)
{
    AbilityRuntime::HandleScope handleScope(env);

    constexpr size_t RESULT_ARGC = 1;
    constexpr size_t RESULT_INDEX = 0;
    size_t argc = RESULT_ARGC;
    napi_value argv[RESULT_ARGC] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    if (argc < RESULT_ARGC) {
        return CompleteArkTsToolWithError("completeArkTsTool requires a result argument");
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(env, argv[RESULT_INDEX], &valueType) != napi_ok || valueType != napi_object) {
        return CompleteArkTsToolWithError("completeArkTsTool requires an object argument");
    }

    int32_t code = 0;
    if (!GetInt32Property(env, argv[RESULT_INDEX], "code", code)) {
        return CompleteArkTsToolWithError("completeArkTsTool requires a numeric code");
    }

    nlohmann::json resultJson;
    bool hasResult = false;
    if (!GetOptionalJsonObjectProperty(env, argv[RESULT_INDEX], "result", resultJson, hasResult)) {
        return CompleteArkTsToolWithError("completeArkTsTool failed to read result");
    }

    nlohmann::json payload;
    payload["code"] = code;
    if (hasResult) {
        payload["result"] = resultJson;
    }

    std::string resultStr = payload.dump();
    TAG_LOGI(AAFwkTag::APPKIT, "completeArkTsTool called");

    FinishCompleteCallback(true, resultStr, "", "");

    return nullptr;
}

bool JsArktsScript::BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid env when binding context");
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
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
    napi_set_named_property(env, globalObj, "scriptContext", scriptContext);
    napi_set_named_property(env, globalObj, "context", scriptContext);
    return true;
}

bool JsArktsScript::BindCompleteArkTsTool(napi_env env, const ResultCallback& callback)
{
    if (env == nullptr) {
        return false;
    }

    AbilityRuntime::HandleScope handleScope(env);
    napi_value global = nullptr;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok || global == nullptr) {
        return false;
    }

    if (g_completeCallbackData != nullptr) {
        ClearCompleteCallbackData();
    }
    g_completeCallbackData = new (std::nothrow) CompleteCallbackData{callback};
    if (g_completeCallbackData == nullptr) {
        return false;
    }

    napi_value func = nullptr;
    status = napi_create_function(env, "completeArkTsTool", NAPI_AUTO_LENGTH,
        CompleteArkTsTool, nullptr, &func);
    if (status != napi_ok || func == nullptr) {
        ClearCompleteCallbackData();
        return false;
    }

    napi_set_named_property(env, global, "completeArkTsTool", func);
    return true;
}

bool JsArktsScript::ResolveFunctionFromExports(JsRuntime* runtime, napi_env env,
    const std::string& abcPath, const std::string& scriptName, const std::string& funName,
    napi_value& receiver, napi_value& func)
{
    if (runtime == nullptr || env == nullptr) {
        return false;
    }
    napi_value global = nullptr;
    if (napi_get_global(env, &global) != napi_ok || global == nullptr) {
        return false;
    }

    auto candidates = BuildModulePathCandidates(abcPath, scriptName);
    func = nullptr;
    receiver = global;

    for (const auto& modulePath : candidates) {
        napi_value namedExport = nullptr;
        if (GetExportObject(runtime, env, modulePath, funName, namedExport) &&
            IsFunction(env, namedExport)) {
            func = namedExport;
            receiver = global;
            TAG_LOGI(AAFwkTag::APPKIT, "resolved named export %{public}s from %{public}s",
                funName.c_str(), modulePath.c_str());
            return true;
        }
    }

    for (const auto& modulePath : candidates) {
        napi_value defaultExport = nullptr;
        if (!GetExportObject(runtime, env, modulePath, "default", defaultExport)) {
            continue;
        }
        napi_value targetReceiver = defaultExport;
        if (IsFunction(env, defaultExport)) {
            auto status = napi_new_instance(env, defaultExport, 0, nullptr, &targetReceiver);
            if (status != napi_ok || targetReceiver == nullptr) {
                continue;
            }
        }
        napi_value method = nullptr;
        if (GetNamedPropertyHelper(env, targetReceiver, funName, method) &&
            IsFunction(env, method)) {
            receiver = targetReceiver;
            func = method;
            TAG_LOGI(AAFwkTag::APPKIT, "resolved default export method %{public}s from %{public}s",
                funName.c_str(), modulePath.c_str());
            return true;
        }
    }

    TAG_LOGE(AAFwkTag::APPKIT,
        "resolve export failed, abcPath:%{public}s scriptName:%{public}s func:%{public}s",
        abcPath.c_str(), scriptName.c_str(), funName.c_str());
    return false;
}

bool JsArktsScript::ResolveFunctionFromGlobal(napi_env env, const std::string& scriptName,
    const std::string& funName, napi_value& receiver, napi_value& func)
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
        return status == napi_ok && IsFunction(env, func);
    }

    const std::string normalizedScriptName = NormalizeScriptName(scriptName);
    napi_value scriptCtor = nullptr;
    status = napi_get_named_property(env, global, normalizedScriptName.c_str(), &scriptCtor);
    if (status != napi_ok || !IsFunction(env, scriptCtor)) {
        return false;
    }

    status = napi_new_instance(env, scriptCtor, 0, nullptr, &receiver);
    if (status != napi_ok || receiver == nullptr) {
        return false;
    }

    status = napi_get_named_property(env, receiver, funName.c_str(), &func);
    return status == napi_ok && IsFunction(env, func);
}

bool JsArktsScript::ResolveFunction(JsRuntime* runtime, napi_env env,
    const std::string& abcPath, const std::string& scriptName, const std::string& funName,
    napi_value& receiver, napi_value& func)
{
    if (ResolveFunctionFromExports(runtime, env, abcPath, scriptName, funName, receiver, func)) {
        return true;
    }
    return ResolveFunctionFromGlobal(env, scriptName, funName, receiver, func);
}

napi_value JsArktsScript::ConvertArgumentsToNapi(napi_env env, const std::vector<std::string>& arguments)
{
    if (env == nullptr) {
        return nullptr;
    }

    AbilityRuntime::HandleEscape escapeScope(env);
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

    return escapeScope.Escape(argsArray);
}

std::string JsArktsScript::StringifyObject(napi_env env, napi_value result)
{
    TAG_LOGD(AAFwkTag::APPKIT, "stringify object");
    AbilityRuntime::HandleScope handleScope(env);
    
    napi_value global;
    auto status = napi_get_global(env, &global);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get global failed %{public}d", status);
        return "";
    }

    napi_value jsonObj;
    status = napi_get_named_property(env, global, "JSON", &jsonObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get JSON object failed %{public}d", status);
        return "";
    }

    napi_value stringifyFunc;
    status = napi_get_named_property(env, jsonObj, "stringify", &stringifyFunc);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "get stringify object failed %{public}d", status);
        return "";
    }

    napi_value stringifyResult;
    constexpr size_t STRINGIFY_ARGC = 1;
    napi_value argv[STRINGIFY_ARGC] = { result };
    status = napi_call_function(env, jsonObj, stringifyFunc, STRINGIFY_ARGC, argv, &stringifyResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "call JSON.stringify failed %{public}d", status);
        return "";
    }

    std::string str;
    if (!AbilityRuntime::ConvertFromJsValue(env, stringifyResult, str)) {
        TAG_LOGW(AAFwkTag::APPKIT, "convert napi value failed");
        return "";
    }

    TAG_LOGD(AAFwkTag::APPKIT, "stringify object %{private}s", str.c_str());
    return str;
}

} // namespace ArktsScript
} // namespace OHOS
