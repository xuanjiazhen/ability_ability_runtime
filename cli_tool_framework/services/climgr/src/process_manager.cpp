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

#include "process_manager.h"

#include <unistd.h>

#include "cli_error_code.h"
#include "exec_tool_param.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace CliTool {

ProcessManager &ProcessManager::GetInstance()
{
    static ProcessManager instance;
    return instance;
}

int32_t ProcessManager::CreateChildProcess(const ExecToolParam &param, const std::string &sandboxConfig,
    const std::map<std::string, std::string> &args) const
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "CreateChildProcess before fork: processUid=%{public}d, processPid=%{public}d, callingUid=%{public}d, "
        "callingPid=%{public}d, tokenId=%{public}u, fullTokenId=%{public}llu",
        getuid(), getpid(), IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid(),
        IPCSkeleton::GetCallingTokenID(), static_cast<unsigned long long>(IPCSkeleton::GetCallingFullTokenID()));

    pid_t pid = fork();
    if (pid < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to fork: %{public}d", errno);
        return ERR_NO_INIT;
    }

    if (pid == 0) {
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "CreateChildProcess child before exec: processUid=%{public}d, processPid=%{public}d, "
            "callingUid=%{public}d, callingPid=%{public}d, tokenId=%{public}u, fullTokenId=%{public}llu",
            getuid(), getpid(), IPCSkeleton::GetCallingUid(), IPCSkeleton::GetCallingPid(),
            IPCSkeleton::GetCallingTokenID(), static_cast<unsigned long long>(IPCSkeleton::GetCallingFullTokenID()));

        std::string clawSandbox = "/system/bin/claw_sandbox";
        std::string configPrompt = "--config";
        std::string cmdPrompt = "--cmd";
        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char *>(clawSandbox.c_str()));
        execArgs.push_back(const_cast<char *>(configPrompt.c_str()));
        execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
        execArgs.push_back(const_cast<char *>(cmdPrompt.c_str()));
        std::string cmdLine = param.toolName;
        if (!param.subcommand.empty()) {
            cmdLine += " " + param.subcommand;
        }
        for (const auto &[key, value] : args) {
            cmdLine += " " + key + " " + value;
        }
        execArgs.push_back(const_cast<char *>(cmdLine.c_str()));
        execArgs.push_back(nullptr);
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Before execvp");
        execvp(execArgs[0], execArgs.data());
        _exit(0);
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CreateChildProcess parent after fork: childPid=%{public}d", pid);
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
