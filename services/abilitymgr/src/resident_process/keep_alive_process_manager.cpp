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

#include "keep_alive_process_manager.h"

#include "ability_manager_service.h"
#include "ability_util.h"
#include "keep_alive_utils.h"
#include "main_element_utils.h"

namespace OHOS {
namespace AAFwk {
KeepAliveProcessManager::KeepAliveProcessManager()
{
    abilityKeepAliveService_ = std::make_shared<AbilityRuntime::AbilityKeepAliveService>();
}

KeepAliveProcessManager::~KeepAliveProcessManager()
{
    abilityKeepAliveService_.reset();
    abilityKeepAliveService_ = nullptr;
}

void KeepAliveProcessManager::StartKeepAliveProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    for (const auto &bundleInfo : bundleInfos) {
        StartKeepAliveProcessWithMainElementPerBundle(bundleInfo, userId);
    }
}

void KeepAliveProcessManager::StartKeepAliveProcessWithMainElementPerBundle(const AppExecFwk::BundleInfo &bundleInfo,
    int32_t userId)
{
    if (abilityKeepAliveService_ == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "abilityKeepAliveService_ null");
        return;
    }
    bool keepAliveEnable = false;
    (void)abilityKeepAliveService_->GetKeepAliveProcessEnable(bundleInfo.name, userId, keepAliveEnable);
    if (!keepAliveEnable) {
        return;
    }

    std::string mainElementName;
    if (!MainElementUtils::CheckMainUIAbility(bundleInfo, userId, mainElementName)) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "bundle has no main uiability");
        return;
    }
    // startAbility
    Want want;
    want.SetElementName(bundleInfo.name, mainElementName);
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "call, bundleName: %{public}s, mainElement: %{public}s",
        bundleInfo.name.c_str(), mainElementName.c_str());
    auto ret = DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, userId,
        DEFAULT_INVAL_VALUE);
    MainElementUtils::UpdateMainElement(bundleInfo.name, bundleInfo.entryModuleName, mainElementName, true, userId);
    if (ret != ERR_OK) {
        AddFailedKeepAliveAbility(bundleInfo.name, mainElementName, userId);
    }
}

int32_t KeepAliveProcessManager::SetApplicationKeepAlive(const std::string &bundleName,
    int32_t userId, bool updateEnable, bool isByEDM)
{
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "input parameter error");
        return INVALID_PARAMETERS_ERR;
    }

    auto bms = AbilityUtil::GetBundleManagerHelper();
    if (bms == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "obtain bms handle failed");
        return INNER_ERR;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (userId == -1) {
        auto abilityMgr = DelayedSingleton<AbilityManagerService>::GetInstance();
        if (abilityMgr == nullptr) {
            TAG_LOGE(AAFwkTag::KEEP_ALIVE, "abilityMgr null");
            return INNER_ERR;
        }
        userId = abilityMgr->GetUserId();
    }

    if (!IN_PROCESS_CALL(bms->GetBundleInfo(
        bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, bundleInfo, userId))) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "get bundle info failed");
        return ERR_INVALID_VALUE;
    }

    CHECK_POINTER_AND_RETURN(abilityKeepAliveService_, INNER_ERR);

    bool localEnable = false;
    auto result = abilityKeepAliveService_->GetKeepAliveProcessEnable(bundleName, userId, localEnable);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetKeepAliveProcessEnable failed:%{public}d", result);
        return INNER_ERR;
    }

    if (updateEnable == localEnable) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "no change to the KeepAlive process setting properties");
        return ERR_OK;
    }

    KeepAliveInfo info;
    info.bundleName = bundleName;
    info.userId = userId;
    if (isByEDM) {
        result = abilityKeepAliveService_->SetApplicationKeepAliveByEDM(info, updateEnable);
    } else {
        result = abilityKeepAliveService_->SetApplicationKeepAlive(info, updateEnable);
    }
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "KeepAlive process attribute update failed");
        return INNER_ERR;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient != nullptr) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Set keep alive enable state.");
        IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveEnableState(bundleName, updateEnable, 0));
    }

    UpdateKeepAliveProcessesStatus(bundleInfo, userId, localEnable, updateEnable);\
    return ERR_OK;
}

void KeepAliveProcessManager::UpdateKeepAliveProcessesStatus(
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId, bool localEnable, bool updateEnable)
{
    if (!updateEnable && localEnable) {
        // just update
        std::vector<AppExecFwk::BundleInfo> bundleInfos{ bundleInfo };
        KeepAliveUtils::NotifyDisableKeepAliveProcesses(bundleInfos, userId);
    }
}

void KeepAliveProcessManager::OnAppStateChanged(const AppInfo &info)
{
    if (info.state != AppState::BEGIN) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "Not a state of concern. state: %{public}d", info.state);
        return;
    }

    if (info.pid <= 0) {
        TAG_LOGD(AAFwkTag::KEEP_ALIVE, "The obtained application pid is incorrect. state: %{public}d", info.pid);
        return;
    }

    // user 0
    int32_t uid = 0;
    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "app scheduler error");
        return;
    }
    std::string bundleName;
    appScheduler->GetBundleNameByPid(info.pid, bundleName, uid);
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "get bundle name by pid failed");
        return;
    }

    if (abilityKeepAliveService_ == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "abilityKeepAliveService_ null");
        return;
    }
    bool localEnable = false;
    auto result = abilityKeepAliveService_->GetKeepAliveProcessEnable(bundleName, -1, localEnable);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "GetKeepAliveProcessEnable failed: %{public}d", result);
        return;
    }

    auto appMgrClient = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "set keep alive enable state error");
        return;
    }
    IN_PROCESS_CALL_WITHOUT_RET(appMgrClient->SetKeepAliveEnableState(bundleName, localEnable, uid));
}

bool KeepAliveProcessManager::IsKeepAliveBundle(const std::string &bundleName, int32_t userId)
{
    CHECK_POINTER_AND_RETURN(abilityKeepAliveService_, false);
    bool keepAliveEnable = false;
    abilityKeepAliveService_->GetKeepAliveProcessEnable(bundleName, userId, keepAliveEnable);
    return keepAliveEnable;
}

bool KeepAliveProcessManager::GetKeepAliveBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos,
    int32_t userId)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);

    CHECK_POINTER_AND_RETURN(abilityKeepAliveService_, false);

    std::vector<KeepAliveInfo> infoList;
    auto ret = abilityKeepAliveService_->GetKeepAliveApplications(userId, infoList);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::KEEP_ALIVE, "failed get keep_alive bundle info: %{public}d", ret);
        return false;
    }
    for (const auto &info: infoList) {
        AppExecFwk::BundleInfo bundleInfo;
        if (!IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(info.bundleName,
            AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId))) {
            TAG_LOGW(AAFwkTag::KEEP_ALIVE, "failed get bundle info: %{public}s", info.bundleName.c_str());
            continue;
        }
        bundleInfos.push_back(bundleInfo);
    }

    return !bundleInfos.empty();
}

void KeepAliveProcessManager::StartFailedKeepAliveAbilities()
{
    unlockedAfterBoot_ = true;
    std::list<KeepAliveAbilityInfo> tmpList;
    {
        std::lock_guard lock(failedKeepAliveAbilityInfoMutex_);
        if (failedKeepAliveAbilityInfos_.empty()) {
            TAG_LOGI(AAFwkTag::KEEP_ALIVE, "no failed abilities");
            return;
        }
        tmpList = std::move(failedKeepAliveAbilityInfos_);
    }
    for (const auto &item: tmpList) {
        Want want;
        want.SetElementName(item.bundleName, item.abilityName);
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "call, bundleName: %{public}s, mainElement: %{public}s",
            item.bundleName.c_str(), item.abilityName.c_str());
        DelayedSingleton<AbilityManagerService>::GetInstance()->StartAbility(want, item.userId,
            DEFAULT_INVAL_VALUE);
    }
}

void KeepAliveProcessManager::AddFailedKeepAliveAbility(const std::string &bundleName,
    const std::string &abilityName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::KEEP_ALIVE, "failed bundleName: %{public}s, mainElement: %{public}s",
        bundleName.c_str(), abilityName.c_str());
    if (unlockedAfterBoot_) {
        TAG_LOGI(AAFwkTag::KEEP_ALIVE, "already unlocked");
        return;
    }

    std::lock_guard lock(failedKeepAliveAbilityInfoMutex_);
    failedKeepAliveAbilityInfos_.push_back(KeepAliveAbilityInfo {
        .bundleName = bundleName,
        .abilityName = abilityName,
        .userId = userId,
    });
}

int32_t KeepAliveProcessManager::QueryKeepAliveApplications(int32_t appType, int32_t userId,
    std::vector<KeepAliveInfo> &infoList, bool isByEDM)
{
    CHECK_POINTER_AND_RETURN(abilityKeepAliveService_, INNER_ERR);
    if (isByEDM) {
        return abilityKeepAliveService_->QueryKeepAliveApplicationsByEDM(userId, appType, infoList);
    }
    return abilityKeepAliveService_->QueryKeepAliveApplications(userId, appType, infoList);
}
}  // namespace AAFwk
}  // namespace OHOS
