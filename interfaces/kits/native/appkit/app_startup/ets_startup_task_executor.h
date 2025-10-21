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


#ifndef ETS_STARTUP_TASK_EXECUTOR_H
#define ETS_STARTUP_TASK_EXECUTOR_H

#include "ani_runtime.h"
#include "startup_task_result.h"
#include "startup_task_result_callback.h"

namespace OHOS {
namespace AbilityRuntime {

class ETSStartupTaskExecutor {
public:
    static int32_t RunOnTaskPool(ani_env *env, ani_object startup, ani_object context, 
        ani_object asyncTaskExecutor, ani_object asyncTaskCallback, const std::string &startupName);
    
    static int32_t RunOnMainThread(ani_env *env, ani_object startup, ani_object context, 
        std::unique_ptr<StartupTaskResultCallback> callback);
    
    static int32_t CallStartupInit(ani_env *env, ani_object startup, ani_object context, 
        std::unique_ptr<StartupTaskResultCallback> &callback, ani_object &returnVal);
    
    static int32_t HandleReturnVal(ani_env *env, ani_object returnVal,
        std::unique_ptr<StartupTaskResultCallback> &callback);
    
    static ani_object ResolveResultCallback(ani_env *env, ani_object thisObj, 
        ani_object *args, int32_t argc, void *data);
    
    static ani_object RejectResultCallback(ani_env *env, ani_object thisObj, 
        ani_object *args, int32_t argc, void *data);
    
    static void ReplyFailed(StartupTaskResultCallback *callback, int32_t resultCode, 
        const std::string &resultMessage);
    
    static void ReplyFailed(std::unique_ptr<StartupTaskResultCallback> callback, 
        int32_t resultCode, const std::string &resultMessage);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // ETS_STARTUP_TASK_EXECUTOR_H