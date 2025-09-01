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

#include "ani_common_child_process_param.h"

#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* CLASSNAME_CHILDPROCESSARGS = "L@ohos/app/ability/ChildProcessArgs/ChildProcessArgsImpl;";
constexpr const char* RECORD_CLASS_NAME = "Lescompat/Record;";
}

void SetFds(ani_env* env, ani_object object, std::map<std::string, int32_t> &fds)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null aniEnv");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_class recordCls = nullptr;
    status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "FindClass failed status: %{public}d", status);
        return;
    }
    ani_method recordGetMethod = nullptr;
    status = env->Class_FindMethod(recordCls, "$_get", "Lstd/core/Object;:Lstd/core/Object;", &recordGetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    ani_method recordSetMethod = nullptr;
    status = env->Class_FindMethod(recordCls, "$_set", "Lstd/core/Object;Lstd/core/Object;:V", &recordSetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Class_FindMethod failed status: %{public}d", status);
        return;
    }
    ani_ref fdsRef = nullptr;
    status = env->Object_CallMethodByName_Ref(object, "<get>fds", ":Lescompat/Record;", &fdsRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Object_CallMethodByName_Ref failed status: %{public}d", status);
        return;
    }
    ani_object fdsObject = static_cast<ani_object>(fdsRef);
    for (auto iter = fds.begin(); iter != fds.end(); ++iter) {
        std::string key = iter->first;
        ani_string aniKey = GetAniString(env, key);
        int32_t value = iter->second;
        ani_double aniValue = ani_double(value);
        status = env->Object_CallMethod_Void(fdsObject, recordSetMethod, aniKey, aniValue);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Object_CallMethod_Void failed status: %{public}d", status);
            return;
        }
    }
}

void GetFds(ani_env* env, ani_string aniKey, ani_object aniValue, std::map<std::string, int32_t> &map)
{
    std::string mapKey = "";
    if (!GetStdString(env, aniKey, mapKey)) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetStdString failed");
        return;
    }
    ani_double value;
    ani_status status = env->Object_CallMethodByName_Double(aniValue, "unboxed", nullptr, &value);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Object_CallMethodByName_Double failed status: %{public}d", status);
        return;
    }
    auto mapValue = static_cast<double>(value);
    map.emplace(mapKey, mapValue);
}

bool UnwrapChildProcessArgs(ani_env* env, ani_object object, ChildProcessArgs &args,
    std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return false;
    }
    if (!GetStringOrUndefined(env, object, "entryParams", args.entryParams)) {
        TAG_LOGI(AAFwkTag::PROCESSMGR, "parameter error");
        errorMsg = "Parameter error. The type of args.entryParams must be string.";
        return false;
    }
    ani_ref fdsRef;
    if (GetRefProperty(env, object, "fds", fdsRef)) {
        ani_object fdsObject = static_cast<ani_object>(fdsRef);
        if (fdsObject == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "fds must be Record<string, number>");
            errorMsg = "The type of args.fds must be Record<string, number>.";
            return false;
        }
        if (!UnwrapChildProcessFds(env, fdsObject, args.fds, errorMsg)) {
            return false;
        }
    }
    return true;
}

bool UnwrapChildProcessFds(ani_env* env, ani_object object, std::map<std::string, int32_t> &map, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return false;
    }
    ani_ref iter;
    ani_status status = ANI_ERROR;
    status = env->Object_CallMethodByName_Ref(object, "$_iterator", nullptr, &iter);
    if (ANI_OK != status) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get keys iterator status: %{public}d", status);
        return false;
    }
    ani_ref next;
    ani_boolean done;
    while (ANI_OK == env->Object_CallMethodByName_Ref(static_cast<ani_object>(iter), "next", nullptr, &next)) {
        status = env->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), "done", &done);
        if (ANI_OK != status) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to check iterator done status: %{public}d", status);
            return false;
        }
        if (done) {
            TAG_LOGD(AAFwkTag::PROCESSMGR, "[forEachMapEntry] done break");
            return true;
        }
        ani_ref keyValue;
        status = env->Object_GetFieldByName_Ref(static_cast<ani_object>(next), "value", &keyValue);
        if (ANI_OK != status) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get key value status: %{public}d", status);
            return false;
        }
        ani_ref aniKey;
        status = env->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>(keyValue), 0, &aniKey);
        if (ANI_OK != status) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get key Item status: %{public}d", status);
            return false;
        }
        ani_ref aniVal;
        status = env->TupleValue_GetItem_Ref(static_cast<ani_tuple_value>(keyValue), 1, &aniVal);
        if (ANI_OK != status) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to get key Item status: %{public}d", status);
            return false;
        }
        GetFds(env, static_cast<ani_string>(aniKey), static_cast<ani_object>(aniVal), map);
    }
    return true;
}

ani_object WrapChildProcessArgs(ani_env* env, ChildProcessArgs &args)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class className = nullptr;
    status = env->FindClass(CLASSNAME_CHILDPROCESSARGS, &className);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(className, "<ctor>", ":V", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "find method failed status: %{public}d", status);
        return nullptr;
    }
    ani_object object = nullptr;
    status = env->Object_New(className, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "new object failed status: %{public}d", status);
        return nullptr;
    }
    status = env->Object_SetPropertyByName_Ref(object, "entryParams", GetAniString(env, args.entryParams));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "set property failed status: %{public}d", status);
        return nullptr;
    }
    SetFds(env, object, args.fds);
    return object;
}

bool UnwrapChildProcessOptions(ani_env* env, ani_object object, ChildProcessOptions &options, std::string &errorMsg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "env null");
        return false;
    }
    ani_ref propRef = nullptr;
    ani_status status = env->Object_GetPropertyByName_Ref(object, "isolationMode", &propRef);
    if (status != ANI_OK || propRef == nullptr) {
        options.isolationMode = false;
        return true;
    }
    ani_boolean isUndefined = false;
    status = env->Reference_IsUndefined(propRef, &isUndefined);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Failed to check undefined for 'isolationMode', status: %{public}d", status);
        return false;
    }
    if (isUndefined) {
        options.isolationMode = false;
        return true;
    }
    ani_boolean flag = false;
    status = env->Object_CallMethodByName_Boolean(reinterpret_cast<ani_object>(propRef), "valueOf", ":Z", &flag);
    if (status != ANI_OK) {
        errorMsg = "Failed to get boolean value for isolationMode.";
        return false;
    }
    options.isolationMode = flag;
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS