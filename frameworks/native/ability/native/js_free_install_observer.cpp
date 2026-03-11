/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_free_install_observer.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
JsFreeInstallObserver::JsFreeInstallObserver(napi_env env) : env_(env) {}

JsFreeInstallObserver::~JsFreeInstallObserver() = default;

void JsFreeInstallObserver::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    wptr<JsFreeInstallObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, bundleName, abilityName, startTime, resultCode](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            AbilityRuntime::HandleScope handleScope(env);
            sptr<JsFreeInstallObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr) {
                jsObserverSptr->HandleOnInstallFinished(bundleName, abilityName, startTime, resultCode);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsFreeInstallObserver::OnInstallFinished", env_, std::make_unique<NapiAsyncTask>(callback,
        std::move(execute), std::move(complete)));
}

void JsFreeInstallObserver::OnInstallFinishedByUrl(const std::string &startTime, const std::string& url,
    int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    wptr<JsFreeInstallObserver> jsObserver = this;
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([jsObserver, startTime, url, resultCode](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            AbilityRuntime::HandleScope handleScope(env);
            sptr<JsFreeInstallObserver> jsObserverSptr = jsObserver.promote();
            if (jsObserverSptr) {
                jsObserverSptr->HandleOnInstallFinishedByUrl(startTime, url, resultCode);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsFreeInstallObserver::OnInstallFinished", env_, std::make_unique<NapiAsyncTask>(callback,
        std::move(execute), std::move(complete)));
}

void JsFreeInstallObserver::OnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, napi_value abilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    std::vector<napi_deferred> promises;
    std::vector<std::shared_ptr<NativeReference>> callbacks;
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
            if ((it->bundleName != bundleName) || (it->abilityName != abilityName) || (it->startTime != startTime)) {
                it++;
                continue;
            }
            if (it->callback == nullptr && it->deferred == nullptr) {
                it++;
                continue;
            }
            if (!it->isAbilityResult) {
                it++;
                continue;
            }
            if (it->deferred != nullptr) {
                promises.emplace_back(it->deferred);
            } else {
                callbacks.emplace_back(it->callback);
            }
            it = jsObserverObjectList_.erase(it);
            TAG_LOGD(AAFwkTag::FREE_INSTALL,
                "jsObserverObjectList_ size:%{public}zu", jsObserverObjectList_.size());
        }
    }

    for (const napi_deferred& promise : promises) {
        CallPromise(promise, abilityResult);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
    for (auto callback : callbacks) {
        CallCallback(callback, abilityResult);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
}

void JsFreeInstallObserver::HandleOnInstallFinished(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    std::vector<napi_deferred> promises;
    std::vector<std::shared_ptr<NativeReference>> callbacks;
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
            if ((it->bundleName != bundleName) || (it->abilityName != abilityName) || (it->startTime != startTime)) {
                it++;
                continue;
            }
            if (it->callback == nullptr && it->deferred == nullptr) {
                it++;
                continue;
            }
            if (it->isAbilityResult && resultCode == ERR_OK) {
                it++;
                continue;
            }
            if (it->deferred != nullptr) {
                promises.emplace_back(it->deferred);
            } else {
                callbacks.emplace_back(it->callback);
            }
            it = jsObserverObjectList_.erase(it);
        }
    }

    for (const napi_deferred& promise : promises) {
        CallPromise(promise, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
    for (auto callback : callbacks) {
        CallCallback(callback, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
}

void JsFreeInstallObserver::HandleOnInstallFinishedByUrl(const std::string &startTime, const std::string& url,
    int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "HandleOnInstallFinishedByUrl, startTime:%{public}s, url:%{public}s, resultCode:%{public}d",
             startTime.c_str(), url.c_str(), resultCode);
    DumpObservers();
    std::vector<napi_deferred> promises;
    std::vector<std::shared_ptr<NativeReference>> callbacks;
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "jsObserverObjectList_ size:%{public}zu", jsObserverObjectList_.size());
        int32_t matchCount = 0;
        int32_t skipCount = 0;
        int32_t index = 0;
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end();) {
            TAG_LOGD(AAFwkTag::FREE_INSTALL, "Checking observer[%{public}d]: startTime:%{public}s, url:%{public}s, callback:%{public}p, deferred:%{public}p, isAbilityResult:%{public}d",
                     index++, it->startTime.c_str(), it->url.c_str(), it->callback.get(), it->deferred, it->isAbilityResult);
            if ((it->startTime != startTime) || (it->url != url)) {
                TAG_LOGD(AAFwkTag::FREE_INSTALL, "Skip observer[%{public}zu], startTime match:%{public}d, url match:%{public}d",
                         std::distance(jsObserverObjectList_.begin(), it),
                         it->startTime == startTime, it->url == url);
                skipCount++;
                it++;
                continue;
            }
            if (it->callback == nullptr && it->deferred == nullptr) {
                TAG_LOGD(AAFwkTag::FREE_INSTALL, "Skip observer[%{public}zu]: both callback and deferred are null",
                         std::distance(jsObserverObjectList_.begin(), it));
                skipCount++;
                it++;
                continue;
            }
            if (it->isAbilityResult && (resultCode == ERR_OK ||
                resultCode == AAFwk::ATOMIC_SERVICE_MINIMIZED)) {
                TAG_LOGD(AAFwkTag::FREE_INSTALL, "Skip observer[%{public}zu]: isAbilityResult:%{public}d, resultCode:%{public}d, ERR_OK:%{public}d",
                         std::distance(jsObserverObjectList_.begin(), it), it->isAbilityResult, resultCode, ERR_OK);
                skipCount++;
                it++;
                continue;
            }
            TAG_LOGD(AAFwkTag::FREE_INSTALL, "Found matching observer[%{public}zu], deferred:%{public}p, callback:%{public}p",
                     std::distance(jsObserverObjectList_.begin(), it), it->deferred, it->callback.get());
            matchCount++;
            if (it->deferred != nullptr) {
                promises.emplace_back(it->deferred);
            } else {
                callbacks.emplace_back(it->callback);
            }
            it = jsObserverObjectList_.erase(it);
        }
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "After search: matches:%{public}d, skipped:%{public}d, promises:%{public}zu, callbacks:%{public}zu",
                 matchCount, skipCount, promises.size(), callbacks.size());
    }

    TAG_LOGD(AAFwkTag::FREE_INSTALL, "Calling %zu promises and %zu callbacks", promises.size(), callbacks.size());
    for (const napi_deferred& promise : promises) {
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "Calling CallPromise, promise:%{public}p, resultCode:%{public}d", promise, resultCode);
        CallPromise(promise, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
    for (auto callback : callbacks) {
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "Calling CallCallback, callback:%{public}p, resultCode:%{public}d", callback.get(), resultCode);
        CallCallback(callback, resultCode);
        FinishAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    }
}

void JsFreeInstallObserver::CallCallback(std::shared_ptr<NativeReference> callback, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    AbilityRuntime::HandleScope handleScope(env_);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null callback");
        return;
    }
    napi_value value;
    if (resultCode == ERR_OK || resultCode == AAFwk::ATOMIC_SERVICE_MINIMIZED) {
        value = CreateJsUndefined(env_);
    } else {
        value = CreateJsError(env_, GetJsErrorCodeByNativeError(resultCode));
    }
    napi_value argv[] = { value };
    napi_value func = callback->GetNapiValue();
    napi_call_function(env_, CreateJsUndefined(env_), func, ArraySize(argv), argv, nullptr);
}

void JsFreeInstallObserver::CallCallback(std::shared_ptr<NativeReference> callback, napi_value abilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    AbilityRuntime::HandleScope handleScope(env_);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null callback");
        return;
    }
    napi_value argv[] = {
        CreateJsError(env_, 0),
        abilityResult,
    };
    napi_value func = callback->GetNapiValue();
    napi_call_function(env_, CreateJsUndefined(env_), func, ArraySize(argv), argv, nullptr);
}

void JsFreeInstallObserver::CallPromise(napi_deferred deferred, int32_t resultCode)
{
    AbilityRuntime::HandleScope handleScope(env_);
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null deferred");
        return;
    }
    if (resultCode == ERR_OK || resultCode == AAFwk::ATOMIC_SERVICE_MINIMIZED) {
        napi_value value = CreateJsUndefined(env_);
        napi_resolve_deferred(env_, deferred, value);
    } else {
        napi_value error = CreateJsError(env_, GetJsErrorCodeByNativeError(resultCode));
        napi_reject_deferred(env_, deferred, error);
    }
}

void JsFreeInstallObserver::CallPromise(napi_deferred deferred, napi_value abilityResult)
{
    if (deferred == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null deferred");
        return;
    }
    napi_resolve_deferred(env_, deferred, abilityResult);
}

void JsFreeInstallObserver::AddJsObserverObject(const std::string &bundleName, const std::string &abilityName,
    const std::string &startTime, napi_value jsObserverObject, napi_value* result, bool isAbilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "call");
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
            if (it->bundleName == bundleName && it->abilityName == abilityName &&
                it->startTime == startTime) {
                TAG_LOGW(AAFwkTag::FREE_INSTALL, "add jsObject");
                return;
            }
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    JsFreeInstallObserverObject object;
    object.bundleName = bundleName;
    object.abilityName = abilityName;
    object.startTime = startTime;
    AddJsObserverCommon(object, jsObserverObject, result, isAbilityResult);
}

void JsFreeInstallObserver::AddJsObserverObject(const std::string &startTime, const std::string &url,
    napi_value jsObserverObject, napi_value* result, bool isAbilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "AddJsObserverObject, startTime:%{public}s, url:%{public}s, isAbilityResult:%{public}d",
             startTime.c_str(), url.c_str(), isAbilityResult);
    {
        std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "jsObserverObjectList_ size before add:%{public}zu", jsObserverObjectList_.size());
        for (auto it = jsObserverObjectList_.begin(); it != jsObserverObjectList_.end(); ++it) {
            if (it->startTime == startTime && it->url == url) {
                TAG_LOGW(AAFwkTag::FREE_INSTALL, "Duplicate observer found, skipping add. startTime:%{public}s, url:%{public}s",
                         startTime.c_str(), url.c_str());
                return;
            }
        }
    }

    StartAsyncTrace(HITRACE_TAG_ABILITY_MANAGER, "StartFreeInstall", atoi(startTime.c_str()));
    JsFreeInstallObserverObject object;
    object.startTime = startTime;
    object.url = url;
    AddJsObserverCommon(object, jsObserverObject, result, isAbilityResult);
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "AddJsObserverObject completed");
}

void JsFreeInstallObserver::AddJsObserverCommon(JsFreeInstallObserverObject &object,
    napi_value jsObserverObject, napi_value* result, bool isAbilityResult)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "AddJsObserverCommon, startTime:%{public}s, url:%{public}s, bundleName:%{public}s, abilityName:%{public}s, isAbilityResult:%{public}d",
             object.startTime.c_str(), object.url.c_str(), object.bundleName.c_str(), object.abilityName.c_str(), isAbilityResult);
    object.isAbilityResult = isAbilityResult;
    napi_valuetype type = napi_undefined;
    napi_typeof(env_, jsObserverObject, &type);
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "jsObserverObject type:%{public}d (function:%{public}d)", type, napi_function);
    if (jsObserverObject == nullptr || type != napi_function) {
        napi_deferred deferred;
        napi_create_promise(env_, &deferred, result);
        object.deferred = deferred;
        object.callback = nullptr;
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "Created promise, deferred:%{public}p", deferred);
    } else {
        napi_ref ref = nullptr;
        napi_get_undefined(env_, result);
        napi_create_reference(env_, jsObserverObject, 1, &ref);
        object.deferred = nullptr;
        object.callback = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference *>(ref));
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "Created callback reference");
    }
    std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
    jsObserverObjectList_.emplace_back(object);
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "Added observer to list, total size:%{public}zu", jsObserverObjectList_.size());
    DumpObservers();
}

void JsFreeInstallObserver::DumpObservers()
{
    std::unique_lock<std::mutex> lock(jsObserverObjectListLock_);
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "Dumping all observers, total:%{public}zu", jsObserverObjectList_.size());
    int32_t index = 0;
    for (const auto& observer : jsObserverObjectList_) {
        TAG_LOGD(AAFwkTag::FREE_INSTALL, "Observer[%{public}d]: startTime=%{public}s, url=%{public}s, abilityName=%{public}s, bundleName=%{public}s, callback=%{public}p, deferred=%{public}p, isAbilityResult=%{public}d",
                 index++, observer.startTime.c_str(), observer.url.c_str(), observer.abilityName.c_str(),
                 observer.bundleName.c_str(), observer.callback.get(), observer.deferred, observer.isAbilityResult);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS