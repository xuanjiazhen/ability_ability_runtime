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
 * @brief Parameters for abc file execution
 * Supports two modes:
 * - Mode1: Direct abc file execution via absolute path
 * - Mode2: HSP-based execution with bundle/module name
 */
class AbcExecuteParam : public Parcelable {
public:
    std::string abcPath;              // Mode1: Path to abc file (absolute path)
    std::string hspBundleName;        // Mode2: HSP bundle name (e.g., "com.example.library")
    std::string hspModuleName;        // Mode2: HSP module name (e.g., "library")
    std::string functionName;          // Function name to call (empty for main)
    std::vector<std::string> args;    // Function arguments
    unsigned int callerTokenId = 0;   // Caller's token ID
    std::string bundleName;           // Bundle name for runtime initialization
    std::string moduleName;           // Module name (optional)

    AbcExecuteParam() = default;
    ~AbcExecuteParam() override = default;

    bool Marshalling(Parcel &parcel) const override;
    static AbcExecuteParam *Unmarshalling(Parcel &parcel);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABC_EXECUTE_PARAM_H
