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
#include "ipc_skeleton.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ABC_EXEC_ERR_OK = 0;
constexpr int32_t ABC_EXEC_ERR_FAILED = -1;
constexpr int32_t ABC_EXEC_ERR_INVALID_PARAM = -2;
constexpr int32_t ABC_EXEC_ERR_INVALID_PATH = -3;
constexpr int32_t ABC_EXEC_ERR_FORK_FAILED = -4;
constexpr int32_t ABC_EXEC_ERR_EXEC_FAILED = -5;

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
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] QueryBundleInfo: bundleName='%{public}s'", bundleName.c_str());
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to get BundleMgrHelper");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] BundleMgrHelper obtained");

    int32_t userId = 0;  // TODO: Get from token
    AppExecFwk::BundleInfo bundleInfo;

    // Use GetBundleInfoV9 with HAP_MODULE flag to get hapModuleInfos
    constexpr int32_t GET_BUNDLE_INFO_FLAGS =
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Calling GetBundleInfoV9 with flags=%{public}d", GET_BUNDLE_INFO_FLAGS);
    auto ret = bundleMgrHelper->GetBundleInfoV9(bundleName, GET_BUNDLE_INFO_FLAGS, bundleInfo, userId);
    if (ret != ABC_EXEC_ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: GetBundleInfoV9 failed for '%{public}s', ret=%{public}d",
            bundleName.c_str(), ret);
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] GetBundleInfoV9 succeeded, hapModuleInfos.size()=%{public}zu",
        bundleInfo.hapModuleInfos.size());

    // Set hap path from module info
    if (!bundleInfo.hapModuleInfos.empty()) {
        contextData.hapPath = bundleInfo.hapModuleInfos[0].hapPath;
        if (contextData.moduleName.empty()) {
            contextData.moduleName = bundleInfo.hapModuleInfos[0].moduleName;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Bundle info: hapPath='%{public}s', moduleName='%{public}s'",
            contextData.hapPath.c_str(), contextData.moduleName.c_str());
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] WARNING: Bundle has no hapModuleInfos");
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
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] FindHspModulePath: hspBundleName='%{public}s', hspModuleName='%{public}s'",
        hspBundleName.c_str(), hspModuleName.c_str());

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to get BundleMgrHelper");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] FindHspModulePath: BundleMgrHelper obtained");

    int32_t userId = 0;
    AppExecFwk::BundleInfo bundleInfo;

    // Get HSP bundle info - use GetBundleInfoV9 with HAP_MODULE flag
    constexpr int32_t GET_BUNDLE_INFO_FLAGS =
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] FindHspModulePath: Calling GetBundleInfoV9 for HSP bundle");
    auto ret = bundleMgrHelper->GetBundleInfoV9(hspBundleName, GET_BUNDLE_INFO_FLAGS, bundleInfo, userId);
    if (ret != ABC_EXEC_ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: GetBundleInfoV9 failed for HSP '%{public}s', ret=%{public}d",
            hspBundleName.c_str(), ret);
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] HSP has %{public}zu module(s)", bundleInfo.hapModuleInfos.size());

    // Find the matching module
    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Checking module: moduleName='%{public}s', hapPath='%{public}s'",
            hapModuleInfo.moduleName.c_str(), hapModuleInfo.hapPath.c_str());

        if (hapModuleInfo.moduleName == hspModuleName) {
            // Found the module
            // Return the hapPath (directory), RuntimeLinker will find abc file
            hapPath = hapModuleInfo.hapPath;
            className = hspModuleName;

            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Found HSP module:");
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   hapPath: '%{public}s'", hapPath.c_str());
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   className: '%{public}s'", className.c_str());
            return true;
        }
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Module '%{public}s' not found in HSP '%{public}s'",
        hspModuleName.c_str(), hspBundleName.c_str());
    return false;
}

int32_t ArktsExecutor::ExecuteAbc(const AbcExecuteParam &param)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc START ==========");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ArktsExecutor::ExecuteAbc called");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Input parameters:");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   abcPath: '%{public}s'", param.abcPath.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   hspBundleName: '%{public}s'", param.hspBundleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   hspModuleName: '%{public}s'", param.hspModuleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   functionName: '%{public}s'", param.functionName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   bundleName: '%{public}s'", param.bundleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   moduleName: '%{public}s'", param.moduleName.c_str());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   callerTokenId: %{public}u", param.callerTokenId);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   args count: %{public}zu", param.args.size());

    // Validate parameters
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Validating parameters...");
    if (!param.IsValid()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Invalid parameters - need either abcPath or hspBundleName+hspModuleName");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (INVALID_PARAM) ==========");
        return ABC_EXEC_ERR_INVALID_PARAM;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Parameters validated successfully");

    // Determine execution path
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Determining execution path...");
    std::string execPath;    // Path to pass to abc_executor
    std::string className;    // Class name (module name)

    if (param.UseHspLookup()) {
        // Mode 2: Lookup hap path from HSP info
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Using HSP lookup mode (query bundle manager)");
        if (!FindHspModulePath(param.hspBundleName, param.hspModuleName, execPath, className)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to find HSP module");
            TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (INVALID_PATH) ==========");
            return ABC_EXEC_ERR_INVALID_PATH;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] HSP module resolved: execPath='%{public}s', className='%{public}s'",
            execPath.c_str(), className.c_str());
    } else {
        // Mode 1: Use provided abc path directly
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Using direct abc path mode");
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
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Extracted className='%{public}s' from abcPath", className.c_str());
    }

    // Validate path
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Validating execPath...");
    if (execPath.empty() || execPath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Invalid path - must be absolute path (starting with '/')");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (INVALID_PATH) ==========");
        return ABC_EXEC_ERR_INVALID_PATH;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Path validated successfully");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Final execPath='%{public}s', className='%{public}s'", execPath.c_str(), className.c_str());


    // Get caller tokenId
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Getting caller tokenId...");
    uint32_t callerTokenId = param.callerTokenId;
    if (callerTokenId == 0) {
        callerTokenId = IPCSkeleton::GetCallingTokenID();
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Using IPC calling tokenId: %{public}u", callerTokenId);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Using provided callerTokenId: %{public}u", callerTokenId);
    }

    // Get bundle name from token if not provided
    std::string bundleName = param.bundleName;
    if (bundleName.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] bundleName not provided, querying from tokenId...");
        Security::AccessToken::HapTokenInfo hapInfo;
        auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(callerTokenId, hapInfo);
        if (ret != ABC_EXEC_ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to get hap token info, ret=%{public}d", ret);
            TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (INVALID_PARAM) ==========");
            return ABC_EXEC_ERR_INVALID_PARAM;
        }
        bundleName = hapInfo.bundleName;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Retrieved bundleName from token: '%{public}s'", bundleName.c_str());
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Using provided bundleName: '%{public}s'", bundleName.c_str());
    }

    // Build ContextData
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Building ContextData...");
    ContextData contextData;
    contextData.bundleName = bundleName;
    contextData.moduleName = param.moduleName;
    contextData.tokenId = callerTokenId;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ContextData: bundleName='%{public}s', moduleName='%{public}s', tokenId=%{public}u",
        contextData.bundleName.c_str(), contextData.moduleName.c_str(), contextData.tokenId);

    // Query additional bundle info
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Querying bundle info from BundleManager...");
    QueryBundleInfo(bundleName, contextData);
    if (!contextData.hapPath.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Bundle info queried: hapPath='%{public}s'", contextData.hapPath.c_str());
    }

    // Serialize context data
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Serializing context data...");
    std::string serializedContext = contextData.Serialize();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Serialized context size: %{public}zu bytes", serializedContext.size());


    // Create pipe for passing context data
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Creating pipe for context data transmission...");
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: pipe() failed: errno=%{public}d (%{public}s)", errno, strerror(errno));
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (PIPE_FAILED) ==========");
        return ABC_EXEC_ERR_FAILED;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Pipe created: pipefd[0]=%{public}d (read), pipefd[1]=%{public}d (write)", pipefd[0], pipefd[1]);

    // Fork child process
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Forking child process to run abc_executor...");
    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: fork() failed: errno=%{public}d (%{public}s)", errno, strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (FORK_FAILED) ==========");
        return ABC_EXEC_ERR_FORK_FAILED;
    }

    if (pid == 0) {
        // Child process
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD PROCESS: Entered child (pid=%{public}d)", getpid());

        // Close write end, keep read end
        close(pipefd[1]);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Closed write end of pipe, keeping read end (fd=%{public}d)", pipefd[0]);

        // Set environment variables
        char fdStr[32];
        snprintf(fdStr, sizeof(fdStr), "%d", pipefd[0]);
        setenv(ENV_CONTEXT_FD, fdStr, 1);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Set env %s=%s", ENV_CONTEXT_FD, fdStr);

        char tokenIdStr[32];
        snprintf(tokenIdStr, sizeof(tokenIdStr), "%u", callerTokenId);
        setenv(ENV_CALLER_TOKEN_ID, tokenIdStr, 1);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Set env %s=%s", ENV_CALLER_TOKEN_ID, tokenIdStr);

        if (!bundleName.empty()) {
            setenv(ENV_BUNDLE_NAME, bundleName.c_str(), 1);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Set env %s=%s", ENV_BUNDLE_NAME, bundleName.c_str());
        }

        if (!param.moduleName.empty()) {
            setenv(ENV_MODULE_NAME, param.moduleName.c_str(), 1);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Set env %s=%s", ENV_MODULE_NAME, param.moduleName.c_str());
        }

        if (!param.functionName.empty()) {
            setenv(ENV_FUNCTION_NAME, param.functionName.c_str(), 1);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Set env %s=%s", ENV_FUNCTION_NAME, param.functionName.c_str());
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

        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Executing %{public}s with %{public}zu args",
            ABC_EXECUTOR_PATH, param.args.size());
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: Command line: %{public}s %{public}s ...",
            ABC_EXECUTOR_PATH, execPath.c_str());
        execvp(ABC_EXECUTOR_PATH, argv.data());

        // execvp only returns if it fails
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] CHILD: ERROR: execvp() failed: errno=%{public}d (%{public}s)", errno, strerror(errno));
        _exit(1);
    }

    // Parent process
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Child process forked successfully, child pid=%{public}d", pid);

    // Close read end, write context data
    close(pipefd[0]);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Closed read end of pipe");

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Writing context data to child (%{public}zu bytes)...", serializedContext.size());
    ssize_t written = write(pipefd[1], serializedContext.data(), serializedContext.size());
    close(pipefd[1]);

    if (written < 0 || static_cast<size_t>(written) != serializedContext.size()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: WARNING: Failed to write complete context data, written=%{public}zd, errno=%{public}d",
            written, errno);
        // Continue waiting for child to exit
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Context data written successfully");
    }

    // Wait for child to complete
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Waiting for child pid=%{public}d to complete...", pid);

    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: ERROR: waitpid() failed: errno=%{public}d (%{public}s)", errno, strerror(errno));
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (WAIT_FAILED) ==========");
        return ABC_EXEC_ERR_FAILED;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Child process exited, waitpid returned");

    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: Child exited normally with code: %{public}d", exitCode);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc END (SUCCESS, exitCode=%{public}d) ==========", exitCode);
        return exitCode == 0 ? ABC_EXEC_ERR_OK : exitCode;
    }

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: ERROR: Child terminated by signal: %{public}d (%{public}s)",
            signal, strsignal(signal));
        TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (TERMINATED_BY_SIGNAL) ==========");
        return ABC_EXEC_ERR_FAILED;
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] PARENT: ERROR: Child exited abnormally (unknown reason)");
    TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (ABNORMAL_EXIT) ==========");
    return ABC_EXEC_ERR_FAILED;
}

} // namespace CliTool
} // namespace OHOS
