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

#include <hilog/log.h>

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
napi_begin_native_async_callback_fn cj_napi_begin_native_async_callback = nullptr;
napi_end_native_async_callback_fn cj_napi_end_native_async_callback = nullptr;
napi_dispatch_threadsafe_function_fn cj_napi_dispatch_threadsafe_function = nullptr;

namespace {

constexpr uint32_t HOOK_LOG_DOMAIN = 0xD001150;
constexpr const char* HOOK_LOG_TAG = "AbilityAceNapiHook";

std::atomic<bool> g_symbols_initialized {false};
std::atomic<CangjieSymbolResolver> g_cangjie_symbol_resolver {nullptr};

void* GetPlatformNapiHandle()
{
    static void* handle = dlopen("libace_napi.z.so", RTLD_NOW | RTLD_LOCAL);
    return handle;
}

void* GetPlatformUvHandle()
{
    static void* handle = dlopen("libuv.so", RTLD_NOW | RTLD_LOCAL);
    return handle;
}

void* GetCangjieNapiHandle()
{
    static void* handle = []() -> void* {
#ifdef __OHOS__
        // libace_napi_hook.z.so lives in the platform namespace after being
        // moved out of the application. A plain dlopen/dlsym is therefore no
        // longer able to see libace_napi.so, which is loaded in a Cangjie SDK
        // namespace. Open the library in the namespace that owns it instead.
        constexpr const char* CANGJIE_NAMESPACES[] = {
            "cj_rom_sdk",
            "cj_compatibility_sdk",
            "cj_app_sdk",
        };
        for (const char* namespace_name : CANGJIE_NAMESPACES) {
            Dl_namespace sdk_namespace {};
            if (dlns_get(namespace_name, &sdk_namespace) != 0) {
                continue;
            }
            void* result = dlopen_ns(
                &sdk_namespace, "libace_napi.so", RTLD_NOW | RTLD_LOCAL);
            if (result != nullptr) {
                return result;
            }
        }
#endif
        return dlopen("libace_napi.so", RTLD_NOW | RTLD_LOCAL);
    }();
    return handle;
}

void* ResolveFromHandle(void* handle, const char* symbol_name)
{
    return handle == nullptr ? nullptr : dlsym(handle, symbol_name);
}

void* ResolvePlatformNapiSymbol(const char* symbol_name)
{
    void* resolved = ResolveFromHandle(GetPlatformNapiHandle(), symbol_name);
    return resolved == nullptr ? dlsym(nullptr, symbol_name) : resolved;
}

void* ResolvePlatformUvSymbol(const char* symbol_name)
{
    void* resolved = ResolveFromHandle(GetPlatformUvHandle(), symbol_name);
    return resolved == nullptr ? dlsym(nullptr, symbol_name) : resolved;
}

template<typename Function>
bool CheckResolved(const char* family, const char* symbol_name, Function function)
{
    if (function != nullptr) {
        return true;
    }
    HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
        "N-API dispatch symbol unresolved: family=%{public}s symbol=%{public}s",
        family, symbol_name);
    return false;
}

template<typename Function>
void ReportOptionalUnresolved(const char* family, const char* symbol_name, Function function)
{
    if (function != nullptr) {
        return;
    }
    HiLogPrint(LOG_APP, LOG_WARN, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
        "optional N-API dispatch symbol unresolved: family=%{public}s symbol=%{public}s",
        family, symbol_name);
}

bool ResolveAllSymbols()
{
#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    orig_##api = reinterpret_cast<api##_fn>(ResolvePlatformNapiSymbol(#api)); \
    cj_##api = reinterpret_cast<api##_fn>(ResolveCangjieSymbol(#cj_api));
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    orig_##api = reinterpret_cast<api##_fn>(ResolvePlatformNapiSymbol(#api)); \
    cj_##api = reinterpret_cast<api##_fn>(ResolveCangjieSymbol(#cj_api));
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API
    orig_napi_wrap_with_size = reinterpret_cast<napi_wrap_with_size_fn>(ResolvePlatformNapiSymbol(
        "_Z19napi_wrap_with_sizeP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PP10napi_ref__m"));
    orig_napi_fatal_error =
        reinterpret_cast<napi_fatal_error_fn>(ResolvePlatformNapiSymbol("napi_fatal_error"));
    orig_napi_send_event =
        reinterpret_cast<napi_send_event_fn>(ResolvePlatformNapiSymbol(
            "_Z15napi_send_eventP10napi_env__RKNSt3__h8functionIFvvEEE19napi_event_priorityPKc25napi_event_barrier_option"));
    orig_napi_remove_async_cleanup_hook = reinterpret_cast<napi_remove_async_cleanup_hook_fn>(
        ResolvePlatformNapiSymbol("napi_remove_async_cleanup_hook"));
    cj_napi_remove_async_cleanup_hook = reinterpret_cast<napi_remove_async_cleanup_hook_fn>(
        ResolveCangjieSymbol("cangjie_napi_remove_async_cleanup_hook"));
    orig_napi_wrap_s = reinterpret_cast<napi_wrap_s_fn>(ResolvePlatformNapiSymbol(
        "_Z11napi_wrap_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PK13napi_type_tagPP10napi_ref__"));
    cj_napi_wrap_s = reinterpret_cast<napi_wrap_s_fn>(ResolveCangjieSymbol("cangjie_napi_wrap_s"));
    orig_napi_unwrap_s = reinterpret_cast<napi_unwrap_s_fn>(ResolvePlatformNapiSymbol(
        "_Z13napi_unwrap_sP10napi_env__P12napi_value__PK13napi_type_tagPPv"));
    cj_napi_unwrap_s = reinterpret_cast<napi_unwrap_s_fn>(ResolveCangjieSymbol("cangjie_napi_unwrap_s"));
    orig_napi_wrap_enhance_s = reinterpret_cast<napi_wrap_enhance_s_fn>(ResolvePlatformNapiSymbol(
        "_Z19napi_wrap_enhance_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_EbS3_mPK13napi_type_tagPP10napi_ref__"));
    cj_napi_wrap_enhance_s = reinterpret_cast<napi_wrap_enhance_s_fn>(
        ResolveCangjieSymbol("cangjie_napi_wrap_enhance_s"));
    orig_napi_wrap_enhance = reinterpret_cast<napi_wrap_enhance_fn>(
        ResolvePlatformNapiSymbol("napi_wrap_enhance"));
    cj_napi_wrap_enhance = reinterpret_cast<napi_wrap_enhance_fn>(
        ResolveCangjieSymbol("cangjie_napi_wrap_enhance"));
    orig_uv_async_init = reinterpret_cast<uv_async_init_fn>(ResolvePlatformUvSymbol("uv_async_init"));
    cj_uv_async_init = reinterpret_cast<uv_async_init_fn>(ResolveCangjieSymbol("cangjie_uv_async_init"));
    orig_uv_async_send = reinterpret_cast<uv_async_send_fn>(ResolvePlatformUvSymbol("uv_async_send"));
    cj_uv_async_send = reinterpret_cast<uv_async_send_fn>(ResolveCangjieSymbol("cangjie_uv_async_send"));
    orig_uv_timer_init = reinterpret_cast<uv_timer_init_fn>(ResolvePlatformUvSymbol("uv_timer_init"));
    cj_uv_timer_init = reinterpret_cast<uv_timer_init_fn>(ResolveCangjieSymbol("cangjie_uv_timer_init"));
    orig_uv_timer_start = reinterpret_cast<uv_timer_start_fn>(ResolvePlatformUvSymbol("uv_timer_start"));
    cj_uv_timer_start = reinterpret_cast<uv_timer_start_fn>(ResolveCangjieSymbol("cangjie_uv_timer_start"));
    orig_uv_timer_stop = reinterpret_cast<uv_timer_stop_fn>(ResolvePlatformUvSymbol("uv_timer_stop"));
    cj_uv_timer_stop = reinterpret_cast<uv_timer_stop_fn>(ResolveCangjieSymbol("cangjie_uv_timer_stop"));
    orig_uv_queue_work = reinterpret_cast<uv_queue_work_fn>(ResolvePlatformUvSymbol("uv_queue_work"));
    cj_uv_queue_work = reinterpret_cast<uv_queue_work_fn>(ResolveCangjieSymbol("cangjie_uv_queue_work"));
    orig_uv_queue_work_with_qos =
        reinterpret_cast<uv_queue_work_with_qos_fn>(ResolvePlatformUvSymbol("uv_queue_work_with_qos"));
    cj_uv_queue_work_with_qos =
        reinterpret_cast<uv_queue_work_with_qos_fn>(ResolveCangjieSymbol("cangjie_uv_queue_work_with_qos"));
    orig_uv_close = reinterpret_cast<uv_close_fn>(ResolvePlatformUvSymbol("uv_close"));
    cj_uv_close = reinterpret_cast<uv_close_fn>(ResolveCangjieSymbol("cangjie_uv_close"));
    cj_napi_begin_native_async_callback = reinterpret_cast<napi_begin_native_async_callback_fn>(
        ResolveCangjieSymbol("cangjie_napi_begin_native_async_callback"));
    cj_napi_end_native_async_callback = reinterpret_cast<napi_end_native_async_callback_fn>(
        ResolveCangjieSymbol("cangjie_napi_end_native_async_callback"));
    cj_napi_dispatch_threadsafe_function = reinterpret_cast<napi_dispatch_threadsafe_function_fn>(
        ResolveCangjieSymbol("cangjie_napi_dispatch_threadsafe_function"));

    // Platform N-API and libuv symbols vary between system-image versions.
    // They are only needed when the corresponding platform path is executed,
    // so report them but do not reject hook startup. The Cangjie dispatch table
    // is required because every tagged env call depends on it.
    bool required_resolved = true;
#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    ReportOptionalUnresolved("platform", #api, orig_##api); \
    required_resolved = CheckResolved("Cangjie", #cj_api, cj_##api) && required_resolved;
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    ReportOptionalUnresolved("platform", #api, orig_##api); \
    required_resolved = CheckResolved("Cangjie", #cj_api, cj_##api) && required_resolved;
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API
#define CHECK_PLATFORM(symbol, function) \
    ReportOptionalUnresolved("platform", symbol, function)
#define CHECK_CANGJIE(symbol, function) \
    required_resolved = CheckResolved("Cangjie", symbol, function) && required_resolved
    CHECK_PLATFORM("napi_fatal_error", orig_napi_fatal_error);
    CHECK_PLATFORM("napi_send_event", orig_napi_send_event);
    CHECK_PLATFORM("napi_remove_async_cleanup_hook", orig_napi_remove_async_cleanup_hook);
    CHECK_CANGJIE("cangjie_napi_remove_async_cleanup_hook", cj_napi_remove_async_cleanup_hook);
    CHECK_PLATFORM("napi_wrap_s", orig_napi_wrap_s);
    CHECK_CANGJIE("cangjie_napi_wrap_s", cj_napi_wrap_s);
    CHECK_PLATFORM("napi_unwrap_s", orig_napi_unwrap_s);
    CHECK_CANGJIE("cangjie_napi_unwrap_s", cj_napi_unwrap_s);
    CHECK_PLATFORM("napi_wrap_enhance_s", orig_napi_wrap_enhance_s);
    CHECK_CANGJIE("cangjie_napi_wrap_enhance_s", cj_napi_wrap_enhance_s);
    CHECK_PLATFORM("napi_wrap_enhance", orig_napi_wrap_enhance);
    CHECK_CANGJIE("cangjie_napi_wrap_enhance", cj_napi_wrap_enhance);
    CHECK_PLATFORM("uv_async_init", orig_uv_async_init);
    CHECK_CANGJIE("cangjie_uv_async_init", cj_uv_async_init);
    CHECK_PLATFORM("uv_async_send", orig_uv_async_send);
    CHECK_CANGJIE("cangjie_uv_async_send", cj_uv_async_send);
    CHECK_PLATFORM("uv_timer_init", orig_uv_timer_init);
    CHECK_CANGJIE("cangjie_uv_timer_init", cj_uv_timer_init);
    CHECK_PLATFORM("uv_timer_start", orig_uv_timer_start);
    CHECK_CANGJIE("cangjie_uv_timer_start", cj_uv_timer_start);
    CHECK_PLATFORM("uv_timer_stop", orig_uv_timer_stop);
    CHECK_CANGJIE("cangjie_uv_timer_stop", cj_uv_timer_stop);
    CHECK_PLATFORM("uv_queue_work", orig_uv_queue_work);
    CHECK_CANGJIE("cangjie_uv_queue_work", cj_uv_queue_work);
    CHECK_PLATFORM("uv_queue_work_with_qos", orig_uv_queue_work_with_qos);
    CHECK_CANGJIE("cangjie_uv_queue_work_with_qos", cj_uv_queue_work_with_qos);
    CHECK_PLATFORM("uv_close", orig_uv_close);
    CHECK_CANGJIE("cangjie_uv_close", cj_uv_close);
    CHECK_CANGJIE("cangjie_napi_begin_native_async_callback", cj_napi_begin_native_async_callback);
    CHECK_CANGJIE("cangjie_napi_end_native_async_callback", cj_napi_end_native_async_callback);
    CHECK_CANGJIE("cangjie_napi_dispatch_threadsafe_function", cj_napi_dispatch_threadsafe_function);
#undef CHECK_CANGJIE
#undef CHECK_PLATFORM
    return required_resolved;
}

}  // namespace

bool RegisterCangjieSymbolResolver(CangjieSymbolResolver resolver)
{
    if (resolver == nullptr) {
        return false;
    }
    CangjieSymbolResolver expected = nullptr;
    return g_cangjie_symbol_resolver.compare_exchange_strong(
        expected, resolver, std::memory_order_release, std::memory_order_acquire) || expected == resolver;
}

void* ResolveCangjieSymbol(const char* symbol_name)
{
    void* resolved = ResolveFromHandle(GetCangjieNapiHandle(), symbol_name);
    if (resolved != nullptr) {
        return resolved;
    }

    // Retain the registered resolver as a compatibility fallback for builds
    // that do not expose OpenHarmony namespace APIs.
    auto resolver = g_cangjie_symbol_resolver.load(std::memory_order_acquire);
    if (resolver != nullptr) {
        void* resolver_result = resolver(symbol_name);
        if (resolver_result != nullptr) {
            return resolver_result;
        }
    }
    return dlsym(nullptr, symbol_name);
}

bool InitNapiHookSymbols()
{
    static std::once_flag once;
    std::call_once(once, []() {
        g_symbols_initialized.store(ResolveAllSymbols(), std::memory_order_release);
    });
    return g_symbols_initialized.load(std::memory_order_acquire);
}

}  // namespace ace_napi_hook
