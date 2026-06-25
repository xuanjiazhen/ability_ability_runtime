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
 * @param bundleName Bundle name to query
 * @param contextData Output context data with hapPath and moduleName
 * @return true if query succeeded, false otherwise
 *
 * Note: This is optional context information for abc_executor.
 * Failure is not critical for HSP mode execution.
 */
static bool QueryBundleInfo(const std::string &bundleName, ContextData &contextData)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] QueryBundleInfo ENTRY: bundleName='%{public}s'", bundleName.c_str());

    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] bundleName is empty, skipping query");
        return false;
    }

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
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Calling GetBundleInfoV9: bundle='%{public}s', flags=%{public}d, userId=%{public}d",
        bundleName.c_str(), GET_BUNDLE_INFO_FLAGS, userId);

    auto ret = bundleMgrHelper->GetBundleInfoV9(bundleName, GET_BUNDLE_INFO_FLAGS, bundleInfo, userId);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] GetBundleInfoV9 returned=%{public}d", ret);

    if (ret != ABC_EXEC_ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: GetBundleInfoV9 failed for '%{public}s', ret=%{public}d",
            bundleName.c_str(), ret);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Possible causes:");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   1. Bundle not installed");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   2. Bundle name incorrect");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   3. Permission denied");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] GetBundleInfoV9 succeeded");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Bundle: name='%{public}s', hapModuleInfos.size()=%{public}zu",
        bundleInfo.name.c_str(), bundleInfo.hapModuleInfos.size());

    // Set hap path from module info
    if (!bundleInfo.hapModuleInfos.empty()) {
        contextData.hapPath = bundleInfo.hapModuleInfos[0].hapPath;
        if (contextData.moduleName.empty()) {
            contextData.moduleName = bundleInfo.hapModuleInfos[0].moduleName;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Bundle info extracted:");
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   hapPath='%{public}s'", contextData.hapPath.c_str());
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   moduleName='%{public}s'", contextData.moduleName.c_str());
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] WARNING: Bundle has no hapModuleInfos");
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] QueryBundleInfo EXIT: success=true");
    return true;
}

/**
 * @brief Find hap module path for HSP
 * @param callerBundleName The caller's bundle name (app that depends on the HSP)
 * @param hspBundleName HSP bundle name to find (e.g., "com.example.library")
 * @param hspModuleName HSP module name (e.g., "library")
 * @param hapPath Output hap module path (the directory containing abc files)
 * @param className Output class name (module name without .abc)
 * @return true if found
 *
 * Uses GetBaseSharedBundleInfos with callerBundleName to query dependent HSPs,
 * then matches the target hspBundleName and hspModuleName from the results.
 * This returns the hapPath directory, the RuntimeLinker will find the abc file.
 */
static bool FindHspModulePath(const std::string &callerBundleName,
    const std::string &hspBundleName, const std::string &hspModuleName,
    std::string &hapPath, std::string &className)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] FindHspModulePath ENTRY");
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Input: callerBundleName='%{public}s', hspBundleName='%{public}s', hspModuleName='%{public}s'",
        callerBundleName.c_str(), hspBundleName.c_str(), hspModuleName.c_str());

    if (callerBundleName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: callerBundleName is empty");
        return false;
    }

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to get BundleMgrHelper");
        return false;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] BundleMgrHelper obtained successfully");

    // Use GetBaseSharedBundleInfos to query dependent HSPs of the caller
    std::vector<AppExecFwk::BaseSharedBundleInfo> baseSharedBundleInfos;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Calling GetBaseSharedBundleInfos for caller '%{public}s'",
        callerBundleName.c_str());

    // Use GET_ALL_DEPENDENT_BUNDLE_INFO to get all dependent HSP modules
    auto ret = bundleMgrHelper->GetBaseSharedBundleInfos(
        callerBundleName,
        baseSharedBundleInfos,
        AppExecFwk::GetDependentBundleInfoFlag::GET_ALL_DEPENDENT_BUNDLE_INFO);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] GetBaseSharedBundleInfos returned=%{public}d", ret);

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: GetBaseSharedBundleInfos failed for caller '%{public}s', ret=%{public}d",
            callerBundleName.c_str(), ret);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Possible causes:");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   1. Caller app not installed");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   2. Caller does not depend on any HSP");
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   3. HSP not installed");
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] GetBaseSharedBundleInfos succeeded, found %{public}zu dependent bundle(s)",
        baseSharedBundleInfos.size());

    // Find the matching HSP module in the dependent bundles
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Searching for HSP: bundle='%{public}s', module='%{public}s'",
        hspBundleName.c_str(), hspModuleName.c_str());
    for (size_t i = 0; i < baseSharedBundleInfos.size(); ++i) {
        const auto &info = baseSharedBundleInfos[i];
        TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Dependent[%{public}zu]: bundle='%{public}s', module='%{public}s', hapPath='%{public}s'",
            i, info.bundleName.c_str(), info.moduleName.c_str(), info.hapPath.c_str());

        // Match the target HSP bundleName and moduleName
        if (info.bundleName == hspBundleName && info.moduleName == hspModuleName) {
            // Found the matching HSP module
            hapPath = info.hapPath;
            className = hspModuleName;

            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] SUCCESS: Found HSP module");
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   hapPath: '%{public}s'", hapPath.c_str());
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   className: '%{public}s'", className.c_str());
            return true;
        }
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: HSP '%{public}s/%{public}s' not found in caller's dependencies",
        hspBundleName.c_str(), hspModuleName.c_str());
    TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Available dependent bundles:");
    for (size_t i = 0; i < baseSharedBundleInfos.size(); ++i) {
        const auto &info = baseSharedBundleInfos[i];
        TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER]   [%{public}zu] bundle='%{public}s', module='%{public}s'",
            i, info.bundleName.c_str(), info.moduleName.c_str());
    }
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

        // Get caller's bundleName for HSP lookup
        std::string callerBundleName = param.bundleName;
        if (callerBundleName.empty()) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Getting caller's bundleName for HSP lookup...");
            uint32_t callerTokenId = param.callerTokenId;
            if (callerTokenId == 0) {
                callerTokenId = IPCSkeleton::GetCallingTokenID();
            }
            Security::AccessToken::HapTokenInfo hapInfo;
            auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(callerTokenId, hapInfo);
            if (ret != ABC_EXEC_ERR_OK) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] ERROR: Failed to get hap token info for HSP lookup, ret=%{public}d", ret);
                TAG_LOGE(AAFwkTag::CLI_TOOL, "========== [EXECUTOR_LAYER] ExecuteAbc EXIT (INVALID_PARAM) ==========");
                return ABC_EXEC_ERR_INVALID_PARAM;
            }
            callerBundleName = hapInfo.bundleName;
            TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Retrieved caller bundleName: '%{public}s'", callerBundleName.c_str());
        }

        if (!FindHspModulePath(callerBundleName, param.hspBundleName, param.hspModuleName, execPath, className)) {
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

    // Query additional bundle info (optional - for context)
    TAG_LOGI(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] Querying bundle info from BundleManager...");
    if (!QueryBundleInfo(bundleName, contextData)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] WARNING: Failed to query bundle info for caller '%{public}s', continuing without it",
            bundleName.c_str());
        TAG_LOGW(AAFwkTag::CLI_TOOL, "[EXECUTOR_LAYER] This is non-critical for HSP mode, execution will continue");
    }
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
