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

#include "js_modular_object_manager.h"

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_modular_object_manager_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "modular_object_extension_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr size_t ARGC_FOUR = 4;
constexpr size_t ARGC_FIVE = 5;
}  // namespace
class JsModularObjectManager {
public:
    JsModularObjectManager() = default;
    ~JsModularObjectManager() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        std::unique_ptr<JsModularObjectManager>(static_cast<JsModularObjectManager*>(data));
    }

    static napi_value QueryAllModularObjectExtensionInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsModularObjectManager, OnQueryAllModularObjectExtensionInfo);
    }

    static napi_value SetModularObjectDisabledState(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsModularObjectManager, OnSetModularObjectDisabledState);
    }

    static napi_value SetModularObjectGlobalDisabledState(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsModularObjectManager, OnSetModularObjectGlobalDisabledState);
    }

    static napi_value GetModularObjectGlobalDisabledState(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsModularObjectManager, OnGetModularObjectGlobalDisabledState);
    }

private:
    napi_value OnQueryAllModularObjectExtensionInfo(napi_env env, NapiCallbackInfo &info);
    napi_value OnSetModularObjectDisabledState(napi_env env, NapiCallbackInfo &info);
    bool GetJsModularObjectInfoArgs(napi_env env, napi_value *argv, AAFwk::ModularObjectExtensionInfo &infoArgs);
    napi_value OnSetModularObjectGlobalDisabledState(napi_env env, NapiCallbackInfo &info);
    napi_value OnGetModularObjectGlobalDisabledState(napi_env env, NapiCallbackInfo &info);
};

napi_value JsModularObjectManager::OnQueryAllModularObjectExtensionInfo(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called OnQueryAllModularObjectExtension");

    HandleEscape handleEscape(env);
    NapiAsyncTask::CompleteCallback complete =
        [](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::JSNAPI, "null amsClient");
                task.Reject(env, CreateJsErrorByNativeErr(env, static_cast<int32_t>(AAFwk::INNER_ERR)));
                return;
            }
            std::vector<AAFwk::ModularObjectExtensionInfo> infos;
            auto innerErrorCode = amsClient->QueryAllModularObjectExtensionInfo(infos);
            if (innerErrorCode == 0) {
                task.Resolve(env, WrapModularObjectExtensionInfoArray(env, infos));
            } else {
                TAG_LOGE(AAFwkTag::JSNAPI, "error: %{public}d", innerErrorCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsModularObjectManager::OnQueryAllModularObjectExtensionInfo",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsModularObjectManager::OnSetModularObjectDisabledState(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called OnSetModularObjectDisabledState");

    HandleEscape handleEscape(env);
    if (info.argc < ARGC_FIVE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::ModularObjectExtensionInfo extensionInfo;
    if (!GetJsModularObjectInfoArgs(env, info.argv, extensionInfo)) {
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [extensionInfo, innerErrorCode, env]() {
        auto amsClient = AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null amsClient");
            *innerErrorCode = static_cast<int32_t>(AAFwk::INNER_ERR);
            return;
        }
        *innerErrorCode = amsClient->SetModularObjectDisabledState(extensionInfo.bundleName, extensionInfo.moduleName,
            extensionInfo.abilityName, extensionInfo.appIndex, extensionInfo.isSystemDisabled);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsModularObjectManager::OnSetModularObjectDisabledState",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

bool JsModularObjectManager::GetJsModularObjectInfoArgs(napi_env env, napi_value *argv,
    AAFwk::ModularObjectExtensionInfo &info)
{
    if (!ConvertFromJsValue(env, argv[ARGC_ZERO], info.bundleName) || info.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse bundleName failed, not string");
        ThrowInvalidParamError(env, "Parse param bundleName failed, must be a string.");
        return false;
    }

    if (!ConvertFromJsValue(env, argv[ARGC_ONE], info.moduleName) || info.moduleName.empty()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse moduleName failed, not string");
        ThrowInvalidParamError(env, "Parse param moduleName failed, must be a string.");
        return false;
    }

    if (!ConvertFromJsValue(env, argv[ARGC_TWO], info.abilityName) || info.abilityName.empty()) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse abilityName failed, not string");
        ThrowInvalidParamError(env, "Parse param abilityName failed, must be a string.");
        return false;
    }

    if (!ConvertFromJsValue(env, argv[ARGC_THREE], info.appIndex)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse appIndex failed, not int32");
        ThrowInvalidParamError(env, "Parse param appIndex failed, must be an int32.");
        return false;
    }

    if (!ConvertFromJsValue(env, argv[ARGC_FOUR], info.isSystemDisabled)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse isSystemDisabled failed, not boolean");
        ThrowInvalidParamError(env, "Parse param isSystemDisabled failed, must be a boolean.");
        return false;
    }

    return true;
}

napi_value JsModularObjectManager::OnSetModularObjectGlobalDisabledState(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called OnSetModularObjectGlobalDisabledState");

    HandleEscape handleEscape(env);
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::JSNAPI, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    bool isDisabled = false;
    if (!ConvertFromJsValue(env, info.argv[ARGC_ZERO], isDisabled)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "parse isDisabled failed, not boolean");
        ThrowInvalidParamError(env, "Parse param isDisabled failed, must be a boolean.");
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [isDisabled, innerErrorCode, env]() {
        auto amsClient = AbilityManagerClient::GetInstance();
        if (amsClient == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
            *innerErrorCode = static_cast<int32_t>(AAFwk::INNER_ERR);
            return;
        }
        *innerErrorCode = amsClient->SetModularObjectGlobalDisabledState(isDisabled);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrorCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "error: %{public}d", *innerErrorCode);
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsModularObjectManager::OnSetModularObjectGlobalDisabledState",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsModularObjectManager::OnGetModularObjectGlobalDisabledState(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called OnGetModularObjectGlobalDisabledState");

    HandleEscape handleEscape(env);
    NapiAsyncTask::CompleteCallback complete =
        [](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            bool isDisabled = false;
            auto amsClient = AbilityManagerClient::GetInstance();
            if (amsClient == nullptr) {
                TAG_LOGE(AAFwkTag::JSNAPI, "null amsClient");
                task.Reject(env, CreateJsErrorByNativeErr(env, static_cast<int32_t>(AAFwk::INNER_ERR)));
                return;
            }
            auto innerErrorCode = amsClient->GetModularObjectGlobalDisabledState(isDisabled);
            if (innerErrorCode == 0) {
                task.Resolve(env, CreateJsValue(env, isDisabled));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, innerErrorCode));
            }
        };

    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsModularObjectManager::OnGetModularObjectGlobalDisabledState",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value InitModularObjectManagerApiModule(napi_env env, napi_value exportObj)
{
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::RECOVERY, "null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JsModularObjectManager> appModularObjectManagerApi =
        std::make_unique<JsModularObjectManager>();
    napi_wrap(env, exportObj, appModularObjectManagerApi.release(),
        JsModularObjectManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "ModularObjectManager";
    BindNativeFunction(env, exportObj, "queryAllModularObjectExtensionInfo", moduleName,
        JsModularObjectManager::QueryAllModularObjectExtensionInfo);
    BindNativeFunction(env, exportObj, "setModularObjectDisabledState", moduleName,
        JsModularObjectManager::SetModularObjectDisabledState);
    BindNativeFunction(env, exportObj, "setModularObjectGlobalDisabledState", moduleName,
        JsModularObjectManager::SetModularObjectGlobalDisabledState);
    BindNativeFunction(env, exportObj, "getModularObjectGlobalDisabledState", moduleName,
        JsModularObjectManager::GetModularObjectGlobalDisabledState);

    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS