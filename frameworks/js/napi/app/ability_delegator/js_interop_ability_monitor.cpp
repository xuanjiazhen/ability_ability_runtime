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

#include "js_interop_ability_monitor.h"

#include "ability_delegator_infos.h"
#include "ets_native_reference.h"
#include "hilog_tag_wrapper.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

typedef struct __hybridgref *hybridgref;

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;

namespace {
void SafeCopyStringProperty(napi_env env, napi_value src, napi_value dest, const char *name)
{
    napi_value value = nullptr;
    napi_status status = napi_get_named_property(env, src, name, &value);
    if (status != napi_ok || value == nullptr) {
        return;
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type != napi_string) {
        return;
    }
    napi_set_named_property(env, dest, name, value);
}

napi_value SafeToJSONCallback(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, &thisArg, nullptr);
    napi_value result = nullptr;
    napi_create_object(env, &result);
    if (thisArg == nullptr || result == nullptr) {
        return result;
    }
    SafeCopyStringProperty(env, thisArg, result, "abilityName");
    SafeCopyStringProperty(env, thisArg, result, "moduleName");
    SafeCopyStringProperty(env, thisArg, result, "name");
    SafeCopyStringProperty(env, thisArg, result, "context");
    return result;
}
}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : abilityName_(abilityName), moduleName_(moduleName)
{}

void JsInteropAbilityMonitor::OnAbilityCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityCreate", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityForeground", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityBackground", abilityObj);
}

void JsInteropAbilityMonitor::OnAbilityDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageCreate", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageRestore", abilityObj);
}

void JsInteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageDestroy", abilityObj);
}

void JsInteropAbilityMonitor::SetJsInteropAbilityMonitor(napi_env env, napi_value monitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    env_ = env;
    napi_ref ref = nullptr;
    napi_create_reference(env, monitor, 1, &ref);
    jsInteropMonitor_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

void JsInteropAbilityMonitor::SetAniEnv(void *aniEnv)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    aniEnvVoid_ = aniEnv;
}

void JsInteropAbilityMonitor::AttachSafeToJSON(napi_env env, napi_value value)
{
    if (env == nullptr || value == nullptr) {
        return;
    }
    napi_value toJSONFn = nullptr;
    napi_create_function(env, "toJSON", NAPI_AUTO_LENGTH, SafeToJSONCallback, nullptr, &toJSONFn);
    if (toJSONFn == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "create toJSON function failed");
        return;
    }
    napi_status status = napi_set_named_property(env, value, "toJSON", toJSONFn);
    if (status != napi_ok) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "set toJSON failed: %{public}d", status);
    }
}

napi_value JsInteropAbilityMonitor::ConvertAbilityToNapiValue(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    if (aniEnvVoid_ == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null aniEnvVoid_, fallback to null");
        return CreateJsNull(env_);
    }

    auto property = abilityObj.lock();
    if (property == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null property, fallback to null");
        return CreateJsNull(env_);
    }

    auto etsProperty = std::static_pointer_cast<AppExecFwk::EtsDelegatorAbilityProperty>(property);
    if (etsProperty == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null etsProperty, fallback to null");
        return CreateJsNull(env_);
    }

    auto etsRef = etsProperty->object_.lock();
    if (etsRef == nullptr || etsRef->aniRef == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "null etsRef, fallback to null");
        return CreateJsNull(env_);
    }

    auto *aniEnv = reinterpret_cast<ani_env *>(aniEnvVoid_);

    hybridgref href = nullptr;
    if (!hybridgref_create_from_ani(aniEnv, etsRef->aniRef, &href) || href == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "hybridgref_create_from_ani failed, fallback to null");
        return CreateJsNull(env_);
    }

    napi_value result = nullptr;
    bool success = hybridgref_get_napi_value(env_, href, &result);
    hybridgref_delete_from_ani(aniEnv, href);

    if (!success || result == nullptr) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "hybridgref_get_napi_value failed, fallback to null");
        return CreateJsNull(env_);
    }

    AttachSafeToJSON(env_, result);
    return result;
}

napi_value JsInteropAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &abilityObj)
{
    if (functionName.empty()) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "empty funcName");
        return nullptr;
    }

    if (!jsInteropMonitor_) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null jsInteropMonitor");
        return nullptr;
    }

    napi_value obj = jsInteropMonitor_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null obj");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, functionName.data(), &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null method: %{public}s", functionName.data());
        return nullptr;
    }

    napi_value abilityValue = ConvertAbilityToNapiValue(abilityObj);
    napi_value argv[] = { abilityValue };
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    return callResult;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
