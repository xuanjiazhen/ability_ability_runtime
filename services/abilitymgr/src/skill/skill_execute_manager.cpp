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

int32_t SkillExecuteManager::GenerateSkillWant(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, const std::shared_ptr<WantParams> &params,
    Want &want)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "generate skill want, bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    want.SetElementName(bundleName, moduleName, "");
    SkillExecuteParam::WriteToWant(want, bundleName, moduleName, skillName, params);
    return ERR_OK;
}

} // namespace AAFwk
} // namespace OHOS
