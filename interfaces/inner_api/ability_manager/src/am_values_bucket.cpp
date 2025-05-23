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

#include "am_values_bucket.h"

#include "values_bucket.h"

namespace OHOS {
namespace NativeRdb {
bool AmValuesBucket::Marshalling(Parcel& parcel) const
{
    if (valuesBucket_) {
        return valuesBucket_->Marshalling(parcel);
    }
    return false;
}

AmValuesBucket* AmValuesBucket::Unmarshalling(Parcel& parcel)
{
    ValuesBucket bucket = ValuesBucket::Unmarshalling(parcel);
    std::shared_ptr<ValuesBucket> valuesBucket = std::make_shared<ValuesBucket>(bucket);
    return new (std::nothrow) AmValuesBucket(valuesBucket);
}

std::shared_ptr<ValuesBucket> AmValuesBucket::GetValuesBucket() const
{
    return valuesBucket_;
}
} // namespace OHOS::NativeRdb
} // namespace NativeRdb

