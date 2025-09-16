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
#include "ani_observer_utils.h"
#include <algorithm>
#include <endian.h>

#include "hilog_tag_wrapper.h"

namespace ani_observerutils {

std::shared_ptr<OHOS::AppExecFwk::EventHandler> JsBaseObserver::mainHandler_;

JsBaseObserver::JsBaseObserver()
{
}

bool JsBaseObserver::SendEventToMainThread(const std::function<void()> func)
{
    if (func == nullptr) {
        return false;
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            TAG_LOGI(AAFwkTag::MISSION, "GetMainEventRunner failed");
            return false;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    mainHandler_->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
    return true;
}

MissionObserver::MissionObserver()
{
    TAG_LOGI(AAFwkTag::MISSION, "DataObserver");
}

MissionObserver::~MissionObserver()
{
    TAG_LOGI(AAFwkTag::MISSION, "~DataObserver");
}

void MissionObserver::AddCallback(VarCallbackType cb, ani_object callbackObj)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ani_ref callbackRef = CreateCallbackRefIfNotDuplicate(callbackObj);
    if (callbackRef != nullptr) {
        auto item = std::make_pair(cb, callbackRef);
        callbackList_.push_back(item);
    }
}

void MissionObserver::DeleteCallback(ani_ref jsCallbackRef)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ani_env *env = taihe::get_env();
    if (env == nullptr) {
        return;
    }
    auto pred = [env, jsCallbackRef](std::pair<VarCallbackType, ani_ref> &obj) {
        ani_boolean is_equal = false;
        if (jsCallbackRef == nullptr) {
            return true;
        }
        return (ANI_OK == env->Reference_StrictEquals(jsCallbackRef, obj.second, &is_equal)) && is_equal;
    };
    for (auto iter = callbackList_.begin(); iter != callbackList_.end();) {
        if (pred(*iter) == true) {
            iter = callbackList_.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool MissionObserver::IsCallbackListEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return callbackList_.size() == 0;
}

ani_ref MissionObserver::CreateCallbackRefIfNotDuplicate(ani_object callbackObj)
{
    ani_ref callbackRef = nullptr;
    ani_env *env = taihe::get_env();
    if (env == nullptr || ANI_OK != env->GlobalReference_Create(callbackObj, &callbackRef)) {
        return nullptr;
    }
    bool isDuplicate = std::any_of(callbackList_.begin(), callbackList_.end(),
        [env, callbackRef](std::pair<VarCallbackType, ani_ref> const& obj) {
            ani_boolean isEqual = false;
            return (ANI_OK == env->Reference_StrictEquals(callbackRef, obj.second, &isEqual)) && isEqual;
        });
    if (isDuplicate) {
        env->GlobalReference_Delete(callbackRef);
        return nullptr;
    }
    return callbackRef;
}

void MissionObserver::OnCallback(const uint32_t continueState, const std::string &srcDeviceId,
    const std::string &bundleName, const std::string &continueType,
    const std::string &srcBundleName)
{
    TAG_LOGI(AAFwkTag::MISSION, "MissionObserver::OnCallback");
    auto taiheState = ohos::distributedmissionmanager::ContinueState::from_value(continueState);
    ::ContinuableInfo::ContinuableInfo taiheInfo = {};
    taiheInfo.srcDeviceId = srcDeviceId;
    taiheInfo.bundleName = bundleName;
    taiheInfo.srcBundleName = ::taihe::optional<::taihe::string>::make(srcBundleName);
    taiheInfo.continueType = ::taihe::optional<::taihe::string>::make(continueType);
    ::ohos::distributedmissionmanager::ContinueCallbackInfo callbackInfo = { .state = taiheState, .info = taiheInfo };

    wptr<MissionObserver> weakthis = this;
    SendEventToMainThread([callbackInfo, weakthis] {
        auto sptrthis = weakthis.promote();
        if (sptrthis != nullptr) {
            sptrthis->OnCallbackInMainThread(callbackInfo);
        }
    });
}

void MissionObserver::OnCallbackInMainThread(const ::ohos::distributedmissionmanager::ContinueCallbackInfo& info)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto iter = callbackList_.begin(); iter != callbackList_.end();) {
        std::pair<VarCallbackType, ani_ref> const& cbItem = *iter;
        auto &jsfunc = std::get<JsOnCallbackType>(cbItem.first);
        jsfunc(info);
    }
}

} // namespace ani_observerutils