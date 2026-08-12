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

#ifndef ACE_NAPI_HOOK_SYMBOL_TABLE_H
#define ACE_NAPI_HOOK_SYMBOL_TABLE_H

#include <functional>
#include <cstdint>
#include <uv.h>
#include "napi_ohos_compat.h"

namespace ace_napi_hook {

using CangjieSymbolResolver = void* (*)(const char* symbol_name);

#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    using api##_fn = ret (*) sig; \
    extern api##_fn orig_##api; \
    extern api##_fn cj_##api;
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    using api##_fn = ret (*) sig; \
    extern api##_fn orig_##api; \
    extern api##_fn cj_##api;
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API

using napi_fatal_error_fn = void (*)(const char* location,
                                     size_t location_len,
                                     const char* message,
                                     size_t message_len);
extern napi_fatal_error_fn orig_napi_fatal_error;
using napi_send_event_fn = napi_status (*)(napi_env env,
                                           const std::function<void()>& cb,
                                           int prior,
                                           const char* name,
                                           int barrier_option);
extern napi_send_event_fn orig_napi_send_event;

using napi_remove_async_cleanup_hook_fn =
    napi_status (*)(napi_async_cleanup_hook_handle remove_handle);
extern napi_remove_async_cleanup_hook_fn orig_napi_remove_async_cleanup_hook;
extern napi_remove_async_cleanup_hook_fn cj_napi_remove_async_cleanup_hook;

using napi_wrap_s_fn = napi_status (*)(napi_env env, napi_value js_object,
                                      void* native_object, napi_finalize finalize_cb,
                                      void* finalize_hint, const napi_type_tag* type_tag,
                                      napi_ref* result);
extern napi_wrap_s_fn orig_napi_wrap_s;
extern napi_wrap_s_fn cj_napi_wrap_s;

using napi_unwrap_s_fn = napi_status (*)(napi_env env, napi_value js_object,
                                        const napi_type_tag* type_tag, void** result);
extern napi_unwrap_s_fn orig_napi_unwrap_s;
extern napi_unwrap_s_fn cj_napi_unwrap_s;

using napi_wrap_enhance_s_fn = napi_status (*)(napi_env env, napi_value js_object,
                                                void* native_object, napi_finalize finalize_cb,
                                                bool async_finalizer, void* finalize_hint,
                                                size_t native_binding_size,
                                                const napi_type_tag* type_tag, napi_ref* result);
extern napi_wrap_enhance_s_fn orig_napi_wrap_enhance_s;
extern napi_wrap_enhance_s_fn cj_napi_wrap_enhance_s;

using napi_wrap_enhance_fn = napi_status (*)(napi_env env, napi_value js_object,
                                              void* native_object, napi_finalize finalize_cb,
                                              bool async_finalizer, void* finalize_hint,
                                              size_t native_binding_size, napi_ref* result);
extern napi_wrap_enhance_fn orig_napi_wrap_enhance;
extern napi_wrap_enhance_fn cj_napi_wrap_enhance;

using uv_async_init_fn = int (*)(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb);
extern uv_async_init_fn orig_uv_async_init;
extern uv_async_init_fn cj_uv_async_init;

using uv_async_send_fn = int (*)(uv_async_t* async);
extern uv_async_send_fn orig_uv_async_send;
extern uv_async_send_fn cj_uv_async_send;

using uv_timer_init_fn = int (*)(uv_loop_t* loop, uv_timer_t* timer);
extern uv_timer_init_fn orig_uv_timer_init;
extern uv_timer_init_fn cj_uv_timer_init;

using uv_timer_start_fn = int (*)(uv_timer_t* timer, uv_timer_cb cb, uint64_t timeout, uint64_t repeat);
extern uv_timer_start_fn orig_uv_timer_start;
extern uv_timer_start_fn cj_uv_timer_start;

using uv_timer_stop_fn = int (*)(uv_timer_t* timer);
extern uv_timer_stop_fn orig_uv_timer_stop;
extern uv_timer_stop_fn cj_uv_timer_stop;

using uv_queue_work_fn = int (*)(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb);
extern uv_queue_work_fn orig_uv_queue_work;
extern uv_queue_work_fn cj_uv_queue_work;

using uv_queue_work_with_qos_fn =
    int (*)(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb, int qos);
extern uv_queue_work_with_qos_fn orig_uv_queue_work_with_qos;
extern uv_queue_work_with_qos_fn cj_uv_queue_work_with_qos;

using uv_close_fn = void (*)(uv_handle_t* handle, uv_close_cb close_cb);
extern uv_close_fn orig_uv_close;
extern uv_close_fn cj_uv_close;

using napi_begin_native_async_callback_fn = napi_status (*)(napi_env env, uint64_t* scope_id);
extern napi_begin_native_async_callback_fn cj_napi_begin_native_async_callback;

using napi_end_native_async_callback_fn = napi_status (*)(napi_env env, uint64_t scope_id);
extern napi_end_native_async_callback_fn cj_napi_end_native_async_callback;

using napi_dispatch_threadsafe_function_fn = napi_status (*)(napi_threadsafe_function func, void* data);
extern napi_dispatch_threadsafe_function_fn cj_napi_dispatch_threadsafe_function;

bool RegisterCangjieSymbolResolver(CangjieSymbolResolver resolver);
void* ResolveCangjieSymbol(const char* symbol_name);
bool InitNapiHookSymbols();

}  // namespace ace_napi_hook

#endif
