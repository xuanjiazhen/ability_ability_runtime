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

#include "abilityautostartupservicec_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#define protected public
#include "ability_auto_startup_service.h"
#undef protected
#undef private

#include "ability_record.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
// App index 0 selects the normal (non-clone, non-sandbox) GetBundleInfo branch.
constexpr int32_t DEFAULT_APP_INDEX = 0;
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::shared_ptr<AbilityAutoStartupService> service = std::make_shared<AbilityAutoStartupService>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t intParam = fdp.ConsumeIntegral<int32_t>();
    bool boolParam = fdp.ConsumeBool();

    AutoStartupInfo info;
    info.bundleName = bundleName;
    info.moduleName = moduleName;
    info.abilityName = abilityName;
    info.appCloneIndex = intParam;
    info.accessTokenId = bundleName;
    info.setterUserId = intParam;
    info.userId = intParam;

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.bundleName = bundleName;
    abilityInfo.name = abilityName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.type = static_cast<AppExecFwk::AbilityType>(fdp.ConsumeIntegral<int32_t>());

    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfo.bundleName = bundleName;
    extensionInfo.name = abilityName;
    extensionInfo.moduleName = moduleName;
    extensionInfo.type = static_cast<AppExecFwk::ExtensionAbilityType>(fdp.ConsumeIntegral<int32_t>());

    AutoStartupAbilityData abilityData;

    service->GetValidUserId(intParam);
    service->GetAbilityTypeName(abilityInfo);
    service->GetExtensionTypeName(extensionInfo);
    service->IsTargetAbility(info, abilityInfo);
    service->IsTargetExtension(info, extensionInfo);
    service->CheckSelfApplication(bundleName);
    service->GetSelfApplicationBundleName();
    service->GetBundleMgrClient();

    AppExecFwk::BundleInfo bundleInfo;
    service->GetBundleInfo(bundleName, intParam, intParam, bundleInfo);
    service->GetBundleInfo(bundleName, intParam, DEFAULT_APP_INDEX, bundleInfo);
    service->GetAbilityData(info, abilityData);
    service->GetAbilityInfo(info, abilityData);
    service->CheckHiddenStartConditions(boolParam, abilityData);
    service->CheckPermissionForSystem();
    service->CheckPermissionForSelf(bundleName);
    service->CheckPermissionForEDM();
    service->InnerApplicationAutoStartupByEDM(info, boolParam, boolParam);
    service->SetApplicationAutoStartupByEDM(info, boolParam, boolParam);
    service->CancelApplicationAutoStartupByEDM(info, boolParam);

    std::vector<sptr<IRemoteObject>> callbackVector;
    service->GetCallbackVector(callbackVector);

    Want want;
    want.SetBundle(bundleName);
    want.SetAbilityName(abilityName);
    service->HiddenStartAutoStartupApp(want, intParam);

    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = bundleName;
    abilityRequest.abilityInfo.name = abilityName;
    abilityRequest.abilityInfo.type = AbilityType::DATA;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord != nullptr) {
        auto task = service->CreateHiddenStartCheckTask(abilityRecord);
        if (task) {
            task();
        }
    }
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
