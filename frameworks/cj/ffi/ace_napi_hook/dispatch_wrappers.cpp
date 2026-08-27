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

#include "dispatch_common.h"
#include "dispatch_decls.h"
#include "fast_values.h"
#include "native_reference_abi.h"
#include "symbol_table.h"
#include "../cj_macro.h"

#include <functional>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <memory>
#include <mutex>
#include <new>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

extern "C" int64_t MRT_GetCurrentThreadID(void) __attribute__((weak));
extern "C" unsigned long long int CJ_CJThreadId(void) __attribute__((weak));

namespace ace_napi_hook {

namespace {

constexpr uintptr_t UV_TAG_MASK = 0xFFull << 56;
constexpr uintptr_t UV_TAG_CUSTOM = 2ull;
constexpr uintptr_t REFERENCE_TAG = UV_TAG_CUSTOM << 56;
constexpr uint64_t COMPATIBLE_REFERENCE_MAGIC = 0x434a4e4150495246ull;

// System modules are built together with the platform N-API runtime and some
// of them treat napi_ref as NativeReference* despite its public opaque type.
// Keep the platform virtual layout at the front and store the real Cangjie
// reference in the tail. The returned pointer carries the same top-byte tag as
// other Cangjie handles; AArch64 TBI permits platform code to dereference it,
// while the class-specific delete removes the tag before releasing storage.
class CompatibleNativeReference final : public NativeReference {
public:
    CompatibleNativeReference(napi_env env, napi_ref ref, uint32_t initial_refcount)
        : cj_env_(env), cj_ref_(ref), ref_count_(initial_refcount)
    {
    }

    ~CompatibleNativeReference() override
    {
        magic_ = 0;
        auto ref = cj_ref_;
        cj_ref_ = nullptr;
        if (ref != nullptr && cj_napi_delete_reference != nullptr) {
            (void)cj_napi_delete_reference(cj_env_, ref);
        }
    }

    uint32_t Ref() override
    {
        uint32_t count = ref_count_.load(std::memory_order_relaxed);
        if (cj_napi_reference_ref != nullptr &&
            cj_napi_reference_ref(cj_env_, cj_ref_, &count) == napi_ok) {
            ref_count_.store(count, std::memory_order_relaxed);
        }
        return count;
    }

    uint32_t Unref() override
    {
        uint32_t count = ref_count_.load(std::memory_order_relaxed);
        if (cj_napi_reference_unref != nullptr &&
            cj_napi_reference_unref(cj_env_, cj_ref_, &count) == napi_ok) {
            ref_count_.store(count, std::memory_order_relaxed);
        }
        return count;
    }

    napi_value Get(NativeEngine*) override
    {
        return Get();
    }

    napi_value Get() override
    {
        return GetForEnv(cj_env_);
    }

    napi_value GetForEnv(napi_env cj_env)
    {
        napi_value value = nullptr;
        if (cj_napi_get_reference_value != nullptr) {
            (void)cj_napi_get_reference_value(cj_env, cj_ref_, &value);
        }
        return value;
    }

    void* GetData() override
    {
        return nullptr;
    }

    operator napi_value() override
    {
        return Get();
    }

    void SetDeleteSelf() override
    {
        delete_self_ = true;
    }

    bool GetDeleteSelf() const override
    {
        return delete_self_;
    }

    uint32_t GetRefCount() override
    {
        return ref_count_.load(std::memory_order_relaxed);
    }

    bool GetFinalRun() override
    {
        return false;
    }

    napi_value GetNapiValue() override
    {
        return Get();
    }

    void ResetFinalizer() override
    {
    }

    uintptr_t GetGlobalRefSlotAddress() const override
    {
        return 0;
    }

    static void operator delete(void* pointer) noexcept
    {
        auto raw = reinterpret_cast<uintptr_t>(pointer) & ~mixed_napi_env::TAG_BIT_MASK;
        ::operator delete(reinterpret_cast<void*>(raw));
    }

    bool IsValid() const
    {
        return magic_ == COMPATIBLE_REFERENCE_MAGIC;
    }

    napi_env GetCangjieEnv() const
    {
        return cj_env_;
    }

    napi_ref GetCangjieRef() const
    {
        return cj_ref_;
    }

private:
    uint64_t magic_ {COMPATIBLE_REFERENCE_MAGIC};
    napi_env cj_env_ {nullptr};
    napi_ref cj_ref_ {nullptr};
    std::atomic<uint32_t> ref_count_ {0};
    bool delete_self_ {false};
};

CompatibleNativeReference* UnwrapCompatibleReference(napi_ref ref)
{
    if (ref == nullptr || !is_tagged_cangjie_pointer(ref)) {
        return nullptr;
    }
    auto raw = reinterpret_cast<uintptr_t>(ref) & ~mixed_napi_env::TAG_BIT_MASK;
    auto* reference = reinterpret_cast<CompatibleNativeReference*>(raw);
    return reference->IsValid() ? reference : nullptr;
}

napi_ref TagCompatibleReference(CompatibleNativeReference* reference)
{
    auto* base = static_cast<NativeReference*>(reference);
    return reinterpret_cast<napi_ref>(reinterpret_cast<uintptr_t>(base) | REFERENCE_TAG);
}

napi_status ExportCompatibleReference(napi_env cj_env, napi_ref cj_ref,
                                      uint32_t initial_refcount, napi_ref* result)
{
    if (result == nullptr || cj_ref == nullptr) {
        return napi_invalid_arg;
    }
    auto* reference = new (std::nothrow) CompatibleNativeReference(cj_env, cj_ref, initial_refcount);
    if (reference == nullptr) {
        if (cj_napi_delete_reference != nullptr) {
            (void)cj_napi_delete_reference(cj_env, cj_ref);
        }
        return napi_generic_failure;
    }
    *result = TagCompatibleReference(reference);
    return napi_ok;
}

napi_status FinishReferenceResult(napi_env cj_env, napi_status status, napi_ref cj_ref,
                                  uint32_t initial_refcount, napi_ref* result)
{
    if (status != napi_ok || result == nullptr) {
        return status;
    }
    return ExportCompatibleReference(cj_env, cj_ref, initial_refcount, result);
}
std::mutex& GetUvHandleMutex()
{
    static std::mutex mu;
    return mu;
}

std::unordered_set<uintptr_t>& GetAsyncHandles()
{
    static std::unordered_set<uintptr_t> handles;
    return handles;
}

std::unordered_set<uintptr_t>& GetTimerHandles()
{
    static std::unordered_set<uintptr_t> handles;
    return handles;
}

template <typename HandleT>
uintptr_t HandleKey(HandleT handle)
{
    return reinterpret_cast<uintptr_t>(handle);
}

enum class DispatchEnvKind : uint32_t {
    State = 0x434a4553u,
    Call = 0x434a4543u,
};

struct DispatchCallEnv;

struct DispatchThreadState {
    DispatchThreadState() : store(false)
    {
        store.push(1, false);
    }

    fast::Store store;
    napi_status last_status {napi_ok};
    bool last_error_clean {true};
    bool pending_exception {false};
};

struct DispatchEnvState {
    DispatchEnvState(napi_env cj, napi_env platform) : cj_env(cj), platform_env(platform) {}

    DispatchEnvKind kind {DispatchEnvKind::State};
    napi_env cj_env {nullptr};
    napi_env platform_env {nullptr};
    std::atomic<bool> closed {false};
    napi_status last_status {napi_ok};
    bool last_error_clean {true};
    bool pending_exception {false};
    std::mutex status_lock;
    std::mutex retired_lock;
    DispatchCallEnv* retired_calls {nullptr};
};

struct DispatchCallEnv {
    explicit DispatchCallEnv(DispatchEnvState* env_state)
        : state(env_state),
          cj_env(env_state == nullptr ? nullptr : env_state->cj_env)
    {
    }

    DispatchEnvKind kind {DispatchEnvKind::Call};
    DispatchEnvState* state {nullptr};
    napi_env cj_env {nullptr};
    std::mutex thread_states_lock;
    std::unordered_map<uint64_t, std::unique_ptr<DispatchThreadState>> thread_states;
    bool retired {false};
    DispatchCallEnv* next_retired {nullptr};
};

struct ActiveDispatchThread {
    DispatchCallEnv* call_env {nullptr};
    DispatchThreadState* thread_state {nullptr};
};

thread_local std::vector<ActiveDispatchThread> g_active_dispatch_threads;

DispatchThreadState* FindActiveThreadState(DispatchCallEnv* call_env)
{
    for (auto it = g_active_dispatch_threads.rbegin();
         it != g_active_dispatch_threads.rend(); ++it) {
        if (it->call_env == call_env) {
            return it->thread_state;
        }
    }
    return nullptr;
}

uint64_t CurrentCJThreadId()
{
    if (MRT_GetCurrentThreadID != nullptr) {
        return static_cast<uint64_t>(MRT_GetCurrentThreadID());
    }
    if (CJ_CJThreadId != nullptr) {
        return static_cast<uint64_t>(CJ_CJThreadId());
    }
    return 0;
}

template <typename T>
T* UnwrapDispatchPointer(napi_env env)
{
    return reinterpret_cast<T*>(mixed_napi_env::unwrap(env));
}

DispatchEnvState* GetDispatchEnvState(napi_env env)
{
    auto* ptr = UnwrapDispatchPointer<DispatchEnvState>(env);
    if (ptr == nullptr || ptr->kind != DispatchEnvKind::State) {
        return nullptr;
    }
    return ptr;
}

DispatchCallEnv* GetDispatchCallEnv(napi_env env)
{
    auto* ptr = UnwrapDispatchPointer<DispatchCallEnv>(env);
    if (ptr == nullptr || ptr->kind != DispatchEnvKind::Call) {
        return nullptr;
    }
    auto* state = ptr->state;
    if (state == nullptr || state->closed.load(std::memory_order_acquire)) {
        return nullptr;
    }
    return ptr;
}

DispatchEnvState* GetDispatchStateFromEnv(napi_env env)
{
    auto* call_env = GetDispatchCallEnv(env);
    if (call_env != nullptr) {
        return call_env->state;
    }
    return GetDispatchEnvState(env);
}

fast::Store* GetFastStore(napi_env env)
{
    auto* call_env = GetDispatchCallEnv(env);
    if (call_env == nullptr) {
        return nullptr;
    }
    auto* thread_state = FindActiveThreadState(call_env);
    return thread_state == nullptr ? nullptr : &thread_state->store;
}

DispatchThreadState* GetActiveThreadState(napi_env env)
{
    auto* call_env = GetDispatchCallEnv(env);
    if (call_env == nullptr) {
        return nullptr;
    }
    return FindActiveThreadState(call_env);
}

DispatchThreadState* GetOrCreateThreadState(DispatchCallEnv* call_env, uint64_t cjthread_id)
{
    if (call_env == nullptr) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(call_env->thread_states_lock);
    auto it = call_env->thread_states.find(cjthread_id);
    if (it != call_env->thread_states.end()) {
        return it->second.get();
    }
    auto thread_state = std::make_unique<DispatchThreadState>();
    auto* result = thread_state.get();
    call_env->thread_states.emplace(cjthread_id, std::move(thread_state));
    return result;
}

bool EnterThreadScope(DispatchCallEnv* call_env, uintptr_t scope_id, bool escapable)
{
    // A nested synchronous callback is still running on the same OS stack and
    // therefore the same Cangjie thread. Reuse that binding so nested native
    // constructors neither query the expensive runtime thread ID nor touch the
    // shared thread-state map. A fresh downward call performs both exactly once.
    auto* thread_state = FindActiveThreadState(call_env);
    if (thread_state == nullptr) {
        thread_state = GetOrCreateThreadState(call_env, CurrentCJThreadId());
    }
    if (thread_state == nullptr) {
        return false;
    }
    g_active_dispatch_threads.push_back({call_env, thread_state});
    if (!fast::OpenScope(&thread_state->store, scope_id, escapable)) {
        g_active_dispatch_threads.pop_back();
        return false;
    }
    return true;
}

bool LeaveThreadScope(DispatchCallEnv* call_env, uintptr_t scope_id)
{
    if (call_env == nullptr || g_active_dispatch_threads.empty() ||
        g_active_dispatch_threads.back().call_env != call_env) {
        return false;
    }
    auto* thread_state = g_active_dispatch_threads.back().thread_state;
    bool closed = fast::CloseScope(&thread_state->store, scope_id);
    g_active_dispatch_threads.pop_back();
    return closed;
}

napi_value UseMaterializedLazyHandle(napi_value value)
{
    auto materialized = fast::GetMaterializedHandle(value);
    return materialized == nullptr ? value : materialized;
}

bool CloseFastScope(napi_env env, uintptr_t scope_id)
{
    return fast::CloseScope(GetFastStore(env), scope_id);
}

napi_env GetCangjieEnv(napi_env env)
{
    auto* call_env = GetDispatchCallEnv(env);
    if (call_env == nullptr) {
        return nullptr;
    }
    return call_env->cj_env;
}

bool IsOrdinaryHandleSafeForScope(
    fast::Store* store, fast::detail::Scope* target, napi_value value)
{
    if (store == nullptr || target == nullptr || value == nullptr) {
        return false;
    }
    const auto raw = reinterpret_cast<uintptr_t>(value);
    const auto scope_index = static_cast<uint16_t>((raw >> 48) & 0xFFFFu);
    const auto scope_id = static_cast<uint16_t>((raw >> 32) & 0xFFFFu);
    if (scope_index == 1) {
        return scope_id == 0;
    }
    auto* source = store->scopeAtIndex(scope_index);
    return source != nullptr && static_cast<uint16_t>(source->id & 0xFFFFu) == scope_id &&
        fast::detail::IsAncestorOrSelf(source, target);
}

class ScopeCapturePlan {
public:
    ScopeCapturePlan(fast::Store* store, fast::detail::Scope* target, napi_value value)
        : store_(store), target_(target), captured_(value)
    {
    }

    napi_status Prepare()
    {
        if (store_ == nullptr || target_ == nullptr || !store_->containsScope(target_)) {
            return napi_handle_scope_mismatch;
        }
        return PlanValue(captured_, &captured_);
    }

    bool NeedsCangjieCapture() const
    {
        return !ordinary_handles_.empty();
    }

    napi_status Commit(napi_env cj_env)
    {
        std::vector<napi_value> captured_handles;
        if (!ordinary_handles_.empty()) {
            if (cj_env == nullptr || cj_napi_capture_values_to_scope == nullptr) {
                return napi_generic_failure;
            }
            if (ordinary_handles_.size() >
                static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
                return napi_generic_failure;
            }
            captured_handles.resize(ordinary_handles_.size());
            auto status = cj_napi_capture_values_to_scope(cj_env, target_->id,
                ordinary_handles_.data(), static_cast<int64_t>(ordinary_handles_.size()),
                captured_handles.data());
            if (status != napi_ok) {
                return status;
            }
        }

        // The plan was fully validated before this point. Moving chunks and
        // lazy nodes is allocation-free and therefore cannot fail in normal
        // operation. A failure still leaves memory retained longer, never
        // shorter, and the caller will not publish the property slot.
        for (auto* chunk : value_chunks_) {
            if (!fast::detail::PromoteValueChunk(store_, chunk, target_)) {
                return napi_generic_failure;
            }
        }
        for (auto* value : fast_values_) {
            fast::detail::SetValueOwnerScope(value, target_);
        }
        for (auto* chunk : lazy_chunks_) {
            if (!fast::detail::PromoteLazyChunk(store_, chunk, target_)) {
                return napi_generic_failure;
            }
        }
        for (auto* lazy : lazy_values_) {
            if (!fast::detail::PromoteLazyValue(store_, lazy, target_)) {
                return napi_generic_failure;
            }
        }
        for (const auto& rewrite : direct_rewrites_) {
            *rewrite.first = rewrite.second;
        }
        for (const auto& rewrite : captured_rewrites_) {
            *rewrite.first = captured_handles[rewrite.second];
        }
        return napi_ok;
    }

    napi_value CapturedValue() const
    {
        return captured_;
    }

private:
    using DirectRewrite = std::pair<napi_value*, napi_value>;
    using CapturedRewrite = std::pair<napi_value*, size_t>;

    napi_status PlanOwner(fast::detail::Scope* owner)
    {
        if (!fast::detail::CanPromoteOwnerToScope(store_, owner, target_)) {
            return napi_handle_scope_mismatch;
        }
        return napi_ok;
    }

    napi_status PlanValueChunk(fast::Value* value)
    {
        auto* logical_owner = fast::detail::GetValueOwnerScope(store_, value);
        if (logical_owner == nullptr || PlanOwner(logical_owner) != napi_ok) {
            return napi_handle_scope_mismatch;
        }
        auto* chunk = fast::detail::FindValueChunk(store_, value);
        if (chunk == nullptr || PlanOwner(chunk->owner) != napi_ok) {
            return napi_handle_scope_mismatch;
        }
        if (fast::detail::IsAncestorOrSelf(target_, logical_owner)) {
            fast_values_.insert(value);
        }
        if (fast::detail::IsAncestorOrSelf(target_, chunk->owner)) {
            value_chunks_.insert(chunk);
        }
        return napi_ok;
    }

    napi_status PlanLazyAllocation(const void* allocation)
    {
        auto* chunk = fast::detail::FindLazyChunk(store_, allocation);
        if (chunk == nullptr || PlanOwner(chunk->owner) != napi_ok) {
            return napi_handle_scope_mismatch;
        }
        if (fast::detail::IsAncestorOrSelf(target_, chunk->owner)) {
            lazy_chunks_.insert(chunk);
        }
        return napi_ok;
    }

    bool IsOrdinaryHandleSafe(napi_value value) const
    {
        return IsOrdinaryHandleSafeForScope(store_, target_, value);
    }

    napi_status PlanOrdinaryHandle(napi_value value, napi_value* slot, bool always_rewrite)
    {
        if (value == nullptr) {
            return napi_invalid_arg;
        }
        if (IsOrdinaryHandleSafe(value)) {
            if (always_rewrite) {
                direct_rewrites_.emplace_back(slot, value);
            }
            return napi_ok;
        }

        const auto raw = reinterpret_cast<uintptr_t>(value);
        const auto scope_index = static_cast<uint16_t>((raw >> 48) & 0xFFFFu);
        const auto scope_id = static_cast<uint16_t>((raw >> 32) & 0xFFFFu);
        auto* source = store_->scopeAtIndex(scope_index);
        if (source == nullptr || static_cast<uint16_t>(source->id & 0xFFFFu) != scope_id ||
            !fast::detail::IsAncestorOrSelf(target_, source)) {
            return napi_handle_scope_mismatch;
        }

        auto [it, inserted] = ordinary_handle_indices_.emplace(raw, ordinary_handles_.size());
        if (inserted) {
            ordinary_handles_.push_back(value);
        }
        captured_rewrites_.emplace_back(slot, it->second);
        return napi_ok;
    }

    napi_status PlanLazyValue(napi_value encoded, napi_value* slot)
    {
        auto* lazy = fast::DecodeLazyPointer(encoded);
        if (lazy == nullptr || fast::detail::FindLazyChunk(store_, lazy) == nullptr ||
            lazy->store != store_ || !fast::detail::IsKnownLazyValue(store_, lazy)) {
            return napi_handle_scope_mismatch;
        }
        if (lazy->materialized != nullptr) {
            return PlanOrdinaryHandle(lazy->materialized, slot, true);
        }
        if (!visited_lazy_.insert(lazy).second) {
            return napi_ok;
        }
        auto status = PlanOwner(lazy->scope);
        if (status != napi_ok) {
            return status;
        }
        if (fast::detail::IsAncestorOrSelf(target_, lazy->scope)) {
            lazy_values_.insert(lazy);
        }
        status = PlanLazyAllocation(lazy);
        if (status != napi_ok) {
            return status;
        }
        if (lazy->text != nullptr) {
            status = PlanLazyAllocation(lazy->text);
            if (status != napi_ok) {
                return status;
            }
        }
        for (auto* segment = lazy->named_head; segment != nullptr; segment = segment->next) {
            status = PlanLazyAllocation(segment);
            if (status != napi_ok) {
                return status;
            }
            for (uint16_t i = 0; i < segment->used; ++i) {
                auto& property = segment->entries[i];
                status = PlanLazyAllocation(property.name);
                if (status != napi_ok) {
                    return status;
                }
                status = PlanValue(property.value, &property.value);
                if (status != napi_ok) {
                    return status;
                }
            }
        }
        for (auto* segment = lazy->elements_head; segment != nullptr; segment = segment->next) {
            status = PlanLazyAllocation(segment);
            if (status != napi_ok) {
                return status;
            }
            for (uint16_t i = 0; i < segment->used; ++i) {
                auto& element = segment->entries[i];
                status = PlanValue(element.value, &element.value);
                if (status != napi_ok) {
                    return status;
                }
            }
        }
        return napi_ok;
    }

    napi_status PlanValue(napi_value value, napi_value* slot)
    {
        if (value == nullptr) {
            return napi_invalid_arg;
        }
        if (fast::IsImmediateValue(value)) {
            return napi_ok;
        }
        if (fast::IsPointerValue(value)) {
            return PlanValueChunk(fast::DecodePointer(value));
        }
        if (fast::IsLazyValue(value)) {
            return PlanLazyValue(value, slot);
        }
        return PlanOrdinaryHandle(value, slot, false);
    }

    fast::Store* store_ {nullptr};
    fast::detail::Scope* target_ {nullptr};
    napi_value captured_ {nullptr};
    std::unordered_set<fast::detail::LazyValue*> visited_lazy_;
    std::unordered_set<fast::Value*> fast_values_;
    std::unordered_set<fast::detail::ValueChunk*> value_chunks_;
    std::unordered_set<fast::detail::LazyChunk*> lazy_chunks_;
    std::unordered_set<fast::detail::LazyValue*> lazy_values_;
    std::unordered_map<uintptr_t, size_t> ordinary_handle_indices_;
    std::vector<napi_value> ordinary_handles_;
    std::vector<DirectRewrite> direct_rewrites_;
    std::vector<CapturedRewrite> captured_rewrites_;
};

napi_status CaptureValueForLazyObject(
    napi_env env, napi_value object, napi_value value, napi_value* captured, bool* fallback)
{
    if (captured == nullptr || fallback == nullptr || !fast::IsUnmaterializedLazyValue(object)) {
        return napi_invalid_arg;
    }
    auto* lazy = fast::DecodeLazyPointer(object);
    auto* store = GetFastStore(env);
    if (lazy == nullptr || lazy->store != store || lazy->scope == nullptr ||
        !fast::IsLazyObjectLike(lazy) || !store->containsScope(lazy->scope)) {
        return napi_invalid_arg;
    }
    auto* target = lazy->scope;
    if (fast::IsImmediateValue(value)) {
        *fallback = false;
        *captured = value;
        return napi_ok;
    }
    if (fast::IsPointerValue(value)) {
        auto* owner = fast::detail::GetValueOwnerScope(store, fast::DecodePointer(value));
        if (owner != nullptr && fast::detail::IsAncestorOrSelf(owner, target)) {
            *fallback = false;
            *captured = value;
            return napi_ok;
        }
    } else if (fast::IsLazyValue(value)) {
        auto* value_lazy = fast::DecodeLazyPointer(value);
        if (value_lazy != nullptr && value_lazy->store == store && value_lazy->scope != nullptr &&
            store->containsScope(value_lazy->scope)) {
            if (value_lazy->materialized != nullptr &&
                IsOrdinaryHandleSafeForScope(store, target, value_lazy->materialized)) {
                *fallback = false;
                *captured = value_lazy->materialized;
                return napi_ok;
            }
            if (value_lazy->materialized == nullptr && value_lazy->scope != nullptr &&
                fast::detail::IsAncestorOrSelf(value_lazy->scope, target)) {
                *fallback = false;
                *captured = value;
                return napi_ok;
            }
        }
    } else if (IsOrdinaryHandleSafeForScope(store, target, value)) {
        *fallback = false;
        *captured = value;
        return napi_ok;
    }
    try {
        ScopeCapturePlan plan(store, lazy->scope, value);
        auto status = plan.Prepare();
        if (status != napi_ok) {
            return status;
        }
        if (plan.NeedsCangjieCapture() && cj_napi_capture_values_to_scope == nullptr) {
            *fallback = true;
            *captured = value;
            return napi_ok;
        }
        status = plan.Commit(GetCangjieEnv(env));
        if (status != napi_ok) {
            return status;
        }
        *fallback = false;
        *captured = plan.CapturedValue();
        return napi_ok;
    } catch (const std::bad_alloc&) {
        return napi_generic_failure;
    }
}

napi_status TrySetCapturedNamedProperty(napi_env env, napi_value object, const char* name,
    napi_value value, bool* handled)
{
    *handled = false;
    if (!fast::IsUnmaterializedLazyValue(object)) {
        return napi_ok;
    }
    auto* lazy = fast::DecodeLazyPointer(object);
    if (!fast::IsLazyObjectLike(lazy)) {
        return napi_ok;
    }
    *handled = true;
    auto* store = GetFastStore(env);
    if (lazy->store != store || lazy->scope == nullptr || !store->containsScope(lazy->scope)) {
        return napi_handle_scope_mismatch;
    }
    fast::NamedPropertyReservation reservation;
    auto status = fast::PrepareNamedProperty(object, name, &reservation);
    if (status != napi_ok) {
        return status;
    }
    napi_value captured = nullptr;
    bool fallback = false;
    status = CaptureValueForLazyObject(env, object, value, &captured, &fallback);
    if (status != napi_ok) {
        return status;
    }
    if (fallback) {
        *handled = false;
        return napi_ok;
    }
    fast::CommitNamedProperty(reservation, captured);
    return napi_ok;
}

napi_status TrySetCapturedElement(napi_env env, napi_value object, uint32_t index,
    napi_value value, bool* handled)
{
    *handled = false;
    if (!fast::IsUnmaterializedLazyValue(object)) {
        return napi_ok;
    }
    auto* lazy = fast::DecodeLazyPointer(object);
    if (!fast::IsLazyObjectLike(lazy)) {
        return napi_ok;
    }
    *handled = true;
    auto* store = GetFastStore(env);
    if (lazy->store != store || lazy->scope == nullptr || !store->containsScope(lazy->scope)) {
        return napi_handle_scope_mismatch;
    }
    fast::ElementReservation reservation;
    auto status = fast::PrepareElement(object, index, &reservation);
    if (status != napi_ok) {
        return status;
    }
    napi_value captured = nullptr;
    bool fallback = false;
    status = CaptureValueForLazyObject(env, object, value, &captured, &fallback);
    if (status != napi_ok) {
        return status;
    }
    if (fallback) {
        *handled = false;
        return napi_ok;
    }
    fast::CommitElement(reservation, captured);
    return napi_ok;
}

napi_status RecordStatus(napi_env env, napi_status status)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        thread_state->last_status = status;
        thread_state->last_error_clean = status == napi_ok;
        if (status == napi_pending_exception) {
            thread_state->pending_exception = true;
        }
        return status;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state != nullptr && !state->closed.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(state->status_lock);
        state->last_status = status;
        state->last_error_clean = status == napi_ok;
        if (status == napi_pending_exception) {
            state->pending_exception = true;
        }
    }
    return status;
}

napi_status RecordPendingException(napi_env env, napi_status status)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        thread_state->last_status = status == napi_ok ? napi_pending_exception : status;
        thread_state->last_error_clean = false;
        if (status == napi_ok || status == napi_pending_exception) {
            thread_state->pending_exception = true;
        }
        return status;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state != nullptr && !state->closed.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(state->status_lock);
        state->last_status = status == napi_ok ? napi_pending_exception : status;
        state->last_error_clean = false;
        if (status == napi_ok || status == napi_pending_exception) {
            state->pending_exception = true;
        }
    }
    return status;
}

void ClearPendingException(napi_env env)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        thread_state->pending_exception = false;
        return;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state != nullptr && !state->closed.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(state->status_lock);
        state->pending_exception = false;
    }
}

void SetPendingException(napi_env env, bool pending)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        thread_state->pending_exception = pending;
        return;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state != nullptr && !state->closed.load(std::memory_order_relaxed)) {
        std::lock_guard<std::mutex> lock(state->status_lock);
        state->pending_exception = pending;
    }
}

void GetLastErrorSnapshot(napi_env env, napi_status& status, bool& clean)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        status = thread_state->last_status;
        clean = thread_state->last_error_clean;
        return;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state == nullptr || state->closed.load(std::memory_order_relaxed)) {
        status = napi_ok;
        clean = true;
        return;
    }
    std::lock_guard<std::mutex> lock(state->status_lock);
    status = state->last_status;
    clean = state->last_error_clean;
}

const char* StatusMessage(napi_status status)
{
    switch (status) {
        case napi_ok:
            return "napi_ok";
        case napi_invalid_arg:
            return "Invalid argument";
        case napi_pending_exception:
            return "napi_pending_exception";
        default:
            return "Invalid status";
    }
}

void FillErrorInfo(napi_extended_error_info& info, napi_status status)
{
    info.error_message = StatusMessage(status);
    info.engine_reserved = nullptr;
    info.engine_error_code = 0;
    info.error_code = status;
}

DispatchEnvState* NewDispatchEnvState(napi_env cj_env, napi_env platform_env)
{
    return new (std::nothrow) DispatchEnvState(cj_env, platform_env);
}

DispatchCallEnv* NewDispatchCallEnv(DispatchEnvState* state)
{
    return new (std::nothrow) DispatchCallEnv(state);
}

void DestroyDispatchEnvState(DispatchEnvState* state)
{
    if (state == nullptr) {
        return;
    }
    state->closed.store(true, std::memory_order_release);
    DispatchCallEnv* retired = nullptr;
    {
        std::lock_guard<std::mutex> lock(state->retired_lock);
        retired = state->retired_calls;
        state->retired_calls = nullptr;
    }
    while (retired != nullptr) {
        auto* next = retired->next_retired;
        delete retired;
        retired = next;
    }
    delete state;
}

void RetireDispatchCallEnv(DispatchCallEnv* call_env)
{
    if (call_env == nullptr || call_env->retired) {
        return;
    }
    auto* state = call_env->state;
    if (state == nullptr || state->closed.load(std::memory_order_acquire)) {
        delete call_env;
        return;
    }
    call_env->retired = true;
    std::lock_guard<std::mutex> lock(state->retired_lock);
    call_env->next_retired = state->retired_calls;
    state->retired_calls = call_env;
}

bool IsTaggedUvLoop(uv_loop_t* loop)
{
    if (loop == nullptr) {
        return false;
    }
    return ((reinterpret_cast<uintptr_t>(loop) & UV_TAG_MASK) >> 56) == UV_TAG_CUSTOM;
}

}  // namespace

extern "C" CJ_EXPORT napi_env CJNAPI_FastValue_BindEnv(napi_env env, napi_env platform_env)
{
    if (env == nullptr || platform_env == nullptr) {
        return nullptr;
    }
    auto* state = NewDispatchEnvState(env, platform_env);
    if (state == nullptr) {
        return nullptr;
    }
    return mixed_napi_env::wrap(reinterpret_cast<napi_env>(state), mixed_napi_env::Cangjie);
}

extern "C" CJ_EXPORT void CJNAPI_FastValue_UnbindEnv(napi_env env)
{
    auto* state = GetDispatchEnvState(env);
    if (state == nullptr) {
        return;
    }
    DestroyDispatchEnvState(state);
}

extern "C" CJ_EXPORT napi_env CJNAPI_FastValue_EnterScope(napi_env env, uint64_t scope_id, int32_t escapable)
{
    (void)scope_id;
    (void)escapable;
    auto* state = GetDispatchEnvState(env);
    if (state == nullptr || state->closed.load(std::memory_order_acquire)) {
        return nullptr;
    }
    auto* call_env = NewDispatchCallEnv(state);
    if (call_env == nullptr) {
        return nullptr;
    }
    return mixed_napi_env::wrap(reinterpret_cast<napi_env>(call_env), mixed_napi_env::Cangjie);
}

namespace {

struct PlatformAsyncWorkBridge {
    napi_env cj_env;
    napi_env callback_env;
    napi_async_work cj_work;
    napi_async_work platform_work;
    napi_async_execute_callback execute;
    napi_async_complete_callback complete;
    void* data;
};

bool BeginNativeCallbackScope(PlatformAsyncWorkBridge* bridge, uint64_t& scope_id)
{
    return ace_napi_hook::cj_napi_begin_native_async_callback != nullptr &&
        ace_napi_hook::cj_napi_begin_native_async_callback(bridge->cj_env, &scope_id) == napi_ok;
}

void EndNativeCallbackScope(PlatformAsyncWorkBridge* bridge, uint64_t scope_id)
{
    if (ace_napi_hook::cj_napi_end_native_async_callback != nullptr) {
        (void)ace_napi_hook::cj_napi_end_native_async_callback(bridge->cj_env, scope_id);
    }
}

void ExecutePlatformAsyncWork(napi_env, void* data)
{
    auto* bridge = static_cast<PlatformAsyncWorkBridge*>(data);
    uint64_t scope_id = 0;
    if (bridge->execute != nullptr && BeginNativeCallbackScope(bridge, scope_id)) {
        bridge->execute(bridge->callback_env, bridge->data);
        EndNativeCallbackScope(bridge, scope_id);
    }
}

void CompletePlatformAsyncWork(napi_env platform_env, napi_status status, void* data)
{
    std::unique_ptr<PlatformAsyncWorkBridge> bridge(
        static_cast<PlatformAsyncWorkBridge*>(data));
    uint64_t scope_id = 0;
    if (bridge->complete != nullptr && BeginNativeCallbackScope(bridge.get(), scope_id)) {
        bridge->complete(bridge->callback_env, status, bridge->data);
        EndNativeCallbackScope(bridge.get(), scope_id);
    }
    if (ace_napi_hook::orig_napi_delete_async_work != nullptr) {
        (void)ace_napi_hook::orig_napi_delete_async_work(platform_env, bridge->platform_work);
    }
}

}  // namespace

extern "C" CJ_EXPORT int32_t CJNAPI_QueuePlatformAsyncWork(
    napi_env platform_env, napi_env cj_env, napi_env callback_env,
    napi_async_work work, napi_async_execute_callback execute,
    napi_async_complete_callback complete, void* data)
{
    if (platform_env == nullptr || cj_env == nullptr || callback_env == nullptr ||
        work == nullptr || execute == nullptr || complete == nullptr ||
        ace_napi_hook::orig_napi_create_string_utf8 == nullptr ||
        ace_napi_hook::orig_napi_create_async_work == nullptr ||
        ace_napi_hook::orig_napi_queue_async_work == nullptr ||
        ace_napi_hook::orig_napi_delete_async_work == nullptr) {
        return napi_invalid_arg;
    }

    napi_value resource_name = nullptr;
    napi_status status = ace_napi_hook::orig_napi_create_string_utf8(
        platform_env, "cangjie_napi_async_work", NAPI_AUTO_LENGTH, &resource_name);
    if (status != napi_ok) {
        return status;
    }

    auto bridge = std::make_unique<PlatformAsyncWorkBridge>();
    bridge->cj_env = cj_env;
    bridge->callback_env = callback_env;
    bridge->cj_work = work;
    bridge->platform_work = nullptr;
    bridge->execute = execute;
    bridge->complete = complete;
    bridge->data = data;
    status = ace_napi_hook::orig_napi_create_async_work(
        platform_env, nullptr, resource_name, ExecutePlatformAsyncWork,
        CompletePlatformAsyncWork, bridge.get(), &bridge->platform_work);
    if (status != napi_ok) {
        return status;
    }

    status = ace_napi_hook::orig_napi_queue_async_work(platform_env, bridge->platform_work);
    if (status != napi_ok) {
        (void)ace_napi_hook::orig_napi_delete_async_work(platform_env, bridge->platform_work);
        return status;
    }
    (void)bridge.release();
    return napi_ok;
}

extern "C" CJ_EXPORT int32_t CJNAPI_QueuePlatformTsfnCall(
    napi_env platform_env, napi_threadsafe_function tsfn, void* data)
{
    auto dispatch = ace_napi_hook::cj_napi_dispatch_threadsafe_function;
    if (platform_env == nullptr || tsfn == nullptr || dispatch == nullptr ||
        ace_napi_hook::orig_napi_send_event == nullptr) {
        return napi_invalid_arg;
    }

    std::function<void()> callback = [dispatch, tsfn, data]() {
        (void)dispatch(tsfn, data);
    };
    return ace_napi_hook::orig_napi_send_event(
        platform_env, callback, 0, "cangjie_napi_tsfn", 0);
}

extern "C" CJ_EXPORT napi_status CJNAPI_FastValue_ExitScope(napi_env env, uint64_t scope_id)
{
    auto* call_env = GetDispatchCallEnv(env);
    if (call_env == nullptr) {
        return napi_invalid_arg;
    }
    (void)scope_id;
    RetireDispatchCallEnv(call_env);
    return napi_ok;
}

extern "C" CJ_EXPORT napi_status CJNAPI_FastValue_OpenScope(napi_env env, uint64_t scope_id, int32_t escapable)
{
    return EnterThreadScope(GetDispatchCallEnv(env), static_cast<uintptr_t>(scope_id), escapable != 0) ?
        napi_ok : napi_generic_failure;
}

extern "C" CJ_EXPORT napi_status CJNAPI_FastValue_CloseScope(napi_env env, uint64_t scope_id)
{
    return LeaveThreadScope(GetDispatchCallEnv(env), static_cast<uintptr_t>(scope_id)) ?
        napi_ok : napi_handle_scope_mismatch;
}

extern "C" CJ_EXPORT int32_t CJNAPI_FastValue_HasPendingException(napi_env env)
{
    auto* thread_state = GetActiveThreadState(env);
    if (thread_state != nullptr) {
        return thread_state->pending_exception ? 1 : 0;
    }
    auto* state = GetDispatchStateFromEnv(env);
    if (state == nullptr || state->closed.load(std::memory_order_relaxed)) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(state->status_lock);
    return state->pending_exception ? 1 : 0;
}

extern "C" CJ_EXPORT int32_t CJNAPI_FastValue_LockLazyValue(napi_value value)
{
    (void)value;
    return 0;
}

extern "C" CJ_EXPORT uint64_t CJNAPI_FastValue_GetOwnerScopeId(napi_value value)
{
    if (!ace_napi_hook::fast::IsLazyValue(value)) {
        return 0;
    }
    auto* lazy = ace_napi_hook::fast::DecodeLazyPointer(value);
    return lazy == nullptr || lazy->scope == nullptr ? 0 : static_cast<uint64_t>(lazy->scope->id);
}

extern "C" CJ_EXPORT void CJNAPI_FastValue_UnlockLazyValue(napi_value value, int32_t locked)
{
    (void)value;
    (void)locked;
}

namespace {

template <typename HandleT>
bool ContainsHandle(const std::unordered_set<uintptr_t>& handles, HandleT handle)
{
    return handles.count(HandleKey(handle)) > 0;
}

template <typename HandleT>
bool IsKnownHandle(std::unordered_set<uintptr_t>& handles, HandleT handle)
{
    std::lock_guard<std::mutex> lk(GetUvHandleMutex());
    return ContainsHandle(handles, handle);
}

template <typename HandleT>
void RememberHandle(std::unordered_set<uintptr_t>& handles, HandleT handle)
{
    std::lock_guard<std::mutex> lk(GetUvHandleMutex());
    handles.insert(HandleKey(handle));
}

template <typename HandleT>
bool ForgetHandle(std::unordered_set<uintptr_t>& handles, HandleT handle)
{
    std::lock_guard<std::mutex> lk(GetUvHandleMutex());
    return handles.erase(HandleKey(handle)) > 0;
}

template <typename HandleT>
bool IsKnownAsyncLikeHandle(HandleT handle)
{
    return IsKnownHandle(GetAsyncHandles(), handle);
}

template <typename HandleT>
bool IsKnownTimerLikeHandle(HandleT handle)
{
    return IsKnownHandle(GetTimerHandles(), handle);
}

}  // namespace

#define DEFINE_ENV_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args) \
extern "C" ret dispatch_##api sig \
{ \
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) { \
        if (ace_napi_hook::cj_##api == nullptr) { \
            return napi_generic_failure; \
        } \
        auto* dispatch_tagged_env = env; \
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env); \
        env = cj_env; \
        return ace_napi_hook::RecordStatus(dispatch_tagged_env, ace_napi_hook::cj_##api cj_args); \
    } \
    if (ace_napi_hook::orig_##api == nullptr) { \
        return napi_generic_failure; \
    } \
    return ace_napi_hook::orig_##api orig_args; \
}

#define DEFINE_NO_ENV_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args) \
extern "C" ret dispatch_##api sig \
{ \
    if (ace_napi_hook::orig_##api == nullptr) { \
        return napi_generic_failure; \
    } \
    return ace_napi_hook::orig_##api orig_args; \
}

#define DEFINE_TSFN_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args) \
extern "C" ret dispatch_##api sig \
{ \
    if (func == nullptr || ace_napi_hook::is_tagged_cangjie_pointer(func)) { \
        if (ace_napi_hook::cj_##api == nullptr) { \
            return napi_generic_failure; \
        } \
        return ace_napi_hook::cj_##api cj_args; \
    } \
    if (ace_napi_hook::orig_##api == nullptr) { \
        return napi_generic_failure; \
    } \
    return ace_napi_hook::orig_##api orig_args; \
}

#define NAPI_HOOK_API(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    DEFINE_##has_env(api, cj_api, ret, sig, cj_args, orig_args)
#define NAPI_HOOK_API_TS_FN(api, cj_api, ret, sig, cj_args, orig_args, has_env) \
    DEFINE_TSFN_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args)
#define DEFINE_1(api, cj_api, ret, sig, cj_args, orig_args) DEFINE_ENV_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args)
#define DEFINE_0(api, cj_api, ret, sig, cj_args, orig_args) DEFINE_NO_ENV_WRAPPER(api, cj_api, ret, sig, cj_args, orig_args)

#define dispatch_napi_get_undefined dispatch_napi_get_undefined_generated
#define dispatch_napi_get_null dispatch_napi_get_null_generated
#define dispatch_napi_get_boolean dispatch_napi_get_boolean_generated
#define dispatch_napi_open_handle_scope dispatch_napi_open_handle_scope_generated
#define dispatch_napi_close_handle_scope dispatch_napi_close_handle_scope_generated
#define dispatch_napi_open_escapable_handle_scope dispatch_napi_open_escapable_handle_scope_generated
#define dispatch_napi_close_escapable_handle_scope dispatch_napi_close_escapable_handle_scope_generated
#define dispatch_napi_escape_handle dispatch_napi_escape_handle_generated
#define dispatch_napi_create_object dispatch_napi_create_object_generated
#define dispatch_napi_create_array dispatch_napi_create_array_generated
#define dispatch_napi_create_array_with_length dispatch_napi_create_array_with_length_generated
#define dispatch_napi_create_int32 dispatch_napi_create_int32_generated
#define dispatch_napi_create_uint32 dispatch_napi_create_uint32_generated
#define dispatch_napi_create_int64 dispatch_napi_create_int64_generated
#define dispatch_napi_create_double dispatch_napi_create_double_generated
#define dispatch_napi_create_string_utf8 dispatch_napi_create_string_utf8_generated
#define dispatch_napi_create_string_latin1 dispatch_napi_create_string_latin1_generated
#define dispatch_napi_set_named_property dispatch_napi_set_named_property_generated
#define dispatch_napi_get_named_property dispatch_napi_get_named_property_generated
#define dispatch_napi_set_property dispatch_napi_set_property_generated
#define dispatch_napi_get_property dispatch_napi_get_property_generated
#define dispatch_napi_has_property dispatch_napi_has_property_generated
#define dispatch_napi_has_own_property dispatch_napi_has_own_property_generated
#define dispatch_napi_has_named_property dispatch_napi_has_named_property_generated
#define dispatch_napi_delete_property dispatch_napi_delete_property_generated
#define dispatch_napi_set_element dispatch_napi_set_element_generated
#define dispatch_napi_get_element dispatch_napi_get_element_generated
#define dispatch_napi_has_element dispatch_napi_has_element_generated
#define dispatch_napi_delete_element dispatch_napi_delete_element_generated
#define dispatch_napi_is_array dispatch_napi_is_array_generated
#define dispatch_napi_is_error dispatch_napi_is_error_generated
#define dispatch_napi_get_array_length dispatch_napi_get_array_length_generated
#define dispatch_napi_get_property_names dispatch_napi_get_property_names_generated
#define dispatch_napi_get_all_property_names dispatch_napi_get_all_property_names_generated
#define dispatch_napi_get_value_string_utf8 dispatch_napi_get_value_string_utf8_generated
#define dispatch_napi_get_value_string_latin1 dispatch_napi_get_value_string_latin1_generated
#define dispatch_napi_create_reference dispatch_napi_create_reference_generated
#define dispatch_napi_get_reference_value dispatch_napi_get_reference_value_generated
#define dispatch_napi_delete_reference dispatch_napi_delete_reference_generated
#define dispatch_napi_reference_ref dispatch_napi_reference_ref_generated
#define dispatch_napi_reference_unref dispatch_napi_reference_unref_generated
#define dispatch_napi_wrap dispatch_napi_wrap_generated
#define dispatch_napi_wrap_with_size dispatch_napi_wrap_with_size_generated
#define dispatch_napi_unwrap dispatch_napi_unwrap_generated
#define dispatch_napi_remove_wrap dispatch_napi_remove_wrap_generated
#define dispatch_napi_add_finalizer dispatch_napi_add_finalizer_generated
#define dispatch_napi_type_tag_object dispatch_napi_type_tag_object_generated
#define dispatch_napi_check_object_type_tag dispatch_napi_check_object_type_tag_generated
#define dispatch_napi_call_function dispatch_napi_call_function_generated
#define dispatch_napi_new_instance dispatch_napi_new_instance_generated
#define dispatch_napi_define_properties dispatch_napi_define_properties_generated
#define dispatch_napi_throw dispatch_napi_throw_generated
#define dispatch_napi_fatal_exception dispatch_napi_fatal_exception_generated
#define dispatch_napi_get_and_clear_last_exception dispatch_napi_get_and_clear_last_exception_generated
#define dispatch_napi_is_exception_pending dispatch_napi_is_exception_pending_generated
#define dispatch_napi_throw_error dispatch_napi_throw_error_generated
#define dispatch_napi_throw_range_error dispatch_napi_throw_range_error_generated
#define dispatch_napi_throw_type_error dispatch_napi_throw_type_error_generated
#define dispatch_napi_create_error dispatch_napi_create_error_generated
#define dispatch_napi_create_type_error dispatch_napi_create_type_error_generated
#define dispatch_napi_create_range_error dispatch_napi_create_range_error_generated
#define dispatch_napi_get_value_int32 dispatch_napi_get_value_int32_generated
#define dispatch_napi_get_value_uint32 dispatch_napi_get_value_uint32_generated
#define dispatch_napi_get_value_int64 dispatch_napi_get_value_int64_generated
#define dispatch_napi_get_value_double dispatch_napi_get_value_double_generated
#define dispatch_napi_get_value_bool dispatch_napi_get_value_bool_generated
#define dispatch_napi_get_last_error_info dispatch_napi_get_last_error_info_generated
#define dispatch_napi_typeof dispatch_napi_typeof_generated
#define dispatch_napi_strict_equals dispatch_napi_strict_equals_generated
#include "api_list.inc"
#undef dispatch_napi_strict_equals
#undef dispatch_napi_typeof
#undef dispatch_napi_get_last_error_info
#undef dispatch_napi_get_value_bool
#undef dispatch_napi_get_value_double
#undef dispatch_napi_get_value_int64
#undef dispatch_napi_get_value_uint32
#undef dispatch_napi_get_value_int32
#undef dispatch_napi_create_range_error
#undef dispatch_napi_create_type_error
#undef dispatch_napi_create_error
#undef dispatch_napi_throw_type_error
#undef dispatch_napi_throw_range_error
#undef dispatch_napi_throw_error
#undef dispatch_napi_is_exception_pending
#undef dispatch_napi_get_and_clear_last_exception
#undef dispatch_napi_fatal_exception
#undef dispatch_napi_throw
#undef dispatch_napi_define_properties
#undef dispatch_napi_new_instance
#undef dispatch_napi_call_function
#undef dispatch_napi_add_finalizer
#undef dispatch_napi_check_object_type_tag
#undef dispatch_napi_type_tag_object
#undef dispatch_napi_remove_wrap
#undef dispatch_napi_unwrap
#undef dispatch_napi_wrap_with_size
#undef dispatch_napi_wrap
#undef dispatch_napi_reference_unref
#undef dispatch_napi_reference_ref
#undef dispatch_napi_delete_reference
#undef dispatch_napi_get_reference_value
#undef dispatch_napi_create_reference
#undef dispatch_napi_get_value_string_latin1
#undef dispatch_napi_get_value_string_utf8
#undef dispatch_napi_get_array_length
#undef dispatch_napi_get_all_property_names
#undef dispatch_napi_get_property_names
#undef dispatch_napi_is_error
#undef dispatch_napi_is_array
#undef dispatch_napi_delete_element
#undef dispatch_napi_has_element
#undef dispatch_napi_get_element
#undef dispatch_napi_set_element
#undef dispatch_napi_get_property
#undef dispatch_napi_set_property
#undef dispatch_napi_delete_property
#undef dispatch_napi_has_named_property
#undef dispatch_napi_has_own_property
#undef dispatch_napi_has_property
#undef dispatch_napi_get_named_property
#undef dispatch_napi_set_named_property
#undef dispatch_napi_create_string_latin1
#undef dispatch_napi_create_string_utf8
#undef dispatch_napi_create_double
#undef dispatch_napi_create_int64
#undef dispatch_napi_create_uint32
#undef dispatch_napi_create_int32
#undef dispatch_napi_create_array_with_length
#undef dispatch_napi_create_array
#undef dispatch_napi_create_object
#undef dispatch_napi_escape_handle
#undef dispatch_napi_close_escapable_handle_scope
#undef dispatch_napi_open_escapable_handle_scope
#undef dispatch_napi_close_handle_scope
#undef dispatch_napi_open_handle_scope
#undef dispatch_napi_get_boolean
#undef dispatch_napi_get_null
#undef dispatch_napi_get_undefined
#undef DEFINE_0
#undef DEFINE_1
#undef NAPI_HOOK_API_TS_FN
#undef NAPI_HOOK_API

extern "C" napi_status dispatch_napi_get_undefined(napi_env env, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::CreateUndefined(store, result));
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_undefined(ace_napi_hook::GetCangjieEnv(env), result));
    }
    return ace_napi_hook::orig_napi_get_undefined(env, result);
}

extern "C" napi_status dispatch_napi_get_null(napi_env env, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::CreateNull(store, result));
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_null(ace_napi_hook::GetCangjieEnv(env), result));
    }
    return ace_napi_hook::orig_napi_get_null(env, result);
}

extern "C" napi_status dispatch_napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::CreateBoolean(store, value, result));
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_boolean(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_boolean(env, value, result);
}

extern "C" napi_status dispatch_napi_open_handle_scope(napi_env env, napi_handle_scope* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (result == nullptr) {
            return napi_invalid_arg;
        }
        auto status = ace_napi_hook::cj_napi_open_handle_scope(ace_napi_hook::GetCangjieEnv(env), result);
        if (status == napi_ok) {
            if (!ace_napi_hook::fast::OpenScope(ace_napi_hook::GetFastStore(env), reinterpret_cast<uintptr_t>(*result), false)) {
                return napi_generic_failure;
            }
        }
        return status;
    }
    return ace_napi_hook::orig_napi_open_handle_scope(ace_napi_hook::mixed_napi_env::unwrap(env), result);
}

extern "C" napi_status dispatch_napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto scope_id = reinterpret_cast<uintptr_t>(scope);
        auto status = ace_napi_hook::cj_napi_close_handle_scope(ace_napi_hook::GetCangjieEnv(env), scope);
        if (status == napi_ok) {
            if (!ace_napi_hook::CloseFastScope(env, scope_id)) {
                return napi_handle_scope_mismatch;
            }
        }
        return status;
    }
    return ace_napi_hook::orig_napi_close_handle_scope(ace_napi_hook::mixed_napi_env::unwrap(env), scope);
}

extern "C" napi_status dispatch_napi_open_escapable_handle_scope(napi_env env, napi_escapable_handle_scope* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (result == nullptr) {
            return napi_invalid_arg;
        }
        auto status = ace_napi_hook::cj_napi_open_escapable_handle_scope(ace_napi_hook::GetCangjieEnv(env), result);
        if (status == napi_ok) {
            if (!ace_napi_hook::fast::OpenScope(ace_napi_hook::GetFastStore(env), reinterpret_cast<uintptr_t>(*result), true)) {
                return napi_generic_failure;
            }
        }
        return status;
    }
    return ace_napi_hook::orig_napi_open_escapable_handle_scope(ace_napi_hook::mixed_napi_env::unwrap(env), result);
}

extern "C" napi_status dispatch_napi_close_escapable_handle_scope(napi_env env, napi_escapable_handle_scope scope)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto scope_id = reinterpret_cast<uintptr_t>(scope);
        auto status = ace_napi_hook::cj_napi_close_escapable_handle_scope(ace_napi_hook::GetCangjieEnv(env), scope);
        if (status == napi_ok) {
            if (!ace_napi_hook::CloseFastScope(env, scope_id)) {
                return napi_handle_scope_mismatch;
            }
        }
        return status;
    }
    return ace_napi_hook::orig_napi_close_escapable_handle_scope(ace_napi_hook::mixed_napi_env::unwrap(env), scope);
}

extern "C" napi_status dispatch_napi_escape_handle(napi_env env, napi_escapable_handle_scope scope,
                                                   napi_value escapee, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_escape_handle(ace_napi_hook::GetCangjieEnv(env), scope, escapee, result));
    }
    return ace_napi_hook::orig_napi_escape_handle(ace_napi_hook::mixed_napi_env::unwrap(env), scope, escapee, result);
}

extern "C" napi_status dispatch_napi_create_object(napi_env env, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateObject(store, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_create_object(ace_napi_hook::GetCangjieEnv(env), result));
    }
    return ace_napi_hook::orig_napi_create_object(env, result);
}

extern "C" napi_status dispatch_napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateArray(store, static_cast<uint32_t>(length), result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_array_with_length(ace_napi_hook::GetCangjieEnv(env), length, result));
    }
    return ace_napi_hook::orig_napi_create_array_with_length(env, length, result);
}

extern "C" napi_status dispatch_napi_create_array(napi_env env, napi_value* result)
{
    return dispatch_napi_create_array_with_length(env, 0, result);
}

extern "C" napi_status dispatch_napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::CreateInt32(store, value, result));
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_create_int32(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_create_int32(env, value, result);
}

extern "C" napi_status dispatch_napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::CreateUInt32(store, value, result));
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_create_uint32(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_create_uint32(env, value, result);
}

extern "C" napi_status dispatch_napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateInt64(store, value, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_create_int64(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_create_int64(env, value, result);
}

extern "C" napi_status dispatch_napi_create_double(napi_env env, double value, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateDouble(store, value, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_create_double(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_create_double(env, value, result);
}

extern "C" napi_status dispatch_napi_create_string_utf8(napi_env env, const char* str, size_t len, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateString(store, ace_napi_hook::fast::detail::LazyKind::StringUtf8, str, len, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_string_utf8(ace_napi_hook::GetCangjieEnv(env), str, len, result));
    }
    return ace_napi_hook::orig_napi_create_string_utf8(env, str, len, result);
}

extern "C" napi_status dispatch_napi_create_string_latin1(napi_env env, const char* str, size_t len, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* store = ace_napi_hook::GetFastStore(env);
        if (ace_napi_hook::fast::HasOpenScope(store)) {
            auto st = ace_napi_hook::fast::CreateString(store, ace_napi_hook::fast::detail::LazyKind::StringLatin1, str, len, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_string_latin1(ace_napi_hook::GetCangjieEnv(env), str, len, result));
    }
    return ace_napi_hook::orig_napi_create_string_latin1(env, str, len, result);
}

extern "C" napi_status dispatch_napi_get_value_int32(napi_env env, napi_value value, int32_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetValueInt32(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_value_int32(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_value_int32(env, value, result);
}

extern "C" napi_status dispatch_napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetValueUInt32(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_value_uint32(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_value_uint32(env, value, result);
}

extern "C" napi_status dispatch_napi_get_value_int64(napi_env env, napi_value value, int64_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetValueInt64(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_value_int64(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_value_int64(env, value, result);
}

extern "C" napi_status dispatch_napi_get_value_double(napi_env env, napi_value value, double* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetValueDouble(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_value_double(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_value_double(env, value, result);
}

extern "C" napi_status dispatch_napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetValueBool(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::cj_napi_get_value_bool(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_value_bool(env, value, result);
}

extern "C" napi_status dispatch_napi_set_named_property(napi_env env, napi_value object, const char* utf8name, napi_value value)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (value == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        bool handled = false;
        auto st = ace_napi_hook::TrySetCapturedNamedProperty(env, object, utf8name, value, &handled);
        if (handled || st != napi_ok) {
            return ace_napi_hook::RecordStatus(env, st);
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_set_named_property(ace_napi_hook::GetCangjieEnv(env), object, utf8name, value));
    }
    return ace_napi_hook::orig_napi_set_named_property(env, object, utf8name, value);
}

extern "C" napi_status dispatch_napi_get_named_property(napi_env env, napi_value object, const char* utf8name, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto st = ace_napi_hook::fast::GetNamedProperty(object, utf8name, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_named_property(ace_napi_hook::GetCangjieEnv(env), object, utf8name, result));
    }
    return ace_napi_hook::orig_napi_get_named_property(env, object, utf8name, result);
}

extern "C" napi_status dispatch_napi_set_property(napi_env env, napi_value object, napi_value key, napi_value value)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (value == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto* text = ace_napi_hook::fast::LazyStringData(key);
            if (text != nullptr) {
                bool handled = false;
                auto st = ace_napi_hook::TrySetCapturedNamedProperty(env, object, text, value, &handled);
                if (handled || st != napi_ok) {
                    return ace_napi_hook::RecordStatus(env, st);
                }
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        key = ace_napi_hook::UseMaterializedLazyHandle(key);
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_set_property(ace_napi_hook::GetCangjieEnv(env), object, key, value));
    }
    return ace_napi_hook::orig_napi_set_property(env, object, key, value);
}

extern "C" napi_status dispatch_napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto* text = ace_napi_hook::fast::LazyStringData(key);
            if (text != nullptr) {
                auto st = ace_napi_hook::fast::GetNamedProperty(object, text, result);
                if (st == napi_ok) {
                    return ace_napi_hook::RecordStatus(env, st);
                }
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        key = ace_napi_hook::UseMaterializedLazyHandle(key);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_property(ace_napi_hook::GetCangjieEnv(env), object, key, result));
    }
    return ace_napi_hook::orig_napi_get_property(env, object, key, result);
}

extern "C" napi_status dispatch_napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto st = ace_napi_hook::fast::HasNamedProperty(object, utf8name, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_has_named_property(ace_napi_hook::GetCangjieEnv(env), object, utf8name, result));
    }
    return ace_napi_hook::orig_napi_has_named_property(env, object, utf8name, result);
}

extern "C" napi_status dispatch_napi_has_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto* text = ace_napi_hook::fast::LazyStringData(key);
            if (text != nullptr) {
                auto st = ace_napi_hook::fast::HasNamedProperty(object, text, result);
                if (st == napi_ok) {
                    return ace_napi_hook::RecordStatus(env, st);
                }
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        key = ace_napi_hook::UseMaterializedLazyHandle(key);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_has_property(ace_napi_hook::GetCangjieEnv(env), object, key, result));
    }
    return ace_napi_hook::orig_napi_has_property(env, object, key, result);
}

extern "C" napi_status dispatch_napi_has_own_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto* text = ace_napi_hook::fast::LazyStringData(key);
            if (text != nullptr) {
                auto st = ace_napi_hook::fast::HasNamedProperty(object, text, result);
                if (st == napi_ok) {
                    return ace_napi_hook::RecordStatus(env, st);
                }
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        key = ace_napi_hook::UseMaterializedLazyHandle(key);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_has_own_property(ace_napi_hook::GetCangjieEnv(env), object, key, result));
    }
    return ace_napi_hook::orig_napi_has_own_property(env, object, key, result);
}

extern "C" napi_status dispatch_napi_delete_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto* text = ace_napi_hook::fast::LazyStringData(key);
            if (text != nullptr) {
                auto st = ace_napi_hook::fast::DeleteNamedProperty(object, text, result);
                if (st == napi_ok) {
                    return ace_napi_hook::RecordStatus(env, st);
                }
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        key = ace_napi_hook::UseMaterializedLazyHandle(key);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_delete_property(ace_napi_hook::GetCangjieEnv(env), object, key, result));
    }
    return ace_napi_hook::orig_napi_delete_property(env, object, key, result);
}

extern "C" napi_status dispatch_napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (value == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        bool handled = false;
        auto st = ace_napi_hook::TrySetCapturedElement(env, object, index, value, &handled);
        if (handled || st != napi_ok) {
            return ace_napi_hook::RecordStatus(env, st);
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_set_element(ace_napi_hook::GetCangjieEnv(env), object, index, value));
    }
    return ace_napi_hook::orig_napi_set_element(env, object, index, value);
}

extern "C" napi_status dispatch_napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto st = ace_napi_hook::fast::GetElement(object, index, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_element(ace_napi_hook::GetCangjieEnv(env), object, index, result));
    }
    return ace_napi_hook::orig_napi_get_element(env, object, index, result);
}

extern "C" napi_status dispatch_napi_has_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto st = ace_napi_hook::fast::HasElement(object, index, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_has_element(ace_napi_hook::GetCangjieEnv(env), object, index, result));
    }
    return ace_napi_hook::orig_napi_has_element(env, object, index, result);
}

extern "C" napi_status dispatch_napi_delete_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(object)) {
            auto st = ace_napi_hook::fast::DeleteElement(object, index, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_delete_element(ace_napi_hook::GetCangjieEnv(env), object, index, result));
    }
    return ace_napi_hook::orig_napi_delete_element(env, object, index, result);
}

extern "C" napi_status dispatch_napi_is_array(napi_env env, napi_value value, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(value)) {
            auto st = ace_napi_hook::fast::IsArray(value, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_is_array(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_is_array(env, value, result);
}

extern "C" napi_status dispatch_napi_is_error(napi_env env, napi_value value, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_is_error(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_is_error(env, value, result);
}

extern "C" napi_status dispatch_napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::fast::IsUnmaterializedLazyValue(value)) {
            auto st = ace_napi_hook::fast::GetArrayLength(value, result);
            if (st == napi_ok) {
                return ace_napi_hook::RecordStatus(env, st);
            }
        }
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_array_length(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_get_array_length(env, value, result);
}

extern "C" napi_status dispatch_napi_get_property_names(napi_env env, napi_value object, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_property_names(ace_napi_hook::GetCangjieEnv(env), object, result));
    }
    return ace_napi_hook::orig_napi_get_property_names(env, object, result);
}

extern "C" napi_status dispatch_napi_get_all_property_names(napi_env env, napi_value object,
                                                            napi_key_collection_mode key_mode,
                                                            napi_key_filter key_filter,
                                                            napi_key_conversion key_conversion,
                                                            napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_all_property_names(ace_napi_hook::GetCangjieEnv(env), object,
                key_mode, key_filter, key_conversion, result));
    }
    return ace_napi_hook::orig_napi_get_all_property_names(env, object, key_mode, key_filter, key_conversion, result);
}

extern "C" napi_status dispatch_napi_get_value_string_utf8(napi_env env, napi_value value, char* buf,
                                                           size_t bufsize, size_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsLazyString(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetString(value, buf, bufsize, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_value_string_utf8(ace_napi_hook::GetCangjieEnv(env), value, buf, bufsize, result));
    }
    return ace_napi_hook::orig_napi_get_value_string_utf8(env, value, buf, bufsize, result);
}

extern "C" napi_status dispatch_napi_get_value_string_latin1(napi_env env, napi_value value, char* buf,
                                                             size_t bufsize, size_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsLazyString(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::GetString(value, buf, bufsize, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_value_string_latin1(ace_napi_hook::GetCangjieEnv(env), value, buf, bufsize, result));
    }
    return ace_napi_hook::orig_napi_get_value_string_latin1(env, value, buf, bufsize, result);
}

extern "C" napi_status dispatch_napi_create_reference(napi_env env, napi_value value,
                                                       uint32_t initial_refcount, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (result == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_create_reference(cj_env, value, initial_refcount, &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, initial_refcount, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_create_reference(env, value, initial_refcount, result);
}

extern "C" napi_status dispatch_napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* reference = ace_napi_hook::UnwrapCompatibleReference(ref);
        if (reference != nullptr) {
            if (result == nullptr) {
                return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
            }
            *result = reference->GetForEnv(ace_napi_hook::GetCangjieEnv(env));
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_get_reference_value(ace_napi_hook::GetCangjieEnv(env), ref, result));
    }
    return ace_napi_hook::orig_napi_get_reference_value(env, ref, result);
}

extern "C" napi_status dispatch_napi_delete_reference(napi_env env, napi_ref ref)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* reference = ace_napi_hook::UnwrapCompatibleReference(ref);
        if (reference != nullptr) {
            delete reference;
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_delete_reference(ace_napi_hook::GetCangjieEnv(env), ref));
    }
    return ace_napi_hook::orig_napi_delete_reference(env, ref);
}

extern "C" napi_status dispatch_napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* reference = ace_napi_hook::UnwrapCompatibleReference(ref);
        if (reference != nullptr) {
            auto count = reference->Ref();
            if (result != nullptr) {
                *result = count;
            }
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        auto status = ace_napi_hook::cj_napi_reference_ref(ace_napi_hook::GetCangjieEnv(env), ref, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_reference_ref(env, ref, result);
}

extern "C" napi_status dispatch_napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto* reference = ace_napi_hook::UnwrapCompatibleReference(ref);
        if (reference != nullptr) {
            auto count = reference->Unref();
            if (result != nullptr) {
                *result = count;
            }
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        auto status = ace_napi_hook::cj_napi_reference_unref(ace_napi_hook::GetCangjieEnv(env), ref, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_reference_unref(env, ref, result);
}

extern "C" napi_status dispatch_napi_wrap(napi_env env, napi_value js_object, void* native_object,
                                          napi_finalize finalize_cb, void* finalize_hint, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_wrap(cj_env, js_object, native_object,
            finalize_cb, finalize_hint, result == nullptr ? nullptr : &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_wrap(env, js_object, native_object, finalize_cb, finalize_hint, result);
}

extern "C" napi_status dispatch_napi_wrap_with_size(napi_env env, napi_value js_object, void* native_object,
                                                    napi_finalize finalize_cb, void* finalize_hint,
                                                    napi_ref* result, size_t native_binding_size)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_wrap_with_size(cj_env, js_object, native_object,
            finalize_cb, finalize_hint, result == nullptr ? nullptr : &cj_ref, native_binding_size);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_wrap_with_size(env, js_object, native_object, finalize_cb, finalize_hint,
                                                   result, native_binding_size);
}

extern "C" napi_status dispatch_napi_wrap_s(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    void* finalize_hint, const napi_type_tag* type_tag, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_wrap_s(
            cj_env, js_object, native_object, finalize_cb, finalize_hint, type_tag,
            result == nullptr ? nullptr : &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_wrap_s(
        env, js_object, native_object, finalize_cb, finalize_hint, type_tag, result);
}

extern "C" napi_status dispatch_napi_unwrap_s(
    napi_env env, napi_value js_object, const napi_type_tag* type_tag, void** result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::cj_napi_unwrap_s == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_generic_failure);
        }
        return ace_napi_hook::RecordStatus(
            env,
            ace_napi_hook::cj_napi_unwrap_s(
                ace_napi_hook::GetCangjieEnv(env), js_object, type_tag, result));
    }
    return ace_napi_hook::orig_napi_unwrap_s(env, js_object, type_tag, result);
}

extern "C" napi_status dispatch_napi_wrap_enhance_s(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    bool async_finalizer, void* finalize_hint, size_t native_binding_size,
    const napi_type_tag* type_tag, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_wrap_enhance_s(
            cj_env, js_object, native_object, finalize_cb, async_finalizer, finalize_hint,
            native_binding_size, type_tag, result == nullptr ? nullptr : &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_wrap_enhance_s(
        env, js_object, native_object, finalize_cb, async_finalizer, finalize_hint,
        native_binding_size, type_tag, result);
}

extern "C" napi_status dispatch_napi_wrap_enhance(
    napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    bool async_finalizer, void* finalize_hint, size_t native_binding_size, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_wrap_enhance(
            cj_env, js_object, native_object, finalize_cb, async_finalizer, finalize_hint,
            native_binding_size, result == nullptr ? nullptr : &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_wrap_enhance(
        env, js_object, native_object, finalize_cb, async_finalizer, finalize_hint,
        native_binding_size, result);
}

extern "C" napi_status dispatch_napi_add_finalizer(napi_env env, napi_value js_object, void* finalize_data,
                                                   napi_finalize finalize_cb, void* finalize_hint, napi_ref* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        auto* cj_env = ace_napi_hook::GetCangjieEnv(env);
        napi_ref cj_ref = nullptr;
        auto status = ace_napi_hook::cj_napi_add_finalizer(cj_env, js_object, finalize_data,
            finalize_cb, finalize_hint, result == nullptr ? nullptr : &cj_ref);
        status = ace_napi_hook::FinishReferenceResult(cj_env, status, cj_ref, 1, result);
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_add_finalizer(env, js_object, finalize_data, finalize_cb, finalize_hint, result);
}

extern "C" napi_status dispatch_napi_unwrap(napi_env env, napi_value js_object, void** result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_unwrap(ace_napi_hook::GetCangjieEnv(env), js_object, result));
    }
    return ace_napi_hook::orig_napi_unwrap(env, js_object, result);
}

extern "C" napi_status dispatch_napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        js_object = ace_napi_hook::UseMaterializedLazyHandle(js_object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_remove_wrap(ace_napi_hook::GetCangjieEnv(env), js_object, result));
    }
    return ace_napi_hook::orig_napi_remove_wrap(env, js_object, result);
}

extern "C" napi_status dispatch_napi_type_tag_object(napi_env env, napi_value value, const napi_type_tag* type_tag)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_type_tag_object(ace_napi_hook::GetCangjieEnv(env), value, type_tag));
    }
    return ace_napi_hook::orig_napi_type_tag_object(env, value, type_tag);
}

extern "C" napi_status dispatch_napi_check_object_type_tag(napi_env env, napi_value value,
                                                           const napi_type_tag* type_tag, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_check_object_type_tag(ace_napi_hook::GetCangjieEnv(env), value, type_tag, result));
    }
    return ace_napi_hook::orig_napi_check_object_type_tag(env, value, type_tag, result);
}

extern "C" napi_status dispatch_napi_call_function(napi_env env, napi_value recv, napi_value func,
                                                   size_t argc, const napi_value* argv, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (argc > 0 && argv == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        recv = ace_napi_hook::UseMaterializedLazyHandle(recv);
        func = ace_napi_hook::UseMaterializedLazyHandle(func);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_call_function(ace_napi_hook::GetCangjieEnv(env), recv, func, argc, argv, result));
    }
    return ace_napi_hook::orig_napi_call_function(env, recv, func, argc, argv, result);
}

extern "C" napi_status dispatch_napi_new_instance(napi_env env, napi_value constructor,
                                                  size_t argc, const napi_value* argv, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (argc > 0 && argv == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        constructor = ace_napi_hook::UseMaterializedLazyHandle(constructor);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_new_instance(
                ace_napi_hook::GetCangjieEnv(env), constructor, argc, argv, result));
    }
    return ace_napi_hook::orig_napi_new_instance(env, constructor, argc, argv, result);
}

extern "C" napi_status dispatch_napi_define_properties(napi_env env, napi_value object,
                                                       size_t property_count,
                                                       const napi_property_descriptor* properties)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (property_count > 0 && properties == nullptr) {
            return ace_napi_hook::RecordStatus(env, napi_invalid_arg);
        }
        object = ace_napi_hook::UseMaterializedLazyHandle(object);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_define_properties(ace_napi_hook::GetCangjieEnv(env), object,
                property_count, properties));
    }
    return ace_napi_hook::orig_napi_define_properties(env, object, property_count, properties);
}

extern "C" napi_status dispatch_napi_throw(napi_env env, napi_value error)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        error = ace_napi_hook::UseMaterializedLazyHandle(error);
        return ace_napi_hook::RecordPendingException(env,
            ace_napi_hook::cj_napi_throw(ace_napi_hook::GetCangjieEnv(env), error));
    }
    return ace_napi_hook::orig_napi_throw(env, error);
}

extern "C" napi_status dispatch_napi_fatal_exception(napi_env env, napi_value err)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        err = ace_napi_hook::UseMaterializedLazyHandle(err);
        return ace_napi_hook::RecordPendingException(env,
            ace_napi_hook::cj_napi_fatal_exception(ace_napi_hook::GetCangjieEnv(env), err));
    }
    return ace_napi_hook::orig_napi_fatal_exception(env, err);
}

extern "C" napi_status dispatch_napi_get_and_clear_last_exception(napi_env env, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto status = ace_napi_hook::cj_napi_get_and_clear_last_exception(
            ace_napi_hook::GetCangjieEnv(env), result);
        if (status == napi_ok) {
            ace_napi_hook::ClearPendingException(env);
        }
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_get_and_clear_last_exception(
        ace_napi_hook::mixed_napi_env::unwrap(env), result);
}

extern "C" napi_status dispatch_napi_is_exception_pending(napi_env env, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        auto status = ace_napi_hook::cj_napi_is_exception_pending(
            ace_napi_hook::GetCangjieEnv(env), result);
        if (status == napi_ok && result != nullptr) {
            ace_napi_hook::SetPendingException(env, *result);
        }
        return ace_napi_hook::RecordStatus(env, status);
    }
    return ace_napi_hook::orig_napi_is_exception_pending(
        ace_napi_hook::mixed_napi_env::unwrap(env), result);
}

extern "C" napi_status dispatch_napi_throw_error(napi_env env, const char* code, const char* msg)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordPendingException(env,
            ace_napi_hook::cj_napi_throw_error(ace_napi_hook::GetCangjieEnv(env), code, msg));
    }
    return ace_napi_hook::orig_napi_throw_error(env, code, msg);
}

extern "C" napi_status dispatch_napi_throw_range_error(napi_env env, const char* code, const char* msg)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordPendingException(env,
            ace_napi_hook::cj_napi_throw_range_error(ace_napi_hook::GetCangjieEnv(env), code, msg));
    }
    return ace_napi_hook::orig_napi_throw_range_error(env, code, msg);
}

extern "C" napi_status dispatch_napi_throw_type_error(napi_env env, const char* code, const char* msg)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordPendingException(env,
            ace_napi_hook::cj_napi_throw_type_error(ace_napi_hook::GetCangjieEnv(env), code, msg));
    }
    return ace_napi_hook::orig_napi_throw_type_error(env, code, msg);
}

extern "C" napi_status dispatch_napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        code = ace_napi_hook::UseMaterializedLazyHandle(code);
        msg = ace_napi_hook::UseMaterializedLazyHandle(msg);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_error(ace_napi_hook::GetCangjieEnv(env), code, msg, result));
    }
    return ace_napi_hook::orig_napi_create_error(env, code, msg, result);
}

extern "C" napi_status dispatch_napi_create_type_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        code = ace_napi_hook::UseMaterializedLazyHandle(code);
        msg = ace_napi_hook::UseMaterializedLazyHandle(msg);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_type_error(ace_napi_hook::GetCangjieEnv(env), code, msg, result));
    }
    return ace_napi_hook::orig_napi_create_type_error(env, code, msg, result);
}

extern "C" napi_status dispatch_napi_create_range_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_create_range_error(ace_napi_hook::GetCangjieEnv(env), code, msg, result));
    }
    return ace_napi_hook::orig_napi_create_range_error(env, code, msg, result);
}

extern "C" napi_status dispatch_napi_get_last_error_info(node_api_basic_env env, const napi_extended_error_info** result)
{
    if (result == nullptr) {
        return napi_invalid_arg;
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        static thread_local napi_extended_error_info info {};
        napi_status last_status = napi_ok;
        bool last_error_clean = true;
        ace_napi_hook::GetLastErrorSnapshot(env, last_status, last_error_clean);
        if (last_error_clean) {
            ace_napi_hook::FillErrorInfo(info, napi_ok);
            *result = &info;
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        auto status = ace_napi_hook::cj_napi_get_last_error_info(
            ace_napi_hook::GetCangjieEnv(env), result);
        if (status == napi_ok) {
            return ace_napi_hook::RecordStatus(env, napi_ok);
        }
        ace_napi_hook::FillErrorInfo(info, last_status);
        *result = &info;
        return ace_napi_hook::RecordStatus(env, napi_ok);
    }
    return ace_napi_hook::orig_napi_get_last_error_info(env, result);
}

extern "C" napi_status dispatch_napi_typeof(napi_env env, napi_value value, napi_valuetype* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsFastValue(value)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::Typeof(env, value, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) && ace_napi_hook::fast::IsUnmaterializedLazyValue(value)) {
        auto st = ace_napi_hook::fast::TypeofLazy(value, result);
        if (st == napi_ok) {
            return ace_napi_hook::RecordStatus(env, st);
        }
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        value = ace_napi_hook::UseMaterializedLazyHandle(value);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_typeof(ace_napi_hook::GetCangjieEnv(env), value, result));
    }
    return ace_napi_hook::orig_napi_typeof(env, value, result);
}

extern "C" napi_status dispatch_napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(env) &&
        ace_napi_hook::fast::IsFastValue(lhs) && ace_napi_hook::fast::IsFastValue(rhs)) {
        return ace_napi_hook::RecordStatus(env, ace_napi_hook::fast::StrictEquals(env, lhs, rhs, result));
    }
    if (ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        lhs = ace_napi_hook::UseMaterializedLazyHandle(lhs);
        rhs = ace_napi_hook::UseMaterializedLazyHandle(rhs);
        return ace_napi_hook::RecordStatus(env,
            ace_napi_hook::cj_napi_strict_equals(ace_napi_hook::GetCangjieEnv(env), lhs, rhs, result));
    }
    return ace_napi_hook::orig_napi_strict_equals(env, lhs, rhs, result);
}
#undef DEFINE_TSFN_WRAPPER
#undef DEFINE_NO_ENV_WRAPPER
#undef DEFINE_ENV_WRAPPER

extern "C" napi_status dispatch_napi_remove_async_cleanup_hook(
    napi_async_cleanup_hook_handle remove_handle)
{
    if (ace_napi_hook::is_tagged_cangjie_pointer(remove_handle)) {
        return ace_napi_hook::cj_napi_remove_async_cleanup_hook(remove_handle);
    }
    return ace_napi_hook::orig_napi_remove_async_cleanup_hook(remove_handle);
}

extern "C" void dispatch_napi_fatal_error(const char* location,
                                          size_t location_len,
                                          const char* message,
                                          size_t message_len)
{
    return ace_napi_hook::orig_napi_fatal_error(location, location_len, message, message_len);
}

extern "C" napi_status dispatch_napi_send_event(napi_env env,
                                                const std::function<void()>& cb,
                                                int prior,
                                                const char* name,
                                                int barrier_option)
{
    if (!ace_napi_hook::is_tagged_cangjie_pointer(env)) {
        if (ace_napi_hook::orig_napi_send_event == nullptr) {
            return napi_generic_failure;
        }
        return ace_napi_hook::orig_napi_send_event(
            ace_napi_hook::mixed_napi_env::unwrap(env), cb, prior, name, barrier_option);
    }
    auto* state = ace_napi_hook::GetDispatchStateFromEnv(env);
    if (state == nullptr || state->platform_env == nullptr ||
        ace_napi_hook::orig_napi_send_event == nullptr) {
        return napi_invalid_arg;
    }
    return ace_napi_hook::orig_napi_send_event(
        state->platform_env, cb, prior, name, barrier_option);
}

extern "C" int dispatch_uv_async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb)
{
    if (IsTaggedUvLoop(loop)) {
        int rc = ace_napi_hook::cj_uv_async_init(loop, async, async_cb);
        if (rc == 0) {
            RememberHandle(GetAsyncHandles(), async);
        }
        return rc;
    }
    return ace_napi_hook::orig_uv_async_init(loop, async, async_cb);
}

extern "C" int dispatch_uv_async_send(uv_async_t* async)
{
    if (IsKnownAsyncLikeHandle(async)) {
        return ace_napi_hook::cj_uv_async_send(async);
    }
    return ace_napi_hook::orig_uv_async_send(async);
}

extern "C" int dispatch_uv_timer_init(uv_loop_t* loop, uv_timer_t* timer)
{
    if (IsTaggedUvLoop(loop)) {
        int rc = ace_napi_hook::cj_uv_timer_init(loop, timer);
        if (rc == 0) {
            RememberHandle(GetTimerHandles(), timer);
        }
        return rc;
    }
    return ace_napi_hook::orig_uv_timer_init(loop, timer);
}

extern "C" int dispatch_uv_timer_start(uv_timer_t* timer, uv_timer_cb cb, uint64_t timeout, uint64_t repeat)
{
    if (IsKnownTimerLikeHandle(timer)) {
        return ace_napi_hook::cj_uv_timer_start(timer, cb, timeout, repeat);
    }
    return ace_napi_hook::orig_uv_timer_start(timer, cb, timeout, repeat);
}

extern "C" int dispatch_uv_timer_stop(uv_timer_t* timer)
{
    if (IsKnownTimerLikeHandle(timer)) {
        return ace_napi_hook::cj_uv_timer_stop(timer);
    }
    return ace_napi_hook::orig_uv_timer_stop(timer);
}

extern "C" int dispatch_uv_queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb)
{
    if (IsTaggedUvLoop(loop)) {
        return ace_napi_hook::cj_uv_queue_work(loop, req, work_cb, after_work_cb);
    }
    return ace_napi_hook::orig_uv_queue_work(loop, req, work_cb, after_work_cb);
}

extern "C" int dispatch_uv_queue_work_with_qos(
    uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb, uv_after_work_cb after_work_cb, int qos)
{
    if (IsTaggedUvLoop(loop)) {
        return ace_napi_hook::cj_uv_queue_work_with_qos(loop, req, work_cb, after_work_cb, qos);
    }
    return ace_napi_hook::orig_uv_queue_work_with_qos(loop, req, work_cb, after_work_cb, qos);
}

extern "C" void dispatch_uv_close(uv_handle_t* handle, uv_close_cb close_cb)
{
    if (ForgetHandle(GetAsyncHandles(), handle) || ForgetHandle(GetTimerHandles(), handle)) {
        ace_napi_hook::cj_uv_close(handle, close_cb);
        return;
    }
    ace_napi_hook::orig_uv_close(handle, close_cb);
}

extern "C" int64_t dispatch_ffrt_this_task_get_id()
{
    // net.socket requires event callbacks to observe the same FFRT task id as
    // the socket instance creation path, while Cangjie work may change threads.
    return 1234;
}

}  // namespace ace_napi_hook
