/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "sts_ability_stage_monitor.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorSts {
using namespace OHOS::AbilityRuntime;

STSAbilityStageMonitor::STSAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance)
    : IAbilityStageMonitor(moduleName, srcEntrance), moduleName_(moduleName), srcEntrance_(srcEntrance)
{}

void STSAbilityStageMonitor::SetSTSAbilityStageMonitor(ani_env *env, ani_object &monitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called SetStsAbilityMonitor");

    stsAbilityStageMonitor_ = std::make_unique<STSNativeReference>();
    ani_ref objRef = nullptr;
    if (env->GlobalReference_Create(monitorObj, &objRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed");
        return;
    }

    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return;
    }
    vm_ = aniVM;
    stsAbilityStageMonitor_->aniObj = monitorObj;
    stsAbilityStageMonitor_->aniRef = objRef;
}
ani_env* STSAbilityStageMonitor::GetAniEnv()
{
    if (vm_ == nullptr) {
        return nullptr;
    }
    ani_env* aniEnv = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &aniEnv) != ANI_OK) {
        return nullptr;
    }
    return aniEnv;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
