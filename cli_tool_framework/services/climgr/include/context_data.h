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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_DATA_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_DATA_H

#include <string>
#include <sstream>

namespace OHOS {
namespace CliTool {

/**
 * @brief ContextData - Serializable context information for passing to child process
 *
 * Contains essential context information for ArkTS runtime initialization
 * in the child process.
 */
struct ContextData {
    std::string bundleName;      // Bundle name for runtime initialization
    std::string moduleName;      // Module name (optional)
    uint32_t tokenId = 0;        // Caller's token ID
    std::string hapPath;         // HAP installation path (optional)

    /**
     * @brief Serialize to string for IPC transfer
     */
    std::string Serialize() const
    {
        std::ostringstream oss;
        oss << bundleName << "\n"
            << moduleName << "\n"
            << tokenId << "\n"
            << hapPath << "\n";
        return oss.str();
    }

    /**
     * @brief Deserialize from string
     */
    static ContextData Deserialize(const std::string &data)
    {
        ContextData ctx;
        std::istringstream iss(data);
        std::string line;
        int lineNum = 0;

        while (std::getline(iss, line)) {
            switch (lineNum++) {
                case 0: ctx.bundleName = line; break;
                case 1: ctx.moduleName = line; break;
                case 2: ctx.tokenId = static_cast<uint32_t>(std::stoul(line)); break;
                case 3: ctx.hapPath = line; break;
            }
        }
        return ctx;
    }

    bool IsValid() const
    {
        return !bundleName.empty() && tokenId != 0;
    }
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CONTEXT_DATA_H
