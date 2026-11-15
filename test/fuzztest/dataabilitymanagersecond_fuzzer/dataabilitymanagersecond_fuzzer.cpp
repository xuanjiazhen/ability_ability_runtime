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

#include "dataabilitymanagersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "data_ability_manager.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
AbilityRequest CreateDataAbilityRequest()
{
    AbilityRequest request;
    request.appInfo.bundleName = "com.example.fuzzTest";
    request.abilityInfo.name = "DataAbility";
    request.abilityInfo.type = AbilityType::DATA;
    return request;
}
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    bool isNotHap = fdp.ConsumeBool();
    auto request = CreateDataAbilityRequest();
    auto record = std::make_shared<DataAbilityRecord>(request);
    auto ability = AbilityRecord::CreateAbilityRecord(request);
    DataAbilityManager mgr;

    // null client + null record
    std::shared_ptr<DataAbilityRecord> nullRecord;
    mgr.ReportDataAbilityAcquired(nullptr, isNotHap, nullRecord);
    mgr.ReportDataAbilityReleased(nullptr, isNotHap, nullRecord);

    // null client + real record
    mgr.ReportDataAbilityAcquired(nullptr, isNotHap, record);
    mgr.ReportDataAbilityReleased(nullptr, isNotHap, record);

    // real client token + real record
    if (ability) {
        sptr<IRemoteObject> token = ability->GetToken();
        mgr.ReportDataAbilityAcquired(token, isNotHap, record);
        mgr.ReportDataAbilityReleased(token, isNotHap, record);
    }
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
