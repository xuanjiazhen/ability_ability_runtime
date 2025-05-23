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

#include "am_abs_shared_result_set.h"

#include "ishared_result_set.h"
namespace OHOS {
namespace NativeRdb {
bool AmAbsSharedResultSet::Marshalling(Parcel &parcel) const
{
    if (resultSet_ == nullptr) {
        return false;
    }
    MessageParcel& data = static_cast<MessageParcel&>(parcel);
    auto result = NativeRdb::ISharedResultSet::WriteToParcel(std::move(resultSet_), data);
    if (result == nullptr) {
        return false;
    }
    return true;
}

AmAbsSharedResultSet* AmAbsSharedResultSet::Unmarshalling(Parcel &parcel)
{
    MessageParcel& data = static_cast<MessageParcel&>(parcel);
    std::shared_ptr<AbsSharedResultSet> sharedPtr = NativeRdb::ISharedResultSet::ReadFromParcel(data);
    return new (std::nothrow) AmAbsSharedResultSet(sharedPtr);
}

std::shared_ptr<AbsSharedResultSet> AmAbsSharedResultSet::GetResultSet() const
{
    return resultSet_;
}
} // namespace NativeRdb
} // namespace OHOS
