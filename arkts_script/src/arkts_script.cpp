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
#include "nlohmann/json.hpp"

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
        " --abcPath abcPath --funcName funName [--params \"{argv1:value1,...}\"]\n"
        "  --abcPath: ABC file path (required)\n"
        "  --funcName: Function name to call (required)\n"
        "  --params: Function arguments as JSON object (optional)\n"
        "    Keys must be in format: argv1, argv2, ...\n"
        "    Values are standard JSON types (string, number, boolean, object, array, null)\n"
        "  -h, --help: Show this help message\n"
        "Examples:\n  " + execName +
        " --abcPath /data/scripts/test.abc --funcName add --params \"{\\\"argv1\\\":10,\\\"argv2\\\":20}\"\n  " +
        execName +
        " --abcPath /data/scripts/test.abc --funcName configure --params \"{\\\"argv1\\\":\\\"prod\\\",\\\"argv2\\\":8080,\\\"argv3\\\":true}\"\n  " +
        execName +
        " --abcPath /data/scripts/test.abc --funcName process --params \"{\\\"argv1\\\":{\\\"key\\\":\\\"value\\\"}}\"";
    if (isErrorLog) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
        return;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
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

bool ParseParamsFromJson(const std::string& jsonStr, std::map<std::string, JsonValue>& params, 
    std::string& errorMessage)
{
    params.clear();
    errorMessage.clear();

    if (jsonStr.empty()) {
        return true;  // 空参数是允许的
    }

    // 解析 JSON
    nlohmann::json jsonObj;
    try {
        jsonObj = nlohmann::json::parse(jsonStr);
    } catch (const nlohmann::json::exception& e) {
        errorMessage = std::string("invalid JSON format: ") + e.what();
        return false;
    }

    if (!jsonObj.is_object()) {
        errorMessage = "params must be a JSON object";
        return false;
    }

    // 遍历 JSON 对象的每个键值对
    for (auto& [key, value] : jsonObj.items()) {
        // 检查键名格式：必须以 argv 开头
        if (key.rfind("argv", 0) != 0) {
            errorMessage = "invalid parameter key '" + key + "', expected format: argvN";
            return false;
        }

        // 验证键名格式 argv1, argv2, ...
        std::string numPart = key.substr(4);  // 去掉 "argv"
        if (numPart.empty()) {
            errorMessage = "invalid parameter key '" + key + "', expected format: argvN where N is a number";
            return false;
        }
        for (char c : numPart) {
            if (!std::isdigit(c)) {
                errorMessage = "invalid parameter key '" + key + "', expected format: argvN where N is a number";
                return false;
            }
        }

        // 直接存储 JSON 值
        params[key] = value;
    }

    return true;
}

bool ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    if (argc < 2) {
        PrintUsage(argv[0], true);
        return false;
    }

    std::string paramsJson;

    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "-h" || option == "--help") {
            PrintUsage(argv[0], false);
            args.showHelp = true;
            return true;
        }
        if (!option.empty() && option[0] != '-') {
            TAG_LOGE(AAFwkTag::APPKIT, "Unknown positional argument: use --abcPath/--funcName/--params");
            PrintUsage(argv[0], true);
            return false;
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

        if (option == "--funcName") {
            if (i + 1 >= argc) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --funcName");
                PrintUsage(argv[0], true);
                return false;
            }
            args.funName = argv[++i];
            continue;
        }

        if (option == "--params") {
            if (i + 1 >= argc) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --params");
                PrintUsage(argv[0], true);
                return false;
            }
            paramsJson = argv[++i];
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
        TAG_LOGE(AAFwkTag::APPKIT, "Missing required option: --funcName");
        PrintUsage(argv[0], true);
        return false;
    }

    // 解析 params JSON
    if (!paramsJson.empty()) {
        std::string errorMessage;
        if (!ParseParamsFromJson(paramsJson, args.params, errorMessage)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to parse params: %{public}s", errorMessage.c_str());
            return false;
        }
    }

    if (!ValidateScriptFile(args.path)) {
        return false;
    }

    return true;
}

void ScriptArgs::Print() const
{
    std::ostringstream paramStream;
    for (const auto& [key, value] : params) {
        if (paramStream.tellp() > 0) {
            paramStream << ", ";
        }
        paramStream << key << "=" << value.dump();
    }
    TAG_LOGI(AAFwkTag::APPKIT, "  Path: %{private}s", path.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Function: %{public}s", funName.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Params: [%{public}s]", paramStream.str().c_str());
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
