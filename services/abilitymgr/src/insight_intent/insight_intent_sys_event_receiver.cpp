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

#include "insight_intent_sys_event_receiver.h"

#include <unordered_map>

#include "extract_insight_intent_profile.h"
#include "common_event_support.h"
#include "function_call_convert.h"
#include "hilog_tag_wrapper.h"
#include "task_handler_wrap.h"
#include "os_account_manager_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ability_util.h"
#include "bundle_mgr_helper.h"
#include "insight_intent_db_cache.h"
#include "ffrt.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* INSIGHT_INTENT_SYS_EVENT_RECERVER = "InsightIntentSysEventReceiver";
const int32_t MAIN_USER_ID = 100;

InsightIntentSysEventReceiver::InsightIntentSysEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{
}

bool InsightIntentSysEventReceiver::SaveInsightIntentInfos(const std::string &bundleName, const std::string &moduleName,
    uint32_t versionCode, int32_t userId)
{
    std::vector<std::string> moduleNameVec;
    std::string profile;
    AbilityRuntime::ExtractInsightIntentProfileInfoVec infos = {};
    std::vector<InsightIntentInfo> configIntentInfos = {};
    TAG_LOGI(AAFwkTag::INTENT, "save insight intent infos, bundle:%{public}s module:%{public}s",
        bundleName.c_str(), moduleName.c_str());
    ErrCode ret;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return false;
    }

    bool anySaved = false;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (std::string moduleNameLocal : moduleNameVec) {
        // Get json profile firstly
        ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE, bundleName,
            moduleNameLocal, profile, userId));
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::INTENT, "GetJsonProfile failed, code: %{public}d", ret);
            if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName)) {
                DeleteInsightIntent(bundleName, moduleNameLocal, userId);
            }
            continue;
        }

        // Transform json string
        bool isTransformExtractIntent = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(profile, infos) ||
            infos.insightIntents.size() == 0);
        bool isTransformConfigIntent = (
            !AbilityRuntime::InsightIntentProfile::TransformTo(profile, configIntentInfos) ||
            configIntentInfos.size() == 0);
        if (isTransformExtractIntent && isTransformConfigIntent) {
            TAG_LOGW(AAFwkTag::INTENT,
                "transform profile failed, deleting config, bundle:%{public}s module:%{public}s",
                bundleName.c_str(), moduleNameLocal.c_str());
            DeleteInsightIntent(bundleName, moduleNameLocal, userId);
            continue;
        }

        // save database
        ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
            bundleName, moduleNameLocal, userId, versionCode, infos, configIntentInfos);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "save intent info failed, bundleName: %{public}s, moduleName: %{public}s, "
                "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
            continue;
        }
        anySaved = true;

        TAG_LOGI(AAFwkTag::INTENT, "save intent info success, bundleName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
    }
    return anySaved;
}

bool InsightIntentSysEventReceiver::ParseInsightIntentInfos(const std::string &bundleName,
    const std::string &moduleName, uint32_t versionCode, int32_t userId, IntentModuleStorageData &out)
{
    std::string profile;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return false;
    }
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE,
        bundleName, moduleName, profile, userId));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetJsonProfile failed, code: %{public}d", ret);
        if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName)) {
            DeleteInsightIntent(bundleName, moduleName, userId);
        }
        return false;
    }
    bool extractFailed = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(
        profile, out.profileInfos) || out.profileInfos.insightIntents.size() == 0);
    bool configFailed = (!AbilityRuntime::InsightIntentProfile::TransformTo(
        profile, out.configInfos) || out.configInfos.size() == 0);
    if (extractFailed && configFailed) {
        TAG_LOGW(AAFwkTag::INTENT, "transform profile failed, bundle:%{public}s module:%{public}s",
            bundleName.c_str(), moduleName.c_str());
        DeleteInsightIntent(bundleName, moduleName, userId);
        return false;
    }
    out.moduleName = moduleName;
    return true;
}

void InsightIntentSysEventReceiver::CollectNewBundles(
    const std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId,
    std::vector<std::pair<std::string, uint32_t>> &newBundles)
{
    auto scanStart = std::chrono::steady_clock::now();
    int32_t hitCacheCount = 0;
    int32_t reparseCount = 0;
    for (auto &bundleInfo : bundleInfos) {
        bool rdbHas = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
            HasInsightIntentByName(bundleInfo.versionCode, bundleInfo.name, userId);
        if (rdbHas) {
            newBundles.emplace_back(bundleInfo.name, bundleInfo.versionCode);
            ++hitCacheCount;
            continue;
        }
        std::vector<IntentModuleStorageData> modules;
        for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
            IntentModuleStorageData m;
            if (ParseInsightIntentInfos(bundleInfo.name, hapInfo.moduleName,
                bundleInfo.versionCode, userId, m)) {
                modules.push_back(std::move(m));
            }
        }
        if (modules.empty()) {
            continue;
        }
        auto saveStart = std::chrono::steady_clock::now();
        int32_t ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
            SaveBundleInsightIntentTotalInfo(bundleInfo.name, userId, bundleInfo.versionCode, modules);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "SaveBundleInsightIntentTotalInfo failed, bundle:%{public}s",
                bundleInfo.name.c_str());
            continue;
        }
        newBundles.emplace_back(bundleInfo.name, bundleInfo.versionCode);
        ++reparseCount;
        auto saveMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - saveStart).count();
        TAG_LOGI(AAFwkTag::INTENT, "[CRRR] bundle batch save done, bundle:%{public}s modules:%{public}zu "
            "elapsed: %{public}lld ms", bundleInfo.name.c_str(), modules.size(),
            static_cast<long long>(saveMs));
    }
    auto scanMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - scanStart).count();
    TAG_LOGI(AAFwkTag::INTENT, "[CRRR] scan bundles done, total:%{public}zu cache-hit:%{public}d "
        "reparse:%{public}d elapsed: %{public}lld ms", bundleInfos.size(), hitCacheCount, reparseCount,
        static_cast<long long>(scanMs));
}

void InsightIntentSysEventReceiver::RegisterAllFunctions(
    const std::vector<std::pair<std::string, uint32_t>> &newBundles,
    const std::vector<ExtractInsightIntentInfo> &allIntentInfos,
    const std::vector<InsightIntentInfo> &allConfigInfos)
{
    TAG_LOGI(AAFwkTag::INTENT, "register all functions, bundles:%{public}zu intent:%{public}zu config:%{public}zu",
        newBundles.size(), allIntentInfos.size(), allConfigInfos.size());
    if (allIntentInfos.empty() && allConfigInfos.empty()) {
        TAG_LOGW(AAFwkTag::INTENT, "register skip: no intent data");
        return;
    }
    std::unordered_map<std::string, uint32_t> bundleVersionMap;
    for (const auto &entry : newBundles) {
        bundleVersionMap[entry.first] = entry.second;
    }
    int32_t successCount = 0;
    CliTool::BatchRegisterInsightIntentFunctions(
        allIntentInfos, allConfigInfos, bundleVersionMap, successCount);
    TAG_LOGI(AAFwkTag::INTENT, "register all functions done, success: %{public}d", successCount);
}

void InsightIntentSysEventReceiver::DeleteInsightIntent(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfoByName(
        bundleName, userId, intentInfos);
    DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetConfigInsightIntentInfoByName(
        bundleName, userId, configIntentInfos);
    if (!intentInfos.empty() || !configIntentInfos.empty()) {
        TAG_LOGI(AAFwkTag::INTENT, "update bundleName: %{public}s to no insight intent",
            bundleName.c_str());
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
            bundleName, moduleName, userId);
        CliTool::UnregisterInsightIntentFunctions(bundleName);
    }
}

int32_t InsightIntentSysEventReceiver::ResolveLoadUserId(int32_t userId)
{
    if (userId != -1) {
        return userId;
    }
    int32_t current = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
    if (current == 0) {
        TAG_LOGI(AAFwkTag::INTENT, "use MAIN_USER_ID(%{public}d) instead of current userId: (%{public}d)",
            MAIN_USER_ID, current);
        return MAIN_USER_ID;
    }
    return current;
}

void InsightIntentSysEventReceiver::BackupAndScheduleRegister(
    std::vector<std::pair<std::string, uint32_t>> &&newBundles, int32_t userId)
{
    auto backupStart = std::chrono::steady_clock::now();
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
    auto backupMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - backupStart).count();
    TAG_LOGI(AAFwkTag::INTENT, "[CRRR] BackupRdb done, elapsed: %{public}lld ms",
        static_cast<long long>(backupMs));
    auto self = shared_from_this();
    auto task = [self, newBundles = std::move(newBundles), userId]() {
        auto registerStart = std::chrono::steady_clock::now();
        std::vector<ExtractInsightIntentInfo> allIntentInfos;
        std::vector<InsightIntentInfo> allConfigInfos;
        DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetAllInsightIntentInfoForRegister(
            userId, allIntentInfos, allConfigInfos);
        TAG_LOGI(AAFwkTag::INTENT, "[CRRR] GetAllForRegister done, intent:%{public}zu config:%{public}zu "
            "bundles:%{public}zu", allIntentInfos.size(), allConfigInfos.size(), newBundles.size());
        self->RegisterAllFunctions(newBundles, allIntentInfos, allConfigInfos);
        auto now = std::chrono::steady_clock::now();
        auto totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - self->bootReceiveTime_).count();
        auto registerMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - registerStart).count();
        TAG_LOGI(AAFwkTag::INTENT, "[CRRR] register done, elapsed: %{public}lld ms, "
            "BUNDLE_SCAN_FINISHED total elapsed: %{public}lld ms",
            static_cast<long long>(registerMs), static_cast<long long>(totalMs));
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::LoadInsightIntentInfos(int32_t userId)
{
    auto loadStart = std::chrono::steady_clock::now();
    userId = ResolveLoadUserId(userId);
    auto initStart = std::chrono::steady_clock::now();
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);
    auto initMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - initStart).count();
    TAG_LOGI(AAFwkTag::INTENT, "[CRRR] InitInsightIntentCache done, userId:%{public}d elapsed: %{public}lld ms",
        userId, static_cast<long long>(initMs));

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return;
    }

    auto queryStart = std::chrono::steady_clock::now();
    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfosV9(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundle info failed");
        return;
    }
    auto queryMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - queryStart).count();
    TAG_LOGI(AAFwkTag::INTENT, "[CRRR] GetBundleInfosV9 done, count:%{public}zu elapsed: %{public}lld ms",
        bundleInfos.size(), static_cast<long long>(queryMs));

    std::vector<std::pair<std::string, uint32_t>> newBundles;
    CollectNewBundles(bundleInfos, userId, newBundles);
    if (!newBundles.empty()) {
        BackupAndScheduleRegister(std::move(newBundles), userId);
    }
    auto loadMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - loadStart).count();
    TAG_LOGI(AAFwkTag::INTENT, "[CRRR] LoadInsightIntentInfos done, elapsed: %{public}lld ms",
        static_cast<long long>(loadMs));
}

void InsightIntentSysEventReceiver::DeleteInsightIntentInfoByUserId(int32_t userId)
{
    int32_t ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->DeleteInsightIntentByUserId(
        userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "delete insight intent by userId failed, userId: %{public}d, ret: %{public}d",
            userId, ret);
        return;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
}

void InsightIntentSysEventReceiver::HandleBundleScanFinished()
{
    auto task = [self = shared_from_this()]() {
        auto taskStart = std::chrono::steady_clock::now();
        auto delayMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            taskStart - self->bootReceiveTime_).count();
        TAG_LOGI(AAFwkTag::INTENT, "[CRRR] HandleBundleScanFinished task start, "
            "ffrt delay from receive: %{public}lld ms", static_cast<long long>(delayMs));
        self->LoadInsightIntentInfos();
        auto loadDoneMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - self->bootReceiveTime_).count();
        TAG_LOGI(AAFwkTag::INTENT,
            "[CRRR] HandleBundleScanFinished load done, total elapsed: %{public}lld ms",
            static_cast<long long>(loadDoneMs));
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::HandleUserSwitched(const EventFwk::CommonEventData &data)
{
    int32_t userId = data.GetCode();
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid switched userId: %{public}d", userId);
    }

    std::lock_guard<std::mutex> lock(userIdMutex_);
    if (userId == lastUserId_) {
        TAG_LOGE(AAFwkTag::INTENT, "same userId: %{public}d", lastUserId_);
        return;
    }

    TAG_LOGI(AAFwkTag::INTENT, "userId: %{public}d switch to  current userId: %{public}d", lastUserId_, userId);
    lastUserId_ = userId;

    if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->IsCacheInitialized(userId)) {
        TAG_LOGI(AAFwkTag::INTENT, "UserSwitched: cache already initialized for userId: %{public}d, skipped",
            userId);
        return;
    }

    auto task = [self = shared_from_this(), userId]() {
        self->LoadInsightIntentInfos(userId);
    };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::HandleUserRemove(const EventFwk::CommonEventData &data)
{
    int32_t userId = data.GetCode();
    if (userId < 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid switched userId: %{public}d", userId);
    }

    std::lock_guard<std::mutex> lock(userIdMutex_);
    if (userId == lastUserId_) {
        TAG_LOGW(AAFwkTag::INTENT, "not allow remove current userId: %{public}d", userId);
        return;
    }

    auto task = [self = shared_from_this(), userId]() { self->DeleteInsightIntentInfoByUserId(userId); };
    ffrt::submit(task);
}

void InsightIntentSysEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    const AAFwk::Want &want = data.GetWant();
    std::string action = want.GetAction();
    TAG_LOGI(AAFwkTag::INTENT, "the action: %{public}s", action.c_str());

    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED) {
        bootReceiveTime_ = std::chrono::steady_clock::now();
        TAG_LOGI(AAFwkTag::INTENT, "[CRRR] BUNDLE_SCAN_FINISHED received, boot timing start");
        HandleBundleScanFinished();
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        HandleUserSwitched(data);
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_REMOVED) {
        HandleUserRemove(data);
    } else {
        TAG_LOGW(AAFwkTag::INTENT, "invalid action");
    }
}
} // namespace AbilityRuntime
} // namespace OHOS