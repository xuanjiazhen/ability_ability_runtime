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
//#include "modular_object_db_cache.h"
#include "os_account_manager_wrapper.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string KEY_USER_ID = "userId";
const std::string KEY_APP_INDEX = "appIndex";
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
    int32_t userId = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "init modular object, userId: %{public}d", userId);

    // whether judge ModularObjectDbCache ready

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null bundleMgrHelper");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfosV9(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get bundle info failed");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "bundleInfos size: %{public}zu", bundleInfos.size());
    for (auto &bundleInfo : bundleInfos) {
        // whether need hapModuleInfos
        // for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
        // }

        // has ModularObjectExtensionInfo
        // to db cache
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle package added");
    std::string bundleName = data.GetWant().GetElement().GetBundleName();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = data.GetWant().GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, 0);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->UpdateModularObjectExtensionInfo(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::HandleBundleRemoved(const EventFwk::CommonEventData &data)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle package removed");
    std::string bundleName = data.GetWant().GetElement().GetBundleName();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = data.GetWant().GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, -1);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->RemoveModularObjectExtensionInfo(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::HandleBundleChanged(const EventFwk::CommonEventData &data)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle package changed");
    std::string bundleName = data.GetWant().GetElement().GetBundleName();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Bundle name is empty");
        return;
    }
    int32_t userId = data.GetWant().GetIntParam(KEY_USER_ID, -1);
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid userId: %{public}d", userId);
        return;
    }
    int32_t appIndex = data.GetWant().GetIntParam(KEY_APP_INDEX, 0);

    ffrt::submit([self = shared_from_this(), bundleName, userId, appIndex]() {
        self->UpdateModularObjectExtensionInfo(bundleName, userId, appIndex);
    });
}

void ModularObjectEventReceiver::UpdateModularObjectExtensionInfo(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ttt UpdateModularObjectExtensionInfo enter");
    AAFwk::ModularObjectExtensionInfo info;
    if(GetModularObjectExtensionInfo(bundleName, userId, appIndex, info)) {
        // db cache
    }
}

void ModularObjectEventReceiver::RemoveModularObjectExtensionInfo(const std::string &bundleName, int32_t userId, int32_t appIndex)
{
    AAFwk::ModularObjectExtensionInfo info;
    if(GetModularObjectExtensionInfo(bundleName, userId, appIndex, info)) {
        // db cache
    }
}

bool ModularObjectEventReceiver::GetModularObjectExtensionInfo(const std::string &bundleName, int32_t userId, int32_t appIndex,
    AAFwk::ModularObjectExtensionInfo &info)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BundleMgrHelper is null");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    uint32_t flags = static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY) |
                     static_cast<uint32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName, flags, bundleInfo, userId));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfoV9 failed, ret=%{public}d", ret);
        return false;
    }

    for (const auto &extensionInfo : bundleInfo.extensionInfos) {
        // ExtensionAbilityType::SERVICE => ExtensionAbilityType::ModularObjectExtensionType
        if (extensionInfo.type != AppExecFwk::ExtensionAbilityType::SERVICE) { 
            continue;
        }
        bool isModularObject = false;
        // whether extensionInfo.metadata
        for (const auto &metadata : extensionInfo.metadata) {
            if (metadata.name == "modularObjectExtension") {
                isModularObject = true;
                break;
            }
        }
        if (!isModularObject) {
            continue;
        }

        info.bundleName = bundleName;
        info.moduleName = extensionInfo.moduleName;
        info.abilityName = extensionInfo.name;
        info.appIndex = appIndex;
        info.launchMode = AAFwk::LaunchMode::IN_PROCESS;
        info.processMode = AAFwk::ModularProcessMode::BUNDLE;
        info.threadMode = AAFwk::ThreadMode::BUNDLE;
        info.isSelfDisabled = false;
        info.isSystemDisabled = false;
        return true;
    }
    return false;
}
} // namespace AbilityRuntime
} // namespace OHOS