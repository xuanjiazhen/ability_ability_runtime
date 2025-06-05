/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ets_startup_task.h"

#include "hilog_tag_wrapper.h"
#include "ets_runtime_utils.h"
#include "ets_startup_task_executor.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {

class AsyncTaskCallBack {
public:
    AsyncTaskCallBack() = default;
    ~AsyncTaskCallBack() = default;

    static napi_value AsyncTaskCompleted(napi_env env, napi_callback_info info)
    {
        TAG_LOGD(AAFwkTag::STARTUP, "called");
        size_t argc = ARGC_TWO;
        napi_value argv[ARGC_TWO] = { nullptr };
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

        std::string startupName;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], startupName)) {
            TAG_LOGE(AAFwkTag::STARTUP, "Convert error");
            return CreateJsUndefined(env);
        }

        napi_value resultJs = argv[INDEX_ONE];
        napi_ref resultRef = nullptr;
        napi_create_reference(env, resultJs, INDEX_ONE, &resultRef);
        std::shared_ptr<NativeReference> result(reinterpret_cast<NativeReference*>(resultRef));
        std::shared_ptr<StartupTaskResult> callbackResult = std::make_shared<JsStartupTaskResult>(result);

        std::shared_ptr<JsStartupTask> jsStartupTask;
        for (const auto& iter : jsStartupTaskObjects_) {
            if (iter.first == startupName) {
                jsStartupTask = iter.second.lock();
            }
        }

        if (jsStartupTask != nullptr) {
            jsStartupTask->OnAsyncTaskCompleted(callbackResult);
            jsStartupTaskObjects_.erase(startupName);
        }

        return CreateJsUndefined(env);
    }

    static napi_value Constructor(napi_env env, napi_callback_info cbinfo)
    {
        TAG_LOGD(AAFwkTag::STARTUP, "called");
        return CreateJsUndefined(env);
    }

    static std::map<std::string, std::weak_ptr<EtsStartupTask>> etsStartupTaskObjects_;
};
std::map<std::string, std::weak_ptr<EtsStartupTask>> AsyncTaskCallBack::etsStartupTaskObjects_;
}

const std::string EtsStartupTask::TASK_TYPE = "Ets";

EtsStartupTask::EtsStartupTask(const std::string& name, STSRuntime& etsRuntime,
    std::unique_ptr<NativeReference>& startupJsRef, std::shared_ptr<NativeReference>& contextJsRef)
    : AppStartupTask(name), etsRuntime_(etsRuntime), startupEtsRef_(std::move(startupEtsRef)), contextEtsRef_(contextEtsRef)
{
}

EtsStartupTask::~EtsStartupTask() = default;

const std::string &EtsStartupTask::GetType() const
{
    return TASK_TYPE;
}

int32_t EtsStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    TAG_LOGI(AAFwkTag::STARTUP, "task: %{public}s init", GetName().c_str());
    if (callCreateOnMainThread_) {
        return EtsStartupTaskExecutor::RunOnMainThread(etsRuntime_, startupEtsRef_, contextEtsRef_, std::move(callback));
    }

    if (LoadEtsAsyncTaskExecutor() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "LoadEtsAsyncTaskExecutor failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    LoadEtsAsyncTaskCallback();

    startupTaskResultCallback_ = std::move(callback);

    auto result = EtsStartupTaskExecutor::RunOnTaskPool(
        etsRuntime_, startupEtsRef_, contextEtsRef_, AsyncTaskExecutorEtsRef_, AsyncTaskExecutorCallbackEtsRef_, name_);
    if (result == ERR_OK) {
        AsyncTaskCallBack::etsStartupTaskObjects_.emplace(name_,
            std::static_pointer_cast<EtsStartupTask>(shared_from_this()));
    }
    return result;
}

int32_t EtsStartupTask::LoadEtsAsyncTaskExecutor()
{
    TAG_LOGD(AAFwkTag::STARTUP, "LoadEtsAsyncTaskExecutor called");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    //New AsyncCallbackWrapper TODO dxp
    // new 
    // napi_value object = nullptr;
    // napi_create_object(env, &object);
    // if (object == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null object");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }

    // AsyncTaskExecutorJsRef_ =
    //     JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskExcutor", &object, 1);
    return ERR_OK;
}

void EtsStartupTask::LoadEtsAsyncTaskCallback()
{
    TAG_LOGD(AAFwkTag::STARTUP, "LoadEtsAsyncTaskCallback called");
    //TODO dxp
    // auto env = jsRuntime_.GetNapiEnv();

    // napi_value config;
    // std::string value = "This is callback value";
    // NAPI_CALL_RETURN_VOID(
    //     env, napi_create_string_utf8(env, value.c_str(), value.length(), &config));

    // napi_property_descriptor props[] = {
    //     DECLARE_NAPI_STATIC_FUNCTION("onAsyncTaskCompleted", AsyncTaskCallBack::AsyncTaskCompleted),
    //     DECLARE_NAPI_INSTANCE_PROPERTY("config", config),
    // };
    // napi_value asyncTaskCallbackClass = nullptr;
    // napi_define_sendable_class(env, "AsyncTaskCallback", NAPI_AUTO_LENGTH, AsyncTaskCallBack::Constructor,
    //     nullptr, sizeof(props) / sizeof(props[0]), props, nullptr, &asyncTaskCallbackClass);
    // AsyncTaskExecutorCallbackJsRef_ =
    //     JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskCallback", &asyncTaskCallbackClass, 1);
}

void EtsStartupTask::OnAsyncTaskCompleted(const std::shared_ptr<StartupTaskResult>  &result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnAsyncTaskCompleted called");
    if (startupTaskResultCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null startupTaskResultCallback");
        return;
    }
    startupTaskResultCallback_->Call(result);
}

int32_t EtsStartupTask::RunTaskOnDependencyCompleted(const std::string &dependencyName,
    const std::shared_ptr<StartupTaskResult> &result)
{
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if (startupEtsRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null ref_:%{public}s", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    // napi_value startupValue = startupJsRef_->GetNapiValue();
    // if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, not napi object", name_.c_str());
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    // napi_value startupOnDepCompleted = nullptr;
    // napi_get_named_property(env, startupValue, "onDependencyCompleted", &startupOnDepCompleted);
    // if (startupOnDepCompleted == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, get onDependencyCompleted failed", name_.c_str());
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    // bool isCallable = false;
    // napi_is_callable(env, startupOnDepCompleted, &isCallable);
    // if (!isCallable) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "onDependencyCompleted not callable:%{public}s", name_.c_str());
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }

    // napi_value jsResult = GetDependencyResult(env, dependencyName, result);
    // napi_value dependency = CreateJsValue(env, dependencyName);
    // constexpr size_t argc = 2;
    // napi_value argv[argc] = { dependency, jsResult };
    // napi_call_function(env, startupValue, startupOnDepCompleted, argc, argv, nullptr);
    return ERR_OK;
}

ani_ref GetDependencyResult(ani_env *env, const std::string &dependencyName,
        const std::shared_ptr<StartupTaskResult> &result)
{
    if (env == nullptr ||
        result == nullptr ||
        result->GetResultType() != StartupTaskResult::ResultType::ETS) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid args");
        return CreateEtsUndefined(env);
    } else {
        std::shared_ptr<EtsStartupTaskResult> etsResultPtr = std::static_pointer_cast<EtsStartupTaskResult>(result);
        if (etsResultPtr == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, convert failed", dependencyName.c_str());
            return CreateEtsUndefined(env);
        }
        std::shared_ptr<STSNativeReference> etsResultRef = etsResultPtr->GetEtsStartupResultRef();
        if (etsResultRef == nullptr) {
            return CreateEtsUndefined(env);
        }
        return etsResultRef->aniRef;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
