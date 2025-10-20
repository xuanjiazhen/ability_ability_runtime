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

#include "foreground_app_connection_manager.h"

#include <chrono>

#include "foreground_app_connection_data.h"
#include "foreground_app_connection_errors.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {

ForegroundAppConnectionManager::ForegroundAppConnectionManager() {}
ForegroundAppConnectionManager::~ForegroundAppConnectionManager() {}

int32_t ForegroundAppConnectionManager::RegisterObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundAppConnectionManager::RegisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return ERR_NULL_OBJECT;
    }

    std::lock_guard<std::mutex> guard(observerMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size before add: %{public}zu", observerList_.size());

    auto isDuplicate = [observer](sptr<AbilityRuntime::IForegroundAppConnection> ob) {
        return ob && (ob->AsObject() == observer->AsObject());
    };
    
    if (std::any_of(observerList_.begin(), observerList_.end(), isDuplicate)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "observer already registered");
        return ERR_OK;
    }

    if (!observerDeathRecipient_) {
        std::weak_ptr<ForegroundAppConnectionManager> weakThis = shared_from_this();
        observerDeathRecipient_ =
            new ConnectionDeathRecipient([weakThis](const wptr<IRemoteObject> &remoteObject) {
                auto manager = weakThis.lock();
                if (manager) {
                    manager->HandleRemoteDied(remoteObject);
                }
            });
    }
    auto observerObj = observer->AsObject();
    if (!observerObj->AddDeathRecipient(observerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "add death recipient failed");        
    }   
    observerList_.emplace_back(observer);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list after add size:%{public}zu", observerList_.size());
    return ERR_OK;
}

int32_t ForegroundAppConnectionManager::UnregisterObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundAppConnectionManager::UnregisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return ERR_NULL_OBJECT;
    }
    std::lock_guard<std::mutex> guard(observerMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size before remove: %{public}zu", observerList_.size());
    for (auto it = observerList_.begin(); it != observerList_.end(); ++it) {
        if (*it == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        if ((*it)->AsObject() == observer->AsObject()) {
            observerList_.erase(it);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "remove success size: %{public}zu", observerList_.size());
            break;
        }
    }
    return ERR_OK;
}

void ForegroundAppConnectionManager::OnConnected(AbilityRuntime::ForegroundAppConnectionData &data) {
    std::lock_guard<std::mutex> guard(observerMutex_);
    for (auto &ob : observerList_) {
        if (ob == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        ob->OnForegroundAppConnected(data);
    }
}

void ForegroundAppConnectionManager::OnDisconnected(AbilityRuntime::ForegroundAppConnectionData &data) {
    std::lock_guard<std::mutex> guard(observerMutex_);
    for (auto &ob : observerList_) {
        if (ob == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        ob->OnForegroundAppDisconnected(data);
    }
}

void ForegroundAppConnectionManager::OnCallerStarted(int32_t callerPid, int32_t callerUid,
        const std::string &bundleName)
{
    std::lock_guard<std::mutex> guard(observerMutex_);
    for (auto &ob : observerList_) {
        if (ob == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        ob->OnForegroundAppCallerStarted(callerPid, callerUid, bundleName);
    }
}

void ForegroundAppConnectionManager::AbilityAddPidConnection(ForegroundAppConnectionInfo &info,
    int32_t abilityRecordId)
{
    int32_t callerPid = info.callerPid_;
    int32_t targetPid = info.targetPid_;
    std::string curObserverKey = std::to_string(callerPid) + "_" + std::to_string(targetPid);
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    auto it = pidMap_.find(curObserverKey);
    if (it != pidMap_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pidConnection already exists: %{public}s", curObserverKey.c_str());
        it->second.AddAbilityRecordId(abilityRecordId);
        return;
    }

    info.AddAbilityRecordId(abilityRecordId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "add pidConnection %{public}s", curObserverKey.c_str());
    AbilityRuntime::ForegroundAppConnectionData data;
    GenerateConnectionData(info, data);
    OnConnected(data);
    pidMap_.emplace(curObserverKey, info);
}

void ForegroundAppConnectionManager::AbilityRemovePidConnection(int32_t callerPid, int32_t targetPid,
    int32_t abilityRecordId)
{
    std::string curObserverKey = std::to_string(callerPid) + "_" + std::to_string(targetPid);
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    auto it = pidMap_.find(curObserverKey);
    if (it == pidMap_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pidConnection not exist, key: %{public}s", curObserverKey.c_str());
        return;
    }
    it->second.RemoveAbilityRecordId(abilityRecordId);
    if (it->second.abilityRecordIds_.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "remove pidConnection %{public}s", curObserverKey.c_str());
        AbilityRuntime::ForegroundAppConnectionData data;
        GenerateConnectionData(info, data);
        OnDisconnected(data);
        pidMap_.erase(it);
    }
}

void ForegroundAppConnectionManager::ProcessRemovePidConnection(int32_t diedPid)
{
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    for (auto it = pidMap_.begin(); it != pidMap_.end();) {
        auto &curConnectInfo = it->second;
        if (curConnectInfo && (curConnectInfo.callerPid_ == diedPid || curConnectInfo.targetPid_ == diedPid)) {
            AbilityRuntime::ForegroundAppConnectionData data;
            GenerateConnectionData(curConnectInfo, data);
            std::string curObserverKey =
                std::to_string(curConnectInfo.callerPid_) + "_" + std::to_string(curConnectInfo.targetPid_);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "remove pidConnection %{public}s", curObserverKey.c_str());
            OnDisconnected(data);
            it = pidMap_.erase(it);
        } else {
            ++it;
        }
    }
}

void ForegroundAppConnectionManager::HandleRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "remote connection observer died");
    auto remoteObj = remoteObject.promote();
    if (!remoteObj) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "remoteObj is null");
        return;
    }
    remoteObj->RemoveDeathRecipient(observerDeathRecipient_);

    std::lock_guard<std::mutex> guard(observerMutex_);
    auto iter = std::find_if(observerList_.begin(), observerList_.end(),
        [&remoteObj](sptr<AbilityRuntime::IForegroundAppConnection> item) {
        return (item && item->AsObject() == remoteObj);
    });
    if (iter != observerList_.end()) {
        observerList_.erase(iter);
    }
}

void ForegroundAppConnectionManager::GenerateConnectionData(
    ForegroundAppConnectionInfo &info, AbilityRuntime::ForegroundAppConnectionData &data)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GenerateConnectionData");
    data.callerPid_ = info.callerPid_;
    data.targetPid_ = info.targetPid_;
    data.callerUid_ = info.callerUid_;
    data.targetUid_ = info.targetUid_;
    data.callerBundleName_ = info.callerBundleName_;
    data.targetBundleName_ = info.targetBundleName_;
}

ForegroundAppConnectionManager::ConnectionDeathRecipient::ConnectionDeathRecipient(
    ConnectionDeathHandler handler) : deathHandler_(handler)
{
}

void ForegroundAppConnectionManager::ConnectionDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remoteObject)
{
    if (deathHandler_) {
        deathHandler_(remoteObject);
    }
}

void ForegroundAppConnectionInfo::AddAbilityRecordId(int32_t abilityRecordId)
{
    for (auto &id : abilityRecordIds_) {
        if (id == abilityRecordId) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRecordId already exists");
            return;
        }
    }
    abilityRecordIds_.emplace_back(abilityRecordId);
}

void ForegroundAppConnectionInfo::RemoveAbilityRecordId(int32_t abilityRecordId) {
    abilityRecordIds_.erase(
        std::remove(abilityRecordIds_.begin(), abilityRecordIds_.end(), abilityRecordId),
        abilityRecordIds_.end()
    );
}
} // namespace AAFwk
} // namespace OHOS