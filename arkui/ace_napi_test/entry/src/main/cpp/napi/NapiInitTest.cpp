/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "common/native_common.h"
#include "napi/native_api.h"

#include <pthread.h>
#include <cstdint>
#include <string>
#include <cstdio>
#include <vector>
#include <malloc.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <unistd.h>
#include <memory.h>
#include <uv.h>

#include "NapiInit.h"
#include "NapiBufferTest.h"

EXTERN_C_START

const int TAG_NUMBER = 666;

const char TEST_STR[TEST_STR_LEN] =
    "Where there is a will, there is a way.";

napi_property_descriptor BUFFER_PROPERTY_LIST[] = {
    DECLARE_NAPI_FUNCTION("CreateBufferCopy", CreateBufferCopy)
};
const size_t BUFFER_PROPERTY_COUNT = sizeof(BUFFER_PROPERTY_LIST) / sizeof(BUFFER_PROPERTY_LIST[0]);

static napi_value SayHello(napi_env env, napi_callback_info info)
{
    printf("Hello\n");
    napi_value ret;
    NAPI_CALL(env, napi_create_int32(env, TAG_NUMBER, &ret));
    return ret;
}

static napi_value MyConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = 0;
    napi_value constructor = nullptr;
    napi_get_cb_info(env, info, &argc, nullptr, &thisVar, nullptr);
    napi_get_new_target(env, info, &constructor);
    if (constructor == nullptr) {
        napi_throw_error(env, nullptr, "is not new instance");
    }
    return thisVar;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_value theValue;
    
    NAPI_CALL(env, napi_create_string_utf8(env, TEST_STR, sizeof(TEST_STR), &theValue));
    NAPI_CALL(env, napi_set_named_property(env, exports, "testStr", theValue));
    
    NAPI_CALL(env, napi_define_properties(env, exports, PROPERTY_LIST_COUNT, PROPERTY_LIST));
    NAPI_CALL(env, napi_define_properties(env, exports, BUFFER_PROPERTY_COUNT, BUFFER_PROPERTY_LIST));

    napi_property_descriptor descClass[] = {
        DECLARE_NAPI_FUNCTION("sayHello", SayHello),
    };
    napi_value myClass;
    napi_define_class(env, "myClass", NAPI_AUTO_LENGTH, MyConstructor, nullptr,
                      sizeof(descClass) / sizeof(descClass[0]), descClass, &myClass);
    napi_set_named_property(env, exports, "myClass", myClass);
    return exports;
}

EXTERN_C_END