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

#include "invoke_function_hook_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS::CliTool {
bool InvokeFunctionHookParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write functionNamespace.");
        return false;
    }
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write functionName.");
        return false;
    }
    if (!parcel.WriteParcelable(&args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write args.");
        return false;
    }
    return true;
}

InvokeFunctionHookParam *InvokeFunctionHookParam::Unmarshalling(Parcel &parcel)
{
    auto *param = new (std::nothrow) InvokeFunctionHookParam();
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate InvokeFunctionHookParam.");
        return nullptr;
    }
    if (!parcel.ReadString(param->functionNamespace)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read functionNamespace.");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read functionName.");
        delete param;
        return nullptr;
    }
    std::unique_ptr<AAFwk::WantParams> args(parcel.ReadParcelable<AAFwk::WantParams>());
    if (args == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read args.");
        delete param;
        return nullptr;
    }
    param->args = *args;
    return param;
}
}
