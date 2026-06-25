/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "abc_execute_param.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

bool AbcExecuteParam::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(abcPath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write abcPath failed");
        return false;
    }
    if (!parcel.WriteString(hspBundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write hspBundleName failed");
        return false;
    }
    if (!parcel.WriteString(hspModuleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write hspModuleName failed");
        return false;
    }
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionName failed");
        return false;
    }
    if (!parcel.WriteStringVector(args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write args failed");
        return false;
    }
    if (!parcel.WriteUint32(callerTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write callerTokenId failed");
        return false;
    }
    if (!parcel.WriteString(bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write bundleName failed");
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write moduleName failed");
        return false;
    }
    return true;
}

AbcExecuteParam *AbcExecuteParam::Unmarshalling(Parcel &parcel)
{
    auto *param = new (std::nothrow) AbcExecuteParam();
    if (param == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to allocate AbcExecuteParam");
        return nullptr;
    }

    if (!parcel.ReadString(param->abcPath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read abcPath failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->hspBundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read hspBundleName failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->hspModuleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read hspModuleName failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionName failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadStringVector(&param->args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read args failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadUint32(param->callerTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read callerTokenId failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read bundleName failed");
        delete param;
        return nullptr;
    }
    if (!parcel.ReadString(param->moduleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read moduleName failed");
        delete param;
        return nullptr;
    }

    return param;
}

} // namespace CliTool
} // namespace OHOS
