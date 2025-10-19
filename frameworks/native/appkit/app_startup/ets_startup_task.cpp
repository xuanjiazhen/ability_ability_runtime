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

#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "stage_context_transfer.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_TWO = 2;

// class AsyncTaskCallBack {
// public:
//     AsyncTaskCallBack() = default;
//     ~AsyncTaskCallBack() = default;

//     static ani_value AsyncTaskCompleted(ani_env env, ani_object thisObj, ani_value* argv, int32_t argc)
//     {
//         TAG_LOGD(AAFwkTag::STARTUP, "called");
//         if (argc < ARGC_TWO) {
//             TAG_LOGE(AAFwkTag::STARTUP, "Invalid argument count");
//             return nullptr;
//         }

//         std::string startupName;
//         if (!ConvertFromEtsValue(env, argv[0], startupName)) {
//             TAG_LOGE(AAFwkTag::STARTUP, "Convert error");
//             return nullptr;
//         }

//         ani_value resultEts = argv[1];
//         ani_ref resultRef = nullptr;
//         ani_status status = env->GlobalReference_Create(resultEts, &resultRef);
//         if (status != ANI_OK) {
//             TAG_LOGE(AAFwkTag::STARTUP, "Create reference failed");
//             return nullptr;
//         }
        
//         std::shared_ptr<NativeReference> result = std::make_shared<EtsStartupTaskResult>(resultRef);
//         std::shared_ptr<StartupTaskResult> callbackResult = std::make_shared<EtsStartupTaskResult>(resultRef);

//         std::shared_ptr<EtsStartupTask> etsStartupTask;
//         for (const auto& iter : etsStartupTaskObjects_) {
//             if (iter.first == startupName) {
//                 etsStartupTask = iter.second.lock();
//             }
//         }

//         if (etsStartupTask != nullptr) {
//             etsStartupTask->OnAsyncTaskCompleted(callbackResult);
//             etsStartupTaskObjects_.erase(startupName);
//         }

//         return nullptr;
//     }

//     static std::map<std::string, std::weak_ptr<EtsStartupTask>> etsStartupTaskObjects_;
// };
// std::map<std::string, std::weak_ptr<EtsStartupTask>> AsyncTaskCallBack::etsStartupTaskObjects_;
}

const std::string EtsStartupTask::TASK_TYPE = "Ets";

EtsStartupTask::EtsStartupTask(ETSRuntime &etsRuntime, const StartupTaskInfo &info, bool lazyLoad)
    : AppStartupTask(info.name), etsRuntime_(etsRuntime), srcEntry_(info.srcEntry), ohmUrl_(info.ohmUrl),
    hapPath_(info.hapPath), esModule_(info.esModule)
{
    if (!lazyLoad) {
        LoadEtsOhmUrl();
    }
}

EtsStartupTask::~EtsStartupTask() = default;

const std::string &EtsStartupTask::GetType() const
{
    return AppStartupTask::TASK_TYPE_ETS;
}

int32_t EtsStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    TAG_LOGI(AAFwkTag::STARTUP, "task: %{public}s init", GetName().c_str());
    if (startupEtsRef_ == nullptr) {
        int32_t result = LoadEtsOhmUrl();
        if (result != ERR_OK) {
            return result;
        }
    }
    
    // if (callCreateOnMainThread_) {
    //     return EtsStartupTaskExecutor::RunOnMainThread(etsRuntime_, startupEtsRef_, contextRef_, std::move(callback));
    // }
    
    // AAFwk::EventInfo eventInfo;
    // if (LoadEtsAsyncTaskExecutor() != ERR_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "LoadEtsAsyncTaskExecutor failed");
    //     eventInfo.errCode = ANI_CREATE_OBJECT_FAILED;
    //     eventInfo.errReason = "LoadEtsAsyncTaskExecutor failed";
    //     AAFwk::EventReport::SendLaunchFrameworkEvent(
    //         AAFwk::EventName::STARTUP_TASK_ERROR, HiSysEventType::FAULT, eventInfo);
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    // LoadEtsAsyncTaskCallback();

    // startupTaskResultCallback_ = std::move(callback);

    // auto result = EtsStartupTaskExecutor::RunOnTaskPool(
    //     etsRuntime_, startupEtsRef_, contextRef_, asyncTaskExecutorEtsRef_, 
    //     asyncTaskExecutorCallbackEtsRef_, name_);
    // if (result == ERR_OK) {
    //     AsyncTaskCallBack::etsStartupTaskObjects_.emplace(name_,
    //         std::static_pointer_cast<EtsStartupTask>(shared_from_this()));
    // }
    // return result;
    return ERR_OK;
}

int32_t EtsStartupTask::LoadEtsOhmUrl()
{
    TAG_LOGD(AAFwkTag::STARTUP, "LoadEtsOhmUrl call");
    if (srcEntry_.empty() && ohmUrl_.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "srcEntry and ohmUrl empty");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    std::string moduleNameWithStartupTask = GetModuleName() + "::startupTask";
    std::string srcPath(GetModuleName() + "/" + srcEntry_);
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid srcEntry");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");
    
    startupEtsRef_ = etsRuntime_.LoadModule(moduleNameWithStartupTask, srcPath, hapPath_, esModule_, false, ohmUrl_);
    if (startupEtsRef_ == nullptr || startupEtsRef_->aniRef == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task null");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    return ERR_OK;
}

int32_t EtsStartupTask::LoadEtsAsyncTaskExecutor()
{
    // TAG_LOGD(AAFwkTag::STARTUP, "called");
    // auto env = etsRuntime_.GetAniEnv();
    // if (env == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null env");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }

    // ani_object object = nullptr;
    // ani_status status = env->Object_Create(&object);
    // if (object == nullptr || status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null object");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }

    // asyncTaskExecutorEtsRef_ = etsRuntime_.LoadSystemModule("app.appstartup.AsyncTaskExcutor", &object, 1);
    return ERR_OK;
}

void EtsStartupTask::LoadEtsAsyncTaskCallback()
{
    // TAG_LOGD(AAFwkTag::STARTUP, "called");
    // auto env = etsRuntime_.GetAniEnv();
    // if (env == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null env");
    //     return;
    // }

    // ani_value config;
    // std::string value = "This is callback value";
    // ani_status status = env->String_Create(value.c_str(), &config);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "create string failed");
    //     return;
    // }

    // // Define the callback class with onAsyncTaskCompleted method
    // ani_class asyncTaskCallbackClass = nullptr;
    // status = env->Class_Define("AsyncTaskCallback", nullptr, 0, nullptr, 0, &asyncTaskCallbackClass);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "define class failed");
    //     return;
    // }

    // // Define the static method
    // ani_method_desc methodDesc = {
    //     "onAsyncTaskCompleted",
    //     "Lohos/app/startup/AsyncTaskCallback;:V",
    //     AsyncTaskCallBack::AsyncTaskCompleted,
    //     ANI_METHOD_STATIC
    // };
    
    // status = env->Class_AddMethod(asyncTaskCallbackClass, &methodDesc, 1);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "add method failed");
    //     return;
    // }

    // asyncTaskExecutorCallbackEtsRef_ = 
    //     etsRuntime_.LoadSystemModule("app.appstartup.AsyncTaskCallback", 
    //                                 reinterpret_cast<ani_value*>(&asyncTaskCallbackClass), 1);
}

void EtsStartupTask::OnAsyncTaskCompleted(const std::shared_ptr<StartupTaskResult> &result)
{
    // TAG_LOGD(AAFwkTag::STARTUP, "called");
    // if (startupTaskResultCallback_ == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null startupTaskResultCallback");
    //     return;
    // }
    // startupTaskResultCallback_->Call(result);
}

void EtsStartupTask::UpdateContextRef(std::shared_ptr<NativeReference> contextRef)
{
    contextRef_ = StageContextTransfer::GetStaticRef(etsRuntime_, contextRef);
}

void EtsStartupTask::UpdateContextRef(ani_ref contextRef)
{
    contextRef_ = contextRef;
}

int32_t EtsStartupTask::RunTaskOnDependencyCompleted(const std::string &dependencyName,
    const std::shared_ptr<StartupTaskResult> &result)
{
    // auto env = etsRuntime_.GetAniEnv();
    // if (env == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null env");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }

    // if (startupEtsRef_ == nullptr) {
    //     int32_t result = LoadEtsOhmUrl();
    //     if (result != ERR_OK) {
    //         return result;
    //     }
    // }
    
    // ani_object startupValue = startupEtsRef_->aniObj;
    // if (startupValue == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, null object", name_.c_str());
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    
    // ani_method startupOnDepCompleted = nullptr;
    // ani_status status = env->Class_FindMethod(startupEtsRef_->aniCls, "onDependencyCompleted", 
    //                                          "Lohos/app/startup/StartupTask;:V", &startupOnDepCompleted);
    // if (startupOnDepCompleted == nullptr || status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, get onDependencyCompleted failed", name_.c_str());
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }

    // ani_object dependency = CreateEtsValue(env, dependencyName);
    // ani_object jsResult = GetDependencyResult(env, dependencyName, result);
    
    // ani_value argv[2] = { dependency, jsResult };
    // status = env->Object_CallMethod_Void_V(startupValue, startupOnDepCompleted, argv, 2);
    // if (status != ANI_OK) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "call onDependencyCompleted failed");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    
    return ERR_OK;
}

// ani_object EtsStartupTask::GetDependencyResult(ani_env env, const std::string &dependencyName,
//     const std::shared_ptr<StartupTaskResult> &result)
// {
//     if (result == nullptr || result->GetResultType() != StartupTaskResult::ResultType::ETS) {
//         return nullptr; // Return undefined equivalent
//     } else {
//         std::shared_ptr<EtsStartupTaskResult> etsResultPtr = std::static_pointer_cast<EtsStartupTaskResult>(result);
//         if (etsResultPtr == nullptr) {
//             TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, convert failed", dependencyName.c_str());
//             return nullptr;
//         }
//         ani_ref etsResultRef = etsResultPtr->GetEtsStartupResultRef();
//         if (etsResultRef == nullptr) {
//             return nullptr;
//         }
//         ani_object resultObj = nullptr;
//         ani_status status = env->GlobalReference_Get(etsResultRef, &resultObj);
//         if (status != ANI_OK) {
//             return nullptr;
//         }
//         return resultObj;
//     }
// }
} // namespace AbilityRuntime
} // namespace OHOS

extern "C" {
ETS_EXPORT OHOS::AbilityRuntime::AppStartupTask* OHOS_CreateEtsStartupTask(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime, const OHOS::AbilityRuntime::StartupTaskInfo &info,
    bool lazyLoad)
{
    if (runtime == nullptr) {
        return nullptr;
    }
    auto &etsRuntime = static_cast<OHOS::AbilityRuntime::ETSRuntime &>(*runtime);
    return new (std::nothrow) OHOS::AbilityRuntime::EtsStartupTask(etsRuntime, info, lazyLoad);
}
}