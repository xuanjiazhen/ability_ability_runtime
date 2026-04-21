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

#include "skill_executor_mgr.h"

#include "hilog_tag_wrapper.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {

IMPLEMENT_DELAYED_SINGLETON(SkillExecutorMgr)

bool SkillExecutorMgr::ExecuteSkill(Runtime &runtime, const SkillExecutorInfo &skillInfo,
    std::unique_ptr<SkillExecutorAsyncCallback> callback)
{
    if (callback == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
        return false;
    }
    if (skillInfo.executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null executeParam");
        TriggerCallbackInner(std::move(callback), ERR_INVALID_VALUE);
        return false;
    }

    auto executor = SkillExecutor::Create(runtime);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create skill executor failed");
        TriggerCallbackInner(std::move(callback), ERR_INVALID_VALUE);
        return false;
    }

    if (!executor->Init(skillInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "init skill executor failed");
        TriggerCallbackInner(std::move(callback), ERR_INVALID_VALUE);
        return false;
    }

    uint64_t executorId = 0;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        executorId = ++executorIdCounter_;
        skillExecutors_[executorId] = executor;
    }

    auto asyncCb = [weak = weak_from_this(), executorId](SkillExecuteResult &result) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "skill executor callback, executorId:%{public}" PRIu64 "", executorId);
        auto executorMgr = weak.lock();
        if (executorMgr == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null executorMgr");
            return;
        }
        executorMgr->RemoveSkillExecutor(executorId);
    };
    callback->Push(asyncCb);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "execute skill, executorId:%{public}" PRIu64 "", executorId);
    bool isAsync = false;
    bool ret = executor->ExecuteSkill(skillInfo.executeParam, std::move(callback), isAsync);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "execute skill failed");
        return false;
    }
    return true;
}

void SkillExecutorMgr::RemoveSkillExecutor(uint64_t executorId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    skillExecutors_.erase(executorId);
}

void SkillExecutorMgr::TriggerCallbackInner(
    std::unique_ptr<SkillExecutorAsyncCallback> callback, int32_t errCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called, errCode:%{public}d", errCode);
    if (callback) {
        SkillExecuteResult result;
        result.code = errCode;
        callback->Call(result);
    }
    callback.reset();
}

} // namespace AbilityRuntime
} // namespace OHOS
