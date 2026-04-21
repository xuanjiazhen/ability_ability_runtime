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

namespace OHOS {
namespace AAFwk {

class SkillExecuteManager {
DECLARE_DELAYED_SINGLETON(SkillExecuteManager)
public:
    int32_t GenerateSkillWant(const std::string &bundleName, const std::string &moduleName,
        const std::string &skillName, const std::shared_ptr<WantParams> &params,
        Want &want);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_EXECUTE_MANAGER_H
