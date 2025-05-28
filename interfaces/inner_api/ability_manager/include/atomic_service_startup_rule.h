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

#ifndef OHOS_ATOMIC_SERVICE_STARTUP_RULE_H
#define OHOS_ATOMIC_SERVICE_STARTUP_RULE_H

#include <parcel.h>
#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {
struct AtomicServiceStartupRule : public Parcelable {
    AtomicServiceStartupRule()
    {
        isOpenAllowed = false;
        isEmbeddedAllowed = false;
    }
    AtomicServiceStartupRule(bool OpenAllowed, bool EmbeddedAllowed) : isOpenAllowed(OpenAllowed),
        isEmbeddedAllowed(EmbeddedAllowed) {}
    bool isOpenAllowed = false;
    bool isEmbeddedAllowed = false;
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AtomicServiceStartupRule *Unmarshalling(Parcel &parcel);
};
}
}

#endif // OHOS_ATOMIC_SERVICE_STARTUP_RULE_H