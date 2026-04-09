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

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(napi_env env, napi_value callback,
    const std::string &abilityName)
    : IInteropAbilityMonitor(abilityName), env_(env)
{
    if (env_ == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or callback");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, callback, 1, &ref);
    jsCallback_.reset(reinterpret_cast<NativeReference*>(ref));
}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(napi_env env, napi_value callback,
    const std::string &abilityName, const std::string &moduleName)
    : IInteropAbilityMonitor(abilityName, moduleName), env_(env)
{
    if (env_ == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or callback");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, callback, 1, &ref);
    jsCallback_.reset(reinterpret_cast<NativeReference*>(ref));
}

napi_value JsInteropAbilityMonitor::GetNapiValueFromProperty(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    auto baseProperty = ability.lock();
    if (!baseProperty) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "expired property");
        return nullptr;
    }
    using PropertyType = AppExecFwk::BaseDelegatorAbilityProperty::PropertyType;
    if (baseProperty->propertyType != PropertyType::JS) {
        TAG_LOGW(AAFwkTag::DELEGATOR, "cross-language property, passing null to callback");
        return nullptr;
    }
    auto jsProperty = std::static_pointer_cast<AppExecFwk::ADelegatorAbilityProperty>(baseProperty);
    auto jsRef = jsProperty->object_.lock();
    if (jsRef) {
        return jsRef->GetNapiValue();
    }
    TAG_LOGE(AAFwkTag::DELEGATOR, "expired js object");
    return nullptr;
}

void JsInteropAbilityMonitor::CallLifecycleCBFunction(const char *methodName,
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    if (env_ == nullptr || jsCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null env or callback");
        return;
    }
    napi_value obj = jsCallback_->GetNapiValue();
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null callback obj");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null method: %{public}s", methodName);
        return;
    }
    napi_value abilityVal = GetNapiValueFromProperty(ability);
    napi_value argv[] = { CreateJsNull(env_) };
    if (abilityVal != nullptr) {
        argv[0] = abilityVal;
    }
    napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void JsInteropAbilityMonitor::OnAbilityCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityCreate", ability);
}

void JsInteropAbilityMonitor::OnAbilityForeground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityForeground", ability);
}

void JsInteropAbilityMonitor::OnAbilityBackground(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityBackground", ability);
}

void JsInteropAbilityMonitor::OnAbilityDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityDestroy", ability);
}

void JsInteropAbilityMonitor::OnWindowStageCreate(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageCreate", ability);
}

void JsInteropAbilityMonitor::OnWindowStageRestore(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageRestore", ability);
}

void JsInteropAbilityMonitor::OnWindowStageDestroy(
    const std::weak_ptr<AppExecFwk::BaseDelegatorAbilityProperty> &ability)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageDestroy", ability);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
