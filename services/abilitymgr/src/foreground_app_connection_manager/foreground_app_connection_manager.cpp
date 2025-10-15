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
 
int32_t ForegroundAppConnectionManager::RegisterObserver(sptr<IForegroundAppConnection> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundAppConnectionManager::RegisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return ERR_NULL_OBJECT;
    }

    std::lock_guard<std::mutex> guard(oberverMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size before add: %{public}zu", observerList_.size());
    for (auto &ob : observerList_) {
        if (ob->AsObject() == observer->AsObject()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "observer already registered");
            return ERR_OK;
        }
    }
    observerList_.emplace_back(observer);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
    return ERR_OK;
}

void ForegroundAppConnectionManager::UnregisterObserver(sptr<IForegroundAppConnection> observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ForegroundAppConnectionManager::UnregisterObserver called");
    if (!observer || !observer->AsObject()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> guard(oberverMutex_);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
    for (auto &ob : observerList_) {
        if (ob->AsObject() == observer->AsObject()) {
            observerList_.erase(ob);
            break;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}zu", observerList_.size());
}

void ForegroundAppConnectionManager::OnConnected(ForegroundAppConnectionData &data) {
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto &ob : observerList_) {
        ob->OnForegroundAppConnected(data);
    }
}

void ForegroundAppConnectionManager::OnDisconnected(ForegroundAppConnectionData &data) {
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto &ob : observerList_) {
        ob->OnForegroundAppDisconnected(data);
    }
}

void ForegroundAppConnectionManager::OnCallerStarted(int32_t callerPid) {
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto &ob : observerList_) {
        ob->OnForegroundAppCallerStarted(callerPid);
    }
}

void ForegroundAppConnectionManager::AbilityAddPidConnection(int32_t callerPid, int32_t targetPid, int32_t abilityRecordId)
{
    curObserverKey = std::to_string(callerPid) + std::to_string(targetPid);
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    auto it = pidMap_.find(curObserverKey);
    if (it != pidMap_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pidConnection already exists");
        it->second.AddAbilityRecordId(abilityRecordId);
        return;
    }

    ForegroundAppConnectionInfo info(callerPid, targetPid);
    info.AddAbilityRecordId(abilityRecordId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "add pidConnection %{public}s", curObserverKey.c_str());
    OnConnected(ForegroundAppConnectionData(callerPid, targetPid));
    pidMap_[curObserverKey] = info;
}

void ForegroundAppConnectionManager::AbilityRemovePidConnection(int32_t callerPid, int32_t targetPid, int32_t abilityRecordId)
{
    curObserverKey = std::to_string(callerPid) + std::to_string(targetPid);
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    auto it = pidMap_.find(curObserverKey);
    if (it == pidMap_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "pidConnection not exist");
        return;
    }
    it->second.RemoveAbilityRecordId(abilityRecordId);
    if (it->second.abilityRecordIds.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "remove pidConnection %{public}s", curObserverKey.c_str());
        OnDisconnected(ForegroundAppConnectionData(callerPid, targetPid));
        pidMap_.erase(it);
    }
}

void ForegroundAppConnectionManager::ProcessRemovePidConnection(int32_t DiedPid)
{
    std::lock_guard<std::mutex> guard(pidMapMutex_);
    for (auto it = pidMap_.begin(); it != pidMap_.end();) {
        if (it->second.callerPid == DiedPid || it->second.targetPid == DiedPid) {
            auto curConnectInfo = it->second;
            ForegroundAppConnectionData data(curConnectInfo.callerPid_, curConnectInfo.targetPid_);
            curObserverKey = std::to_string(curConnectInfo.callerPid_) + std::to_string(curConnectInfo.targetPid_);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "remove pidConnection %{public}s", curObserverKey.c_str());
            OnDisconnected(data);
            pidMap_.erase(it++);
        } else {
            ++it;
        }
    }
}

void ForegroundAppConnectionInfo::AddAbilityRecordId(int32_t abilityRecordId)
{
    for (auto &id : abilityRecordIds) {
        if (id == abilityRecordId) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "abilityRecordId already exists");
            return;
        }
    }
    abilityRecordIds.emplace_back(abilityRecordId);
}

void ForegroundAppConnectionInfo::RemoveAbilityRecordId(int32_t abilityRecordId) {
    for (auto it = abilityRecordIds.begin(); it != abilityRecordIds.end(); ++it) {
        if (*it == abilityRecordId) {
            abilityRecordIds.erase(it);
            break;
        }
    }
}
} // namespace AAFwk
} // namespace OHOS