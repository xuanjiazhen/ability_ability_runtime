/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "localpendingwantfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "local_pending_want.h"
#include "parcel.h"

using namespace OHOS::AbilityRuntime::WantAgent;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
const std::string GET_BUNDLE_INFO_PERMISSION = "ohos.permission.GET_BUNDLE_INFO";
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    
    std::shared_ptr<Want> want = std::make_shared<Want>();
    Parcel paramsParcel;
    paramsParcel.WriteString(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    paramsParcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
    std::shared_ptr<WantParams> extraInfo;
    WantParams* params = WantParams::Unmarshalling(paramsParcel);
    if (params) {
        extraInfo = std::make_shared<WantParams>(*params);
        delete params;
    }

    TriggerInfo paramsInfo(GET_BUNDLE_INFO_PERMISSION, extraInfo, want, 0);
    sptr<CompletedDispatcher> callBack;
    sptr<IRemoteObject> callerToken;

    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t operType = fdp.ConsumeIntegral<int32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t hashCode = fdp.ConsumeIntegral<int32_t>();
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    std::shared_ptr<Want> randomWant = std::make_shared<Want>();
    randomWant->SetElementName(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH), 
                               fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

    LocalPendingWant localPendingWant = LocalPendingWant(bundleName, want, operType);

    localPendingWant.SetBundleName(bundleName);
    localPendingWant.SetUid(uid);
    localPendingWant.SetType(operType);
    localPendingWant.SetWant(randomWant);
    localPendingWant.SetHashCode(hashCode);
    localPendingWant.SetTokenId(tokenId);

    localPendingWant.GetBundleName();
    localPendingWant.GetUid();
    localPendingWant.GetType();
    localPendingWant.GetWant();
    localPendingWant.GetHashCode();
    localPendingWant.GetTokenId();
    localPendingWant.Send(callBack, paramsInfo, callerToken);

    localPendingWant.Marshalling(paramsParcel);

    std::shared_ptr<LocalPendingWant> localWant = std::make_shared<LocalPendingWant>(bundleName, want, operType);
    std::shared_ptr<LocalPendingWant> otherWant = std::make_shared<LocalPendingWant>(bundleName, want, operType);
    LocalPendingWant::IsEquals(localWant, otherWant);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}