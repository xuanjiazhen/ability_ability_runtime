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

#include "ets_interop_ability_monitor.h"

#include "ability_delegator_infos.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorEts {

EtsInteropAbilityMonitor::EtsInteropAbilityMonitor(ani_env *env, ani_object callback,
    const std::string &abilityName)
    : IInteropAbilityMonitor(abilityName)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return;
    }
    vm_ = aniVM;
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null callback");
        return;
    }
    ani_status status = env->GlobalReference_Create(callback, &callbackRef_);
    if (status != ANI_OK || callbackRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed: %{public}d", status);
    }
}

EtsInteropAbilityMonitor::EtsInteropAbilityMonitor(ani_env *env, ani_object callback,
    const std::string &abilityName, const std::string &moduleName)
    : IInteropAbilityMonitor(abilityName, moduleName)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetVM failed");
        return;
    }
    vm_ = aniVM;
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null callback");
        return;
    }
    ani_status status = env->GlobalReference_Create(callback, &callbackRef_);
    if (status != ANI_OK || callbackRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GlobalReference_Create failed: %{public}d", status);
    }
}

EtsInteropAbilityMonitor::~EtsInteropAbilityMonitor()
{
    if (vm_ == nullptr || callbackRef_ == nullptr) {
        return;
    }
    ani_env *env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
        env->GlobalReference_Delete(callbackRef_);
    }
    callbackRef_ = nullptr;
}

ani_env *EtsInteropAbilityMonitor::GetAniEnv()
{
    if (vm_ == nullptr) {
        return nullptr;
    }
    ani_env *env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        return nullptr;
    }
    return env;
}

ani_ref EtsInteropAbilityMonitor::GetAniRefFromProperty(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    auto baseProperty = ability.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "expired property");
        return nullptr;
    }
    using PropertyType = AppExecFwk::BaseDelegatorAbilityProperty::PropertyType;
    if (baseProperty->propertyType != PropertyType::ETS) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "cross-language property, passing null to callback");
        return nullptr;
    }
    auto etsProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(baseProperty);
    auto etsRef = etsProperty->object_.lock();
    if (etsRef) {
        return etsRef->aniRef;
    }
    TAG_LOGE(AAFwkTag::DELEGATOR, "expired ets object");
    return nullptr;
}

void EtsInteropAbilityMonitor::CallLifecycleCBFunction(const char *methodName, ani_ref abilityRef)
{
    ani_env *env = GetAniEnv();
    if (env == nullptr || callbackRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or callback");
        return;
    }

    ani_status status = ANI_OK;
    ani_object monitorObj = reinterpret_cast<ani_object>(callbackRef_);
    ani_ref funRef;
    status = env->Object_GetPropertyByName_Ref(monitorObj, methodName, &funRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetPropertyByName failed: %{public}d", status);
        return;
    }
    ani_boolean isNull = false;
    ani_boolean isUndefined = false;
    env->Reference_IsNullishValue(funRef, &isNull);
    env->Reference_IsUndefined(funRef, &isUndefined);
    if (isUndefined || isNull) {
        return;
    }
    ani_fn_object onFn = reinterpret_cast<ani_fn_object>(funRef);
    ani_ref result;
    if (abilityRef != nullptr) {
        std::vector<ani_ref> argv = { abilityRef };
        env->FunctionalObject_Call(onFn, 1, argv.data(), &result);
    } else {
        env->FunctionalObject_Call(onFn, 0, nullptr, &result);
    }
}

void EtsInteropAbilityMonitor::OnAbilityCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityCreate", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityForeground", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityBackground", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnAbilityDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityDestroy", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageCreate", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageRestore", GetAniRefFromProperty(ability));
}

void EtsInteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageDestroy", GetAniRefFromProperty(ability));
}

}  // namespace AbilityDelegatorEts
}  // namespace OHOS
