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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_MGR_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_MGR_H

#include <map>
#include <mutex>
#include <string>

#include "skill_executor.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {

class SkillExecutorMgr : public std::enable_shared_from_this<SkillExecutorMgr> {
    DECLARE_DELAYED_SINGLETON(SkillExecutorMgr)

public:
    bool ExecuteSkill(Runtime &runtime, const SkillExecutorInfo &skillInfo,
        std::unique_ptr<SkillExecutorAsyncCallback> callback);

    static void TriggerCallbackInner(
        std::unique_ptr<SkillExecutorAsyncCallback> callback, int32_t errCode);

private:
    void RemoveSkillExecutor(uint64_t skillId);

    std::mutex mutex_;
    std::map<uint64_t, std::shared_ptr<SkillExecutor>> skillExecutors_;
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_MGR_H
