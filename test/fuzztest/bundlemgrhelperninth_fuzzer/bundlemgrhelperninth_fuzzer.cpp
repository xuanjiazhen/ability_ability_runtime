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

#include "bundlemgrhelperninth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#include "bundle_mgr_helper.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

void ExerciseUncoveredWrappers(FuzzedDataProvider &fdp, const std::shared_ptr<BundleMgrHelper> &bms)
{
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t appIndex = fdp.ConsumeIntegral<int32_t>();
    uint32_t flags = fdp.ConsumeIntegral<uint32_t>();
    bool isFirstLaunched = fdp.ConsumeBool();

    std::string name;
    int32_t index = 0;
    bms->GetNameAndIndexForUid(fdp.ConsumeIntegral<int32_t>(), name, index);

    BundleInfo bundleInfo;
    bms->GetBundleInfoV9(bundleName, fdp.ConsumeIntegral<int32_t>(), bundleInfo, userId);
    bms->GetCloneBundleInfoExt(bundleName, flags, appIndex, userId, bundleInfo);

    std::vector<AbilityInfo> abilityInfos;
    bms->GetLauncherAbilityInfoSync(bundleName, userId, abilityInfos);

    bms->SetBundleFirstLaunch(bundleName, userId, appIndex, isFirstLaunched);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto bms = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bms == nullptr) {
        return false;
    }
    ExerciseUncoveredWrappers(fdp, bms);
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
