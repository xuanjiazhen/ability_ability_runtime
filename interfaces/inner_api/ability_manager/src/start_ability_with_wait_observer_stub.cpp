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
    // 加锁
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    startAbilityWithWaitDataInfo_.isCompleteFirstFrameFlag = startAbilityWithWaitData.isCompleteFirstFrameFlag;
    startAbilityWithWaitDataInfo_.abilityManagerServiceTime = startAbilityWithWaitData.abilityManagerServiceTime;
    startAbilityWithWaitDataInfo_.completeFirstFrameDrawingTime = startAbilityWithWaitData.completeFirstFrameDrawingTime;
    startAbilityWithWaitDataInfo_.coldStart = startAbilityWithWaitData.coldStart;
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai %{public}lld",
        startAbilityWithWaitDataInfo_.completeFirstFrameDrawingTime);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai %{public}lld",
        startAbilityWithWaitDataInfo_.abilityManagerServiceTime);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai %{public}d",
        startAbilityWithWaitDataInfo_.isCompleteFirstFrameFlag);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::OnStartAbilityWithWai %{public}d",
        startAbilityWithWaitDataInfo_.coldStart);
}

StartAbilityWithWaitData StartAbilityWithWaitObserverStub::GetStartAbilityWithWaitData() // const
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetStartAbilityWithWaitData start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetStartAbilityWithWaitData end");
    return startAbilityWithWaitDataInfo_;

}

bool StartAbilityWithWaitObserverStub::GetCompleteFirstFrameFlag()
{
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetCompleteFirstFrameFlag start");
    std::unique_lock<std::mutex> lock(callbackMutex_);
    bool res = startAbilityWithWaitDataInfo_.isCompleteFirstFrameFlag;
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetCompleteFirstFrameFlag res :: %{public}d", res);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "StartAbilityWithWaitObserverStub::GetCompleteFirstFrameFlag end");
    return res;
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