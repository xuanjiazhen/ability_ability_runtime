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

#include "abilityinterfacesappmgrapplicationstatefilter_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#define protected public
#include "application_state_filter.h"
#undef protected
#undef private
#include "parcel.h"
#include <iostream>
#include "securec.h"
#include "configuration.h"
using namespace OHOS::AppExecFwk;
namespace OHOS {
bool DoSomethingInterestingWithMyAPI(FuzzedDataProvider *fdp)
{
    FilterCallback callbacks = static_cast<FilterCallback>(fdp->ConsumeIntegral<uint32_t>());
    FilterBundleType bundleTypes = static_cast<FilterBundleType>(fdp->ConsumeIntegral<uint32_t>());
    FilterAppStateType appStateTypes = static_cast<FilterAppStateType>(fdp->ConsumeIntegral<uint32_t>());
    FilterProcessStateType processStateTypes = static_cast<FilterProcessStateType>(fdp->ConsumeIntegral<uint32_t>());
    FilterAbilityStateType abilityStateTypes = static_cast<FilterAbilityStateType>(fdp->ConsumeIntegral<uint32_t>());
    AppStateFilter appStateFilter(callbacks, bundleTypes, appStateTypes, processStateTypes, abilityStateTypes);

    MessageParcel parcel1;
    parcel1.WriteUint32(static_cast<uint32_t>(callbacks));
    parcel1.WriteUint32(static_cast<uint32_t>(bundleTypes));
    parcel1.WriteUint32(static_cast<uint32_t>(appStateTypes));
    parcel1.WriteUint32(static_cast<uint32_t>(processStateTypes));
    parcel1.WriteUint32(static_cast<uint32_t>(abilityStateTypes));
    AppStateFilter fromParcel;
    fromParcel.ReadFromParcel(parcel1);

    MessageParcel parcel2;
    parcel2.WriteUint32(static_cast<uint32_t>(callbacks));
    parcel2.WriteUint32(static_cast<uint32_t>(bundleTypes));
    parcel2.WriteUint32(static_cast<uint32_t>(appStateTypes));
    parcel2.WriteUint32(static_cast<uint32_t>(processStateTypes));
    parcel2.WriteUint32(static_cast<uint32_t>(abilityStateTypes));

    AppStateFilter* unmarshalled = AppStateFilter::Unmarshalling(parcel2);
    if (unmarshalled != nullptr) {
        delete unmarshalled;
    }

    MessageParcel parcel3;
    appStateFilter.Marshalling(parcel3);

    appStateFilter.Match(appStateFilter);

    ApplicationState applicationState = static_cast<ApplicationState>(fdp->ConsumeIntegral<uint32_t>());
    FilterAppStateType filterAppState  = GetFilterTypeFromApplicationState(applicationState);

    AppProcessState appProcessState = static_cast<AppProcessState>(fdp->ConsumeIntegral<uint32_t>());
    FilterProcessStateType filterProcessStateType = GetFilterTypeFromAppProcessState(appProcessState);

    AbilityState abilityState = static_cast<AbilityState>(fdp->ConsumeIntegral<uint32_t>());
    FilterAbilityStateType filterAbilityStateType = GetFilterTypeFromAbilityState(abilityState);

    ExtensionState extensionState = static_cast<ExtensionState>(fdp->ConsumeIntegral<uint32_t>());
    FilterAbilityStateType filterAbilityStateType1 = GetFilterTypeFromExtensionState(extensionState);

    BundleType bundleType = static_cast<BundleType>(fdp->ConsumeIntegral<uint32_t>());
    FilterBundleType filterBundleType = GetFilterTypeFromBundleType(bundleType);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::DoSomethingInterestingWithMyAPI(&fdp);
    return 0;
}
