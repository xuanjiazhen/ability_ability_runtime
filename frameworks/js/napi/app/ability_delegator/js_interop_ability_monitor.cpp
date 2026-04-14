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
#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

JsInteropAbilityMonitor::JsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : abilityName_(abilityName), moduleName_(moduleName)
{}

void JsInteropAbilityMonitor::OnAbilityCreate()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityCreate");
}

void JsInteropAbilityMonitor::OnAbilityForeground()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityForeground");
}

void JsInteropAbilityMonitor::OnAbilityBackground()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityBackground");
}

void JsInteropAbilityMonitor::OnAbilityDestroy()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onAbilityDestroy");
}

void JsInteropAbilityMonitor::OnWindowStageCreate()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageCreate");
}

void JsInteropAbilityMonitor::OnWindowStageRestore()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageRestore");
}

void JsInteropAbilityMonitor::OnWindowStageDestroy()
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    CallLifecycleCBFunction("onWindowStageDestroy");
}

void JsInteropAbilityMonitor::SetJsInteropAbilityMonitor(napi_env env, napi_value monitor)
{
    TAG_LOGD(AAFwkTag::DELEGATOR, "called");
    env_ = env;
    napi_ref ref = nullptr;
    napi_create_reference(env, monitor, 1, &ref);
    jsInteropMonitor_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
}

napi_value JsInteropAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName)
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

    // Cross-language: pass null for ability object since we cannot bridge ETS objects to JS
    napi_value argv[] = { CreateJsNull(env_) };
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    return callResult;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
