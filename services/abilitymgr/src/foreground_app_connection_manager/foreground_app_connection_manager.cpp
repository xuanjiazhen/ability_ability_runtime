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
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {

ForegroundAppConnectionManager& ForegroundAppConnectionManager::GetInstance()
{
    static ForegroundAppConnectionManager instance;
    return instance;
}
 
int32_t ForegroundAppConnectionManager::RegisterObserver(sptr<AbilityRuntime::IForegroundAppConnection> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundAppConnectionManager::RegisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return ERR_NULL_OBJECT;
    }

    std::lock_guard<std::mutex> guard(observerMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size before add: %{public}zu", observerList_.size());

    auto isDuplicate = [observer](const sptr<AbilityRuntime::IForegroundAppConnection>& ob) {
        return ob && (ob->AsObject() == observer->AsObject());
    };
    
    if (std::any_of(observerList_.begin(), observerList_.end(), isDuplicate)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "observer already registered");
        return ERR_OK;
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
        try {
            ob->OnForegroundAppConnected(data);
        } catch (...) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "exception in OnForegroundAppConnected");
        }
    }
}

void ForegroundAppConnectionManager::OnDisconnected(AbilityRuntime::ForegroundAppConnectionData &data) {
    std::lock_guard<std::mutex> guard(observerMutex_);
    for (auto &ob : observerList_) {
        if (ob == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        try {
            ob->OnForegroundAppDisconnected(data);
        } catch (...) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "exception in OnForegroundAppDisconnected");
        }
    }
}

void ForegroundAppConnectionManager::OnCallerStarted(int32_t callerPid) {
    std::lock_guard<std::mutex> guard(observerMutex_);
    for (auto &ob : observerList_) {
        if (ob == nullptr) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        try {
            ob->OnForegroundAppCallerStarted(callerPid);
        } catch (...) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "exception occurred in OnForegroundAppCallerStarted");
        }
    }
}

void ForegroundAppConnectionManager::AbilityAddPidConnection(int32_t callerPid, int32_t targetPid,
    int32_t abilityRecordId)
{
    std::string curObserverKey = std::to_string(callerPid) + "_" + std::to_string(targetPid);
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    auto it = pidMap_.find(curObserverKey);
    if (it != pidMap_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pidConnection already exists: %{public}s", curObserverKey.c_str());
        it->second.AddAbilityRecordId(abilityRecordId);
        return;
    }

    ForegroundAppConnectionInfo info(callerPid, targetPid);
    info.AddAbilityRecordId(abilityRecordId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "add pidConnection %{public}s", curObserverKey.c_str());
    AbilityRuntime::ForegroundAppConnectionData data(callerPid, targetPid);
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
        AbilityRuntime::ForegroundAppConnectionData data(callerPid, targetPid);
        OnDisconnected(data);
        pidMap_.erase(it);
    }
}

void ForegroundAppConnectionManager::ProcessRemovePidConnection(int32_t DiedPid)
{
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    for (auto it = pidMap_.begin(); it != pidMap_.end();) {
        if (it->second.callerPid_ == DiedPid || it->second.targetPid_ == DiedPid) {
            auto &curConnectInfo = it->second;
            AbilityRuntime::ForegroundAppConnectionData data(curConnectInfo.callerPid_, curConnectInfo.targetPid_);
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