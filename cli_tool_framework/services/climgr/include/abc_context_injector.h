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

#ifndef OHOS_ABILITY_RUNTIME_ABC_CONTEXT_INJECTOR_H
#define OHOS_ABILITY_RUNTIME_ABC_CONTEXT_INJECTOR_H

#include <memory>
#include <string>

#include "ability_context.h"

// Forward declarations for ANI types
struct ani_env;
struct ani_ref;

namespace OHOS {
namespace AbilityRuntime {
class Context;
}

namespace CliTool {

/**
 * @brief AbcContextInjector - Injects Context into ArkTS runtime for abc files
 *
 * This class provides functionality to inject a Context object into the
 * ArkTS runtime so that abc files can access it to call system APIs.
 */
class AbcContextInjector {
public:
    /**
     * @brief Set the context to be injected
     * @param aniEnv The ANI environment
     * @param context The native context object
     */
    static void SetContext(ani_env *aniEnv, const std::shared_ptr<AbilityRuntime::Context> &context);

    /**
     * @brief Clear the injected context
     */
    static void ClearContext();

    /**
     * @brief Get the injected context reference
     * @return The ANI reference to the context object
     */
    static ani_ref GetContext();

    /**
     * @brief Inject the context as a global variable in the runtime
     * @param aniEnv The ANI environment
     * @param contextName The name for the global context variable
     * @return true if successful
     */
    static bool InjectContextAsGlobal(ani_env *aniEnv, const std::string &contextName = "context");
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ABC_CONTEXT_INJECTOR_H
