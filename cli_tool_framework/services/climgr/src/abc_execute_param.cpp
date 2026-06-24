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

namespace OHOS {
namespace CliTool {

bool AbcExecuteParam::Marshalling(Parcel &parcel) const
{
    // Marshal abcPath
    if (!parcel.WriteString(abcPath)) {
        return false;
    }
    // Marshal HSP info
    if (!parcel.WriteString(hspBundleName)) {
        return false;
    }
    if (!parcel.WriteString(hspModuleName)) {
        return false;
    }
    // Marshal functionName
    if (!parcel.WriteString(functionName)) {
        return false;
    }

    // Marshal args
    if (!parcel.WriteInt32(static_cast<int32_t>(args.size()))) {
        return false;
    }
    for (const auto &arg : args) {
        if (!parcel.WriteString(arg)) {
            return false;
        }
    }

    // Marshal context info
    if (!parcel.WriteUint32(callerTokenId)) {
        return false;
    }
    if (!parcel.WriteString(bundleName)) {
        return false;
    }
    if (!parcel.WriteString(moduleName)) {
        return false;
    }

    return true;
}

bool AbcExecuteParam::ReadFromParcel(Parcel &parcel)
{
    // Read abcPath
    if (!parcel.ReadString(abcPath)) {
        return false;
    }
    // Read HSP info
    if (!parcel.ReadString(hspBundleName)) {
        return false;
    }
    if (!parcel.ReadString(hspModuleName)) {
        return false;
    }
    // Read functionName
    if (!parcel.ReadString(functionName)) {
        return false;
    }

    // Read args
    int32_t argsSize = 0;
    if (!parcel.ReadInt32(argsSize)) {
        return false;
    }
    args.clear();
    args.reserve(argsSize);
    for (int32_t i = 0; i < argsSize; i++) {
        std::string arg;
        if (!parcel.ReadString(arg)) {
            return false;
        }
        args.push_back(arg);
    }

    // Read context info
    if (!parcel.ReadUint32(callerTokenId)) {
        return false;
    }
    if (!parcel.ReadString(bundleName)) {
        return false;
    }
    if (!parcel.ReadString(moduleName)) {
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
