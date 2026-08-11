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

#include "ace_napi_hook_init.h"

#include <cstdlib>
#include <memory>
#include <mutex>

#include <hilog/log.h>

#include "dispatch_decls.h"
#include "hook_backend.h"
#include "symbol_table.h"

namespace ace_napi_hook {

namespace {

constexpr uint32_t HOOK_LOG_DOMAIN = 0xD001150;
constexpr const char* HOOK_LOG_TAG = "AbilityAceNapiHook";

struct HookInitState {
    DispatchMethods methods;
    std::once_flag init_once;
    std::unique_ptr<NapiHookBackend> backend;
    bool init_ok {false};
};

HookInitState& GetHookInitState()
{
    static HookInitState state;
    return state;
}

extern "C" int64_t dispatch_ffrt_this_task_get_id();

void AppendUvDispatchMethods(std::vector<std::pair<std::string, void*>>& methods)
{
    methods.emplace_back("uv_async_init",
                         reinterpret_cast<void*>(&dispatch_uv_async_init));
    methods.emplace_back("uv_async_send",
                         reinterpret_cast<void*>(&dispatch_uv_async_send));
    methods.emplace_back("uv_timer_init",
                         reinterpret_cast<void*>(&dispatch_uv_timer_init));
    methods.emplace_back("uv_timer_start",
                         reinterpret_cast<void*>(&dispatch_uv_timer_start));
    methods.emplace_back("uv_timer_stop",
                         reinterpret_cast<void*>(&dispatch_uv_timer_stop));
    methods.emplace_back("uv_queue_work",
                         reinterpret_cast<void*>(&dispatch_uv_queue_work));
    methods.emplace_back("uv_queue_work_with_qos",
                         reinterpret_cast<void*>(&dispatch_uv_queue_work_with_qos));
    methods.emplace_back("uv_close",
                         reinterpret_cast<void*>(&dispatch_uv_close));
    methods.emplace_back("ffrt_this_task_get_id",
                         reinterpret_cast<void*>(&dispatch_ffrt_this_task_get_id));
}

std::vector<std::pair<std::string, void*>> BuildDispatchMethodsImpl()
{
    std::vector<std::pair<std::string, void*>> methods;
#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    methods.emplace_back(#api, reinterpret_cast<void*>(&dispatch_##api));
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    methods.emplace_back(#api, reinterpret_cast<void*>(&dispatch_##api));
#include "api_list.inc"
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API
    methods.emplace_back(
        "_Z19napi_wrap_with_sizeP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PP10napi_ref__m",
        reinterpret_cast<void*>(&dispatch_napi_wrap_with_size));
    methods.emplace_back(
        "_Z11napi_wrap_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_ES3_PK13napi_type_tagPP10napi_ref__",
        reinterpret_cast<void*>(&dispatch_napi_wrap_s));
    methods.emplace_back(
        "_Z13napi_unwrap_sP10napi_env__P12napi_value__PK13napi_type_tagPPv",
        reinterpret_cast<void*>(&dispatch_napi_unwrap_s));
    methods.emplace_back(
        "_Z19napi_wrap_enhance_sP10napi_env__P12napi_value__PvPFvS0_S3_S3_EbS3_mPK13napi_type_tagPP10napi_ref__",
        reinterpret_cast<void*>(&dispatch_napi_wrap_enhance_s));
    methods.emplace_back("napi_wrap_enhance",
                         reinterpret_cast<void*>(&dispatch_napi_wrap_enhance));
    methods.emplace_back("napi_fatal_error",
                         reinterpret_cast<void*>(&dispatch_napi_fatal_error));
    methods.emplace_back(
        "_Z15napi_send_eventP10napi_env__RKNSt3__h8functionIFvvEEE19napi_event_priorityPKc25napi_event_barrier_option",
                         reinterpret_cast<void*>(&dispatch_napi_send_event));
    methods.emplace_back("napi_remove_async_cleanup_hook",
                         reinterpret_cast<void*>(&dispatch_napi_remove_async_cleanup_hook));
    AppendUvDispatchMethods(methods);
    return methods;
}

}  // namespace

std::vector<std::pair<std::string, void*>> BuildAllDispatchMethods()
{
    return GetHookInitState().methods;
}

bool InstallAllNapiHooks()
{
    return InitAceNapiHook();
}

bool InitAceNapiHook()
{
    auto& state = GetHookInitState();
    std::call_once(state.init_once, [&state]() {
        if (!InitNapiHookSymbols()) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "hook initialization failed: N-API dispatch symbols could not be resolved");
            state.init_ok = false;
            return;
        }
        state.methods = BuildDispatchMethodsImpl();
        state.backend = TryCreateNamespaceBackend(state.methods);
        if (state.backend == nullptr) {
            state.backend = CreateXHookBackend(state.methods);
        }
        state.init_ok = state.backend != nullptr;
        if (!state.init_ok) {
            HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
                "hook initialization failed: no backend could be initialized");
            return;
        }
        HiLogPrint(LOG_APP, LOG_INFO, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "hook backend selected: %{public}s", state.backend->Name());
    });
    return state.init_ok;
}

bool RefreshAceNapiHook(const char* so_path)
{
    return RefreshAceNapiHooks(so_path, nullptr, 0);
}

bool RefreshAceNapiHooks(const char* primary_path, const char* const* additional_paths,
                         size_t additional_count)
{
    if (!InitAceNapiHook()) {
        return false;
    }
    auto& state = GetHookInitState();
    return state.backend->RefreshLibraries(
        primary_path, additional_paths, additional_count);
}

int32_t RequireSystemModule(const char* module_name,
    const char* const* additional_dep_libs, size_t additional_dep_count)
{
    if (!InitAceNapiHook()) {
        HiLogPrint(LOG_APP, LOG_ERROR, HOOK_LOG_DOMAIN, HOOK_LOG_TAG,
            "system module load failed: hook initialization failed");
        return -1;
    }

    auto& state = GetHookInitState();
    return state.backend->RequireSystemModule(
        module_name, additional_dep_libs, additional_dep_count);
}

bool FindNativeModule(const char* module_name, NativeModule* result)
{
    if (result == nullptr) {
        return false;
    }
    *result = NativeModule {};
    if (!InitAceNapiHook()) {
        return false;
    }
    return GetHookInitState().backend->FindModule(module_name, result);
}

}  // namespace ace_napi_hook

extern "C" void ACE_NAPI_HOOK_InitOrAbort()
{
    if (!ace_napi_hook::InitAceNapiHook()) {
        std::abort();
    }
}

extern "C" void ACE_NAPI_HOOK_RefreshOrAbort(const char* so_path)
{
    if (!ace_napi_hook::RefreshAceNapiHook(so_path)) {
        std::abort();
    }
}

extern "C" void ACE_NAPI_HOOK_RefreshManyOrAbort(const char* primary_path,
                                                  const char* const* additional_paths,
                                                  size_t additional_count)
{
    if (!ace_napi_hook::RefreshAceNapiHooks(primary_path, additional_paths,
                                             additional_count)) {
        std::abort();
    }
}

extern "C" int32_t ACE_NAPI_HOOK_RequireSystemModule(const char* module_name,
    const char* const* additional_dep_libs, size_t additional_dep_count)
{
    return ace_napi_hook::RequireSystemModule(
        module_name, additional_dep_libs, additional_dep_count);
}

extern "C" int ace_napi_hook_module_find(const char* name, NativeModule* result)
{
    return ace_napi_hook::FindNativeModule(name, result) ? 1 : 0;
}
