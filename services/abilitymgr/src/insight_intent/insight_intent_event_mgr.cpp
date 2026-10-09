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

#include <unordered_map>

#include "insight_intent_sys_event_receiver.h"
#include "insight_intent_db_cache.h"
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "function_call_convert.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "extract_insight_intent_profile.h"
#include "os_account_manager_wrapper.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "app_utils.h"
#include "ffrt.h"
#include "global_constant.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
InsightIntentEventMgr::DualModeContext ResolveDualModeContext(
    const std::string &bundleName, int32_t userId, int32_t appIndexFromEvent)
{
    InsightIntentEventMgr::DualModeContext ctx;
    ctx.appIndex = appIndexFromEvent;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "bundleMgrHelper null, fallback same-name-same-package");
        return ctx;
    }
    AppExecFwk::BundleInfo bundleInfo;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfoV9(bundleName,
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_DEFAULT),
        bundleInfo, userId));
    if (ret == ERR_OK) {
        auto policy = bundleInfo.deviceModeDistributionPolicy;
        bool isDifferentPackage =
            (policy == AppExecFwk::DeviceModeDistributionPolicy::UNIVERSAL_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE ||
             policy == AppExecFwk::DeviceModeDistributionPolicy::FULL_COMPATIBLE_DIFFERENT_PACKAGE);
        if (isDifferentPackage) {
            ctx.isDualMode = true;
            ctx.appIndex = bundleInfo.appIndex;
            TAG_LOGI(AAFwkTag::INTENT, "same-name-different-package app, bundle:%{public}s, "
                "policy:%{public}d, appIndex:%{public}d",
                bundleName.c_str(), static_cast<int32_t>(policy), bundleInfo.appIndex);
        } else {
            ctx.isDualMode = false;
            ctx.appIndex = 0;
            TAG_LOGI(AAFwkTag::INTENT, "same-name-same-package app, bundle:%{public}s, policy:%{public}d",
                bundleName.c_str(), static_cast<int32_t>(policy));
        }
    } else {
        TAG_LOGI(AAFwkTag::INTENT, "GetBundleInfoV9 failed, bundle:%{public}s, fallback by appIndex",
            bundleName.c_str());
    }
    return ctx;
}
} // namespace

void InsightIntentEventMgr::DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName,
    int32_t userId, const DualModeContext &ctx)
{
    if (!DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId, ctx.appIndex, ctx.isDualMode)) {
        return;
    }
    TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent", bundleName.c_str());
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
    int32_t appIndex)
{
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleNameRef();
    if (bundleName.empty() || moduleName.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "input param empty, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    if (userId < 0) {
        TAG_LOGW(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }
    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGI(AAFwkTag::INTENT, "BOPD or rescue mode, skip package update: %{public}s", bundleName.c_str());
        return;
    }
    auto ctx = ResolveDualModeContext(bundleName, userId, appIndex);
    ffrt::submit([moduleName, bundleName, userId, ctx] () {
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
        ExtractInsightIntentProfileInfoVec allInfos = {};
        std::vector<InsightIntentInfo> allConfigInfos = {};
        std::vector<InsightIntentSaveParam> saveParams;
        CollectModuleSaveParams(bundleName, moduleName, userId, ctx,
            bundleMgrHelper, saveParams, allInfos, allConfigInfos);
        if (allInfos.insightIntents.empty() && allConfigInfos.empty()) {
            ClearStaleIntentFunctions(bundleName, userId);
            return;
        }
        for (auto &sp : saveParams) {
            sp.appIndex = ctx.appIndex;
            sp.isDualMode = ctx.isDualMode;
        }
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveBatchInsightIntentTotalInfo(
            bundleName, userId, bundleInfo.versionCode, saveParams);
        FilterAndRegisterIntentFunctions(bundleName, userId, bundleInfo.versionCode, allInfos, allConfigInfos);
    });
}

void InsightIntentEventMgr::CollectModuleSaveParams(const std::string &bundleName,
    const std::string &moduleName, int32_t userId, const DualModeContext &ctx,
    const std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper,
    std::vector<InsightIntentSaveParam> &saveParams,
    ExtractInsightIntentProfileInfoVec &allInfos,
    std::vector<InsightIntentInfo> &allConfigInfos)
{
    std::vector<std::string> moduleNameVec;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (std::string moduleNameLocal : moduleNameVec) {
        std::string profile;
        ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE,
            bundleName, moduleNameLocal, profile, userId));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::INTENT, "get json failed code: %{public}d", ret);
            DeleteInsightIntent(bundleName, moduleNameLocal, userId, ctx);
            continue;
        }
        ExtractInsightIntentProfileInfoVec infos = {};
        std::vector<InsightIntentInfo> configIntentInfos = {};
        bool isTransformExtractIntent = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(
            profile, infos) || infos.insightIntents.size() == 0);
        bool isTransformConfigIntent = (!AbilityRuntime::InsightIntentProfile::TransformTo(
            profile, configIntentInfos) || configIntentInfos.size() == 0);
        if (isTransformExtractIntent && isTransformConfigIntent) {
            TAG_LOGW(AAFwkTag::INTENT, "transform profile failed, profile:%{public}s", profile.c_str());
            DeleteInsightIntent(bundleName, moduleNameLocal, userId, ctx);
            continue;
        }
        for (auto &item : configIntentInfos) {
            item.bundleName = bundleName;
            item.moduleName = moduleNameLocal;
        }
        for (auto &item : infos.insightIntents) {
            item.bundleName = bundleName;
            item.moduleName = moduleNameLocal;
        }
        InsightIntentSaveParam saveParam;
        saveParam.moduleName = moduleNameLocal;
        saveParam.profileInfos = infos;
        saveParam.configInfos = configIntentInfos;
        saveParams.emplace_back(std::move(saveParam));
        for (const auto &item : infos.insightIntents) {
            allInfos.insightIntents.push_back(item);
        }
        for (const auto &item : configIntentInfos) {
            allConfigInfos.push_back(item);
        }
    }
}

void InsightIntentEventMgr::ClearStaleIntentFunctions(const std::string &bundleName, int32_t userId)
{
    TAG_LOGI(AAFwkTag::INTENT, "no intent in new version, clear stale functions, bundle:%{public}s",
        bundleName.c_str());
    bool unregistered = CliTool::UnregisterInsightIntentFunctions(bundleName, userId);
    TAG_LOGI(AAFwkTag::INTENT, "unregister done, bundle:%{public}s, success:%{public}d",
        bundleName.c_str(), unregistered);
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::FilterAndRegisterIntentFunctions(const std::string &bundleName,
    int32_t userId, uint32_t versionCode,
    const ExtractInsightIntentProfileInfoVec &allInfos,
    std::vector<InsightIntentInfo> &allConfigInfos)
{
    TAG_LOGI(AAFwkTag::INTENT, "collected intents for batch update, profile:%{public}zu config:%{public}zu, "
        "bundle:%{public}s", allInfos.insightIntents.size(), allConfigInfos.size(), bundleName.c_str());
    std::vector<AbilityRuntime::ExtractInsightIntentInfo> genericInfos;
    for (const auto &profileInfo : allInfos.insightIntents) {
        AbilityRuntime::ExtractInsightIntentInfo generic;
        if (AbilityRuntime::ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, generic)) {
            genericInfos.push_back(std::move(generic));
        }
    }
    CliTool::IntentFilterUtil intentFilter;
    TAG_LOGI(AAFwkTag::INTENT, "before filter, generic:%{public}zu config:%{public}zu",
        genericInfos.size(), allConfigInfos.size());
    intentFilter.FilterGeneric(genericInfos);
    intentFilter.FilterConfig(allConfigInfos);
    TAG_LOGI(AAFwkTag::INTENT, "after filter, generic:%{public}zu config:%{public}zu, bundle:%{public}s",
        genericInfos.size(), allConfigInfos.size(), bundleName.c_str());
    CliTool::BatchUpdateInsightIntentFunctions(
        genericInfos, allConfigInfos, bundleName, versionCode, userId);
    TAG_LOGI(AAFwkTag::INTENT, "batch update request done, bundle:%{public}s", bundleName.c_str());
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentEventMgr::DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
    int32_t appIndex)
{
    auto bundleName = elementName.GetBundleName();
    auto moduleName = elementName.GetModuleNameRef();
    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "input bundleName empty, bundleName: %{public}s, moduleName: %{public}s",
            bundleName.c_str(), moduleName.c_str());
        return;
    }

    // 克隆/DLP/AI Agent 索引：维持原有跳过行为
    if (appIndex > 0 && appIndex != AbilityRuntime::GlobalConstant::PC_TABLET_INDEX) {
        TAG_LOGI(AAFwkTag::INTENT, "clone/dlp app, skip delete, "
            "bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
        return;
    }

    if (userId < 0) {
        TAG_LOGI(AAFwkTag::INTENT, "invalid userId: %{public}d", userId);
        return;
    }

    if (AAFwk::AppUtils::GetInstance().IsBopdOrRescueMode()) {
        TAG_LOGI(AAFwkTag::INTENT, "BOPD or rescue mode, skip package delete: %{public}s", bundleName.c_str());
        return;
    }

    // 卸载场景不能依赖 GetBundleInfoV9（应用已卸载），也不能依赖 HasBundleCache（缓存 key 格式不匹配）
    // 直接调存储层，同时尝试双模式和非双模式两种 Key 格式删除
    ffrt::submit([bundleName, moduleName, userId, appIndex]() {
        auto storage = DelayedSingleton<AbilityRuntime::InsightRdbStorageMgr>::GetInstance();
        // 尝试双模式 Key 删除（匹配 userId/bundleName/appIndex/... 格式的 Key）
        storage->DeleteStorageInsightIntentData(bundleName, moduleName, userId, appIndex, true);
        // 尝试非双模式 Key 删除（匹配 userId/bundleName/moduleName/... 格式的 Key）
        storage->DeleteStorageInsightIntentData(bundleName, moduleName, userId, 0, false);
        auto dbCache = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance();
        dbCache->BackupRdb();
        bool unregistered = CliTool::UnregisterInsightIntentFunctions(bundleName, userId);
        TAG_LOGI(AAFwkTag::INTENT, "delete done, bundleName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d, appIndex: %{public}d, unregistered: %{public}d",
            bundleName.c_str(), moduleName.c_str(), userId, appIndex, unregistered);
    });
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
