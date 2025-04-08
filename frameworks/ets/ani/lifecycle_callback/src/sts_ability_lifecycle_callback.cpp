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

std::optional<std::pair<ani_class, ani_object>> STSAbilityLifecycleCallback::InitializeCallbackObject(
    const std::string &functionName)
{
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return std::nullopt;
    }
    ani_status status = ANI_OK;
    env_->DescribeError();
    env_->ResetError();
    ani_class cls = nullptr;
    if ((status = env_->FindClass("L@ohos/app/ability/AbilityLifecycleCallback/AbilityLifecycleCallbackInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "status : %{public}d", status);
        return std::nullopt;
    }
    ani_method method = nullptr;
    status = env_->Class_FindMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "call Class_FindMethod ctor failed");
        return std::nullopt;
    }
    ani_object obj = nullptr;
    status = env_->Object_New(cls, method, &obj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "call Object_New obj failed");
        return std::nullopt;
    }
    return std::make_pair(cls, obj);
}
void STSAbilityLifecycleCallback::CallLifecycleCallbackOptionalFunction(
    const std::string &functionName,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    if (!abilityObj) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is nullptr");
        return;
    }
    auto context = InitializeCallbackObject(functionName);
    if (!context) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "InitializeCallbackObject failed");
        return;
    }
    auto [cls, obj] = *context;
    ani_status status = ANI_OK;
    ani_ref fn_ref = nullptr;
    if ((status = env_->Object_GetPropertyByName_Ref(obj, functionName.c_str(), &fn_ref)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "call Object_GetPropertyByName_Ref failed");
        return;
    }
    std::vector<ani_ref> vec;
    vec.push_back(abilityObj->aniRef);
    if (windowStageObj) {
        vec.push_back(windowStageObj->aniRef);
    }
    ani_ref fnReturnVal = nullptr;
    if ((status = env_->FunctionalObject_Call(static_cast<ani_fn_object>(fn_ref), vec.size(), vec.data(),
        &fnReturnVal)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "call FunctionalObject_Call failed");
        return;
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCallbackOptionalFunction, name: %{public}s end", functionName.c_str());
}

void STSAbilityLifecycleCallback::CallLifecycleCallbackFunction(
    const std::string &functionName,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    if (!abilityObj) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "abilityObj is nullptr");
        return;
    }
    auto context = InitializeCallbackObject(functionName);
    if (!context) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "InitializeCallbackObject failed");
        return;
    }
    auto [cls, obj] = *context;
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env_->Class_FindMethod(cls, functionName.c_str(), nullptr, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod status : %{public}d", status);
        return;
    }
    status = windowStageObj
        ? env_->Object_CallMethod_Void(obj, method, abilityObj->aniObj, windowStageObj->aniObj)
        : env_->Object_CallMethod_Void(obj, method, abilityObj->aniObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed, status: %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "CallLifecycleCallbackFunction, name: %{public}s end", functionName.c_str());
}

void STSAbilityLifecycleCallback::OnAbilityCreate(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityCreate");
    CallLifecycleCallbackFunction("onAbilityCreate", abilityObj);
}

void STSAbilityLifecycleCallback::OnWindowStageCreate(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageCreate");
    CallLifecycleCallbackFunction("onWindowStageCreate", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWindowStageActive(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
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

void STSAbilityLifecycleCallback::OnWindowStageDestroy(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageDestroy");
    CallLifecycleCallbackFunction("onWindowStageDestroy", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnAbilityDestroy(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityDestroy");
    CallLifecycleCallbackFunction("onAbilityDestroy", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityForeground(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityForeground");
    CallLifecycleCallbackFunction("onAbilityForeground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityBackground(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityBackground");
    CallLifecycleCallbackFunction("onAbilityBackground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityContinue(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityContinue");
    CallLifecycleCallbackFunction("onAbilityContinue", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillCreate(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillCreate");
    CallLifecycleCallbackOptionalFunction("onAbilityWillCreate", abilityObj);
}

void STSAbilityLifecycleCallback::OnWindowStageWillCreate(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageWillCreate");
    CallLifecycleCallbackOptionalFunction("onWindowStageWillCreate", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWillNewWant(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWillNewWant");
    CallLifecycleCallbackOptionalFunction("onWillNewWant", abilityObj);
}

void STSAbilityLifecycleCallback::OnNewWant(const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnNewWant");
    CallLifecycleCallbackOptionalFunction("onNewWant", abilityObj);
}

void STSAbilityLifecycleCallback::OnWindowStageWillDestroy(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageWillDestroy");
    CallLifecycleCallbackOptionalFunction("onWindowStageWillDestroy", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillDestroy(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillDestroy");
    CallLifecycleCallbackOptionalFunction("onAbilityWillDestroy", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillForeground(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillForeground");
    CallLifecycleCallbackOptionalFunction("onAbilityWillForeground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillBackground(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillBackground");
    CallLifecycleCallbackOptionalFunction("onAbilityWillBackground", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillContinue(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillContinue");
    CallLifecycleCallbackOptionalFunction("onAbilityWillContinue", abilityObj);
}

void STSAbilityLifecycleCallback::OnWindowStageWillRestore(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageWillRestore");
    CallLifecycleCallbackOptionalFunction("onWindowStageWillRestore", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnWindowStageRestore(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj,
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &windowStageObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnWindowStageRestore");
    CallLifecycleCallbackOptionalFunction("onWindowStageRestore", abilityObj, windowStageObj);
}

void STSAbilityLifecycleCallback::OnAbilityWillSaveState(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilityWillSaveState");
    CallLifecycleCallbackOptionalFunction("onAbilityWillSaveState", abilityObj);
}

void STSAbilityLifecycleCallback::OnAbilitySaveState(
    const std::shared_ptr<AbilityRuntime::STSNativeReference> &abilityObj)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "called OnAbilitySaveState");
    CallLifecycleCallbackOptionalFunction("onAbilitySaveState", abilityObj);
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS