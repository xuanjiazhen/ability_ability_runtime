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

#include "abilitymgrappexitreasonhelpersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

// Access private helper methods directly. The public API cannot reach
// AddProcessExitReason and the record helpers in the fuzz environment. The
// macro must be named after the C++ keyword to take effect, so it intentionally
// deviates from the UPPER_CASE macro naming rule.
#define private public
#include "app_exit_reason_helper.h"
#include "ability_manager_service.h"
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// Covers valid Reason values (0..REASON_MAX=11) and invalid values up to 20,
// so both the valid and invalid branches of IsExitReasonValid are exercised.
constexpr int32_t REASON_VALUE_MAX = 20;
} // namespace

ExitReasonCompability MakeExitReason(FuzzedDataProvider &fdp)
{
    ExitReasonCompability exitReason;
    exitReason.reason = static_cast<Reason>(fdp.ConsumeIntegralInRange<int32_t>(0, REASON_VALUE_MAX));
    exitReason.subReason = fdp.ConsumeIntegral<int32_t>();
    exitReason.killId = fdp.ConsumeIntegral<int32_t>();
    exitReason.exitMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    exitReason.killMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    exitReason.innerMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    return exitReason;
}

void ExerciseRecordPaths(FuzzedDataProvider &fdp, const std::shared_ptr<AppExitReasonHelper> &helper,
    int32_t pid, int32_t uid, int32_t userId, const std::string &bundleName,
    const std::string &abilityName, const ExitReasonCompability &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo)
{
    std::vector<AppExecFwk::RunningProcessInfo> processInfoList;
    processInfoList.emplace_back(processInfo);
    AppExecFwk::RunningProcessInfo invalidInfo;
    processInfoList.emplace_back(invalidInfo);

    helper->RecordAppWithReason(pid, uid, exitReason);
    helper->RecordAppsWithReasonByProcessInfoList(exitReason, processInfoList);
    helper->RecordInvalidKillId(pid, exitReason, bundleName, userId);
    helper->RecordInvalidKillId(pid, exitReason);

    std::vector<std::string> abilityList;
    abilityList.emplace_back(abilityName);
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    abilityInfo.name = abilityName;
    ExitReason exitReasonValue(static_cast<Reason>(fdp.ConsumeIntegralInRange<int32_t>(0, REASON_VALUE_MAX)),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    helper->RecordProcessExitReasonForTimeout(abilityInfo, exitReasonValue, abilityList, processInfo);
}

void ExerciseAddPaths(FuzzedDataProvider &fdp, const std::shared_ptr<AppExitReasonHelper> &helper,
    int32_t pid, int32_t uid, int32_t userId, int32_t appIndex, uint32_t accessTokenId,
    const std::string &bundleName, const ExitReasonCompability &exitReason,
    const AppExecFwk::RunningProcessInfo &processInfo)
{
    RecordExitReasonParams params;
    params.pid = pid;
    params.bundleName = bundleName;
    params.uid = uid;
    params.accessTokenId = accessTokenId;
    params.exitReason = exitReason;
    params.processInfo = processInfo;
    params.fromKillWithReason = fdp.ConsumeBool();
    helper->AddProcessExitReason(params);

    helper->AddAppExitReason(bundleName, pid, uid, appIndex, exitReason);
    helper->AddBundleExitReason(bundleName, userId, appIndex, exitReason);

    std::vector<AppExecFwk::RunningProcessInfo> infos = helper->GetRunningProcessInfos(userId, bundleName);
    (void)infos;
    helper->RecordAppWithReasonInner(exitReason, processInfo);
    AppExecFwk::RunningProcessInfo emptyInfo;
    helper->RecordAppWithReasonInner(exitReason, emptyInfo);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<SubManagersHelper> subManagersHelper;
    std::shared_ptr<AppExitReasonHelper> helper = std::make_shared<AppExitReasonHelper>(subManagersHelper);
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t appIndex = fdp.ConsumeIntegral<int32_t>();
    uint32_t accessTokenId = fdp.ConsumeIntegral<uint32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    ExitReasonCompability exitReason = MakeExitReason(fdp);

    AppExecFwk::RunningProcessInfo processInfo;
    processInfo.pid_ = pid;
    processInfo.uid_ = uid;
    processInfo.accessTokenId_ = accessTokenId;
    processInfo.bundleNames.emplace_back(bundleName);

    ExerciseRecordPaths(fdp, helper, pid, uid, userId, bundleName, abilityName, exitReason, processInfo);
    ExerciseAddPaths(fdp, helper, pid, uid, userId, appIndex, accessTokenId, bundleName, exitReason, processInfo);
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
