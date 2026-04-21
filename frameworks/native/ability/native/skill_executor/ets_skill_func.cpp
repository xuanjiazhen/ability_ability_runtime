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

#include "ets_skill_func.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

std::shared_ptr<EtsSkillFunc> EtsSkillFunc::Create(JsRuntime &runtime)
{
    return std::make_shared<EtsSkillFunc>(runtime);
}

EtsSkillFunc::EtsSkillFunc(JsRuntime &runtime) : JsSkillFunc(runtime)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ETS skill func created");
}

} // namespace AbilityRuntime
} // namespace OHOS
