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

#ifndef OHOS_START_ABILITY_WITH_WAIT_OBSERVER_STUB_H
#define OHOS_START_ABILITY_WITH_WAIT_OBSERVER_STUB_H

#include <map>
#include <mutex>

#include "start_ability_with_wait_observer_interface.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class StartAbilityWithWaitObserverStub : public IRemoteStub<IStartAbilityWithWaitObserver> {
public:
    StartAbilityWithWaitObserverStub();
    virtual ~StartAbilityWithWaitObserverStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    void NotifyStartMode() override;

    void NotifyAbilityType() override;

    void OnStartAbilityWithWait(const StartAbilityWithWaitData &startAbilityWithWaitData) override;

    StartAbilityWithWaitData GetStartAbilityWithWaitData();

    bool GetStartStatus();

    StartAbilityWithWaitData startAbilityWithWaitDataInfo_;
    bool isImplicitStart_ = false;
    bool abilityTypeWrong_ = false;
private:
    int32_t HandleOnStartAbilityWithWaitChanged(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyStartMode(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyAbilityType(MessageParcel &data, MessageParcel &reply);
    std::mutex callbackMutex_;
    DISALLOW_COPY_AND_MOVE(StartAbilityWithWaitObserverStub);
};

/**
 * @class StartAbilityWithWaitObserverRecipient
 * StartAbilityWithWaitObserverRecipient notices IRemoteBroker died.
 */
class StartAbilityWithWaitObserverRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit StartAbilityWithWaitObserverRecipient(RemoteDiedHandler handler);
    virtual ~StartAbilityWithWaitObserverRecipient() = default;
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    RemoteDiedHandler handler_;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FIRST_FRAME_STATE_TIME_OBSERVER_STUB_H
