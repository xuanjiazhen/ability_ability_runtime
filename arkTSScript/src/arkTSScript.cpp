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

#include "arkTSScript.h"

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <regex>
#include <sstream>

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

    const size_t pos = path.find_last_of("/\\");
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
    appInfo->name = "arkTSScript";
    appInfo->bundleName = "com.ohos.arktsscript";
    appInfo->process = "arkTSScript";
    appInfo->codePath = codePath;
    appInfo->dataDir = codePath;
    appInfo->cacheDir = codePath + "/cache";
    appInfo->debug = true;
    return appInfo;
}

void PrintUsage(const char* execName, std::ostream& os)
{
    os << "Usage: " << execName << " <path> -f funName [-a arg...]" << std::endl;
    os << "  path: ABC file path (required)" << std::endl;
    os << "  -f, --function: Function name to call (required)" << std::endl;
    os << "  -a, --arg: Function argument (optional, can be multiple, supports typed values)" << std::endl;
    os << "  -h, --help: Show this help message" << std::endl;
    os << std::endl;
    os << "Argument types (auto-detected or explicit):" << std::endl;
    os << "  Auto-detected: 123 (number), true/false (boolean), null, undefined" << std::endl;
    os << "  Explicit type: str:value, num:123, bool:true, null:null, json:{...}" << std::endl;
    os << std::endl;
    os << "Examples:" << std::endl;
    os << "  " << execName << " script.abc -f add -a 10 -a 20" << std::endl;
    os << "  " << execName << " script.abc -f greet -a str:Alice -a num:25 -a bool:true" << std::endl;
    os << "  " << execName << " script.abc -f calc -a 3.14 -a bool:false -a null" << std::endl;
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
        std::cerr << "Script path cannot be empty" << std::endl;
        return false;
    }

    std::ifstream input(path, std::ios::binary);
    if (!input.is_open()) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to open script file: %{private}s", path.c_str());
        std::cerr << "Failed to open script file: " << path << std::endl;
        return false;
    }

    if (path.size() < 4 || path.substr(path.size() - 4) != ".abc") {
        TAG_LOGW(AAFwkTag::APPKIT, "script path does not end with .abc: %{private}s", path.c_str());
        std::cerr << "Warning: script path does not end with .abc: " << path << std::endl;
    }

    return true;
}

std::string CallStatusToString(CallStatus status)
{
    switch (status) {
        case CallStatus::SUCCESS:
            return "success";
        case CallStatus::INVALID_ENV:
            return "invalid env";
        case CallStatus::ARGUMENT_CONVERSION_FAILED:
            return "argument conversion failed";
        case CallStatus::FUNCTION_NOT_FOUND:
            return "function not found";
        case CallStatus::PROPERTY_ACCESS_FAILED:
            return "property access failed";
        case CallStatus::NOT_A_FUNCTION:
            return "not a function";
        case CallStatus::CALL_FAILED:
            return "call failed";
        default:
            return "unknown";
    }
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
        case ArgType::NULL_VAL:
            return "null";
        case ArgType::UNDEFINED:
            return "undefined";
        case ArgType::OBJECT:
            return GetString();
        default:
            return "unknown";
    }
}

ArgValue ParseTypedArg(const std::string& input)
{
    if (input.empty()) {
        return ArgValue(std::string(""));
    }

    size_t colonPos = input.find(':');
    if (colonPos != std::string::npos && colonPos > 0) {
        std::string typePrefix = input.substr(0, colonPos);
        std::string value = input.substr(colonPos + 1);

        if (typePrefix == "str" || typePrefix == "string") {
            return ArgValue(value);
        }

        if (typePrefix == "num" || typePrefix == "number") {
            double num = 0.0;
            if (ParseDouble(value, num)) {
                return ArgValue(num);
            }
            return ArgValue(value);
        }

        if (typePrefix == "bool" || typePrefix == "boolean") {
            if (value == "true" || value == "1" || value == "yes") {
                return ArgValue(true);
            }
            if (value == "false" || value == "0" || value == "no") {
                return ArgValue(false);
            }
            return ArgValue(value);
        }

        if (typePrefix == "null") {
            return ArgValue(nullptr);
        }

        if (typePrefix == "undefined" || typePrefix == "undef") {
            return ArgValue();
        }

        if (typePrefix == "json" || typePrefix == "object") {
            ArgValue arg;
            arg.type = ArgType::OBJECT;
            arg.value = value;
            return arg;
        }
    }

    if (input == "true") {
        return ArgValue(true);
    }
    if (input == "false") {
        return ArgValue(false);
    }
    if (input == "null") {
        return ArgValue(nullptr);
    }
    if (input == "undefined") {
        return ArgValue();
    }

    static const std::regex numberRegex(R"(^-?\d+(\.\d+)?$)");
    if (std::regex_match(input, numberRegex)) {
        double num = 0.0;
        if (ParseDouble(input, num)) {
            return ArgValue(num);
        }
    }

    return ArgValue(input);
}

bool ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    if (argc < 2) {
        PrintUsage(argv[0], std::cerr);
        return false;
    }

    std::string firstArg = argv[1];
    if (firstArg == "-h" || firstArg == "--help") {
        PrintUsage(argv[0], std::cout);
        args.showHelp = true;
        return true;
    }

    if (!firstArg.empty() && firstArg[0] == '-') {
        std::cerr << "Missing required path argument" << std::endl;
        PrintUsage(argv[0], std::cerr);
        return false;
    }

    args.path = firstArg;

    int opt = 0;
    int optionIndex = 0;
    static struct option longOptions[] = {
        {"function", required_argument, nullptr, 'f'},
        {"arg", required_argument, nullptr, 'a'},
        {"help", no_argument, nullptr, 'h'},
        {0, 0, 0, 0}
    };

    optind = 2;
    while ((opt = getopt_long(argc, argv, "f:a:h", longOptions, &optionIndex)) != -1) {
        switch (opt) {
            case 'f':
                args.funName = optarg;
                break;
            case 'a':
                args.argv.emplace_back(ParseTypedArg(optarg));
                break;
            case 'h':
                PrintUsage(argv[0], std::cout);
                args.showHelp = true;
                return true;
            case '?':
                std::cerr << "Unknown option or missing argument" << std::endl;
                PrintUsage(argv[0], std::cerr);
                return false;
            default:
                break;
        }
    }

    if (optind < argc) {
        std::cerr << "Unexpected extra argument: " << argv[optind] << std::endl;
        PrintUsage(argv[0], std::cerr);
        return false;
    }

    if (args.funName.empty()) {
        std::cerr << "Missing required option: -f, --function" << std::endl;
        PrintUsage(argv[0], std::cerr);
        return false;
    }

    if (!ValidateScriptFile(args.path)) {
        return false;
    }

    return true;
}

void ScriptArgs::Print() const
{
    std::cout << "  Path: " << path << std::endl;
    std::cout << "  Function: " << funName << std::endl;
    std::cout << "  Arguments: [";
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i > 0) {
            std::cout << ", ";
        }
        std::cout << argv[i].ToString();
    }
    std::cout << "]" << std::endl;
}

std::unique_ptr<JsRuntime> CreateLiteJsRuntime(const std::string& scriptPath)
{
    Runtime::Options options;
    options.eventRunner = EventRunner::GetMainEventRunner();
    if (options.eventRunner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get main event runner");
        std::cerr << "Failed to get main event runner" << std::endl;
        return nullptr;
    }

    auto bundleMgrHelper = OHOS::DelayedSingleton<OHOS::AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create BundleMgrHelper");
        std::cerr << "Failed to create BundleMgrHelper" << std::endl;
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
            std::cerr << "Failed to create JsRuntime: bundle name is empty" << std::endl;
            return nullptr;
        }
        appInfo = std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo);
    } else {
        TAG_LOGW(AAFwkTag::APPKIT, "GetBundleInfoForSelf failed, fallback to shell mode: %{public}d", ret);
        std::cerr << "Warning: failed to get bundle info for self, fallback to shell mode, ret: "
                  << ret << std::endl;
        appInfo = BuildFallbackApplicationInfo(scriptPath);
    }
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo is null");
        std::cerr << "Failed to create JsRuntime: applicationInfo is null" << std::endl;
        return nullptr;
    }

    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.bundleName is empty");
        std::cerr << "Failed to create JsRuntime: applicationInfo.bundleName is empty" << std::endl;
        return nullptr;
    }
    if (appInfo->codePath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.codePath is empty");
        std::cerr << "Failed to create JsRuntime: applicationInfo.codePath is empty" << std::endl;
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
        std::cerr << "Failed to create JsRuntime" << std::endl;
        return nullptr;
    }

    return runtime;
}

std::shared_ptr<ContextImpl> CreateScriptContext(const std::string& scriptPath)
{
    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ContextImpl");
        std::cerr << "Failed to create ContextImpl" << std::endl;
        return nullptr;
    }

    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    std::string processName;
    auto bundleMgrHelper = OHOS::DelayedSingleton<OHOS::AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null bundleMgrHelper");
        std::cerr << "Failed to get BundleMgrHelper instance" << std::endl;
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
        std::cerr << "Warning: failed to get application info for self, fallback to shell mode, ret: "
                  << ret << std::endl;
        appInfo = BuildFallbackApplicationInfo(scriptPath);
        processName = "arkTSScript";
    }
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ApplicationInfo");
        std::cerr << "Failed to create ApplicationInfo" << std::endl;
        return nullptr;
    }

    if (appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.name is empty");
        std::cerr << "Failed to create ScriptContext: applicationInfo.name is empty" << std::endl;
        return nullptr;
    }
    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.bundleName is empty");
        std::cerr << "Failed to create ScriptContext: applicationInfo.bundleName is empty" << std::endl;
        return nullptr;
    }
    if (processName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.process is empty");
        std::cerr << "Failed to create ScriptContext: applicationInfo.process is empty" << std::endl;
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
        case ArgType::OBJECT: {
            const std::string jsonValue = arg.GetString();
            const std::string evalSource = "(" + jsonValue + ")";
            napi_value script = nullptr;
            status = napi_create_string_utf8(env, evalSource.c_str(), evalSource.length(), &script);
            if (status != napi_ok || script == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "failed to create JSON eval source: %{public}s",
                    GetNapiErrorMessage(env).c_str());
                return nullptr;
            }

            status = napi_run_script(env, script, &result);
            if (status != napi_ok || result == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "failed to evaluate JSON object: %{public}s",
                    GetNapiErrorMessage(env).c_str());
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

int main(int argc, char* argv[])
{
    using namespace ArkTSScript;

    std::cout << "=== ArkTSScript Runner ===" << std::endl;

    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        return 1;
    }
    if (args.showHelp) {
        return 0;
    }

    std::cout << "\nParsed arguments:" << std::endl;
    args.Print();
    std::cout << std::endl;

    std::cout << "Creating lite JsRuntime..." << std::endl;
    auto runtime = CreateLiteJsRuntime(args.path);
    if (runtime == nullptr) {
        return 1;
    }
    std::cout << "JsRuntime created successfully" << std::endl;

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get napi_env");
        std::cerr << "Failed to get napi_env" << std::endl;
        return 1;
    }

    std::cout << "Creating script context..." << std::endl;
    auto context = CreateScriptContext(args.path);
    if (context == nullptr) {
        return 1;
    }
    std::cout << "Context created successfully" << std::endl;

    std::cout << "Binding context to globalThis..." << std::endl;
    if (!BindContextToGlobal(env, context)) {
        std::cerr << "Failed to bind scriptContext to globalThis" << std::endl;
        return 1;
    }
    std::cout << "Context bound successfully" << std::endl;

    std::cout << "Loading ABC file: " << args.path << std::endl;
    if (!LoadAbcFile(runtime.get(), args.path)) {
        std::cerr << "Failed to load ABC file: " << args.path << std::endl;
        return 1;
    }
    std::cout << "ABC file loaded successfully" << std::endl;

    std::cout << "Calling function: " << args.funName << "..." << std::endl;
    CallResult callResult = CallFunction(env, args.funName, args.argv);
    if (!callResult.IsSuccess()) {
        TAG_LOGE(AAFwkTag::APPKIT, "CallFunction failed, status=%{public}s, message=%{public}s",
            CallStatusToString(callResult.status).c_str(), callResult.errorMessage.c_str());
        std::cerr << "Failed to call function '" << args.funName << "': "
                  << callResult.errorMessage << std::endl;
        return 1;
    }

    std::cout << "Function returned: " << NapiValueToString(env, callResult.value) << std::endl;
    std::cout << "\n=== Execution completed ===" << std::endl;
    return 0;
}
