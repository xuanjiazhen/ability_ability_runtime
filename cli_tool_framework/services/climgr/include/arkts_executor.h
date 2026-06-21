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

#ifndef OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H

#include <string>
#include <vector>

#include "cli_error_code.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief ArktsExecutor - Executes ArkTS abc files using a dedicated executor process
 *
 * This class provides functionality to execute ArkTS bytecode files by spawning
 * a separate abc_executor process. This avoids the threading issues that would
 * occur from fork()ing a process that contains an initialized ArkTS runtime.
 *
 * The abc_executor binary:
 * - Initializes a fresh ArkTS runtime environment
 * - Loads the specified .abc file
 * - Executes the main method
 * - Returns an appropriate exit code
 */
class ArktsExecutor {
public:
    /**
     * @brief Execute an ArkTS abc file by spawning abc_executor process
     * @param abcPath Path to the .abc file (must be absolute path)
     * @param args Command line arguments to pass to the abc entry method
     * @param bundleName Bundle name for ArkTS runtime initialization
     * @return int32_t ERR_OK on success, error code otherwise
     *
     * This method forks a child process and execs the abc_executor binary.
     * The abc executor will:
     * 1. Initialize the ArkTS runtime in a clean process
     * 2. Load the abc file
     * 3. Execute its main method with the provided arguments
     *
     * The abc file must contain a class with a 'main(args: string[])' method
     * that will receive the arguments as a string array.
     */
    static int32_t ExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H
