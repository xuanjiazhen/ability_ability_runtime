/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "invoke_function_result.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool InvokeFunctionResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(invokeSuccess)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write invokeSuccess.");
        return false;
    }
    if (!parcel.WriteInt32(errorCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write errorCode.");
        return false;
    }
    if (!parcel.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write resultCode.");
        return false;
    }
    if (result != nullptr) {
        if (!parcel.WriteBool(true)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result flag.");
            return false;
        }
        if (!parcel.WriteParcelable(result.get())) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result.");
            return false;
        }
    } else {
        if (!parcel.WriteBool(false)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result flag.");
            return false;
        }
    }
    if (!parcel.WriteString(message)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write message.");
        return false;
    }
    return true;
}

InvokeFunctionResult *InvokeFunctionResult::Unmarshalling(Parcel &parcel)
{
    auto *result = new (std::nothrow) InvokeFunctionResult();
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate InvokeFunctionResult.");
        return nullptr;
    }
    if (!parcel.ReadBool(result->invokeSuccess)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read invokeSuccess.");
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt32(result->errorCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read errorCode.");
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt32(result->resultCode)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read resultCode.");
        delete result;
        return nullptr;
    }
    bool hasResult = false;
    if (!parcel.ReadBool(hasResult)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read result flag.");
        delete result;
        return nullptr;
    }
    if (hasResult) {
        auto *wantParams = parcel.ReadParcelable<AAFwk::WantParams>();
        if (wantParams == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read result.");
            delete result;
            return nullptr;
        }
        result->result = std::shared_ptr<AAFwk::WantParams>(wantParams);
    }
    if (!parcel.ReadString(result->message)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read message.");
        delete result;
        return nullptr;
    }
    return result;
}
} // namespace CliTool
} // namespace OHOS
