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

#include "ets_daemon_service_manager.h"

#include <array>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ani_common_util.h"
#include "daemon_service_errors.h"
#include "errors.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *DAEMON_MANAGER_NAMESPACE = "@ohos.daemonServiceManager.daemonServiceManager";
constexpr const char *SET_ENABLED_SIGNATURE =
    "C{std.core.String}C{std.core.String}zC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr int32_t EXTERNAL_PERMISSION_DENIED = 201;
constexpr int32_t EXTERNAL_INVALID_PARAMETER = 401;
constexpr int32_t EXTERNAL_INNER_ERROR = 16000050;

int32_t ToExternalError(int32_t result)
{
    if (result == AAFwk::CHECK_PERMISSION_FAILED) {
        return EXTERNAL_PERMISSION_DENIED;
    }
    if (result == ERR_INVALID_VALUE) {
        return EXTERNAL_INVALID_PARAMETER;
    }
    if (result == AAFwk::ERR_DAEMON_NOT_REGISTERED || result == AAFwk::ERR_DAEMON_STORAGE_FAILED ||
        result == AAFwk::ERR_DAEMON_IPC_DEAD) {
        return result;
    }
    return result == ERR_OK ? ERR_OK : EXTERNAL_INNER_ERROR;
}

std::string GetDaemonErrorMessage(int32_t result)
{
    switch (result) {
        case EXTERNAL_PERMISSION_DENIED:
            return "Permission denied.";
        case EXTERNAL_INVALID_PARAMETER:
            return "Parameter error.";
        case AAFwk::ERR_DAEMON_NOT_REGISTERED:
            return "Daemon is not registered.";
        case AAFwk::ERR_DAEMON_STORAGE_FAILED:
            return "Failed to persist daemon state.";
        case AAFwk::ERR_DAEMON_IPC_DEAD:
            return "Daemon service is unavailable.";
        default:
            return "Internal error.";
    }
}

void SetDaemonEnabled(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName,
    ani_boolean enabled, ani_object callback)
{
    if (env == nullptr) {
        return;
    }
    std::string bundleName;
    std::string daemonName;
    if (!AppExecFwk::GetStdString(env, aniBundleName, bundleName) || bundleName.empty() ||
        !AppExecFwk::GetStdString(env, aniDaemonName, daemonName) || daemonName.empty()) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, EXTERNAL_INVALID_PARAMETER, "Names must be non-empty strings."), nullptr);
        return;
    }

    int32_t result = EXTERNAL_INNER_ERROR;
    auto client = AAFwk::AbilityManagerClient::GetInstance();
    if (client != nullptr) {
        result = client->SetDaemonEnabled(bundleName, daemonName, enabled != ANI_FALSE);
    }
    int32_t externalCode = ToExternalError(result);
    if (externalCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, externalCode, GetDaemonErrorMessage(externalCode)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}
} // namespace

void EtsDaemonServiceManagerInit(ani_env *env)
{
    ani_namespace ns = nullptr;
    if (env == nullptr || env->FindNamespace(DAEMON_MANAGER_NAMESPACE, &ns) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "find daemon service manager namespace failed");
        return;
    }
    std::array methods = {
        ani_native_function { "nativeSetDaemonEnabled", SET_ENABLED_SIGNATURE,
            reinterpret_cast<void *>(SetDaemonEnabled) },
    };
    if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bind daemon service manager methods failed");
    }
}

extern "C" ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr || result == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK || env == nullptr) {
        return ANI_NOT_FOUND;
    }
    EtsDaemonServiceManagerInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
