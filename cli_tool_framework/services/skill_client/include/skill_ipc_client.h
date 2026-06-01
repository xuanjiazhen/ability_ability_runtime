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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_IPC_CLIENT_H
#define OHOS_ABILITY_RUNTIME_SKILL_IPC_CLIENT_H

#include <mutex>
#include <string>

#include "iremote_object.h"
#include "refbase.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

class SkillCallbackLite;

class SkillIpcClient {
public:
    static SkillIpcClient &GetInstance();

    int32_t QuerySkillType(const std::string &bundleName,
        const std::string &moduleName, const std::string &skillName, int32_t &skillType);

    int32_t ExecuteInAppSkillWithTokenId(uint32_t callerTokenId,
        const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, const std::string &scriptPath,
        const std::string &functionName,
        const std::shared_ptr<AAFwk::WantParams> &skillArgs,
        const sptr<SkillCallbackLite> &callback);
    void ResetProxy(const wptr<IRemoteObject> &remote);

private:
    SkillIpcClient() = default;
    ~SkillIpcClient() = default;
    DISALLOW_COPY_AND_MOVE(SkillIpcClient);

    sptr<IRemoteObject> GetAbilityManagerProxy();
    bool WriteInterfaceToken(MessageParcel &data);

    static constexpr int32_t ABILITY_MGR_SERVICE_ID = 180;
    static constexpr uint32_t QUERY_SKILL_TYPE_CODE = 6171;
    static constexpr uint32_t EXECUTE_IN_APP_SKILL_CODE = 6173;

    std::mutex mutex_;
    sptr<IRemoteObject> proxy_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_IPC_CLIENT_H
