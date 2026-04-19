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

#include "modular_object_manager.h"

#include "ability_manager_errors.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "app_utils.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "modular_object_rdb_storage_mgr.h"
#include "os_account_manager_wrapper.h"
#include "parameters.h"
#include "running_process_info.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
ModularObjectManager::ModularObjectManager() = default;
ModularObjectManager::~ModularObjectManager() = default;

void ModularObjectManager::SetConnectionHandler(const std::shared_ptr<ModularObjectConnectionHandler> &handler)
{
    connectionHandler_ = handler;
}

int32_t ModularObjectManager::QuerySelfModularObjectExtensionInfos(int32_t userId, const std::string &bundleName,
    int32_t appIndex, std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    std::string key = std::to_string(userId) + "_" + bundleName + "_" + std::to_string(appIndex);
    return DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
}

int32_t ModularObjectManager::ConnectModularObjectExtension(const AAFwk::Want &want,
    sptr<AAFwk::IAbilityConnection> connect, sptr<IRemoteObject> callerToken, int32_t userId)
{
    if (!AppUtils::GetInstance().IsSupportModularObjectExtension()) {
        TAG_LOGE(AAFwkTag::EXT, "device not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::EXT,
        "ConnectModularObjectExtension bundlename: %{public}s, ability: %{public}s, userId: %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), userId);
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connect->AsObject(), ERR_INVALID_VALUE);

    auto element = want.GetElement();
    std::string bundleName = element.GetBundleName();
    std::string abilityName = element.GetAbilityName();
    int32_t appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    std::string key = std::to_string(userId) + "_" + bundleName + "_" + std::to_string(appIndex);
    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    auto ret = DelayedSingleton<ModularObjectExtensionRdbStorageMgr>::GetInstance()->QueryData(key, infos);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "QueryData failed, ret: %{public}d", ret);
        return ret;
    }
    ModularObjectExtensionInfo targetExtensionInfo;
    bool found = false;
    for (const auto &info : infos) {
        if (info.bundleName == bundleName && info.abilityName == abilityName) {
            targetExtensionInfo = info;
            found = true;
            break;
        }
    }
    if (!found) {
        TAG_LOGE(AAFwkTag::EXT, "Extension not found: %{public}s/%{public}s",
            bundleName.c_str(), abilityName.c_str());
        return INNER_ERR;
    }
    ret = ValidateConnectRequest(bundleName, appIndex, targetExtensionInfo, userId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "ValidateConnectRequest failed, ret: %{public}d", ret);
        return ret;
    }
    CHECK_POINTER_AND_RETURN(connectionHandler_, ERR_INVALID_VALUE);
    return connectionHandler_->ConnectAbilityCommon(want, connect, callerToken,
        AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT, userId);
}

int32_t ModularObjectManager::ValidateConnectRequest(const std::string &targetBundleName, int32_t targetAppIndex,
    const ModularObjectExtensionInfo &extensionInfo, int32_t userId)
{
    auto ret = CheckExtensionEnabled(extensionInfo);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = CheckCallerForeground();
    if (ret != ERR_OK) {
        return ret;
    }

    // Get caller bundleName from uid
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, INNER_ERR);
    std::string callerBundleName;
    int32_t callerAppIndex = 0;
    auto bmsRet = IN_PROCESS_CALL(bundleMgrHelper->GetNameAndIndexForUid(callingUid, callerBundleName, callerAppIndex));
    if (bmsRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "Get caller info failed, callingUid: %{public}d, ret: %{public}d",
            callingUid, bmsRet);
        return INNER_ERR;
    }

    // Get caller and target ApplicationInfo
    int32_t callerUserId = -1;
    auto osAccountRet = DelayedSingleton<AppExecFwk::OsAccountManagerWrapper>::GetInstance()
        ->GetOsAccountLocalIdFromUid(callingUid, callerUserId);
    if (osAccountRet != 0) {
        TAG_LOGE(AAFwkTag::EXT, "Get caller userId failed, callingUid: %{public}d", callingUid);
        return INNER_ERR;
    }

    AppExecFwk::ApplicationInfo callerAppInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfoWithAppIndex(
        callerBundleName, callerAppIndex, callerUserId, callerAppInfo))) {
        TAG_LOGE(AAFwkTag::EXT, "Get caller appInfo failed, bundle: %{public}s",
            callerBundleName.c_str());
        return INNER_ERR;
    }

    AppExecFwk::ApplicationInfo targetAppInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfoWithAppIndex(
        targetBundleName, targetAppIndex, userId, targetAppInfo))) {
        TAG_LOGE(AAFwkTag::EXT, "Get target appInfo failed, bundle: %{public}s",
            targetBundleName.c_str());
        return INNER_ERR;
    }

    // 3. Check caller and target app distribution type
    ret = CheckAppDistributionType(callerAppInfo.appDistributionType, targetAppInfo.appDistributionType);
    if (ret != ERR_OK) {
        return ret;
    }

    // 4. Check target has running UIAbility or UIExtension
    ret = CheckTargetHasRunningAbility(targetAppInfo.uid, userId, targetBundleName);
    if (ret != ERR_OK) {
        return ret;
    }

    return ERR_OK;
}

int32_t ModularObjectManager::CheckExtensionEnabled(const ModularObjectExtensionInfo &info)
{
    if (info.isDisabled) {
        TAG_LOGE(AAFwkTag::EXT, "Extension is disabled: %{public}s/%{public}s",
            info.bundleName.c_str(), info.abilityName.c_str());
        return ERR_MODULAR_OBJECT_DISABLED;
    }
    return ERR_OK;
}

int32_t ModularObjectManager::CheckCallerForeground()
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    AppExecFwk::RunningProcessInfo processInfo;
    auto ret = IN_PROCESS_CALL(DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance()->GetRunningProcessInfoByPid(
        callingPid, processInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::EXT, "GetRunningProcessInfoByPid fail:%{public}d", ret);
        return ret;
    }
    if (processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
        TAG_LOGE(AAFwkTag::EXT, "Caller not foreground, callingPid: %{public}d, state: %{public}d",
            callingPid, static_cast<int32_t>(processInfo.state_));
        return NOT_TOP_ABILITY;
    }
    return ERR_OK;
}

int32_t ModularObjectManager::CheckAppDistributionType(
    const std::string &callerAppDistributionType, const std::string &targetAppDistributionType)
{
    bool isDeveloperMode = system::GetBoolParameter("const.security.developermode.state", false);
    if (isDeveloperMode) {
        TAG_LOGD(AAFwkTag::EXT, "Developer mode, allow");
        return ERR_OK;
    }

    if (callerAppDistributionType == "none") {
        TAG_LOGE(AAFwkTag::EXT, "Caller appDistributionType is none, not allowed");
        return ERR_PERMISSION_DENIED;
    }
    if (targetAppDistributionType == "none") {
        TAG_LOGE(AAFwkTag::EXT, "Target appDistributionType is none, not allowed");
        return ERR_PERMISSION_DENIED; // todo新增错误码
    }
    return ERR_OK;
}

int32_t ModularObjectManager::CheckTargetHasRunningAbility(int32_t targetUid, int32_t userId,
    const std::string &targetBundleName)
{
    CHECK_POINTER_AND_RETURN(connectionHandler_, INNER_ERR);
    if (!connectionHandler_->HasRunningUIAbilityOrExtension(targetUid, userId)) {
        TAG_LOGE(AAFwkTag::EXT,
            "Target has no running UIAbility or UIExtension, uid: %{public}d, bundle: %{public}s",
            targetUid, targetBundleName.c_str());
        return ERR_NO_RUNNING_ABILITIES_WITH_UI;
    }
    return ERR_OK;
}

int32_t ModularObjectManager::DisconnectModularObjectExtension(sptr<AAFwk::IAbilityConnection> connect)
{
    if (!AppUtils::GetInstance().IsSupportModularObjectExtension()) {
        TAG_LOGE(AAFwkTag::EXT, "device not supported");
        return ERR_CAPABILITY_NOT_SUPPORT;
    }
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::EXT, "DisconnectModularObjectExtension");
    CHECK_POINTER_AND_RETURN(connect, ERR_INVALID_VALUE);
    CHECK_POINTER_AND_RETURN(connectionHandler_, ERR_INVALID_VALUE);
    return connectionHandler_->DisconnectAbility(connect);
}
}
}
