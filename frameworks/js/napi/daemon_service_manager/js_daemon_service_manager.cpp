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

#include "js_daemon_service_manager.h"

#include <functional>
#include <memory>

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "daemon_service_errors.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_TWO = 2;
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
    if (result == AAFwk::ERR_DAEMON_ALREADY_REGISTERED || result == AAFwk::ERR_DAEMON_APPROVAL_REJECTED ||
        result == AAFwk::ERR_DAEMON_NOT_REGISTERED || result == AAFwk::ERR_DAEMON_DISABLED ||
        result == AAFwk::ERR_DAEMON_NOT_RUNNING || result == AAFwk::ERR_DAEMON_DEVICE_NOT_SUPPORTED ||
        result == AAFwk::ERR_DAEMON_IPC_DEAD || result == AAFwk::ERR_DAEMON_STORAGE_FAILED) {
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
        case AAFwk::ERR_DAEMON_ALREADY_REGISTERED:
            return "Daemon is already registered.";
        case AAFwk::ERR_DAEMON_APPROVAL_REJECTED:
            return "Daemon registration was rejected by the administrator.";
        case AAFwk::ERR_DAEMON_NOT_REGISTERED:
            return "Daemon is not registered.";
        case AAFwk::ERR_DAEMON_DISABLED:
            return "Daemon is disabled.";
        case AAFwk::ERR_DAEMON_NOT_RUNNING:
            return "Daemon is not running.";
        case AAFwk::ERR_DAEMON_DEVICE_NOT_SUPPORTED:
            return "Device does not support daemon service.";
        case AAFwk::ERR_DAEMON_STORAGE_FAILED:
            return "Failed to persist daemon state.";
        case AAFwk::ERR_DAEMON_IPC_DEAD:
            return "Daemon service is unavailable.";
        default:
            return "Internal error.";
    }
}
        *resultCode = client->SetDaemonEnabled(bundleName, daemonName, enabled);
    };
    NapiAsyncTask::CompleteCallback complete = [resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        int32_t externalCode = ToExternalError(*resultCode);
        if (externalCode != ERR_OK) {
            task.Reject(env, CreateJsError(env, externalCode, GetDaemonErrorMessage(externalCode)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsDaemonServiceManager::SetDaemonEnabled", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

using DaemonClientCall = std::function<int32_t(const std::shared_ptr<AAFwk::AbilityManagerClient> &,
    const std::string &, const std::string &)>;

napi_value InvokeDaemonCall(napi_env env, napi_callback_info info, const std::string &name,
    const DaemonClientCall &call)
{
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc != ARGC_TWO) {
        ThrowError(env, EXTERNAL_INVALID_PARAMETER, "Exactly two parameters are required.");
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    std::string daemonName;
    if (!ConvertFromJsValue(env, argv[0], bundleName) || bundleName.empty() ||
        !ConvertFromJsValue(env, argv[1], daemonName) || daemonName.empty()) {
        ThrowError(env, EXTERNAL_INVALID_PARAMETER, "bundleName and daemonName must be non-empty strings.");
        return CreateJsUndefined(env);
    }

    auto resultCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [bundleName, daemonName, call, resultCode]() {
        auto client = AAFwk::AbilityManagerClient::GetInstance();
        if (client == nullptr) {
            *resultCode = EXTERNAL_INNER_ERROR;
            return;
        }
        *resultCode = call(client, bundleName, daemonName);
    };
    NapiAsyncTask::CompleteCallback complete = [resultCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        int32_t externalCode = ToExternalError(*resultCode);
        if (externalCode != ERR_OK) {
            task.Reject(env, CreateJsError(env, externalCode, GetDaemonErrorMessage(externalCode)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos(name, env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value RegisterDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsDaemonServiceManager::RegisterDaemon",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->RegisterDaemon(bundleName, daemonName); });
}

napi_value UnregisterDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsDaemonServiceManager::UnregisterDaemon",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->UnregisterDaemon(bundleName, daemonName); });
}
} // namespace

napi_value JsDaemonServiceManagerInit(napi_env env, napi_value exports)
{
    BindNativeFunction(env, exports, "setDaemonEnabled", "JsDaemonServiceManager", SetDaemonEnabled);
    BindNativeFunction(env, exports, "register", "JsDaemonServiceManager", RegisterDaemon);
    BindNativeFunction(env, exports, "unregister", "JsDaemonServiceManager", UnregisterDaemon);
    BindNativeFunction(env, exports, "start", "JsDaemonServiceManager", StartDaemon);
    BindNativeFunction(env, exports, "stop", "JsDaemonServiceManager", StopDaemon);
    return exports;
}
} // namespace AbilityRuntime
} // namespace OHOS
