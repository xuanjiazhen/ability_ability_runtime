/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "resident_process_manager.h"

#include <chrono>
#include <thread>

#include "ability_manager_service.h"
#include "ability_resident_process_rdb.h"
#include "ability_util.h"
#include "app_utils.h"
#include "ffrt.h"
#include "keep_alive_utils.h"
#include "main_element_utils.h"
#include "mission_info.h"
#include "user_controller/user_controller.h"
#ifdef SUPPORT_POWER
#include "power_mgr_client.h"
#endif


namespace OHOS {
namespace AAFwk {
ResidentAbilityInfoGuard::ResidentAbilityInfoGuard(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    residentId_ = DelayedSingleton<ResidentProcessManager>::GetInstance()->PutResidentAbility(bundleName,
        abilityName, userId);
}

ResidentAbilityInfoGuard::~ResidentAbilityInfoGuard()
{
    if (residentId_ != -1) {
        DelayedSingleton<ResidentProcessManager>::GetInstance()->RemoveResidentAbility(residentId_);
    }
}

void ResidentAbilityInfoGuard::SetResidentAbilityInfo(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    if (residentId_ != -1) {
        return;
    }
    residentId_ = DelayedSingleton<ResidentProcessManager>::GetInstance()->PutResidentAbility(bundleName,
        abilityName, userId);
}

ResidentProcessManager::ResidentProcessManager()
{}

ResidentProcessManager::~ResidentProcessManager()
{}

void ResidentProcessManager::Init()
{
    auto &amsRdb = AmsResidentProcessRdb::GetInstance();
    amsRdb.Init();
    RegisterScreenStateCallback();
}

void ResidentProcessManager::StartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    DelayedSingleton<AppScheduler>::GetInstance()->StartupResidentProcess(bundleInfos);
}

void ResidentProcessManager::StartResidentProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    std::set<uint32_t> needEraseIndexSet;

    for (size_t i = 0; i < bundleInfos.size(); i++) {
        StartResidentProcessWithMainElementPerBundle(bundleInfos[i], i, needEraseIndexSet, userId);
    }

    // delete item which process has been started.
    for (auto iter = needEraseIndexSet.rbegin(); iter != needEraseIndexSet.rend(); iter++) {
        bundleInfos.erase(bundleInfos.begin() + *iter);
    }
}

void ResidentProcessManager::StartResidentProcessWithMainElementPerBundle(const AppExecFwk::BundleInfo &bundleInfo,
    size_t index, std::set<uint32_t> &needEraseIndexSet, int32_t userId)
{
    if (userId != 0 && !AppUtils::GetInstance().InResidentWhiteList(bundleInfo.name)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "not in resident allow list");
        needEraseIndexSet.insert(index);
        return;
    }
    std::string processName = bundleInfo.applicationInfo.process;
    bool keepAliveEnable = bundleInfo.isKeepAlive;
    // Check startup permissions
    AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(bundleInfo.name, keepAliveEnable);
    if (!bundleInfo.isKeepAlive && keepAliveEnable) {
        auto &bundleName = bundleInfo.name;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "keepAliveBundle changed, bundle:%{public}s", bundleName.c_str());
        AmsResidentProcessRdb::GetInstance().RemoveData(bundleName);
        keepAliveEnable = false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "Precheck,bundle:%{public}s, process:%{public}s, keepAlive:%{public}d, enable:%{public}d",
        bundleInfo.name.c_str(), processName.c_str(), bundleInfo.isKeepAlive, keepAliveEnable);
    if (!keepAliveEnable || processName.empty()) {
        needEraseIndexSet.insert(index);
        return;
    }
    for (auto hapModuleInfo : bundleInfo.hapModuleInfos) {
        StartResidentProcessWithMainElementPerBundleHap(hapModuleInfo, processName, index, needEraseIndexSet, userId);
    }
}

void ResidentProcessManager::StartResidentProcessWithMainElementPerBundleHap(
    const AppExecFwk::HapModuleInfo &hapModuleInfo, const std::string &processName,
    size_t index, std::set<uint32_t> &needEraseIndexSet, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartResidentProcessWithMainElementPerBundleHap called, bundle: %{public}s, userId: %{public}d",
        hapModuleInfo.bundleName.c_str(), userId);

    std::string mainElement;
    bool isDataAbility = false;
    std::string uriStr;
    if (!MainElementUtils::CheckMainElement(hapModuleInfo,
        processName, mainElement, isDataAbility, uriStr, userId)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "CheckMainElement returned false, isDataAbility: %{public}d",
            isDataAbility);
        if (isDataAbility) {
            // dataability, need use AcquireDataAbility
            needEraseIndexSet.insert(index);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "call, mainElement: %{public}s, uri: %{public}s",
                mainElement.c_str(), uriStr.c_str());
            Uri uri(uriStr);
            DelayedSingleton<AbilityManagerService>::GetInstance()->AcquireDataAbility(uri, true, nullptr);
        }
        return;
    }

    needEraseIndexSet.insert(index);
    // startAbility
    Want want;
    want.SetElementName(hapModuleInfo.bundleName, mainElement);
    want.SetParam(AbilityConfig::PARAM_IS_RESIDENT_ABILITY, true);
    ResidentAbilityInfoGuard residentAbilityInfoGuard(hapModuleInfo.bundleName, mainElement, userId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartResidentAbility, bundleName: %{public}s, mainElement: %{public}s",
        hapModuleInfo.bundleName.c_str(), mainElement.c_str());
    auto ret = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, userId,
        DEFAULT_INVAL_VALUE);
    MainElementUtils::UpdateMainElement(hapModuleInfo.bundleName,
        hapModuleInfo.name, mainElement, true, userId);
    if (ret != ERR_OK) {
        AddFailedResidentAbility(hapModuleInfo.bundleName, mainElement, userId);
    } else {
        // Start memory defragmentation task after 140 seconds
        // Use public API GetResidentAbilityRecord to retrieve AbilityRecord
        StartResidentMemoryDefragTask(hapModuleInfo.bundleName, mainElement, userId);
    }
}

int32_t ResidentProcessManager::SetResidentProcessEnabled(
    const std::string &bundleName, const std::string &callerName, bool updateEnable)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetResidentProcessEnabled,bundle:%{public}s,caller:%{public}s,enable:%{public}d",
        bundleName.c_str(), callerName.c_str(), updateEnable);
    if (bundleName.empty() || callerName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "input parameter error");
        return INVALID_PARAMETERS_ERR;
    }
    auto &rdb = AmsResidentProcessRdb::GetInstance();
    auto rdbResult = rdb.VerifyConfigurationPermissions(bundleName, callerName);
    auto configResult = rdb.GetResidentProcessRawData(bundleName, callerName);
    if (rdbResult != Rdb_OK && configResult != Rdb_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "obtain permissions failed. result: %{public}d, configResult: %{public}d",
            rdbResult, configResult);
        return ERR_NO_RESIDENT_PERMISSION;
    }

    bool localEnable = false;
    rdbResult = rdb.GetResidentProcessEnable(bundleName, localEnable);
    if (rdbResult != Rdb_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetResidentProcess failed:%{public}d", rdbResult);
        return INNER_ERR;
    }

    if (updateEnable == localEnable) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no change to the resident process setting properties");
        return ERR_OK;
    }

    rdbResult = rdb.UpdateResidentProcessEnable(bundleName, updateEnable);
    if (rdbResult != Rdb_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resident process attribute update failed");
        return INNER_ERR;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Set keep alive enable state.");
        IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveEnableState(bundleName, updateEnable, 0));
    }
    ffrt::submit([self = shared_from_this(), bundleName, localEnable, updateEnable]() {
        self->UpdateResidentProcessesStatus(bundleName, localEnable, updateEnable);
    });
    return ERR_OK;
}

void ResidentProcessManager::UpdateResidentProcessesStatus(
    const std::string &bundleName, bool localEnable, bool updateEnable)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundle name empty");
        return;
    }

    auto bms = AbilityUtil::GetBundleManagerHelper();
    if (bms == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "obtain bms handle failed");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto userId = AbilityRuntime::UserController::GetInstance().GetCallerUserId();
    std::set<int32_t> users{0, userId};

    for (const auto &userId: users) {
        if (!IN_PROCESS_CALL(bms->GetBundleInfo(
            bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "get bundle info failed, userId:%{public}d", userId);
            continue;
        }

        if (updateEnable && !localEnable) {
            // need start
            std::vector<AppExecFwk::BundleInfo> bundleInfos{ bundleInfo };
            StartResidentProcessWithMainElement(bundleInfos, userId);
            if (!bundleInfos.empty()) {
                StartResidentProcess(bundleInfos);
            }
        } else if (!updateEnable && localEnable) {
            // just update
            std::vector<AppExecFwk::BundleInfo> bundleInfos{ bundleInfo };
            KeepAliveUtils::NotifyDisableKeepAliveProcesses(bundleInfos, userId);
        }
    }
}

void ResidentProcessManager::OnAppStateChanged(const AppInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called");
    if (info.state != AppState::BEGIN) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not a state of concern. state: %{public}d", info.state);
        return;
    }

    if (info.pid <= 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The obtained application pid is incorrect. state: %{public}d", info.pid);
        return;
    }

    std::string bundleName;
    // user 0
    int32_t uid = 0;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "app scheduler error");
        return;
    }
    appScheduler->GetBundleNameByPid(info.pid, bundleName, uid);
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get bundle name by pid failed");
        return;
    }

    bool localEnable = false;
    auto rdbResult = AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(bundleName, localEnable);
    if (rdbResult != Rdb_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "rdbResult: %{public}d", rdbResult);
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set keep alive enable state error");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveEnableState(bundleName, localEnable, 0));
}

int32_t ResidentProcessManager::PutResidentAbility(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    std::lock_guard lock(residentAbilityInfoMutex_);
    auto residentId = residentId_++;
    residentAbilityInfos_.push_back(ResidentAbilityInfo {
        .bundleName = bundleName,
        .abilityName = abilityName,
        .userId = userId,
        .residentId = residentId
    });
    return residentId;
}

bool ResidentProcessManager::IsResidentAbility(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    std::lock_guard lock(residentAbilityInfoMutex_);
    for (const auto &item: residentAbilityInfos_) {
        if (item.bundleName == bundleName && item.abilityName == abilityName && item.userId == userId) {
            return true;
        }
    }
    return false;
}

void ResidentProcessManager::RemoveResidentAbility(int32_t residentId)
{
    std::lock_guard lock(residentAbilityInfoMutex_);
    for (auto it = residentAbilityInfos_.begin(); it != residentAbilityInfos_.end(); ++it) {
        if (it->residentId == residentId) {
            residentAbilityInfos_.erase(it);
            return;
        }
    }
}

bool ResidentProcessManager::GetResidentBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);

    const auto &residentWhiteList = AppUtils::GetInstance().GetResidentWhiteList();
    if (userId == 0 || residentWhiteList.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "userId: %{public}d", userId);
        return IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfos(OHOS::AppExecFwk::GET_BUNDLE_DEFAULT,
            bundleInfos, userId));
    }

    for (const auto &bundleName: residentWhiteList) {
        AppExecFwk::BundleInfo bundleInfo;
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "failed get bundle info: %{public}s", bundleName.c_str());
            continue;
        }
        bundleInfos.push_back(bundleInfo);
    }

    return !bundleInfos.empty();
}

void ResidentProcessManager::StartFailedResidentAbilities()
{
    unlockedAfterBoot_ = true;
    std::list<ResidentAbilityInfo> tmpList;
    {
        std::lock_guard lock(failedResidentAbilityInfoMutex_);
        if (failedResidentAbilityInfos_.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "no failed abilities");
            return;
        }
        tmpList = std::move(failedResidentAbilityInfos_);
    }
    for (const auto &item: tmpList) {
        ResidentAbilityInfoGuard residentAbilityInfoGuard(item.bundleName, item.abilityName, item.userId);
        Want want;
        want.SetElementName(item.bundleName, item.abilityName);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "call, bundleName: %{public}s, mainElement: %{public}s",
            item.bundleName.c_str(), item.abilityName.c_str());
        DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, item.userId,
            DEFAULT_INVAL_VALUE);
    }
}

void ResidentProcessManager::AddFailedResidentAbility(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "failed bundleName: %{public}s, mainElement: %{public}s",
        bundleName.c_str(), abilityName.c_str());
    if (unlockedAfterBoot_) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "already unlocked");
        return;
    }

    std::lock_guard lock(failedResidentAbilityInfoMutex_);
    failedResidentAbilityInfos_.push_back(ResidentAbilityInfo {
        .bundleName = bundleName,
        .abilityName = abilityName,
        .userId = userId,
    });
}

void ResidentProcessManager::StartResidentMemoryDefragTask(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Deferring memory defrag task for %{public}s/%{public}s until first screen off",
        bundleName.c_str(), abilityName.c_str());

    // Store the task for later execution after first screen off
    std::lock_guard lock(pendingDeepCopyTasksMutex_);
    pendingDeepCopyTasks_.push_back(ResidentAbilityInfo {
        .bundleName = bundleName,
        .abilityName = abilityName,
        .userId = userId,
    });
}

void ResidentProcessManager::RegisterScreenStateCallback()
{
#ifdef SUPPORT_POWER
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Registering for screen state changes");
    // We'll poll for screen state changes in a background thread
    std::thread([self = shared_from_this()]() {
        bool wasScreenOn = true;
        while (true) {
            bool isScreenOn = PowerMgr::PowerMgrClient::GetInstance().IsScreenOn();
            if (wasScreenOn && !isScreenOn) {
                // Screen just turned off
                TAG_LOGI(AAFwkTag::ABILITYMGR, "Screen turned off detected");
                self->OnScreenStateChanged(false);
            }
            wasScreenOn = isScreenOn;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();
#else
    TAG_LOGW(AAFwkTag::ABILITYMGR, "SUPPORT_POWER not defined, cannot monitor screen state");
#endif
}

void ResidentProcessManager::OnScreenStateChanged(bool isScreenOn)
{
    if (isScreenOn) {
        return;  // We only care about screen off events
    }

    // Check if this is the first screen off
    bool expectedFirst = false;
    if (!isFirstScreenOffOccurred_.compare_exchange_strong(expectedFirst, true)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Screen off already occurred, ignoring");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "First screen off detected, triggering deep copy tasks");
    ExecutePendingDeepCopyTasks();
}

void ResidentProcessManager::ExecutePendingDeepCopyTasks()
{
    std::list<ResidentAbilityInfo> tasksToExecute;
    {
        std::lock_guard lock(pendingDeepCopyTasksMutex_);
        if (pendingDeepCopyTasks_.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "No pending deep copy tasks to execute");
            return;
        }
        tasksToExecute = std::move(pendingDeepCopyTasks_);
    }

    auto task = [tasks = std::move(tasksToExecute)]() {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Executing %{public}zu pending deep copy tasks", tasks.size());
        
        for (const auto& task : tasks) {
            auto ams = DelayedSingleton<AbilityManagerService>::GetInstance();
            if (ams == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get AbilityManagerService instance");
                continue;
            }

            // Get service map through public API GetServiceMapByUserId
            AbilityConnectManager::ServiceMapType serviceMap;
            int32_t ret = ams->GetServiceMapByUserId(serviceMap, task.userId);
            if (ret != ERR_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "Failed to get service map for userId %{public}d, skipping",
                    task.userId);
                continue;
            }

            // Create ElementName from bundleName and abilityName to build service key
            AppExecFwk::ElementName elementName("", task.bundleName, task.abilityName);
            std::string targetServiceKey = elementName.GetURI();

            // Search for matching extension record in service map
            std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
            for (const auto& [serviceKey, extensionRecord] : serviceMap) {
                if (extensionRecord == nullptr) {
                    continue;
                }
                // Check if the serviceKey contains both bundleName and abilityName
                if (serviceKey.find(task.bundleName) != std::string::npos &&
                    serviceKey.find(task.abilityName) != std::string::npos) {
                    abilityRecord = std::static_pointer_cast<AbilityRecord>(extensionRecord);
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "Found ServiceExtension for %{public}s/%{public}s",
                        task.bundleName.c_str(), task.abilityName.c_str());
                    break;
                }
            }

            if (abilityRecord == nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "ServiceExtension not found for %{public}s/%{public}s, skipping",
                    task.bundleName.c_str(), task.abilityName.c_str());
                continue;
            }

            if (!abilityRecord->IsResidentAbility()) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityRecord no longer marked as resident, skipping");
                continue;
            }

            TAG_LOGI(AAFwkTag::ABILITYMGR, "Performing memory defrag for %{public}s/%{public}s",
                task.bundleName.c_str(), task.abilityName.c_str());

            // Create a deep copy to trigger memory reallocation
            auto deepCopiedRecord = abilityRecord->DeepCopy();

            TAG_LOGI(AAFwkTag::ABILITYMGR, "Deep copy completed for %{public}s/%{public}s",
                task.bundleName.c_str(), task.abilityName.c_str());

            // Clear the resident ability flag to avoid repeated execution
            abilityRecord->SetResidentAbilityFlag(false);
        }
    };

    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetTaskHandler();
    if (handler != nullptr) {
        handler->SubmitTask(task, 100);  // Delay 100ms as requested
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Scheduled deep copy tasks to execute after 100ms");
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get task handler, executing immediately");
        task();
    }
}

}  // namespace AAFwk
}  // namespace OHOS
