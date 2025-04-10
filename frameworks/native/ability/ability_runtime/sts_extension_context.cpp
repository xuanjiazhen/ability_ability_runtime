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

#include "sts_extension_context.h"
#include "ani_common_configuration.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

void BindConfiguration(ani_env* aniEnv, ani_class contextClass, ani_object& contextObj,
    std::shared_ptr<ExtensionContext> context)
{
    ani_field configField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "config", &configField)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "find config failed");
        return;
    }
    auto config = context->GetConfiguration();
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetConfiguration failed");
        return;
    }
    AppExecFwk::Configuration configuration = *config;
    ani_object object = AppExecFwk::WrapConfiguration(aniEnv, configuration);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "WrapConfiguration failed");
        return;
    }
    if (aniEnv->Object_SetField_Ref(contextObj, configField, object) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_SetField_Ref failed");
        return;
    }
}

ani_object CreateJsExtensionContext(ani_env* env, const std::shared_ptr<ExtensionContext>& context)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_object contextObj = nullptr;
    ani_method method {};

    if ((status = env->FindClass("Lapplication/ExtensionContext/ExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UIABILITY, "status : %{public}d", status);
        return nullptr;
    }
    BindConfiguration(env, cls, contextObj, context);

    return contextObj;
}

}
}