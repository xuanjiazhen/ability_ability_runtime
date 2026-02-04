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

#include "fa_ability_register.h"
#include "fa_ability_thread.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

extern "C" {

/**
 * @brief 导出函数：创建FAAbilityThread实例
 * 此函数由fa_ability_loader通过dlsym调用
 * @return FAAbilityThread的智能指针
 */
sptr<AbilityThread> CreateFAAbilityThread()
{
    TAG_LOGD(AAFwkTag::ABILITY, "CreateFAAbilityThread called");
    return sptr<AbilityRuntime::FAAbilityThread>::MakeSptr();
}

}  // extern "C"

}  // namespace AppExecFwk
}  // namespace OHOS
