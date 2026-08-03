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

#include "abilityappmgreventfifth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#define private public
#include "app_mgr_event.h"
#undef private

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

void ExerciseEventSend(FuzzedDataProvider &fdp)
{
    std::shared_ptr<AppRunningRecord> callerAppRecord;
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string stringParam = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    AppMgrEventUtil::SendCreateAtomicServiceProcessEvent(callerAppRecord, appRecord, stringParam, stringParam);

    AAFwk::EventInfo eventInfo;
    AppMgrEventUtil::SendProcessStartEvent(callerAppRecord, appRecord, eventInfo);

    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    int64_t restartTime = fdp.ConsumeIntegral<int64_t>();
    AppMgrEventUtil::SendReStartProcessEvent(eventInfo, appUid, restartTime);
    AppMgrEventUtil::GetCallerPid(callerAppRecord);

    std::shared_ptr<AbilityInfo> abilityInfo;
    int32_t abilityType = fdp.ConsumeIntegral<int32_t>();
    int32_t extensionType = fdp.ConsumeIntegral<int32_t>();
    AppMgrEventUtil::UpdateStartupType(abilityInfo, abilityType, extensionType);

    std::shared_ptr<ChildProcessRecord> childRecord;
    AppMgrEventUtil::SendChildProcessStartFailedEvent(childRecord,
        static_cast<ProcessStartFailedReason>(fdp.ConsumeIntegral<int32_t>()), fdp.ConsumeIntegral<int32_t>());
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    ExerciseEventSend(fdp);
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
