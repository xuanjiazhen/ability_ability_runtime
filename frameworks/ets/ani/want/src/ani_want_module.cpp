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

#include "ani_want_module.h"

#include "ani_common_util.h"
#include "double_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"

namespace OHOS::AppExecFwk {
namespace {
constexpr const char *ETS_NATIVE_WANT_PARAMS_CLASS_NAME = "L@ohos/app/ability/Want/NativeWantParams;";
constexpr const char *ETS_NATIVE_WANT_PARAMS_CLEANER_CLASS_NAME = "L@ohos/app/ability/Want/NativeWantParamsCleaner;";
} // namespace

ani_long EtsWantParams::NativeCreate(ani_env *env, ani_object)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return 0;
    }
    auto *params = new(std::nothrow) AAFwk::WantParams();
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null params");
        return 0;
    }

    return reinterpret_cast<ani_long>(params);
}

void EtsWantParams::NativeDestroy(ani_env *env, ani_object, ani_long nativeWantParams)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return;
    }

    auto *params = reinterpret_cast<AAFwk::WantParams *>(nativeWantParams);
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null nativeWantParams");
        return;
    }

    delete params;
}

ani_boolean EtsWantParams::NativeSetStringParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
     ani_string value)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return false;
    }

    auto *params = reinterpret_cast<AAFwk::WantParams *>(nativeWantParams);
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null nativeWantParams");
        return false;
    }

    std::string keyString;
    if (!GetStdString(env, key, keyString)) {
        TAG_LOGE(AAFwkTag::ANI, "get key failed");
        return false;
    }
    std::string valueString;
    if (!GetStdString(env, value, valueString)) {
        TAG_LOGE(AAFwkTag::ANI, "get key failed");
        return false;
    }
    params->SetParam(keyString, AAFwk::String::Box(valueString));
    return true;
}

ani_boolean EtsWantParams::NativeSetNumberParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
    ani_double value)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return false;
    }

    auto *params = reinterpret_cast<AAFwk::WantParams *>(nativeWantParams);
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null nativeWantParams");
        return false;
    }

    std::string keyString;
    if (!GetStdString(env, key, keyString)) {
        TAG_LOGE(AAFwkTag::ANI, "get key failed");
        return false;
    }

    params->SetParam(keyString, AAFwk::Double::Box(value));
    return true;
}

ani_boolean EtsWantParams::NativeSetIntParam(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
    ani_int value)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return false;
    }

    auto *params = reinterpret_cast<AAFwk::WantParams *>(nativeWantParams);
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null nativeWantParams");
        return false;
    }

    std::string keyString;
    if (!GetStdString(env, key, keyString)) {
        TAG_LOGE(AAFwkTag::ANI, "get key failed");
        return false;
    }

    params->SetParam(keyString, AAFwk::Integer::Box(value));
    return true;
}

ani_boolean EtsWantParams::NativeSetWantParams(ani_env *env, ani_object, ani_long nativeWantParams, ani_string key,
    ani_long value)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null env");
        return false;
    }

    auto *params = reinterpret_cast<AAFwk::WantParams *>(nativeWantParams);
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null nativeWantParams");
        return false;
    }

    std::string keyString;
    if (!GetStdString(env, key, keyString)) {
        TAG_LOGE(AAFwkTag::ANI, "get key failed");
        return false;
    }

    auto *valueWantParams = reinterpret_cast<AAFwk::WantParams *>(value);
    if (valueWantParams == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null valueWantParams");
        return false;
    }

    params->SetParam(keyString, AAFwk::WantParamWrapper::Box(*valueWantParams));
    return true;
}

ani_status BindNativeFunctions(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::WANT, "call");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null ani env");
        return ANI_INVALID_ARGS;
    }

    ani_class nativeWantParamsCls = nullptr;
    auto status = aniEnv->FindClass(ETS_NATIVE_WANT_PARAMS_CLASS_NAME, &nativeWantParamsCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::WANT, "FindClass NativeWantParams failed status: %{public}d", status);
        return status;
    }

    std::array nativeFuncs = {
        ani_native_function{
            "nativeCreate", ":J",
            reinterpret_cast<void *>(EtsWantParams::NativeCreate)
        },
        ani_native_function{
            "nativeSetStringParam", "JLstd/core/String;Lstd/core/String;:Z",
            reinterpret_cast<void *>(EtsWantParams::NativeSetStringParam)
        },
        ani_native_function{
            "nativeSetNumberParam", "JLstd/core/String;D:Z",
            reinterpret_cast<void *>(EtsWantParams::NativeSetNumberParam)
        },
        ani_native_function{
            "nativeSetIntParam", "JLstd/core/String;I:Z",
            reinterpret_cast<void *>(EtsWantParams::NativeSetIntParam)
        },
        ani_native_function{
            "nativeSetWantParams", "JLstd/core/String;J:Z",
            reinterpret_cast<void *>(EtsWantParams::NativeSetWantParams)
        },
    };
    status = aniEnv->Class_BindNativeMethods(nativeWantParamsCls, nativeFuncs.data(), nativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::WANT, "Class_BindNativeMethods failed status: %{public}d", status);
        return status;
    }

    ani_class nativeWantParamsCleanerCls = nullptr;
    status = aniEnv->FindClass(ETS_NATIVE_WANT_PARAMS_CLEANER_CLASS_NAME, &nativeWantParamsCleanerCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::WANT, "FindClass NativeWantParams failed status: %{public}d", status);
        return status;
    }
    std::array cleanerNativeFuncs = {
        ani_native_function{
            "nativeDestroy", "J:V",
            reinterpret_cast<void *>(EtsWantParams::NativeDestroy)
        },
    };
    status = aniEnv->Class_BindNativeMethods(nativeWantParamsCleanerCls, cleanerNativeFuncs.data(),
        cleanerNativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::WANT, "Class_BindNativeMethods failed status: %{public}d", status);
        return status;
    }
    return ANI_OK;
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::WANT, "ANI_Constructor");
    ani_env *env = nullptr;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "null vm");
        return ANI_NOT_FOUND;
    }
    ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::WANT, "GetEnv failed status: %{public}d, or null env", status);
        return status;
    }
    if ((status = BindNativeFunctions(env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::WANT, "BindNativeFunctions failed status: %{public}d", status);
        return status;
    }
    *result = ANI_VERSION_1;
    return ANI_OK;
}
}
} // namespace OHOS::AppExecFwk
