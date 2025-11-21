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
#include "ani_remotelistener_utils.h"
#include "ani_error_utils.h"
#include "ani_utils.h"
#include <algorithm>
#include <endian.h>

#include "hilog_tag_wrapper.h"

namespace ani_remotelistenerutils {

static std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_;

AniRemoteMissionListener::AniRemoteMissionListener(const ::MissionCallbacks::MissionCallback &ref)
    :callbacks_(ref)
{
}

AniRemoteMissionListener::~AniRemoteMissionListener()
{
}

void AniRemoteMissionListener::SetCallbacks(const ::MissionCallbacks::MissionCallback &ref)
{
    callbacks_ = ref;
}

void AniRemoteMissionListener::NotifyMissionsChanged(const std::string &deviceId)
{
    wptr<AniRemoteMissionListener> weakthis = this;
    SendEventToMainThread([deviceId, weakthis] {
        auto sptrthis = weakthis.promote();
        if (sptrthis != nullptr) {
            sptrthis->NotifyMissionsChangedInMainThread(deviceId);
        }
    });
}

void AniRemoteMissionListener::NotifySnapshot(const std::string &deviceId, int32_t missionId)
{
    wptr<AniRemoteMissionListener> weakthis = this;
    SendEventToMainThread([deviceId, missionId, weakthis] {
        auto sptrthis = weakthis.promote();
        if (sptrthis != nullptr) {
            sptrthis->NotifySnapshotInMainThread(deviceId, missionId);
        }
    });
}

void AniRemoteMissionListener::NotifyNetDisconnect(const std::string &deviceId, int32_t state)
{
    wptr<AniRemoteMissionListener> weakthis = this;
    SendEventToMainThread([deviceId, state, weakthis] {
        auto sptrthis = weakthis.promote();
        if (sptrthis != nullptr) {
            sptrthis->NotifyNetDisconnectInMainThread(deviceId, state);
        }
    });
}

void AniRemoteMissionListener::NotifyMissionsChangedInMainThread(const std::string &deviceId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (released_) {
        return;
    }
    ::taihe::string taiheDeviceId(deviceId);
    callbacks_.notifyMissionsChanged(taiheDeviceId);
}

void AniRemoteMissionListener::NotifySnapshotInMainThread(const std::string &deviceId, int32_t missionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (released_) {
        return;
    }
    ::taihe::string taiheDeviceId(deviceId);
    callbacks_.notifySnapshot(taiheDeviceId, missionId);
}

void AniRemoteMissionListener::NotifyNetDisconnectInMainThread(const std::string &deviceId, int32_t state)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (released_) {
        return;
    }
    ::taihe::string taiheDeviceId(deviceId);
    callbacks_.notifyNetDisconnect(taiheDeviceId, state);
}

bool AniRemoteMissionListener::SendEventToMainThread(const std::function<void()> func)
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

void AniRemoteMissionListener::Release()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    released_ = true;
}

AniMissionContinue::AniMissionContinue(::ContinueCallback::ContinueCallback const& ref)
    :callbacks_(ref)
{
    TAG_LOGI(AAFwkTag::MISSION, "xxx AniMissionContinue 1");
}

bool AniMissionContinue::SendEventToMainThread(const std::function<void()> func)
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

void AniMissionContinue::Release()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    released_ = true;
    deferred_ = nullptr;
    callbacks_.reset();
}

void AniMissionContinue::SetDeferred(ani_env *env, ani_resolver deferred)
{ 
    env_ = env;
    deferred_ = deferred; 
    env_->GetVM(&vm_);
}

void AniMissionContinue::OnContinueDone(int32_t result)
{
    TAG_LOGI(AAFwkTag::MISSION, "xxx OnContinueDone result %{public}d", result);
    if (deferred_ != nullptr) {
        if (vm_ == nullptr) {
            TAG_LOGI(AAFwkTag::MISSION, "xxx OnContinueDone null env or vm");
            return;
        }
        ani_utils::AniExecuteFunc(vm_, [this, result] (ani_env* currentEnv) {
            this->PromiseResult(currentEnv, result);
        });
    } else {
        auto sharedThis = shared_from_this();
        SendEventToMainThread([sharedThis, result] {
            sharedThis->OnContinueDoneInMainThread(result);
        });
    }
}

void AniMissionContinue::OnContinueDoneInMainThread(int32_t result)
{
    TAG_LOGI(AAFwkTag::MISSION, "OnContinueDoneInMainThread");
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (callbacks_.has_value()) {
        callbacks_.value().onContinueDone(result);
    }
}

void AniMissionContinue::PromiseResult(ani_env* currentEnv, int32_t result)
{
    TAG_LOGI(AAFwkTag::MISSION, "PromiseResult");
    if (currentEnv == nullptr) {
        return;
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (deferred_) {
        ani_object resolveResult = nullptr;
        ani_utils::AniCreateInt(currentEnv, result, resolveResult);
        ani_errorutils::AniPromiseCallback(currentEnv, deferred_, 0, resolveResult);
        deferred_ = nullptr;
    }
}

} // namespace ani_observerutils