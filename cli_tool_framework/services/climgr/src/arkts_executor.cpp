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

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FAILED = -1;

// Path to the abc_executor binary
constexpr const char *ABC_EXECUTOR_PATH = "/system/bin/abc_executor";
} // namespace

int32_t ArktsExecutor::ExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecuteAbc: %{public}s, args=%{public}zu, bundleName=%{public}s",
             abcPath.c_str(), args.size(), bundleName.c_str());

    // Validate path
    if (abcPath.empty() || abcPath[0] != '/') {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid abc path: must be absolute path");
        return ERR_INVALID_PATH;
    }

    // Validate bundle name
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "bundleName is empty");
        return ERR_INVALID_PARAM;
    }

    // Fork child process
    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "fork failed: %{public}d", errno);
        return ERR_FAILED;
    }

    if (pid == 0) {
        // Child process: exec abc_executor
        // Build argv array for execvp
        // argv[0] = program name, argv[1] = abcPath, argv[2] = bundleName, argv[3..] = args
        std::vector<char *> argv;
        argv.reserve(args.size() + 4);  // +4 for: program, abcPath, bundleName, nullptr

        argv.push_back(const_cast<char *>(ABC_EXECUTOR_PATH));
        argv.push_back(const_cast<char *>(abcPath.c_str()));
        argv.push_back(const_cast<char *>(bundleName.c_str()));

        for (const auto &arg : args) {
            argv.push_back(const_cast<char *>(arg.c_str()));
        }
        argv.push_back(nullptr);

        TAG_LOGI(AAFwkTag::CLI_TOOL, "Child process executing: %{public}s", ABC_EXECUTOR_PATH);
        execvp(ABC_EXECUTOR_PATH, argv.data());

        // execvp only returns if it fails
        TAG_LOGE(AAFwkTag::CLI_TOOL, "execvp failed: %{public}d", errno);
        _exit(1);
    }

    // Parent process: wait for child to complete
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Child process started with pid: %{public}d", pid);

    int status;
    pid_t result = waitpid(pid, &status, 0);
    if (result < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "waitpid failed: %{public}d", errno);
        return ERR_FAILED;
    }

    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Child exited with code: %{public}d", exitCode);
        // Map abc_executor exit codes to our error codes
        // 0 = success, 1+ = various errors
        return exitCode == 0 ? ERR_OK : ERR_FAILED;
    }

    if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Child terminated by signal: %{public}d", signal);
        return ERR_FAILED;
    }

    TAG_LOGE(AAFwkTag::CLI_TOOL, "Child exited abnormally");
    return ERR_FAILED;
}

} // namespace CliTool
} // namespace OHOS
