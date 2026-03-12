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

#include "error_manager_ani_util.h"
#include "hilog_tag_wrapper.h"
#include "ierror_obserrver.h"

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

ani_object CreateErrorObject(ani_env *env, const AppExecFwk::ErrorObject &errorObj)
{
    ani_object error {};
    if (env == nullptr) {
        return error;
    }
    ani_class cls {};
    if (env->FindClass(CLASS_NAME_BUSINESSERROR, &cls) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "find class %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return error;
    }
    ani_method ctor {};
    if (env->Class_FindMethod(cls, "<ctor>", ":", &ctor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "find method BusinessError constructor failed");
        return error;
    }
    if (env->Object_New(cls, ctor, &error) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "new object %{public}s failed", CLASS_NAME_BUSINESSERROR);
        return error;
    }
    ani_string messageRef {};
    std::string message = errorObj.message;
    if (env->String_NewUTF8(message.c_str(), message.size(), &messageRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "new message string failed");
        return error;
    }
    if (env->Object_SetPropertyByName_Ref(error, "message", static_cast<ani_ref>(messageRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "set property BusinessError.message failed");
        return error;
    }
    return error;
}
}  // namespace AbilityRuntime
}  // namespace OHOS

