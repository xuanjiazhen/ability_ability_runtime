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
#include <functional>
#include <memory>

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
constexpr const char *LAUNCH_SERVICE_MANAGER_NAMESPACE = "@ohos.launchServiceManager.launchServiceManager";
constexpr const char *LAUNCH_SERVICE_INFO_CLASS_NAME = "@ohos.launchServiceManager.LaunchServiceInformationImpl";
constexpr const char *DAEMON_CALL_SIGNATURE =
    "C{std.core.String}C{std.core.String}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr int32_t EXTERNAL_PERMISSION_DENIED = 201;
constexpr int32_t EXTERNAL_INVALID_PARAMETER = 401;
constexpr int32_t EXTERNAL_CAPABILITY_NOT_SUPPORT = 801;
constexpr int32_t EXTERNAL_INNER_ERROR = 16000050;

int32_t ToExternalError(int32_t result)
{
    switch (result) {
        case ERR_OK:
            return ERR_OK;
        case AAFwk::CHECK_PERMISSION_FAILED:
            return EXTERNAL_PERMISSION_DENIED;
        case ERR_INVALID_VALUE:
            return EXTERNAL_INVALID_PARAMETER;
        case AAFwk::ERR_DAEMON_DEVICE_NOT_SUPPORTED:
            return EXTERNAL_CAPABILITY_NOT_SUPPORT;
        case AAFwk::ERR_DAEMON_NOT_REGISTERED:
            return AAFwk::ERR_DAEMON_EXT_NOT_FOUND;
        case AAFwk::ERR_DAEMON_ALREADY_REGISTERED:
            return AAFwk::ERR_DAEMON_EXT_ALREADY_REGISTERED;
        case AAFwk::ERR_DAEMON_DISABLED:
            return AAFwk::ERR_DAEMON_EXT_DISABLED;
        case AAFwk::ERR_DAEMON_NOT_RUNNING:
            return AAFwk::ERR_DAEMON_EXT_NOT_RUNNING;
        case AAFwk::ERR_DAEMON_APPROVAL_REJECTED:
            return AAFwk::ERR_DAEMON_EXT_APPROVAL_REJECTED;
        default:
            return EXTERNAL_INNER_ERROR;
    }
}

std::string GetDaemonErrorMessage(int32_t result)
{
    switch (result) {
        case EXTERNAL_PERMISSION_DENIED:
            return "Permission denied.";
        case EXTERNAL_INVALID_PARAMETER:
            return "Parameter error.";
        case EXTERNAL_CAPABILITY_NOT_SUPPORT:
            return "Device does not support launch service.";
        case AAFwk::ERR_DAEMON_EXT_NOT_FOUND:
            return "Launch service is not found.";
        case AAFwk::ERR_DAEMON_EXT_ALREADY_REGISTERED:
            return "Launch service is already registered.";
        case AAFwk::ERR_DAEMON_EXT_DISABLED:
            return "Launch service is disabled.";
        case AAFwk::ERR_DAEMON_EXT_NOT_RUNNING:
            return "Launch service is not running.";
        case AAFwk::ERR_DAEMON_EXT_APPROVAL_REJECTED:
            return "Launch service registration was rejected by the administrator.";
        default:
            return "System service connection failed.";
    }
}

using DaemonClientCall = std::function<int32_t(const std::shared_ptr<AAFwk::AbilityManagerClient> &,
    const std::string &, const std::string &)>;

void InvokeDaemonCall(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName,
    ani_object callback, const DaemonClientCall &call)
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
        result = call(client, bundleName, daemonName);
    }
    int32_t externalCode = ToExternalError(result);
    if (externalCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, externalCode, GetDaemonErrorMessage(externalCode)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

void RegisterDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->RegisterDaemon(bundleName, daemonName); });
}

void UnregisterDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->UnregisterDaemon(bundleName, daemonName); });
}

void StartDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StartDaemon(bundleName, daemonName); });
}

void StopDaemon(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
{
    InvokeDaemonCall(env, aniBundleName, aniDaemonName, callback,
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StopDaemon(bundleName, daemonName); });
}

ani_object WrapDaemonInformation(ani_env *env, const AAFwk::DaemonInformation &information)
{
    ani_object object = nullptr;
    if (!AppExecFwk::CreateObjectByClassName(env, LAUNCH_SERVICE_INFO_CLASS_NAME, object)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create launch service information failed");
        return nullptr;
    }
    AppExecFwk::SetRefProperty(env, object, "bundleName", AppExecFwk::GetAniString(env, information.bundleName));
    AppExecFwk::SetRefProperty(env, object, "daemonName", AppExecFwk::GetAniString(env, information.daemonName));
    AppExecFwk::SetIntPropertyValue(env, object, "state", static_cast<int32_t>(information.status));
    AppExecFwk::SetIntPropertyValue(env, object, "pid", information.pid);
    return object;
}

void GetInformation(ani_env *env, ani_string aniBundleName, ani_string aniDaemonName, ani_object callback)
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
    AAFwk::DaemonInformation information;
    information.bundleName = bundleName;
    information.daemonName = daemonName;
    auto client = AAFwk::AbilityManagerClient::GetInstance();
    if (client != nullptr) {
        result = client->GetDaemonInformation(bundleName, daemonName, information);
    }
    int32_t externalCode = ToExternalError(result);
    if (externalCode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, externalCode, GetDaemonErrorMessage(externalCode)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, WrapDaemonInformation(env, information));
}
} // namespace

void EtsLaunchServiceManagerInit(ani_env *env)
{
    ani_namespace ns = nullptr;
    if (env == nullptr || env->FindNamespace(LAUNCH_SERVICE_MANAGER_NAMESPACE, &ns) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "find launch service manager namespace failed");
        return;
    }
    std::array methods = {
        ani_native_function { "nativeRegisterDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(RegisterDaemon) },
        ani_native_function { "nativeUnregisterDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(UnregisterDaemon) },
        ani_native_function { "nativeStartDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(StartDaemon) },
        ani_native_function { "nativeStopDaemon", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(StopDaemon) },
        ani_native_function { "nativeGetInformation", DAEMON_CALL_SIGNATURE,
            reinterpret_cast<void *>(GetInformation) },
    };
    if (env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bind launch service manager methods failed");
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
    EtsLaunchServiceManagerInit(env);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
