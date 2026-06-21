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

#ifndef OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H

#include <string>
#include <vector>

#include "ani_common.h"
#include "cli_error_code.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief ArktsExecutor - Executes ArkTS abc files in child processes
 *
 * This class provides functionality to dynamically execute ArkTS bytecode
 * files without requiring module.json5 configuration.
 */
class ArktsExecutor {
public:
    /**
     * @brief Execute an ArkTS abc file by forking a child process
     * @param abcPath Path to the .abc file (absolute path)
     * @param args Command line arguments to pass to the abc entry method
     * @param bundleName Bundle name for ArkTS runtime initialization
     * @return int32_t ERR_OK on success, error code otherwise
     *
     * The abc file must contain a class with a 'main(args: string[])' method
     * that will receive the arguments as a string array.
     */
    static int32_t ExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName);

private:
    /**
     * @brief Child process entry - initializes runtime and executes abc
     * @param abcPath Path to .abc file
     * @param args Arguments to pass
     * @param bundleName Bundle name for runtime initialization
     */
    static void ChildProcessEntry(const std::string &abcPath, const std::vector<std::string> &args, const std::string &bundleName);

    /**
     * @brief Initialize ArkTS Runtime in the child process
     * @param bundleName Bundle name for runtime initialization
     * @return bool true if successful
     */
    static bool InitializeArkTsRuntime(const std::string &bundleName);

    /**
     * @brief Load and execute abc file
     * @param abcPath Path to .abc file
     * @param args Arguments to pass to the main method
     * @return bool true if successful
     */
    static bool LoadAndExecuteAbc(const std::string &abcPath, const std::vector<std::string> &args);

    /**
     * @brief Create ANI string array from C++ string vector
     * @param env ANI environment
     * @param args Input string vector
     * @param aniArgs Output ANI array
     * @return bool true if successful
     */
    static bool CreateAniStringArray(ani_env *env, const std::vector<std::string> &args, ani_array &aniArgs);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ARKTS_EXECUTOR_H
