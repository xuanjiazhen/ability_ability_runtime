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

#include "applicationstateobserverstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>

#include "application_state_observer_stub.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string APPLICATIONSTATEOBSERVER_INTERFACE_TOKEN = u"ohos.appexecfwk.IApplicationStateObserver";

class ApplicationStateObserverStubFuzz : public ApplicationStateObserverStub {
public:
    ApplicationStateObserverStubFuzz() = default;
    virtual ~ApplicationStateObserverStubFuzz()
    {}
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
}  // namespace

bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    uint32_t code = OHOS::FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(
        IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);

    parcel.WriteInterfaceToken(APPLICATIONSTATEOBSERVER_INTERFACE_TOKEN);

    switch (code % 5) {
        case 0:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_FOREGROUND_APPLICATION_CHANGED);
            FuzzUtil::WriteMaliciousAppStateDataPhased(parcel, fdp,
                fdp.ConsumeBool() ? OHOS::FuzzUtil::PHASE_BEFORE_SERIALIZE
                                   : OHOS::FuzzUtil::PHASE_AFTER_DESERIALIZE);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_ABILITY_STATE_CHANGED);
            FuzzUtil::WriteMaliciousAbilityStateData(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_CREATED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 3:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PROCESS_DIED);
            FuzzUtil::WriteMaliciousProcessData(parcel, fdp);
            break;
        case 4:
            actualCode = static_cast<uint32_t>(
                IApplicationStateObserver::Message::TRANSACT_ON_PAGE_SHOW);
            FuzzUtil::WriteMaliciousPageStateData(parcel, fdp);
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<ApplicationStateObserverStub> stub = std::make_shared<ApplicationStateObserverStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char* ch = static_cast<char*>(malloc(size + 1));
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
