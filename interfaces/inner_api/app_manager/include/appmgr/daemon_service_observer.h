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

#ifndef OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_OBSERVER_H

#include <cstdint>

#include "daemon_service_info.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
enum class LaunchServiceState : int32_t {
    NOT_REGISTERED = 0,
    ENABLED = 1,
    DISABLED = 2,
};

enum class LaunchServiceProcessState : int32_t {
    CREATED = 0,
    TERMINATED = 1,
};

struct LaunchServiceStateData : public Parcelable {
    LaunchServiceDescriptor descriptor;
    LaunchServiceState state = LaunchServiceState::NOT_REGISTERED;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static LaunchServiceStateData *Unmarshalling(Parcel &parcel);
};

struct LaunchServiceProcessStateData : public Parcelable {
    LaunchServiceDescriptor descriptor;
    int32_t pid = 0;
    LaunchServiceProcessState state = LaunchServiceProcessState::CREATED;

    bool ReadFromParcel(Parcel &parcel);
    bool Marshalling(Parcel &parcel) const override;
    static LaunchServiceProcessStateData *Unmarshalling(Parcel &parcel);
};

class ILaunchServiceObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.ILaunchServiceObserver");

    enum Message : uint32_t {
        ON_STATUS_CHANGED = 0,
        ON_PROCESS_STATE_CHANGED,
    };

    virtual void OnLaunchServiceStatusChanged(const LaunchServiceStateData &stateData) = 0;
    virtual void OnLaunchServiceProcessStateChanged(const LaunchServiceProcessStateData &processStateData) = 0;
};

class LaunchServiceObserverProxy final : public IRemoteProxy<ILaunchServiceObserver> {
public:
    explicit LaunchServiceObserverProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<ILaunchServiceObserver>(impl) {}

    void OnLaunchServiceStatusChanged(const LaunchServiceStateData &stateData) override;
    void OnLaunchServiceProcessStateChanged(const LaunchServiceProcessStateData &processStateData) override;

private:
    static BrokerDelegator<LaunchServiceObserverProxy> delegator_;
};

class LaunchServiceObserverStub : public IRemoteStub<ILaunchServiceObserver> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_DAEMON_SERVICE_OBSERVER_H
