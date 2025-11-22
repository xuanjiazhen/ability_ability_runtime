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

#include "extension_record_base.h"

#include <singleton.h>

#include "ability_util.h"
#include "connection_state_manager.h"
#include "hitrace_meter.h"
#include "ui_extension_utils.h"

namespace OHOS {
namespace AAFwk {
ExtensionAbilityBase::ExtensionAbilityBase(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode) {}

std::shared_ptr<ExtensionAbilityBase> ExtensionAbilityBase::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<ExtensionAbilityBase>(abilityRequest.want, abilityRequest.abilityInfo,
        abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->Init(abilityRequest);
    return abilityRecord;
}

AbilityRecordType ExtensionAbilityBase::GetAbilityRecordType()
{
    return AbilityRecordType::EXTENSION_ABILITY;
}

void ExtensionAbilityBase::AddConnectRecordToList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    std::lock_guard guard(connRecordListMutex_);
    auto it = std::find(connRecordList_.begin(), connRecordList_.end(), connRecord);
    // found it
    if (it != connRecordList_.end()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Found it in list, so no need to add same connection");
        return;
    }
    // no found then add new connection to list
    TAG_LOGD(AAFwkTag::ABILITYMGR, "No found in list, so add new connection to list");
    connRecordList_.push_back(connRecord);
}

std::list<std::shared_ptr<ConnectionRecord>> ExtensionAbilityBase::GetConnectRecordList() const
{
    std::lock_guard guard(connRecordListMutex_);
    return connRecordList_;
}

void ExtensionAbilityBase::RemoveConnectRecordFromList(const std::shared_ptr<ConnectionRecord> &connRecord)
{
    CHECK_POINTER(connRecord);
    std::lock_guard guard(connRecordListMutex_);
    connRecordList_.remove(connRecord);
    if (connRecordList_.empty()) {
        isConnected = false;
    }
}

std::shared_ptr<ConnectionRecord> ExtensionAbilityBase::GetConnectingRecord() const
{
    std::lock_guard guard(connRecordListMutex_);
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::CONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

std::list<std::shared_ptr<ConnectionRecord>> ExtensionAbilityBase::GetConnectingRecordList()
{
    std::lock_guard guard(connRecordListMutex_);
    std::list<std::shared_ptr<ConnectionRecord>> connectingList;
    for (auto record : connRecordList_) {
        if (record && record->GetConnectState() == ConnectionState::CONNECTING) {
            connectingList.push_back(record);
        }
    }
    return connectingList;
}

std::shared_ptr<ConnectionRecord> ExtensionAbilityBase::GetDisconnectingRecord() const
{
    std::lock_guard guard(connRecordListMutex_);
    auto connect =
        std::find_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
            return record->GetConnectState() == ConnectionState::DISCONNECTING;
        });
    return (connect != connRecordList_.end()) ? *connect : nullptr;
}

size_t ExtensionAbilityBase::GetConnectedListSize()
{
    std::lock_guard guard(connRecordListMutex_);
    return std::count_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
        return record && record->GetConnectState() == ConnectionState::CONNECTED;
    });
}

bool ExtensionAbilityBase::IsConnectListEmpty()
{
    std::lock_guard guard(connRecordListMutex_);
    return connRecordList_.empty();
}

bool ExtensionAbilityBase::NeedConnectAfterCommand()
{
    return !IsConnectListEmpty() && !isConnected;
}

size_t ExtensionAbilityBase::GetConnectingListSize()
{
    std::lock_guard guard(connRecordListMutex_);
    return std::count_if(connRecordList_.begin(), connRecordList_.end(), [](std::shared_ptr<ConnectionRecord> record) {
        return record && record->GetConnectState() == ConnectionState::CONNECTING;
    });
}

uint32_t ExtensionAbilityBase::GetInProgressRecordCount()
{
    std::lock_guard guard(connRecordListMutex_);
    uint32_t count = 0;
    for (auto record : connRecordList_) {
        if (record && (record->GetConnectState() == ConnectionState::CONNECTING ||
            record->GetConnectState() == ConnectionState::CONNECTED)) {
            count++;
        }
    }
    return count;
}

void ExtensionAbilityBase::DisconnectAbility()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "DisconnectAbility, bundle:%{public}s, ability:%{public}s.",
        GetAbilityInfo().applicationInfo.bundleName.c_str(), GetAbilityInfo().name.c_str());
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->DisconnectAbility(GetWant());
    if (GetAbilityInfo().extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        if (GetInProgressRecordCount() == 0) {
            isConnected = false;
        }
    } else {
        isConnected = false;
    }
}

void ExtensionAbilityBase::DisconnectAbilityWithWant(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "DisconnectAbilityWithWant:%{public}s.", GetAbilityInfo().name.c_str());
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->DisconnectAbility(want);
    if (GetInProgressRecordCount() == 0) {
        isConnected = false;
    }
}

void ExtensionAbilityBase::DumpService(std::vector<std::string> &info, bool isClient) const
{
    std::vector<std::string> params;
    DumpService(info, params, isClient);
}

void ExtensionAbilityBase::DumpService(std::vector<std::string> &info, std::vector<std::string> &params, bool isClient) const
{
    info.emplace_back("      AbilityRecord ID #" + std::to_string(GetRecordId()) + "   state #" +
                      AbilityRecord::ConvertAbilityState(GetAbilityState()) + "   start time [" +
                      std::to_string(GetStartTime()) + "]");
    info.emplace_back("      main name [" + GetAbilityInfo().name + "]");
    info.emplace_back("      bundle name [" + GetAbilityInfo().bundleName + "]");
    bool isUIExtension = UIExtensionUtils::IsUIExtension(GetAbilityInfo().extensionAbilityType);
    if (isUIExtension) {
        info.emplace_back("      ability type [UIEXTENSION]");
    } else {
        if (GetAbilityInfo().extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
            info.emplace_back("      ability type [UI_SERVICE]");
            info.emplace_back("      windowConfig windowType [" +
                std::to_string(GetAbilityWindowConfig().windowType) + "]");
            info.emplace_back("      windowConfig windowId [" +
                std::to_string(GetAbilityWindowConfig().windowId) + "]");
        } else {
            info.emplace_back("      ability type [SERVICE]");
        }
    }
    info.emplace_back("      app state #" + AbilityRecord::ConvertAppState(GetAppState()));

    std::string isKeepAlive = GetKeepAlive() ? "true" : "false";
    info.emplace_back("        isKeepAlive: " + isKeepAlive);
    if (IsLauncherRoot()) {
        info.emplace_back("      can restart num #" + std::to_string(GetRestartCount()));
    }
    decltype(connRecordList_) connRecordListCpy;
    {
        std::lock_guard guard(connRecordListMutex_);
        connRecordListCpy = connRecordList_;
    }

    info.emplace_back("      Connections: " + std::to_string(connRecordListCpy.size()));
    for (auto &&conn : connRecordListCpy) {
        if (conn) {
            conn->Dump(info);
        }
    }
    // add dump client info
    DumpClientInfo(info, params, isClient);
    DumpUIExtensionRootHostInfo(info);
    DumpUIExtensionPid(info, isUIExtension);
}

void ExtensionAbilityBase::DumpUIExtensionRootHostInfo(std::vector<std::string> &info) const
{
    if (!UIExtensionUtils::IsUIExtension(GetAbilityInfo().extensionAbilityType)) {
        // Dump host info only for uiextension.
        return;
    }

    sptr<IRemoteObject> token = GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null token");
        return;
    }

    UIExtensionHostInfo hostInfo;
    auto ret = IN_PROCESS_CALL(AAFwk::AbilityManagerClient::GetInstance()->GetUIExtensionRootHostInfo(token, hostInfo));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: %{public}d", ret);
        return;
    }

    std::string dumpInfo = "      root host bundle name [" + hostInfo.elementName_.GetBundleName() + "]";
    info.emplace_back(dumpInfo);
    dumpInfo = "      root host module name [" + hostInfo.elementName_.GetModuleName() + "]";
    info.emplace_back(dumpInfo);
    dumpInfo = "      root host ability name [" + hostInfo.elementName_.GetAbilityName() + "]";
    info.emplace_back(dumpInfo);
}

void ExtensionAbilityBase::DumpUIExtensionPid(std::vector<std::string> &info, bool isUIExtension) const
{
    if (!isUIExtension) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Not ui extension type.");
        return;
    }

    auto appScheduler = DelayedSingleton<AppScheduler>::GetInstance();
    if (appScheduler == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null appScheduler");
        return;
    }
    AppExecFwk::RunningProcessInfo processInfo;
    appScheduler->GetRunningProcessInfoByToken(GetToken(), processInfo);
    info.emplace_back("      pid: " + std::to_string(processInfo.pid_));
}

void ExtensionAbilityBase::ConnectAbilityWithWant(const Want &want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Connect ability.");
    CHECK_POINTER(lifecycleDeal_);
    if (isConnected) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "state err");
    }
    lifecycleDeal_->ConnectAbility(want);
    isConnected = true;
}

void ExtensionAbilityBase::ConnectAbility()
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "%{public}s called.", __func__);
    Want want = GetWant();
    UpdateDmsCallerInfo(want);
    ConnectAbilityWithWant(want);
}

void ExtensionAbilityBase::SetConnRemoteObject(const sptr<IRemoteObject> &remoteObject)
{
    connRemoteObject_ = remoteObject;
}

sptr<IRemoteObject> ExtensionAbilityBase::GetConnRemoteObject() const
{
    return connRemoteObject_;
}

} // namespace AAFwk
} // namespace OHOS