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
#include <cstring>
#include <fstream>
#include <iostream>

#include "context.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ArktsScript {
using OHOS::AbilityRuntime::JsRuntime;
namespace {

void PrintUsage(const char* execName, bool isErrorLog)
{
    const std::string usage = std::string("Usage: ") + execName +
        " [options] -abc module.abc [script.ets] functionName [arguments]\n\n"
        "Options:\n"
        "  -h, --help    Show this help message\n\n"
        "Arguments:\n"
        "  -abc module.abc    ABC file path (required)\n"
        "  [script.ets]       Script class name (optional, for standalone skill)\n"
        "  functionName       Function name to execute (required)\n"
        "  [arguments]        Function arguments as string array (optional)\n\n"
        "Examples:\n"
        "  " + execName + " -abc /data/app/test.abc calculate \"100\" \"200\"\n"
        "  " + execName + " -abc /data/skills/myskill.abc MySkill process \"input.json\"";

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

    if (i < argc && argv[i][0] != '-') {
        args.scriptClass = argv[i++];
        args.skillType = SkillType::STANDALONE_SKILL;
    } else {
        args.skillType = SkillType::APP_SKILL;
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
    TAG_LOGI(AAFwkTag::APPKIT, "  Skill Type: %{public}d", static_cast<int>(skillType));
    if (!scriptClass.empty()) {
        TAG_LOGI(AAFwkTag::APPKIT, "  Script Class: %{public}s", scriptClass.c_str());
    }
    TAG_LOGI(AAFwkTag::APPKIT, "  Function: %{public}s", funName.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "  Arguments: [%{public}zu]", arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        TAG_LOGI(AAFwkTag::APPKIT, "    argv[%{public}zu] = %{public}s", i, arguments[i].c_str());
    }
}

SkillType ResolveSkillType(const std::string& abcPath)
{
    if (abcPath.find("/standalone_skills/") != std::string::npos ||
        abcPath.find("/skills/") != std::string::npos) {
        return SkillType::STANDALONE_SKILL;
    }
    return SkillType::APP_SKILL;
}

JsRuntime* GetExistingJsRuntime()
{
    TAG_LOGI(AAFwkTag::APPKIT, "Attempting to get existing JsRuntime from application context");

    auto applicationContext = AbilityRuntime::Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "No application context found, not running in application process");
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Application context found, but Runtime retrieval not yet implemented");
    return nullptr;
}

JsRuntime* CreateRuntimeForSkill(SkillType skillType)
{
    if (skillType == SkillType::APP_SKILL) {
        JsRuntime* existingRuntime = GetExistingJsRuntime();
        if (existingRuntime != nullptr) {
            TAG_LOGI(AAFwkTag::APPKIT, "Using existing application Runtime for APP_SKILL");
            return existingRuntime;
        }
        TAG_LOGW(AAFwkTag::APPKIT, "No existing Runtime found, creating new Runtime for APP_SKILL");
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Creating new Runtime for skill type: %{public}d", static_cast<int>(skillType));

    Runtime::Options options;
    options.lang = Runtime::Language::ETS;
    options.isBundle = false;
    options.isStageModel = true;
    options.loadAce = true;

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
