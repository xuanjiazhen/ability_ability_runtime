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

#include "baseextensionrecord_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "base_extension_record.h"
#include "connection_record.h"
#undef protected
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
class MyAbilityConnection : public IAbilityConnection {
public:
    MyAbilityConnection() = default;
    virtual ~MyAbilityConnection() = default;
    void OnAbilityConnectDone(const AppExecFwk::ElementName& element,
        const sptr<IRemoteObject>& remoteObject, int resultCode) override
    {}
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override
    {}
    sptr<IRemoteObject> AsObject() override
    {
        return {};
    }
};
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    std::string bundleName = fdp.ConsumeRandomLengthString(64);
    std::string abilityName = fdp.ConsumeRandomLengthString(64);
    std::string requestId = fdp.ConsumeRandomLengthString(64);
    std::string taskId = fdp.ConsumeRandomLengthString(64);
    std::string agentId = fdp.ConsumeRandomLengthString(64);
    int32_t recordId = fdp.ConsumeIntegral<int32_t>();
    int64_t attributionId = fdp.ConsumeIntegral<int64_t>();
    int32_t clientPid = fdp.ConsumeIntegral<int32_t>();
    bool isInProcess = fdp.ConsumeBool();
    bool isClient = fdp.ConsumeBool();
    uint32_t messageId = fdp.ConsumeIntegral<uint32_t>();

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = abilityName;
    abilityRequest.abilityInfo.type = AbilityType::EXTENSION;
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    if (!record) {
        return false;
    }

    // static helpers + record type
    (void)BaseExtensionRecord::TransferToExtensionRecordBase(record);
    (void)BaseExtensionRecord::TransferToExtensionRecordBase(nullptr);
    (void)record->GetAbilityRecordType();

    // connection record list
    sptr<IRemoteObject> token = record->GetToken();
    sptr<IAbilityConnection> connect = new MyAbilityConnection();
    auto connRecord = std::make_shared<ConnectionRecord>(token, record, connect, nullptr);
    record->AddConnectRecordToList(connRecord);
    record->AddConnectRecordToList(nullptr);
    (void)record->GetConnectRecordList();
    (void)record->GetConnectingRecordList();
    (void)record->GetConnectingRecord();
    (void)record->GetDisconnectingRecord();
    (void)record->GetConnectedListSize();
    (void)record->GetConnectingListSize();
    (void)record->GetInProgressRecordCount();
    (void)record->IsConnectListEmpty();
    (void)record->NeedConnectAfterCommand();
    record->RemoveConnectRecordFromList(connRecord);
    record->RemoveConnectRecordFromList(nullptr);

    // remote object / pid / process / request id
    record->SetConnRemoteObject(nullptr);
    (void)record->GetConnRemoteObject();
    record->SetClientPid(clientPid);
    (void)record->GetClientPid();
    record->SetIsInProcess(isInProcess);
    (void)record->IsInProcess();
    record->SetRequestId(requestId);
    (void)record->GetRequestId();

    // task attribution
    AgentTaskAttribution attribution;
    attribution.taskId = taskId;
    attribution.attributionId = attributionId;
    attribution.agentId = agentId;
    record->AddTaskAttribution(recordId, attribution);
    (void)record->GetTaskAttributions(recordId);
    record->RemoveTaskAttribution(recordId, taskId, attributionId);
    (void)record->CleanTaskAttributionsByTaskId(taskId);

    // dump
    std::vector<std::string> info;
    std::vector<std::string> params;
    record->DumpService(info, isClient);
    record->DumpService(info, params, isClient);
    record->DumpUIExtensionRootHostInfo(info);
    record->DumpUIExtensionPid(info, isClient);

    // connect / disconnect
    record->PostUIExtensionAbilityTimeoutTask(messageId);
    record->ConnectAbility();
    record->ConnectAbilityWithWant(abilityRequest.want);
    record->DisconnectAbility();
    record->DisconnectAbilityWithWant(abilityRequest.want);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}