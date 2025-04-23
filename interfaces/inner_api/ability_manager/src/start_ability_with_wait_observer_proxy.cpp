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

#include "start_ability_with_wait_observer_proxy.h"

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"

namespace OHOS {
namespace AppExecFwk {
StartAbilityWithWaitObserverProxy::StartAbilityWithWaitObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IStartAbilityWithWaitObserver>(impl)
{}

bool StartAbilityWithWaitObserverProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(StartAbilityWithWaitObserverProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write token failed");
        return false;
    }
    return true;
}

void StartAbilityWithWaitObserverProxy::OnStartAbilityWithWait(
    const StartAbilityWithWaitData &startAbilityWithWaitData)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write Token failed");
        return;
    }
    if (!data.WriteParcelable(&startAbilityWithWaitData)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write abilityFirstFrameStateData failed");
        return;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::ON_START_ABILITY_WITH_WAIT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", ret);
    }
}

void StartAbilityWithWaitObserverProxy::NotifyStartMode()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write Token failed");
        return ;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::NOTIFY_START_MODE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", ret);
    }
}

void StartAbilityWithWaitObserverProxy::NotifyAbilityType()
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write Token failed");
        return ;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::NOTIFY_ABILITY_TYPE),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", ret);
    }
}

void StartAbilityWithWaitObserverProxy::NotifyAbilityManagerServiceTime(int64_t abilityManagerServiceTime)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write Token failed");
        return ;
    }
    if (!data.WriteInt64(abilityManagerServiceTime)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "type write fail");
        return ;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(IStartAbilityWithWaitObserver::Message::NOTIFY_ABILITY_MANAGER_SERVICE_TIME),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest error: %{public}d", ret);
    }
}
} // namespace AppExecFwk
} // namespace OHOS