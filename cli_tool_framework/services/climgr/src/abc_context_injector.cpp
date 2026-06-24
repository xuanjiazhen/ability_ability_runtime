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

/**
 * @file abc_context_injector.cpp
 * @brief Context injector for abc files
 *
 * This module provides functionality to inject a Context into the ArkTS runtime
 * so that abc files can use it to call system APIs like startAbility.
 */

#include "abc_context_injector.h"
#include "ets_environment.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
// Global pointer to the injected context
ani_ref g_injectedContextRef = nullptr;
ani_env *g_aniEnv = nullptr;

constexpr const char *CONTEXT_CLASS_NAME = "ohos.hiview.AbilityContext";
constexpr const char *CONTEXT_MODULE_NAME = "@ohos.app.ability.UIAbilityContext";
} // namespace

void AbcContextInjector::SetContext(ani_env *aniEnv, const std::shared_ptr<AbilityRuntime::Context> &context)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Setting context for abc files");

    if (aniEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid parameters: aniEnv or context is null");
        return;
    }

    // Clean up old reference if exists
    if (g_injectedContextRef != nullptr && g_aniEnv != nullptr) {
        g_aniEnv->ReleaseGlobalRef(g_injectedContextRef);
        g_injectedContextRef = nullptr;
    }

    g_aniEnv = aniEnv;

    // Note: In a real implementation, you would need to:
    // 1. Create a JavaScript wrapper object for the native Context
    // 2. Store the global reference
    // 3. Make it available to abc files via a global property or module import

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Context set successfully");
}

void AbcContextInjector::ClearContext()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Clearing context");

    if (g_injectedContextRef != nullptr && g_aniEnv != nullptr) {
        g_aniEnv->ReleaseGlobalRef(g_injectedContextRef);
        g_injectedContextRef = nullptr;
    }

    g_aniEnv = nullptr;
}

ani_ref AbcContextInjector::GetContext()
{
    return g_injectedContextRef;
}

bool AbcContextInjector::InjectContextAsGlobal(ani_env *aniEnv, const std::string &contextName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Injecting context as global: %{public}s", contextName.c_str());

    if (aniEnv == nullptr || g_injectedContextRef == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Cannot inject context: aniEnv or contextRef is null");
        return false;
    }

    ani_status status = ANI_ERROR;

    // Get the global object
    ani_ref globalRef = nullptr;
    status = aniEnv->GetGlobal(&globalRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetGlobal failed: %{public}d", status);
        return false;
    }

    // Set the context as a global property
    status = aniEnv->SetNamedProperty(globalRef, contextName.c_str(), g_injectedContextRef);
    aniEnv->ReleaseLocalRef(globalRef);

    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SetNamedProperty failed: %{public}d", status);
        return false;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Context injected as global '%{public}s' successfully", contextName.c_str());
    return true;
}

} // namespace CliTool
} // namespace OHOS
