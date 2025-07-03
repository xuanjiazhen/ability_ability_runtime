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
#include <cstring>

#include "ani_common_util.h"

#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
constexpr const char* CLASSNAME_DOUBLE = "Lstd/core/Double;";
constexpr const char* CLASSNAME_BOOL = "Lstd/core/Boolean;";
constexpr const char* CLASSNAME_ARRAY = "Lescompat/Array;";
constexpr const char* CLASSNAME_ASYNC_CALLBACK_WRAPPER = "Lutils/AbilityUtils/AsyncCallbackWrapper;";
constexpr const char* SET_OBJECT_VOID_SIGNATURE = "ILstd/core/Object;:V";

bool GetFieldDoubleByName(ani_env *env, ani_object object, const char *name, double &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_double aniValue = 0.0;
    if ((status = env->Object_CallMethodByName_Double(
        reinterpret_cast<ani_object>(field), "doubleValue", nullptr, &aniValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<double>(aniValue);
    return true;
}

bool SetFieldDoubleByName(ani_env *env, ani_class cls, ani_object object, const char *name, double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object obj = CreateDouble(env, value);
    if (obj == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "CreateDouble failed");
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, reinterpret_cast<ani_ref>(obj))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldBoolByName(ani_env *env, ani_object object, const char *name, bool &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_boolean aniValue = ANI_FALSE;
    if ((status = env->Object_CallMethodByName_Boolean(
        reinterpret_cast<ani_object>(field), "booleanValue", nullptr, &aniValue)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<bool>(aniValue);
    return true;
}

bool SetFieldBoolByName(ani_env *env, ani_class cls, ani_object object, const char *name, bool value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Boolean(object, field, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldStringByName(ani_env *env, ani_object object, const char *name, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref field = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(field, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    if (!GetStdString(env, reinterpret_cast<ani_string>(field), value)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString failed");
        return false;
    }
    return true;
}

bool SetFieldStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }

    if (value.empty()) {
        ani_ref nullRef = nullptr;
        if ((status = env->GetNull(&nullRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
        if ((status = env->Object_SetField_Ref(object, field, nullRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
        return true;
    }
    ani_string aniStr = nullptr;
    if ((status = env->String_NewUTF8(value.c_str(), value.size(), &aniStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, aniStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldIntByName(ani_env *env, ani_object object, const char *name, int &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_int aniInt = 0;
    if ((status = env->Object_GetFieldByName_Int(object, name, &aniInt)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value = static_cast<int>(aniInt);
    return true;
}

bool SetFieldIntByName(ani_env *env, ani_class cls, ani_object object, const char *name, int value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Int(object, field, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldStringArrayByName(ani_env *env, ani_object object, const char *name, std::vector<std::string> &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_ref arrayObj = nullptr;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(arrayObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s: undefined", name);
        return false;
    }
    ani_double length = 0;
    if ((status = env->Object_GetPropertyByName_Double(reinterpret_cast<ani_object>(arrayObj),
        "length", &length)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    for (int i = 0; i < static_cast<int>(length); i++) {
        ani_ref stringEntryRef;
        status = env->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(arrayObj),
            "$_get", "I:Lstd/core/Object;", &stringEntryRef, (ani_int)i);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d, index: %{public}d", status, i);
            return false;
        }
        std::string str = "";
        if (!GetStdString(env, reinterpret_cast<ani_string>(stringEntryRef), str)) {
            TAG_LOGE(AAFwkTag::ANI, "GetStdString failed, index: %{public}d", i);
            return false;
        }
        value.emplace_back(str);
        TAG_LOGD(AAFwkTag::ANI, "GetStdString index: %{public}d %{public}s", i, str.c_str());
    }
    return true;
}

bool SetFieldArrayStringByName(ani_env *env, ani_class cls, ani_object object, const char *name,
    const std::vector<std::string> &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_class arrayCls = nullptr;
    if ((status = env->FindClass(CLASSNAME_ARRAY, &arrayCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_method arrayCtor = nullptr;
    if ((status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_object arrayObj = nullptr;
    if ((status = env->Object_New(arrayCls, arrayCtor, &arrayObj, value.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    for (size_t i = 0; i < value.size(); i++) {
        ani_string str = nullptr;
        if ((status = env->String_NewUTF8(value[i].c_str(), value[i].size(), &str)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
        if ((status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V",
            i, str)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
            return false;
        }
    }
    if ((status = env->Object_SetField_Ref(object, field, arrayObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetFieldRefByName(ani_env *env, ani_object object, const char *name, ani_ref &ref)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Ref(object, name, &ref)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_boolean isUndefined = ANI_TRUE;
    if ((status = env->Reference_IsUndefined(ref, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s is undefined", name);
        return false;
    }
    return true;
}

bool SetFieldRefByName(ani_env *env, ani_class cls, ani_object object, const char *name, ani_ref value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, name, &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetField_Ref(object, field, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetStdString(ani_env *env, ani_string str, std::string &value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_size sz = 0;
    if ((status = env->String_GetUTF8Size(str, &sz)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value.resize(sz + 1);
    if ((status = env->String_GetUTF8SubString(str, 0, sz, value.data(), value.size(), &sz)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    value.resize(sz);
    return true;
}

ani_string GetAniString(ani_env *env, const std::string &str)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_string aniStr = nullptr;
    ani_status status = env->String_NewUTF8(str.c_str(), str.size(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return aniStr;
}

ani_object CreateDouble(ani_env *env, ani_double value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(CLASSNAME_DOUBLE, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_method ctor = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "D:V", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object CreateBoolean(ani_env *env, ani_boolean value)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(CLASSNAME_BOOL, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_method ctor = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "Z:V", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, ctor, &obj, value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return nullptr;
    }
    return obj;
}

bool AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_status status = ANI_ERROR;
    ani_class clsCall = nullptr;

    if ((status = env->FindClass(CLASSNAME_ASYNC_CALLBACK_WRAPPER, &clsCall)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(
        clsCall, "invoke", "L@ohos/base/BusinessError;Lstd/core/Object;:V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    if (result == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        result = reinterpret_cast<ani_object>(nullRef);
    }
    if ((status = env->Object_CallMethod_Void(call, method, error, result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status: %{public}d", status);
        return false;
    }
    return true;
}

bool GetDoubleOrUndefined(ani_env *env, ani_object param, const char *name, ani_double &value)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;

    if  (env == nullptr || param == nullptr || name == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env or param or name");
        return false;
    }

    if ((status = env->Object_GetFieldByName_Ref(param, name, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if ((status = env->Object_CallMethodByName_Double(
        reinterpret_cast<ani_object>(obj), "doubleValue", nullptr, &value)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    return true;
}

bool GetStringOrUndefined(ani_env *env, ani_object param, const char *name, std::string &res)
{
    ani_ref obj = nullptr;
    ani_boolean isUndefined = true;
    ani_status status = ANI_ERROR;
 
    if (env == nullptr || param == nullptr || name == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env or param or name");
        return false;
    }

    if ((status = env->Object_GetFieldByName_Ref(param, name, &obj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if ((status = env->Reference_IsUndefined(obj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "status : %{public}d", status);
        return false;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::ANI, "%{public}s : undefined", name);
        return false;
    }
    if (!GetStdString(env, reinterpret_cast<ani_string>(obj), res)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString failed");
        return false;
    }
    return true;
}

bool WrapArrayString(ani_env *env, ani_object &arrayObj, const std::vector<std::string> &values)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "env null or arrayObj null");
        return false;
    }
    ani_class arrayCls = nullptr;
    ani_method arrayCtor;
    ani_string aniStr = nullptr;
    ani_status status = ANI_ERROR;
    status = env->FindClass(CLASSNAME_ARRAY, &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
        return false;
    }
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
        return false;
    }
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, values.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
        return false;
    }

    for (size_t i = 0; i < values.size(); i++) {
        aniStr = nullptr;
        status = env->String_NewUTF8(values[i].c_str(), values[i].size(), &aniStr);
        if (aniStr == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null aniStr");
            return false;
        }
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
            return false;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", SET_OBJECT_VOID_SIGNATURE, i, aniStr);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
            return false;
        }
    }
    return true;
}

bool UnwrapArrayString(ani_env *env, const ani_object &arrayObj, std::vector<std::string> &stringList)
{
    if (env == nullptr || arrayObj == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "env null or arrayObj null");
        return false;
    }
    stringList.clear();
    ani_size size = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Array_GetLength(reinterpret_cast<ani_array>(arrayObj), &size)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d", status);
        return false;
    }
    ani_ref ref;
    ani_size idx;
    for (idx = 0; idx < size; idx++) {
        if ((status = env->Array_Get_Ref(reinterpret_cast<ani_array_ref>(arrayObj), idx, &ref)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::JSNAPI, "status: %{public}d, index: %{public}zu", status, idx);
            return false;
        }
        if (ref == nullptr) {
            TAG_LOGE(AAFwkTag::JSNAPI, "null ref");
            return false;
        }
        std::string str = "";
        if (!OHOS::AppExecFwk::GetStdString(env, reinterpret_cast<ani_string>(ref), str)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "GetStdString failed, index: %{public}zu", idx);
            return false;
        }
        stringList.push_back(str);
    }
    return true;
}

bool SetProcessInformation(ani_env *env, ani_object object, const AppExecFwk::RunningProcessInfo &processInfo)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return false;
    }
    ani_status status = ANI_OK;
    if ((status = env->Object_SetPropertyByName_Double(object, "pid", processInfo.pid_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "pid failed status:%{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Double(object, "uid", processInfo.uid_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "uid failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Ref(object, "processName",
        AppExecFwk::GetAniString(env, processInfo.processName_));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "processName failed status:%{public}d", status);
        return false;
    }
    ani_object arrayObj = nullptr;
    WrapArrayString(env, arrayObj, processInfo.bundleNames);
    status = env->Object_SetPropertyByName_Ref(object, "bundleNames", arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "bundleNames failed status:%{public}d", status);
        return false;
    }
    ani_enum_item stateItem {};
    AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        "L@ohos/app/ability/appManager/appManager/ProcessState;", processInfo.state_, stateItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "state", stateItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "state failed status:%{public}d", status);
        return false;
    }
    ani_enum_item bundleTypeItem {};
    AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(env,
        "L@ohos/bundle/bundleManager/bundleManager/BundleType;",
        processInfo.bundleType, bundleTypeItem);
    if ((status = env->Object_SetPropertyByName_Ref(object, "bundleType", bundleTypeItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "bundleType failed status:%{public}d", status);
        return false;
    }
    status = env->Object_SetPropertyByName_Ref(object, "appCloneIndex",
        CreateDouble(env, processInfo.appCloneIndex));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "appCloneIndex failed status:%{public}d", status);
        return false;
    }
    return true;
}

ani_object WrapProcessInformation(ani_env *env, const AppExecFwk::RunningProcessInfo &processInfo)
{
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method {};
    ani_object object = nullptr;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return nullptr;
    }
    if ((status = env->FindClass("Lapplication/ProcessInformation/ProcessInformationInner;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null cls");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &object)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Object_New ProcessInformationInner failed status: %{public}d", status);
        return nullptr;
    }
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null object");
        return nullptr;
    }
    if (!SetProcessInformation(env, object, processInfo)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "SetProcessInformation failed");
        return nullptr;
    }
    return object;
}

ani_object CreateRunningProcessInfoArray(ani_env *env, std::vector<AppExecFwk::RunningProcessInfo> infos)
{
    ani_class arrayCls = nullptr;
    ani_status status = ANI_OK;

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return nullptr;
    }

    status = env->FindClass("Lescompat/Array;", &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }

    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }

    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Object_New array status: %{public}d", status);
        return arrayObj;
    }
    ani_size index = 0;
    for (auto &processInfo : infos) {
        ani_object ani_info = WrapProcessInformation(env, processInfo);
        if (ani_info == nullptr) {
            TAG_LOGW(AAFwkTag::JSNAPI, "null ani_info");
            break;
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", "ILstd/core/Object;:V", index, ani_info);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::JSNAPI, "Object_CallMethodByName_Void failed status: %{public}d", status);
            break;
        }
        index++;
    }
    return arrayObj;
}

ani_object CreateEmptyArray(ani_env *env)
{
    ani_status status = ANI_OK;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::JSNAPI, "null env");
        return nullptr;
    }
    ani_class arrayCls = nullptr;
    status = env->FindClass("Lescompat/Array;", &arrayCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method arrayCtor;
    status = env->Class_FindMethod(arrayCls, "<ctor>", ":V", &arrayCtor);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "find ctor failed status: %{public}d", status);
        return nullptr;
    }
    ani_object arrayObj;
    status = env->Object_New(arrayCls, arrayCtor, &arrayObj);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Object_New array failed status: %{public}d", status);
        return nullptr;
    }
    return arrayObj;
}
}  // namespace AppExecFwk
}  // namespace OHOS
