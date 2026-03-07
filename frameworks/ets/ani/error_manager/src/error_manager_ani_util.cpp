/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#include "error_manager_ani_util.h"
 
#include "ani.h"
#include <cstdint>
#include <unistd.h>
#include <memory>
#include "ability_business_error.h"
#include "app_recovery.h"
#include "application_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
 

namespace OHOS {
namespace AbilityRuntime {
bool IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}
 
bool IsNull(ani_env *env, ani_ref ref)
{
    ani_boolean isNull = ANI_FALSE;
    env->Reference_IsNull(ref, &isNull);
    return isNull;
}

ani_vm* GetAniVm(ani_env *env)
{
    ani_vm* vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetVM failed");
        return nullptr;
    }
    return vm;
}
 
ani_env* GetAniEnv(ani_vm *vm)
{
    ani_env* env = nullptr;
    if (vm->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "GetEnv failed");
        return nullptr;
    }
    return env;
}
 
ani_string StdStringToAniString(ani_env *env, const std::string &str)
{
    ani_string res;
    env->String_NewUTF8(str.c_str(), str.size(), &res);
    return res;
}
 
ani_object CreateErrorObject(ani_env *env, ani_string summary, ani_string name, ani_string stack, ani_string message,
    bool isError, const std::string instanceName, ani_int instanceType)
{
    ani_class cls{};
    if (ANI_OK != env->FindClass(CLASS_NAME_EVENT_PACKAGE_HOLDER, &cls)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "yhl -- FindClass %{public}s Failed", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }
 
    ani_method ctor{};
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "yhl -- get %{public}s ctor Failed", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }
 
    ani_object obj{};
 
    if (isError &&
        ANI_OK != env->Object_New(
                      cls, ctor, &obj, summary, message, name, stack, StdStringToAniString(env, instanceName), instanceType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "yhl -- Create Object Failed: %{public}s", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    } else if (ANI_OK !=
               env->Object_New(
                   cls, ctor, &obj, summary, message, name, stack, StdStringToAniString(env, instanceName), instanceType)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "yhl -- Create Object Failed: %{public}s", CLASS_NAME_EVENT_PACKAGE_HOLDER);
    }
    return obj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS