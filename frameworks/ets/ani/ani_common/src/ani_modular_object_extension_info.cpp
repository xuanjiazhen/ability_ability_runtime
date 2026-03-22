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

#include "ani_modular_object_extension_info.h"

#include <memory>
#include "ani_enum_convert.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "hilog_tag_wrapper.h"
#include "want_params.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr const char *MODULAR_OBJECT_EXTENSION_INFO_CLASS_NAME =
    "@ohos.app.ability.modularObjectManager.modularObjectManager.ModularObjectExtensionInfoInner";
constexpr const char *ARRAY_CLASS_NAME = "std.core.Array";

bool WrapStringOfModularObjectExtensionInfo(ani_env *env, ani_object object,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;

    // bundleName: string
    status = env->Object_SetPropertyByName_Ref(object, "bundleName", AppExecFwk::GetAniString(env, info.bundleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "bundleName failed status:%{public}d", status);
        return false;
    }

    // moduleName: string
    status = env->Object_SetPropertyByName_Ref(object, "moduleName", AppExecFwk::GetAniString(env, info.moduleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "moduleName failed status:%{public}d", status);
        return false;
    }

    // abilityName: string
    status = env->Object_SetPropertyByName_Ref(object, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "abilityName failed status:%{public}d", status);
        return false;
    }
    return true;
}

bool WrapIntOfModularObjectExtensionInfo(ani_env *env, ani_object object,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;

    // appIndex: int
    if ((status = env->Object_SetPropertyByName_Int(object, "appIndex", info.appIndex)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "appIndex failed status:%{public}d", status);
        return false;
    }
    return true;
}

bool WrapBoolOfModularObjectExtensionInfo(ani_env *env, ani_object object,
    const AAFwk::ModularObjectExtensionInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;

    // isSelfDisabled: boolean
    if ((status = env->Object_SetPropertyByName_Boolean(object, "isSelfDisabled", info.isSelfDisabled)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "isSelfDisabled failed status:%{public}d", status);
        return false;
    }

    // isSystemDisabled: boolean
    if ((status = env->Object_SetPropertyByName_Boolean(object, "isSystemDisabled", info.isSystemDisabled)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "isSystemDisabled failed status:%{public}d", status);
        return false;
    }
    return true;
}
}  // namespace
ani_object CreateEmptyAniArray(ani_env *env)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    status = env->FindClass(ARRAY_CLASS_NAME, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", ":", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New array failed status : %{public}d", status);
        return arrayObj;
    }
    return arrayObj;
}

ani_object WrapModularObjectExtensionInfo(ani_env *env, const AAFwk::ModularObjectExtensionInfo &info)
{
    TAG_LOGD(AAFwkTag::ANI, "called WrapModularObjectExtensionInfo");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object object = {};

    if ((status = env->FindClass(MODULAR_OBJECT_EXTENSION_INFO_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI,
            "Object_New ModularObjectExtensionInfoInner failed status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null object");
        return nullptr;
    }
    if (!WrapStringOfModularObjectExtensionInfo(env, object, info)) {
        return nullptr;
    }
    if (!WrapIntOfModularObjectExtensionInfo(env, object, info)) {
        return nullptr;
    }
    if (!WrapBoolOfModularObjectExtensionInfo(env, object, info)) {
        return nullptr;
    }
    return object;
}

ani_object WrapModularObjectExtensionInfoArray(ani_env *env,
    const std::vector<AAFwk::ModularObjectExtensionInfo> &infos)
{
    TAG_LOGD(AAFwkTag::ANI, "called WrapModularObjectExtensionInfoArray");

    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }

    status = env->FindClass(ARRAY_CLASS_NAME, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "i:", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New array status: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto &info : infos) {
        ani_object aniInfo = WrapModularObjectExtensionInfo(env, info);
        if (aniInfo == nullptr) {
            TAG_LOGW(AAFwkTag::ANI, "null aniInfo");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "iY:", index, aniInfo);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::ANI,
                "Object_CallMethodByName_Void failed status: %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
