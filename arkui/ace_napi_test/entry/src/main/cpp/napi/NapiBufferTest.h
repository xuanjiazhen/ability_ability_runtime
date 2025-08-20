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

#ifndef NAPI_BUFFER_TEST_H
#define NAPI_BUFFER_TEST_H

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

napi_value CreateBufferCopy(napi_env env, napi_callback_info info);

#endif // NAPI_BUFFER_TEST_H