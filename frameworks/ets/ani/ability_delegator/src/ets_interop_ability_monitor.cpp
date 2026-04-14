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

#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityDelegatorEts {
using namespace OHOS::AbilityRuntime;

EtsInteropAbilityMonitor::EtsInteropAbilityMonitor(const std::string &abilityName)
    : IAbilityMonitor(abilityName), abilityName_(abilityName)
{}

EtsInteropAbilityMonitor::EtsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : IAbilityMonitor(abilityName), abilityName_(abilityName), moduleName_(moduleName)
{}

void EtsInteropAbilityMonitor::OnAbilityStart(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityStart");
    CallLifecycleCBFunction("onAbilityCreate");
}

void EtsInteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityForeground");
    CallLifecycleCBFunction("onAbilityForeground");
}

void EtsInteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityBackground");
    CallLifecycleCBFunction("onAbilityBackground");
}

void EtsInteropAbilityMonitor::OnAbilityStop(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityStop");
    CallLifecycleCBFunction("onAbilityDestroy");
}

void EtsInteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageCreate");
    CallLifecycleCBFunction("onWindowStageCreate");
}

void EtsInteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageRestore");
    CallLifecycleCBFunction("onWindowStageRestore");
}

void EtsInteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageDestroy");
    CallLifecycleCBFunction("onWindowStageDestroy");
}

void EtsInteropAbilityMonitor::SetEtsInteropAbilityMonitor(ani_env *env, ani_object &monitorObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called SetEtsInteropAbilityMonitor");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }
    etsInteropMonitor_ = std::make_unique<AppExecFwk::ETSNativeReference>();
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
    if (etsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null etsInteropMonitor_");
        return;
    }
    etsInteropMonitor_->aniObj = monitorObj;
    etsInteropMonitor_->aniRef = objRef;
}

void EtsInteropAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCBFunction, name: %{public}s start", functionName.c_str());
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return;
    }

    if (etsInteropMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null etsInteropMonitor_");
        return;
    }

    ani_env *env = GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env");
        return;
    }

    ani_status status = ANI_OK;
    ani_object monitorObj = reinterpret_cast<ani_object>(etsInteropMonitor_->aniRef);
    ani_ref funRef;
    status = env->Object_GetPropertyByName_Ref(monitorObj, functionName.c_str(), &funRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_GetPropertyByName_Ref failed status: %{public}d", status);
        return;
    }
    ani_boolean isValue = false;
    ani_boolean isUndefined = false;
    env->Reference_IsNullishValue(funRef, &isValue);
    env->Reference_IsUndefined(funRef, &isUndefined);
    if (isUndefined || isValue) {
        return;
    }
    ani_fn_object onFn = reinterpret_cast<ani_fn_object>(funRef);
    // Cross-language: pass undefined for ability object since we cannot bridge JS objects to ETS
    ani_ref undefinedRef;
    env->Reference_CreateUndefined(&undefinedRef);
    std::vector<ani_ref> argv = { undefinedRef };
    ani_ref result;
    if ((status = env->FunctionalObject_Call(onFn, 1, argv.data(), &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FunctionalObject_Call failed, status: %{public}d", status);
        return;
    }
}

ani_env *EtsInteropAbilityMonitor::GetAniEnv()
{
    if (vm_ == nullptr) {
        return nullptr;
    }
    ani_env *aniEnv = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &aniEnv) != ANI_OK) {
        return nullptr;
    }
    return aniEnv;
}
}  // namespace AbilityDelegatorEts
}  // namespace OHOS
