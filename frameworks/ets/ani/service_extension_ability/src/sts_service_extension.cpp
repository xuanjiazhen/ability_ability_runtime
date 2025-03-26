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

#include "sts_service_extension.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "js_service_extension_context.h"
#include "ani_remote_object.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

void OnDisconnectPromiseCallback(ani_env* env, ani_object aniObj)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "OnDisconnectPromiseCallback");
    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long disconnectCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "disconnectCallbackPoint",
        &disconnectCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    auto *callbackInfo =
        reinterpret_cast<OHOS::AppExecFwk::AbilityTransactionCallbackInfo<> *>(disconnectCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    OHOS::AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
}

void PromiseCallback(ani_env* env, ani_object aniObj)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "PromiseCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long destroyCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "destroyCallbackPoint", &destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<> *>(destroyCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
}

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

StsServiceExtension* StsServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new StsServiceExtension(static_cast<STSRuntime&>(*runtime));
}

StsServiceExtension::StsServiceExtension(STSRuntime& stsRuntime) : stsRuntime_(stsRuntime) {}
StsServiceExtension::~StsServiceExtension()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
}

void StsServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
}
#ifdef SUPPORT_GRAPHICS
void StsServiceExtension::SystemAbilityStatusChangeListener::OnAddSystemAbility(int32_t systemAbilityId,
    const std::string& deviceId)
{
    std::array functions = {
        ani_native_function { "nativeOnDestroyCallback", ":V", reinterpret_cast<void*>(PromiseCallback) },
    };
    ani_status status = ANI_ERROR;
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        return;
    }
    if ((status = env->Class_BindNativeMethods(stsAbilityObj_->aniCls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
    }
}
#endif //SUPPORT_GRAPHICS

void StsServiceExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");

    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found Ability.sts");
        return;
    }

    ani_ref wantRef = WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }

    const char* signature = "L@ohos/app/ability/Want/Want;:V";
    CallObjectMethod(false, "onCreate", signature, wantRef);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void StsServiceExtension::OnStop()
{
    ServiceExtension::OnStop();
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    const char* signature = ":V";
    CallObjectMethod(false, "onDestroy", signature);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void StsServiceExtension::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Begin");

    SERVICE_EXT::OnStop();
    std::weak_ptr<SERVICE_EXT> weakPtr = shared_from_this();
    auto asyncCallback = [abilityWeakPtr = weakPtr]() {
        auto ability = abilityWeakPtr.lock();
        if (ability == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null ability");
            return;
        }
        ability->OnStopCallback();
    };
    callbackInfo->Push(asyncCallback);

    ani_long destroyCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr || stsAbilityObj_ == nullptr) {
        isAsyncCallback = false;
        OnStop();
        return;
    }
    if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "destroyCallbackPoint", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
    }
    if ((status = env->Object_SetField_Long(stsAbilityObj_->aniObj, field, destroyCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
    }

    isAsyncCallback = CallObjectMethod(true, "callOnDestroy", ":Z");
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "callOnDestroy isAsyncCallback: %{public}d", isAsyncCallback);
    if (!isAsyncCallback) {
        OnStopCallback();
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

sptr<IRemoteObject> StsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found Ability.sts");
        return nullptr;
    }
    ani_ref wantRef = WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return nullptr;
    }
    const char* signature = "L@ohos/app/ability/Want/Want;:V";
    CallObjectMethod(false, "onConnect", signature, wantRef);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return nullptr;
}
void StsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found Ability.sts");
        return;
    }
    ani_ref wantRef = WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    const char* signature  = "L@ohos/app/ability/Want/Want;:V";
    CallObjectMethod(false, "onDisconnect", signature, wantRef);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void StsServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found");
        return;
    }
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnDisconnect(want);
        return;
    }
    ani_long disconnectCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    ani_class cls = nullptr;
    if ((status = env->FindClass("L@ohos/app/ability/ServiceExtensionAbility/ServiceExtensionAbility;", &cls))
        != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    ani_object object = nullptr;
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "disconnectCallbackPoint", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Long(object, field, disconnectCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    CallObjectMethod(true, "callOnDisconnect", "L@ohos/app/ability/Want/Want;:Z");
}

void StsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto env = stsRuntime_.GetAniEnv();
    if (!env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "env not found Ability.sts");
        return;
    }
    ani_ref wantRef = WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
    }
    ani_int iStartId = static_cast<ani_int>(startId);
    const char* signature  = "L@ohos/app/ability/Want/Want;I:V";
    CallObjectMethod(false, "onRequest", signature, wantRef, iStartId);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return;
}

bool StsServiceExtension::HandleInsightIntent(const AAFwk::Want &want)
{
    return true;
}

bool StsServiceExtension::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo &executorInfo)
{
    return true;
}

bool StsServiceExtension::OnInsightIntentExecuteDone(uint64_t intentId,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    return true;
}

bool StsServiceExtension::CallObjectMethod(bool withResult, const char* name, const char* signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGI(AAFwkTag::UIABILITY, "StsUIAbility call sts, name: %{public}s", name);
    if (stsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null stsAbilityObj");
        return false;
    }
    auto env = stsRuntime_.GetAniEnv();
    auto obj = stsAbilityObj_->aniObj;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return false;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "get '%{public}s' from ability object failed", name);
        return false;
    }
    if (withResult) {
        ani_boolean res = 0;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(obj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        }
        va_end(args);
        return res;
    }
    int64_t timeStart = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(obj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
    }
    va_end(args);
    int64_t timeEnd = AbilityRuntime::TimeUtil::SystemTimeMillisecond();
    TAG_LOGI(AAFwkTag::UIABILITY, "end, name: %{public}s, time: %{public}s", name,
        std::to_string(timeEnd - timeStart).c_str());
    return false;
}

void StsServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
}

void StsServiceExtension::ConfigurationUpdated()
{
}

void StsServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
}

#ifdef SUPPORT_GRAPHICS
void StsServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "enter");
}

void StsServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "exit");
}

void StsServiceExtension::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId,
    float density, Rosen::DisplayOrientation orientation)
{
}

void StsServiceExtension::OnChange(Rosen::DisplayId displayId)
{
}
#endif
} // AbilityRuntime
} // OHOS