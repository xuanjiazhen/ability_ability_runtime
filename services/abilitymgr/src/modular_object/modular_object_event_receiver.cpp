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

#include "modular_object_event_receiver.h"

#include "bundle_mgr_helper.h"
#include "common_event_support.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "modular_object_rdb_storage_mgr.h"
#include "os_account_manager_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string KEY_USER_ID = "userId";
const std::string KEY_APP_INDEX = "appIndex";
const std::string IS_SELF_DISABLED = "isSelfDisabled";
const std::string PROCESS_MODE = "processMode";
const std::string THREAD_MODE = "threadMode";
const std::string LAUNCH_MODE = "launchMode";
} // namespace

ModularObjectEventReceiver::ModularObjectEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ModularObjectExtension sys event receiver created");
}

void ModularObjectEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "received event action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED) {
        HandleBundleScanFinished(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        HandleBundleInstall(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        HandleBundleRemoved(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        HandleBundleChanged(data);
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "unhandled action: %{public}s", action.c_str());
    }
}

void ModularObjectEventReceiver::LoadModularObjectExtensionInfos()
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleMgrHelper");
        return;
    }

    int32_t userId = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "init modular object, userId: %{public}d", userId);

    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfos(AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get bundle infos failed");
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleInfos size: %{public}zu", bundleInfos.size());
    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    for (const auto &bundleInfo : bundleInfos) {
        GetModularObjectExtensionInfos(bundleInfo, infos);
        std::string key = std::to_string(userId) + bundleInfo.name + std::to_string(bundleInfo.appIndex);
        DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->InsertData(key, infos);
        infos.clear();
    }
}

void ModularObjectEventReceiver::HandleBundleScanFinished(const EventFwk::CommonEventData &data)
{
    auto task = [self = shared_from_this()]() { self->LoadModularObjectExtensionInfos(); };
    ffrt::submit(task);
}

void ModularObjectEventReceiver::HandleBundleInstall(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want& want = data.GetWant();
    std::string bundleName = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle common event package add, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, 0);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->InsertModularObjectExtensionInfo(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::HandleBundleRemoved(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want& want = data.GetWant();
    std::string bundleName = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle common event package remove, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, 0);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->RemoveModularObjectExtensionInfo(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::HandleBundleChanged(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want& want = data.GetWant();
    std::string bundleName = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "handle common event package changed, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, 0);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->UpdateModularObjectExtensionInfos(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::InsertModularObjectExtensionInfo(
    const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleMgrHelper is null");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, userId))) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get bundle info failed, bundleName: %{public}s", bundleName.c_str());
        return;
    }

    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    GetModularObjectExtensionInfos(bundleInfo, infos);
    if (infos.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not include modular extension, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    std::string key = std::to_string(userId) + bundleName + std::to_string(appIndex);
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->InsertData(key, infos);
}

void ModularObjectEventReceiver::UpdateModularObjectExtensionInfos(const std::string &bundleName, int32_t userId,
    int32_t appIndex)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BundleMgrHelper is null");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfo, userId))) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "get bundle info failed, bundleName: %{public}s", bundleName.c_str());
        return;
    }

    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    GetModularObjectExtensionInfos(bundleInfo, infos);
    std::string key = std::to_string(userId) + bundleName + std::to_string(appIndex);
    if (infos.empty()) {
        DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->DeleteData(key);
        return;
    }
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->UpdateData(key, infos);
}

void ModularObjectEventReceiver::RemoveModularObjectExtensionInfo(
    const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    std::string key = std::to_string(userId) + bundleName + std::to_string(appIndex);
    DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->DeleteData(key);
}

void ModularObjectEventReceiver::GetModularObjectExtensionInfos(const AppExecFwk::BundleInfo &bundleInfo,
    std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    for (const auto &extensionInfo : bundleInfo.extensionInfos) {
        if (extensionInfo.type != AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT) {
            continue;
        }
        AAFwk::ModularObjectExtensionInfo info;
        info.bundleName = extensionInfo.bundleName;
        info.moduleName = extensionInfo.moduleName;
        info.abilityName = extensionInfo.name;
        info.appIndex = extensionInfo.appIndex;
        for (const auto &metadata : extensionInfo.metadata) {
            if (metadata.name == IS_SELF_DISABLED) {
                info.isSelfDisabled = metadata.value == "true" ? true : false;
            }
            if (metadata.name == LAUNCH_MODE) {
                if (metadata.value == "IN_PROCESS") {
                    info.launchMode = LaunchMode::IN_PROCESS;
                } else if (metadata.value == "CROSS_PROCESS") {
                    info.launchMode = LaunchMode::CROSS_PROCESS;
                }
            }
            if (metadata.name == THREAD_MODE) {
                if (metadata.value == "BUNDLE") {
                    info.threadMode = ThreadMode::BUNDLE;
                } else if (metadata.value == "TYPE") {
                    info.threadMode = ThreadMode::TYPE;
                } else if (metadata.value == "INSTANCE") {
                    info.threadMode = ThreadMode::INSTANCE;
                }
            }
            if (metadata.name == PROCESS_MODE) {
                if (metadata.value == "BUNDLE") {
                    info.processMode = ThreadMode::BUNDLE;
                } else if (metadata.value == "TYPE") {
                    info.processMode = ThreadMode::TYPE;
                } else if (metadata.value == "INSTANCE") {
                    info.processMode = ThreadMode::INSTANCE;
                }
            }
        }
        infos.emplace_back(info);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS