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

#include "skill_execute_manager.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

IMPLEMENT_DELAYED_SINGLETON(SkillExecuteManager)

void SkillExecuteRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "skill execute recipient died, skillId:%{public}" PRIu64 "", skillId_);
    DelayedSingleton<SkillExecuteManager>::GetInstance()->RemoteDied(skillId_);
}

int32_t SkillExecuteManager::RegisterSkillExecution(const sptr<IRemoteObject> &callerToken,
    const std::string &bundleName, uint64_t &skillId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "register skill execution, bundleName:%{public}s", bundleName.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    skillId = ++skillIdCount_;
    auto record = std::make_shared<SkillExecuteRecord>();
    record->callerToken = callerToken;
    record->skillId = skillId;
    record->bundleName = bundleName;
    record->state = SkillExecuteState::EXECUTING;

    if (callerToken != nullptr) {
        record->deathRecipient = sptr<IRemoteObject::DeathRecipient>(
            new SkillExecuteRecipient(skillId));
        callerToken->AddDeathRecipient(record->deathRecipient);
    }

    records_[skillId] = record;
    return ERR_OK;
}

int32_t SkillExecuteManager::GenerateSkillWant(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, const std::shared_ptr<WantParams> &params,
    uint64_t skillId, Want &want, const std::string &srcEntrance,
    const std::string &hapPath, bool isEsModule)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "generate skill want, bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    want.SetElementName(bundleName, moduleName, "");
    SkillExecuteParam::WriteToWant(want, bundleName, moduleName, skillName, params, skillId,
        srcEntrance, hapPath, isEsModule);
    return ERR_OK;
}

int32_t SkillExecuteManager::ExecuteSkillDone(uint64_t skillId, const SkillExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "execute skill done, skillId:%{public}" PRIu64 " code:%{public}d", skillId, result.code);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(skillId);
    if (it == records_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "skillId not found:%{public}" PRIu64 "", skillId);
        return ERR_INVALID_VALUE;
    }
    it->second->state = SkillExecuteState::EXECUTE_DONE;
    return ERR_OK;
}

int32_t SkillExecuteManager::RemoteDied(uint64_t skillId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "remote died, skillId:%{public}" PRIu64 "", skillId);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(skillId);
    if (it == records_.end()) {
        return ERR_INVALID_VALUE;
    }
    it->second->state = SkillExecuteState::REMOTE_DIED;
    return ERR_OK;
}

int32_t SkillExecuteManager::GetBundleName(uint64_t skillId, std::string &bundleName) const
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(skillId);
    if (it == records_.end()) {
        return ERR_INVALID_VALUE;
    }
    bundleName = it->second->bundleName;
    return ERR_OK;
}

} // namespace AAFwk
} // namespace OHOS
