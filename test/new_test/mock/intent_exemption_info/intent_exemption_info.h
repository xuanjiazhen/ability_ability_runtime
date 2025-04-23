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

#ifndef MOCK_OHOS_ABILITY_INTENT_EXEMPTION_INFO_H
#define MOCK_OHOS_ABILITY_INTENT_EXEMPTION_INFO_H

#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct IntentExemptionInfo : public Parcelable {
    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }

    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }

    static IntentExemptionInfo *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }

    int32_t uid_ = 0;
    int64_t duration_ = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
