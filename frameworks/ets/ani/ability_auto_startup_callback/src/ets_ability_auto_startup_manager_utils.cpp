/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ets_ability_auto_startup_manager_utils.h"

#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
bool UnwrapAutoStartupInfo(ani_env *env, ani_object param, AutoStartupInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "env is nullptr");
        return false;
    }

    if (!AppExecFwk::GetStringProperty(env, param, "bundleName", info.bundleName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert bundleName failed");
        return false;
    }

    if (!AppExecFwk::GetStringProperty(env, param, "abilityName", info.abilityName)) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert abilityName failed");
        return false;
    }

    ani_double appCloneIndex = 0.0;
    if (AppExecFwk::IsExistsProperty(env, param, "appCloneIndex")) {
        if (!AppExecFwk::GetDoubleOrUndefined(env, param, "appCloneIndex", appCloneIndex)) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "convert appCloneIndex failed");
            return false;
        }
    }
    info.appCloneIndex = static_cast<int32_t>(appCloneIndex);
    
    AppExecFwk::GetStringProperty(env, param, "moduleName", info.moduleName);
    return true;
}

ani_object CreateEtsAutoStartupInfoArray(ani_env *env, const std::vector<AutoStartupInfo> &infoList)
{
    TAG_LOGD(AAFwkTag::AUTO_STARTUP, "called");
    ani_class arratCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "env is nullptr");
        return nullptr;
    }

    status = env->FindClass("Lescompat/Array", &arratCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "FindClass failed satus : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arratCls, "<cotr>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Class_FindMethod failed satus : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    status = env->Object_New(arratCls, arrayCtor, &arrayObj, infoList.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Objiect_New failed satus : %{public}d", status);
        return nullptr;
    }

    ani_size index = 0;
    for (auto &startUpInfo : infoList) {
        ani_object ani_info = WrapAutoStartupInfo(env, startUpInfo);
        if (ani_info == nullptr) {
            TAG_LOGW(AAFwkTag::AUTO_STARTUP, "null ani_info");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "Object_CallMethodByName_void failed satus : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

bool SetAutoStartupInfo(ani_env *env, ani_object object, const AutoStartupInfo &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "env is nullptr");
        return false;
    }
        
    ani_status status = ANI_OK;
    status = env->Object_SetPropertyByName_Ref(object, "bundleName",
        AppExecFwk::GetAniString(env, info.bundleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "bundleName failed status:%{public}d", status);
        return false;
    }

    status = env->Object_SetPropertyByName_Ref(object, "abilityName",
        AppExecFwk::GetAniString(env, info.abilityName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "abilityName failed status:%{public}d", status);
        return false;
    }

    status = env->Object_SetPropertyByName_Ref(object, "moduleName",
        AppExecFwk::GetAniString(env, info.moduleName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "moduleName failed status:%{public}d", status);
        return false;
    }

    status = env->Object_SetPropertyByName_Ref(object, "abilityTypeName",
        AppExecFwk::GetAniString(env, info.abilityTypeName));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTO_STARTUP, "abilityTypeName failed status:%{public}d", status);
        return false;
    }
    
    if (info.appCloneIndex >= 0 && info.appCloneIndex < GlobalConstant::MAX_APP_CLONE_INDEX) {
        status = env->Object_SetPropertyByName_Ref(object, "appCloneIndex",
            AppExecFwk::GetAniString(env, info.abilityTypeName));
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::AUTO_STARTUP, "appCloneIndex failed status:%{public}d", status);
            return false;
        }
    }
    return true;
}

ani_object WrapAutoStartupInfo(ani_env *env, const AutoStartupInfo &info)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    ani_method method {};
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null env");
        return nullptr;
    }
    if ((status = env->FindClass("Lapplication/AutoStartupInfo/AutoStartupInfoImpl;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "FindClass failed status : %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "find ctor failed status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Object_New failed status : %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null object");
        return nullptr;
    }
    if (!SetAutoStartupInfo(env, object, info)) {
        TAG_LOGE(AAFwkTag::APPMGR, "SetAutoStartupInfo failed");
        return nullptr;
    }

    return object;
}

} // namespace AbilityRuntime
} // namespace OHOS