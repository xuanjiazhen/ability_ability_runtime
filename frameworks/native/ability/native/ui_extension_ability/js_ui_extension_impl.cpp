/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_ui_extension_impl.h"
#include "hilog_tag_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
} // namespace

JsUIExtensionImpl::JsUIExtensionImpl(const std::unique_ptr<Runtime> &runtime)
    : JsUIExtensionBase(runtime)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl constructor");
}

JsUIExtensionImpl::~JsUIExtensionImpl()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl destructor");
}

sptr<IRemoteObject> JsUIExtensionImpl::OnConnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = NAPI_ohos_rpc_getNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "remoteObj is null");
    }
    return remoteObj;
}

void JsUIExtensionImpl::OnDisconnect(const AAFwk::Want &want)
{
    HandleScope handleScope(jsRuntime_);
    CallOnDisconnect(want, false);
}

napi_value JsUIExtensionImpl::CallOnConnect(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl CallOnConnect");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl null object");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl invalid object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl onConnect func not exist");
        return nullptr;
    }

    napi_value remoteNative = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (remoteNative == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl remote is nullptr");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl CallOnConnect end");
    return remoteNative;
}

napi_value JsUIExtensionImpl::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl CallOnDisconnect");
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl null object");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl invalid object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "JsUIExtensionImpl onDisconnect func not exist");
        return nullptr;
    }

    if (withResult) {
        napi_value result = nullptr;
        napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        return handleEscape.Escape(result);
    } else {
        napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "JsUIExtensionImpl CallOnDisconnect end");
}
} // namespace AbilityRuntime
} // namespace OHOS
