/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "start_ability_with_wait_observer_stub.h"

#include "hilog_tag_wrapper.h"
#include "appexecfwk_errors.h"
#include "ipc_types.h"
#include "iremote_object.h"

#include "start_ability_with_wait_data.h"

namespace OHOS {
namespace AppExecFwk {
StartAbilityWithWaitObserverStub::StartAbilityWithWaitObserverStub() {}

StartAbilityWithWaitObserverStub::~StartAbilityWithWaitObserverStub() {}

int32_t StartAbilityWithWaitObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "called");
    std::u16string descriptor = StartAbilityWithWaitObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid descriptor");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::ON_START_ABILITY_WITH_WAIT)) {
        return HandleOnStartAbilityWithWaitChanged(data, reply);
    }
    if (code == static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::NOTIFY_WAIT_TYPE)) {
        return HandleNotifyWaitType(data, reply);
    }
    if (code == static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::NOTIFY_ABILITY_MANAGER_SERVICE_TIME)) {
        return HandleNotifyAbilityManagerServiceTime(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t StartAbilityWithWaitObserverStub::HandleOnStartAbilityWithWaitChanged(MessageParcel &data,
    MessageParcel &reply)
{
    std::unique_ptr<StartAbilityWithWaitData> stateData(data.ReadParcelable<StartAbilityWithWaitData>());
    if (stateData == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null stateData");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    OnStartAbilityWithWait(*stateData);
    return NO_ERROR;
}

void StartAbilityWithWaitObserverStub::OnStartAbilityWithWait(const StartAbilityWithWaitData &startAbilityWithWaitData)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    startAbilityWithWaitDataInfo_.endTime = startAbilityWithWaitData.endTime;
    startAbilityWithWaitDataInfo_.coldStart = startAbilityWithWaitData.coldStart;
    startAbilityWithWaitDataInfo_.abilityName = startAbilityWithWaitData.abilityName;
    startAbilityWithWaitDataInfo_.bundleName = startAbilityWithWaitData.bundleName;
    startAbilityWithWaitDataInfo_.startStatus = startAbilityWithWaitData.startStatus;
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait endTime: %{public}lld",
        startAbilityWithWaitDataInfo_.endTime);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait coldStart: %{public}d",
        startAbilityWithWaitDataInfo_.coldStart);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait startStatus: %{public}d",
        startAbilityWithWaitDataInfo_.startStatus);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait %{public}s",
        startAbilityWithWaitDataInfo_.abilityName.c_str());
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWait %{public}s",
        startAbilityWithWaitDataInfo_.bundleName.c_str());
}

StartAbilityWithWaitData StartAbilityWithWaitObserverStub::GetStartAbilityWithWaitData() // const
{
    std::unique_lock<std::mutex> lock(callbackMutex_);
    return startAbilityWithWaitDataInfo_;
}

bool StartAbilityWithWaitObserverStub::GetStartStatus()
{
    std::unique_lock<std::mutex> lock(callbackMutex_);
    return startAbilityWithWaitDataInfo_.startStatus;
}

int32_t StartAbilityWithWaitObserverStub::HandleNotifyWaitType(MessageParcel &data,
    MessageParcel &reply)
{
    uint32_t type = data.ReadUint32();
    NotifyWaitType(type);
    return NO_ERROR;
}

int32_t StartAbilityWithWaitObserverStub::HandleNotifyAbilityManagerServiceTime(MessageParcel &data,
    MessageParcel &reply)
{
    int64_t abilityManagerServiceTime = data.ReadInt64();
    NotifyAbilityManagerServiceTime(abilityManagerServiceTime);
    return NO_ERROR;
}

void StartAbilityWithWaitObserverStub::NotifyWaitType(uint32_t type)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::NotifyWaitType start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    startModetype_ = static_cast<StartModeType>(type);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::NotifyWaitType end");
}

bool StartAbilityWithWaitObserverStub::GetWaitType()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetStartMode start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    if (startModetype_ == StartModeType::ABILITY_TYPE_WRONG || startModetype_ == StartModeType::IS_IMPLICIT_START) {
        return true;
    }
    return false;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetStartMode end");
}

void StartAbilityWithWaitObserverStub::NotifyAbilityManagerServiceTime(int64_t abilityManagerServiceTime)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::NotifyAbilityManagerServiceTime start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    abilityManagerServiceTime_ = abilityManagerServiceTime;
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai %{public}lld",
        abilityManagerServiceTime);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::NotifyAbilityManagerServiceTime end");
}

StartAbilityWithWaitObserverRecipient::StartAbilityWithWaitObserverRecipient(RemoteDiedHandler handler)
    : handler_(handler)
{}

void StartAbilityWithWaitObserverRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote died");
    if (handler_) {
        handler_(remote);
    }
}
} // namespace AppExecFwk
} // namespace OHOS