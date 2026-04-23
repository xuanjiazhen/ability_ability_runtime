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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H

#include <csignal>
#include <functional>
#include <map>
#include <memory>

#include "session_record.h"

namespace OHOS {
namespace CliTool {

class ProcessManager {
public:
    explicit ProcessManager();
    ~ProcessManager();

    static ProcessManager &GetInstance();

    bool SpawnProcess(std::string &cmdLine,
                      const std::string &sandboxConfig,
                      const std::map<std::string, std::string> &args,
                      const std::shared_ptr<SessionRecord> &record);

    void TerminateProcess(pid_t pid, int signal = SIGTERM);

private:
    ProcessManager() = default;
    ~ProcessManager() = default;

    bool CreatePipes(SessionRecord &record);
    void CloseAllPipes(SessionRecord &record);

    void ExecuteChildProcess(std::string &cmdLine,
                             const std::string &sandboxConfig,
                             const std::map<std::string, std::string> &args,
                             const std::shared_ptr<SessionRecord> &record);
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H