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

#ifndef OHOS_ABILITY_RUNTIME_FA_ABILITY_REGISTER_H
#define OHOS_ABILITY_RUNTIME_FA_ABILITY_REGISTER_H

#include "ability_thread.h"

namespace OHOS {
namespace AppExecFwk {

extern "C" {
    /**
     * @brief 导出函数：创建FAAbilityThread实例
     * 此函数由fa_ability_loader通过dlsym调用
     * @return FAAbilityThread的智能指针
     */
    sptr<AbilityThread> CreateFAAbilityThread();
}

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_FA_ABILITY_REGISTER_H
