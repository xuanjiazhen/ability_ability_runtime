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

#include "ets_mission_info_utils.h"

#include "ani_common_want.h"
#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"


namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr const char* ETS_MISSION_INFO_NAME = "Lapplication/MissionInfo/MissionInfoInner;";
}

bool InnerWrapWant(ani_env *env, ani_class cls, ani_object object, const AAFwk::Want &want)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "env is null");
        return false;
    }
    ani_object wantObj = AppExecFwk::WrapWant(env, want);
    if (wantObj == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "wrap want failed");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field wantField = nullptr;
    status = env->Class_FindField(cls, "want", &wantField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "find want field failed status: %{public}d", status);
        return false;
    }
    status = env->Object_SetField_Ref(object, wantField, reinterpret_cast<ani_ref>(wantObj));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "Object_SetField_Ref failed status: %{public}d", status);
        return false;
    }
    return true;
}

bool WrapMissionInfo(ani_env *env, ani_class cls, ani_object object, const AAFwk::MissionInfo &missionInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "result or env is null");
        return false;
    }
    if (!AppExecFwk::SetDoublePropertyValue(env, object, "missionId", missionInfo.id)) {
        TAG_LOGE(AAFwkTag::MISSION, "set missionId failed");
        return false;
    }
    if (!AppExecFwk::SetDoublePropertyValue(env, object, "runningState", missionInfo.runningState)) {
        TAG_LOGE(AAFwkTag::MISSION, "set runningState failed");
        return false;
    }
    if (!AppExecFwk::SetFieldBoolean(env, cls, object, "lockedState", missionInfo.lockedState)) {
        TAG_LOGE(AAFwkTag::MISSION, "set lockedState failed");
        return false;
    }
    if (!AppExecFwk::SetFieldString(env, cls, object, "timestamp", missionInfo.time)) {
        TAG_LOGE(AAFwkTag::MISSION, "set timestamp failed");
        return false;
    }
    if (!AppExecFwk::SetFieldString(env, cls, object, "label", missionInfo.label)) {
        TAG_LOGE(AAFwkTag::MISSION, "set label failed");
        return false;
    }
    if (!AppExecFwk::SetFieldString(env, cls, object, "iconPath", missionInfo.iconPath)) {
        TAG_LOGE(AAFwkTag::MISSION, "set iconPath failed");
        return false;
    }
    if (!AppExecFwk::SetFieldBoolean(env, cls, object, "continuable", missionInfo.continuable)) {
        TAG_LOGE(AAFwkTag::MISSION, "set continuable failed");
        return false;
    }
    if (!AppExecFwk::SetDoublePropertyValue(env, object, "abilityState", missionInfo.abilityState)) {
        TAG_LOGE(AAFwkTag::MISSION, "set abilityState failed");
        return false;
    }
    if (!AppExecFwk::SetFieldBoolean(env, cls, object, "unclearable", missionInfo.unclearable)) {
        TAG_LOGE(AAFwkTag::MISSION, "set unclearable failed");
        return false;
    }
    if (!InnerWrapWant(env, cls, object, missionInfo.want)) {
        TAG_LOGE(AAFwkTag::MISSION, "wrap want failed");
        return false;
    }
    return true;
}

ani_object CreateEtsMissionInfo(ani_env *env, const AAFwk::MissionInfo &missionInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "result or env is null");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    status = env->FindClass(ETS_MISSION_INFO_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "find MissionInfo failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "Class_FindMethod ctor failed status: %{public}d", status);
        return nullptr;
    }
    ani_object object = nullptr;
    if (env->Object_New(cls, method, &object) != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "Object_New failed status: %{public}d", status);
        return nullptr;
    }

    if (!WrapMissionInfo(env, cls, object, missionInfo)) {
        TAG_LOGE(AAFwkTag::MISSION, "WrapMissionInfo failed");
        return nullptr;
    }
    return object;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
