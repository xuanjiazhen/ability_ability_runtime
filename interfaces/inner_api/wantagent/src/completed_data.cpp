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

#include "completed_data.h"

namespace OHOS {
namespace OHOS::AbilityRuntime::WantAgent {
bool CompletedData::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteParcelable(&pendingWant) && parcel.WriteParcelable(&want) && parcel.WriteInt32(resultCode) &&
        parcel.WriteString(resultData) && parcel.WriteParcelable(&resultExtras));
}

bool CompletedData::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<PendingWant> pendingWantResquest(parcel.ReadParcelable<PendingWant>());
    if (pendingWant == nullptr) {
        return false;
    }
    pendingWant = *pendingWantResquest;
    std::unique_ptr<Want> wantResquest(parcel.ReadParcelable<Want>());
    if (want == nullptr) {
        return false;
    }
    want = *wantResquest;
    resultCode = parcel.ReadInt32();
    resultData = parcel.ReadString();
    std::unique_ptr<WantParams> resultExtrasResquest(parcel.ReadParcelable<WantParams>());
    if (resultExtrasResquest == nullptr) {
        return false;
    }
    resultExtras = *resultExtrasResquest;
    return true;
}

CompletedData *CompletedData::Unmarshalling(Parcel &parcel)
{
    CompletedData *completedData = new (std::nothrow) CompletedData();
    if (completedData && !completedData->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "completedData failed, because ReadFromParcel failed");
        delete completedData;
        completedData = nullptr;
    }
    return completedData;
}
}  // namespace OHOS::AbilityRuntime::WantAgent
#endif  // OHOS_ABILITY_RUNTIME_COMPLETED_DATA_H