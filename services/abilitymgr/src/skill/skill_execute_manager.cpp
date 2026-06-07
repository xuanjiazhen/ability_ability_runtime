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

#include "skill_execute_manager.h"

#include <cinttypes>

#include "ability_event_handler.h"
#include "ability_manager_errors.h"
#include "ability_manager_service.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_ability_definition.h"
#ifdef SUPPORT_UPMS
#include "uri_permission_manager_client.h"
#endif

namespace OHOS {
namespace AAFwk {

SkillExecuteManager::SkillExecuteManager() {}

SkillExecuteManager::~SkillExecuteManager() {}

sptr<AppExecFwk::IBundleSkillManager> SkillExecuteManager::GetSkillManagerProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SystemAbilityManager");
        return nullptr;
    }
    auto bmsObj = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BMS from samgr");
        return nullptr;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bmsObj);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to cast IBundleMgr");
        return nullptr;
    }
    return bundleMgr->GetSkillManagerProxy();
}

int32_t SkillExecuteManager::QuerySkillInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, int32_t userId, AppExecFwk::SkillInfo &skillInfo)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "query skill info, bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    auto skillMgr = GetSkillManagerProxy();
    if (skillMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SkillManager proxy");
        return ERR_INVALID_VALUE;
    }

    uint32_t flags = static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_SRC_ENTRIES) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_PERMISSIONS) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_REQUEST_PERMISSIONS);

    auto ret = skillMgr->GetSkillInfo(bundleName, moduleName, skillName, flags, userId, skillInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSkillInfo failed, ret:%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo,
    uint32_t callerTokenId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "check skill permission for skill:%{public}s",
        skillInfo.skillName.c_str());

    auto permVerif = PermissionVerification::GetInstance();
    if (!permVerif->IsSACall()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller is not SA");
        return ERR_NOT_SYSTEM_APP;
    }

    if (skillInfo.permissions.empty() ||
        Security::AccessToken::AccessTokenKit::VerifyAccessToken(
            callerTokenId, PermissionConstants::PERMISSION_START_INVISIBLE_ABILITY, false) == ERR_OK) {
        return ERR_OK;
    }

    for (const auto &permission : skillInfo.permissions) {
        if (permission.empty()) {
            continue;
        }
        auto ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
            callerTokenId, permission, false);
        if (ret != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "caller lacks permission:%{public}s for skill:%{public}s",
                permission.c_str(), skillInfo.skillName.c_str());
            return ERR_PERMISSION_DENIED;
        }
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
    const std::string &requestCode,
    const std::string &scriptPath, const std::string &functionName,
    const std::shared_ptr<AAFwk::WantParams> &skillArgs)
{
    const std::string &abilityName = skillInfo.abilityName;
    want.SetElementName("", skillInfo.bundleName, abilityName, skillInfo.moduleName);
    AppExecFwk::SkillExecuteParam::WriteToWant(want, skillInfo.bundleName, skillInfo.moduleName,
        skillInfo.skillName, scriptPath, functionName, skillArgs, skillInfo.srcEntries, requestCode,
        skillInfo.hapPath);
    return ERR_OK;
}

std::string SkillExecuteManager::CreateExecuteRecord(const sptr<IRemoteObject> &callerToken,
    const std::string &targetBundleName, const std::string &callerBundleName,
    uint32_t callerTokenId,
    const sptr<ISkillExecuteCallback> &callback,
    const std::string &externalRequestCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    uint64_t currentSeq = ++requestCodeSeq_;
    std::string requestCode;
    if (!externalRequestCode.empty()) {
        requestCode = externalRequestCode;
    } else {
        requestCode = std::to_string(currentSeq);
    }
    auto record = std::make_shared<SkillExecuteRecord>();
    record->requestCode = requestCode;
    record->callerToken = callerToken;
    record->targetBundleName = targetBundleName;
    record->callerBundleName = callerBundleName;
    record->callerTokenId = callerTokenId;
    record->requestCodeSeq = currentSeq;
    record->state = SkillExecuteState::EXECUTING;
    record->callback = callback;

    if (callerToken != nullptr) {
        auto deathRecipient = sptr<CallerDeathRecipient>::MakeSptr(
            [this](const std::string &reqCode) { OnCallerDied(reqCode); }, requestCode);
        callerToken->AddDeathRecipient(deathRecipient);
        record->deathRecipient = deathRecipient;
    }

    if (callerToken == nullptr && callback != nullptr && callback->AsObject() != nullptr) {
        auto cbDeathRecipient = sptr<CallerDeathRecipient>::MakeSptr(
            [this](const std::string &reqCode) { OnCallbackDied(reqCode); }, requestCode);
        callback->AsObject()->AddDeathRecipient(cbDeathRecipient);
        record->callbackDeathRecipient = cbDeathRecipient;
    }

    records_[requestCode] = record;
    PostSkillExecuteTimeout(requestCode, currentSeq);
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "create execute record, requestCode:%{public}s", requestCode.c_str());
    return requestCode;
}

int32_t SkillExecuteManager::ExecuteSkillDone(const std::string &requestCode, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result, const std::string &callerBundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "execute skill done, requestCode:%{public}s code:%{public}d",
        requestCode.c_str(), resultCode);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "record not found, requestCode:%{public}s", requestCode.c_str());
        return ERR_CODE_INVALID_ID;
    }

    auto record = it->second;
    if (record->targetBundleName != callerBundleName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "bundleName %{public}s and %{public}s mismatch",
            callerBundleName.c_str(), record->targetBundleName.c_str());
        return ERR_INVALID_VALUE;
    }
    if (record->state != SkillExecuteState::EXECUTING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid state:%{public}d", static_cast<int>(record->state));
        return ERR_INVALID_VALUE;
    }

#ifdef SUPPORT_UPMS
    if (!result.uris.empty() && !record->callerBundleName.empty()) {
        std::vector<Uri> uriList;
        for (const auto &uriStr : result.uris) {
            uriList.emplace_back(uriStr);
        }
        auto &uriPermClient = UriPermissionManagerClient::GetInstance();
        auto ret = uriPermClient.GrantUriPermission(
            uriList, result.flags, record->callerBundleName, 0, record->callerTokenId);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "GrantUriPermission failed, ret:%{public}d", ret);
        }
    }
#endif

    RemoveSkillExecuteTimeoutLocked(record->requestCodeSeq);
    record->state = SkillExecuteState::EXECUTE_DONE;
    if (record->callback != nullptr) {
        record->callback->OnExecuteDone(requestCode, resultCode, result);
    }
    RemoveRecord(requestCode);
    return ERR_OK;
}

void SkillExecuteManager::RemoveRecord(const std::string &requestCode)
{
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        return;
    }
    auto record = it->second;
    if (record->callerToken != nullptr && record->deathRecipient != nullptr) {
        record->callerToken->RemoveDeathRecipient(record->deathRecipient);
    }
    if (record->callback != nullptr && record->callbackDeathRecipient != nullptr &&
        record->callback->AsObject() != nullptr) {
        record->callback->AsObject()->RemoveDeathRecipient(record->callbackDeathRecipient);
    }
    records_.erase(it);
}

void SkillExecuteManager::OnCallerDied(const std::string &requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller died, requestCode:%{public}s", requestCode.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it != records_.end() && it->second->state == SkillExecuteState::EXECUTING) {
        RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
        it->second->state = SkillExecuteState::REMOTE_DIED;
        RemoveRecord(requestCode);
    }
}

void SkillExecuteManager::OnCallbackDied(const std::string &requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "callback died, requestCode:%{public}s", requestCode.c_str());
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it != records_.end() && it->second->state == SkillExecuteState::EXECUTING) {
        RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
        it->second->state = SkillExecuteState::REMOTE_DIED;
        it->second->callbackDeathRecipient = nullptr;
        RemoveRecord(requestCode);
    }
}

int32_t SkillExecuteManager::ResolveTargetType(const std::string &bundleName,
    const std::string &moduleName, const std::string &abilityName,
    int32_t userId, AppExecFwk::ExtensionAbilityType &targetType)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BundleMgrHelper");
        return ERR_INVALID_VALUE;
    }

    Want queryWant;
    queryWant.SetElementName("", bundleName, abilityName, moduleName);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    auto extFlags = static_cast<int32_t>(
        AppExecFwk::GetExtensionAbilityInfoFlag::GET_EXTENSION_ABILITY_INFO_DEFAULT);
    bool extRet = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(
        queryWant, extFlags, userId, extensionInfos));
    if (extRet && !extensionInfos.empty()) {
        if (extensionInfos[0].type == AppExecFwk::ExtensionAbilityType::SERVICE) {
            targetType = AppExecFwk::ExtensionAbilityType::SERVICE;
            return ERR_OK;
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "unsupported extension type:%{public}d for ability:%{public}s",
            static_cast<int>(extensionInfos[0].type), abilityName.c_str());
        return ERR_INVALID_VALUE;
    }

    AppExecFwk::AbilityInfo abilityInfo;
    auto abilityFlags = static_cast<int32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_DEFAULT);
    bool abRet = IN_PROCESS_CALL(bundleMgrHelper->QueryAbilityInfo(
        queryWant, abilityFlags, userId, abilityInfo));
    if (abRet && abilityInfo.type == AppExecFwk::AbilityType::PAGE) {
        targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
        return ERR_OK;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR,
        "ability:%{public}s is neither UIAbility nor ServiceExtension", abilityName.c_str());
    return ERR_INVALID_VALUE;
}

void SkillExecuteManager::PostSkillExecuteTimeout(
    const std::string &requestCode, uint64_t requestCodeSeq)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null event handler");
        return;
    }
    uint32_t timeout = static_cast<uint32_t>(
        AmsConfigurationParameter::GetInstance().GetAppStartTimeoutTime()) *
        static_cast<uint32_t>(GlobalConstant::SKILL_EXECUTE_TIMEOUT_MULTIPLE);
    seqToRequestCodeMap_[requestCodeSeq] = requestCode;
    auto event = EventWrap(AbilityManagerService::SKILL_EXECUTE_TIMEOUT_MSG,
        static_cast<int64_t>(requestCodeSeq));
    event.SetTimeout(timeout);
    handler->SendEvent(event, timeout, false);
}

void SkillExecuteManager::RemoveSkillExecuteTimeoutLocked(uint64_t requestCodeSeq)
{
    auto handler = DelayedSingleton<AbilityManagerService>::GetInstance()->GetEventHandler();
    if (handler != nullptr) {
        handler->RemoveEvent(AbilityManagerService::SKILL_EXECUTE_TIMEOUT_MSG,
            static_cast<int64_t>(requestCodeSeq));
    }
    seqToRequestCodeMap_.erase(requestCodeSeq);
}

void SkillExecuteManager::OnTimeout(int64_t requestCodeSeq)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "called, seq:%{public}" PRId64, requestCodeSeq);
    std::string requestCode;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto seqIt = seqToRequestCodeMap_.find(requestCodeSeq);
        if (seqIt == seqToRequestCodeMap_.end()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "seq not found");
            return;
        }
        requestCode = seqIt->second;
        seqToRequestCodeMap_.erase(seqIt);
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        return;
    }
    auto record = it->second;
    if (record->state != SkillExecuteState::EXECUTING) {
        return;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "skill execute timed out, req:%{public}s", requestCode.c_str());
    record->state = SkillExecuteState::TIMED_OUT;
    if (record->callback != nullptr) {
        AppExecFwk::SkillExecuteResult emptyResult;
        record->callback->OnExecuteDone(requestCode, ERR_TIMED_OUT, emptyResult);
    }
    RemoveRecord(requestCode);
}

void SkillExecuteManager::CancelExecuteRecord(const std::string &requestCode)
{
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto it = records_.find(requestCode);
    if (it == records_.end()) {
        return;
    }
    RemoveSkillExecuteTimeoutLocked(it->second->requestCodeSeq);
    RemoveRecord(requestCode);
}

} // namespace AAFwk
} // namespace OHOS
