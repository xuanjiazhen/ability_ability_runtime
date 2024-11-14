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
#include "disposed_observer/disposed_observer_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "app_mgr_util.h"
#include "hilog_tag_wrapper.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
DisposedObserverManager::DisposedObserverManager()
{}

DisposedObserverManager::~DisposedObserverManager()
{}

int32_t DisposedObserverManager::RegisterFirstFrameObserver(const std::string &bundleName,
    AppExecFwk::DisposedRule disposedRule, DisposedModalType type)
{
    sptr<DisposedFirstFrameObserver> disposedObserver = sptr<DisposedFirstFrameObserver>::MakeSptr(disposedRule);
    CHECK_POINTER_AND_RETURN(disposedObserver, ERR_INVALID_VALUE);
    disposedObserver->SetModalType(type);
    int32_t ret = IN_PROCESS_CALL(
        DelayedSingleton<AbilityManagerService>::GetInstance()->RegisterAbilityFirstFrameStateObserver(
            disposedObserver, bundleName));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register failed, err:%{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<ffrt::mutex> guard(observerLock_);
        disposedFirstFrameObserverMap_.emplace(bundleName, disposedObserver);
    }
    return ERR_OK;
}

int32_t DisposedObserverManager::RegisterAppStateObserver(const std::string &bundleName,
    AppExecFwk::DisposedRule disposedRule)
{
    auto appMgr = AppMgrUtil::GetAppMgr();
    CHECK_POINTER_AND_RETURN(appMgr, ERR_INVALID_VALUE);
    sptr<DisposedAppStateObserver> disposedObserver = sptr<DisposedAppStateObserver>::MakeSptr(disposedRule);
    CHECK_POINTER_AND_RETURN(disposedObserver, ERR_INVALID_VALUE);
    std::vector<std::string> bundleNameList;
    bundleNameList.push_back(bundleName);
    int32_t ret = IN_PROCESS_CALL(appMgr->RegisterApplicationStateObserver(disposedObserver, bundleNameList));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register failed, err:%{public}d", ret);
        return ret;
    }
    {
        std::lock_guard<ffrt::mutex> guard(observerLock_);
        disposedAppStateObserverMap_.emplace(bundleName, disposedObserver);
    }
    return ERR_OK;
}

void DisposedObserverManager::UnregisterObserver(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    auto iter = disposedAppStateObserverMap_.find(bundleName);
    if (iter != disposedAppStateObserverMap_.end()) {
        auto appManager = AppMgrUtil::GetAppMgr();
        CHECK_POINTER(appManager);
        IN_PROCESS_CALL(appManager->UnregisterApplicationStateObserver(iter->second));
        disposedAppStateObserverMap_.erase(iter);
    }
    auto item = disposedFirstFrameObserverMap_.find(bundleName);
    if (item != disposedFirstFrameObserverMap_.end()) {
        IN_PROCESS_CALL(DelayedSingleton<AbilityManagerService>::GetInstance()->
            UnregisterAbilityFirstFrameStateObserver(item->second));
        disposedFirstFrameObserverMap_.erase(item);
    }
}

bool DisposedObserverManager::IsExistObserver(const std::string &bundleName)
{
    std::lock_guard<ffrt::mutex> guard(observerLock_);
    if (disposedAppStateObserverMap_.find(bundleName) == disposedAppStateObserverMap_.end() &&
        disposedFirstFrameObserverMap_.find(bundleName) == disposedFirstFrameObserverMap_.end()) {
        return false;
    }
    return true;
}
} // namespace AAFwk
} // namespace OHOS