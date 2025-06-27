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

#include "ets_ui_service_extension_context.h"

#include <chrono>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_runtime/js_caller_complex.h"

#include "ani_common_start_options.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_remote_object.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "start_options.h"
#include "sts_data_struct_converter.h"
#include "sts_error_utils.h"
#include "sts_free_install_observer.h"
#include "sts_runtime.h"
#include "ui_service_extension.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
// constexpr int32_t INDEX_ZERO = 0;
// constexpr int32_t INDEX_ONE = 1;
// constexpr int32_t INDEX_TWO = 2;
constexpr int32_t ERR_FAILURE = -1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr int32_t ERROR_CODE_TWO = 2;
// constexpr size_t ARGC_ZERO = 0;
// constexpr size_t ARGC_ONE = 1;
// constexpr size_t ARGC_TWO = 2;
// constexpr size_t ARGC_THREE = 3;
// constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);

struct ConnectionKey {
    Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};
static std::mutex g_connectsMutex;
static std::map<ConnectionKey, sptr<EtsUIServiceExtensionConnection>, key_compare> g_connects;
// static int64_t g_serialNumber = 0;

class EtsUIServiceExtensionContext final {
public:
    explicit EtsUIServiceExtensionContext(
        const std::shared_ptr<UIServiceExtensionContext>& context) : context_(context) {}
    ~EtsUIServiceExtensionContext() = default;

    static std::shared_ptr<UIServiceExtensionContext> GetContext();
    static void StartAbility(ani_env *env, ani_object callback, ani_object aniWant, ani_object aniStartOption);
    static void TerminateSelf(ani_env *env, ani_object callback);
    static void StartAbilityByType(ani_env *env, ani_string aniType, ani_object aniWantParam,
        ani_object abilityStartCallback, ani_object callback);
    static ani_double ConnectServiceExtensionAbility(ani_env *env, ani_object aniWant, ani_object aniOptions);
    static void DisConnectServiceExtensionAbility(ani_env *env, ani_double aniConnectionId, ani_object callback);
private:
    static void InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions);
    static bool CheckConnectionParam(napi_env env, ani_object object,
        sptr<EtsUIServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId = -1) const;
    static void RemoveConnection(int64_t connectId);
    static void FindConnection(AAFwk::Want &want, sptr<EtsUIServiceExtensionConnection> &connection, int64_t &connectId,
        int32_t &accountId) const;

    std::weak_ptr<UIServiceExtensionContext> context_;
    static std::unique_ptr<EtsUIServiceExtensionContext> instance_;

}; // EtsUIServiceExtensionContext

std::unique_ptr<EtsUIServiceExtensionContext> EtsUIServiceExtensionContext::instance_ = nullptr;

std::shared_ptr<UIServiceExtensionContext> EtsUIServiceExtensionContext::GetContext()
{
    if (instance_) {
        std::shared_ptr<UIServiceExtensionContext> context = nullptr;
        if (instance_->context_.expired() && (context = instance_->context_.lock()) != nullptr) {
            return context;
        }
    }
    return nullptr;
}

void EtsUIServiceExtensionContext::StartAbility(
    ani_env *env, ani_object callback, ani_object aniWant, ani_object aniStartOption)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "StartAbility Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWant  failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniStartOption, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "Failed to check undefined status : %{public}d", status);
        return;
    }
    AAFwk::StartOptions startOptions;
    if (!isUndefined) {
        if (!AppExecFwk::UnwrapStartOptions(env, aniStartOption, startOptions)) {
            TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapStartOptions filed");
            AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param startOptions failed.");
            return;
        }
    }
#ifdef SUPPORT_SCREEN
    InitDisplayId(want, startOptions);
#endif
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(
                    AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
        return;
    }
    innerErrCode = static_cast<int32_t>(context->StartAbility(want, startOptions));
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbility innerErrCode: %{public}d", innerErrCode);
    if (innerErrCode == static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(
                    AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
            nullptr);
    }
}

#ifdef SUPPORT_SCREEN
void EtsUIServiceExtensionContext::InitDisplayId(AAFwk::Want &want, AAFwk::StartOptions &startOptions)
{
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        return;
    }
    auto window = context->GetWindow();
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null window");
        return;
    }
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "startOption displayId %{public}d", startOptions.GetDisplayID());
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "window displayId %{public}" PRIu64, window->GetDisplayId());
    startOptions.SetDisplayID(window->GetDisplayId());
}
#endif

void EtsUIServiceExtensionContext::TerminateSelf(ani_env *env, ani_object callback)
{
    TAG_LOGI(AAFwkTag::UISERVC_EXT, "TerminateSelf Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);

    auto context = GetContext();
    if (!context) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(ERROR_CODE_ONE)),
            nullptr);
        return;
    }
    innerErrCode= context->TerminateSelf();
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "TerminateSelf innerErrCode: %{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
        nullptr);
}

void EtsUIServiceExtensionContext::StartAbilityByType(ani_env *env, ani_string aniType,
    ani_object aniWantParam, ani_object abilityStartCallback, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "StartAbilityByType Called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return;
    }
    std::string type;
    if (!OHOS::AppExecFwk::GetStdString(env, aniType, type)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "GetStdString Failed");
        AbilityRuntime::ThrowStsError(env, "Incorrect parameter types, param type must be a string");
        return;
    }

    AAFwk::WantParams wantParam;
    if (!AppExecFwk::UnwrapWantParams(env, aniWantParam, wantParam)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "parse wantParam failed");
        ThrowStsError(env, INVALID_PARAM, "Parameter error. The type of \"WantParams\" must be array");
        return;
    }

    std::shared_ptr<StsUIExtensionCallback> callback = std::make_shared<StsUIExtensionCallback>(env);
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "get aniVM failed");
        return;
    }
    callback->SetStsCallbackObject(aniVM, abilityStartCallback);
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (!context) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(
                    AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
        return;
    }
    innerErrCode = context->StartAbilityByType(type, wantParam, callback);
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityByType innerErrCode: %{public}d", innerErrCode);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
        nullptr);
}

ani_double EtsUIServiceExtensionContext::ConnectServiceExtensionAbility(ani_env *env, ani_object aniWant, ani_object aniConnectObj)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Connect ServiceExtensionAbility called.");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null env");
        return ERR_FAILURE;
    }
    // Unwrap want and connection
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "UnwrapWant  failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param want failed, must be a Want.");
        return ERR_FAILURE;
    }
    sptr<EtsUIServiceExtensionConnection> connection = new EtsUIServiceExtensionConnection(env);
    if (!CheckConnectionParam(env, aniConnectObj, connection, want)) {
        ThrowStsInvalidParamError(env, "Parse param options failed, must be a ConnectOptions.");
        return ERR_FAILURE;
    }
    int64_t connectId = connection->GetConnectionId();
    int32_t innerErrorCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(
                    AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)),
            nullptr);
        return ERR_FAILURE;
    }
    innerErrorCode = context->ConnectServiceExtensionAbility(want, connection);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "ConnectServiceExtensionAbility innerErrorCode: %{public}d", innerErrorCode);
    if (innerErrorCode != 0) {
        connection->CallJsFailed(innerErrorCode);
        RemoveConnection(connectId);
    }
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(innerErrorCode)),
        AppExecFwk::createDouble(env, innerErrorCode));
    return innerErrorCode;
}

bool EtsUIServiceExtensionContext::CheckConnectionParam(napi_env env, ani_object connectObj,
    sptr<EtsUIServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId = -1) const
{
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null connection");
        return false;
    }
    connection->SetEtsConnectionObject(connectObj);
    ConnectionKey key;
    {
        std::lock_guard guard(g_connectsMutex);
        key.id = g_serialNumber;
        key.want = want;
        key.accountId = accountId;
        connection->SetConnectionId(key.id);
        g_connects.emplace(key, connection);
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
    }
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Unable to find connection, make new one");
    return true;
}

void EtsUIServiceExtensionContext::RemoveConnection(int64_t connectId)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "RemoveConnection called");
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability exist.");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "remove conn ability not exist.");
    }
}

void EtsUIServiceExtensionContext::DisConnectServiceExtensionAbility(
    ani_env *env, ani_double aniConnectionId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnect ServiceExtensionAbility start");
    TAG_LOGD(AAFwkTag::APPMGR, "DisConnectServiceExtensionAbility aniConnectionId:%{public}f", aniConnectionId);
    int64_t connectionId = static_cast<int64_t>(aniConnectionId);
    AAFwk::Want want;
    sptr<EtsUIServiceExtensionConnection> connection = nullptr;
    int32_t accountId = -1;
    FindConnection(want, connection, connectionId, accountId);
    // begin disconnect
    int32_t innerErrCode = static_cast<int32_t>(ERR_OK);
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::UISERVC_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(ERROR_CODE_ONE)),
            nullptr);
        return;
    }
    if (!connection) {
        TAG_LOGW(AAFwkTag::UISERVC_EXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsError(env,
                static_cast<AbilityRuntime::AbilityErrorCode>(ERROR_CODE_TWO)),
            nullptr);
        return;
    }
    innerErrCode = context->DisConnectServiceExtensionAbility(want, connection, accountId);
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "DisConnectServiceExtensionAbility innerErrorCode: %{public}d", innerErrorCode);
     AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(innerErrCode)),
        nullptr);
}

void EtsUIServiceExtensionContext::FindConnection(
    AAFwk::Want &want, sptr<EtsUIServiceExtensionConnection> &connection, int64_t &connectId, int32_t &accountId) const
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        accountId = item->first.accountId;
        TAG_LOGD(AAFwkTag::UISERVC_EXT, "find conn ability exist");
    }
    return;
}
} // namespace

ani_object CreateEtsUIServiceExtensionContext(ani_env *env, std::shared_ptr<UIServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::UISERVC_EXT, "Call");
    // std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    // if (context) {
    //     abilityInfo = context->GetAbilityInfo();
    // }
    // napi_value object = CreateJsExtensionContext(env, context, abilityInfo);

    // std::unique_ptr<EtsUIServiceExtensionContext> jsUIContext =
    //     std::make_unique<EtsUIServiceExtensionContext>(context);
    // napi_wrap(env, object, jsUIContext.release(), EtsUIServiceExtensionContext::Finalizer, nullptr, nullptr);

    // const char *moduleName = "JsUIServiceExtensionContext";
    // BindNativeFunction(env, object, "startAbility", moduleName, EtsUIServiceExtensionContext::StartAbility);
    // BindNativeFunction(env, object, "terminateSelf", moduleName, EtsUIServiceExtensionContext::TerminateSelf);
    // BindNativeFunction(env, object, "startAbilityByType", moduleName,
    //     EtsUIServiceExtensionContext::StartAbilityByType);
    // BindNativeFunction(env, object, "connectServiceExtensionAbility", moduleName,
    //     EtsUIServiceExtensionContext::ConnectServiceExtensionAbility);
    // BindNativeFunction(env, object, "disconnectServiceExtensionAbility", moduleName,
    //     EtsUIServiceExtensionContext::DisConnectServiceExtensionAbility);
    return nullptr;
}

EtsUIServiceExtensionConnection::EtsUIServiceExtensionConnection(ani_env *env) : env_(env) {}

EtsUIServiceExtensionConnection::~EtsUIServiceExtensionConnection()
{
    // if (jsConnectionObject_ == nullptr) {
    //     return;
    // }

    // uv_loop_t *loop = nullptr;
    // napi_get_uv_event_loop(env_, &loop);
    // if (loop == nullptr) {
    //     return;
    // }

    // uv_work_t *work = new (std::nothrow) uv_work_t;
    // if (work == nullptr) {
    //     return;
    // }
    // work->data = reinterpret_cast<void *>(jsConnectionObject_.release());
    // int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
    // [](uv_work_t *work, int status) {
    //     if (work == nullptr) {
    //         return;
    //     }
    //     if (work->data == nullptr) {
    //         delete work;
    //         work = nullptr;
    //         return;
    //     }
    //     delete reinterpret_cast<NativeReference *>(work->data);
    //     work->data = nullptr;
    //     delete work;
    //     work = nullptr;
    // });
    // if (ret != 0) {
    //     delete reinterpret_cast<NativeReference *>(work->data);
    //     work->data = nullptr;
    //     delete work;
    //     work = nullptr;
    // }
}

void EtsUIServiceExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

int64_t EtsUIServiceExtensionConnection::GetConnectionId()
{
    return connectionId_;
}

void EtsUIServiceExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityConnectDone, resultCode:%{public}d", resultCode);
    // wptr<EtsUIServiceExtensionConnection> connection = this;
    // std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
    //     ([connection, element, remoteObject, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
    //         sptr<EtsUIServiceExtensionConnection> connectionSptr = connection.promote();
    //         if (!connectionSptr) {
    //             TAG_LOGE(AAFwkTag::UISERVC_EXT, "null connectionSptr");
    //             return;
    //         }
    //         connectionSptr->HandleOnAbilityConnectDone(element, remoteObject, resultCode);
    //     });

    // napi_ref callback = nullptr;
    // std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    // NapiAsyncTask::Schedule("EtsUIServiceExtensionConnection::OnAbilityConnectDone",
    //     env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void EtsUIServiceExtensionConnection::HandleOnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "resultCode:%{public}d", resultCode);
    // // wrap ElementName
    // napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);

    // // wrap RemoteObject
    // napi_value napiRemoteObject = NAPI_ohos_rpc_CreateJsRemoteObject(env_, remoteObject);
    // napi_value argv[] = {napiElementName, napiRemoteObject};
    // if (jsConnectionObject_ == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
    //     return;
    // }
    // napi_value obj = jsConnectionObject_->GetNapiValue();
    // if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object error");
    //     return;
    // }
    // napi_value methodOnConnect = nullptr;
    // napi_get_named_property(env_, obj, "onConnect", &methodOnConnect);
    // if (methodOnConnect == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null methodOnConnect");
    //     return;
    // }
    // napi_call_function(env_, obj, methodOnConnect, ARGC_TWO, argv, nullptr);
}

void EtsUIServiceExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    // wptr<EtsUIServiceExtensionConnection> connection = this;
    // std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
    //     ([connection, element, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
    //         sptr<EtsUIServiceExtensionConnection> connectionSptr = connection.promote();
    //         if (!connectionSptr) {
    //             TAG_LOGI(AAFwkTag::UISERVC_EXT, "null connectionSptr");
    //             return;
    //         }
    //         connectionSptr->HandleOnAbilityDisconnectDone(element, resultCode);
    //     });
    // napi_ref callback = nullptr;
    // std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    // NapiAsyncTask::Schedule("EtsUIServiceExtensionConnection::OnAbilityDisconnectDone",
    //     env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void EtsUIServiceExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "HandleOnAbilityDisconnectDone, resultCode:%{public}d", resultCode);
    // napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    // napi_value argv[] = {napiElementName};
    // if (jsConnectionObject_ == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
    //     return;
    // }
    // napi_value obj = jsConnectionObject_->GetNapiValue();
    // if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object error");
    //     return;
    // }

    // napi_value method = nullptr;
    // napi_get_named_property(env_, obj, "onDisconnect", &method);
    // if (method == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null method");
    //     return;
    // }

    // // release connect
    // {
    //     std::lock_guard guard(g_connectsMutex);
    //     TAG_LOGD(AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone g_connects.size:%{public}zu", g_connects.size());
    //     std::string bundleName = element.GetBundleName();
    //     std::string abilityName = element.GetAbilityName();
    //     auto item = std::find_if(g_connects.begin(),
    //         g_connects.end(),
    //         [bundleName, abilityName, connectionId = connectionId_](
    //             const auto &obj) {
    //             return (bundleName == obj.first.want.GetBundle()) &&
    //                 (abilityName == obj.first.want.GetElement().GetAbilityName()) &&
    //                 connectionId == obj.first.id;
    //         });
    //     if (item != g_connects.end()) {
    //         // match bundlename && abilityname
    //         g_connects.erase(item);
    //         TAG_LOGD(
    //             AAFwkTag::UISERVC_EXT, "OnAbilityDisconnectDone erase g_connects.size:%{public}zu", g_connects.size());
    //     }
    // }
    // napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
}

void EtsUIServiceExtensionConnection::SetEtsConnectionObject(ani_object etsConnectionObject)
{
//     napi_ref ref = nullptr;
//     napi_create_reference(env_, jsConnectionObject, 1, &ref);
//     jsConnectionObject_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void EtsUIServiceExtensionConnection::RemoveConnectionObject()
{
    etsConnectionObject_.reset();
}

void EtsUIServiceExtensionConnection::CallJsFailed(int32_t errorCode)
{
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallJsFailed begin");
    // if (jsConnectionObject_ == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null jsConnectionObject_");
    //     return;
    // }
    // napi_value obj = jsConnectionObject_->GetNapiValue();
    // if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "get object wrong");
    //     return;
    // }

    // napi_value method = nullptr;
    // napi_get_named_property(env_, obj, "onFailed", &method);
    // if (method == nullptr) {
    //     TAG_LOGE(AAFwkTag::UISERVC_EXT, "null method");
    //     return;
    // }
    // napi_value argv[] = {CreateJsValue(env_, errorCode)};
    // napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    // TAG_LOGD(AAFwkTag::UISERVC_EXT, "CallJsFailed end");
}
} // namespace AbilityRuntime
}  // namespace OHOS
