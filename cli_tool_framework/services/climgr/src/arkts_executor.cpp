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

#include "arkts_executor.h"

#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "hilog_tag_wrapper.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_helper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FAILED = -1;
constexpr int32_t ERR_INVALID_PARAM = -2;
constexpr int32_t ERR_INVALID_PATH = -3;
constexpr int32_t ERR_FORK_FAILED = -4;
constexpr int32_t ERR_EXEC_FAILED = -5;

// Path to the abc_executor binary
constexpr const char *ABC_EXECUTOR_PATH = "/system/bin/abc_executor";

// Environment variable names
constexpr const char *ENV_CONTEXT_FD = "ABC_CONTEXT_FD";
constexpr const char *ENV_CALLER_TOKEN_ID = "ABC_CALLER_TOKEN_ID";
constexpr const char *ENV_BUNDLE_NAME = "ABC_BUNDLE_NAME";
constexpr const char *ENV_MODULE_NAME = "ABC_MODULE_NAME";
constexpr const char *ENV_FUNCTION_NAME = "ABC_FUNCTION_NAME";
} // namespace

/**
 * @brief Query bundle information from BundleManager
 */
static bool QueryBundleInfo(const std::string &bundleName, ContextData &contextData)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get BundleMgrHelper");
        return false;
    }

    int32_t userId = 0;  // TODO: Get from token
    AppExecFwk::BundleInfo bundleInfo;

    auto ret = bundleMgrHelper->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITY_INFO, userId, bundleInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetBundleInfo failed for %{public}s", bundleName.c_str());
        return false;
    }

    // Set code path and hap path
    if (!bundleInfo.hapModuleInfos.empty()) {
        contextData.codePath = bundleInfo.codePath;
        contextData.hapPath = bundleInfo.hapModuleInfos[0].hapPath;
        if (contextData.moduleName.empty()) {
            contextData.moduleName = bundleInfo.hapModuleInfos[0].moduleName;
        }
    }

    return true;
}

/**
 * @brief Find hap module path for HSP
 * @param hspBundleName HSP bundle name (e.g., "com.example.library")
 * @param hspModuleName HSP module name (e.g., "library")
 * @param hapPath Output hap module path (the directory containing abc files)
 * @param className Output class name (module name without .abc)
 * @return true if found
 *
 * Note: This returns the hapPath directory, not the abc file path.
 * The RuntimeLinker will automatically find the abc file within this directory.
 */
static bool FindHspModulePath(const std::string &hspBundleName, const std::string &hspModuleName,
    std::string &hapPath, std::string &className)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Finding HSP module path: bundle=%{public}s, module=%{public}s",
        hspBundleName.c_str(), hspModuleName.c_str());

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get BundleMgrHelper");
        return false;
    }

    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;

    // Get HSP bundle info
    auto ret = bundleMgrHelper->GetBundleInfo(hspBundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITY_INFO, userId, bundleInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetBundleInfo failed for HSP %{public}s", hspBundleName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "HSP has %{public}zu module(s)", bundleInfo.hapModuleInfos.size());

    // Find the matching module
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Checking module: %{public}s (hapPath=%{public}s)",
            hapModuleInfo.moduleName.c_str(), hapModuleInfo.hapPath.c_str());

        if (hapModuleInfo.moduleName == hspModuleName) {
            // Found the module
            // Return the hapPath (directory), RuntimeLinker will find abc file
            hapPath = hapModuleInfo.hapPath;
            className = hspModuleName;

            TAG_LOGI(AAFwkTag::CLI_TOOL, "Found HSP module:");
            TAG_LOGI(AAFwkTag::CLI_TOOL, "  hapPath: %{public}s", hapPath.c_str());
            TAG_LOGI(AAFwkTag::CLI_TOOL, "  className: %{public}s", className.c_str());
            return true;
        }
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "Module %{public}s not found in HSP %{public}s",
        hspModuleName.c_str(), hspBundleName.c_str());
    return false;
}

int32_t ArktsExecutor::ExecuteAbc(const AbcExecuteParam &param)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ExecuteAbc START ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "abcPath: %{public}s", param.abcPath.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "hspBundleName: %{public}s", param.hspBundleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "hspModuleName: %{public}s", param.hspModuleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "functionName: %{public}s", param.functionName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "args count: %{public}zu", param.args.size());

    // Validate parameters
    if (!param.IsValid()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid parameters: need either abcPath or hspBundleName+hspModuleName");
        return ERR_INVALID_PARAM;
    }

    // Determine execution path
    std::string execPath;    // Path to pass to abc_executor
    std::string className;    // Class name (module name)

    if (param.UseHspLookup()) {
        // Mode 2: Lookup hap path from HSP info
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Using HSP lookup mode");
        if (!FindHspModulePath(param.hspBundleName, param.hspModuleName, execPath, className)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to find HSP module");
            return ERR_INVALID_PATH;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "HSP module resolved: execPath=%{public}s, className=%{public}s",
            execPath.c_str(), className.c_str());
    } else {
        // Mode 1: Use provided abc path directly
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Using direct abc path mode");
        execPath = param.abcPath;
        // Extract className from abcPath
        size_t pos = execPath.rfind('/');
        if (pos != std::string::npos) {
            className = execPath.substr(pos + 1);
        }
        pos = className.rfind(".abc");
        if (pos != std::string::npos) {
            className = className.substr(0, pos);
        }
    }

    // Validate path
    if (execPath.empty() || execPath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid path: must be absolute path");
        return ERR_INVALID_PATH;
    }

    // Get caller tokenId
    uint32_t callerTokenId = param.callerTokenId;
    if (callerTokenId == 0) {
        callerTokenId = IPCSkeleton::GetCallingTokenID();
    }

    // Get bundle name from token if not provided
    std::string bundleName = param.bundleName;
    if (bundleName.empty()) {
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(callerTokenId, hapInfo);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get hap token info");
            return ERR_INVALID_PARAM;
        }
        bundleName = hapInfo.bundleName;
    }

    // Build ContextData
    ContextData contextData;
    contextData.bundleName = bundleName;
    contextData.moduleName = param.moduleName;
    contextData.tokenId = callerTokenId;

    // Query additional bundle info
    QueryBundleInfo(bundleName, contextData);

    // Serialize context data
    std::string serializedContext = contextData.Serialize();

    // Create pipe for passing context data
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "pipe failed: %{public}d", errno);
        return ERR_FAILED;
    }

    // Fork child process
    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "fork failed: %{public}d", errno);
        close(pipefd[0]);
        close(pipefd[1]);
        return ERR_FORK_FAILED;
    }

    if (pid == 0) {
        // Child process

        // Close write end, keep read end
        close(pipefd[1]);

        // Set environment variables
        char fdStr[32];
        snprintf(fdStr, sizeof(fdStr), "%d", pipefd[0]);
        setenv(ENV_CONTEXT_FD, fdStr, 1);

        char tokenIdStr[32];
        snprintf(tokenIdStr, sizeof(tokenIdStr), "%u", callerTokenId);
        setenv(ENV_CALLER_TOKEN_ID, tokenIdStr, 1);

        if (!bundleName.empty()) {
            setenv(ENV_BUNDLE_NAME, bundleName.c_str(), 1);
        }

        if (!param.moduleName.empty()) {
            setenv(ENV_MODULE_NAME, param.moduleName.c_str(), 1);
        }

        if (!param.functionName.empty()) {
            setenv(ENV_FUNCTION_NAME, param.functionName.c_str(), 1);
        }

        // Build argv array for execvp
        std::vector<char *> argv;
        argv.reserve(param.args.size() + 3);  // +3 for: program, path, nullptr

        argv.push_back(const_cast<char *>(ABC_EXECUTOR_PATH));
        argv.push_back(const_cast<char *>(execPath.c_str()));  // Use resolved path

        for (const auto &arg : param.args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);

        TAG_LOGI(AAFwkTag::CLI_TOOL, "Child executing: %{public}s", ABC_EXECUTOR_PATH);
        execvp(ABC_EXECUTOR_PATH, argv.data());

        // execvp only returns if it fails
        TAG_LOGE(AAFwkTag::CLI_TOOL, "execvp failed: %{public}d", errno);
        _exit(1);
    }

    // Parent process

    // Close read end, write context data
    close(pipefd[0]);

    ssize_t written = write(pipefd[1], serializedContext.data(), serializedContext.size());
    close(pipefd[1]);

    if (written < 0 || static_cast<size_t>(written) != serializedContext.size()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write context data");
        // Continue waiting for child to exit
    }

    // Wait for child to complete
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Waiting for child pid: %{public}d", pid);

    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "waitpid failed: %{public}d", errno);
        return ERR_FAILED;
    }

    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Child exited with code: %{public}d", exitCode);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ExecuteAbc END ==========");
        return exitCode == 0 ? ERR_OK : exitCode;
    }

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Child terminated by signal: %{public}d", signal);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ExecuteAbc FAILED (signal) ==========");
        return ERR_FAILED;
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "Child exited abnormally");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== ExecuteAbc FAILED (abnormal) ==========");
    return ERR_FAILED;
}

} // namespace CliTool
} // namespace OHOS
