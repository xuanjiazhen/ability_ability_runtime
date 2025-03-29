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


#include "sts_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ERR_MSG_TOO_FEW_PARAM = "Parameter error. Too few parameters.";
constexpr const char* ERR_MSG_NOT_MAINTHREAD = "Caller error. Caller from non-main thread.";
constexpr const char* ERR_MSG_INVALID_NUM_PARAMS = "Parameter error. The number of parameters is invalid.";
constexpr const char* NOT_SYSTEM_APP = "The application is not system-app, can not use system-api.";

constexpr const char* BUSINESS_ERROR_CLASS = "L@ohos/base/BusinessError;";
} // namespace

void ThrowStsError(ani_env *env, ani_object err)
{
    if (err == nullptr) {
        return;
    }
    env->ThrowError(static_cast<ani_error>(err));
}

void ThrowStsError(ani_env *env, int32_t errCode, const std::string &errorMsg)
{
    ThrowStsError(env, CreateStsError(env, errCode, errorMsg));
}

void ThrowStsError(ani_env *env, const AbilityErrorCode &err)
{
    ThrowStsError(env, CreateStsError(env, static_cast<int32_t>(err), GetErrorMsg(err)));
}

void ThrowStsInvalidCallerError(ani_env *env)
{
    ThrowStsError(env, CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER),
        ERR_MSG_NOT_MAINTHREAD));
}

void ThrowStsTooFewParametersError(ani_env *env)
{
    ThrowStsError(env, CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
        ERR_MSG_TOO_FEW_PARAM));
}

void ThrowStsInvalidNumParametersError(ani_env *env)
{
    ThrowStsError(env, CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
        ERR_MSG_INVALID_NUM_PARAMS));
}

void ThrowStsNoPermissionError(ani_env *env, const std::string &permission)
{
    ThrowStsError(env, CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED),
        GetNoPermissionErrorMsg(permission)));
}

void ThrowStsNotSystemAppError(ani_env *env)
{
    ThrowStsError(env, CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP),
        NOT_SYSTEM_APP));
}

void ThrowStsInvalidParamError(ani_env *env, const std::string &message)
{
    ThrowStsError(env, CreateStsInvalidParamError(env, message));
}

void ThrowStsErrorByNativeErr(ani_env *env, int32_t err)
{
    ThrowStsError(env, CreateStsErrorByNativeErr(env, err));
}

ani_object CreateStsError(ani_env *env, const AbilityErrorCode &err)
{
    return CreateStsError(env, static_cast<int32_t>(err), GetErrorMsg(err));
}

ani_object CreateStsInvalidParamError(ani_env *env, const std::string &message)
{
    return CreateStsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM), message);
}

ani_object CreateStsNoPermissionError(ani_env *env, const std::string &permission)
{
    return CreateStsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED),
        GetNoPermissionErrorMsg(permission));
}

ani_object CreateStsErrorByNativeErr(ani_env *env, int32_t err, const std::string &permission)
{
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty()) ?
        GetNoPermissionErrorMsg(permission) : GetErrorMsg(errCode);
    return CreateStsError(env, static_cast<int32_t>(errCode), errMsg);
}

ani_object CreateStsError(ani_env *env, ani_int code, const std::string &msg)
{
    if (env == nullptr) {
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_field field = nullptr;
    ani_method method = nullptr;
    ani_object obj = nullptr;
    ani_status status = env->FindClass(BUSINESS_ERROR_CLASS, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "FindClass failed %{public}d", status);
        return nullptr;
    }
    status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    status = env->Object_New(cls, method, &obj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_New failed %{public}d", status);
        return nullptr;
    }
    status = env->Class_FindField(cls, "code", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Class_FindField failed %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetField_Double(obj, field, code);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_SetField_Double failed %{public}d", status);
        return nullptr;
    }
    status = env->Class_FindField(cls, "message", &field);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Class_FindField failed %{public}d", status);
        return nullptr;
    }
    ani_string string = nullptr;
    env->String_NewUTF8(msg.c_str(), msg.size(), &string);
    status = env->Object_SetField_Ref(obj, field, string);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_SetField_Ref failed %{public}d", status);
        return nullptr;
    }
    return obj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS