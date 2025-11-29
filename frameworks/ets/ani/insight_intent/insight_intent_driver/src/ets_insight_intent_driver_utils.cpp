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

#include "ets_insight_intent_driver_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* CLASSNAME_ARRAY = "Lescompat/Array;";
constexpr const char* EXECUTE_MODE_FOR_CONFIGURATION= "L@ohos/app/ability/insightIntentDriver/insightIntentDriver/ExecuteModeForConfiguration;";
constexpr const char* EXECUTE_MODE= "L@ohos/app/ability/insightIntent/insightIntent/ExecuteMode;";
}
ani_object CreateEtsEntityInfoForArray(ani_env *env, const std::vector<EntityInfoForQuery> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &info : infos) {
        ani_ref infoRef = CreateEtsEntityInfo(env, info);
        if (infoRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null infoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, infoRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsEntityInfo(ani_env *env, const EntityInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/EntityInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    env->Object_SetPropertyByName_Ref(objValue, "className", AppExecFwk::GetAniString(env, info.className));
    env->Object_SetPropertyByName_Ref(objValue, "entityId", AppExecFwk::GetAniString(env, info.entityId));
    env->Object_SetPropertyByName_Ref(objValue, "entityCategory", AppExecFwk::GetAniString(env, info.entityCategory));
    env->Object_SetPropertyByName_Ref(objValue, "parameters", AppExecFwk::WrapWantParams(env, AAFwk::WantParamWrapper::ParseWantParams(info.parameters)));
    env->Object_SetPropertyByName_Ref(objValue, "parentClassName", AppExecFwk::GetAniString(env, info.parentClassName));
    return objValue;
}

ani_object CreateEtsLinkInfoForQuery(ani_env *env, const LinkInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/LinkIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "uri", AppExecFwk::GetAniString(env, info.uri));
    return objValue;
}

ani_object CreateEtsPageInfoForQuery(ani_env *env, const PageInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/PageIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref( objValue, "uiAbility", AppExecFwk::GetAniString(env, info.uiAbility));
    env->Object_SetPropertyByName_Ref(objValue, "pagePath", AppExecFwk::GetAniString(env, info.pagePath));
    env->Object_SetPropertyByName_Ref(objValue, "navigationId", AppExecFwk::GetAniString(env, info.navigationId));
    env->Object_SetPropertyByName_Ref(objValue, "navDestinationName", AppExecFwk::GetAniString(env, info.navDestinationName));
    return objValue;
}

ani_object CreateEtsEntryInfoForQuery(ani_env *env, const EntryInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/EntryIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "executeMode", CreateExecuteModeArray(env, info.executeMode, EXECUTE_MODE));
    return objValue;
}

ani_object CreateEtsUiAbilityInfoForQuery(ani_env *env, const UIAbilityIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/UIAbilityIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if (info.abilityName.empty()) {
        return nullptr;
    }
    
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "executeMode", CreateExecuteModeArray(
        env, info.supportExecuteMode, EXECUTE_MODE_FOR_CONFIGURATION));
    return objValue;
}

ani_object CreateExecuteModeArray(ani_env *env, const std::vector<AppExecFwk::ExecuteMode> &executeModes, 
    const char *itemName)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, executeModes.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &executeMode : executeModes) {
        ani_enum_item modeItem = nullptr;
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
            env, itemName, executeMode, modeItem);
        if (modeItem == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null infoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, modeItem);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsUiExtensionInfoForQuery(ani_env *env, const UIExtensionIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/UIExtensionIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    return objValue;
}

ani_object CreateEtsServiceExtensionInfoForQuery(ani_env *env, const ServiceExtensionIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/ServiceExtensionIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    return objValue;
}

ani_object CreateEtsFormIntentInfoForQuery(ani_env *env, const FormIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/FormIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    if (info.abilityName.empty()) {
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "formName", AppExecFwk::GetAniString(env, info.formName));
    return objValue;
}

ani_object CreateEtsFunctionInfoForQuery(ani_env *env, const FunctionInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/FunctionIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    return objValue;
}

ani_object CreateEtsFormInfoForQuery(ani_env *env, const FormInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/FormIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "abilityName", AppExecFwk::GetAniString(env, info.abilityName));
    env->Object_SetPropertyByName_Ref(objValue, "formName", AppExecFwk::GetAniString(env, info.formName));
    return objValue;
}

ani_object CreateEtsInsightIntentInfoForQueryArray(ani_env *env, const std::vector<InsightIntentInfoForQuery> &infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &intentInfo : infos) {
        ani_ref intentInfoRef = CreateEtsInsightIntentInfoForQuery(env, intentInfo);
        if (intentInfoRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null intentInfoRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, intentInfoRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsConfigPutParams(ani_env *env, const std::vector<std::string> &putParams)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor = nullptr;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj = nullptr;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, putParams.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
        return arrayObj;
    }

    ani_size index = 0;
    for (auto &putParam : putParams) {
        ani_ref putParamRef =  AppExecFwk::WrapWantParams(env, AAFwk::WantParamWrapper::ParseWantParams(putParam));
        if (putParamRef == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null putParamRef");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, putParamRef);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "status : %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEtsConfigIntentInfo(ani_env *env, const InsightIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/SubIntentInfoForConfigurationInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }
    env->Object_SetPropertyByName_Ref(objValue, "srcEntry", AppExecFwk::GetAniString(env, info.srcEntry));
    if (info.inputParams.size() > 0) {
        env->Object_SetPropertyByName_Ref(objValue, "inputParams", CreateEtsConfigPutParams(env, info.inputParams));
    }
    if (info.outputParams.size() > 0) {
        env->Object_SetPropertyByName_Ref(objValue, "outputParams", CreateEtsConfigPutParams(env, info.outputParams));
    }
    ani_object uiAbilityInfo = CreateEtsUiAbilityInfoForQuery(env, info.uiAbilityIntentInfo);
    if (uiAbilityInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "uiAbility", uiAbilityInfo);
    }
    ani_object uiExtensionInfo = CreateEtsUiExtensionInfoForQuery(env, info.uiExtensionIntentInfo);
    if (uiExtensionInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "uiExtension", uiExtensionInfo);
    }
    ani_object serviceExtensionInfo = CreateEtsServiceExtensionInfoForQuery(env, info.serviceExtensionIntentInfo);
    if (serviceExtensionInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "serviceExtension", serviceExtensionInfo);
    }
    ani_object formIntentInfo = CreateEtsFormIntentInfoForQuery(env, info.formIntentInfo);
    if (formIntentInfo != nullptr) {
        env->Object_SetPropertyByName_Ref(objValue, "formIntent", formIntentInfo);
    }
    if (!info.cfgEntities.empty()) {
        env->Object_SetPropertyByName_Ref(objValue, "entities", AppExecFwk::WrapWantParams(env, AAFwk::WantParamWrapper::ParseWantParams(info.cfgEntities)));
    }
    return objValue;
}

ani_object CreateEtsInsightIntentInfoForQuery(ani_env *env, const InsightIntentInfoForQuery &info)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return nullptr;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method ctor = nullptr;
    ani_object objValue = {};

    if ((status = env->FindClass("L@ohos/app/ability/insightIntentDriver/insightIntentDriver/InsightIntentInfoInner;",
        &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    if ((status = env->Object_New(cls, ctor, &objValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        return nullptr;
    }

    status = env->Object_SetPropertyByName_Ref(objValue, "bundleName", AppExecFwk::GetAniString(env, info.bundleName));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "moduleName", AppExecFwk::GetAniString(env, info.moduleName));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "intentName", AppExecFwk::GetAniString(env, info.intentName));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "domain", AppExecFwk::GetAniString(env, info.domain));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "intentVersion", AppExecFwk::GetAniString(env, info.intentVersion));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "displayName", AppExecFwk::GetAniString(env, info.displayName));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "displayDescription", AppExecFwk::GetAniString(env, info.displayDescription));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "schema", AppExecFwk::GetAniString(env, info.schema));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "icon", AppExecFwk::GetAniString(env, info.icon));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    status = env->Object_SetPropertyByName_Ref(objValue, "llmDescription", AppExecFwk::GetAniString(env, info.llmDescription));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    if (info.isConfig) {
        std::string intentType = INSIGHT_INTENTS_TYPE_ENTRY;
        status = env->Object_SetPropertyByName_Ref(objValue, "intentType", AppExecFwk::GetAniString(env, intentType));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    } else {
        status = env->Object_SetPropertyByName_Ref(objValue, "intentType", AppExecFwk::GetAniString(env, info.intentType));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    }
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}s", info.parameters.c_str());
    if (!AAFwk::WantParamWrapper::ValidateStr(info.parameters)) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid Custom data string");
    }
    status = env->Object_SetPropertyByName_Ref(objValue, "parameters", AppExecFwk::WrapWantParams(env, AAFwk::WantParamWrapper::ParseWantParams(info.parameters)));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    AAFwk::WantParamWrapper::ParseWantParams(info.parameters).DumpInfo(0);
    ani_object stringArray = nullptr;
    AppExecFwk::WrapArrayString(env, stringArray, info.keywords);
    status = env->Object_SetPropertyByName_Ref(objValue, "keywords", stringArray);
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    if (!info.develoType.empty()) {
        status = env->Object_SetPropertyByName_Ref(objValue, "developType", AppExecFwk::GetAniString(env, info.develoType));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    }
    status = env->Object_SetPropertyByName_Ref(objValue, "entities", CreateEtsEntityInfoForArray(env, info.entities));
    TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    if (info.intentType == INSIGHT_INTENTS_TYPE_LINK) {
         status = env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsLinkInfoForQuery(env, info.linkInfo));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_PAGE) {
        status = env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsPageInfoForQuery(env, info.pageInfo));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_ENTRY) {
         status = env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsEntryInfoForQuery(env, info.entryInfo));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        } else if (info.intentType == INSIGHT_INTENTS_TYPE_FUNCTION) {
        status = env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsFunctionInfoForQuery(env, info.functionInfo));
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    } else if (info.intentType == INSIGHT_INTENTS_TYPE_FORM) {
        status = env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsFormInfoForQuery(env, info.formInfo));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    } else {
        if (info.isConfig) {
           status =  env->Object_SetPropertyByName_Ref(objValue, "subIntentInfo", CreateEtsEntryInfoForQuery(env, info.entryInfo));
            TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
        } else {
            //to do null subIntentInfoObj
        }
    }
    if (info.isConfig) {
       status =  env->Object_SetPropertyByName_Ref(objValue, "subIntentInfoForConfiguration", CreateEtsConfigIntentInfo(env, info));
        TAG_LOGE(AAFwkTag::INTENT, "status: %{public}d", status);
    }
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS