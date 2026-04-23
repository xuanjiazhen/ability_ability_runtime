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
#include <fstream>
#include <iostream>

#include "application_context.h"
#include "context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ArktsScript {
using OHOS::AbilityRuntime::JsRuntime;
namespace {

bool LooksLikeScriptName(const std::string& value)
{
    if (value.empty()) {
        return false;
    }
    if (value.size() > 4 && value.substr(value.size() - 4) == ".ets") {
        return true;
    }
    return std::isupper(static_cast<unsigned char>(value.front())) != 0;
}

void PrintUsage(const char* execName, bool isErrorLog)
{
    const std::string usage = std::string("Usage: ") + execName +
        " [options] -abc module.abc [script.ets] functionName [arguments]\n\n"
        "Options:\n"
        "  -h, --help    Show this help message\n\n"
        "Arguments:\n"
        "  -abc module.abc    ABC file path (required)\n"
        "  [script.ets]      Script class name for standalone skill (optional)\n"
        "  functionName       Function name to execute (required)\n"
        "  [arguments]        Function arguments as string array (optional)\n\n"
        "Examples:\n"
        "  " + execName + " -abc /data/app/test.abc calculate \"100\" \"200\"\n"
        "  " + execName + " -abc /data/skills/myskill.abc MySkill process \"input.json\"\n"
        "  " + execName + " -abc /data/app/test.abc process \"arg1\" \"arg2\"";

    if (isErrorLog) {
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
    }
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

bool ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) {
            continue;
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0], false);
            args.showHelp = true;
            return true;
        }
    }

    int i = 1;
    while (i < argc && argv[i][0] == '-' && strcmp(argv[i], "-abc") != 0) {
        std::string option = argv[i];
        TAG_LOGE(AAFwkTag::APPKIT, "Unknown option: %{public}s", option.c_str());
        PrintUsage(argv[0], true);
        return false;
    }

    if (argc < 4) {
        PrintUsage(argv[0], true);
        return false;
    }

    if (i >= argc || strcmp(argv[i], "-abc") != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing -abc option");
        PrintUsage(argv[0], true);
        return false;
    }
    i++;
    if (i >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing value for -abc");
        return false;
    }
    args.abcPath = argv[i++];

    if (i >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing functionName");
        PrintUsage(argv[0], true);
        return false;
    }
    if ((argc - i) >= 2 && LooksLikeScriptName(argv[i])) {
        args.scriptName = argv[i++];
    }

    if (i >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing functionName");
        PrintUsage(argv[0], true);
        return false;
    }
    args.funName = argv[i++];

    while (i < argc) {
        args.arguments.push_back(argv[i++]);
    }

    return ValidateScriptFile(args.abcPath);
}

void ScriptArgs::Print() const
{
    TAG_LOGI(AAFwkTag::APPKIT, "  ABC Path: %{private}s", abcPath.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Script: %{public}s", scriptName.empty() ? "<global>" : scriptName.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Function: %{public}s", funName.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Arguments: [%{public}zu]", arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        TAG_LOGI(AAFwkTag::APPKIT, "    argv[%{public}zu] = %{public}s", i, arguments[i].c_str());
    }
}

std::shared_ptr<ContextImpl> CreateScriptContext()
{
    auto scriptContext = std::make_shared<ContextImpl>();
    if (scriptContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to allocate ContextImpl");
        return nullptr;
    }

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "application context is unavailable, using standalone ContextImpl");
        return scriptContext;
    }

    scriptContext->SetParentContext(applicationContext);
    scriptContext->SetApplicationInfo(applicationContext->GetApplicationInfo());
    scriptContext->SetConfiguration(applicationContext->GetConfiguration());
    scriptContext->SetToken(applicationContext->GetToken());
    scriptContext->SetProcessName(applicationContext->GetProcessName());
    return scriptContext;
}

JsRuntime* CreateJsRuntime(const std::shared_ptr<ContextImpl>& context)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Creating new Runtime");

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::ETS;
    options.loadAce = false;
    options.isBundle = false;
    options.isStageModel = true;
    if (context != nullptr) {
        options.bundleName = context->GetBundleName();
        options.codePath = context->GetBundleCodePath();
        options.bundleCodeDir = context->GetBundleCodeDir();
    }

    auto runtime = JsRuntime::Create(options);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create JsRuntime");
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "JsRuntime created successfully");
    return runtime.release();
}

} // namespace ArktsScript
} // namespace OHOS
