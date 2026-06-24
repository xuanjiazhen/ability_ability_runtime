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
 * Usage: abc_executor <path> [arg1] [arg2] ...
 *
 * The <path> parameter can be:
 *   - Full path to abc file (e.g., /data/.../module.abc)
 *   - HAP module directory (e.g., /data/.../library)
 *     RuntimeLinker will automatically find the abc file in the directory.
 *
 * Environment variables:
 *   ABC_CONTEXT_FD - File descriptor to read context data from
 *   ABC_CALLER_TOKEN_ID - Caller's token ID
 *   ABC_BUNDLE_NAME - Bundle name for runtime initialization
 *   ABC_MODULE_NAME - Module name (optional)
 *   ABC_FUNCTION_NAME - Function name to call (optional, empty for main)
 */

#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "event_handler.h"
#include "ets_environment.h"
#include "hilog_tag_wrapper.h"
#include "context_data.h"
#include "ability_manager_client.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FAILED = 1;
constexpr int32_t ERR_INVALID_ARGS = 2;
constexpr int32_t ERR_INVALID_PATH = 3;
constexpr int32_t ERR_RUNTIME_INIT_FAILED = 4;
constexpr int32_t ERR_LOAD_FAILED = 5;
constexpr int32_t ERR_CALL_FAILED = 6;

void PrintUsage(const char *progName)
{
    std::cout << "Usage: " << progName << " <abcPath> [arg1] [arg2] ...\n"
              << "\n"
              << "Arguments:\n"
              << "  abcPath     Absolute path to the .abc file to execute\n"
              << "  argN        Optional arguments to pass to the function\n"
              << "\n"
              << "Environment Variables:\n"
              << "  ABC_CONTEXT_FD         - File descriptor to read context data\n"
              << "  ABC_CALLER_TOKEN_ID    - Caller's token ID\n"
              << "  ABC_BUNDLE_NAME        - Bundle name for runtime init\n"
              << "  ABC_MODULE_NAME        - Module name (optional)\n"
              << "  ABC_FUNCTION_NAME      - Function name to call (empty for main)\n"
              << "\n"
              << "Exit codes:\n"
              << "  0 - Success\n"
              << "  1 - General error\n"
              << "  2 - Invalid arguments\n"
              << "  3 - Invalid path\n"
              << "  4 - Runtime initialization failed\n"
              << "  5 - Failed to load abc file\n"
              << "  6 - Failed to call function\n"
              << std::endl;
}
} // namespace

/**
 * @brief Initialize ArkTS Runtime with caller's context
 */
bool InitializeArkTSRuntime(const ContextData &contextData)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== InitializeArkTSRuntime START ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Initializing ArkTS Runtime: bundle=%{public}s, module=%{public}s, tokenId=%{public}u",
        contextData.bundleName.c_str(), contextData.moduleName.c_str(), contextData.tokenId);

    // 1. Initialize namespaces
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Step 1: Initializing SDK and System namespaces");
    ETSEnvironment::InitETSSDKNS("/system/lib64");
    ETSEnvironment::InitETSSysNS("/system/lib64");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Namespaces initialized");

    // 2. Create EventRunner for runtime
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Step 2: Creating EventRunner");
    auto eventRunner = AppExecFwk::EventRunner::Create(true);
    if (!eventRunner) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "EventRunner::Create failed");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "EventRunner created successfully");

    // 3. Initialize ETSEnvironment with caller's bundle name
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Step 3: Getting ETSEnvironment instance");
    auto &etsEnv = ETSEnvironment::GetInstance();
    if (!etsEnv) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::GetInstance failed");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ETSEnvironment instance obtained");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Step 4: Initializing ETSEnvironment with bundleName=%{public}s",
        contextData.bundleName.c_str());
    if (!etsEnv->Initialize(eventRunner, false, contextData.bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::Initialize failed for bundle=%{public}s",
            contextData.bundleName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== InitializeArkTSRuntime SUCCESS ==========");
    return true;
}

/**
 * @brief Call a static method with string array arguments
 * @param functionName The name of the function to call (defaults to "main" if empty)
 */
bool CallArrayMethod(ani_env *aniEnv, ani_class cls, const std::string &functionName,
    const std::vector<std::string> &args)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallArrayMethod START ==========");

    if (aniEnv == nullptr || cls == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid parameters: aniEnv=%{public}p, cls=%{public}p",
            aniEnv, cls);
        return false;
    }

    ani_status status = ANI_ERROR;

    // Use provided function name or default to "main"
    const std::string &funcName = functionName.empty() ? "main" : functionName;

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Looking for array method: %{public}s(args: string[])",
        funcName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Arguments count: %{public}zu", args.size());

    // Find the method: static func(args: string[]): void | number
    ani_method method = nullptr;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Trying signature: Lstd/core/StringArray;->E (void)");
    status = aniEnv->Class_FindStaticMethod(cls, funcName.c_str(), "Lstd/core/StringArray;->E", &method);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Signature Lstd/core/StringArray;->E not found, trying ->I (int)");
        status = aniEnv->Class_FindStaticMethod(cls, funcName.c_str(), "Lstd/core/StringArray;->I", &method);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Class_FindStaticMethod for %{public}s failed: %{public}d",
                funcName.c_str(), status);
            return false;
        }
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Method found: %{public}s", funcName.c_str());

    // Create String array for arguments
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Creating String array with %{public}zu elements", args.size());
    ani_ref stringClassRef = nullptr;
    status = aniEnv->FindClass("std/core/String", reinterpret_cast<ani_class *>(&stringClassRef));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "FindClass std/core/String failed: %{public}d", status);
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "String class found");

    ani_array argsArray = nullptr;
    status = aniEnv->Array_NewString(static_cast<ani_size>(args.size()),
                                     reinterpret_cast<ani_class>(stringClassRef), &argsArray);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Array_NewString failed: %{public}d", status);
        aniEnv->ReleaseLocalRef(stringClassRef);
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "String array created successfully");

    // Fill the array with argument strings
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Filling array with arguments:");
    for (size_t i = 0; i < args.size(); i++) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "  [%{public}zu]: %{public}s", i, args[i].c_str());
        ani_string argString = nullptr;
        status = aniEnv->String_NewUTF8(args[i].c_str(), args[i].length(), &argString);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "String_NewUTF8 failed for arg[%{public}zu]", i);
            aniEnv->ReleaseLocalRef(argsArray);
            aniEnv->ReleaseLocalRef(stringClassRef);
            return false;
        }

        status = aniEnv->Array_SetStringElement(argsArray, static_cast<ani_size>(i), argString);
        aniEnv->ReleaseLocalRef(argString);

        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Array_SetStringElement failed for arg[%{public}zu]", i);
            break;
        }
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Array filled successfully");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Calling %{public}s method with %{public}zu arguments",
        funcName.c_str(), args.size());

    // Call the method
    ani_ref result = nullptr;
    status = aniEnv->Class_CallStaticMethod_Ref(cls, method, &result, argsArray);

    // Clean up
    aniEnv->ReleaseLocalRef(argsArray);
    aniEnv->ReleaseLocalRef(stringClassRef);
    if (result != nullptr) {
        aniEnv->ReleaseLocalRef(result);
    }

    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Class_CallStaticMethod_Ref failed: %{public}d", status);
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallArrayMethod SUCCESS: %{public}s ==========",
        funcName.c_str());
    return true;
}

/**
 * @brief Call a named static function with string arguments
 */
bool CallNamedFunction(ani_env *aniEnv, ani_class cls, const std::string &functionName,
    const std::vector<std::string> &args)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallNamedFunction START ==========");

    if (aniEnv == nullptr || cls == nullptr || functionName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid parameters: aniEnv=%{public}p, cls=%{public}p, func=%{public}s",
            aniEnv, cls, functionName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Looking for function: %{public}s", functionName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Arguments count: %{public}zu", args.size());

    // Try to find static function with signature: func(arg1: string, arg2: string, ...): void
    // For simplicity, we'll try common signatures

    ani_method method = nullptr;
    ani_status status = ANI_ERROR;

    // Try with no arguments
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Trying signature: func() -> void");
    status = aniEnv->Class_FindStaticMethod(cls, functionName.c_str(), "->E", &method);
    if (status == ANI_OK && args.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Found: %{public}s() -> void, calling...", functionName.c_str());
        status = aniEnv->Class_CallStaticMethod_Ref(cls, method, nullptr);
        if (status == ANI_OK) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallNamedFunction SUCCESS (no args) ==========");
            return true;
        }
    }

    // Try with one string argument
    if (args.size() == 1) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Trying signature: func(string) -> void");
        status = aniEnv->Class_FindStaticMethod(cls, functionName.c_str(), "Lstd/core/String;->E", &method);
        if (status == ANI_OK) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Found: %{public}s(string) -> void, calling with: %{public}s",
                functionName.c_str(), args[0].c_str());
            ani_string argString = nullptr;
            status = aniEnv->String_NewUTF8(args[0].c_str(), args[0].length(), &argString);
            if (status == ANI_OK) {
                status = aniEnv->Class_CallStaticMethod_Ref(cls, method, nullptr, argString);
                aniEnv->ReleaseLocalRef(argString);
                if (status == ANI_OK) {
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallNamedFunction SUCCESS (1 string arg) ==========");
                    return true;
                }
            }
        }
    }

    // Try with two number arguments: func(a: number, b: number): number
    if (args.size() == 2) {
        // Try double double -> double (DD->D)
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Trying signature: func(number, number) -> number (DD->D)");
        status = aniEnv->Class_FindStaticMethod(cls, functionName.c_str(), "DD->D", &method);
        if (status == ANI_OK) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Found: DD->D signature");
            try {
                ani_double arg1 = std::stod(args[0]);
                ani_double arg2 = std::stod(args[1]);
                ani_double result;
                status = aniEnv->Class_CallStaticMethod_Double(cls, method, &result, arg1, arg2);
                if (status == ANI_OK) {
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallNamedFunction SUCCESS ==========");
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "Function %{public}s(%+.2f, %+.2f) = %+.2f",
                        functionName.c_str(), arg1, arg2, result);
                    return true;
                }
            } catch (const std::exception &e) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse numbers: %{public}s", e.what());
            }
        } else {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "DD->D signature not found");
        }

        // Try int int -> int (II->I)
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Trying signature: func(int, int) -> int (II->I)");
        status = aniEnv->Class_FindStaticMethod(cls, functionName.c_str(), "II->I", &method);
        if (status == ANI_OK) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Found: II->I signature");
            try {
                ani_int arg1 = std::stoi(args[0]);
                ani_int arg2 = std::stoi(args[1]);
                ani_int result;
                status = aniEnv->Class_CallStaticMethod_Int(cls, method, &result, arg1, arg2);
                if (status == ANI_OK) {
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== CallNamedFunction SUCCESS ==========");
                    TAG_LOGI(AAFwkTag::CLI_TOOL, "Function %{public}s(%d, %d) = %d",
                        functionName.c_str(), arg1, arg2, result);
                    return true;
                }
            } catch (const std::exception &e) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse integers: %{public}s", e.what());
            }
        } else {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "II->I signature not found");
        }
    }

    // For more complex signatures, use array-style interface
    // Try to find function(args: string[]): void | number
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Falling back to array-style interface: func(args: string[])");
    return CallArrayMethod(aniEnv, cls, functionName, args);
}

/**
 * @brief Load and execute abc file
 */
bool LoadAndExecuteAbc(const std::string &abcPath, const std::string &functionName,
    const std::vector<std::string> &args)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== LoadAndExecuteAbc START ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "abcPath: %{public}s", abcPath.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "functionName: %{public}s", functionName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "args count: %{public}zu", args.size());

    auto &etsEnv = ETSEnvironment::GetInstance();
    if (!etsEnv) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ETSEnvironment::GetInstance failed");
        return false;
    }

    ani_env *aniEnv = etsEnv->GetAniEnv();
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAniEnv failed");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ANI Environment obtained successfully");

    // Extract module path and class name
    std::string modulePath = abcPath;
    size_t pos = abcPath.rfind('/');
    if (pos != std::string::npos) {
        modulePath = abcPath.substr(0, pos);
    }

    std::string className = abcPath;
    pos = abcPath.rfind('/');
    if (pos != std::string::npos) {
        className = abcPath.substr(pos + 1);
    }
    pos = className.rfind(".abc");
    if (pos != std::string::npos) {
        className = className.substr(0, pos);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Extracted - ModulePath: %{public}s, ClassName: %{public}s",
        modulePath.c_str(), className.c_str());

    // Load the module
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Loading module from path: %{public}s", modulePath.c_str());
    void *clsPtr = nullptr;
    void *obj = nullptr;
    void *ref = nullptr;

    if (!etsEnv->LoadModule(modulePath, className, clsPtr, obj, ref)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "LoadModule failed for: %{public}s/%{public}s",
            modulePath.c_str(), className.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "abc loaded successfully: %{public}s", className.c_str());

    ani_class classRef = static_cast<ani_class>(clsPtr);

    // Call the function
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Calling function...");
    bool result = false;
    if (functionName.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionName is empty, calling 'main' method");
        result = CallArrayMethod(aniEnv, classRef, functionName, args);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "FunctionName is '%{public}s', calling named function",
            functionName.c_str());
        result = CallNamedFunction(aniEnv, classRef, functionName, args);
    }

    if (!result) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "CallFunction failed for %{public}s", functionName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== LoadAndExecuteAbc SUCCESS ==========");
    return true;
}

/**
 * @brief Read context data from pipe
 */
ContextData ReadContextData()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ReadContextData START ==========");

    const char *fdStr = std::getenv("ABC_CONTEXT_FD");
    if (fdStr == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "ABC_CONTEXT_FD not set, will try env vars");
        return ContextData();
    }

    int fd = atoi(fdStr);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Reading context from fd: %{public}d", fd);

    // Read data
    std::string data;
    char buffer[1024];
    ssize_t bytesRead;
    size_t totalBytes = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        data.append(buffer, bytesRead);
        totalBytes += bytesRead;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Read %{public}zd bytes (total: %{public}zu)",
            bytesRead, totalBytes);
    }

    close(fd);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Pipe closed, total bytes read: %{public}zu", totalBytes);

    if (data.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "No context data received from pipe");
        return ContextData();
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Parsing context data from %{public}zu bytes", data.size());
    ContextData result = ContextData::Deserialize(data);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ReadContextData END ==========");
    return result;
}

} // namespace CliTool
} // namespace OHOS

int main(int argc, char *argv[])
{
    using namespace OHOS::CliTool;

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== abc_executor START ==========");

    // Parse arguments
    if (argc < 2) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid arguments: argc=%{public}d", argc);
        std::cerr << "Error: Invalid arguments" << std::endl;
        PrintUsage(argv[0]);
        return ERR_INVALID_ARGS;
    }

    const char *abcPath = argv[1];

    // Validate abc path
    if (abcPath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "abcPath must be absolute path: %{public}s", abcPath);
        std::cerr << "Error: abcPath must be absolute path: " << abcPath << std::endl;
        return ERR_INVALID_PATH;
    }

    // Collect optional arguments
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // Get function name from environment
    std::string functionName;
    const char *funcNameEnv = std::getenv("ABC_FUNCTION_NAME");
    if (funcNameEnv != nullptr) {
        functionName = funcNameEnv;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "ABC_FUNCTION_NAME env: %{public}s", funcNameEnv);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "========== abc_executor PARAMS ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "abcPath: %{public}s", abcPath);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "functionName: %{public}s", functionName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "args count: %{public}zu", args.size());
    for (size_t i = 0; i < args.size(); ++i) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "  args[%{public}zu]: %{public}s", i, args[i].c_str());
    }

    // Read context data from pipe
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== Reading Context Data ==========");
    ContextData contextData = ReadContextData();

    // If no context from pipe, try environment variables (fallback)
    if (contextData.bundleName.empty()) {
        const char *bundleEnv = std::getenv("ABC_BUNDLE_NAME");
        if (bundleEnv != nullptr) {
            contextData.bundleName = bundleEnv;
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Using ABC_BUNDLE_NAME env: %{public}s", bundleEnv);
        }
    }

    if (contextData.moduleName.empty()) {
        const char *moduleEnv = std::getenv("ABC_MODULE_NAME");
        if (moduleEnv != nullptr) {
            contextData.moduleName = moduleEnv;
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Using ABC_MODULE_NAME env: %{public}s", moduleEnv);
        }
    }

    if (contextData.tokenId == 0) {
        const char *tokenIdEnv = std::getenv("ABC_CALLER_TOKEN_ID");
        if (tokenIdEnv != nullptr) {
            try {
                contextData.tokenId = static_cast<uint32_t>(std::stoul(tokenIdEnv));
                TAG_LOGI(AAFwkTag::CLI_TOOL, "Using ABC_CALLER_TOKEN_ID env: %{public}s", tokenIdEnv);
            } catch (...) {
                TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse ABC_CALLER_TOKEN_ID: %{public}s", tokenIdEnv);
            }
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== Context Data Summary ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "bundleName: %{public}s", contextData.bundleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "moduleName: %{public}s", contextData.moduleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "tokenId: %{public}u", contextData.tokenId);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "hapPath: %{public}s", contextData.hapPath.c_str());

    // ========== Initialize ArkTS Runtime ==========
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== Initializing ArkTS Runtime ==========");
    if (!contextData.IsValid()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid context data");
        std::cerr << "Error: Invalid context data" << std::endl;
        return ERR_RUNTIME_INIT_FAILED;
    }

    if (!InitializeArkTSRuntime(contextData)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to initialize ArkTS Runtime");
        std::cerr << "Error: Failed to initialize ArkTS Runtime" << std::endl;
        return ERR_RUNTIME_INIT_FAILED;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ArkTS Runtime initialized successfully");

    // TODO: Inject Context object into runtime
    // InjectContextProxy(etsEnv->GetAniEnv(), contextData);

    // ========== Load and Execute ABC ==========
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== Loading and Executing ABC ==========");
    if (!LoadAndExecuteAbc(abcPath, functionName, args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to load or execute abc file");
        std::cerr << "Error: Failed to load or execute abc file" << std::endl;
        return ERR_LOAD_FAILED;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== abc_executor COMPLETED SUCCESSFULLY ==========");
    return ERR_OK;
}
