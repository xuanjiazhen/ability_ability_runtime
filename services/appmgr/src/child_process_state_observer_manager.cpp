/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "child_process_state_observer_manager.h"

#include "hilog_tag_wrapper.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
ChildProcessStateObserverManager::ChildProcessStateObserverManager()
{}

ChildProcessStateObserverManager::~ChildProcessStateObserverManager()
{}

void ChildProcessStateObserverManager::Init()
{
    if (!handler_) {
        handler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("child_process_state_task_queue");
    }
}

int32_t ChildProcessStateObserverManager::RegisterChildProcessStateObserver(
    const sptr<IChildProcessStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null handler");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null self");
            return;
        }
        self->HandleRegisterChildProcessStateObserver(observer);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void ChildProcessStateObserverManager::HandleRegisterChildProcessStateObserver(
    const sptr<IChildProcessStateObserver> &observer)
{
    if (deathRecipient_ == nullptr) {
        std::weak_ptr<ChildProcessStateObserverManager> thisWeakPtr(shared_from_this());
        deathRecipient_ =
            new (std::nothrow) ChildProcessStateObserverRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto childProcessStateObserverManager = thisWeakPtr.lock();
                if (childProcessStateObserverManager) {
                    childProcessStateObserverManager->OnObserverDied(remote);
                }
            });
        if (deathRecipient_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "ChildProcessStateObserverManager failed");
        }
    }

    auto observerObj = observer->AsObject();
    if (!observerObj || !observerObj->AddDeathRecipient(deathRecipient_)) {
        TAG_LOGE(AAFwkTag::APPMGR, "AddDeathRecipient failed");
    }

    observerList_.emplace_back(observer);
    TAG_LOGD(AAFwkTag::APPMGR, "observerList_ size:%{public}zu", observerList_.size());
}

int32_t ChildProcessStateObserverManager::UnregisterChildProcessStateObserver(
    const sptr<IChildProcessStateObserver> &observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "begin.");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null handler");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), observer]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null self");
            return;
        }
        self->HandleUnregisterChildProcessStateObserver(observer);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void ChildProcessStateObserverManager::HandleUnregisterChildProcessStateObserver(
    const sptr<IChildProcessStateObserver> &observer)
{
    auto it = std::find_if(observerList_.begin(), observerList_.end(),
        [&observer](const sptr<IChildProcessStateObserver> &item) {
        return (item && item->AsObject() == observer->AsObject());
    });
    if (it != observerList_.end()) {
        observerList_.erase(it);
        TAG_LOGI(AAFwkTag::APPMGR, "observerList_ size:%{public}zu", observerList_.size());
        return;
    }
    TAG_LOGE(AAFwkTag::APPMGR, "Observer not exist or has been removed");
}

int32_t ChildProcessStateObserverManager::OnChildProcessStateChanged(
    int32_t pid, int32_t uid, int32_t callingPid, int32_t callingUid, int32_t state)
{
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null handler");
        return ERR_INVALID_VALUE;
    }
    auto task = [weak = weak_from_this(), pid, uid, callingPid, callingUid, state]() {
        auto self = weak.lock();
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null self");
            return;
        }
        self->HandleOnChildProcessStateChanged(pid, uid, callingPid, callingUid, state);
    };
    handler_->SubmitTask(task);
    return ERR_OK;
}

void ChildProcessStateObserverManager::HandleOnChildProcessStateChanged(
    int32_t pid, int32_t uid, int32_t callingPid, int32_t callingUid, int32_t state)
{
    ChildProcessStateData data = WrapChildProcessStateData(pid, uid, callingPid, callingUid, state);
    TAG_LOGD(AAFwkTag::APPMGR,
        "pid:%{public}d, uid:%{public}d, callingPid:%{public}d, callingUid:%{public}d, state:%{public}d",
        data.pid, data.uid, data.callingPid, data.callingUid, data.state);
    for (auto it = observerList_.begin(); it != observerList_.end(); ++it) {
        if ((*it) == nullptr) {
            continue;
        }
        (*it)->OnChildProcessStateChanged(data);
    }
}

ChildProcessStateData ChildProcessStateObserverManager::WrapChildProcessStateData(
    int32_t pid, int32_t uid, int32_t callingPid, int32_t callingUid, int32_t state)
{
    ChildProcessStateData childProcessStateData;
    childProcessStateData.pid = pid;
    childProcessStateData.uid = uid;
    childProcessStateData.callingPid = callingPid;
    childProcessStateData.callingUid = callingUid;
    childProcessStateData.state = state;
    return childProcessStateData;
}

void ChildProcessStateObserverManager::OnObserverDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::APPMGR, "call");
    auto remoteObj = remote.promote();
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    remoteObj->RemoveDeathRecipient(deathRecipient_);

    sptr<IChildProcessStateObserver> observer = iface_cast<IChildProcessStateObserver>(remoteObj);
    UnregisterChildProcessStateObserver(observer);
}

ChildProcessStateObserverRecipient::ChildProcessStateObserverRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

ChildProcessStateObserverRecipient::~ChildProcessStateObserverRecipient()
{}

void ChildProcessStateObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGE(AAFwkTag::APPMGR, "call");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS