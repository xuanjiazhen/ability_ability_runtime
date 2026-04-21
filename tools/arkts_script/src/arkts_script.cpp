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
    const std::string usage = std::string("Usage:\n  ") + execName +
        " --abcPath <path> --funcName <name> [--params '{...}']\n  " + execName +
        " --bundleName <name> --skillName <name> [--params '{...}']\n  " + execName +
        " --bundleName <name> --moduleName <name> --funcName <name> [--params '{...}']\n  " + execName +
        " --bundleName <name> --skillName <name> --hdc [--params '{...}']\n"
        "Options:\n"
        "  --abcPath: ABC file path (direct mode, mutually exclusive with --bundleName)\n"
        "  --bundleName: Target bundle name (skill/module mode)\n"
        "  --skillName: Skill name to execute (skill mode)\n"
        "  --moduleName: Module name within the bundle (module mode)\n"
        "  --funcName: Function name to call (required in direct/module mode)\n"
        "  --params: Function arguments as JSON object (optional)\n"
        "    Keys must be in format: argv1, argv2, ...\n"
        "    Values are standard JSON types (string, number, boolean, object, array, null)\n"
        "  --hdc: Use HDC IPC mode to execute skill in target app process\n"
        "  -h, --help: Show this help message\n"
        "Examples:\n  " + execName +
        " --abcPath /data/scripts/test.abc --funcName add "
        "--params '{\"argv1\":10,\"argv2\":20}'\n  " + execName +
        " --bundleName com.example.myskill --skillName mySkill "
        "--params '{\"argv1\":\"test\"}'\n  " + execName +
        " --bundleName com.example.myskill --moduleName entry --funcName execute\n  " + execName +
        " --bundleName com.example.myskill --skillName weather --hdc --params '{\"argv1\":\"test\"}'";
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

bool StripQuotesAndParseParams(std::string& paramsJson, ScriptArgs& args)
{
    if (!paramsJson.empty()) {
        size_t start = 0;
        size_t end = paramsJson.length();
        while (start < end && (paramsJson[start] == '\'' || paramsJson[start] == '"')) {
            start++;
        }
        while (end > start && (paramsJson[end - 1] == '\'' || paramsJson[end - 1] == '"')) {
            end--;
        }
        if (start > 0 || end < paramsJson.length()) {
            paramsJson = paramsJson.substr(start, end - start);
        }
    }

    if (!paramsJson.empty()) {
        std::string errorMessage;
        if (!ParseParamsFromJson(paramsJson, args.params, errorMessage)) {
            TAG_LOGE(AAFwkTag::APPKIT, "Failed to parse params: %{public}s", errorMessage.c_str());
            return false;
        }
    }
    return true;
}

bool ParseOneOption(const std::string& option, int& i, int argc, char* argv[],
    ScriptArgs& args, std::string& paramsJson, bool& collectingParams)
{
    if (option == "--abcPath") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --abcPath");
            return false;
        }
        args.path = argv[++i];
        return true;
    }

    if (option == "--funcName") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --funcName");
            return false;
        }
        args.funName = argv[++i];
        return true;
    }

    if (option == "--bundleName") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --bundleName");
            return false;
        }
        args.bundleName = argv[++i];
        return true;
    }

    if (option == "--skillName") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --skillName");
            return false;
        }
        args.skillName = argv[++i];
        return true;
    }

    if (option == "--moduleName") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --moduleName");
            return false;
        }
        args.moduleName = argv[++i];
        return true;
    }

    if (option == "--params") {
        if (i + 1 >= argc) {
            TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --params");
            return false;
        }
        collectingParams = true;
        return true;
    }

    if (option == "--hdc") {
        args.hdcMode = true;
        return true;
    }

    TAG_LOGE(AAFwkTag::APPKIT, "Unknown option: %{public}s", option.c_str());
    return false;
}

bool DetermineExecutionMode(ScriptArgs& args)
{
    if (!args.path.empty() && !args.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "--abcPath and --bundleName are mutually exclusive");
        return false;
    }

    if (args.hdcMode && !args.bundleName.empty()) {
        if (args.skillName.empty()) {
            TAG_LOGE(AAFwkTag::APPKIT, "--hdc mode requires --skillName");
            return false;
        }
        args.mode = ExecutionMode::HDC_SKILL;
        return true;
    }

    if (!args.bundleName.empty()) {
        if (!args.skillName.empty()) {
            args.mode = ExecutionMode::SKILL_BUNDLE;
        } else if (!args.moduleName.empty()) {
            args.mode = ExecutionMode::MODULE_FUNC;
        } else {
            TAG_LOGE(AAFwkTag::APPKIT,
                "--bundleName requires either --skillName or --moduleName");
            return false;
        }
    } else {
        args.mode = ExecutionMode::DIRECT_ABC;
    }

    if (args.mode == ExecutionMode::DIRECT_ABC && args.path.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing required option: --abcPath");
        return false;
    }

    if (args.mode != ExecutionMode::SKILL_BUNDLE && args.funName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing required option: --funcName");
        return false;
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

    // 解析 JSON (使用 nullptr, false 避免抛出异常)
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        errorMessage = "invalid JSON format";
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
    bool collectingParams = false;

    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];

        if (collectingParams) {
            if (!paramsJson.empty()) {
                paramsJson += " ";
            }
            paramsJson += option;
            continue;
        }

        if (option == "-h" || option == "--help") {
            PrintUsage(argv[0], false);
            args.showHelp = true;
            return true;
        }

        if (!ParseOneOption(option, i, argc, argv, args, paramsJson, collectingParams)) {
            PrintUsage(argv[0], true);
            return false;
        }
    }

    if (!DetermineExecutionMode(args)) {
        PrintUsage(argv[0], true);
        return false;
    }

    if (!StripQuotesAndParseParams(paramsJson, args)) {
        return false;
    }

    if (args.mode == ExecutionMode::DIRECT_ABC && !ValidateScriptFile(args.path)) {
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

    TAG_LOGI(AAFwkTag::APPKIT, "  Mode: %{public}d", static_cast<int>(mode));
    if (!path.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  Path: %{private}s", path.c_str());
    }
    if (!bundleName.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  BundleName: %{public}s", bundleName.c_str());
    }
    if (!skillName.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  SkillName: %{public}s", skillName.c_str());
    }
    if (!moduleName.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  ModuleName: %{public}s", moduleName.c_str());
    }
    if (!funName.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  Function: %{public}s", funName.c_str());
    }
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
