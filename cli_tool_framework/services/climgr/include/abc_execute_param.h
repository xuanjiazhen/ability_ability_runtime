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

#ifndef OHOS_ABILITY_RUNTIME_ABC_EXECUTE_PARAM_H
#define OHOS_ABILITY_RUNTIME_ABC_EXECUTE_PARAM_H

#include <parcel.h>
#include <string>
#include <vector>

namespace OHOS {
namespace CliTool {

/**
 * @brief AbcExecuteParam - Parameters for abc file execution
 */
class AbcExecuteParam : public Parcelable {
    // 方式1: 直接提供 abc 文件路径（绝对路径）
    std::string abcPath;

    // 方式2: 通过 HSP 信息自动查找 abc 路径
    std::string hspBundleName;     // HSP 的 bundle 名称（如 "com.example.library"）
    std::string hspModuleName;     // HSP 的 module 名称（如 "library"）

    // 函数信息
    std::string functionName;      // 函数名（空则调用 main）

    // 参数
    std::vector<std::string> args;  // 字符串参数

    // Context 信息（调用方）
    uint32_t callerTokenId = 0;
    std::string bundleName;        // 调用方 bundle 名称（用于初始化 Runtime）
    std::string moduleName;        // 调用方 module 名称（可选）

    /**
     * @brief Check if essential parameters are valid
     * 方式1和方式2至少需要一种
     */
    bool IsValid() const
    {
        // 方式1: 直接提供 abc 路径
        if (!abcPath.empty() && abcPath[0] == '/') {
            return true;
        }
        // 方式2: 通过 HSP 信息查找
        if (!hspBundleName.empty() && !hspModuleName.empty()) {
            return true;
        }
        return false;
    }

    /**
     * @brief Check if using HSP lookup mode
     */
    bool UseHspLookup() const
    {
        return abcPath.empty() && !hspBundleName.empty() && !hspModuleName.empty();
    }

    /**
     * @brief Marshalling for IPC
     */
    bool Marshalling(Parcel &parcel) const override;

    /**
     * @brief Unmarshalling for IPC
     */
    static AbcExecuteParam *Unmarshalling(Parcel &parcel);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABC_EXECUTE_PARAM_H
