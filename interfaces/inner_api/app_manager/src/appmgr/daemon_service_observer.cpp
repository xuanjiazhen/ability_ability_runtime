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

#include "daemon_service_observer.h"

#include <memory>

#include "errors.h"
#include "ipc_types.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr size_t MAX_DESCRIPTOR_FIELD_LENGTH = 512;

bool IsValidState(int32_t state)
{
    return state >= static_cast<int32_t>(LaunchServiceState::NOT_REGISTERED) &&
        state <= static_cast<int32_t>(LaunchServiceState::DISABLED);
}

bool IsValidProcessState(int32_t state)
{
    return state >= static_cast<int32_t>(LaunchServiceProcessState::CREATED) &&
        state <= static_cast<int32_t>(LaunchServiceProcessState::TERMINATED);
}

bool WriteDescriptor(Parcel &parcel, const LaunchServiceDescriptor &descriptor)
{
    return !descriptor.bundleName.empty() && descriptor.bundleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        !descriptor.moduleName.empty() && descriptor.moduleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        !descriptor.serviceName.empty() && descriptor.serviceName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        descriptor.userId >= -1 && descriptor.appIndex >= 0 &&
        parcel.WriteString(descriptor.bundleName) && parcel.WriteString(descriptor.moduleName) &&
        parcel.WriteString(descriptor.serviceName) && parcel.WriteInt32(descriptor.userId) &&
        parcel.WriteInt32(descriptor.appIndex);
}

bool ReadDescriptor(Parcel &parcel, LaunchServiceDescriptor &descriptor)
{
    if (!parcel.ReadString(descriptor.bundleName) || !parcel.ReadString(descriptor.moduleName) ||
        !parcel.ReadString(descriptor.serviceName) || !parcel.ReadInt32(descriptor.userId) ||
        !parcel.ReadInt32(descriptor.appIndex)) {
        return false;
    }
    return !descriptor.bundleName.empty() && descriptor.bundleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        !descriptor.moduleName.empty() && descriptor.moduleName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        !descriptor.serviceName.empty() && descriptor.serviceName.size() <= MAX_DESCRIPTOR_FIELD_LENGTH &&
        descriptor.userId >= -1 && descriptor.appIndex >= 0;
}

template <typename T>
void SendObserverRequest(const sptr<IRemoteObject> &remote, uint32_t code, const T &value)
{
    if (remote == nullptr) {
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(ILaunchServiceObserver::GetDescriptor()) ||
        !data.WriteParcelable(&value)) {
        return;
    }
    remote->SendRequest(code, data, reply, option);
}
} // namespace

bool LaunchServiceStateData::ReadFromParcel(Parcel &parcel)
{
    int32_t stateValue = 0;
    if (!ReadDescriptor(parcel, descriptor) || !parcel.ReadInt32(stateValue) || !IsValidState(stateValue)) {
        return false;
    }
    state = static_cast<LaunchServiceState>(stateValue);
    return true;
}

bool LaunchServiceStateData::Marshalling(Parcel &parcel) const
{
    return IsValidState(static_cast<int32_t>(state)) && WriteDescriptor(parcel, descriptor) &&
        parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceStateData *LaunchServiceStateData::Unmarshalling(Parcel &parcel)
{
    auto data = std::make_unique<LaunchServiceStateData>();
    return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

bool LaunchServiceProcessStateData::ReadFromParcel(Parcel &parcel)
{
    int32_t stateValue = 0;
    if (!ReadDescriptor(parcel, descriptor) || !parcel.ReadInt32(pid) || pid < 0 ||
        !parcel.ReadInt32(stateValue) || !IsValidProcessState(stateValue)) {
        return false;
    }
    state = static_cast<LaunchServiceProcessState>(stateValue);
    return true;
}

bool LaunchServiceProcessStateData::Marshalling(Parcel &parcel) const
{
    return IsValidProcessState(static_cast<int32_t>(state)) && pid >= 0 &&
        WriteDescriptor(parcel, descriptor) && parcel.WriteInt32(pid) &&
        parcel.WriteInt32(static_cast<int32_t>(state));
}

LaunchServiceProcessStateData *LaunchServiceProcessStateData::Unmarshalling(Parcel &parcel)
{
    auto data = std::make_unique<LaunchServiceProcessStateData>();
    return data->ReadFromParcel(parcel) ? data.release() : nullptr;
}

void LaunchServiceObserverProxy::OnLaunchServiceStatusChanged(const LaunchServiceStateData &stateData)
{
    SendObserverRequest(Remote(), ILaunchServiceObserver::ON_STATUS_CHANGED, stateData);
}

BrokerDelegator<LaunchServiceObserverProxy> LaunchServiceObserverProxy::delegator_;

void LaunchServiceObserverProxy::OnLaunchServiceProcessStateChanged(
    const LaunchServiceProcessStateData &processStateData)
{
    SendObserverRequest(Remote(), ILaunchServiceObserver::ON_PROCESS_STATE_CHANGED, processStateData);
}

int LaunchServiceObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != ILaunchServiceObserver::GetDescriptor()) {
        return ERR_INVALID_STATE;
    }
    if (code == ILaunchServiceObserver::ON_STATUS_CHANGED) {
        std::unique_ptr<LaunchServiceStateData> stateData(data.ReadParcelable<LaunchServiceStateData>());
        if (stateData == nullptr) {
            return ERR_INVALID_VALUE;
        }
        OnLaunchServiceStatusChanged(*stateData);
        return ERR_OK;
    }
    if (code == ILaunchServiceObserver::ON_PROCESS_STATE_CHANGED) {
        std::unique_ptr<LaunchServiceProcessStateData> processStateData(
            data.ReadParcelable<LaunchServiceProcessStateData>());
        if (processStateData == nullptr) {
            return ERR_INVALID_VALUE;
        }
        OnLaunchServiceProcessStateChanged(*processStateData);
        return ERR_OK;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
} // namespace AAFwk
} // namespace OHOS
