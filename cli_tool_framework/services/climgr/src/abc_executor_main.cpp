/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

/**
 * @file abc_executor_main.cpp
 * @brief Standalone executor for ArkTS abc files
 *
 * Usage: abc_executor <abcPath> <bundleName> [arg1] [arg2] ...
 *
 * This program initializes the ArkTS runtime and executes the specified
 * .abc file in a clean process environment.
 */

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "event_handler.h"
#include "ets_environment.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FAILED = 1;
constexpr int32_t ERR_INVALID_ARGS = 2;
constexpr int32_t ERR_INVALID_PATH = 3;
constexpr int32_t ERR_RUNTIME_INIT_FAILED = 4;
constexpr int32_t ERR_LOAD_FAILED = 5;
constexpr int32_t ERR_CALL_MAIN_FAILED = 6;

void PrintUsage(const char *progName)
{
    std::cout << "Usage: " << progName << " <abcPath> <bundleName> [arg1] [arg2] ...\n"
              << "\n"
              << "Arguments:\n"
              << "  abcPath     Absolute path to the .abc file to execute\n"
              << "  bundleName  Bundle name for ArkTS runtime initialization\n"
              << "  argN        Optional arguments to pass to the abc main method\n"
              << "\n"
              << "Exit codes:\n"
              << "  0 - Success\n"
              << "  1 - General error\n"
              << "  2 - Invalid arguments\n"
              << "  3 - Invalid path\n"
              << "  4 - Runtime initialization failed\n"
              << "  5 - Failed to load abc file\n"
              << "  6 - Failed to call main method\n"
              << std::endl;
}

bool InitializeArkTsRuntime(const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Initializing ArkTS Runtime with bundleName: %{public}s", bundleName.c_str());

    // 1. Initialize namespaces
    ETSEnvironment::InitETSSDKNS("/system/lib64");
    ETSEnvironment::InitETSSysNS("/system/lib64");

    // 2. Create EventRunner for runtime
    auto eventRunner = AppExecFwk::EventRunner::Create(true);
    if (!eventRunner) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "EventRunner::Create failed");
        return false;
    }

    // 3. Initialize ETSEnvironment with provided bundle name
    auto &etsEnv = ETSEnvironment::GetInstance();
    if (!etsEnv) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::GetInstance failed");
        return false;
    }

    if (!etsEnv->Initialize(eventRunner, false, bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::Initialize failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "ArkTS Runtime initialized successfully");
    return true;
}

/**
 * @brief Call the main method of the loaded abc class with arguments
 * @param aniEnv The ANI environment
 * @param cls The loaded class
 * @param args Arguments to pass to main method
 * @return true if successful
 */
bool CallMainMethod(ani_env *aniEnv, ani_class cls, const std::vector<std::string> &args)
{
    if (aniEnv == nullptr || cls == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid parameters: aniEnv or cls is null");
        return false;
    }

    ani_status status = ANI_ERROR;

    // 1. Find the main method: static main(args: string[]): void | number
    ani_method mainMethod = nullptr;
    status = aniEnv->Class_FindStaticMethod(cls, "main", "Lstd/core/StringArray;->E", &mainMethod);
    if (status != ANI_OK) {
        // Try alternative signature: main(args: string[]): number
        status = aniEnv->Class_FindStaticMethod(cls, "main", "Lstd/core/StringArray;->I", &mainMethod);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Class_FindStaticMethod for main failed: %{public}d", status);
            return false;
        }
    }

    // 2. Create a String array for arguments
    ani_ref stringClassRef = nullptr;
    status = aniEnv->FindClass("std/core/String", reinterpret_cast<ani_class *>(&stringClassRef));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "FindClass std/core/String failed: %{public}d", status);
        return false;
    }

    // Create array of strings
    ani_array argsArray = nullptr;
    status = aniEnv->Array_NewString(static_cast<ani_size>(args.size()),
                                     reinterpret_cast<ani_class>(stringClassRef), &argsArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Array_NewString failed: %{public}d", status);
        aniEnv->ReleaseLocalRef(stringClassRef);
        return false;
    }

    // Fill the array with argument strings
    for (size_t i = 0; i < args.size(); i++) {
        ani_string argString = nullptr;
        status = aniEnv->String_NewUTF8(args[i].c_str(), args[i].length(), &argString);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "String_NewUTF8 failed for arg[%{public}zu]: %{public}d", i, status);
            aniEnv->ReleaseLocalRef(argsArray);
            aniEnv->ReleaseLocalRef(stringClassRef);
            return false;
        }

        status = aniEnv->Array_SetStringElement(argsArray, static_cast<ani_size>(i), argString);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Array_SetStringElement failed for arg[%{public}zu]: %{public}d", i, status);
            aniEnv->ReleaseLocalRef(argString);
            aniEnv->ReleaseLocalRef(argsArray);
            aniEnv->ReleaseLocalRef(stringClassRef);
            return false;
        }

        aniEnv->ReleaseLocalRef(argString);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Calling main method with %{public}zu arguments", args.size());

    // 3. Call the main method
    ani_ref result = nullptr;
    status = aniEnv->Class_CallStaticMethod_Ref(cls, mainMethod, &result, argsArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Class_CallStaticMethod_Ref failed: %{public}d", status);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Main method called successfully");
    }

    // Clean up local references
    aniEnv->ReleaseLocalRef(argsArray);
    aniEnv->ReleaseLocalRef(stringClassRef);
    if (result != nullptr) {
        aniEnv->ReleaseLocalRef(result);
    }

    return status == ANI_OK;
}

bool LoadAndExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Loading abc: %{public}s with %{public}zu args", abcPath.c_str(), args.size());

    auto &etsEnv = ETSEnvironment::GetInstance();
    if (!etsEnv) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::GetInstance failed");
        return false;
    }

    // Get ANI environment
    ani_env *aniEnv = etsEnv->GetAniEnv();
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAniEnv failed");
        return false;
    }

    // Extract module path (everything before last slash)
    std::string modulePath = abcPath;
    size_t pos = abcPath.rfind('/');
    if (pos != std::string::npos) {
        modulePath = abcPath.substr(0, pos);
    }

    // Extract filename without extension as class name
    std::string className = abcPath;
    pos = abcPath.rfind('/');
    if (pos != std::string::npos) {
        className = abcPath.substr(pos + 1);
    }
    pos = className.rfind(".abc");
    if (pos != std::string::npos) {
        className = className.substr(0, pos);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "ModulePath: %{public}s, ClassName: %{public}s",
             modulePath.c_str(), className.c_str());

    // Load the module using ETSEnvironment
    void *cls = nullptr;
    void *obj = nullptr;
    void *ref = nullptr;

    if (!etsEnv->LoadModule(modulePath, className, cls, obj, ref)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "LoadModule failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "abc loaded successfully");

    // Call the main method with arguments
    if (!CallMainMethod(aniEnv, static_cast<ani_class>(cls), args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "CallMainMethod failed");
        return false;
    }

    return true;
}

} // namespace

} // namespace CliTool
} // namespace OHOS

int main(int argc, char *argv[])
{
    using namespace OHOS::CliTool;

    // Parse arguments
    if (argc < 3) {
        std::cerr << "Error: Invalid arguments" << std::endl;
        PrintUsage(argv[0]);
        return ERR_INVALID_ARGS;
    }

    const char *abcPath = argv[1];
    const char *bundleName = argv[2];

    // Validate abc path
    if (abcPath[0] != '/') {
        std::cerr << "Error: abcPath must be absolute path: " << abcPath << std::endl;
        return ERR_INVALID_PATH;
    }

    // Collect optional arguments (argv[3] onwards)
    std::vector<std::string> args;
    for (int i = 3; i < argc; i++) {
        args.push_back(argv[i]);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "abc_executor: abcPath=%{public}s, bundleName=%{public}s, args=%{public}zu",
             abcPath, bundleName, args.size());

    // Initialize ArkTS Runtime
    if (!InitializeArkTsRuntime(bundleName)) {
        std::cerr << "Error: Failed to initialize ArkTS Runtime" << std::endl;
        return ERR_RUNTIME_INIT_FAILED;
    }

    // Load and execute abc
    if (!LoadAndExecuteAbc(abcPath, args)) {
        std::cerr << "Error: Failed to load or execute abc file" << std::endl;
        return ERR_LOAD_FAILED;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "abc execution completed successfully");
    return ERR_OK;
}
