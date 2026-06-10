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

#include "insight_intent_event_mgr.h"

#include "insight_intent_sys_event_receiver.h"
#include "insight_intent_db_cache.h"
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "extract_insight_intent_profile.h"
#include "os_account_manager_wrapper.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "event_report.h"
#include "ffrt.h"

namespace OHOS {
namespace AbilityRuntime {
void InsightIntentEventMgr::DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName,
    int32_t userId)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(
        bundleName, userId, intentInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(
        bundleName, userId, configIntentInfos);
    if (!intentInfos.empty() || !configIntentInfos.empty()) {
        TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent", bundleName.c_str());
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
            bundleName, moduleName, userId);
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
    }

    return;
}

void InsightIntentEventMgr::UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId)
{
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleName();
    if (bundleName.empty() || moduleName.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "input param empty, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    if (userId < 0) {
        TAG_LOGW(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }
    ffrt::submit([bundleName, moduleName, userId] () {
        HandleInsightIntentUpdateInner(bundleName, moduleName, userId);
    });
}

void InsightIntentEventMgr::HandleInsightIntentUpdateInner(
    const std::string &bundleName, const std::string &moduleName, int32_t userId)
{
    std::vector<std::string> moduleNameVec;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundleMgrHelper instance failed");
        return;
    }
    AppExecFwk::BundleInfo bundleInfo;
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_DEFAULT),
        bundleInfo, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundle info failed");
        return;
    }
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (const auto &moduleNameLocal : moduleNameVec) {
        ProcessModuleIntentUpdate(bundleName, userId, bundleInfo.versionCode, moduleNameLocal);
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::ProcessModuleIntentUpdate(
    const std::string &bundleName, int32_t userId, uint32_t versionCode,
    const std::string &moduleNameLocal)
{
    ErrCode ret;
    std::string profile;
    AbilityRuntime::ExtractInsightIntentProfileInfoVec infos = {};
    std::vector<InsightIntentInfo> configIntentInfos = {};
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        return;
    }
    ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE,
        bundleName, moduleNameLocal, profile, userId));
    if (ret != ERR_OK) {
        TAG_LOGI(AAFwkTag::INTENT, "get json failed code: %{public}d", ret);
        DeleteInsightIntent(bundleName, moduleNameLocal, userId);
        return;
    }
    bool isExtractFailed = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(
        profile, infos) || infos.insightIntents.size() == 0);
    bool isConfigFailed = (!AbilityRuntime::InsightIntentProfile::TransformTo(
        profile, configIntentInfos) || configIntentInfos.size() == 0);
    if (isExtractFailed && isConfigFailed) {
        TAG_LOGW(AAFwkTag::INTENT, "transform profile failed");
        DeleteInsightIntent(bundleName, moduleNameLocal, userId);
        return;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
        bundleName, moduleNameLocal, userId, versionCode, infos, configIntentInfos);
    AAFwk::EventInfo installInfo;
    installInfo.userId = userId;
    installInfo.bundleName = bundleName;
    installInfo.moduleName = moduleNameLocal;
    if (!isExtractFailed) {
        for (const auto &it : infos.insightIntents) {
            installInfo.intentNameList.emplace_back(it.intentName);
        }
    }
    if (!isConfigFailed) {
        for (const auto &it : configIntentInfos) {
            installInfo.intentNameList.emplace_back(it.intentName);
        }
    }
    AAFwk::EventReport::SendInsightIntentInfoEvent(
        AAFwk::EventName::INSIGHT_INTENT_INSTALL_INFO, HISYSEVENT_BEHAVIOR, installInfo);
}

void InsightIntentEventMgr::DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
    int32_t appIndex)
{
    ErrCode ret;
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleName();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "input bundleName empty, bundleName: %{public}s, moduleName: %{public}s",
            bundleName.c_str(), moduleName.c_str());
        return;
    }

    if (appIndex > 0) {
        TAG_LOGI(AAFwkTag::INTENT, "this application is a simulation, not support to delete intent info, "
            "bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
        return;
    }

    if (userId < 0) {
        TAG_LOGI(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }

    ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "delete intent info failed, bundleName: %{public}s, "
            "moduleName: %{public}s, userId: %{public}d", bundleName.c_str(), moduleName.c_str(), userId);
        return;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
    TAG_LOGI(AAFwkTag::INTENT, "delete intent info success, bundleName: %{public}s, "
        "moduleName: %{public}s, userId: %{public}d", bundleName.c_str(), moduleName.c_str(), userId);
    AAFwk::EventInfo uninstallInfo;
    uninstallInfo.userId = userId;
    uninstallInfo.bundleName = bundleName;
    uninstallInfo.moduleName = moduleName;
    AAFwk::EventReport::SendInsightIntentInfoEvent(
        AAFwk::EventName::INSIGHT_INTENT_UNINSTALL_INFO, HISYSEVENT_BEHAVIOR, uninstallInfo);
}

void InsightIntentEventMgr::SubscribeSysEventReceiver()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    subscribeInfo.SetThreadMode(EventFwk::CommonEventSubscribeInfo::COMMON);
    insightIntentSysEventReceiver_ = std::make_shared<AbilityRuntime::InsightIntentSysEventReceiver>(subscribeInfo);
    bool subResult = EventFwk::CommonEventManager::SubscribeCommonEvent(insightIntentSysEventReceiver_);
    if (!subResult) {
        TAG_LOGE(AAFwkTag::INTENT, "subscribe common event failed");
        return;
    }

    TAG_LOGI(AAFwkTag::INTENT, "subscribe common event success");
}

} // namespace AbilityRuntime
} // namespace OHOS
