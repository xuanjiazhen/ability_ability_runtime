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

#include "sts_form_extension_context.h"

#include <algorithm>
#include <iterator>
#include "ani_common/ani_common_configuration.h"
#include "form_extension_context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
ani_object STSFormExtensionContext::SetFormExtensionContext(
    ani_env *env, const std::shared_ptr<FormExtensionContext> &context)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "SetFormExtensionContext call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env is nullptr");
        return nullptr;
    }
    ani_long nativeContextLong = (ani_long)context.get();
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_object contextObj = nullptr;
    ani_method method = nullptr;
    ani_field field = nullptr;

    if ((status = env->FindClass("LFormExtensionContext/FormExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "SetFormExtensionContext end");
    return contextObj;
}

void STSFormExtensionContext::startAbility1(const OHOS::AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "startAbility1 start");
}

ani_ref STSFormExtensionContext::CreateStsExtensionContext(ani_env *env,
    const std::shared_ptr<FormExtensionContext> &context,
    const std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> &abilityInfo)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateStsExtensionContext call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env is nullptr");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass("LFormExtensionContext/FormExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "startAbility1", "L@ohos/app/ability/Want/Want;:V",
            reinterpret_cast<void *>(STSFormExtensionContext::startAbility1) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_object contextObj = STSFormExtensionContext::SetFormExtensionContext(env, context);
    ani_field field = nullptr;
    auto configuration = context->GetConfiguration();
    ani_ref configurationRef = OHOS::AppExecFwk::WrapConfiguration(env, *configuration);
    if ((status = env->Class_FindField(cls, "config", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetField_Ref(contextObj, field, configurationRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateStsExtensionContext end");
    return contextObj;
}

ani_ref CreateStsFormExtensionContext(ani_env *env, const std::shared_ptr<FormExtensionContext> &context)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateStsFormExtensionContext call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env is nullptr");
        return nullptr;
    }
    auto abilityInfo = context->GetAbilityInfo();
    ani_ref object = STSFormExtensionContext::CreateStsExtensionContext(env, context, abilityInfo);
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateStsFormExtensionContext end");
    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS