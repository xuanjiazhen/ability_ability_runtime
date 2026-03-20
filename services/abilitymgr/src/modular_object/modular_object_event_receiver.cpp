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
const std::string IS_SYSTEM_DISABLED = "isSystemDisabled";
} // namespace

ModularObjectEventReceiver::ModularObjectEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ModularObjectExtension sys event receiver created");
}

void ModularObjectEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const std::string action = data.GetWant().GetAction();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Received event action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED) {
        HandleBundleScanFinished(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_ADDED) {
        HandleBundleInstall(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        HandleBundleRemoved(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_CHANGED) {
        HandleBundleChanged(data);
    } else {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Unhandled action: %{public}s", action.c_str());
    }
}

void ModularObjectEventReceiver::LoadModularObjectExtensionInfos()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadModularObjectExtensionInfos enter");
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleMgrHelper");
        return;
    }

    int32_t userId = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "init modular object, userId: %{public}d", userId);

    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfos(AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES,
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get bundle info failed");
        return;
    }
    // std::string key = std::to_string(userId) + bundleName + std::to_string(appIndex);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "bundleInfos size: %{public}zu", bundleInfos.size());
    for (const auto &bundleInfo : bundleInfos) {
        std::vector<AAFwk::ModularObjectExtensionInfo> infos;
        GetModularObjectExtensionInfos(bundleInfo, infos);
        // db
        // DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->InsertData(key, infos);
        infos.clear();
    }
}

void ModularObjectEventReceiver::HandleBundleScanFinished(const EventFwk::CommonEventData &data)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleBundleScanFinished enter");
    auto task = [self = shared_from_this()]() { self->LoadModularObjectExtensionInfos(); };
    ffrt::submit(task);
}

void ModularObjectEventReceiver::HandleBundleInstall(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want& want = data.GetWant();
    std::string bundleName = want.GetElement().GetBundleName();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle common event package add, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle common event package remove, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle common event package changed, bundleName: %{public}s", bundleName.c_str());
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = want.GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BundleMgrHelper is null");
        return;
    }

    AppExecFwk::BundleInfo bundleInfo;
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfo failed, ret=%{public}d", ret);
        return;
    }

    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    GetModularObjectExtensionInfos(bundleInfo, infos);
    if (infos.empty()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Get bundle info failed, bundleName: %{public}s", bundleName.c_str());
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
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfo(bundleName,
        AppExecFwk::BundleFlag::GET_BUNDLE_WITH_ABILITIES, bundleInfo, userId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfo failed, ret=%{public}d", ret);
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
        for (const auto &metadata : extensionInfo.metadata) {
            if (metadata.name == IS_SYSTEM_DISABLED) {
                AAFwk::ModularObjectExtensionInfo info;
                info.bundleName = extensionInfo.bundleName;
                info.moduleName = extensionInfo.moduleName;
                info.abilityName = extensionInfo.name;
                info.appIndex = extensionInfo.appIndex;
                info.isSystemDisabled = metadata.value == "true" ? true : false;
                infos.emplace_back(info);
            }
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS