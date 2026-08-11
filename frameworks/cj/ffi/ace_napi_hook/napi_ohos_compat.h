/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ACE_NAPI_HOOK_OHOS_COMPAT_H
#define ACE_NAPI_HOOK_OHOS_COMPAT_H

#ifndef NAPI_VERSION
#define NAPI_VERSION 9
#endif

#include <node_api_types.h>
typedef napi_env node_api_basic_env;
typedef void (*napi_cleanup_hook)(void* arg);
#include <node_api.h>

typedef void* (*napi_native_binding_detach_callback)(napi_env env, void* native_object, void* hint);
typedef napi_value (*napi_native_binding_attach_callback)(napi_env env, void* native_object, void* hint);

#endif

