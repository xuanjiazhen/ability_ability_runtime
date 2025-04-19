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
 
#include "ani_common_connect_options.h"
 
#include "hilog_tag_wrapper.h"
 
namespace OHOS {
namespace AppExecFwk {
 
static ani_object CreateConnectOptions(ani_env* env,
    ani_fn_object onConnect, ani_fn_object onDisconnect, ani_fn_object onFiled) {
    TAG_LOGI(AAFwkTag::JSNAPI, "called");
    const char* className = "Lability/connectOptions/ConnectOptionsInner;";
    ani_class cls;
    env->FindClass(className, &cls);
 
    ani_method ctor;
    env->Class_FindMethod(cls, "<ctor>", "V", &ctor);
 
    ani_object obj;
    env->Object_New(cls, ctor, &obj);
 
    env->Object_SetFieldByName_Ref(obj, "onConnect", onConnect);
    env->Object_SetFieldByName_Ref(obj, "onDisconnect", onDisconnect);
    env->Object_SetFieldByName_Ref(obj, "onFiled", onFiled);
    return obj;
}
 
static void TriggerOnConnect(ani_env* env, ani_object options, ani_object elementName, ani_object remote) {
    ani_ref callbackRef;
    env->Object_GetFieldByName_Ref(options, "onConnect", &callbackRef);
 
    if (callbackRef) {
        std::vector<ani_ref> emptyArgs;
        ani_ref dummyResult;
        env->FunctionObject_call(static_cast<ani_fn_object>(callbackRef),
            0, emptyArgs.data(), &dummyResult);
    }
}
 
static void TriggerOnDisconnect(ani_env* env, ani_object options, ani_object elementName) {
    ani_ref callbackRef;
    env->Object_GetFieldByName_Ref(options, "onDisconnect", &callbackRef);
 
    if (callbackRef) {
        std::vector<ani_ref> args = { elementName };
        ani_ref dummyResult;
        env->FunctionObject_call(static_cast<ani_fn_object>(callbackRef),
            0, args.data(), &dummyResult);
    }
}
 
static void TriggerOnFailed(ani_env* env, ani_object options, ani_int code) {
    ani_ref callbackRef;
    env->Object_GetFieldByName_Ref(options, "onFailed", &callbackRef);
 
    if (callbackRef) {
        std::vector<ani_ref> args = { code };
        ani_ref dummyResult;
        env->FunctionObject_call(static_cast<ani_fn_object>(callbackRef),
            0, args.data(), &dummyResult);
    }
}
}
}