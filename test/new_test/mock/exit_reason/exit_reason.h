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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_EXIT_REASON_H
#define MOCK_OHOS_ABILITY_RUNTIME_EXIT_REASON_H

#include <string>

#include "ability_state.h"
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
struct ExitReason : public Parcelable {
    ExitReason() = default;
    ExitReason(const Reason reason, const std::string &exitMsg) {}
    ExitReason(const Reason &reason, int32_t subReason, const std::string &exitMsg) {}

    Reason reason = Reason::REASON_UNKNOWN;
    int32_t subReason = -1;
    std::string exitMsg = "";

    bool ReadFromParcel(Parcel &parcel)
    {
        return false;
    }
    virtual bool Marshalling(Parcel &parcel) const override
    {
        return false;
    }
    static ExitReason *Unmarshalling(Parcel &parcel)
    {
        return nullptr;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
