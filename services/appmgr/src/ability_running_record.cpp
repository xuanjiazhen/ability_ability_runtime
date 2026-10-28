/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ability_running_record.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* IS_HOOK = "ohos.ability_runtime.is_hook";
static const std::string EMPTY_NAME;
}
AbilityRunningRecord::AbilityRunningRecord(std::shared_ptr<AbilityInfo> info,
    sptr<IRemoteObject> token, int32_t abilityRecordId)
    : abilityRecordId_(abilityRecordId), info_(info), token_(token)
{}

AbilityRunningRecord::~AbilityRunningRecord()
{}

void AbilityRunningRecord::ClearPayloadAfterLoad()
{
    if (payloadCleared_ || !info_) {
        return;
    }
    payloadCleared_ = true;
    std::vector<AppExecFwk::Metadata>().swap(info_->metadata);
    std::vector<std::string>().swap(info_->deviceTypes);
    std::vector<std::string>().swap(info_->deviceCapabilities);
    std::vector<std::string>().swap(info_->permissions);
    std::vector<std::string>().swap(info_->configChanges);
    std::vector<std::string>().swap(info_->supportExtNames);
    std::vector<std::string>().swap(info_->supportMimeTypes);
    std::vector<std::string>().swap(info_->continueType);
    std::unordered_set<std::string>().swap(info_->continueBundleNames);
    for (auto &s : {&info_->uri, &info_->readPermission, &info_->writePermission,
        &info_->label, &info_->description, &info_->iconPath, &info_->targetAbility,
        &info_->srcPath, &info_->srcEntrance, &info_->package, &info_->codePath,
        &info_->className, &info_->originalClassName, &info_->versionName,
        &info_->privacyUrl, &info_->privacyName, &info_->downloadUrl,
        &info_->uriPermissionMode, &info_->uriPermissionPath, &info_->libPath,
        &info_->theme, &info_->kind, &info_->originalBundleName, &info_->appName,
        &info_->startWindow, &info_->startWindowIcon, &info_->startWindowBackground,
        &info_->preferMultiWindowOrientation, &info_->srcLanguage, &info_->arkTSMode,
        &info_->resourcePath, &info_->hapPath}) {
        s->clear();
        s->shrink_to_fit();
    }
    info_->startWindowResource = AppExecFwk::StartWindowResource();
    std::vector<AppExecFwk::ModuleInfo>().swap(info_->applicationInfo.moduleInfos);
    std::vector<std::string>().swap(info_->applicationInfo.moduleSourceDirs);
    std::vector<std::string>().swap(info_->applicationInfo.permissions);
    std::vector<std::string>().swap(info_->applicationInfo.targetBundleList);
    std::vector<AppExecFwk::ApplicationEnvironment>().swap(info_->applicationInfo.appEnvironments);
    std::vector<AppExecFwk::AlternateIcon>().swap(info_->applicationInfo.alternateIcons);
    std::vector<std::string>().swap(info_->applicationInfo.assetAccessGroups);
    std::vector<std::string>().swap(info_->applicationInfo.allowCommonEvent);
    std::vector<std::string>().swap(info_->applicationInfo.allowListenBundleChangedEvent);
    std::vector<int32_t>().swap(info_->applicationInfo.resourcesApply);
    info_->applicationInfo.metaData.clear();
    info_->applicationInfo.metadata.clear();
    info_->applicationInfo.hnpPackages.clear();
}

const std::string &AbilityRunningRecord::GetName() const
{
    if (info_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetName info_ nullptr");
        return EMPTY_NAME;
    }
    return info_->name;
}

const std::string &AbilityRunningRecord::GetBundleName() const
{
    if (info_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetBundleName info_ nullptr");
        return EMPTY_NAME;
    }
    return info_->bundleName;
}

const std::string &AbilityRunningRecord::GetModuleName() const
{
    if (info_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetModuleName info_ nullptr");
        return EMPTY_NAME;
    }
    return info_->moduleName;
}

const std::shared_ptr<AbilityInfo> &AbilityRunningRecord::GetAbilityInfo() const
{
    return info_;
}

const std::shared_ptr<AAFwk::Want> &AbilityRunningRecord::GetWant() const
{
    return want_;
}

void AbilityRunningRecord::SetWant(const std::shared_ptr<AAFwk::Want> &want)
{
    want_ = want;
}

const sptr<IRemoteObject> &AbilityRunningRecord::GetToken() const
{
    return token_;
}

void AbilityRunningRecord::SetState(const AbilityState state)
{
    state_ = state;
}

AbilityState AbilityRunningRecord::GetState() const
{
    return state_;
}

void AbilityRunningRecord::SetEventId(const int64_t eventId)
{
    eventId_ = eventId;
}

int64_t AbilityRunningRecord::GetEventId() const
{
    return eventId_;
}

void AbilityRunningRecord::SetTerminating()
{
    isTerminating_ = true;
}

bool AbilityRunningRecord::IsTerminating() const
{
    return isTerminating_;
}

void AbilityRunningRecord::SetOwnerUserId(int32_t ownerUserId)
{
    ownerUserId_ = ownerUserId;
}

int32_t AbilityRunningRecord::GetOwnerUserId() const
{
    return ownerUserId_;
}

void AbilityRunningRecord::SetIsSingleUser(bool flag)
{
    isSingleUser_ = flag;
}

bool AbilityRunningRecord::IsSingleUser() const
{
    return isSingleUser_;
}

void AbilityRunningRecord::UpdateFocusState(bool isFocus)
{
    isFocused_ = isFocus;
}

bool AbilityRunningRecord::GetFocusFlag() const
{
    return isFocused_;
}

void AbilityRunningRecord::SetUIExtensionAbilityId(const int32_t uiExtensionAbilityId)
{
    uiExtensionAbilityId_ = uiExtensionAbilityId;
}

int32_t AbilityRunningRecord::GetUIExtensionAbilityId() const
{
    return uiExtensionAbilityId_;
}

void AbilityRunningRecord::SetUserRequestCleaningStatus()
{
    isUserRequestCleaning_ = true;
}

bool AbilityRunningRecord::IsUserRequestCleaning() const
{
    return isUserRequestCleaning_;
}

bool AbilityRunningRecord::IsSceneBoard() const
{
    if (info_ == nullptr) {
        return false;
    }
    return info_->name == SCENEBOARD_ABILITY_NAME && info_->bundleName == BUNDLE_NAME_SCENEBOARD;
}

bool AbilityRunningRecord::IsHook() const
{
    return want_ && want_->GetBoolParam(IS_HOOK, false);
}

void AbilityRunningRecord::SetUIExtensionBindAbilityId(const int32_t uiExtensionBindAbilityId)
{
    uiExtensionBindAbilityId_ = uiExtensionBindAbilityId;
}

int32_t AbilityRunningRecord::GetUIExtensionBindAbilityId() const
{
    return uiExtensionBindAbilityId_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
