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
#include <string>
#include <vector>

#define private public
#include "ability_record.h"
#include "base_extension_record.h"
#undef private

#include "ability_fuzz_util.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

void ExerciseRecordQueries(FuzzedDataProvider &fdp, const std::shared_ptr<BaseExtensionRecord> &record)
{
    std::vector<std::string> info;
    record->DumpService(info);
    std::vector<std::string> params;
    bool isClient = fdp.ConsumeBool();
    record->DumpService(info, params, isClient);

    record->GetAbilityRecordType();
    record->GetClientPid();
    record->GetConnRemoteObject();
    record->GetRequestId();
    record->GetConnectRecordList();
    record->GetConnectingRecordList();
    record->GetConnectedListSize();
    record->GetConnectingListSize();
    record->GetInProgressRecordCount();
    record->IsConnectListEmpty();
    record->NeedConnectAfterCommand();
}

void ExerciseRecordMutations(FuzzedDataProvider &fdp, const std::shared_ptr<BaseExtensionRecord> &record)
{
    pid_t clientPid = fdp.ConsumeIntegral<pid_t>();
    record->SetClientPid(clientPid);
    std::string requestId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    record->SetRequestId(requestId);
    sptr<IRemoteObject> remote = fdp.ConsumeBool() ? record->GetToken() : nullptr;
    record->SetConnRemoteObject(remote);

    record->AddConnectRecordToList(nullptr);
    record->RemoveConnectRecordFromList(nullptr);
    // Null entries exercise the null-guarded connection list query branches.
    record->connRecordList_.emplace_back(nullptr);
    record->GetConnectRecordList();
    record->GetConnectingRecordList();
    record->GetConnectedListSize();
    record->GetConnectingListSize();
    record->GetInProgressRecordCount();
    record->connRecordList_.clear();

    Want want;
    want.SetElementName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    record->ConnectAbility();
    record->ConnectAbilityWithWant(want);
    record->DisconnectAbility();
    record->DisconnectAbilityWithWant(want);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    AbilityRequest abilityRequest;
    AbilityFuzzUtil::GenerateAbilityRequestInfo(fdp, abilityRequest);
    abilityRequest.abilityInfo.extensionAbilityType = fdp.ConsumeBool() ?
        AppExecFwk::ExtensionAbilityType::SERVICE :
        static_cast<AppExecFwk::ExtensionAbilityType>(fdp.ConsumeIntegral<int32_t>());
    std::shared_ptr<BaseExtensionRecord> record = BaseExtensionRecord::CreateBaseExtensionRecord(abilityRequest);
    if (record == nullptr) {
        return false;
    }
    ExerciseRecordQueries(fdp, record);
    ExerciseRecordMutations(fdp, record);

    std::shared_ptr<BaseExtensionRecord> transferred =
        BaseExtensionRecord::TransferToExtensionRecordBase(record);
    (void)transferred;
    BaseExtensionRecord::TransferToExtensionRecordBase(nullptr);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
