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

#ifndef OHOS_ABILITY_RUNTIME_POLICY_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_POLICY_INTERFACE_H

#include "ierror_observer.h"
#include "iremote_object.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct ExecutionPolicy : public Parcelable {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ExecutionPolicy *Unmarshalling(Parcel &parcel);
    uint32_t executionQuantity;
    uint32_t intervalTime;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_POLICY_INTERFACE_H