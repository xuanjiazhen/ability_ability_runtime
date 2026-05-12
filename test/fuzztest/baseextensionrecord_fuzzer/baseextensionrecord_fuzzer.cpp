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
#include <fuzzer/FuzzedDataProvider.h>

#include "base_extension_record.h"

#include "ability_manager_service.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int32_t API_CREATE_AND_TRANSFER = 0;
constexpr int32_t API_SIMPLE_GETTER_SETTER = 1;
constexpr int32_t API_CONNECT_MANAGEMENT = 2;
constexpr int32_t API_DUMP_SERVICE = 3;
constexpr int32_t API_TIMEOUT_TASK = 4;
constexpr int32_t API_RECORD_TYPE = 5;
constexpr int32_t MAX_API_CASE = API_RECORD_TYPE;
constexpr uint32_t INVALID_TIMEOUT_MSG = 0xFFFFFFFF;

AbilityRequest CreateExtensionRequest(ExtensionAbilityType extType)
{
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "TestExtension";
    request.abilityInfo.type = AbilityType::EXTENSION;
    request.abilityInfo.extensionAbilityType = extType;
    return request;
}
} // namespace

void FuzzCreateAndTransfer(FuzzedDataProvider &fdp)
{
    // normal extension
    auto request = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    // TransferToExtensionRecordBase: null
    BaseExtensionRecord::TransferToExtensionRecordBase(nullptr);
    // TransferToExtensionRecordBase: extension type
    if (record) {
        BaseExtensionRecord::TransferToExtensionRecordBase(record);
    }
    // TransferToExtensionRecordBase: non-extension (PAGE type)
    AbilityRequest pageReq;
    pageReq.abilityInfo.type = AbilityType::PAGE;
    auto pageRecord = AbilityRecord::CreateAbilityRecord(pageReq);
    if (pageRecord) {
        BaseExtensionRecord::TransferToExtensionRecordBase(pageRecord);
    }
    // APP_SERVICE + TYPE process mode
    auto appServiceReq = CreateExtensionRequest(ExtensionAbilityType::APP_SERVICE);
    appServiceReq.extensionProcessMode = ExtensionProcessMode::TYPE;
    auto appServiceRecord = BaseExtensionRecord::CreateBaseExtensionRecord(appServiceReq);
}

void FuzzSimpleGetterSetter(FuzzedDataProvider &fdp)
{
    auto request = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    if (!record) {
        return;
    }
    // ConnRemoteObject
    sptr<IRemoteObject> obj = nullptr;
    record->SetConnRemoteObject(obj);
    record->GetConnRemoteObject();
    // ClientPid
    pid_t pid = fdp.ConsumeIntegral<pid_t>();
    record->SetClientPid(pid);
    record->GetClientPid();
    // RequestId
    std::string requestId = fdp.ConsumeRandomLengthString(64);
    record->SetRequestId(requestId);
    record->GetRequestId();
    // Connect list queries on empty list
    record->IsConnectListEmpty();
    record->NeedConnectAfterCommand();
    record->GetConnectRecordList();
    record->GetConnectingRecord();
    record->GetConnectingRecordList();
    record->GetDisconnectingRecord();
    record->GetConnectedListSize();
    record->GetConnectingListSize();
    record->GetInProgressRecordCount();
}

void FuzzConnectManagement(FuzzedDataProvider &fdp)
{
    auto request = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    if (!record) {
        return;
    }
    // AddConnectRecordToList with null
    record->AddConnectRecordToList(nullptr);
    // RemoveConnectRecordFromList with null
    record->RemoveConnectRecordFromList(nullptr);
    // Connect/Disconnect without lifecycleDeal_
    record->DisconnectAbility();
    Want want;
    record->DisconnectAbilityWithWant(want);
    record->ConnectAbility();
    record->ConnectAbilityWithWant(want);
}

void FuzzDumpService(FuzzedDataProvider &fdp)
{
    // SERVICE extension type
    auto serviceReq = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto serviceRecord = BaseExtensionRecord::CreateBaseExtensionRecord(serviceReq);
    if (serviceRecord) {
        std::vector<std::string> info;
        serviceRecord->DumpService(info);
        std::vector<std::string> params;
        serviceRecord->DumpService(info, params);
    }
    // UI_SERVICE extension type
    auto uiServiceReq = CreateExtensionRequest(ExtensionAbilityType::UI_SERVICE);
    auto uiServiceRecord = BaseExtensionRecord::CreateBaseExtensionRecord(uiServiceReq);
    if (uiServiceRecord) {
        std::vector<std::string> info2;
        uiServiceRecord->DumpService(info2);
    }
    // UI_EXTENSION type (isUIExtension = true)
    auto uiExtReq = CreateExtensionRequest(ExtensionAbilityType::UI_EXTENSION);
    auto uiExtRecord = BaseExtensionRecord::CreateBaseExtensionRecord(uiExtReq);
    if (uiExtRecord) {
        std::vector<std::string> info3;
        uiExtRecord->DumpService(info3);
    }
}

void FuzzTimeoutTask(FuzzedDataProvider &fdp)
{
    auto request = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    if (!record) {
        return;
    }
    // LOAD_TIMEOUT_MSG
    record->PostUIExtensionAbilityTimeoutTask(
        AbilityManagerService::LOAD_TIMEOUT_MSG);
    // FOREGROUND_TIMEOUT_MSG
    record->PostUIExtensionAbilityTimeoutTask(
        AbilityManagerService::FOREGROUND_TIMEOUT_MSG);
    // unknown message (default branch)
    record->PostUIExtensionAbilityTimeoutTask(INVALID_TIMEOUT_MSG);
}

void FuzzRecordType(FuzzedDataProvider &fdp)
{
    auto request = CreateExtensionRequest(ExtensionAbilityType::SERVICE);
    auto record = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    if (!record) {
        return;
    }
    // GetAbilityRecordType should return EXTENSION_ABILITY
    auto type = record->GetAbilityRecordType();
    // verify it's extension type
    auto base = BaseExtensionRecord::TransferToExtensionRecordBase(record);
    if (base) {
        base->GetAbilityRecordType();
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto apiCase = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_API_CASE);
    switch (apiCase) {
        case API_CREATE_AND_TRANSFER:
            FuzzCreateAndTransfer(fdp);
            break;
        case API_SIMPLE_GETTER_SETTER:
            FuzzSimpleGetterSetter(fdp);
            break;
        case API_CONNECT_MANAGEMENT:
            FuzzConnectManagement(fdp);
            break;
        case API_DUMP_SERVICE:
            FuzzDumpService(fdp);
            break;
        case API_TIMEOUT_TASK:
            FuzzTimeoutTask(fdp);
            break;
        case API_RECORD_TYPE:
            FuzzRecordType(fdp);
            break;
        default:
            break;
    }
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
