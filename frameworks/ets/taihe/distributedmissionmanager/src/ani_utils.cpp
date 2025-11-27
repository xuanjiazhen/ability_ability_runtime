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
#define LOG_TAG "AniUtils"
#include "ani_utils.h"

namespace ani_utils {

ani_status AniCreateInt(ani_env* env, int32_t value, ani_object& result)
{
    ani_status state;
    ani_class intClass;
    if ((state = env->FindClass("std.core.Int", &intClass)) != ANI_OK) {
        LOGE("FindClass std/core/Int failed, %{public}d", state);
        return state;
    }
    ani_method intClassCtor;
    if ((state = env->Class_FindMethod(intClass, "<ctor>", "i:", &intClassCtor)) != ANI_OK) {
        LOGE("Class_FindMethod Int ctor failed, %{public}d", state);
        return state;
    }
    ani_int aniValue = value;
    if ((state = env->Object_New(intClass, intClassCtor, &result, aniValue)) != ANI_OK) {
        LOGE("New Int object failed, %{public}d", state);
    }
    if (state != ANI_OK) {
        result = nullptr;
    }
    return state;
}

void AniExecuteFunc(ani_vm* vm, const std::function<void(ani_env*)> func)
{
    LOGI("AniExecutePromise");
    if (vm == nullptr) {
        LOGE("AniExecutePromise, vm error");
        return;
    }
    ani_env *currentEnv = nullptr;
    ani_status aniResult = vm->GetEnv(ANI_VERSION_1, &currentEnv);
    if (ANI_OK == aniResult && currentEnv != nullptr) {
        LOGI("AniExecutePromise, env exist");
        func(currentEnv);
        return;
    }

    ani_env* newEnv = nullptr;
    ani_options aniArgs { 0, nullptr };
    aniResult = vm->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &newEnv);
    if (ANI_OK != aniResult || newEnv == nullptr) {
        LOGE("AniExecutePromise, AttachCurrentThread error");
        return;
    }
    func(newEnv);
    aniResult = vm->DetachCurrentThread();
    if (ANI_OK != aniResult) {
        LOGE("AniExecutePromise, DetachCurrentThread error");
        return;
    }
}

} //namespace ani_utils