/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "data_ability_result.h"

#include <memory>

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {

DataAbilityResult::DataAbilityResult(int count) : uri_("")
{
    count_ = count;
}

DataAbilityResult::DataAbilityResult(Parcel &parcel) : uri_(""), count_(0)
{
    ReadFromParcel(parcel);
}

DataAbilityResult::DataAbilityResult(const Uri &uri) : uri_(uri.ToString()), count_(0)
{}

DataAbilityResult::DataAbilityResult(const Uri &uri, int count) : uri_(uri.ToString())
{
    count_ = count;
}

DataAbilityResult::~DataAbilityResult()
{}

Uri DataAbilityResult::GetUri()
{
    return uri_;
}

int DataAbilityResult::GetCount()
{
    return count_;
}

DataAbilityResult *DataAbilityResult::CreateFromParcel(Parcel &parcel)
{
    DataAbilityResult *dataAbilityResult = new (std::nothrow) DataAbilityResult(parcel);
    if (dataAbilityResult == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null dataAbilityResult");
    }
    return dataAbilityResult;
}

std::string DataAbilityResult::ToString()
{
    std::string stringBuilder = "DataAbilityResult(";
    stringBuilder.append("uri=").append(uri_.ToString()).append(" ");
    stringBuilder.append("count=").append(std::to_string(count_)).append(" ");
    stringBuilder.erase(stringBuilder.length() - 1, 1);
    stringBuilder.append(")");
    return stringBuilder;
}

bool DataAbilityResult::Marshalling(Parcel &parcel) const
{
    // uri_
    if (uri_.ToString().empty()) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, VALUE_NULL);
    } else {
        if (!parcel.WriteInt32(VALUE_OBJECT)) {
            return false;
        }
        if (!parcel.WriteParcelable(&uri_)) {
            return false;
        }
    }

    // count_
    if (!parcel.WriteInt32(count_)) {
        return false;
    }

    return true;
}

DataAbilityResult *DataAbilityResult::Unmarshalling(Parcel &parcel)
{
    DataAbilityResult *dataAbilityResult = new (std::nothrow) DataAbilityResult(0);
    if (dataAbilityResult != nullptr) {
        if (!dataAbilityResult->ReadFromParcel(parcel)) {
            delete dataAbilityResult;
            dataAbilityResult = nullptr;
        }
    }

    return dataAbilityResult;
}

bool DataAbilityResult::ReadFromParcel(Parcel &parcel)
{
    // uri_
    int32_t empty = VALUE_NULL;
    if (!parcel.ReadInt32(empty)) {
        return false;
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "Failed to read parcel");
    }

    if (empty == VALUE_OBJECT) {
        auto uri = parcel.ReadParcelable<Uri>();
        if (uri != nullptr) {
            uri_ = *uri;
            delete uri;
            uri = nullptr;
        } else {
            return false;
            TAG_LOGD(AAFwkTag::DATA_ABILITY, "Failed to read parcel");
        }
    }

    // count_
    if (!parcel.ReadInt32(count_)) {
        return false;
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
