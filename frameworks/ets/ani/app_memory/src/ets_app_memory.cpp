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

#include "ets_app_memory.h"

#include <array>
#include <string>
#include <vector>

#include "ani_common_util.h"
#include "ets_error_utils.h"
#include "file_page_releaser.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AppMemoryEts {
namespace {
using Security::AccessToken::TokenIdKit;
// Mirror of appManager namespace scheme. Requires the matching TS namespace
// declaration (@ohos.app.ability.appMemory) in the interface SDK — see O-4.
constexpr const char* APP_MEMORY_SPACE_NAME = "@ohos.app.ability.appMemory.appMemory";
constexpr const char* RELEASE_FILE_PAGES_SIG = "C{utils.AbilityUtils.AsyncCallbackWrapper}C{std.core.Array}:";
constexpr const char* RELEASE_RAW_FILE_PAGES_SIG = "C{utils.AbilityUtils.AsyncCallbackWrapper}C{std.core.String}:";

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMemory: not system app");
        return false;
    }
    return true;
}

void NativeReleaseFilePages(ani_env *env, ani_object callback, ani_object aniFileNames)
{
    if (env == nullptr) {
        return;
    }
    if (!CheckCallerIsSystemApp()) {
        AbilityRuntime::EtsErrorUtil::ThrowNotSystemAppError(env);
        return;
    }
    std::vector<std::string> fileNames;
    if (!AppExecFwk::UnwrapArrayString(env, aniFileNames, fileNames) || fileNames.empty()) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(env, "fileNames must be a non-empty string array."),
            nullptr);
        return;
    }
    int32_t errCode = AbilityRuntime::FilePageReleaser::ReleaseByFileNames(fileNames);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
}

void NativeReleaseRawFilePages(ani_env *env, ani_object callback, ani_string aniModuleName)
{
    if (env == nullptr) {
        return;
    }
    if (!CheckCallerIsSystemApp()) {
        AbilityRuntime::EtsErrorUtil::ThrowNotSystemAppError(env);
        return;
    }
    std::string moduleName;
    if (!AppExecFwk::GetStdString(env, aniModuleName, moduleName) || moduleName.empty()) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(env, "moduleName must be a non-empty string."),
            nullptr);
        return;
    }
    int32_t errCode = AbilityRuntime::FilePageReleaser::ReleaseRawFileByModule(moduleName);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, errCode), nullptr);
}
} // namespace

void EtsAppMemoryRegistryInit(ani_env *env)
{
    if (env == nullptr) {
        return;
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "ResetError failed");
    }
    ani_namespace ns;
    ani_status status = env->FindNamespace(APP_MEMORY_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        // Expected until the interface SDK declares the @ohos.app.ability.appMemory namespace (O-4).
        TAG_LOGE(AAFwkTag::APPMGR, "FindNamespace appMemory failed status: %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function {"nativeReleaseFilePages", RELEASE_FILE_PAGES_SIG,
            reinterpret_cast<void *>(NativeReleaseFilePages)},
        ani_native_function {"nativeReleaseRawFilePages", RELEASE_RAW_FILE_PAGES_SIG,
            reinterpret_cast<void *>(NativeReleaseRawFilePages)},
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        return ANI_NOT_FOUND;
    }
    EtsAppMemoryRegistryInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // extern "C"
} // namespace AppMemoryEts
} // namespace OHOS
