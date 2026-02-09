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

#include "fa_ability_thread.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace AbilityRuntime;

extern "C" {

/**
 * @brief Create a FAAbilityThread instance.
 * @return Returns a sptr<AbilityThread> pointing to FAAbilityThread.
 *
 * 注意：由于 sptr 是智能指针，直接返回值即可。
 * 调用方接收后会自动管理引用计数。
 */
sptr<AbilityThread> CreateFAAbilityThread()
{
    TAG_LOGI(AAFwkTag::ABILITY, "CreateFAAbilityThread called");
    auto thread = sptr<FAAbilityThread>::MakeSptr();
    return sptr<AbilityThread>(thread);
}

} // extern "C"
