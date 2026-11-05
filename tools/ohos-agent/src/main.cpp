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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "cli_agent_connection.h"
#include "ohos_agent_command.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

using namespace OHOS;
constexpr uint32_t COMMAND_TIME_OUT = 60;

class CommandTimer {
public:
    CommandTimer(const std::string &timerName, uint32_t timeout, const std::string &operation)
    {
        // connect is a long-lived session: do NOT arm the 60s XCollie timer (would kill the session).
        // Other one-shot subcommands (e.g. help) keep the timer for hang protection.
        if (operation != "connect") {
            setTimer_ = true;
            timerId_ = HiviewDFX::XCollie::GetInstance().SetTimer("ability::ohos_agent_cli_command", timeout,
                nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_RECOVERY);
        }
    }
    ~CommandTimer()
    {
        if (setTimer_) {
            HiviewDFX::XCollie::GetInstance().CancelTimer(timerId_);
        }
    }

private:
    bool setTimer_ = false;
    int32_t timerId_ = 0;
};

int main(int argc, char* argv[])
{
    std::string operation;
    if (argc > 1) {
        operation = argv[1];
    }

    if (argc > 0 && strstr(argv[0], "ohos-agent") != nullptr) {
        CommandTimer commandTimer("ability::ohos_agent_cli_command", COMMAND_TIME_OUT, operation);
        OHOS::AAFwk::OhosAgentShellCommand cmd(argc, argv);
        cmd.CreateErrorInfoMap();
        // Bare shell (no cliMgr-injected ohos_cli_callerIdentity): print the pre-loop help/error
        // text. Under cliMgr stdout is reserved for session events, except a help-only request
        // (no session events are emitted in that run).
        std::string preLoopText = cmd.ExecCommand();
        bool bareShell = getenv("ohos_cli_callerIdentity") == nullptr;
        if (!preLoopText.empty() && (bareShell || cmd.IsHelpOnly())) {
            std::cout << preLoopText;
        }
        OHOS::AAFwk::DrainStdoutForExit();
        _exit(cmd.GetExitCode());
    }
    OHOS::AAFwk::DrainStdoutForExit();
    _exit(0);
}
