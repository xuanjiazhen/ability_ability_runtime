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
#include "parcel.h"
#include "services/common/include/hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

bool AbcExecuteParam::Marshalling(Parcel &parcel) const
{
    // Marshal abcPath
    if (!parcel.WriteString(abcPath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write abcPath failed");
        return false;
    }
    // Marshal HSP info
    if (!parcel.WriteString(hspBundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write hspBundleName failed");
        return false;
    }
    if (!parcel.WriteString(hspModuleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write hspModuleName failed");
        return false;
    }
    // Marshal functionName
    if (!parcel.WriteString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write functionName failed");
        return false;
    }
    // Marshal args - use WriteStringVector for consistency
    if (!parcel.WriteStringVector(args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write args failed");
        return false;
    }
    // Marshal context info
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

bool AbcExecuteParam::ReadFromParcel(Parcel &parcel)
{
    // Read abcPath
    if (!parcel.ReadString(abcPath)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read abcPath failed");
        return false;
    }
    // Read HSP info
    if (!parcel.ReadString(hspBundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read hspBundleName failed");
        return false;
    }
    if (!parcel.ReadString(hspModuleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read hspModuleName failed");
        return false;
    }
    // Read functionName
    if (!parcel.ReadString(functionName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read functionName failed");
        return false;
    }
    // Read args - use ReadStringVector for consistency
    if (!parcel.ReadStringVector(&args)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read args failed");
        return false;
    }
    // Read context info
    if (!parcel.ReadUint32(callerTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read callerTokenId failed");
        return false;
    }
    if (!parcel.ReadString(bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read bundleName failed");
        return false;
    }
    if (!parcel.ReadString(moduleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read moduleName failed");
        return false;
    }

    return true;
}

AbcExecuteParam *AbcExecuteParam::Unmarshalling(Parcel &parcel)
{
    auto *param = new (std::nothrow) AbcExecuteParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        return nullptr;
    }
    return param;
}

} // namespace CliTool
} // namespace OHOS
