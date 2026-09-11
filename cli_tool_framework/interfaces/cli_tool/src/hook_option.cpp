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

#include "hook_option.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
bool HookOption::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(dmSessionId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write dmSessionId.");
        return false;
    }
    if (!parcel.WriteInt32(toolCallId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write toolCallId.");
        return false;
    }
    return true;
}

HookOption *HookOption::Unmarshalling(Parcel &parcel)
{
    auto *option = new (std::nothrow) HookOption();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate HookOption.");
        return nullptr;
    }
    if (!parcel.ReadInt32(option->dmSessionId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read dmSessionId.");
        delete option;
        return nullptr;
    }
    if (!parcel.ReadInt32(option->toolCallId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read toolCallId.");
        delete option;
        return nullptr;
    }
    return option;
}
} // namespace CliTool
} // namespace OHOS
