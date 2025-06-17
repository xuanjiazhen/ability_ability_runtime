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

#include "js_start_abilities_observer.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
 
namespace OHOS {
namespace AbilityRuntime {
JsStartAbilitiesObserver::JsStartAbilitiesObserver(napi_env env) : env_(env) {}

JsStartAbilitiesObserver::~JsStartAbilitiesObserver() = default;

void JsStartAbilitiesObserver::OnStartAbilitiesFinished(const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "OnStartAbilitiesFinished");
    wptr<JsStartAbilitiesObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, startTime, resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            sptr<JsStartAbilitiesObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr) {
                jsObserverSptr->HandleOnStartAbilitiesFinished(startTime, resultCode);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsStartAbilitiesObserver::OnStartAbilitiesFinished",
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsStartAbilitiesObserver::HandleOnStartAbilitiesFinished(const std::string &startTime, const int &resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "HandleOnStartAbilitiesFinished");
    // 为什么会是vector？
    std::vector<napi_deferred> promises;
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
            if (it->startTime != startTime) {
                it++;
                continue;
            }
            if (it->deferred != nullptr) {
                promises.emplace_back(it->deferred);
            }
            it = jsObserverObjectList_.erase(it);
        }
    }

    for (const napi_deferred& promise : promises) {
        CallPromise(promise, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartAbilities", atoi(startTime.c_str()));
    }
}

void JsStartAbilitiesObserver::CallPromise(napi_deferred deferred, int32_t resultCode)
{
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null deferred");
        return;
    }
    if (resultCode == ERR_OK) {
        napi_value value = CreateJsUndefined(env_);
        napi_resolve_deferred(env_, deferred, value);
    } else {
        napi_value error = CreateJsError(env_, GetJsErrorCodeByNativeError(resultCode));
        napi_reject_deferred(env_, deferred, error);
    }
}

void JsStartAbilitiesObserver::AddJsObserverObject(const std::string &startTime, napi_value* result)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call  AddJsObserverObject");
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
            if (it->startTime == startTime) {
                TAG_LOGW(AAFwkTag::FREE_INSTALL, "add jsObject");
                return;
            }
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartStartAbilities", atoi(startTime.c_str()));
    JsStartAbilitiesObserverObject object;
    object.startTime = startTime;
    napi_deferred deferred;
    napi_create_promise(env_, &deferred, result);
    object.deferred = deferred;
    std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
    jsObserverObjectList_.emplace_back(object);
}
} // namespace AbilityRuntime
} // namespace OHOS