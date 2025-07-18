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
#include "want.h"


namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr const char* ETS_MISSION_INFO_NAME = "Lapplication/MissionInfo/MissionInfoInner;";
constexpr const char* ID = "missionId";
constexpr const char* RUNNING_STATE = "runningState";
constexpr const char* LOCKED_STATE = "lockedState";
constexpr const char* TIMESTAMP = "timestamp";
constexpr const char* LABEL = "label";
constexpr const char* ICON_PATH = "iconPath";
constexpr const char* CONTINUABLE = "continuable";
constexpr const char* ABILITY_STATE = "abilityState";
constexpr const char* UNCLEARABLE = "unclearable";
constexpr const char* WANT = "want";
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
    status = env->Class_FindField(cls, WANT, &wantField);
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
    if (!AppExecFwk::SetDoublePropertyValue(env, object, ID, missionInfo.id) ||
        !AppExecFwk::SetDoublePropertyValue(env, object, RUNNING_STATE, missionInfo.runningState) ||
        !AppExecFwk::SetFieldBoolByName(env, cls, object, LOCKED_STATE, missionInfo.lockedState) ||
        !AppExecFwk::SetFieldStringByName(env, cls, object, TIMESTAMP, missionInfo.time) ||
        !AppExecFwk::SetFieldStringByName(env, cls, object, LABEL, missionInfo.label) ||
        !AppExecFwk::SetFieldStringByName(env, cls, object, ICON_PATH, missionInfo.iconPath) ||
        !AppExecFwk::SetFieldBoolByName(env, cls, object, CONTINUABLE, missionInfo.continuable) ||
        !AppExecFwk::SetDoublePropertyValue(env, object, ABILITY_STATE, missionInfo.abilityState) ||
        !AppExecFwk::SetFieldBoolByName(env, cls, object, UNCLEARABLE, missionInfo.unclearable)) {
            TAG_LOGE(AAFwkTag::MISSION, "set mission info failed");
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
