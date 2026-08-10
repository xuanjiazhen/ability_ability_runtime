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

#include "observerstubfield_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "application_state_observer_stub.h"
#include "data_ability_observer_stub.h"
#include "remote_on_listener_stub.h"
#include "remote_intent_result_callback_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string APPLICATION_STATE_OBSERVER_INTERFACE_TOKEN = u"ohos.appexecfwk.IApplicationStateObserver";
const std::u16string DATA_ABILITY_OBSERVER_INTERFACE_TOKEN = u"ohos.aafwk.DataAbilityObserver";
const std::u16string REMOTE_ON_LISTENER_INTERFACE_TOKEN = u"ohos.aafwk.RemoteOnListener";
const std::u16string REMOTE_INTENT_RESULT_CALLBACK_INTERFACE_TOKEN =
    u"ohos.distributedschedule.IRemoteIntentResultCallback";
}

class ApplicationStateObserverStubFieldFuzz : public ApplicationStateObserverStub {
public:
    ApplicationStateObserverStubFieldFuzz() = default;
    ~ApplicationStateObserverStubFieldFuzz() = default;
    void OnForegroundApplicationChanged(const AppStateData &appStateData) override {}
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData) override {}
    void OnExtensionStateChanged(const AbilityStateData &abilityStateData) override {}
    void OnProcessCreated(const ProcessData &processData) override {}
    void OnProcessStateChanged(const ProcessData &processData) override {}
    void OnWindowShow(const ProcessData &processData) override {}
    void OnWindowHidden(const ProcessData &processData) override {}
    void OnProcessDied(const ProcessData &processData) override {}
    void OnApplicationStateChanged(const AppStateData &appStateData) override {}
    void OnAppStateChanged(const AppStateData &appStateData) override {}
    void OnProcessReused(const ProcessData &processData) override {}
    void OnAppStarted(const AppStateData &appStateData) override {}
    void OnAppStopped(const AppStateData &appStateData) override {}
    void OnPageShow(const PageStateData &pageStateData) override {}
    void OnPageHide(const PageStateData &pageStateData) override {}
    void OnAppCacheStateChanged(const AppStateData &appStateData) override {}
    void OnProcessBindingRelationChanged(const ProcessBindData &processBindData) override {}
    void OnProcessTypeChanged(const ProcessData &processData) override {}
    void OnKeepAliveStateChanged(const ProcessData &processData) override {}
    void OnProcessPreForegroundChanged(const PreloadProcessData &preloadProcessData) override {}
};

class DataAbilityObserverStubFieldFuzz : public DataAbilityObserverStub {
public:
    DataAbilityObserverStubFieldFuzz() = default;
    ~DataAbilityObserverStubFieldFuzz() = default;
    void OnChange() override {}
};

class RemoteOnListenerStubFieldFuzz : public RemoteOnListenerStub {
public:
    RemoteOnListenerStubFieldFuzz() = default;
    ~RemoteOnListenerStubFieldFuzz() = default;
    void OnCallback(const OnCallbackInfo &info) override {}
};

class RemoteIntentResultCallbackStubFieldFuzz : public RemoteIntentResultCallbackStub {
public:
    RemoteIntentResultCallbackStubFieldFuzz() = default;
    ~RemoteIntentResultCallbackStubFieldFuzz() = default;
    void OnIntentResult(uint64_t requestCode, int32_t resultCode, const std::string &resultMsg) override {}
    void OnLinkDisconnected(uint64_t requestCode, int32_t reason) override {}
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = 0;
    uint8_t stubChoice = fdp.ConsumeIntegral<uint8_t>() % 4;

    switch (stubChoice) {
        case 0: {
            parcel.WriteInterfaceToken(APPLICATION_STATE_OBSERVER_INTERFACE_TOKEN);
            switch (code % 4) {
                case 0:
                    actualCode = static_cast<uint32_t>(
                        IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);
                    parcel.WriteBool(true);
                    FuzzUtil::WriteMaliciousAppStateData(parcel, fdp);
                    break;
                case 1:
                    actualCode = static_cast<uint32_t>(
                        IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED);
                    parcel.WriteBool(true);
                    FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
                    break;
                case 2:
                    actualCode = static_cast<uint32_t>(
                        IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED);
                    parcel.WriteBool(true);
                    FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
                    break;
                case 3:
                    actualCode = static_cast<uint32_t>(
                        IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW);
                    parcel.WriteBool(true);
                    FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
                    break;
            }
            parcel.RewindRead(0);
            std::shared_ptr<ApplicationStateObserverStub> appStateStub =
                std::make_shared<ApplicationStateObserverStubFieldFuzz>();
            appStateStub->OnRemoteRequest(actualCode, parcel, reply, option);
            break;
        }
        case 1: {
            parcel.WriteInterfaceToken(DATA_ABILITY_OBSERVER_INTERFACE_TOKEN);
            switch (code % 3) {
                case 0:
                    actualCode = IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE;
                    break;
                case 1:
                    actualCode = IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT;
                    parcel.WriteUint32(static_cast<uint32_t>(fdp.ConsumeIntegral<uint8_t>() % 5));
                    parcel.WriteUint32(0);
                    parcel.WriteUint32(0);
                    parcel.WriteUint32(0);
                    break;
                case 2:
                    actualCode = IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_PREFERENCES;
                    parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
                    break;
            }
            parcel.RewindRead(0);
            std::shared_ptr<DataAbilityObserverStub> dataObsStub =
                std::make_shared<DataAbilityObserverStubFieldFuzz>();
            dataObsStub->OnRemoteRequest(actualCode, parcel, reply, option);
            break;
        }
        case 2: {
            parcel.WriteInterfaceToken(REMOTE_ON_LISTENER_INTERFACE_TOKEN);
            actualCode = IRemoteOnListener::ON_CALLBACK;
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            uint32_t arraySize = fdp.ConsumeIntegral<uint8_t>() % 4;
            parcel.WriteInt32(static_cast<int32_t>(arraySize));
            for (uint32_t i = 0; i < arraySize; i++) {
                parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            }
            parcel.RewindRead(0);
            std::shared_ptr<RemoteOnListenerStub> remoteOnStub =
                std::make_shared<RemoteOnListenerStubFieldFuzz>();
            remoteOnStub->OnRemoteRequest(actualCode, parcel, reply, option);
            break;
        }
        case 3: {
            parcel.WriteInterfaceToken(REMOTE_INTENT_RESULT_CALLBACK_INTERFACE_TOKEN);
            switch (code % 2) {
                case 0:
                    actualCode = IRemoteIntentResultCallback::ON_INTENT_RESULT;
                    parcel.WriteInt64(static_cast<int64_t>(fdp.ConsumeIntegral<uint64_t>()));
                    parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
                    parcel.WriteString(FuzzUtil::BuildOversizedString(fdp));
                    break;
                case 1:
                    actualCode = IRemoteIntentResultCallback::ON_LINK_DISCONNECTED;
                    parcel.WriteInt64(static_cast<int64_t>(fdp.ConsumeIntegral<uint64_t>()));
                    parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
                    break;
            }
            parcel.RewindRead(0);
            std::shared_ptr<RemoteIntentResultCallbackStub> remoteIntentStub =
                std::make_shared<RemoteIntentResultCallbackStubFieldFuzz>();
            remoteIntentStub->OnRemoteRequest(actualCode, parcel, reply, option);
            break;
        }
        default:
            break;
    }
    return true;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
