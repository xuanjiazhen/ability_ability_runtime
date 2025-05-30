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

#include "sts_query_erms_observer.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "sts_error_utils.h"
#include "sts_runtime.h"
#include "sts_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ATOMIC_SERVICE_STARTUP_Rule_IMPL_CLASS_NAME = "L@ohos/app/ability/abilityManager/AtomicServiceStartupRuleImpl;";
}
StsQueryERMSObserver::StsQueryERMSObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

StsQueryERMSObserver::~StsQueryERMSObserver() = default;

void StsQueryERMSObserver::OnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "OnQueryFinished");
    HandleOnQueryFinished(appId, startTime, rule, resultCode);
}

void StsQueryERMSObserver::HandleOnQueryFinished(const std::string &appId, const std::string &startTime,
    const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "HandleOnQueryFinished");
    std::vector<ani_object> callbacks;
    {
        std::unique_lock<std::mutex> lock(stsObserverObjectListLock_);
        for (auto it = stsObserverObjectList_.begin(); it != stsObserverObjectList_.end();) {
            if (it->appId != appId || it->startTime != startTime || it->deferred == nullptr) {
                it++;
                continue;
            }
            callbacks.emplace_back(it->deferred);
            it = stsObserverObjectList_.erase(it);
        }
    }
    for (auto callback : callbacks) {
        CallCallback(callback, rule, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    }
}

void StsQueryERMSObserver::CallCallback(ani_object callback, const AtomicServiceStartupRule &rule, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "CallCallback");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null etsVm");
        return;
    }
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null callback");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null env");
        return;
    }
    ani_object aniObject = CreateStsError(env, AbilityErrorCode::ERROR_OK);
    if (resultCode != ERR_OK) {
        aniObject = CreateStsErrorByNativeErr(env, resultCode);
    }
    ani_object ruleObj = nullptr;
    if(!WrapAtomicServiceStartupRule(env, rule, ruleObj)) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "WrapAtomicServiceStartupRule failed");
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, ruleObj);
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }
}

void StsQueryERMSObserver::AddStsObserverObject(const std::string &appId, const std::string &startTime,
    ani_object callBack)
{
    TAG_LOGD(AAFwkTag::QUERY_ERMS, "call");
    {
        std::unique_lock<std::mutex> lock(stsObserverObjectListLock_);
        for (auto it = stsObserverObjectList_.begin(); it != stsObserverObjectList_.end(); ++it) {
            if (it->appId == appId && it->startTime == startTime) {
                TAG_LOGW(AAFwkTag::QUERY_ERMS, "The stsObject has been added");
                return;
            }
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartQueryERMS", atoi(startTime.c_str()));
    StsQueryERMSObserverObject object;
    object.appId = appId;
    object.startTime = startTime;
    object.callBack = reinterpret_cast<ani_object>(global);

    std::unique_lock<std::mutex> lock(stsObserverObjectListLock_);
    stsObserverObjectList_.emplace_back(object);
}

bool StsQueryERMSObserver::WrapAtomicServiceStartupRule(ani_env *env,
    const AbilityRuntime::AtomicServiceStartupRule &rule, ani_object &ruleObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "null env");
        return false;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if((status = env->FindClass(ATOMIC_SERVICE_STARTUP_Rule_IMPL_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::QUERY_ERMS, "Find AtomicServiceStartupRuleImpl Class failed");
        return false;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null cls");
        return false;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return false;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null method");
        return false;
    }
    if ((status = env->Object_New(cls, method, &ruleObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return false;
    }
    if (ruleObj == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null ruleObj");
        return false;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Ref(ruleObj, "isOpenAllowed", rule.isOpenAllowed)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetProperty isOpenAllowed failed");
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Ref(ruleObj, "isEmbeddedAllowed", rule.isEmbeddedAllowed)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "SetProperty isEmbeddedAllowed failed");
        return false;
    }
    //WrapAtomicServiceStartupRuleInner(env, rule, ruleObj);
}
// bool StsQueryERMSObserver::WrapAtomicServiceStartupRuleInner(ani_env *env,
//     const AbilityRuntime::AtomicServiceStartupRule &rule, ani_object &ruleObj) 
// {

// }
} // namespace AbilityRuntime
} // namespace OHOS