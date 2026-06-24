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

#include "extract_insight_intent_profile.h"
#include "common_event_support.h"
#include "function_call_convert.h"
#include "cli_tool_mgr_client.h"
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

void InsightIntentSysEventReceiver::SaveInsightIntentInfos(const std::string &bundleName, const std::string &moduleName,
    uint32_t versionCode, int32_t userId)
{
    TAG_LOGI(AAFwkTag::INTENT, "save insight intent infos, bundle:%{public}s module:%{public}s",
        bundleName.c_str(), moduleName.c_str());
    std::vector<std::string> moduleNameVec;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (const auto &moduleNameLocal : moduleNameVec) {
        std::string profile;
        AbilityRuntime::ExtractInsightIntentProfileInfoVec infos = {};
        std::vector<InsightIntentInfo> configIntentInfos = {};
        if (!PrepareIntentInfosFromProfile(bundleName, moduleNameLocal, userId, profile, infos, configIntentInfos)) {
            if (DelayedSingleton<InsightIntentDbCache>::GetInstance()->HasBundleCache(bundleName)) {
                DeleteInsightIntent(bundleName, moduleNameLocal, userId);
            }
            continue;
        }

        ErrCode ret = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveInsightIntentTotalInfo(
            bundleName, moduleNameLocal, userId, versionCode, infos, configIntentInfos);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "save intent info failed, bundleName: %{public}s, moduleName: %{public}s, "
                "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
            continue;
        }
        CliTool::RegisterInsightIntentFunctions(infos, configIntentInfos, bundleName, versionCode);
        TAG_LOGI(AAFwkTag::INTENT, "save intent info success, bundleName: %{public}s, moduleName: %{public}s, "
            "userId: %{public}d", bundleName.c_str(), moduleNameLocal.c_str(), userId);
    }
}

void InsightIntentSysEventReceiver::RegisterInsightIntentFunctionsOnly(const std::string &bundleName,
    const std::string &moduleName, uint32_t versionCode, int32_t userId)
{
    TAG_LOGI(AAFwkTag::INTENT, "register-only, bundle:%{public}s module:%{public}s versionCode:%{public}u",
        bundleName.c_str(), moduleName.c_str(), versionCode);
    std::vector<std::string> moduleNameVec;
    OHOS::SplitStr(moduleName, ",", moduleNameVec);
    for (const auto &moduleNameLocal : moduleNameVec) {
        std::string profile;
        AbilityRuntime::ExtractInsightIntentProfileInfoVec infos = {};
        std::vector<InsightIntentInfo> configIntentInfos = {};
        // KV rebuild path: Intent RDB already has records, only re-register FunctionCall KV.
        // Do NOT touch Intent RDB on failure.
        if (!PrepareIntentInfosFromProfile(bundleName, moduleNameLocal, userId, profile, infos, configIntentInfos)) {
            TAG_LOGW(AAFwkTag::INTENT, "register-only prepare failed, bundle:%{public}s module:%{public}s",
                bundleName.c_str(), moduleNameLocal.c_str());
            continue;
        }
        CliTool::RegisterInsightIntentFunctions(infos, configIntentInfos, bundleName, versionCode);
        TAG_LOGI(AAFwkTag::INTENT, "register-only success, bundle:%{public}s module:%{public}s",
            bundleName.c_str(), moduleNameLocal.c_str());
    }
}

bool InsightIntentSysEventReceiver::PrepareIntentInfosFromProfile(const std::string &bundleName,
    const std::string &moduleName, int32_t userId, std::string &profile,
    AbilityRuntime::ExtractInsightIntentProfileInfoVec &infos, std::vector<InsightIntentInfo> &configInfos)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return false;
    }
    ErrCode ret = IN_PROCESS_CALL(bundleMgrHelper->GetJsonProfile(AppExecFwk::INTENT_PROFILE, bundleName,
        moduleName, profile, userId));
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetJsonProfile failed, bundle:%{public}s module:%{public}s code:%{public}d",
            bundleName.c_str(), moduleName.c_str(), ret);
        return false;
    }
    bool noExtract = (!AbilityRuntime::ExtractInsightIntentProfile::TransformTo(profile, infos) ||
        infos.insightIntents.size() == 0);
    bool noConfig = (!AbilityRuntime::InsightIntentProfile::TransformTo(profile, configInfos) ||
        configInfos.size() == 0);
    if (noExtract && noConfig) {
        TAG_LOGW(AAFwkTag::INTENT, "transform profile failed, bundle:%{public}s module:%{public}s",
            bundleName.c_str(), moduleName.c_str());
        return false;
    }
    return true;
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

void InsightIntentSysEventReceiver::LoadInsightIntentInfos(int32_t userId)
{
    if (userId == -1) {
        userId = AppExecFwk::OsAccountManagerWrapper::GetCurrentActiveAccountId();
        if (userId == 0) {
            TAG_LOGI(AAFwkTag::INTENT, "use MAIN_USER_ID(%{public}d) instead of current userId: (%{public}d)",
                     MAIN_USER_ID, userId);
            userId = MAIN_USER_ID;
        }
    }

    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->InitInsightIntentCache(userId);

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null bundleMgrHelper");
        return;
    }

    std::vector<AppExecFwk::BundleInfo> bundleInfos {};
    if (IN_PROCESS_CALL(bundleMgrHelper->GetBundleInfosV9(
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE),
        bundleInfos, userId)) != ERR_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get bundle info failed");
        return;
    }

    bool hasNewIntent = false;
    for (auto &bundleInfo : bundleInfos) {
        bool rdbHas = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->
            HasInsightIntentByName(bundleInfo.versionCode, bundleInfo.name, userId);
        if (rdbHas) {
            ReconcileFunctionsForBundle(bundleInfo, userId);
            continue;
        }
        for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
            SaveInsightIntentInfos(bundleInfo.name, hapInfo.moduleName, bundleInfo.versionCode, userId);
            hasNewIntent = true;
        }
    }
    if (hasNewIntent) {
        DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->BackupRdb();
    }
}

void InsightIntentSysEventReceiver::ReconcileFunctionsForBundle(
    const AppExecFwk::BundleInfo &bundleInfo, int32_t userId)
{
    std::vector<CliTool::FunctionInfo> functions;
    auto ret = CliTool::CliToolMGRClient::GetInstance().GetFunctionByNamespace(
        bundleInfo.name, functions);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::INTENT, "GetFunctionByNamespace failed, bundle:%{public}s code:%{public}d",
            bundleInfo.name.c_str(), ret);
        return;
    }
    if (!NeedRegisterFunctions(functions, bundleInfo.versionCode)) {
        return;
    }
    TAG_LOGI(AAFwkTag::INTENT, "reconcile functions, bundle:%{public}s version:%{public}u",
        bundleInfo.name.c_str(), bundleInfo.versionCode);
    for (const auto &hapInfo : bundleInfo.hapModuleInfos) {
        RegisterInsightIntentFunctionsOnly(bundleInfo.name, hapInfo.moduleName,
            bundleInfo.versionCode, userId);
    }
}

bool InsightIntentSysEventReceiver::NeedRegisterFunctions(
    const std::vector<CliTool::FunctionInfo> &functions, uint32_t versionCode)
{
    if (functions.empty()) {
        TAG_LOGI(AAFwkTag::INTENT, "KV empty, need register, version:%{public}u", versionCode);
        return true;
    }
    for (const auto &function : functions) {
        uint32_t kvVersion = 0;
        try {
            kvVersion = static_cast<uint32_t>(std::stoul(function.version));
        } catch (...) {
            TAG_LOGW(AAFwkTag::INTENT, "parse version failed: %{public}s, need register",
                function.version.c_str());
            return true;
        }
        if (kvVersion < versionCode) {
            TAG_LOGI(AAFwkTag::INTENT, "stale KV version %{public}u < %{public}u, need register",
                kvVersion, versionCode);
            return true;
        }
    }
    return false;
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
        self->LoadInsightIntentInfos();
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