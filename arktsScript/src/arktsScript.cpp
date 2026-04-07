/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "arktsScript.h"

#include <cerrno>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

#include "js_runtime_utils.h"
#include "nlohmann/json.hpp"

#include "application_info.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_interface.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "js_context_utils.h"

namespace ArkTSScript {

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace {

std::string GetParentDir(const std::string& path)
{
    if (path.empty()) {
        return ".";
    }

    const size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) {
        return ".";
    }
    if (pos == 0) {
        return path.substr(0, 1);
    }
    return path.substr(0, pos);
}

std::shared_ptr<ApplicationInfo> BuildFallbackApplicationInfo(const std::string& scriptPath)
{
    auto appInfo = std::make_shared<ApplicationInfo>();
    if (appInfo == nullptr) {
        return nullptr;
    }

    const std::string codePath = GetParentDir(scriptPath);
    appInfo->name = "arktsScript";
    appInfo->bundleName = "com.ohos.arktsscript";
    appInfo->process = "arktsScript";
    appInfo->codePath = codePath;
    appInfo->dataDir = codePath;
    appInfo->cacheDir = codePath + "/cache";
    appInfo->debug = true;
    return appInfo;
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

void PrintUsage(const char* execName, bool isErrorLog)
{
    const std::string usage = std::string("Usage: ") + execName +
        " --abcPath abcPath --functionName funName [--arg1 typedArg ...]\n"
        "  --abcPath: ABC file path (required)\n"
        "  --functionName: Function name to call (required)\n"
        "  --argN: Function argument with explicit type prefix (optional, sequential from arg1)\n"
        "  -h, --help: Show this help message\n"
        "Argument types:\n"
        "  str:value\n"
        "  num:123\n"
        "  bool:true\n"
        "  array:[1,2,3]\n"
        "  jsonString:{\"key\":\"value\"}\n"
        "Examples:\n  " + execName +
        " --abcPath /data/scripts/test.abc --functionName add --arg1 num:10 --arg2 num:20\n  " + execName +
        " --abcPath /data/scripts/test.abc --functionName configure --arg1 str:prod --arg2 num:8080 --arg3 bool:true";
    if (isErrorLog) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
        return;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
}

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

bool ParseDouble(const std::string& input, double& result)
{
    if (input.empty()) {
        return false;
    }

    errno = 0;
    char* end = nullptr;
    const double value = std::strtod(input.c_str(), &end);
    if (end == input.c_str() || end == nullptr || *end != '\0' || errno == ERANGE) {
        return false;
    }

    result = value;
    return true;
}

bool ParseArgOptionIndex(const std::string& optionName, int32_t& index)
{
    constexpr char ARG_PREFIX[] = "--arg";
    if (optionName.rfind(ARG_PREFIX, 0) != 0 || optionName.size() <= sizeof(ARG_PREFIX) - 1) {
        return false;
    }

    index = 0;
    for (size_t i = sizeof(ARG_PREFIX) - 1; i < optionName.size(); ++i) {
        const char ch = optionName[i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        index = index * 10 + (ch - '0');
    }
    return index > 0;
}

bool ParseArgAssignment(const std::string& optionName, int32_t& index, std::string& value)
{
    const size_t equalPos = optionName.find('=');
    if (equalPos == std::string::npos) {
        return false;
    }

    const std::string key = optionName.substr(0, equalPos);
    if (!ParseArgOptionIndex(key, index)) {
        return false;
    }

    value = optionName.substr(equalPos + 1);
    return true;
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

bool ValidateScriptFile(const std::string& path)
{
    if (path.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "empty script path");
        return false;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to open script file: %{private}s", path.c_str());
        return false;
    }

    if (path.size() < 4 || path.substr(path.size() - 4) != ".abc") {
        TAG_LOGW(AAFwkTag::APPKIT, "script path does not end with .abc: %{private}s", path.c_str());
    }

    return true;
}

} // namespace

std::string ArgValue::GetString() const
{
    return std::holds_alternative<std::string>(value) ? std::get<std::string>(value) : "";
}

double ArgValue::GetNumber() const
{
    return std::holds_alternative<double>(value) ? std::get<double>(value) : 0.0;
}

bool ArgValue::GetBool() const
{
    return std::holds_alternative<bool>(value) ? std::get<bool>(value) : false;
}

std::string ArgValue::ToString() const
{
    switch (type) {
        case ArgType::STRING:
            return "\"" + GetString() + "\"";
        case ArgType::NUMBER: {
            std::ostringstream os;
            os << GetNumber();
            return os.str();
        }
        case ArgType::BOOLEAN:
            return GetBool() ? "true" : "false";
        case ArgType::ARRAY:
        case ArgType::JSON_OBJECT:
            return GetString();
        case ArgType::NULL_VAL:
            return "null";
        case ArgType::UNDEFINED:
            return "undefined";
        default:
            return "unknown";
    }
}

bool ParseTypedArg(const std::string& input, ArgValue& argValue, std::string& errorMessage)
{
    if (input.empty()) {
        errorMessage = "argument value cannot be empty";
        return false;
    }

    const size_t colonPos = input.find(':');
    if (colonPos == std::string::npos || colonPos == 0) {
        errorMessage = "argument value must include an explicit type prefix";
        return false;
    }

    const std::string typePrefix = input.substr(0, colonPos);
    const std::string value = input.substr(colonPos + 1);

    if (typePrefix == "str") {
        argValue = ArgValue(value);
        return true;
    }

    if (typePrefix == "num") {
        double num = 0.0;
        if (!ParseDouble(value, num)) {
            errorMessage = "invalid number argument: " + value;
            return false;
        }
        argValue = ArgValue(num);
        return true;
    }

    if (typePrefix == "bool") {
        if (value == "true") {
            argValue = ArgValue(true);
            return true;
        }
        if (value == "false") {
            argValue = ArgValue(false);
            return true;
        }
        errorMessage = "invalid boolean argument: " + value;
        return false;
    }

    if (typePrefix == "array") {
        ArgValue arg;
        arg.type = ArgType::ARRAY;
        arg.value = value;
        argValue = arg;
        return true;
    }

    if (typePrefix == "jsonString") {
        ArgValue arg;
        arg.type = ArgType::JSON_OBJECT;
        arg.value = value;
        argValue = arg;
        return true;
    }

    errorMessage = "unsupported argument type prefix: " + typePrefix;
    return false;
}

bool ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    if (argc < 2) {
        PrintUsage(argv[0], true);
        return false;
    }

    std::map<int32_t, ArgValue> indexedArgs;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "-h" || option == "--help") {
            PrintUsage(argv[0], false);
            args.showHelp = true;
            return true;
        }
        if (!option.empty() && option[0] != '-') {
            TAG_LOGE(AAFwkTag::APPKIT, "Unknown positional argument: use --abcPath/--functionName/--argN");
            PrintUsage(argv[0], true);
            return false;
        }

        if (option.rfind("--abcPath=", 0) == 0) {
            args.path = option.substr(std::string("--abcPath=").length());
            continue;
        }
        if (option == "--abcPath") {
            if (i + 1 >= argc) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --abcPath");
                PrintUsage(argv[0], true);
                return false;
            }
            args.path = argv[++i];
            continue;
        }

        if (option.rfind("--functionName=", 0) == 0) {
            args.funName = option.substr(std::string("--functionName=").length());
            continue;
        }
        if (option == "--functionName") {
            if (i + 1 >= argc) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --functionName");
                PrintUsage(argv[0], true);
                return false;
            }
            args.funName = argv[++i];
            continue;
        }

        int32_t argIndex = 0;
        std::string inlineValue;
        if (ParseArgAssignment(option, argIndex, inlineValue)) {
            if (indexedArgs.find(argIndex) != indexedArgs.end()) {
                TAG_LOGE(AAFwkTag::APPKIT, "Duplicate argument option: --arg%{public}d", argIndex);
                return false;
            }

            ArgValue argValue;
            std::string errorMessage;
            if (!ParseTypedArg(inlineValue, argValue, errorMessage)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid value for --arg%{public}d: %{public}s",
                    argIndex, errorMessage.c_str());
                return false;
            }
            indexedArgs.emplace(argIndex, argValue);
            continue;
        }

        if (ParseArgOptionIndex(option, argIndex)) {
            if (i + 1 >= argc) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for %{public}s", option.c_str());
                PrintUsage(argv[0], true);
                return false;
            }
            if (indexedArgs.find(argIndex) != indexedArgs.end()) {
                TAG_LOGE(AAFwkTag::APPKIT, "Duplicate argument option: %{public}s", option.c_str());
                return false;
            }

            ArgValue argValue;
            std::string errorMessage;
            if (!ParseTypedArg(argv[++i], argValue, errorMessage)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid value for %{public}s: %{public}s",
                    option.c_str(), errorMessage.c_str());
                return false;
            }
            indexedArgs.emplace(argIndex, argValue);
            continue;
        }

        TAG_LOGE(AAFwkTag::APPKIT, "Unknown option: %{public}s", option.c_str());
        PrintUsage(argv[0], true);
        return false;
    }

    if (args.path.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing required option: --abcPath");
        PrintUsage(argv[0], true);
        return false;
    }

    if (args.funName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing required option: --functionName");
        PrintUsage(argv[0], true);
        return false;
    }

    int32_t expectedArgIndex = 1;
    for (const auto& [argIndex, argValue] : indexedArgs) {
        if (argIndex != expectedArgIndex) {
            TAG_LOGE(AAFwkTag::APPKIT, "Arguments must start at --arg1 and be continuous");
            return false;
        }
        args.argv.emplace_back(argValue);
        ++expectedArgIndex;
    }

    if (!ValidateScriptFile(args.path)) {
        return false;
    }

    return true;
}

void ScriptArgs::Print() const
{
    std::ostringstream argStream;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i > 0) {
            argStream << ", ";
        }
        argStream << argv[i].ToString();
    }
    TAG_LOGI(AAFwkTag::APPKIT, "  Path: %{private}s", path.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Function: %{public}s", funName.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Arguments: [%{public}s]", argStream.str().c_str());
}

std::unique_ptr<JsRuntime> CreateLiteJsRuntime(const std::string& scriptPath)
{
    Runtime::Options options;
    options.eventRunner = EventRunner::GetMainEventRunner();
    if (options.eventRunner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get main event runner");
        return nullptr;
    }

    auto bundleMgrHelper = OHOS::DelayedSingleton<OHOS::AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create BundleMgrHelper");
        return nullptr;
    }

    BundleInfo bundleInfo;
    OHOS::ErrCode ret = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
        bundleInfo);
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    if (ret == OHOS::ERR_OK) {
        if (bundleInfo.name.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "bundleInfo.name is empty");
            return nullptr;
        }
        appInfo = std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo);
    } else {
        TAG_LOGW(AAFwkTag::APPKIT, "GetBundleInfoForSelf failed, fallback to shell mode: %{public}d", ret);
        appInfo = BuildFallbackApplicationInfo(scriptPath);
    }
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo is null");
        return nullptr;
    }

    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.bundleName is empty");
        return nullptr;
    }
    if (appInfo->codePath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.codePath is empty");
        return nullptr;
    }

    options.bundleName = appInfo->bundleName;
    options.codePath = appInfo->codePath;
    options.eventRunner = EventRunner::GetMainEventRunner();
    options.loadAce = false;
    options.isDebugVersion = appInfo->debug;
    options.arkNativeFilePath = appInfo->arkNativeFilePath;
    options.uid = appInfo->uid;
    options.apiTargetVersion = appInfo->apiTargetVersion;
    options.allowArkTsLargeHeap = appInfo->allowArkTsLargeHeap;
    options.versionCode = appInfo->versionCode;
    options.arkTSMode = appInfo->arkTSMode;

    auto runtime = JsRuntime::Create(options);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "JsRuntime::Create failed");
        return nullptr;
    }

    return runtime;
}

std::shared_ptr<ContextImpl> CreateScriptContext(const std::string& scriptPath)
{
    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ContextImpl");
        return nullptr;
    }

    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::string processName;
    auto bundleMgrHelper = OHOS::DelayedSingleton<OHOS::AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null bundleMgrHelper");
        return nullptr;
    }

    BundleInfo bundleInfo;
    OHOS::ErrCode ret = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION),
        bundleInfo);
    if (ret == OHOS::ERR_OK) {
        appInfo = std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo);
        if (!bundleInfo.applicationInfo.process.empty()) {
            processName = bundleInfo.applicationInfo.process;
        }
    } else {
        TAG_LOGW(AAFwkTag::APPKIT, "GetBundleInfoForSelf failed while creating context, fallback to shell mode: "
            "%{public}d", ret);
        appInfo = BuildFallbackApplicationInfo(scriptPath);
        processName = "arktsScript";
    }
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ApplicationInfo");
        return nullptr;
    }

    if (appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.name is empty");
        return nullptr;
    }
    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.bundleName is empty");
        return nullptr;
    }
    if (processName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.process is empty");
        return nullptr;
    }

    contextImpl->SetApplicationInfo(appInfo);
    contextImpl->SetProcessName(processName);
    return contextImpl;
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

napi_value ArgValueToNapi(napi_env env, const ArgValue& arg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is null when converting argument");
        return nullptr;
    }

    napi_value result = nullptr;
    napi_status status = napi_ok;
    switch (arg.type) {
        case ArgType::STRING:
        {
            const std::string stringValue = arg.GetString();
            status = napi_create_string_utf8(env, stringValue.c_str(), stringValue.length(), &result);
            break;
        }
        case ArgType::NUMBER:
            status = napi_create_double(env, arg.GetNumber(), &result);
            break;
        case ArgType::BOOLEAN:
            status = napi_get_boolean(env, arg.GetBool(), &result);
            break;
        case ArgType::NULL_VAL:
            status = napi_get_null(env, &result);
            break;
        case ArgType::UNDEFINED:
            status = napi_get_undefined(env, &result);
            break;
        case ArgType::ARRAY:
        case ArgType::JSON_OBJECT: {
            const std::string jsonValue = arg.GetString();
            nlohmann::json jsonObject = nlohmann::json::parse(jsonValue, nullptr, false);
            if (jsonObject.is_discarded()) {
                TAG_LOGE(AAFwkTag::APPKIT, "invalid JSON argument: %{public}s", jsonValue.c_str());
                return nullptr;
            }
            if (arg.type == ArgType::ARRAY && !jsonObject.is_array()) {
                TAG_LOGE(AAFwkTag::APPKIT, "array argument must be a top-level array: %{public}s", jsonValue.c_str());
                return nullptr;
            }
            if (arg.type == ArgType::JSON_OBJECT && !jsonObject.is_object()) {
                TAG_LOGE(AAFwkTag::APPKIT, "jsonString argument must be a top-level object: %{public}s",
                    jsonValue.c_str());
                return nullptr;
            }

            result = JsonToNapiValue(env, jsonObject);
            if (result == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT,
                    "failed to convert JSON argument to napi, only object/array/string/number/boolean/null are supported: "
                    "%{public}s",
                    jsonValue.c_str());
                return nullptr;
            }
            return result;
        }
        default:
            status = napi_get_undefined(env, &result);
            break;
    }

    if (status != napi_ok || result == nullptr) {
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

} // namespace ArkTSScript
