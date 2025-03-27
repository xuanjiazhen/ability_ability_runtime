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
#include "sts_ability_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AbilityRuntime;

STSAbilityLifecycleCallback::STSAbilityLifecycleCallback(ani_env* env)
    : env_(env)
{}

void STSAbilityLifecycleCallback::CallLifecycleCallbackFunction(
    const std::string &functionName,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return;
    }
    ani_status status = ANI_OK;
    env_->DescribeError();
    env_->ResetError();
    ani_class cls = nullptr;
    if ((status = env_->FindClass("L@ohos/app/ability/AbilityLifecycleCallback/AbilityLifecycleCallbackInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "status : %{public}d", status);
        return;
    }
    ani_method method = nullptr;
    status = env_->Class_FindMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "call Class_FindMethod ctor failed");
        return;
    }
    ani_object obj = nullptr;
    status = env_->Object_New(cls, method, &obj);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "call Object_New obj failed");
        return;
    }

    method = nullptr;
    if ((status = env_->Class_FindMethod(cls, functionName.c_str(), nullptr, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod status : %{public}d", status);
        return;
    }
    if (windowStageObj) {
        if (abilityObj && (status = env_->Object_CallMethod_Void(obj, method, abilityObj->aniObj,
            windowStageObj->aniObj)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void_V status : %{public}d", status);
            return;
        }
    } else {
        if (abilityObj && (status = env_->Object_CallMethod_Void(obj, method, abilityObj->aniObj)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void_V status : %{public}d", status);
            return;
        }
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCBFunction, name: %{public}s end", functionName.c_str());
}

void STSAbilityLifecycleCallback::OnAbilityCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityCreate");
    CallLifecycleCallbackFunction("onAbilityCreate", abilityObj);
}

void STSAbilityLifecycleCallback::OnWindowStageCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageCreate");
    CallLifecycleCallbackFunction("onWindowStageCreate", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWindowStageActive(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageActive");
    CallLifecycleCallbackFunction("onWindowStageActive", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWindowStageInactive(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageInactive");
    CallLifecycleCallbackFunction("onWindowStageInactive", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWindowStageDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageDestroy");
    CallLifecycleCallbackFunction("onWindowStageDestroy", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnAbilityDestroy(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityDestroy");
    CallLifecycleCallbackFunction("onAbilityDestroy", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityForeground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityForeground");
    CallLifecycleCallbackFunction("onAbilityForeground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityBackground(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityBackground");
    CallLifecycleCallbackFunction("onAbilityBackground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityContinue(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityContinue");
    CallLifecycleCallbackFunction("onAbilityContinue", abilityObj);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS