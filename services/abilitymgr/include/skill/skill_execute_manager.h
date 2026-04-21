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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H

#include <map>
#include "cpp/mutex.h"
#include "iremote_object.h"
#include "singleton.h"
#include "skill/skill_execute_param.h"
#include "skill/skill_execute_result.h"

namespace OHOS {
namespace AAFwk {

enum class SkillExecuteState {
    UNKNOWN = 0,
    EXECUTING,
    EXECUTE_DONE,
    REMOTE_DIED
};

struct SkillExecuteRecord {
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    uint64_t skillId = 0;
    std::string bundleName;
    SkillExecuteState state = SkillExecuteState::UNKNOWN;
};

class SkillExecuteRecipient : public IRemoteObject::DeathRecipient {
public:
    explicit SkillExecuteRecipient(uint64_t skillId) : skillId_(skillId) {}
    ~SkillExecuteRecipient() override = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
private:
    uint64_t skillId_ = 0;
};

class SkillExecuteManager {
DECLARE_DELAYED_SINGLETON(SkillExecuteManager)
public:
    int32_t RegisterSkillExecution(const sptr<IRemoteObject> &callerToken,
        const std::string &bundleName, uint64_t &skillId);

    int32_t GenerateSkillWant(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, const std::shared_ptr<WantParams> &params,
        uint64_t skillId, Want &want);

    int32_t ExecuteSkillDone(uint64_t skillId, const SkillExecuteResult &result);

    int32_t RemoteDied(uint64_t skillId);

    int32_t GetBundleName(uint64_t skillId, std::string &bundleName) const;

private:
    mutable ffrt::mutex mutex_;
    uint64_t skillIdCount_ = 0;
    std::map<uint64_t, std::shared_ptr<SkillExecuteRecord>> records_;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H
