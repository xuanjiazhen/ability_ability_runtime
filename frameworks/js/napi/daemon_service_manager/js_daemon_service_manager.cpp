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
constexpr size_t ARGC_ONE = 1;
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

bool GetDescriptor(napi_env env, napi_value value, std::string &bundleName, std::string &daemonName)
{
    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_object) {
        return false;
    }
    napi_value bundleValue = nullptr;
    napi_value daemonValue = nullptr;
    if (napi_get_named_property(env, value, "bundleName", &bundleValue) != napi_ok ||
        napi_get_named_property(env, value, "daemonName", &daemonValue) != napi_ok) {
        return false;
    }
    return ConvertFromJsValue(env, bundleValue, bundleName) && !bundleName.empty() &&
        ConvertFromJsValue(env, daemonValue, daemonName) && !daemonName.empty();
}

using DaemonClientCall = std::function<int32_t(const std::shared_ptr<AAFwk::AbilityManagerClient> &,
    const std::string &, const std::string &)>;

napi_value InvokeDaemonCall(napi_env env, napi_callback_info info, const std::string &name,
    const DaemonClientCall &call)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc != ARGC_ONE) {
        ThrowError(env, EXTERNAL_INVALID_PARAMETER, "A launch service descriptor is required.");
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    std::string daemonName;
    if (!GetDescriptor(env, argv[0], bundleName, daemonName)) {
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
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Register",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->RegisterDaemon(bundleName, daemonName); });
}

napi_value UnregisterDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Unregister",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->UnregisterDaemon(bundleName, daemonName); });
}

napi_value StartDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Start",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StartDaemon(bundleName, daemonName); });
}

napi_value StopDaemon(napi_env env, napi_callback_info info)
{
    return InvokeDaemonCall(env, info, "JsLaunchServiceManager::Stop",
        [](const std::shared_ptr<AAFwk::AbilityManagerClient> &client, const std::string &bundleName,
            const std::string &daemonName) { return client->StopDaemon(bundleName, daemonName); });
}

napi_value GetDaemonInformation(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    if (napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr) != napi_ok || argc != ARGC_ONE) {
        ThrowError(env, EXTERNAL_INVALID_PARAMETER, "A launch service descriptor is required.");
        return CreateJsUndefined(env);
    }

    std::string bundleName;
    std::string daemonName;
    if (!GetDescriptor(env, argv[0], bundleName, daemonName)) {
        ThrowError(env, EXTERNAL_INVALID_PARAMETER, "bundleName and daemonName must be non-empty strings.");
        return CreateJsUndefined(env);
    }

    auto resultCode = std::make_shared<int32_t>(ERR_OK);
    auto information = std::make_shared<AAFwk::DaemonInformation>();
    NapiAsyncTask::ExecuteCallback execute = [bundleName, daemonName, resultCode, information]() {
        auto client = AAFwk::AbilityManagerClient::GetInstance();
        if (client == nullptr) {
            *resultCode = EXTERNAL_INNER_ERROR;
            return;
        }
        *resultCode = client->GetDaemonInformation(bundleName, daemonName, *information);
    };
    NapiAsyncTask::CompleteCallback complete = [resultCode, information](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        int32_t externalCode = ToExternalError(*resultCode);
        if (externalCode != ERR_OK) {
            task.Reject(env, CreateJsError(env, externalCode, GetDaemonErrorMessage(externalCode)));
            return;
        }
        napi_value result = nullptr;
        napi_create_object(env, &result);
        napi_value bundleValue = nullptr;
        napi_value daemonValue = nullptr;
        napi_value stateValue = nullptr;
        napi_value pidValue = nullptr;
        napi_create_string_utf8(env, information->bundleName.c_str(), NAPI_AUTO_LENGTH, &bundleValue);
        napi_create_string_utf8(env, information->daemonName.c_str(), NAPI_AUTO_LENGTH, &daemonValue);
        napi_create_int32(env, static_cast<int32_t>(information->status), &stateValue);
        napi_create_int32(env, information->pid, &pidValue);
        napi_set_named_property(env, result, "bundleName", bundleValue);
        napi_set_named_property(env, result, "daemonName", daemonValue);
        napi_set_named_property(env, result, "state", stateValue);
        napi_set_named_property(env, result, "pid", pidValue);
        task.ResolveWithNoError(env, result);
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsLaunchServiceManager::GetInformation", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}
} // namespace

napi_value JsLaunchServiceManagerInit(napi_env env, napi_value exports)
{
    BindNativeFunction(env, exports, "register", "JsLaunchServiceManager", RegisterDaemon);
    BindNativeFunction(env, exports, "unregister", "JsLaunchServiceManager", UnregisterDaemon);
    BindNativeFunction(env, exports, "start", "JsLaunchServiceManager", StartDaemon);
    BindNativeFunction(env, exports, "stop", "JsLaunchServiceManager", StopDaemon);
    BindNativeFunction(env, exports, "getInformation", "JsLaunchServiceManager", GetDaemonInformation);
    return exports;
}
} // namespace AbilityRuntime
} // namespace OHOS
