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

#ifndef OHOS_NativeRdb_AM_ABS_SHARED_RESULT_SET_H
#define OHOS_NativeRdb_AM_ABS_SHARED_RESULT_SET_H

#include <memory>
#include <parcel.h>

namespace OHOS::NativeRdb {
class AbsSharedResultSet;

class AmAbsSharedResultSet : public Parcelable {
public:
    AmAbsSharedResultSet() = default;
    explicit AmAbsSharedResultSet(const std::shared_ptr<AbsSharedResultSet>& resultSet) :
        resultSet_(resultSet) {}
    ~AmAbsSharedResultSet() = default;

    bool Marshalling(Parcel& parcel) const override;
    static AmAbsSharedResultSet* Unmarshalling(Parcel& parcel);
    std::shared_ptr<AbsSharedResultSet> GetResultSet() const;

private:
    std::shared_ptr<AbsSharedResultSet> resultSet_;
};
} // namespace OHOS::NativeRdb

#endif // OHOS_NativeRdb_AM_ABS_SHARED_RESULT_SET_H