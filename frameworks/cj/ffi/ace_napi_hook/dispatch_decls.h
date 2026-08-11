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

#ifndef ACE_NAPI_HOOK_DISPATCH_DECLS_H
#define ACE_NAPI_HOOK_DISPATCH_DECLS_H

#include <functional>
#include <uv.h>
#include "napi_ohos_compat.h"

#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) extern "C" ret dispatch_##api sig;
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) extern "C" ret dispatch_##api sig;
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API

extern "C" void dispatch_napi_fatal_error(const char* location,
                                          size_t location_len,
                                          const char* message,
                                          size_t message_len);
extern "C" napi_status dispatch_napi_send_event(napi_env env,
                                                const std::function<void()>& cb,
                                                int prior,
                                                const char* name,
                                                int barrier_option);
extern "C" napi_status dispatch_napi_remove_async_cleanup_hook(
    napi_async_cleanup_hook_handle remove_handle);
extern "C" napi_status dispatch_napi_wrap_s(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    void* finalize_hint, const napi_type_tag* type_tag, napi_ref* result);
extern "C" napi_status dispatch_napi_unwrap_s(
    napi_env env, napi_value js_object, const napi_type_tag* type_tag, void** result);
extern "C" napi_status dispatch_napi_wrap_enhance_s(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    bool async_finalizer, void* finalize_hint, size_t native_binding_size,
    const napi_type_tag* type_tag, napi_ref* result);
extern "C" napi_status dispatch_napi_wrap_enhance(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    bool async_finalizer, void* finalize_hint, size_t native_binding_size, napi_ref* result);
extern "C" int dispatch_uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb);
extern "C" int dispatch_uv_async_send(uv_async_t* async);
extern "C" int dispatch_uv_timer_init(uv_loop_t* loop, uv_timer_t* timer);
extern "C" int dispatch_uv_timer_start(uv_timer_t* timer, uv_timer_cb cb, uint64_t timeout, uint64_t repeat);
extern "C" int dispatch_uv_timer_stop(uv_timer_t* timer);
extern "C" int dispatch_uv_queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb);
extern "C" int dispatch_uv_queue_work_with_qos(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb,
                                               uv_after_work_cb after_work_cb, int qos);
extern "C" void dispatch_uv_close(uv_handle_t* handle, uv_close_cb close_cb);

#endif

