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

#include "atomic_service_startup_rule.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AbilityRuntime {

bool AtomicServiceStartupRule::ReadFromParcel(Parcel &parcel)
{
    isOpenAllowed = parcel.ReadBool();
    isEmbeddedAllowed = parcel.ReadBool();
    return true;
}

AtomicServiceStartupRule *AtomicServiceStartupRule::Unmarshalling(Parcel &parcel)
{
    AtomicServiceStartupRule *info = new (std::nothrow) AtomicServiceStartupRule();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool AtomicServiceStartupRule::Marshalling(Parcel &parcel) const
{
    parcel.WriteBool(isOpenAllowed);
    parcel.WriteBool(isEmbeddedAllowed);
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS