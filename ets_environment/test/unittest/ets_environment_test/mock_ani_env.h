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
#ifndef OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_MOCK_ANI_ENV_H
#define OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_MOCK_ANI_ENV_H

#include <cstddef>
#include <string>
#include <vector>

#include "ani.h"
#include "runtime.h"

namespace OHOS {
namespace EtsEnv {

class MockAniEnv {
public:
    struct State {
        ani_status getUndefinedStatus { ANI_OK };
        ani_status arrayNewStatus { ANI_OK };
        ani_status stringNewStatus { ANI_OK };
        ani_status arraySetStatus { ANI_OK };
        ani_status callMethodStatus { ANI_OK };
        ani_status findClassStatus { ANI_OK };
        ani_status classFindMethodStatus { ANI_OK };
        ani_status classFindFieldStatus { ANI_OK };
        ani_status objectNewStatus { ANI_OK };
        ani_status objectGetFieldLongStatus { ANI_OK };
        ani_status objectSetFieldLongStatus { ANI_OK };
        ani_status globalRefCreateStatus { ANI_OK };
        ani_status globalRefDeleteStatus { ANI_OK };
        ani_status arrayNewRefStatus { ANI_OK };
        ani_status arraySetRefStatus { ANI_OK };
        ani_status classCallStaticMethodStatus { ANI_OK };
        ani_status existUnhandledErrorStatus { ANI_OK };
        ani_status getUnhandledErrorStatus { ANI_OK };
        ani_status resetErrorStatus { ANI_OK };
        ani_status objectGetTypeStatus { ANI_OK };
        ani_status classFindGetterStatus { ANI_OK };
        ani_status objectCallMethodRefStatus { ANI_OK };
        ani_status stringGetUtf8SizeStatus { ANI_OK };
        ani_status stringGetUtf8SubStringStatus { ANI_OK };
        ani_boolean unhandledErrorExists { ANI_FALSE };
        std::string errorName;
        std::string errorMessage;
        std::string errorStack;
        std::vector<ani_status> callMethodStatusSequence;
        size_t callMethodStatusIndex { 0 };
    };

    MockAniEnv();
    ~MockAniEnv() = default;

    ani_env *GetEnv();
    State &GetState();

private:
    void InitializeApi();
    static void ResetState(State &state);

    static ani_status GetUndefined(ani_env *env, ani_ref *result);
    static ani_status ArrayNew(ani_env *env, ani_size length, ani_ref initValue, ani_array *result);
    static ani_status ArrayNewRef(ani_env *env, ani_type type, ani_size length,
        ani_ref initValue, ani_array *result);
    static ani_status StringNewUtf8(ani_env *env, const char *utf8, ani_size len, ani_string *result);
    static ani_status ArraySet(ani_env *env, ani_array array, ani_size index, ani_ref value);
    static ani_status ArraySetRef(ani_env *env, ani_array array, ani_size index, ani_ref value);
    static ani_status CallMethodVoid(ani_env *env, ani_object obj, const char *name, const char *sig, va_list args);

    static ani_status FindClass(ani_env *env, const char *name, ani_class *cls);
    static ani_status ClassFindMethod(ani_env *env, ani_class cls, const char *name, const char *sig,
        ani_method *method);
    static ani_status ClassCallStaticMethodByNameVoid(ani_env *env, ani_class cls, const char *name,
        const char *sig, va_list args);
    static ani_status ClassFindField(ani_env *env, ani_class cls, const char *name, ani_field *field);
    static ani_status ObjectNew(ani_env *env, ani_class cls, ani_method method, ani_object *result, va_list args);
    static ani_status ObjectGetType(ani_env *env, ani_object obj, ani_type *result);
    static ani_status ClassFindGetter(ani_env *env, ani_class cls, const char *name, ani_method *result);
    static ani_status ObjectCallMethodRef(ani_env *env, ani_object obj, ani_method method, ani_ref *result,
        va_list args);
    static ani_status StringGetUtf8Size(ani_env *env, ani_string string, ani_size *result);
    // NOLINTNEXTLINE: ANI callback signature is fixed by the interface.
    static ani_status StringGetUtf8SubString(ani_env *env, ani_string string, ani_size substrOffset,
        ani_size substrSize, char *buffer, ani_size bufferSize, ani_size *result);
    static ani_status ObjectGetFieldLong(ani_env *env, ani_object obj, ani_field field, ani_long *result);
    static ani_status ObjectSetFieldLong(ani_env *env, ani_object obj, ani_field field, ani_long value);
    static ani_status GlobalReferenceCreate(ani_env *env, ani_ref ref, ani_ref *result);
    static ani_status GlobalReferenceDelete(ani_env *env, ani_ref ref);
    static ani_status ExistUnhandledError(ani_env *env, ani_boolean *result);
    static ani_status GetUnhandledError(ani_env *env, ani_error *result);
    static ani_status ResetError(ani_env *env);

    __ani_interaction_api api_ {};
    ani_env env_ {};
    static State state_;
};
} // namespace EtsEnv
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_MOCK_ANI_ENV_H
