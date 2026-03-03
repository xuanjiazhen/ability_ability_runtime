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

#include "ets_context_module.h"

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

namespace OHOS {
namespace AbilityRuntime {

ani_object EtsContextModule::NativeTransferStatic(ani_env *aniEnv, ani_object self, ani_object input, ani_object type)
{
    return nullptr;
}

ani_object EtsContextModule::CreateStaticObject(ani_env *aniEnv, ani_object type, std::shared_ptr<Context> context)
{
    return nullptr;
}

bool EtsContextModule::LoadTargetModule(ani_env *aniEnv, const std::string &className)
{
    return false;
}

std::unique_ptr<NativeReference> EtsContextModule::CreateNativeReference(napi_env napiEnv,
    std::shared_ptr<Context> context)
{
    return nullptr;
}

napi_value EtsContextModule::GetOrCreateDynamicObject(napi_env napiEnv, std::shared_ptr<Context> context)
{
    return nullptr;
}

ani_object EtsContextModule::NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls, ani_object input)
{
    return nullptr;
}

ani_object EtsContextModule::CreateDynamicObject(ani_env *aniEnv, ani_class aniCls,
    std::shared_ptr<Context> contextPtr)
{
    return nullptr;
}

void EtsContextModuleInit(ani_env *aniEnv)
{
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    if (vm == nullptr) {
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        return ANI_NOT_FOUND;
    }

    EtsContextModuleInit(aniEnv);
    *result = ANI_VERSION_1;
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
