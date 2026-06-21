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

#include "arkts_executor.h"

#include <sys/wait.h>
#include <unistd.h>

#include "event_handler.h"
#include "ets_environment.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

int32_t ArktsExecutor::ExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLIMGR, "ExecuteAbc: %{public}s, args=%{public}zu, bundleName=%{public}s",
             abcPath.c_str(), args.size(), bundleName.c_str());

    // Validate path
    if (abcPath.empty() || abcPath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLIMGR, "Invalid abc path: must be absolute path");
        return ERR_INVALID_PATH;
    }

    // Fork child process
    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLIMGR, "fork failed: %{public}d", errno);
        return ERR_FAILED;
    }

    if (pid == 0) {
        // Child process
        ChildProcessEntry(abcPath, args, bundleName);
        // Should not reach here
        _exit(1);
    }

    // Parent process: wait for child to complete
    TAG_LOGI(AAFwkTag::CLIMGR, "Child process started with pid: %{public}d", pid);

    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::CLIMGR, "waitpid failed: %{public}d", errno);
        return ERR_FAILED;
    }

    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        TAG_LOGI(AAFwkTag::CLIMGR, "Child exited with code: %{public}d", exitCode);
        return exitCode == 0 ? ERR_OK : ERR_FAILED;
    }

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        TAG_LOGE(AAFwkTag::CLIMGR, "Child terminated by signal: %{public}d", signal);
        return ERR_FAILED;
    }

    TAG_LOGE(AAFwkTag::CLIMGR, "Child exited abnormally");
    return ERR_FAILED;
}

void ArktsExecutor::ChildProcessEntry(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLIMGR, "Child process executing: %{public}s, args=%{public}zu, bundleName=%{public}s",
             abcPath.c_str(), args.size(), bundleName.c_str());

    // Initialize ArkTS Runtime
    if (!InitializeArkTsRuntime(bundleName)) {
        TAG_LOGE(AAFwkTag::CLIMGR, "InitializeArkTsRuntime failed");
        _exit(1);
    }

    // Load and execute abc
    if (!LoadAndExecuteAbc(abcPath, args)) {
        TAG_LOGE(AAFwkTag::CLIMGR, "LoadAndExecuteAbc failed");
        _exit(1);
    }

    TAG_LOGI(AAFwkTag::CLIMGR, "abc execution completed successfully");
    _exit(0);
}

bool ArktsExecutor::InitializeArkTsRuntime(const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLIMGR, "Initializing ArkTS Runtime with bundleName: %{public}s", bundleName.c_str());

    // 1. Initialize namespaces
    ETSEnvironment::InitETSSDKNS("/system/lib64");
    ETSEnvironment::InitETSSysNS("/system/lib64");

    // 2. Create EventRunner for runtime
    auto eventRunner = AppExecFwk::EventRunner::Create(true);
    if (!eventRunner) {
        TAG_LOGE(AAFwkTag::CLIMGR, "EventRunner::Create failed");
        return false;
    }

    // 3. Initialize ETSEnvironment with provided bundle name
    auto &etsEnv = ETSEnvironment::GetInstance();
    if (!etsEnv->Initialize(eventRunner, false, bundleName)) {
        TAG_LOGE(AAFwkTag::CLIMGR, "ETSEnvironment::Initialize failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLIMGR, "ArkTS Runtime initialized successfully");
    return true;
}

bool ArktsExecutor::LoadAndExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args)
{
    TAG_LOGI(AAFwkTag::CLIMGR, "Loading abc: %{public}s with %{public}zu args", abcPath.c_str(), args.size());

    auto &etsEnv = ETSEnvironment::GetInstance();
    ani_env *env = etsEnv->GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::CLIMGR, "GetAniEnv failed");
        return false;
    }

    // Extract module name and class name from abc path
    // Path format: /data/app/.../moduleName.abc
    std::string abcPathWithoutExt = abcPath;
    size_t pos = abcPath.rfind(".abc");
    if (pos != std::string::npos) {
        abcPathWithoutExt = abcPath.substr(0, pos);
    }

    // Extract filename without extension as class name
    std::string className = abcPathWithoutExt;
    pos = abcPathWithoutExt.rfind('/');
    if (pos != std::string::npos) {
        className = abcPathWithoutExt.substr(pos + 1);
    }

    // Extract module path (everything before last slash)
    std::string modulePath = abcPath;
    pos = abcPath.rfind('/');
    if (pos != std::string::npos) {
        modulePath = abcPath.substr(0, pos);
    }

    TAG_LOGI(AAFwkTag::CLIMGR, "ModulePath: %{public}s, ClassName: %{public}s",
             modulePath.c_str(), className.c_str());

    // Construct full module name (e.g., "entry::ClassName")
    std::string moduleName = modulePath + "::" + className;

    // Load the module
    void *cls = nullptr;
    void *obj = nullptr;
    void *ref = nullptr;

    if (!etsEnv->LoadModule(modulePath, className, cls, obj, ref)) {
        TAG_LOGE(AAFwkTag::CLIMGR, "LoadModule failed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLIMGR, "Module loaded successfully");

    // Create ANI string array from args
    ani_array aniArgs = nullptr;
    if (!CreateAniStringArray(env, args, aniArgs)) {
        TAG_LOGE(AAFwkTag::CLIMGR, "CreateAniStringArray failed");
        return false;
    }

    // Find and call the 'main' method with signature: ([Ljava/lang/String;)V
    ani_method method = nullptr;
    ani_status status = env->Class_FindMethod(
        static_cast<ani_class>(cls),
        "main",
        "([Ljava/lang/String;)V",
        &method);

    if (status != ANI_OK || !method) {
        TAG_LOGE(AAFwkTag::CLIMGR, "FindMethod 'main' failed, status: %{public}d", status);
        return false;
    }

    // Call the method with args array
    status = env->Object_CallMethod_Void(static_cast<ani_object>(obj), method, aniArgs);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLIMGR, "CallMethod 'main' failed, status: %{public}d", status);
        return false;
    }

    TAG_LOGI(AAFwkTag::CLIMGR, "abc executed successfully");
    return true;
}

bool ArktsExecutor::CreateAniStringArray(ani_env *env, const std::vector<std::string> &args, ani_array &aniArgs)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::CLIMGR, "CreateAniStringArray: env is null");
        return false;
    }

    // Create a String array of the appropriate size
    ani_status status = env->Array_Create_String(args.size(), &aniArgs);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLIMGR, "Array_Create_String failed, status: %{public}d", status);
        return false;
    }

    // Fill the array with string values
    for (size_t i = 0; i < args.size(); ++i) {
        ani_string aniStr = nullptr;
        status = env->String_CreateUTF8(args[i].c_str(), args[i].length(), &aniStr);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLIMGR, "String_CreateUTF8 failed for index %{public}zu, status: %{public}d", i, status);
            return false;
        }

        status = env->Array_Set(aniArgs, i, aniStr);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::CLIMGR, "Array_Set failed for index %{public}zu, status: %{public}d", i, status);
            return false;
        }
    }

    TAG_LOGD(AAFwkTag::CLIMGR, "Created ANI string array with %{public}zu elements", args.size());
    return true;
}

} // namespace CliTool
} // namespace OHOS
