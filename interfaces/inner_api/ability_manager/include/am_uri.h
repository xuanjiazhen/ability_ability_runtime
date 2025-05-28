/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AM_URI_H
#define OHOS_ABILITY_RUNTIME_AM_URI_H

#include "uri.h"
#include "parcel.h"

namespace OHOS {
using Uri = OHOS::Uri;
struct AmUri : public Parcelable {
    Uri uri;
    AmUri() : uri("") {}
    AmUri(const Uri &uri) : uri(uri) {}

    bool ReadFromParcel(Parcel &parcel) {
        auto tempUri = parcel.ReadParcelable<Uri>();
        if (tempUri == nullptr) {
            return false;
        }
        uri = *tempUri;
        delete tempUri;
        tempUri = nullptr;
        return true;
    }

    virtual bool Marshalling(Parcel &parcel) const override {
        if (!parcel.WriteParcelable(&uri)) {
            return false;
        }
        return true;
    }

    static AmUri *Unmarshalling(Parcel &parcel) {
        AmUri *amUri = new (std::nothrow) AmUri();
        if (amUri == nullptr) {
            return nullptr;
        }

        if (!amUri->ReadFromParcel(parcel)) {
            delete amUri;
            amUri = nullptr;
        }
        return amUri;
    }
};
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AM_URI_H