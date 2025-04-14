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
#include "ability_delegator_registry.h"
#include "sts_ability_monitor.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorSts {
using namespace OHOS::AbilityRuntime;
STSAbilityMonitor::STSAbilityMonitor(const std::string &abilityName)
    : IAbilityMonitor(abilityName), abilityName_(abilityName)
{}

STSAbilityMonitor::STSAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : IAbilityMonitor(abilityName), abilityName_(abilityName), moduleName_(moduleName)
{}

void STSAbilityMonitor::OnSTSAbilityStart(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityCreate");
    CallLifecycleCBFunction("onAbilityCreate", abilityObj);
}

void STSAbilityMonitor::OnSTSAbilityForeground(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityForeground");
    CallLifecycleCBFunction("onAbilityForeground", abilityObj);
}

void STSAbilityMonitor::OnSTSAbilityBackground(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityBackground");
    CallLifecycleCBFunction("onAbilityBackground", abilityObj);
}

void STSAbilityMonitor::OnSTSAbilityDestroy(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityDestroy");
    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);
}

void STSAbilityMonitor::OnSTSWindowStageCreate(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageCreate");
    CallLifecycleCBFunction("onWindowStageCreate", abilityObj);
}

void STSAbilityMonitor::OnSTSWindowStageRestore(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageRestore");
    CallLifecycleCBFunction("onWindowStageRestore", abilityObj);
}

void STSAbilityMonitor::OnSTSWindowStageDestroy(const std::weak_ptr<STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageDestroy");
    CallLifecycleCBFunction("onWindowStageDestroy", abilityObj);
}

void STSAbilityMonitor::SetSTSAbilityMonitor(ani_vm *aniVM, ani_object &abilityMonitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called SetStsAbilityMonitor");
    vm_ = aniVM;
    ani_env *aniEnv = GetAniEnv();
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }

    stsAbilityMonitor_ = std::make_unique<STSNativeReference>();
    ani_ref objRef = nullptr;
    if (aniEnv->GlobalReference_Create(abilityMonitorObj, &objRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed");
        return;
    }
    stsAbilityMonitor_->aniObj = abilityMonitorObj;
    stsAbilityMonitor_->aniRef = objRef;
}

void STSAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCBFunction, name: %{public}s start", functionName.c_str());
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return;
    }

    if (abilityObj.expired()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null stsAbilityMonitor");
        return;
    }

    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }

    ani_status status = ANI_OK;
    ani_class cls = nullptr;
    if ((status = env->FindClass("Lapplication/AbilityMonitor/AbilityMonitorInner;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "status : %{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, functionName.c_str(), nullptr, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod status : %{public}d", status);
        return;
    }

    auto lockedPtr = abilityObj.lock();
    if (lockedPtr && (status = env->Object_CallMethod_Void(
        reinterpret_cast<ani_object>(stsAbilityMonitor_->aniRef), method,
        reinterpret_cast<ani_object>(lockedPtr->aniRef))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void_V status : %{public}d", status);
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCBFunction, name: %{public}s end", functionName.c_str());
}

ani_env* STSAbilityMonitor::GetAniEnv()
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
