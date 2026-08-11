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

#include "symbol_table.h"

#include <atomic>
#include <cstdlib>
#include <dlfcn.h>
#include <mutex>

namespace ace_napi_hook {

#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    api##_fn orig_##api = nullptr; \
    api##_fn cj_##api = nullptr;
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    api##_fn orig_##api = nullptr; \
    api##_fn cj_##api = nullptr;
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API

napi_fatal_error_fn orig_napi_fatal_error = nullptr;
napi_send_event_fn orig_napi_send_event = nullptr;
napi_remove_async_cleanup_hook_fn orig_napi_remove_async_cleanup_hook = nullptr;
napi_remove_async_cleanup_hook_fn cj_napi_remove_async_cleanup_hook = nullptr;
napi_wrap_s_fn orig_napi_wrap_s = nullptr;
napi_wrap_s_fn cj_napi_wrap_s = nullptr;
napi_unwrap_s_fn orig_napi_unwrap_s = nullptr;
napi_unwrap_s_fn cj_napi_unwrap_s = nullptr;
napi_wrap_enhance_s_fn orig_napi_wrap_enhance_s = nullptr;
napi_wrap_enhance_s_fn cj_napi_wrap_enhance_s = nullptr;
napi_wrap_enhance_fn orig_napi_wrap_enhance = nullptr;
napi_wrap_enhance_fn cj_napi_wrap_enhance = nullptr;
uv_async_init_fn orig_uv_async_init = nullptr;
uv_async_init_fn cj_uv_async_init = nullptr;
uv_async_send_fn orig_uv_async_send = nullptr;
uv_async_send_fn cj_uv_async_send = nullptr;
uv_timer_init_fn orig_uv_timer_init = nullptr;
uv_timer_init_fn cj_uv_timer_init = nullptr;
uv_timer_start_fn orig_uv_timer_start = nullptr;
uv_timer_start_fn cj_uv_timer_start = nullptr;
uv_timer_stop_fn orig_uv_timer_stop = nullptr;
uv_timer_stop_fn cj_uv_timer_stop = nullptr;
uv_queue_work_fn orig_uv_queue_work = nullptr;
uv_queue_work_fn cj_uv_queue_work = nullptr;
uv_queue_work_with_qos_fn orig_uv_queue_work_with_qos = nullptr;
uv_queue_work_with_qos_fn cj_uv_queue_work_with_qos = nullptr;
uv_close_fn orig_uv_close = nullptr;
uv_close_fn cj_uv_close = nullptr;

namespace {

std::atomic<bool> g_symbols_initialized {false};

void* GetAceNapiHandle()
{
    static void* handle = []() {
        void* result = dlopen("libace_napi.z.so", RTLD_NOW | RTLD_LOCAL);
        if (result == nullptr) {
            result = dlopen("libace_napi.so", RTLD_NOW | RTLD_LOCAL);
        }
        return result;
    }();
    return handle;
}

void* ResolveSymbol(const char* symbol)
{
    void* resolved = dlsym(nullptr, symbol);
    if (resolved != nullptr) {
        return resolved;
    }
    auto* ace_napi = GetAceNapiHandle();
    return ace_napi == nullptr ? nullptr : dlsym(ace_napi, symbol);
}

void ResolveAllSymbols()
{
#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    orig_##api = reinterpret_cast<api##_fn>(ResolveSymbol(#api)); \
    cj_##api = reinterpret_cast<api##_fn>(ResolveSymbol(#cj_api));
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    orig_##api = reinterpret_cast<api##_fn>(ResolveSymbol(#api)); \
    cj_##api = reinterpret_cast<api##_fn>(ResolveSymbol(#cj_api));
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API
    orig_napi_wrap_with_size = reinterpret_cast<napi_wrap_with_size_fn>(ResolveSymbol(
        "_Z19napi_wrap_with_sizeP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PP10napi_ref__m"));
    orig_napi_fatal_error =
        reinterpret_cast<napi_fatal_error_fn>(ResolveSymbol("napi_fatal_error"));
    orig_napi_send_event =
        reinterpret_cast<napi_send_event_fn>(ResolveSymbol(
            "_Z15napi_send_eventP10napi_env__RKNSt3__h8functionIFvvEEE19napi_event_priorityPKc25napi_event_barrier_option"));
    orig_napi_remove_async_cleanup_hook = reinterpret_cast<napi_remove_async_cleanup_hook_fn>(
        ResolveSymbol("napi_remove_async_cleanup_hook"));
    cj_napi_remove_async_cleanup_hook = reinterpret_cast<napi_remove_async_cleanup_hook_fn>(
        ResolveSymbol("cangjie_napi_remove_async_cleanup_hook"));
    orig_napi_wrap_s = reinterpret_cast<napi_wrap_s_fn>(ResolveSymbol(
        "_Z11napi_wrap_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PK13napi_type_tagPP10napi_ref__"));
    cj_napi_wrap_s = reinterpret_cast<napi_wrap_s_fn>(ResolveSymbol("cangjie_napi_wrap_s"));
    orig_napi_unwrap_s = reinterpret_cast<napi_unwrap_s_fn>(ResolveSymbol(
        "_Z13napi_unwrap_sP10napi_env__P12napi_value__PK13napi_type_tagPPv"));
    cj_napi_unwrap_s = reinterpret_cast<napi_unwrap_s_fn>(ResolveSymbol("cangjie_napi_unwrap_s"));
    orig_napi_wrap_enhance_s = reinterpret_cast<napi_wrap_enhance_s_fn>(ResolveSymbol(
        "_Z19napi_wrap_enhance_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_EbS3_mPK13napi_type_tagPP10napi_ref__"));
    cj_napi_wrap_enhance_s = reinterpret_cast<napi_wrap_enhance_s_fn>(
        ResolveSymbol("cangjie_napi_wrap_enhance_s"));
    orig_napi_wrap_enhance = reinterpret_cast<napi_wrap_enhance_fn>(
        ResolveSymbol("napi_wrap_enhance"));
    cj_napi_wrap_enhance = reinterpret_cast<napi_wrap_enhance_fn>(
        ResolveSymbol("cangjie_napi_wrap_enhance"));
    orig_uv_async_init = reinterpret_cast<uv_async_init_fn>(ResolveSymbol("uv_async_init"));
    cj_uv_async_init = reinterpret_cast<uv_async_init_fn>(ResolveSymbol("cangjie_uv_async_init"));
    orig_uv_async_send = reinterpret_cast<uv_async_send_fn>(ResolveSymbol("uv_async_send"));
    cj_uv_async_send = reinterpret_cast<uv_async_send_fn>(ResolveSymbol("cangjie_uv_async_send"));
    orig_uv_timer_init = reinterpret_cast<uv_timer_init_fn>(ResolveSymbol("uv_timer_init"));
    cj_uv_timer_init = reinterpret_cast<uv_timer_init_fn>(ResolveSymbol("cangjie_uv_timer_init"));
    orig_uv_timer_start = reinterpret_cast<uv_timer_start_fn>(ResolveSymbol("uv_timer_start"));
    cj_uv_timer_start = reinterpret_cast<uv_timer_start_fn>(ResolveSymbol("cangjie_uv_timer_start"));
    orig_uv_timer_stop = reinterpret_cast<uv_timer_stop_fn>(ResolveSymbol("uv_timer_stop"));
    cj_uv_timer_stop = reinterpret_cast<uv_timer_stop_fn>(ResolveSymbol("cangjie_uv_timer_stop"));
    orig_uv_queue_work = reinterpret_cast<uv_queue_work_fn>(ResolveSymbol("uv_queue_work"));
    cj_uv_queue_work = reinterpret_cast<uv_queue_work_fn>(ResolveSymbol("cangjie_uv_queue_work"));
    orig_uv_queue_work_with_qos =
        reinterpret_cast<uv_queue_work_with_qos_fn>(ResolveSymbol("uv_queue_work_with_qos"));
    cj_uv_queue_work_with_qos =
        reinterpret_cast<uv_queue_work_with_qos_fn>(ResolveSymbol("cangjie_uv_queue_work_with_qos"));
    orig_uv_close = reinterpret_cast<uv_close_fn>(ResolveSymbol("uv_close"));
    cj_uv_close = reinterpret_cast<uv_close_fn>(ResolveSymbol("cangjie_uv_close"));
}

}  // namespace

bool InitNapiHookSymbols()
{
    static std::once_flag once;
    std::call_once(once, []() {
        ResolveAllSymbols();
        g_symbols_initialized.store(true, std::memory_order_release);
    });
    return g_symbols_initialized.load(std::memory_order_acquire);
}

}  // namespace ace_napi_hook

