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

#ifndef OHOS_AGENT_RUNTIME_AGENT_MODULE_CHECKER_H
#define OHOS_AGENT_RUNTIME_AGENT_MODULE_CHECKER_H

#include "module_checker_delegate.h"

namespace OHOS {
namespace AgentRuntime {

/**
 * @brief Agent module load checker. check whether module can be loaded in agent
 *
 */
class AgentModuleChecker : public ModuleCheckerDelegate {
public:
    AgentModuleChecker() = default;
    ~AgentModuleChecker() override = default;

    /**
 * @brief Check whether the module is allowed to be loaded in agent
     * @param moduleName module name
     * @return return true if the module can be loaded, false can not be loaded
     */
    bool CheckModuleLoadable(const char* moduleName,
        std::unique_ptr<ApiAllowListChecker>& apiAllowListChecker, bool isAppModule) override;

    bool DiskCheckOnly() override;
};

} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_MODULE_CHECKER_H
