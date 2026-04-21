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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_H

#include <memory>
#include <string>

#include "ability_transaction_callback_info.h"
#include "skill/skill_execute_result.h"
#include "skill_executor_info.h"

namespace OHOS {
namespace AppExecFwk {
struct SkillExecuteResult;
template <typename> class AbilityTransactionCallbackInfo;
} // namespace AppExecFwk
namespace AbilityRuntime {
class Runtime;

using SkillExecutorAsyncCallback =
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::SkillExecuteResult>;

class SkillExecutor {
public:
    static std::shared_ptr<SkillExecutor> Create(Runtime &runtime);
protected:
    SkillExecutor() = default;
public:
    SkillExecutor(const SkillExecutor &) = delete;
    SkillExecutor(SkillExecutor &&) = delete;
    SkillExecutor &operator=(const SkillExecutor &) = delete;
    SkillExecutor &operator=(SkillExecutor &&) = delete;
    virtual ~SkillExecutor() = default;

    virtual bool Init(const SkillExecutorInfo &skillInfo) = 0;

    virtual bool ExecuteSkill(
        std::shared_ptr<SkillExecuteParam> skillParam,
        std::unique_ptr<SkillExecutorAsyncCallback> callback,
        bool &isAsync) = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTOR_H
