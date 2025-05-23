/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_MGR_INTERFACE_H

#include <string>
#include <vector>

#include "mock_common.h"

namespace OHOS {
namespace AppExecFwk {
class IAppMgr : public IRemoteBroker {
public:
    virtual int32_t GetConfiguration(Configuration& config) = 0;

    virtual int32_t SignRestartAppFlag(int32_t uid, const std::string &instanceKey)
    {
        return 0;
    }

    virtual int32_t GetAllUIExtensionRootHostPid(pid_t pid, std::vector<pid_t> &hostPids)
    {
        return 0;
    }

    virtual int32_t GetAllUIExtensionProviderPid(pid_t hostPid, std::vector<pid_t> &providerPids)
    {
        return 0;
    }

    virtual void RestartResidentProcessDependedOnWeb() {}

    virtual int32_t KillAppSelfWithInstanceKey(const std::string &instanceKey, bool clearPageStack,
        const std::string& reason)
    {
        return 0;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif
