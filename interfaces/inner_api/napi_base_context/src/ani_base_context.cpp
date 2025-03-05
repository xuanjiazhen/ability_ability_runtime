#include "ani_base_context.h"

ani_status IsStageContext(ani_env* env, ani_object object, ani_boolean& stageMode)
{
    ani_class cls =nullptr;
    if ((env->FindClass("LBaseContest/BaseContest;", &cls)) != ANI_OK)
    {
        std::cerr << "FindClass failed" << std::endl;
    }

    ani_field filedStageMode = nullptr;
    if (env->Class_FindField(cls, "stageMode", &filedStageMode) != ANI_OK) {
        std::cerr << "FindField failed" << std::endl;
        return ANI_ERROR;
    }

    if (env->Object_GetField_Boolean(object, filedStageMode, &stageMode) != ANI_OK) {
        std::cerr << "GetField failed" << std::endl;
        return ANI_ERROR;
    }

    std::cerr << "GetField failed, stageMode : " << std::to_string(stageMode) << std::endl;
    return ANI_OK;
}

std::shared_ptr<OHOS::AbilityRuntime::Context> GetStageModeContext(ani_env* env, ani_object object)
{
    ani_class cls =nullptr;
    if ((env->FindClass("LBaseContest/Contest;", &cls)) != ANI_OK)
    {
        std::cerr << "FindClass failed" << std::endl;
    }

    ani_field field = nullptr;
    if ((env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK)
    {
        std::cerr << "Class_FindField failed" << std::endl;
    }

    ani_long nativeContextLong;
    if ((env->Object_GetField_Long(cls, field, &nativeContextLong)) != ANI_OK)
    {
        std::cerr << "Object_GetField_Long failed" << std::endl;
    }
    std::shared_ptr<OHOS::AbilityRuntime::Context> ptr((OHOS::AbilityRuntime::Context*)nativeContextLong);

    return ptr;
}

AppExecFwk::Ability* GetCurrentAbility(ani_env env)
{
    return nullptr;
}
