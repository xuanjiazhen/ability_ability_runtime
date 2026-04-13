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

#include <cerrno>
#include <cctype>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

#include "application_info.h"
#include "bundle_mgr_helper.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ArktsScript {

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
namespace {

bool ResolveCallerApplicationInfo(std::shared_ptr<ApplicationInfo>& appInfo)
{
    appInfo = nullptr;

    auto bundleMgrHelper = OHOS::DelayedSingleton<OHOS::AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] failed to create BundleMgrHelper");
        return false;
    }

    BundleInfo bundleInfo;
    const auto ret = bundleMgrHelper->GetBundleInfoForSelf(
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) +
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT,
            "[helper] GetBundleInfoForSelf failed, ret:%{public}d",
            ret);
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT,
        "[helper] caller bundle info resolved in sandbox process, bundleName:%{public}s, appName:%{public}s, "
        "process:%{public}s, uid:%{public}d, hapModuleCount:%{public}zu",
        bundleInfo.applicationInfo.bundleName.c_str(), bundleInfo.applicationInfo.name.c_str(),
        bundleInfo.applicationInfo.process.c_str(), bundleInfo.applicationInfo.uid, bundleInfo.hapModuleInfos.size());

    appInfo = std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo);
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] failed to create caller ApplicationInfo");
        return false;
    }

    if (appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] caller applicationInfo.name is empty");
        return false;
    }
    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] caller applicationInfo.bundleName is empty");
        return false;
    }
    if (appInfo->codePath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] caller applicationInfo.codePath is empty");
        return false;
    }
    if (appInfo->process.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "[helper] caller applicationInfo.process is empty");
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT,
        "caller application info resolved, bundleName:%{public}s, name:%{public}s, process:%{public}s, "
        "uid:%{public}d, appIndex:%{public}d",
        appInfo->bundleName.c_str(), appInfo->name.c_str(), appInfo->process.c_str(), appInfo->uid,
        appInfo->appIndex);

    return true;
}

bool ValidateApplicationInfo(const std::shared_ptr<ApplicationInfo>& appInfo)
{
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo is null");
        return false;
    }
    if (appInfo->name.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.name is empty");
        return false;
    }
    if (appInfo->bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.bundleName is empty");
        return false;
    }
    if (appInfo->codePath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.codePath is empty");
        return false;
    }
    if (appInfo->process.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "applicationInfo.process is empty");
        return false;
    }
    return true;
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

std::unique_ptr<JsRuntime> CreateScriptJsRuntime(const std::shared_ptr<ContextImpl>& context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is null");
        return nullptr;
    }

    Runtime::Options options;
    options.eventRunner = EventRunner::GetMainEventRunner();
    if (options.eventRunner == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get main event runner");
        return nullptr;
    }

    std::shared_ptr<ApplicationInfo> appInfo = context->GetApplicationInfo();
    if (!ValidateApplicationInfo(appInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get valid application info from context");
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

    auto runtime = JsRuntime::Create(options);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "JsRuntime::Create failed");
        return nullptr;
    }

    return runtime;
}

std::shared_ptr<ContextImpl> CreateScriptContext()
{
    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ContextImpl");
        return nullptr;
    }

    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    if (!ResolveCallerApplicationInfo(appInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to resolve caller application info while creating context");
        return nullptr;
    }
    if (appInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create ApplicationInfo");
        return nullptr;
    }

    if (!ValidateApplicationInfo(appInfo)) {
        return nullptr;
    }

    contextImpl->SetApplicationInfo(appInfo);
    contextImpl->SetProcessName(appInfo->process);
    return contextImpl;
}

} // namespace ArktsScript
} // namespace OHOS
